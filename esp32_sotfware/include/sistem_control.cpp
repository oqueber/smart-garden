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

    http.begin( (urlGetUser+getMac()) );
    int httpCode = http.GET();  // Realizar petición
  
    if (httpCode > 0) {

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        
        String payload = http.getString();   // Obtener respuesta
        auto error = deserializeJson(localUser, payload);
        writeFile(SD, "/userData.txt", payload.c_str() );
        if( error == DeserializationError::Ok ){  
          if(debugging){ Serial.println(payload); };   // Mostrar respuesta por serial
          UserFinded = true;
        }
      }else if(httpCode == 204){
        sisError (3); // 3: User don't exist
        Serial.println("User don't exist");
      }else{
        sisError (4); // 4: Bad comunication in http
      }
    }else {
      sisError (2, http.errorToString(httpCode).c_str() ); // 2: impossible to reach the server
    }
  
    http.end();
  }

  return UserFinded;  
}

