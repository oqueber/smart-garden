#include <Arduino.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#define ARDUINOJSON_USE_LONG_LONG 1 // we need to store long long
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <SD.cpp>

#include "soc/sens_reg.h" // needed for manipulating ADC2 control register


#include "config.h"

static RTC_NOINIT_ATTR int reg_b; // place in RTC slow memory so available after deepsleep
static RTC_NOINIT_ATTR int counterSleep; 
static RTC_DATA_ATTR time_t timeSystem;;
DynamicJsonDocument localUser (2048);
static RTC_DATA_ATTR struct Plant_status plantStatus[MAX_PLANTS];

#include <sistem_error.cpp>

#include <./wifi/wifi.h>
#include <./wifi/wifi.cpp>


#include <./mqtt/mqtt.h>

#include <./mqtt/mqtt.cpp>

#include <sistem_control.cpp>
#include "setup_sensors.cpp"
#define Threshold 40 /* Greater the value, more the sensitivity */


TaskHandle_t Task0;
TaskHandle_t Task1;
TaskHandle_t Task2;

void taskCore2( void * pvParameters);
void taskCore1( void * pvParameters);
void taskCore0( void * pvParameters);
void callback_touch(){};

bool finishedCore1 = false;
bool finishedCore0 = false;
uint8_t iServerMqtt = 0;

uint8_t iMsgMqtt = 0;

struct SendMqttMsg {
  bool send;
  String msg;
};
struct SendMqttMsg SendMqttMsg[MAX_PLANTS];


