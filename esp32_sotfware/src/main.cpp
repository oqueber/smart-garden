#include <Arduino.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#define ARDUINOJSON_USE_LONG_LONG 1 // we need to store long long
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <SD.cpp>
#include <FastLED.h>
#include "soc/sens_reg.h" // needed for manipulating ADC2 control register

#include "config.h"


//--------------------------------------------------------
//--------------  Definiciones LOCALES       -------------
//--------------------------------------------------------

// Numero de intentos maximos consecutivos para conectarse al servidor MQTT
#define MAX_N_CONNECT_MQTT_SERVER 5

// Valor de sensivilidad del boton capacitivo 
#define Threshold 40 

//--------------------------------------------------------
//--------------  VARIABLES EN MEMORIA RAM   -------------
//--------------------------------------------------------

// place in RTC slow memory so available after deepsleep
static RTC_NOINIT_ATTR int reg_b;

// Estado y tareas actual de cada planta 
static RTC_DATA_ATTR struct Plant_status plantStatus[MAX_PLANTS];

static RTC_DATA_ATTR CRGB leds[MAX_N_LEDS];

//--------------------------------------------------------
//--------------    VARIABLES GLOBALES       -------------
//--------------------------------------------------------

// Datos del usuario en formato JSON
DynamicJsonDocument localUser (2048);

// Hora del sistema.
struct tm systemTime;

// Indice de planta a tratar
byte iPlant;
byte max_plants_user;

// Buffer de mensajes MQTT por enviar
struct ST_MQTT_SEND buffer_mqtt_msg_send[MAX_BUFFER_PLANTS];

//--------------------------------------------------------
//--------------    FLAGS DE CONTROL         -------------
//--------------------------------------------------------

// Micro controlado desde la plataforma web
bool fl_manual = false;

//Estructura para rellenar con los datos de los sensores
byte iMsgMqtt;


#include <sistem_error.cpp>

#include <./wifi/wifi.h>
#include <./wifi/wifi.cpp>


#include <./mqtt/mqtt.h>

#include <./mqtt/mqtt.cpp>

#include <sistem_control.cpp>
#include "setup_sensors.cpp"

TaskHandle_t Task1;
TaskHandle_t Task2;

void taskCore2( void * pvParameters);
void taskCore1( void * pvParameters);
void callback_touch(){};

// Recopilacion de variables Digitales
void getDataDig(String select ){
  
  DynamicJsonDocument parte1(200);
  DynamicJsonDocument parte2(200);
  
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

      // Agregamos un nuevo mensaje al buffer de salida
      iMsgMqtt++;
      buffer_mqtt_msg_send[iMsgMqtt].send = (parte1.size() > 0) ? true : false;
      buffer_mqtt_msg_send[iMsgMqtt].topic = "Huerta/Push/Digital"; 
      serializeJson(parte1, buffer_mqtt_msg_send[iMsgMqtt].msg);
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

  // Agregamos un nuevo mensaje al buffer de salida
  iMsgMqtt++;
  buffer_mqtt_msg_send[iMsgMqtt].send = (parte2.size() > 0) ? true : false; 
  buffer_mqtt_msg_send[iMsgMqtt].topic = "Huerta/Push/Digital"; 
  serializeJson(parte2, buffer_mqtt_msg_send[iMsgMqtt].msg);
}

// Recopilacion de variables Analogicas
void getDataAnalog(JsonObjectConst User, String plant){
  
  // Documento JSON
  DynamicJsonDocument analog(1024);

  // ID de la planta
  analog["plantId"] = plant;

  // Variables para hacer la media 
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
    
    path["pin"] = element.value();
    SumaAnalog = 0;
  }

  // Agregamos un nuevo mensaje al buffer de salida
  iMsgMqtt++;
  buffer_mqtt_msg_send[iMsgMqtt].send = (analog.size() > 0) ? true : false; 
  buffer_mqtt_msg_send[iMsgMqtt].topic = "Huerta/Push/Analog"; 
  serializeJson(analog, buffer_mqtt_msg_send[iMsgMqtt].msg);
}

// Actualizacion de la hora del sistema con o sin WIFI
bool update_time( void ){
  
  if(!getLocalTime(&systemTime)){
    Serial.println("Failed to obtain time");
    return false;
  }

  Serial.println(&systemTime, "%A, %B %d %Y %H:%M:%S");
  return true;
}

// Inicializacion de variables
void init_var_system( void )
{
  // reiniciamos el indice del buffer de envio de mensaje
  iMsgMqtt = 0;

  // Reiniciamos el indice de planta a tratar
  iPlant = 0;
  max_plants_user = 0;

  // Estado inicial del sistema
  fl_manual = false;
}

