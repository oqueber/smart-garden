#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <PubSubClient.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <SparkFunBME280.h> //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include <Adafruit_Si7021.h>
#include <Adafruit_TCS34725.h>

#define LED_BUILTIN 2

const bool debugging = true;
const int sleepTimeS = 30*60;
const int sleepTimeS_reconnect= 60;

// Set these to run example.
const char* WIFI_SSID = "MIWIFI_2G_jPek";
const char* WIFI_PASSWORD = "TdvM6Urk";

const int mqttPort = 1883;
const char* mqtt_server = "35.224.59.94";

const char* willTopic = "PW";
const char* willMessage = "Offline";

//Global sensor objects
#define CCS811_ADDR 0x5A //Alternate I2C Address

bool availableSensor[8];
const int I2C_SLAVE_ADDR = 0x20;
const char ASK_FOR_LENGTH = 'L';
const char ASK_FOR_DATA = 'D';
const char REQ_DATA = 'R';


char activateSensor = '?';

CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;
Adafruit_Si7021 sensor = Adafruit_Si7021();
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

WiFiClient espClient;
PubSubClient client(espClient);

typedef enum{
    s_TCS34725,
    s_Si7021,
    s_BME280,
    s_CCS811,
    s_wifi
} Sensors;

String getMac(){
  byte mac[6];
  WiFi.macAddress(mac);
  return (String(mac[5])+':'+String(mac[4])+':'+String(mac[3])+':'+String(mac[2])+':'+String(mac[1])+':'+String(mac[0]));
}

void getUser(){
    if ( (WiFi.status() == WL_CONNECTED) ) { //Check the current connection status
 
      HTTPClient http;
      String urlUser = (String(mqtt_server)+"/Users/GetData/"+ getMac());

      http.begin(urlUser); //Specify the URL
      int httpCode = http.GET();                                        //Make the request
 
      if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
      else {
        Serial.println("Error on HTTP request");
      }
      http.end(); //Free the resources
    }
}

void reconnect() {
  uint8_t intentos = 0;

  // Loop until we're reconnected
  while (!client.connected() && intentos < 10){
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (!client.connect( clientId.c_str(),willTopic, 2, false, willMessage )) {
      if(debugging){
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
      // Wait 5 seconds before retrying
      intentos++;
      delay(5000);
    }
  }
}
void send_mqtt(String msg_topic, String msg_payload){
  if (!client.connected()) {
    reconnect();
  }

  if (client.connected()) {
    DynamicJsonDocument doc(2048);
    auto error = deserializeJson(doc, msg_payload);

    if( error == DeserializationError::Ok ){
      String json = "";
      doc["Device"] = getMac();
      serializeJson(doc, json);
      
      uint8_t intentos = 0;
      while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
        intentos++;
        delay(1000);
      }
    }else{
      ESP.deepSleep(sleepTimeS_reconnect * 1000000);
    }
  }else{
    ESP.deepSleep(sleepTimeS_reconnect * 1000000);
  }

  if (debugging){
    Serial.println("----------------------------------------------------------");
    Serial.print("Status mqtt: ");
    Serial.println( client.state() );
    Serial.print("connected wifi: ");
    Serial.println(WiFi.status() == WL_CONNECTED);
    Serial.print("connected mqtt: ");
    Serial.println(client.connected());
    Serial.print("Topic:");
    Serial.println(msg_topic);
    Serial.print("Topic:");
    Serial.println(msg_payload);
    Serial.println("----------------------------------------------------------");
  }
}

void sleepSensorAnalog(){
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write( (char)( (B01000000) + 15) );
  Wire.endTransmission();
}
void selecPlant(int selec){
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write( (char)( (B01000000) + selec) );
  Wire.endTransmission();
}
void actSensorAnalog(){
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(activateSensor);
  Wire.endTransmission();
}
void callback(char* topic, byte* payload, unsigned int length) {
  if (debugging){
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }
 /*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  */
}
bool Setup_CCS811 (){
  uint8_t reintentos = 0;
  while (reintentos <= 5 && !myCCS811.begin() ) {
    reintentos++;
    delay(500);
  }
  return (myCCS811.begin());
}
bool Setup_Si7021 (){
  uint8_t reintentos = 0;
  while (reintentos <= 5 && !sensor.begin()) {
    reintentos++;
    delay(500);
  }

  return (sensor.begin());
}
bool Setup_wifi (){
 // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint8_t reintentos = 0;
  while ((reintentos <= 60) && (WiFi.status() != WL_CONNECTED)) {
    delay(1000);
  }
  return(WiFi.status() == WL_CONNECTED);
}
bool Setup_TCS34725 (){

  uint8_t reintentos = 0;
  while (reintentos <= 5 && !tcs.begin()) {
    reintentos++;
    delay(500);
  }
  return (tcs.begin());
}
bool Setup_BME280 (){
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x76;

  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x76;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  //myBME280.begin();

  uint8_t reintentos = 0;
  while (reintentos <= 5 && !myBME280.beginI2C()) {
    reintentos++;
    delay(500);
  }

  return (myBME280.beginI2C());
}
void printSensorError(){
  uint8_t error = myCCS811.getErrorRegister();

  if ( error == 0xFF ) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}
