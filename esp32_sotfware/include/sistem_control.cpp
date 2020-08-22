#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

bool userLocalFlag = false;

Adafruit_NeoPixel pixels(num_pixels, pin_pixel, NEO_GRB + NEO_KHZ800);
DynamicJsonDocument doc (200);
DynamicJsonDocument localUser (2048);
JsonArray userTasks = doc.to<JsonArray>();


void receptionSystem( String topic, String message){
  if ( topic == "device/get/task" ) {
    userTasks.add(message);
  } 
}


void taskWater( int pin_humCap, int pin_humEc,int limit, int pinout){  Serial.println("Activado el sistema de riego"); };
void taskLight( int led_start,  int led_end, int r, int g, int b){ Serial.println("Activado el sistema de iluminacion"); };
void taskSystem( JsonObjectConst plant ){
 
 struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
    
  //Observamos el entorno por cada planta
  // Last_time_water : "YYYY:MM:DDD"
  time_t timeinfo_2 = mktime(&timeinfo);
  unsigned long plant_water_time = plant["water"]["last_water"].as<unsigned long>();

  double diff = difftime (timeinfo_2, plant_water_time);

  if( (diff/(60*60*24))  >=  plant["water"]["frequency"].as<String>().toInt() ) {
    
    taskWater( plant["pinout"]["humCap"].as<int>(),
               plant["pinout"]["humEC"].as<int>(),
               plant["water"]["limit"].as<int>(),
               plant["water"]["pinout"].as<int>());
  
  }


  if(    (timeinfo.tm_hour <= (plant["light"]["time_start"].as<String>()).substring (0 , 2).toInt()) 
      && (timeinfo.tm_min  <= (plant["light"]["time_start"].as<String>()).substring (3 , 5).toInt()) 
      && (timeinfo.tm_hour >= (plant["light"]["time_stop"].as<String>()).substring (0 , 2).toInt()) 
      && (timeinfo.tm_min  >= (plant["light"]["time_stop"].as<String>()).substring (3 , 5).toInt()) 
  ){

    taskLight( plant["light"]["led_start"].as<int>(), 
               plant["light"]["led_end"].as<int>(),
               plant["light"]["color_red"].as<int>(),
               plant["light"]["color_green"].as<int>(),
               plant["light"]["color_blue"].as<int>());
  }
  
  if( debugging ){
    Serial.println("------------------  taskSystem   ---------------------------------");
    
    Serial.print("Time Hours: "); Serial.println( timeinfo.tm_hour );
    Serial.print("Time min: "); Serial.println( timeinfo.tm_min );
    Serial.print("ID plant: "); Serial.println( plant["info"]["date"].as<int>());

    Serial.print("[light][time_start]: "); Serial.println( plant["light"]["time_start"].as<String>() );
    Serial.print("[light][time_start][Hour]: "); Serial.println( (plant["light"]["time_start"].as<String>()).substring (0 , 2).toInt()  );
    Serial.print("[light][time_start][min]: "); Serial.println(  (plant["light"]["time_start"].as<String>()).substring (3 , 5).toInt() );
   
    Serial.print("[light][time_stop]: "); Serial.println( plant["light"]["time_stop"].as<String>()  );
    Serial.print("[light][time_stop][Hour]: "); Serial.println( (plant["light"]["time_stop"].as<String>()).substring (0 , 2).toInt()  );
    Serial.print("[light][time_stop][min]: "); Serial.println(  (plant["light"]["time_stop"].as<String>()).substring (3 , 5).toInt() );

    Serial.print("[water][last_water]: "); Serial.println( plant_water_time );
    Serial.print("time: "); Serial.println( timeinfo_2 );
    Serial.print("diff: "); Serial.println( diff/(60*60*60*24) );
    Serial.print("[water][frequency]: "); Serial.println( plant["water"]["frequency"].as<String>() );
    
    Serial.println("------------------  taskSystem end   ---------------------------------");
  }
    
  

}
void tasksControl(){
  time_t now;
  time(&now);

  Serial.println(" -> Init the recoletion of object ");
  for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
    
    Serial.println("Key:");
    Serial.println(kvp.key().c_str());
    Serial.println("Value:");
    Serial.println(kvp.value().as<String>());


  }
  Serial.println(" -> end the recoletion of object ");
  
}
void controlSystem( void ){
  
  Serial.println("Stored tasks");
  for(JsonVariant v : userTasks) {
    String message = v.as<String>();
    Serial.println(message);

    unsigned int first_slash  = message.indexOf('/');
    unsigned int second_slash = message.indexOf('/',first_slash+1);
    unsigned int third_slash  = message.indexOf('/',second_slash+1);
    unsigned int  end = message.length();

    String element  = message.substring (0 , first_slash);
    String io = message.substring (first_slash+1, second_slash); 
    String plant = message.substring (second_slash+1 , third_slash);     
    String limit = message.substring (third_slash+1 , end); 

    if (debugging_mqtt){
      Serial.print("Split message : ");
      Serial.print(element + " ");
      Serial.print(io +" ");
      Serial.print(plant + " ");
      Serial.print(limit + " ");
      Serial.println();
    }
    /*
    if(message == "led/on/plan"){
      digitalWrite(4, HIGH);
      
    }else if(message == "led/off/plan"){
      digitalWrite(4, LOW);
    }
    else if(message == "water/off/plan"){
      digitalWrite(5, LOW);
    }
    else if(message == "water/on/plan"){
      digitalWrite(5, HIGH);
    }
    */
  } 
}


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

    Serial.print("search user: "); Serial.println(urlGetUser+getMac() );
    if( http.begin(espClient, urlGetUser+getMac() )){
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();  // Realizar petición

      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        
          String payload = http.getString();   // Obtener respuesta
          auto error = deserializeJson(localUser, payload);
        
          if( error == DeserializationError::Ok ){  
          
            // If we can't fount the user in local. Go to sleep
            if(!SD.begin()){
              Serial.println("Card Mount Failed");
              sisError(5);
            }else{
              listDir(SD, "/", 0);
              if ( SD.exists(SD_path_user) ){
                if( payload != readFile(SD, SD_path_user)){
                  Serial.println("Different user both cloud and local");
                  deleteFile(SD, SD_path_user);
                  delay(100);
                  writeFile(SD, SD_path_user, payload.c_str() );
                }
              }else{
                writeFile(SD, SD_path_measure, payload.c_str() );
              }
            }
            SD.end();
            //if(debugging){ Serial.println(payload); };   // Mostrar respuesta por serial
            UserFinded = true;
          }
        }else if(httpCode == 204){
          sisError (3); // 3: User don't exist
          Serial.print("search user: "); Serial.println(getMac());
          Serial.println("User don't exist in the server");
        }else{
          sisError (4); // 4: Bad comunication in http
        }
      }else {
        sisError (2, http.errorToString(httpCode).c_str() ); // 2: impossible to reach the server
      }
  
      http.end();

    }else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
  return UserFinded;  
}

