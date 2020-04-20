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

#include "config.h"
#include "setup_sensors.cpp"
#include "mqtt.cpp"


// Return an Object with User settings 
void getUser(){
    if ( (WiFi.status() == WL_CONNECTED) ) { //Check the current connection status
 
      HTTPClient http;
      String urlUser = (String(mqtt_server)+"/Users/GetData/"+ getMac());
      Serial.println("Connected to :");
      Serial.println(urlUser);
      http.begin(urlUser); //Specify the URL
      int httpCode = http.GET();                                        //Make the request
 
      if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        Serial.println("getting..:");
        Serial.println(httpCode);
        Serial.println(payload);
      }
      else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
    }
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
    SumaAnalog = 0;
  }

  serializeJson(doc, json);
  send_mqtt("Huerta/Push/Analog" ,json);
}

void setup(){

  if (debugging){
	  Serial.begin(115200);
  }

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(SW1, OUTPUT);

  Wire.begin();
  
  if ( Setup_wifi() ){
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);
  }

  getUser();
  if (debugging){
    Serial.println();
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.print("User: ");
    Serial.println(getMac());
    Serial.println("\nWaiting for time");
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.println("----------------------------------------------------------");
  }
}

void loop(){
/*
  digitalWrite(LED_BUILTIN, LOW);
  
  if(WiFi.status() == WL_CONNECTED){
    getUser();
    
    for( int i = 0; i<=1 ;i++){
      selecPlant( i ); // Analog sensor data actived
      delay(1500);
      askSlave();  // Data length 
    }

    delay(100);
    getData();
    delay(100);
      
    sleepSensorAnalog();
    client.loop();
    delay(2000);
    client.disconnect();
  }  

  delay(1000);
  ESP.deepSleep(sleepTimeS * 1000000);
*/
}