// Rutina de cuidado de la planta. Encargada de apagar el riego o la iluminacion
void taskPlantCare( void)
{
  byte iPlantCare;

  for( iPlantCare = 0; iPlantCare < max_plants_user; iPlantCare++)
  {
    // Protecciones
    //if ( (plantStatus[iPlantCare].ticks_water == 0) && (plantStatus[iPlantCare].ticks_light == 0))
    //  continue;

    Serial.printf("\n Tareas de la planta [%u] %s \n", iPlantCare ,plantStatus[iPlantCare].Id.c_str());
    Serial.printf(" ticks_light [%u] \n", plantStatus[iPlantCare].ticks_light);
    Serial.printf(" ticks_water [%u] \n", plantStatus[iPlantCare].ticks_water);

    // Mantener la iluminacion encendida hasta que transcurra el tiempo establecido
    if (plantStatus[iPlantCare].ticks_light)
    {
      
      plantStatus[iPlantCare].ticks_light--;
      
      // Enviar al servidor MQTT que sigue encendido la luz
      // Agregamos un nuevo mensaje al buffer de salida
      iMsgMqtt++;
      buffer_mqtt_msg_send[iMsgMqtt].send = true;
      buffer_mqtt_msg_send[iMsgMqtt].topic = "Huerta/update/light"; 
      buffer_mqtt_msg_send[iMsgMqtt].msg = "{\"plant\":\""+plantStatus[iPlantCare].Id+"\",\"status\":1}";
    }     
    else
    {
      taskLight(
                localUser["plants"][plantStatus[iPlantCare].Id]["light"]["led_start"].as<int>(),
                localUser["plants"][plantStatus[iPlantCare].Id]["light"]["led_end"].as<int>(),
                0,
                0,
                0);

      // Enviar al servidor MQTT que sigue permanece apagado la iluminacion
      // Agregamos un nuevo mensaje al buffer de salida
      iMsgMqtt++;
      buffer_mqtt_msg_send[iMsgMqtt].send = true;
      buffer_mqtt_msg_send[iMsgMqtt].topic = "Huerta/update/light"; 
      buffer_mqtt_msg_send[iMsgMqtt].msg = "{\"plant\":\""+plantStatus[iPlantCare].Id+"\",\"status\":0}";
    }

    // Mantener el riego de agua a un nivel estable
    if(plantStatus[iPlantCare].ticks_water)
    {
      plantStatus[iPlantCare].ticks_water--;
      
      taskWater(
        localUser["plants"][plantStatus[iPlantCare].Id]["pinout"]["humCap"].as<int>(),
        localUser["plants"][plantStatus[iPlantCare].Id]["pinout"]["humEC"].as<int>(),
        localUser["plants"][plantStatus[iPlantCare].Id]["water"]["limit"].as<int>(),
        localUser["plants"][plantStatus[iPlantCare].Id]["water"]["pinout"].as<int>(),
        localUser["plants"][plantStatus[iPlantCare].Id]["water"]["open"].as<int>(),
        localUser["plants"][plantStatus[iPlantCare].Id]["water"]["close"].as<int>()
      );

      // Enviar al servidor MQTT que se ha regado la planta
      // Agregamos un nuevo mensaje al buffer de salida
      iMsgMqtt++;
      buffer_mqtt_msg_send[iMsgMqtt].send = true;
      buffer_mqtt_msg_send[iMsgMqtt].topic = "Huerta/update/water"; 
      buffer_mqtt_msg_send[iMsgMqtt].msg = "{\"plant\":\""+plantStatus[iPlantCare].Id+"}";
    } 

    
  }
}

