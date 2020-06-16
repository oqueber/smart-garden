#include <Arduino.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <esp_bt.h>


#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include "soc/sens_reg.h" // needed for manipulating ADC2 control register

static RTC_NOINIT_ATTR int reg_b; // place in RTC slow memory so available after deepsleep

#include "config.h"
#include <sistem_error.cpp>

#include <./wifi/wifi.h>
#include <./wifi/wifi.cpp>

#include <sistem_control.cpp>

#include <./mqtt/mqtt.h>
#include <./mqtt/mqtt.cpp>

#include "setup_sensors.cpp"
#define Threshold 40 /* Greater the value, more the sensitivity */

RTC_DATA_ATTR int bootCount = 0;
const int touchPin = 4;


TaskHandle_t Task0;
TaskHandle_t Task1;
TaskHandle_t Task2;

bool finishedCore1 = false;
bool finishedCore0 = false;

void Sensors_on_off(int io){
  digitalWrite(SW1, io);
  delay(1000);
}
void getDataDig(String select){
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
  
  if (debugging){
    Serial.println("");
    Serial.println("json:");
    Serial.println(json);
  }
  send_mqtt("Huerta/Push/Digital" ,json);
}
void getDataAnalog(JsonObjectConst User, String plant ){
  String json = "";
  DynamicJsonDocument doc(1024);
  JsonObject analog = doc.createNestedObject("Analog");
  analog["plantId"] = plant;

  unsigned int SumaAnalog = 0;
  unsigned int samples = 10; 

  for (auto element : User) {

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
    //path["pin"] = element.value();
    SumaAnalog = 0;
  }

  serializeJson(doc, json);
  Serial.println("Enviando a mqtt");
  Serial.println(json);
  send_mqtt("Huerta/Push/Analog" ,json);
}
void taskCore2( void * pvParameters);
void taskCore1( void * pvParameters);
void taskCore0( void * pvParameters);
void callback_touch(){
  //placeholder callback function
}
void setup(){
  if (debugging || debugging_mqtt){
    Serial.begin(115200);
    delay(100);
  }
  
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : digitalWrite(LED_BLUE,HIGH); Serial.println("Wakeup caused by touchpad"); break;
    default : 
      reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
      Serial.println("Reading reg b.....");
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }

  //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(touchPin, callback_touch, Threshold);

  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

  esp_wifi_start();

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  Wire.begin();

  if ( wifi_status() ){
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);
  }

  delay(100);
  xTaskCreatePinnedToCore(taskCore1, "Task1", 10000, NULL, 1, &Task1,  1); 
  delay(500);

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TOUCHPAD){
    xTaskCreatePinnedToCore(taskCore2, "Task2", 10000, NULL, 1, &Task2,  0); 
    delay(500); 
  }
  else{
    xTaskCreatePinnedToCore(taskCore0, "Task0", 10000, NULL, 1, &Task0,  0); 
    delay(500); 
  }
  

  if (debugging || debugging_mqtt){
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.print("User: ");
    Serial.println(getMac());
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +" Seconds");
    Serial.println("----------------------------------------------------------");
  }

}

void taskCore1( void * pvParameters){
  Serial.print("Task of the core 1:");
  Serial.println(xPortGetCoreID());
  for(;;){
    digitalWrite(LED_GREEN, HIGH);
    if( wifi_status() ){
      if ( getUser() ){
        /* 
        *  Each plant has 4 measurements (2 Photocel, 1 HumCap and HumEC)
        *  and this is send to database for store it.
        */
        for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
          getDataAnalog(kvp.value(), kvp.key().c_str() ); 
        }
        delay(100);
        /* 
        *  Each User has max 4 measurements ( TCS34725, BME280, CCS811 and Si7021 )
        *  and this is send to database for store it.
        */
        getDataDig(  localUser["Measurements"] );
      }
    }  
    finishedCore1 = true;
    controlSystem();

    while ( !finishedCore0 ){
      delay(1000);
    }

    Serial.println("Core 1 finished");
    Serial.flush();
    toSleep(TIME_TO_SLEEP);
  }
  
}
void taskCore2( void * pvParameters){
  Serial.print("Task by touch:");
  Serial.println(xPortGetCoreID());
  delay(10000);
  for(;;){

    if( touchRead(touchPin) < Threshold){
      digitalWrite(LED_BLUE,LOW);
      Serial.println("Task2 by touch finished");
      client.disconnect();
      delay(5000);
      toSleep(TIME_TO_SLEEP);
    }


    Serial.println("Task2 by touch new loop");
    if (!client.connected())  // Reconnect if connection is lost
    {
      reconnect();
    }
    client.loop();
    delay(500);
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

/*
{"Measurements":"1111",
"Plants":{
  "Plant1":{"PhotoCell1":"A0","PhotoCell2":"A1","HumCap":"A2","HumEC":"A3"},
  "Plant2":{"HumCap":"A4","HumEC":"A5","Photocell1":"A0","Photocell2":"A1"}
  }
}
*/
void loop(){
  
}