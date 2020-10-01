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


void updateUser();
void receptionSystem( String topic, String message){
  if ( topic == "device/get/task" ) {
    userTasks.add(message);
  } 
}


void taskWater( int pin_humCap, int pin_humEc,int limit, int pinout){  Serial.println("Activado el sistema de riego"); };
void taskLight( int led_start,  int led_end, int r, int g, int b){ 

    Serial.println("Activado el sistema de iluminacion"); 
    

    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for(int i=led_start; i<=led_end; i++) { // For each pixel...

      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(r, g, b));
      pixels.show();   // Send the updated pixel colors to the hardware.
      delay(DELAYVAL); // Pause before next pass through loop
    }

};
void taskSystem( JsonObject plant , String plant_id){
 
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Variables para calcular el dia de regiego
  int rightNow = (timeinfo.tm_hour *60*60) + (timeinfo.tm_min *60);
  int hourStart_hour= (plant["light"]["time_start"].as<String>()).substring (0 , 2).toInt() ;
      hourStart_hour= hourStart_hour*60*60;
  int hourStart_min=  (plant["light"]["time_start"].as<String>()).substring (3 , 5).toInt() ;
      hourStart_min= hourStart_min*60;
  int hourStop_hour= (plant["light"]["time_stop"].as<String>()).substring (0 , 2).toInt() ;
      hourStop_hour= hourStop_hour*60*60;
  int hourStop_min=  (plant["light"]["time_stop"].as<String>()).substring (3 , 5).toInt() ;
      hourStop_min= hourStop_min*60; 

    
  // Calculamos la diferencia de dias desde la ultima vez de riego
  // Last_time_water : "YYYY:MM:DDD"
  time_t timeinfo_2 = mktime(&timeinfo);
  unsigned long plant_water_time = plant["water"]["last_water"];
  double diff = difftime(timeinfo_2,  plant_water_time ) ;


  if( (diff/(60*60*24))  >=   plant["water"]["frequency"].as<int>()  ) {
    
    //Enviar una actualizacion de la fecha del agua agua y actualizar la variable en local
    //Hacer un sistema de backup, que utilice solo el contador reinicios del micro para activar el riego
    // counterRiego >= frecuencia*24  --> CounterRiego aumenta 1 cada 30min. asi establecemos el encendido del micro
    taskWater( plant["pinout"]["humCap"].as<int>(),
               plant["pinout"]["humEC"].as<int>(),
               plant["water"]["limit"].as<int>(),
               plant["water"]["pinout"].as<int>());
    
    Serial.print("Before: ");
    Serial.println(plant["water"]["last_water"].as<String>());
    plant["water"]["last_water"] = timeinfo_2;

    Serial.print("After: ");
    Serial.println(plant["water"]["last_water"].as<String>());
    //enviar mqtt con los cambios
    send_mqtt("Huerta/update/water" , (plant_id+"/"+String(timeinfo_2)), true);
    // actualizar el usuario local
    //updateUser();

  }
  
  if( (rightNow >= (hourStart_hour + hourStart_min)) &&  (rightNow <= (hourStop_hour + hourStop_min)) ){

    //Si es la primera vez, encendemos la luz
    if( plant["light"]["status"].as<bool>() == false || itsHardReboot ){

      if (!itsHardReboot){
        plant["light"]["status"] = true;
        plant["light"]["last_light"] = timeinfo_2;
        //enviar mqtt con los cambios
        send_mqtt("Huerta/update/light" ,(plant_id+"/1/"+String(timeinfo_2)) , true);
      }

      taskLight( plant["light"]["led_start"].as<int>(), 
                plant["light"]["led_end"].as<int>(),
                plant["light"]["color_red"].as<int>(),
                plant["light"]["color_green"].as<int>(),
                plant["light"]["color_blue"].as<int>());

    }

  }else{
    if ( plant["light"]["status"].as<bool>() == true  ){
      plant["light"]["status"] == false;
      send_mqtt("Huerta/update/light" , (plant_id+"/0/"+String(timeinfo_2)) , true);
    }

  }

  if( debugging ){
    Serial.println("------------------  taskSystem   ---------------------------------");
    
    Serial.print("Time now: "); Serial.println( rightNow);
    Serial.print("LightStatus: "); Serial.println( plant["light"]["status"].as<bool>() );
    Serial.print("ItsHardReboot: "); Serial.println( itsHardReboot);
    
    Serial.print("ID plant: "); Serial.println( plant_id );

    Serial.print("[light][time_start]: "); Serial.println( plant["light"]["time_start"].as<String>() );
    Serial.print("[light][time_start][seg]: "); Serial.println( (hourStart_hour + hourStart_min) );
   
    Serial.print("[light][time_stop]: "); Serial.println( plant["light"]["time_stop"].as<String>()  );
    Serial.print("[light][time_stop][seg]: "); Serial.println( (hourStop_hour + hourStop_min) );

    Serial.print("[water][last_water]: "); Serial.println( plant_water_time);
    Serial.print("time: "); Serial.println( timeinfo_2 );
    Serial.print("diff: "); Serial.println( diff/(60*60*24) );
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

void updateUser(){
  String msgUpdate = "";
  
  serializeJson(localUser, msgUpdate);      
  
  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    sisError(5);
  }else{
    listDir(SD, "/", 0);
    if ( SD.exists(SD_path_user) ){
      if( msgUpdate != readFile(SD, SD_path_user)){
        Serial.println("Different user both update and local");
        deleteFile(SD, SD_path_user);
        delay(100);
        writeFile(SD, SD_path_user, msgUpdate.c_str());
      }
    }else{
        writeFile(SD, SD_path_user, msgUpdate.c_str());
    }
  }
  SD.end();

}

bool userUpdateData (String newData){
  bool userUpdate = false;
  auto error1 = deserializeJson(localUser, newData);    

  if( error1 == DeserializationError::Ok ){  
    if(!SD.begin()){
      // If we can't fount the user in local. Go to sleep
      Serial.println("Card Mount Failed");
      sisError(5);
    }else{
      listDir(SD, "/", 0);
      if ( SD.exists(SD_path_user) ){
        String UserLocalData = readFile(SD, SD_path_user);
        String dataUpdated = "";

        //we compare the user's local data and user's cloud data
        if( newData != UserLocalData){
          Serial.println("Different user both cloud and local");
          DynamicJsonDocument dataJsonSD(2048);
          auto error2 = deserializeJson(dataJsonSD, UserLocalData);    

          Serial.println("Antes: ");
          Serial.println(newData);
          if( error2 == DeserializationError::Ok ){ 
            // if the same plant, update all, excepte this elements.
            for( auto element:  localUser["plants"].as<JsonObject>() ){
              // If we already have the plant in local, we keep the old control data.
              if( dataJsonSD["plants"].containsKey( element.key()) ){
                  String plantId = element.key().c_str();
                  //localUser in this cases is the new data from cloud
                  localUser["plants"][plantId]["water"]["last_water"] = dataJsonSD["plants"][plantId]["water"]["last_water"];
                  localUser["plants"][plantId]["light"]["last_light"] = dataJsonSD["plants"][plantId]["light"]["last_light"];
                  localUser["plants"][plantId]["light"]["status"] = false;
                  //localUser["plants"][plantId]["light"]["status"] = dataJsonSD["plants"][plantId]["light"]["status"];
                  // Update the cloud data
                  send_mqtt("Huerta/update/water" , (plantId+"/"+dataJsonSD["plants"][plantId]["light"]["last_light"].as<String>() ), true);
                  send_mqtt("Huerta/update/light" , (plantId+"/0"), true);
              }
            }
          }

          deleteFile(SD, SD_path_user);
          delay(100);
          serializeJson(localUser, dataUpdated); 
          userUpdate = true;   
          writeFile(SD, SD_path_user, dataUpdated.c_str() );

          //if(debugging){
          //  Serial.println("The data was updated:");    // Mostrar respuesta por serial
          //  Serial.println(dataUpdated);                // Mostrar respuesta por serial
          //}
        }else{
          writeFile(SD, SD_path_user, newData.c_str() );
        }
      }
    }
    SD.end();
  }
  return userUpdate;
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
          userUpdateData(payload);
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