// Configuracion del sistema
void setup(){

  // Numero de intentos de conexion al servidor mqtt
  byte nServerMqtt = 0;

  if (debugging || debugging_mqtt){
    Serial.begin(115200);
    delay(100);
  }

  FastLED.addLeds<NEOPIXEL, pin_pixel>(leds, MAX_N_LEDS);

  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_GREEN, OUTPUT);
  pinMode(SW1, OUTPUT);
  pinMode(waterPin0, OUTPUT);
  pinMode(waterPin1, OUTPUT);
  pinMode(waterPin1, OUTPUT);
  pinMode(waterPin2, OUTPUT);
  pinMode(waterPin3, OUTPUT);
  pinMode(neoPin, OUTPUT);
  
  // Inicializacion de variables
  init_var_system();
  
  // Causa del despetar/inicio del micro
  switch( esp_sleep_get_wakeup_cause() ){

    // Se desperto por timeout
    case ESP_SLEEP_WAKEUP_TIMER :
      //digitalWrite(LED_GREEN,HIGH);
      break;

    // Se desperto por el boton capacitivo
    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
      
      Serial.println("\n Wakeup caused by touchpad");

      // Control del micro manualmente
      fl_manual = true;

      // Indicador led que se inicio correctamente el inicio manual
      // Proteccion para que de tiempo de soltar el boton
      for(int i = 0; i<10; i++){
        digitalWrite(LED_GREEN,HIGH);
        delay(500);
        digitalWrite(LED_GREEN,LOW);
        delay(500);
      } 

      break;

    // Arranque del micro. Se a conectado la alimentacion
    default : 
      
      Serial.printf("\n Arranque del Micro \n");

      //Guardamos el registro de la lectura analogicas 2
      reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);

      // Definimos y rellenamos el espacio de memoria
      memset(plantStatus, 0,MAX_PLANTS*sizeof(Plant_status));
      
      for( iPlant = 0; iPlant < MAX_PLANTS; iPlant++)
      {
        plantStatus[iPlant].Id = "";
        plantStatus[iPlant].ticks_light = 0;
        plantStatus[iPlant].ticks_water = 0;
      }

      // Reiniciamos el indice de planta para la proxima iteracion
      iPlant = 0;

      //Limpieza de la tira para hacer la lectura analogicas.
      FastLED.clear();
      FastLED.show();

      break;
  }
    
  // Establecemos el GPIO15 como un boton capacitivo
  touchAttachInterrupt(touchPin, callback_touch, Threshold);

  // Habilitamos despertar el micro por interrupcion de un boton capacitivo
  esp_sleep_enable_touchpad_wakeup();

  // Habilitamos el uso del i2c
  Wire.begin();

  //Limpieza de la tira para hacer la lectura analogicas.
  //FastLED.clear();
  //FastLED.show();

  //Led indicador que esta en funcionamiento
  digitalWrite(LED_GREEN, HIGH);

  // Obtener los datos desde la tarjeta SD. 
  if( (getUserSD() == true) && (fl_manual == false) )
  {
    //Damos electricidad a los sensores analogicos
    swichs_on_off(HIGH);

    // Cada planta tiene como maximo 4 sensores i2c ( TCS34725, BME280, CCS811 and Si7021 )
    getDataDig(  localUser["Measurements"] );

    // Cada planta tiene 4 medidas analogicas (2 Photocel, 1 HumCap and HumEC)
    for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
    
      //Definir los datos analogicos y los datos de riego y de iluminacion para ser activados
      getDataAnalog(kvp.value(), kvp.key().c_str() ); 

      // llenado de valores
      plantStatus[iPlant].Id = kvp.key().c_str();

      // Proteccion
      if ( iPlant++ > MAX_PLANTS)
        break; // Proteccion del sistema 
    }
    // Guardamos cuantas plantas hay en el json 
    max_plants_user = iPlant;

    // Reiniciamos el indice de planta para la proxima iteracion
    iPlant = 0;
    
    // Tareas pendientes por realizar
    taskPlantCare();

    // Quitar alimentacion a los sensores analogicos
    swichs_on_off(LOW);
  }
  
  // Encendido del WIFI
  esp_wifi_start();
  delay(1000);
  
  // Actualizamos la hora por NTP 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.println(" Buscando conexion wifi ");

  if ( wifi_status() == true )
  {
    // Verificamos si existe una actualizacion de la informacion actual del usuario
    getUser();

    // Definimos el servidor y el puerto del MQTT
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);

    // Intentamos conectarnos al servidor mqtt
    do
    {
      reconnect();

      if (nServerMqtt)
        delay(5000);
      
    }while((!client.connected()) && ( nServerMqtt++ <= MAX_N_CONNECT_MQTT_SERVER ));
    
  }
  
  //Actualizamos la hora con o sin wifi.
  update_time();

  if (debugging || debugging_mqtt){
    Serial.println("----------------------------------------------------------");
    Serial.println("--------------------- SETUP END --------------------------");
    Serial.print("connected by: ");
    Serial.println(WiFi.localIP());
    Serial.printf("User mac: %s \n", getMac().c_str() );
    //Serial.printf("HardReboot: %s\n",fl_cold_reset ? "Yes" : "No") ;
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_SLEEP_MINUTS) +" Minutes");
    Serial.println("----------------------------------------------------------");
  }

  if( (nServerMqtt >= MAX_N_CONNECT_MQTT_SERVER) ||
      (wifi_status() == false)
    )
  {
    // Apagar el WIFI
    esp_wifi_stop();
    delay(1000);

    iPlant = 0;

    //Damos electricidad a los sensores analogicos
    swichs_on_off(HIGH);

    //por cada planta, realizamos la rutina de tareas (Riego y/o iluminacion)
    for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
    
      //Definir los datos analogicos y los datos de riego y de iluminacion para ser activados
      taskSystem(kvp.value(), kvp.key().c_str() ); 

      if ( iPlant++ > MAX_PLANTS)
        break; 
    }
    
    //Apagamos los sensores analogicos
    swichs_on_off(LOW);

    // Sin datos que publicar, a dormir....
    toSleep(TIME_TO_SLEEP);
  }
  // Hay conexion wifi y se esta conectado al servidor MQTT
  else if ( fl_manual == true){
    Serial.printf("\n MANUAL task\n");

    xTaskCreatePinnedToCore(taskCore2, "Task2", 10000, NULL, 1, &Task2,  0); 
    delay(500); 
  }
  else
  {
    Serial.printf("\n WIFI task\n");
    
    delay(100);
    xTaskCreatePinnedToCore(taskCore1, "Task1", 10000, NULL, 1, &Task1,  1); 
  }
}

