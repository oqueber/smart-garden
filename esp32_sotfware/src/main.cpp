#include <Arduino.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <SD.cpp>

#include "soc/sens_reg.h" // needed for manipulating ADC2 control register

static RTC_NOINIT_ATTR int reg_b; // place in RTC slow memory so available after deepsleep
static RTC_NOINIT_ATTR int counterSleep; 
static RTC_NOINIT_ATTR bool plant_LEDOn = false; 

#include "config.h"
#include <sistem_error.cpp>

#include <./wifi/wifi.h>
#include <./wifi/wifi.cpp>

#include <sistem_control.cpp>

#include <./mqtt/mqtt.h>
#include <./mqtt/mqtt.cpp>

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

void swichs_on_off(int io){
  digitalWrite(SW1, io);
  delay(5000);
}
void getDataDig(String select , bool send = true){
  String json = "";
  DynamicJsonDocument doc(1024);
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

      JsonObject Data = doc.createNestedObject("TCS34725");
      Data["R"] = r;
      Data["G"] = g;
      Data["B"] = b;
      Data["C"] = c;
      Data["ColorTemp"] = colorTemp;
      Data["Lux"] = lux;
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

        JsonObject Data = doc.createNestedObject("CCS811");
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

      JsonObject Data = doc.createNestedObject("Si7021");
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

      JsonObject Data = doc.createNestedObject("BME280");
      Data["Temp"] = readTempC;
      Data["Pressure"] = readFloatPressure;
      Data["Altitude"] = readFloatAltitudeMeters;
    
    }
  }

  serializeJson(doc, json);
  if(send) {
    send_mqtt("Huerta/Push/Digital" ,json);
  }else{
    Serial.println("");
    Serial.println("json:");
    Serial.println(json);
  }
}
void getDataAnalog(JsonObjectConst User, String plant, bool send = true ){
  String json = "";
  DynamicJsonDocument doc(1024);
  JsonObject analog = doc.createNestedObject("Analog");
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

  serializeJson(doc, json);
  if(send) {
    send_mqtt("Huerta/Push/Analog" ,json);
  }else{
    Serial.println("Enviando a mqtt");
    Serial.println(json);
  }
}


void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  /*
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour,3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay,10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
  */
}

void setup(){
  if (debugging || debugging_mqtt){
    Serial.begin(115200);
    delay(100);
  }

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
      
      if(plant_LEDOn){
        counterSleep++; 
      }

      Serial.println("Wakeup caused by timer"); break;
      digitalWrite(LED_GREEN,HIGH);
      delay(1000);

    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
      Serial.println("Wakeup caused by touchpad"); break;
      for(int i = 0; i<10; i++){
        digitalWrite(LED_GREEN,LOW);
        delay(1000);
        digitalWrite(LED_GREEN,HIGH);
        delay(1000);
      }  
    default : 
      counterSleep = 0;
      reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
      Serial.println("Reading reg b.....");
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
    
  //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(touchPin, callback_touch, Threshold);
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

  esp_wifi_start();



  // If we can connected to wifi..
  if ( wifi_status() ){
    // definimos el servidor y el puerto del MQTT
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);

    // get NTP time every time connect to wifi, not necessary but wont hurts
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Try to get the user by the server
    if ( !getUser() ){
      // Try to get the user by SDs
      if( !SD.begin() ){
        //if we fount any user. we have nothing to do, go to sleep
        sisError(0);
      }
      SD.end();
    }
    
  }else{
    
    // If we can't fount the user in local. Go to sleep
    if(!SD.begin()){
      Serial.println("Card Mount Failed");
      //sisError(5);
      //Nothing to do, reboot
      sisError(0);
    }else{
      // Check to see if the file exists:
      if (SD.exists(SD_path_user)) {
        String stringUser = readFile(SD, SD_path_user);

        auto error = deserializeJson(localUser, stringUser );
        if( error == DeserializationError::Ok ){  
          userLocalFlag = true ;
        }

      } else {
        Serial.println("userData.txt doesn't exist.");
      }

    }  
    SD.end();
  }
  

  Wire.begin();
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
  
  if (debugging || debugging_mqtt){
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.print("User mac: ");
    Serial.println(getMac());
    Serial.print("SD on: ");
    Serial.println(SD.cardType() == CARD_NONE ? "No" : "Yes");
    Serial.print("LED ON: ");
    Serial.println(plant_LEDOn == false ? "No" : "Yes");
    Serial.print("Reinicios: ");
    Serial.println(counterSleep);

    if ( userLocalFlag ){
      Serial.print("LocalUser: True");
      //Serial.println( readFile(SD, SD_path_user) );
    }else{
      Serial.println("LocalUser: False");
    }

    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +" Seconds");
    printLocalTime();
    Serial.println("----------------------------------------------------------");
  }

}

void taskCore0( void * pvParameters){
  Serial.print("Task of the core 0:");
  Serial.println(xPortGetCoreID());
  for(;;){

    while ( !finishedCore1 )
    {  
      if (!client.connected())  // Reconnect if connection is lost
      {
        reconnect();
      }

      client.loop();
      delay(500);
    }

    finishedCore0 = true;
    client.disconnect();
    Serial.println("Core 0 finished");
    while (true) { delay(1000);}
  }
  
}
void taskCore1( void * pvParameters){
  Serial.print("Task of the core 1:");
  Serial.println(xPortGetCoreID());
  for(;;){
    digitalWrite(LED_GREEN, HIGH);
    swichs_on_off(HIGH);
    /* 
    *  Each plant has 4 measurements (2 Photocel, 1 HumCap and HumEC)
    *  and this is send to database for store it.
    */
    for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
      
      //Definir los datos analogicos y los datos de riego y de iluminacion para ser activados
      getDataAnalog(kvp.value(), kvp.key().c_str() ); 
      taskSystem(kvp.value() ); 

    }
    /* 
    *  Each User has max 4 measurements ( TCS34725, BME280, CCS811 and Si7021 )
    *  and this is send to database for store it.
    */
    getDataDig(  localUser["Measurements"] );
    swichs_on_off(LOW);
    
    Serial.println("re send the mqtt messages");
    //sendPedientingMessage();
    //  Do something with all tasks received
    //controlSystem();
    //tasksControl();
    
    finishedCore1 = true;

    while ( !finishedCore0 ){
      delay(1000);
    }

    Serial.println("Core 1 finished");
    Serial.flush();
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
    if( wifi_status() ){
      if ( !getUser()){
        if( !userLocalFlag ){
          sisError(0);
        }
      }
    }
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