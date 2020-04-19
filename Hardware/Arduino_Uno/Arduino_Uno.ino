// **** INCLUDES *****
#include "LowPower.h"
#include "Wire.h"
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>



// Use pin 2 as wake up pin
const int wakeUpPin = 3;

int NumPlan;
bool availableSensor[4];
bool actData = false;
const byte I2C_SLAVE_ADDR = 0x20;
const bool debugging = true;

unsigned int plantSelec = enableAng1;

// Buffer for the JSON
String json = "";
typedef enum{
    PhotoCell1,
    PhotoCell2,
    HumCap,
    HumEC
} Sensors;



void wakeUp(){}

void Default_Sensor(){
  availableSensor[PhotoCell1] = false;
  availableSensor[PhotoCell2] = false;
  availableSensor[HumCap] = false;
  availableSensor[HumEC] = false;
}
void getData(){
  DynamicJsonDocument doc(1024);
  json = "";

  //implementar el switch para activar los sensores
  digitalWrite(plantSelec, HIGH); 
  doc["PlantSelec"] = NumPlan;

  delay(300);
  if (availableSensor[PhotoCell1]){
    
    int SumaAnalog = 0;
    for(int i = 0; i<5; i++){
      SumaAnalog += analogRead(Photocell1Pin);
      delay(50);
    }
    float photocell1Reading = (SumaAnalog/10.0);
    float Volt = (photocell1Reading/204.8) ;
    float Res  = ((5/Volt) -1)*10000;

    if(debugging){
    Serial.println("-----------------------------------------------------------------------");
    Serial.print("photocell1 Sensor:");
    Serial.print(photocell1Reading,2);Serial.print(",");
    Serial.print(Volt,2);Serial.print(",");
    Serial.print(Res,2);Serial.print(",");
    }

    JsonObject photocell = doc.createNestedObject("Photocell1");
    photocell["RawData"] = photocell1Reading;
    photocell["Res"] = Res;

  }
  if (availableSensor[PhotoCell2]){
    
    int SumaAnalog = 0;
    for(int i = 0; i<5; i++){
      SumaAnalog += analogRead(Photocell2Pin);
      delay(50);
    }
    float photocell2Reading = (SumaAnalog/10.0); 
    float Volt = (photocell2Reading/204.8) ;
    float Res  = ((5/Volt) -1)*10000;
    if(debugging){
    Serial.print("photocell2 Sensor:");
    Serial.print(photocell2Reading,2);Serial.print(",");
    Serial.print(Volt,2);Serial.print(",");
    Serial.print(Res,2);Serial.print(",");
    }
    JsonObject photocell = doc.createNestedObject("Photocell2");
    photocell["RawData"] = photocell2Reading;
    photocell["Res"] = Res;
  }

  if (availableSensor[HumEC]){

    int SumaAnalog = 0;
    for(int i = 0; i<5; i++){
      SumaAnalog += analogRead(HumECPin);
      delay(50);
    }

    float HumEC_read = (SumaAnalog/10.0);

     if(debugging){
    Serial.print("HumEC Sensor:");
    Serial.print(HumEC_read,2);Serial.println("");
     }
    JsonObject photocell = doc.createNestedObject("HumEC");
    photocell["RawData"] = HumEC_read;
  }

    if (availableSensor[HumCap]){
      int SumaAnalog = 0;
      for(int i = 0; i<10; i++){
        SumaAnalog += analogRead(HumCapPin);
        delay(50);
      }
    float HumCap_read =(SumaAnalog/10.0); 
    if(debugging){
    Serial.print("HumCap Sensor:");
    Serial.print(HumCap_read,2);Serial.println("");
    }
    JsonObject photocell = doc.createNestedObject("HumCap");
    photocell["RawData"] = HumCap_read;
  }

  digitalWrite(plantSelec, LOW); 

  serializeJson(doc, json);
  if(debugging){
    Serial.println("");
    Serial.println("json:");
    Serial.println(json);
  }
}
 