void swichs_on_off(int io){
  Serial.print("\nSwitch mode: ");
  Serial.println(io);
  digitalWrite(SW1, io);
  delay(5000);
}
void getDataDig(String select , bool send = true){
  String json = "";
  DynamicJsonDocument parte1(200);
  DynamicJsonDocument parte2(200);
  //doc["PlantId"] = "measurement";
  
  if (select[0] == '1'){
    if (Setup_TCS34725() ){
    
      uint16_t r, g, b, c;
      int lux,colorTemp;

      tcs.setInterrupt(false);
      delay(710); // Cuesta 700ms capturar el color
      tcs.getRawData(&r, &g, &b, &c);
      tcs.setInterrupt(true);
    
      colorTemp = tcs.calculateColorTemperature(r, g, b);
      lux = tcs.calculateLux(r, g, b);

      if (debugging){
          Serial.print("TCS34725 Sensor:");
          Serial.print(r);Serial.print(",");
          Serial.print(g);Serial.print(",");
          Serial.print(b);Serial.print(",");
          Serial.print(c);Serial.print(",");
          Serial.print(colorTemp);Serial.print(",");
          Serial.print(lux);Serial.println("");
      }

      JsonObject Data = parte1.createNestedObject("TCS34725");
      Data["R"] = r;
      Data["G"] = g;
      Data["B"] = b;
      Data["C"] = c;
      Data["ColorTemp"] = colorTemp;
      Data["Lux"] = lux;

      serializeJson(parte1, json);
      if(send) {
        send_mqtt("Huerta/Push/Digital" ,json);
      }else{
        Serial.println("");
        Serial.println("json:");
        Serial.println(json);
      }
      json = "";
    }
  }
  if (select[3] == '1'){
    if ( Setup_CCS811() ){
      //Check to see if data is available
      if (myCCS811.dataAvailable()){
        //Calling this function updates the global tVOC and eCO2 variables
        myCCS811.readAlgorithmResults();

        int CO2 = myCCS811.getCO2();
        int TVOC = myCCS811.getTVOC();

        if (debugging){
          Serial.print("CCS811 Sensor:");
          Serial.print(CO2);Serial.print(",");
          Serial.print(TVOC);Serial.println("");
        }

        JsonObject Data = parte2.createNestedObject("CCS811");
        Data["CO2"] = CO2;
        Data["TVOC"] = TVOC;
        
      }
      else if (myCCS811.checkForStatusError() && debugging ){
        //If the CCS811 found an internal error, print it.
        printSensorError();
      }
    }
  }
  if (select[2] == '1'){
    if ( Setup_Si7021() ){
      float SItempC = sensor.readTemperature();
      float SIhumid = sensor.readHumidity();

      if (select[3] == '1'){        
        //This sends the temperature data to the CCS811
        myCCS811.setEnvironmentalData(SIhumid, SItempC);
      }

      if (debugging){
        Serial.print("Si7021 Sensor:");
        Serial.print(SItempC,2);Serial.print(",");
        Serial.print(SIhumid,2);Serial.println("");  
      }

      JsonObject Data = parte2.createNestedObject("Si7021");
      Data["Temp"] = SItempC;
      Data["Humi"] = SIhumid;
      
    }
  }
  if (select[1] == '1'){
    if (Setup_BME280()){

      float readTempC = myBME280.readTempC();
      float readFloatPressure = myBME280.readFloatPressure();
      float readFloatAltitudeMeters = myBME280.readFloatAltitudeMeters();

      if (debugging){
          Serial.print("myBME280 Sensor:");
          Serial.print(readTempC,2);Serial.print(",");
          Serial.print(readFloatPressure, 2);Serial.print(",");
          Serial.print(readFloatAltitudeMeters, 2);Serial.println("");
      }

      JsonObject Data = parte2.createNestedObject("BME280");
      Data["Temp"] = readTempC;
      Data["Pressure"] = readFloatPressure;
      Data["Altitude"] = readFloatAltitudeMeters;
    
    }
  }

  serializeJson(parte2, json);
  if(send) {
    send_mqtt("Huerta/Push/Digital" ,json);
  }else{
    Serial.println("");
    Serial.println("json:");
    Serial.println(json);
  }
}
void getDataAnalog(JsonObjectConst User, String plant, bool send = true ){
  DynamicJsonDocument analog(1024);
  //JsonObject analog = doc.createNestedObject("Analog");
  analog["plantId"] = plant;

  unsigned int SumaAnalog = 0;
  unsigned int samples = 10; 

  for (auto element : User["pinout"].as<JsonObject>() ) {

    for(int i = 0; i < samples; i++){
      
      if (element.value() < 30){
        // ADC2 control register restoring
        WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
        //VERY IMPORTANT: DO THIS TO NOT HAVE INVERTED VALUES!
        SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
        //We have to do the 2 previous instructions BEFORE EVERY analogRead() calling!
      }
      SumaAnalog += analogRead( element.value() );
      delay(50);
    }

    JsonObject path = analog.createNestedObject(element.key());
    path["rawData"] = (SumaAnalog/samples);
    
    if(!send){path["pin"] = element.value();}
    SumaAnalog = 0;
  }

  serializeJson(analog, SendMqttMsg[iMsgMqtt].msg);
  if(send) {
    SendMqttMsg[iMsgMqtt].send = true;
  }else{
    SendMqttMsg[iMsgMqtt].send = false;
  }

}
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup(){

  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
  //#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  //  clock_prescale_set(clock_div_1);
  //#endif
  // END of Trinket-specific code.
  //pixels.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)

  FastLED.addLeds<NEOPIXEL, pin_pixel>(leds, num_pixels);

  if (debugging || debugging_mqtt){
    Serial.begin(115200);
    delay(100);
  }

  //Inicializacion de variables
  iMsgMqtt = 0;


  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_GREEN, OUTPUT);
  pinMode(SW1, OUTPUT);
  pinMode(waterPin0, OUTPUT);
  pinMode(waterPin1, OUTPUT);
  pinMode(waterPin1, OUTPUT);
  pinMode(waterPin2, OUTPUT);
  pinMode(waterPin3, OUTPUT);
  pinMode(neoPin, OUTPUT);
    
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_TIMER :
      
      Serial.println(""); 
      Serial.print("Wakeup caused by timer:");
      Serial.println(counterSleep);  
      digitalWrite(LED_GREEN,HIGH);
      delay(1000);
      
      if(counterSleep++ >= 48){
        Serial.println("reset preventivo.");
        //Traer la ultima version del usuario
        getUser();
        //Introducirle los datos almacenados en local
        updateLastData();
        //Guardarlos en la SD, antes de reinicio preventivo
        getUserSD( true );
        ESP.restart();
      }

      break;

    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
      Serial.println("Wakeup caused by touchpad");
      for(int i = 0; i<10; i++){
        digitalWrite(LED_GREEN,LOW);
        delay(1000);
        digitalWrite(LED_GREEN,HIGH);
        delay(1000);
      } 
      break;

    default : 
      counterSleep = 0;
      itsHardReboot = true;
      //pixels.clear();
      //pixels.show();

      reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
      Serial.println("Reading reg b.....");
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
      break;
  }
    
  //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(touchPin, callback_touch, Threshold);
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

  esp_wifi_start();
  Wire.begin();

  // If we can connected to wifi..
  if ( wifi_status() ){
    Serial.println("We connected to the wifi...");

    // definimos el servidor y el puerto del MQTT
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);

    // get NTP time every time connect to wifi, not necessary but wont hurts
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Try to get the user by the server
    // True: updated or same data
    // false: bad json was send
    if( !getUser())
    { 
      //if we fount any user. we have nothing to do, go to sleep
      sisError(0);
    }
    
    delay(100);


  }else if(!getUserSD()){
    //Nothing to do, reboot
    sisError(0);
  }

  updateLastData();

  if (debugging || debugging_mqtt){
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.print("User mac: ");
    Serial.println(getMac());
    Serial.print("HardReboot: ");
    Serial.println(itsHardReboot ? "Yes" : "No");
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +" Seconds");
    printLocalTime();
    Serial.println("----------------------------------------------------------");
  }

  do
  {
    reconnect();
    delay(5000);
    iServerMqtt++;
  }while((!client.connected()) && (iServerMqtt <= 10 ));

  if( iServerMqtt >= 10)
  {
    ESP.restart();
    Serial.println("server unreach, power off uC");
  }

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD){
    xTaskCreatePinnedToCore(taskCore2, "Task2", 10000, NULL, 1, &Task2,  0); 
    delay(500); 
  
  }else{

    delay(100);
    xTaskCreatePinnedToCore(taskCore1, "Task1", 10000, NULL, 1, &Task1,  1); 
    delay(500);
    xTaskCreatePinnedToCore(taskCore0, "Task0", 10000, NULL, 1, &Task0,  0); 
    delay(500); 
  }
  time(&now);
}