void taskCore1( void * pvParameters){

  bool finishedCore1 = false;
  //Numero de intentos en enviar un mensaje mqtt
  byte n_send_msg_mqtt = 0; 
  // Reiniciamos el indice del buffer
  iMsgMqtt = 0;

  for(;;){

    if (!client.connected())  // Reconnect if connection is lost
      reconnect();
    
       
    // Intentamos enviar los mensajes
    if( buffer_mqtt_msg_send[iMsgMqtt].send )
    {
      Serial.printf("\n Enviando iMsg: %u send:%u intento:%u", iMsgMqtt , buffer_mqtt_msg_send[iMsgMqtt].send, n_send_msg_mqtt );
      Serial.printf("\n topic: %s \n msg:%s \n", 
        buffer_mqtt_msg_send[iMsgMqtt].topic.c_str(),
        buffer_mqtt_msg_send[iMsgMqtt].msg.c_str());

      if(send_mqtt(buffer_mqtt_msg_send[iMsgMqtt].topic ,buffer_mqtt_msg_send[iMsgMqtt].msg) == true)
      { 
        iMsgMqtt++;
        n_send_msg_mqtt = 0;
      }
      else
        n_send_msg_mqtt++;
    }

    //Logica para el siguiente mensaje
    if( iMsgMqtt >= (MAX_BUFFER_PLANTS-1 ))
      finishedCore1 = true;
    else if( (n_send_msg_mqtt >= N_SEND_MQTT) || 
             (buffer_mqtt_msg_send[iMsgMqtt].send == false)
    )
    {
      iMsgMqtt++;
      n_send_msg_mqtt = 0;
    }  

    client.loop();
    delay(100);

    if( finishedCore1 == true)
    {
      // Encendemos el wifi
      esp_wifi_stop();

      iPlant = 0;

      delay(1000);

      //por cada planta, realizamos la rutina de tareas (Riego y/o iluminacion)
      for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
      
        //Definir los datos analogicos y los datos de riego y de iluminacion para ser activados
        taskSystem(kvp.value(), kvp.key().c_str() ); 

         // Proteccion
        if ( iPlant++ > MAX_PLANTS)
          break; // Proteccion del sistema 
      }

      toSleep(TIME_TO_SLEEP);
    }
  }
}

// manual mood
void taskCore2( void * pvParameters){

  bool toogle = 1;

  for(;;){

    // Reiniciamos el indice del buffer pero no enviamos ningun mensaje
    iMsgMqtt = 0;

    digitalWrite(LED_GREEN,toogle);
    toogle = toogle ? 0:1;

    if( touchRead(touchPin) < Threshold){
      digitalWrite(LED_GREEN,HIGH);
      Serial.println("Task2 by touch finished");
      client.unsubscribe("action/user/on");
      delay(5000);
      client.disconnect();
      delay(5000);
      toSleep(TIME_TO_SLEEP);
    }

    Serial.println("Task2 by touch new loop");

    // Tareas pendientes por realizar
    taskPlantCare();

    if (!client.connected()){  // Reconnect if connection is lost
      reconnect();
    }
    client.loop();
    delay(1000);

  }
  
}


void loop(){}