void setup()
{
  if(debugging){
    Serial.begin(9600);
    delay(100);
    Serial.println("Iniciando...");
  }
  // This will consumes few uA of current.
  pinMode(wakeUpPin, INPUT); 
  pinMode(enableAng1, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(enableAng2, OUTPUT); 
  pinMode(enableAng3, OUTPUT); 
  pinMode(enableI2C, OUTPUT); 
  pinMode(enableLux, OUTPUT); 
  
  Default_Sensor();
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}
 
const char ASK_FOR_ACT = 'A';
const char ASK_FOR_DATA = 'D';
const char ASK_FOR_LENGTH = 'L';
/*
-> [0] = Activar A0
-> [1] = Activar A1
-> [2] = Activar A2
-> [3] = Activar A3

Ejemplo con codigo ascii
        llega 0 -> no activa ninguno;
        llega 1 -> 0001  activa A0
        llega 2 -> 0010  activa A1
        llega 3 -> 0011  activa A1 y A0
        llega 4 -> 0100  activa A2
        llega 5 -> 0101  activa A2 y A0
        llega 6 -> 0110  activa A2 y A1
        llega 7 -> 0111  activa A2, A1 y A0
        llega 8 -> 1000  activa A3
        llega 9 -> 1001  activa A3 y A0
        llega : (10) -> 1010  activa A3 y A1
        llega ; (11)-> 1011  activa A3, A1 y A0
        llega < (12)-> 1100  activa A3 y A2
        llega = (13)-> 1101  activa A3, A2 y A0
        llega > (14)-> 1110  activa A3, A2 y A1
        llega ? (15)-> 1111  activa todos

*/
 
char request = ' ';
int requestIndex = 0;
 
void receiveEvent(int bytes)
{
   while (Wire.available()){
     request = (char)Wire.read();
   }

    if(debugging){
      Serial.println("");
      Serial.print("LLega esto por el I2C");
      Serial.println(request);
    }
   
   // Codigo llega en formato ascii y nos quedamos con los ultimos 4 bits
   if ( (int)request >= 48  && (int)request <= 63 ){
    availableSensor[PhotoCell1] = ((int)request&0xF)&0x1;
    availableSensor[PhotoCell2] = ((int)request&0xF)&0x2;
    availableSensor[HumCap]     = ((int)request&0xF)&0x4;
    availableSensor[HumEC]      = ((int)request&0xF)&0x8;
    actData = true;
    digitalWrite(enableI2C, HIGH); 
    if(debugging){
      Serial.println("Activando los sensores");
    }
    
   }

   if ( (int)request >= 65  && (int)request <= 79 ){
     NumPlan= ((int)request&0xF);
     switch ( NumPlan ){
      case 1: plantSelec = enableAng1; break;
      case 2: plantSelec = enableAng2; break;
      case 3: plantSelec = enableAng3; break;
      case 15:
            digitalWrite(enableI2C, LOW); 
            // Allow wake up pin to trigger interrupt on low.
            attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);
            // Enter power down state with ADC and BOD module disabled.
            // Wake up when wake up pin is low.
            LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
            // Disable external pin interrupt on wake up pin.
            detachInterrupt(0); 
      break;
     
     default:
      plantSelec = enableAng1;
      break;
     }
   }
   request = ' ';
}
 
void requestEvent(){

  if(request == ASK_FOR_ACT){
    byte activos = availableSensor[PhotoCell1] * 0x1 +
                  availableSensor[PhotoCell2] * 0x2 +
                  availableSensor[HumCap] * 0x4 +
                  availableSensor[HumEC] * 0x8;

    Wire.write(activos);
  }
  if(request == ASK_FOR_LENGTH)
   {
      String valor = String(json.length());
      Wire.write(valor.c_str());
      requestIndex = 0;
   }
   if(request == ASK_FOR_DATA)
   {
      int json_length = json.length(); 
      
      if(requestIndex < ( json_length / 32)) 
      {
         Wire.write( json.substring(requestIndex * 32, (requestIndex +1)* 32).c_str() , 32);
         requestIndex ++;
      }
      else
      {
         Wire.write(json.c_str() + requestIndex * 32, (json_length % 32));
         requestIndex = 0;
      }
   }
 
}
void loop() {
  if(actData){actData = false;     getData();}
}