void getSensor(){
  delay(100);
  actSensorAnalog();
  delay(100);
  
  availableSensor[s_TCS34725] = Setup_TCS34725();
  if (debugging){
    Serial.println("");
    Serial.print("Sensor TCS34725 :");
    Serial.println(availableSensor[s_TCS34725] == true? "found" : "Not found");
  }
  
  availableSensor[s_Si7021] = Setup_Si7021();
  if (debugging){
    Serial.println("");
    Serial.print("Sensor Si7021 :");
    Serial.println(availableSensor[s_Si7021] == true? "found" : "Not found");
  }
  
  availableSensor[s_CCS811] = Setup_CCS811();
  if (debugging){
    Serial.println("");
    Serial.print("Sensor CCS811 :");
    Serial.println(availableSensor[s_CCS811] == true? "found" : "Not found");
  }
  
  availableSensor[s_BME280] = Setup_BME280();
  if (debugging){
    Serial.println("");
    Serial.print("Sensor BME280 :");
    Serial.println(availableSensor[s_BME280] == true? "found" : "Not found");
  }

  availableSensor[s_wifi] = Setup_wifi();
  if (debugging){
    Serial.println("");
    Serial.print("Sensor wifi :");
    Serial.println(availableSensor[s_wifi] == true? "found" : "Not found");
  }
}
void getData(){
  String json = "";
  DynamicJsonDocument doc(1024);

  if (availableSensor[s_TCS34725]){
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
  if (availableSensor[s_Si7021]){
    float SItempC = sensor.readTemperature();
    float SIhumid = sensor.readHumidity();

    if (availableSensor[s_CCS811]){        
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
  if (availableSensor[s_BME280]){

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
  if (availableSensor[s_CCS811]){
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

  serializeJson(doc, json);
  send_mqtt("Huerta/Push/Digital" ,json);
  
  if (debugging){
    Serial.println("");
    Serial.println("json:");
    Serial.println(json);
  }
}

unsigned int askForLength(){
	Wire.beginTransmission(I2C_SLAVE_ADDR);
	Wire.write(ASK_FOR_LENGTH);
	Wire.endTransmission();

	Wire.requestFrom(I2C_SLAVE_ADDR, 3);
	String responseLenght = "";
  	while (Wire.available()){
      responseLenght += (char)Wire.read();
    } 
	return (responseLenght.toInt());
}

void askForData(unsigned int responseLenght){
  String response = "";
	Wire.beginTransmission(I2C_SLAVE_ADDR);
	Wire.write(ASK_FOR_DATA);
	Wire.endTransmission();

	for (int requestIndex = 0; requestIndex <= (responseLenght / 32); requestIndex++){
	 Wire.requestFrom(I2C_SLAVE_ADDR, requestIndex < (responseLenght / 32) ? 32 : responseLenght % 32);
   delay(500);
		while (Wire.available()){
			response += (char)Wire.read();
		}
   delay(100);
	}
  send_mqtt("Huerta/Push/Analog" ,response);
}
void askSlave(){
	unsigned int responseLenght = 0;
  responseLenght = askForLength();

  if(debugging){
    Serial.println("");
    Serial.print("lenght: " );
    Serial.println(responseLenght);
  }

	if (responseLenght != 0){
	  askForData(responseLenght);
  };

}

void setup(){

  if (debugging){
	  Serial.begin(115200);
  }
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Wire.begin();
  // Assigment the addres to slave  
  delay(100); 
  getSensor();
  
  if (availableSensor[s_wifi]){
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);
  }

  if (debugging){
    Serial.println();
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.print("User: ");
    Serial.println(getMac());
    getUser();
    Serial.println("\nWaiting for time");
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.println("----------------------------------------------------------");
  }
}

void loop(){

  if (availableSensor[s_TCS34725] &&
      availableSensor[s_Si7021]&&
      availableSensor[s_CCS811]&&
      availableSensor[s_BME280]&&
      availableSensor[s_wifi])
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  if(WiFi.status() == WL_CONNECTED){
    
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

}