void taskCore0( void * pvParameters){
  //Serial.print("Task of the core 0:");
  //Serial.println(xPortGetCoreID());
  for(;;){

    while ( !finishedCore1 )
    {  
      if (!client.connected())  // Reconnect if connection is lost
      {
        reconnect();
      }
      client.loop();
      delay(100);
    }

    finishedCore0 = true;
    client.disconnect();
    Serial.println("Core 0 finished");
    while (true) { delay(1000);}
  }
  
}
void taskCore1( void * pvParameters){
  //Serial.print("Task of the core 1:");
  //Serial.println(xPortGetCoreID());
  for(;;){

    //Led indicador que esta en funcionamiento
    
    digitalWrite(LED_GREEN, HIGH);

    /* 
    *  Each User has max 4 measurements ( TCS34725, BME280, CCS811 and Si7021 )
    *  and this is send to database for store it.
    */
    getDataDig(  localUser["Measurements"] );

    //Damos electricidad a los sensores analogicos
    swichs_on_off(HIGH);
    /* 
    *  Each plant has 4 measurements (2 Photocel, 1 HumCap and HumEC)
    *  and this is send to database for store it.
    */
    for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
      
      //Definir los datos analogicos y los datos de riego y de iluminacion para ser activados
      getDataAnalog(kvp.value(), kvp.key().c_str() ); 
      iMsgMqtt++;
    }
    //Apagamos los sensores analogicos
    swichs_on_off(LOW);
    
    //Por cada planta, enviamos cada mensaje analogico
    for(iMsgMqtt = 0; iMsgMqtt< MAX_PLANTS; iMsgMqtt++)
    {
      Serial.printf("\n Enviando a mqtt: %d \n", SendMqttMsg[iMsgMqtt].send );
      Serial.println(SendMqttMsg[iMsgMqtt].msg);

      if (SendMqttMsg[iMsgMqtt].send == true )
      {
        send_mqtt("Huerta/Push/Analog" ,SendMqttMsg[iMsgMqtt].msg);
      }
    }

    //por cada planta, realizamos la rutina de tareas (Riego y/o iluminacion)
    for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
      
      //Definir los datos analogicos y los datos de riego y de iluminacion para ser activados
      taskSystem(kvp.value(), kvp.key().c_str() ); 
    }
    
    Serial.println("re send the mqtt messages");
    //sendPedientingMessage();
    //  Do something with all tasks received
    //controlSystem();
    //tasksControl();
    
    finishedCore1 = true;

    while ( !finishedCore0 ){
      delay(1000);
    }

    saveUser();
    toSleep(TIME_TO_SLEEP);
  }
}
// manual mood
void taskCore2( void * pvParameters){
  Serial.print("Task by touch:");
  Serial.println(xPortGetCoreID());
  delay(10000);
  for(;;){

    if( touchRead(touchPin) < Threshold){
      digitalWrite(LED_GREEN,LOW);
      Serial.println("Task2 by touch finished");
      client.disconnect();
      delay(5000);
      toSleep(TIME_TO_SLEEP);
    }


    Serial.println("Task2 by touch new loop");
    //if( wifi_status() ){
    //  if ( !getUser() && !getUserSD() ){
    //    sisError(0);
    //  }
    //}
    
    swichs_on_off(HIGH);
    /* 
    *  Each plant has 4 measurements (2 Photocel, 1 HumCap and HumEC)
    *  and this is send to database for store it.
    */
    for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
      getDataAnalog(kvp.value(), kvp.key().c_str(),false ); 
    }
    /* 
    *  Each User has max 4 measurements ( TCS34725, BME280, CCS811 and Si7021 )
    *  and this is send to database for store it.
    */
    getDataDig(  localUser["Measurements"],false );
    
    swichs_on_off(LOW);
    if (!client.connected()){  // Reconnect if connection is lost
      reconnect();
    }
    client.loop();
    delay(10000);

  }
  
}


void loop(){}