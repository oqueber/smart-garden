#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif


Adafruit_NeoPixel pixels(num_pixels, pin_pixel, NEO_GRB + NEO_KHZ800);
DynamicJsonDocument doc (200);
JsonArray userTasks = doc.to<JsonArray>();


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
  }
  
  if( (rightNow >= (hourStart_hour + hourStart_min)) &&  (rightNow <= (hourStop_hour + hourStop_min)) ){

    //Si es la primera vez, encendemos la luz
    if( plant["light"]["status"].as<bool>() == false || itsHardReboot ){

      
      plant["light"]["status"] = true;
      plant["light"]["last_light"] = timeinfo_2;
      //enviar mqtt con los cambios
      send_mqtt("Huerta/update/light" ,(plant_id+"/1/"+String(timeinfo_2)) , true);
      

      taskLight( plant["light"]["led_start"].as<int>(), 
                plant["light"]["led_end"].as<int>(),
                plant["light"]["color_red"].as<int>(),
                plant["light"]["color_green"].as<int>(),
                plant["light"]["color_blue"].as<int>());

    }

  }else{
    if ( plant["light"]["status"].as<bool>() == true  ){
      plant["light"]["status"] == false;
      send_mqtt("Huerta/update/light" , (plant_id+"/0") , true);
    }
    taskLight(  plant["light"]["led_start"].as<int>(), 
                plant["light"]["led_end"].as<int>(),
                0,
                0,
                0);
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

bool getUserSD( bool update = false ){
  bool fl_SD = false;

  // If we can't fount the user in local. Go to sleep
  if(!SD.begin())
  {
    Serial.println("Card Mount Failed");
    sisError(5);
  }
  else
  {
    listDir(SD, "/", 0);
    // Check to see if the file exists:
    if (SD.exists(SD_path_user)) 
    {
      
      if( debugging_SD )
      {
        Serial.println("Datos eliminados desde SD");
        deleteFile(SD, SD_path_user);
        delay(100);
        
      }
      else if(update == true)
      {
        String msg = "";
        deleteFile(SD, SD_path_user);
        delay(100);
        serializeJson(localUser, msg); 
        writeFile(SD, SD_path_user, msg.c_str() );
      }
      else
      {
        String stringUser = readFile(SD, SD_path_user);

        auto error = deserializeJson(localUser, stringUser );
        if( error == DeserializationError::Ok )
        {  
          fl_SD = true;
          Serial.println("Datos Actualizados desde SD");
        }
      }

    }
    else if( update )
    {
        String msg = "";
        serializeJson(localUser, msg); 
        writeFile(SD, SD_path_user, msg.c_str() );
    }
    else 
    {
      Serial.println("userData.txt doesn't exist.");
    }

  }  
  SD.end();
  return fl_SD;
}

bool userUpdateData (String newData){
  String userLocalData;
  String userDataUpdated;
  bool userUpdate = false;
  DynamicJsonDocument newDataUser(2048);
  auto error_newUser = deserializeJson(newDataUser, newData);  

  if( error_newUser == DeserializationError::Ok )
  {
    // Checkout if data change o the SD is Empty
    if ( newDataUser["update"].as<bool>() || ( localUser.size() == 0 ))
    {
      Serial.println("Hay cambios del usuario en el sevidor o fallo la SD");
      if(!SD.begin())
      {
        Serial.println("Card Mount Failed");
        sisError(5);

        // Update only the userLocal in ROM
        error_newUser = deserializeJson(localUser, newData);
        if( error_newUser == DeserializationError::Ok )
        {
          userUpdate = true;
        }

      }
      else
      {
    
        //listDir(SD, "/", 0);
        if ( SD.exists(SD_path_user) )
        {
          Serial.println("Actualizando los datos nuevos con la ultima imagen");
          
          newDataUser["update"]= false;
          
          // if the same plant, update all, excepte this elements.
          for( auto element:  localUser["plants"].as<JsonObject>() )
          {
            // If we already have the plant in local, we keep the old control data.
            if( newDataUser["plants"].containsKey( element.key()) )
            {
              String plantId = element.key().c_str();
              //We save the last user image 
              newDataUser["plants"][plantId]["water"]["last_water"] = localUser["plants"][plantId]["water"]["last_water"];
              newDataUser["plants"][plantId]["light"]["last_light"] = localUser["plants"][plantId]["light"]["last_light"];
              newDataUser["plants"][plantId]["light"]["status"] = localUser["plants"][plantId]["light"]["status"];
              newDataUser["plants"][plantId]["water"]["status"] = localUser["plants"][plantId]["water"]["status"];
              // Update the cloud data
              send_mqtt("Huerta/update/water" , (plantId+"/"+newDataUser["plants"][plantId]["water"]["last_water"].as<String>() ), true);
              send_mqtt("Huerta/update/light" , (plantId+"/"+newDataUser["plants"][plantId]["light"]["status"].as<String>()+"/"+newDataUser["plants"][plantId]["light"]["last_light"].as<String>()), true);
            }
          }

          deleteFile(SD, SD_path_user);
          delay(100);
          serializeJson(newDataUser, userDataUpdated); 
          writeFile(SD, SD_path_user, userDataUpdated.c_str() );

          if ( deserializeJson(localUser, userDataUpdated) == DeserializationError::Ok  )
          {
            userUpdate = true;   
          }
        

        }
        else
        {
          Serial.println(" The SD its empty");
          writeFile(SD, SD_path_user, newData.c_str() );

          auto error1 = deserializeJson(localUser, newData);    
          if( error1 == DeserializationError::Ok )
          {
            userUpdate = true;
          } 
        }
      }

    }
    else
    {
      Serial.println("No hay datos que actualizar");
      userUpdate = true;
    }

  }
  
  SD.end();
  return userUpdate;
}

void updateLastData (){
  bool find_plant[MAX_PLANTS];
  
  //implementar que si el valor de now es malo, coger el del estadoa anterior
  // Inicializamos los valores por defecto para poder encontrar cuales plantas siguen con el usuario
  for (int i = 0 ; i< MAX_PLANTS; i++)
  {
    find_plant[i] = false;
  }

  for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
    
    String Id_plant = kvp.key().c_str();

    for (int i = 0; i < MAX_PLANTS; i++)
    {
      if( plantStatus[i].Id == strtoll(kvp.key().c_str(), (char **) NULL, 10))
      {
        find_plant[i] = true;

        printf("\n ---before---- \n");
        printf("  Id: %lld in indice:%d \n", plantStatus[i].Id, i);
        printf("  water: %d \n", localUser["plants"][Id_plant]["water"]["status"].as<bool>());
        printf(" last_time_water: %ld \n", localUser["plants"][Id_plant]["water"]["last_water"].as<unsigned long>() );
        printf(" light: %d \n", localUser["plants"][Id_plant]["light"]["status"].as<bool>());
        printf(" last_time_light: %ld \n",  localUser["plants"][Id_plant]["light"]["last_light"].as<unsigned long>() );

        localUser["plants"][Id_plant]["water"]["status"] = plantStatus[i].water;
        localUser["plants"][Id_plant]["water"]["last_water"] = plantStatus[i].last_time_water;
        localUser["plants"][Id_plant]["light"]["status"] = plantStatus[i].light;
        localUser["plants"][Id_plant]["light"]["last_light"] = plantStatus[i].last_time_light;

        printf("\n ---after---- \n");
        printf("  Id: %lld in indice:%d \n", plantStatus[i].Id, i);
        printf("  water: %d \n", localUser["plants"][Id_plant]["water"]["status"].as<bool>());
        printf(" last_time_water: %ld \n", localUser["plants"][Id_plant]["water"]["last_water"].as<unsigned long>() );
        printf(" light: %d \n", localUser["plants"][Id_plant]["light"]["status"].as<bool>());
        printf(" last_time_light: %ld \n",  localUser["plants"][Id_plant]["light"]["last_light"].as<unsigned long>() );
        Serial.println("");
      }
    }
  }

  for (int i = 0; i < MAX_PLANTS; i++)
  {
    if( find_plant[i] == false)
    {
      Serial.printf("No se a encontrdo a la planta id:%lld en el indice %d",plantStatus[i].Id, i );
      plantStatus[i].Id = 0;
      plantStatus[i].water = 0;
      plantStatus[i].last_time_water= 0;
      plantStatus[i].light = 0;
      plantStatus[i].last_time_light= 0;
    }
  }
}
void saveUser()
{

  Serial.println("\n Guardando datos en local: ");
  //serializeJson(localUser, localUser_string );
  //Serial.println(localUser_string);
  int indice = 0;

  timeSystem = now;
  // Guardamos todas las plantas actuales para reestablecer su valore al reinicio.
  for (auto kvp : ( localUser["plants"].as<JsonObject>() ) ) {
    
    String Id_plant = kvp.key().c_str();
    if ( indice < MAX_PLANTS)
    {
      plantStatus[indice].Id = strtoll(kvp.key().c_str(), (char **) NULL, 10); 
      plantStatus[indice].water = localUser["plants"][Id_plant]["water"]["status"].as<bool>();
      plantStatus[indice].last_time_water= localUser["plants"][Id_plant]["water"]["last_water"].as<unsigned long>();
      plantStatus[indice].light = localUser["plants"][Id_plant]["light"]["status"].as<bool>();
      plantStatus[indice].last_time_light= localUser["plants"][Id_plant]["light"]["last_light"].as<unsigned long>();

      printf("\n ------- \n");
      printf("  Id: %lld (%s) in indice:%d \n", plantStatus[indice].Id, kvp.key().c_str(), indice);
      printf(" water: %d (%s) \n",              plantStatus[indice].water, localUser["plants"][Id_plant]["water"]["status"].as<String>());
      printf(" last_time_light: %lu (%s) \n",   plantStatus[indice].last_time_water, localUser["plants"][Id_plant]["water"]["last_water"].as<String>());
      printf(" light: %d (%s) \n",              plantStatus[indice].light, localUser["plants"][Id_plant]["light"]["status"].as<String>());
      printf(" last_time_light: %lu (%s)\n",     plantStatus[indice].last_time_light, localUser["plants"][Id_plant]["light"]["last_light"].as<String>());
      
      indice++;
    } 
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

  getUserSD();
  
  //And then check if we have update
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
          UserFinded =  userUpdateData(payload);
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

  Serial.printf( "Tamaño del localUser %d \n", localUser.size() );
  if ( localUser.size() != 0 )
  {
    UserFinded = true;
    Serial.println("localUSER: ");
    Serial.println( localUser.as<String>() );
  }

  return UserFinded;  
}

