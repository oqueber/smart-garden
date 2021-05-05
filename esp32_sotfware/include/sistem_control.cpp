//#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>

//#ifdef __AVR__
// #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
//#endif
//#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
//  clock_prescale_set(clock_div_1);
//#endif


//Adafruit_NeoPixel pixels(MAX_N_LEDS, pin_pixel, NEO_GRB + NEO_KHZ800);
DynamicJsonDocument doc (200);


#define waterOpen   0
#define waterMiddle 90
#define waterClose  180
#define timeOpen    2000

#define humCap_seco   3166
#define humCap_mojado 1284

#define humEc_seco    0
#define humEc_mojado  2460

// Ajustara en algua durante 5 ciclos 
#define TICKS_WATER_DEFAULT 5

void swichs_on_off(int io){
  Serial.print("\nSwitch mode: ");
  Serial.println(io);
  digitalWrite(SW1, io);
  delay(5000);
}

bool getUserSD( bool update = false ){
  
  bool fl_SD = false;

  // Buscamos la SD
  if(!SD.begin())
  {
    Serial.println("Card Mount Failed");
    sisError(5);
  }
  else
  {
    // Se imprime todos los archivos de la SD
    listDir(SD, "/", 0);

    // Se comprueba si existe datos del usuario guardados en la SD.
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

        for( iPlant = 0; iPlant < MAX_PLANTS; iPlant++)
        {
          //plantStatus[iPlant].Id = "";
          plantStatus[iPlant].ticks_light = 0;
          //plantStatus[iPlant].ticks_water = 0;
        }

        iPlant = 0;
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



void taskWater( int pin_humCap, int pin_humEc,int limit, int pinout, int open, int close)
{  

  unsigned int analogValue = 0;
  bool riegoEc = false;
  bool riegoCap = false;
  
  int  muxPin = 0;

  Serial.printf("\n Activado el sistema de riego pin %d open:%d close:%d \n", pinout,open, close); 
  

  for( int ianalog = 0; ianalog < 2; ianalog++)
  {
    // Variables para hacer la media 
    unsigned int SumaAnalog = 0;
    unsigned int samples = 10; 

    // ianalog = 0 -> HumCap
    // ianalog = 1 -> HumEc

    muxPin = (ianalog == 0) ? pin_humCap : pin_humEc;

    for(int i = 0; i < samples; i++)
    {
      
      if (pin_humEc < 30)
      {
        // ADC2 control register restoring
        WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
        //VERY IMPORTANT: DO THIS TO NOT HAVE INVERTED VALUES!
        SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
        //We have to do the 2 previous instructions BEFORE EVERY analogRead() calling!
      }

      SumaAnalog += analogRead( muxPin );
      delay(50);
    }

    analogValue = ceil(SumaAnalog/samples);
    Serial.printf("\n %s: %d \n", (ianalog == 0) ? "pin_humCap" : "pin_humEc" ,analogValue); 
    
    if (ianalog == 0)
      riegoEc =  (  analogValue >= humEc_mojado/2 ) ? true : false;
    else
      riegoCap = ( analogValue <= humCap_seco/2) ? true : false;
  }
  
  Serial.printf(" humCap:%u humEc:%u \n",riegoCap ,riegoEc ); 

  //if ( riegoCap && riegoEc )
  {
    Serial.printf("\n water time \n"); 
    Servo servo;  // create servo object to control a servo
    
    servo.attach(pinout);
    servo.write(open);
    delay(10000); // [TO DO] -> segun el nivel permaneceras mas tiempo regando
    servo.write(close);
    delay(3000);

    servo.detach();
  }

};
void taskLight(int led_start,  int led_end, int r, int g, int b){ 

    Serial.println("Activado el sistema de iluminacion"); 
    
    if((led_start == 0) && (led_end == 0)&& (r == 0) && (g == 0) && (b == 0))
    {
      FastLED.clear();
      FastLED.show();
      return; 
    }


    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for(int i=led_start; i<=led_end; i++) { // For each pixel...

      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      //pixels.setPixelColor(i, pixels.Color(r, g, b));
      //pixels.show();   // Send the updated pixel colors to the hardware.
      //delay(DELAYVAL); // Pause before next pass through loop

      leds[i].r = r;
      leds[i].g = g;
      leds[i].b = b; 

      leds[53-i].r = r;
      leds[53-i].g = g;
      leds[53-i].b = b; 

      leds[54+i].r = r;
      leds[54+i].g = g;
      leds[54+i].b = b; 

      FastLED.show(); 
      delay(30); 
    }

};

void taskSystem( JsonObject plant , String plant_id){
 
  if(systemTime.tm_year < (2016 - 1900)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Calculamos la hora y minutos actuales en segundos
  unsigned int rightNow = (systemTime.tm_hour*60*60) + (systemTime.tm_min *60);
  // Obtenemos la hora y minutos en segundos del inicio de la iluminacion 
  int hourStart_hour= (plant["light"]["time_start"].as<String>()).substring (0 , 2).toInt() ;
      hourStart_hour=  (hourStart_hour == 0) ? 24*60*60 : hourStart_hour*60*60;
  int hourStart_min=  (plant["light"]["time_start"].as<String>()).substring (3 , 5).toInt() ;
      hourStart_min= hourStart_min*60;
  // Obtenemos la hora y minutos en segundos del fin de la iluminacion 
  int hourStop_hour= (plant["light"]["time_stop"].as<String>()).substring (0 , 2).toInt() ;
      hourStop_hour= (hourStop_hour == 0) ? 24*60*60 : hourStop_hour*60*60;
  int hourStop_min=  (plant["light"]["time_stop"].as<String>()).substring (3 , 5).toInt() ;
      hourStop_min= hourStop_min*60; 

  // Se calcula la diferencia de dias desde la ultima vez de riego
  // Last_time_water : "YYYY:MM:DDD"
  time_t timeinfo_2 = mktime(&systemTime);
  unsigned long plant_water_time = plant["water"]["last_water"];
  double diff = difftime(timeinfo_2,  plant_water_time ) ;

  // Proceso no inicializado
  if ( plantStatus[iPlant].ticks_water <= 0)
  {
    
    // Compobar si toca regar la planta
    if( (diff/(60*60*24))  >=   plant["water"]["frequency"].as<int>()  ) {
      
      // [TO DO] Almacenar
      Serial.print("\nRiego activo\n");
      //plantStatus[iPlant].Id = plant_id;
      plantStatus[iPlant].ticks_water = TICKS_WATER_DEFAULT;


      Serial.print("Before: ");
      Serial.println(plant["water"]["last_water"].as<String>());
      plant["water"]["last_water"] = timeinfo_2;

      Serial.print("After: ");
      Serial.println(plant["water"]["last_water"].as<String>());

      // Actualizamos los datos de riego de la planta
      getUserSD( true );
    }
    
  }
  
  if (plantStatus[iPlant].ticks_light <= 0)
  {
    // De un dia para otro dia
    if( (hourStart_hour + hourStart_min) > (hourStop_hour  + hourStop_min) )
    {
      // Encendemos si -> Inicio de iluminacion <= Tiempo actual < fin de la iluminacion
      if( (rightNow >= (hourStart_hour + hourStart_min)))
      {
        // Encendemos la iluminacion
        Serial.print("\nLuz activo\n");
        plantStatus[iPlant].ticks_light = ceil( (((24*60*60)- rightNow) + (hourStop_hour  + hourStop_min)) /(TIME_SLEEP_MINUTS*TIME_SLEEP_S_TO_M_FACTOR)  ) ;

        taskLight(
          localUser["plants"][plant_id]["light"]["led_start"].as<int>(),
          localUser["plants"][plant_id]["light"]["led_end"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_red"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_green"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_blue"].as<int>()
        );
      }
      else if ((rightNow < ((hourStop_hour + hourStop_min) - (TIME_SLEEP_MINUTS*TIME_SLEEP_S_TO_M_FACTOR)))) 
      {
        // Encendemos la iluminacion
        Serial.print("\nLuz activo\n");
        plantStatus[iPlant].ticks_light = ceil( ( (hourStop_hour  + hourStop_min) - rightNow) /(TIME_SLEEP_MINUTS*TIME_SLEEP_S_TO_M_FACTOR)  ) ;

        taskLight(
          localUser["plants"][plant_id]["light"]["led_start"].as<int>(),
          localUser["plants"][plant_id]["light"]["led_end"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_red"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_green"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_blue"].as<int>()
        );
      } 

      // Ticks se queda en cero porque no hay que encender nada

    }
    // En el mismo dia
    else if ((hourStart_hour + hourStart_min) < (hourStop_hour  + hourStop_min))
    {
      // Encendemos si -> Inicio de iluminacion <= Tiempo actual < fin de la iluminacion
      if( (rightNow >= (hourStart_hour + hourStart_min)) && 
          (rightNow <= (hourStop_hour  + hourStop_min)) )
        {

        // Encendemos la iluminacion
        Serial.print("\nLuz activo\n");
        //plantStatus[iPlant].Id = plant_id;
        plantStatus[iPlant].ticks_light = ceil( ((hourStop_hour  + hourStop_min) - (rightNow))/(TIME_SLEEP_MINUTS*TIME_SLEEP_S_TO_M_FACTOR)  ) ;

        taskLight(
          localUser["plants"][plant_id]["light"]["led_start"].as<int>(),
          localUser["plants"][plant_id]["light"]["led_end"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_red"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_green"].as<int>(),
          localUser["plants"][plant_id]["light"]["color_blue"].as<int>()
        );
      }
    }

  }

  if( debugging ){
    Serial.println("------------------  taskSystem   ---------------------------------");
    
    Serial.print("ID plant: "); Serial.println( plant_id );
    Serial.print("ID statusPlant: "); Serial.println( plantStatus[iPlant].Id );
    Serial.print("Time_hour_min_now: "); Serial.println( rightNow);
    Serial.print("iPlant: "); Serial.println( iPlant );
    Serial.print("ticks_water: "); Serial.println( plantStatus[iPlant].ticks_water );
    Serial.print("ticks_light: "); Serial.println( plantStatus[iPlant].ticks_light);    

    Serial.print("[light][time_start]: "); Serial.println( plant["light"]["time_start"].as<String>() );
    Serial.print("[light][time_start][seg]: "); Serial.println( (hourStart_hour + hourStart_min) );
   
    Serial.print("[light][time_stop]: "); Serial.println( plant["light"]["time_stop"].as<String>()  );
    Serial.print("[light][time_stop][seg]: "); Serial.println( (hourStop_hour + hourStop_min) );

    
    Serial.print("[water][frequency]: "); Serial.println( plant["water"]["frequency"].as<String>() );
    Serial.print("[water][last_water]: "); Serial.println( plant_water_time);
    Serial.print("time now: "); Serial.println( timeinfo_2 );
    Serial.print("diff: "); Serial.println( diff/(60*60*24) );
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


/* The getUser() function is used as a flag if a user is found or not.
* Each device has a unique identification to connect to the internet (MAC)
* This code is used as a uuid and each measurement reported by this divice
* has this code as the author of a book
* --> Measurement by uuid (Device M.A.C)
*/
bool getUser(){

  bool UserFinded= false;

  //getUserSD();
  
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

