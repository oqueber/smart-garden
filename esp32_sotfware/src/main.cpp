#include <Arduino.h>
#include <Wire.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <SparkFunBME280.h> //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include <Adafruit_Si7021.h>
#include <Adafruit_TCS34725.h>

#include <HTTPClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument localUser (2048);

#include "config.h"
#include "setup_sensors.cpp"
#include "mqtt.cpp"

/* The getUser() function is used as a flag if a user is found or not.
* Each device has a unique identification to connect to the internet (MAC)
* This code is used as a uuid and each measurement reported by this divice
* has this code as the author of a book
* --> Measurement by uuid (Device M.A.C)
*/
bool getUser(){

  bool UserFinded= false;

  if( wifi_status() ){
    HTTPClient http;

    http.begin( (urlGetUser+getMac()) );
    int httpCode = http.GET();  // Realizar petición
  
    if (httpCode > 0) {

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        
        
        String payload = http.getString();   // Obtener respuesta
        auto error = deserializeJson(localUser, payload);

        if( error == DeserializationError::Ok ){  
          if(debugging){ Serial.println(payload); };   // Mostrar respuesta por serial
          UserFinded = true;
        }
      }else{
        sisError (2);
      }
    }else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  
    http.end();
  }

  return UserFinded;  
}

void Sensors_on_off(int io){
  digitalWrite(SW1, io);
  delay(1000);
}
void getDataDig(uint8_t select){
  String json = "";
  DynamicJsonDocument doc(1024);
  doc["PlantId"] = "measurement";
  
  if (select & 0x1) {
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
  if (select & 0x2){
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
  if (select & 0x4) {
    if ( Setup_Si7021() ){
      float SItempC = sensor.readTemperature();
      float SIhumid = sensor.readHumidity();

      if (select & 0x2){        
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
  if (select & 0x8){
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
  send_mqtt("Huerta/Push/Digital" ,json);
  
  if (debugging){
    Serial.println("");
    Serial.println("json:");
    Serial.println(json);
  }
}
void getDataAnalog(JsonObjectConst User, String plant ){
  String json = "";
  DynamicJsonDocument doc(1024);
  JsonObject analog = doc.createNestedObject("Analog");
  analog["PlantId"] = plant;

  unsigned int SumaAnalog = 0;
  unsigned int samples = 10; 

  for (auto element : User) {

    for(int i = 0; i < samples; i++){
      SumaAnalog += analogRead( element.value() );
      delay(50);
    }

    JsonObject path = analog.createNestedObject(element.key());
    path["RawData"] = (SumaAnalog/samples);
    path["Pin"] = element.value();
    SumaAnalog = 0;
  }

  serializeJson(doc, json);
  //Serial.println("Enviando a mqtt");
  //Serial.println(json);
  send_mqtt("Huerta/Push/Analog" ,json);
}

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(SW1, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  Wire.begin();
  
  if ( wifi_status() ){
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);
  }

  if (debugging || debugging_mqtt){
    Serial.begin(115200);
    delay (100);
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.print("User: ");
    Serial.println(getMac());
    Serial.println("----------------------------------------------------------");
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
  digitalWrite(LED_BUILTIN, LOW);
  
  if( wifi_status() ){
    if ( getUser() ){
      /* Each plant has 4 measurements (2 Photocel, 1 HumCap and HumEC)
      *  and this is send to database for store it.
      */
      for (auto kvp : ( localUser["Plants"].as<JsonObject>() ) ) {
        getDataAnalog(kvp.value(), kvp.key().c_str() ); 
      }
      getDataDig(  localUser["Measurements"] );
    }else{
      ESP.deepSleep(sleepTime_reconnect);
    }

    client.loop();
    delay(2000);
    client.disconnect();
  }  

  delay(1000);
  ESP.deepSleep(sleepTime);
}