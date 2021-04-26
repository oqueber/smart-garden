#define MQTT_MAX_PACKET_SIZE = 500
#include <PubSubClient.h>

extern void taskWater( int pin_humCap, int pin_humEc,int limit, int pinout, int open, int close);
extern void taskLight( int led_start,  int led_end, int r, int g, int b);

PubSubClient client(espClient);

void web_platform_task ( String string_task)
{
  DynamicJsonDocument json_task (200);

  auto error = deserializeJson(json_task, string_task);  

  if( error == DeserializationError::Ok )
  {
    String plantId = json_task["plantId"].as<String>();
    
    if( json_task["action"].as<String>() == "water")
    {
      Serial.printf("\nActivando agua\n");
      
      plantStatus[9].Id = plantId;
      plantStatus[9].ticks_water = 10;

      taskWater(
        localUser["plants"][plantId]["pinout"]["humCap"].as<int>(),
        localUser["plants"][plantId]["pinout"]["humEC"].as<int>(),
        localUser["plants"][plantId]["water"]["limit"].as<int>(),
        localUser["plants"][plantId]["water"]["pinout"].as<int>(),
        localUser["plants"][plantId]["water"]["open"].as<int>(),
        localUser["plants"][plantId]["water"]["close"].as<int>()
      );
    }
    else if ( json_task["action"].as<String>() == "light") 
    {
      Serial.printf("\nActivando luz\n");
      switch( json_task["mode"].as<int>() )
      {
        //Limpieza de la tira led
        case 0:
          taskLight(
            0,
            0,
            0,
            0,
            0 
          );
          break;
        // Encender con la configuracion establecida de la planta
        case 1:
          plantStatus[9].Id = plantId;
          plantStatus[9].ticks_light = 50;
          taskLight(
            localUser["plants"][plantId]["light"]["led_start"].as<int>(),
            localUser["plants"][plantId]["light"]["led_end"].as<int>(),
            localUser["plants"][plantId]["light"]["color_red"].as<int>(),
            localUser["plants"][plantId]["light"]["color_green"].as<int>(),
            localUser["plants"][plantId]["light"]["color_blue"].as<int>()
          );
          break;
        // Encender con la configuracion de la web
        case 2: 
          plantStatus[9].Id = plantId;
          plantStatus[9].ticks_light = 50;
          taskLight(
            json_task["ledStart"].as<int>(),
            json_task["ledEnd"].as<int>(),
            json_task["red"].as<int>(),
            json_task["green"].as<int>(),
            json_task["blue"].as<int>()
          );
          break;
        default:
          break;
      }
      
    }
    else
    {
      Serial.printf("Activando nada....\n");
      //Respondes que no se pudo realizar la tarea
    }
    
  }
  else
  {
    Serial.printf("Problemas de deserializacion....\n");
    //Respondes que no se pudo realizar la tarea
  }
  
}

// Conectarse al servidor Mqtt
void reconnect() {

  // Numero de intentos
  byte intentos = 0;

  // Maximo 10 intentos de conexion seguidos
  while (!client.connected()){
    
    // Nombre del dispositivos conectado
    const String clientId = "device/"+getMac();

    if( client.connect(clientId.c_str()) ){
        
      // Conexion manual agrega el dispositivo a una lista de dispositivos conectado
      if (fl_manual)
        client.subscribe("esp32/connect");
      
      // Trazas de debug
      if(debugging_mqtt){
        Serial.println("connected");
        Serial.println(clientId);
        Serial.print("State, rc=");
        Serial.print(client.state());
      }

      break;  //Salimos del while

    }
    
    // Timeout
    if( intentos++ < 10) 
    {
      // Esperamos 5 minutos y se vuelve a intentar hasta un maximo de 10 veces
      Serial.println("Mqtt try to connecte to mqtt server");
      delay(5000);
      continue; // Continuamos con los intentos
    }
    
    break; //Salimos del while
  }
  
  // Notificamos que no se logro conectarse al servidor mqtt
  if(!client.connected())
    sisError(6); 

}

// Try to send the message to the Mqtt server
bool send_mqtt(String msg_topic, String msg_payload){

  bool sendMsg = false;

  //Si no hay conexion, nada que enviar
  if (!client.connected())
    return sendMsg;

  DynamicJsonDocument doc(2048);
  byte intentos = 0;
  String json = "";

  // Empaqueta el mensaje con el dispositivo y el tiempo actual.
  auto error = deserializeJson(doc, msg_payload);

  if( error == DeserializationError::Ok )
  {
    doc["device"] = getMac();
    doc["timestamps"] = mktime(&systemTime);
    serializeJson(doc, json);
  }
 
  if(debugging_mqtt )
  {
    Serial.println("----------- mqtt packgate---------------");
    Serial.print("Status mqtt: ");
    Serial.println( client.state() );
    Serial.print("connected wifi: ");
    Serial.println(WiFi.status() == WL_CONNECTED);
    Serial.print("connected mqtt: ");
    Serial.println(client.connected());
    Serial.print("Topic:");
    Serial.println(msg_topic);
    Serial.print("Msg:");
    Serial.println(json);
    Serial.println("----------------------------------------------------------");
  } 

  while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
    intentos++;
    Serial.print("try send number: ");
    Serial.println(intentos);
    delay(2000);
  }

  // Envio el mensaje
  if( intentos <= 10)
    sendMsg = true;

  // the message can not sent. reconnect() so we store
  /*
  if ( intentos > 10  || !client.connected() ){
    String dataSave = msg_topic + "-" + json + "\n";
    String pathSave = "/db/"+doc["timestamps"].as<String>()+".txt";
    if(debugging_mqtt ){
      Serial.println("-----------Store message---------------");
      Serial.print("path:");
      Serial.println(pathSave);
      Serial.print("data:");
      Serial.println(dataSave);
      Serial.println("----------------------------------------------------------");
    }

  }
  */
  return sendMsg;
}

// Recepcion de paquetes MQTT 
void callback(char* topic, byte* payload, unsigned int length) {
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i]; 
  }

  // Tarea enviada desde la plataforma web 
  web_platform_task(message);

  // Depuracion de los mensajes
  if (debugging_mqtt){
    Serial.print("Message arrived on topic[");
    Serial.print(topic);
    Serial.print("] : ");
    Serial.print(message);
    Serial.println();
  }

}


void sendPedientingMessage(){
  if ( client.connected()) {

    String topic;
    String payload;
    String failMsg;
    unsigned int first_slash;
    String msg = "";

    File file = SD.open(SD_path_measure);
    if(!file){
        Serial.println("Failed to open file for reading");
    }

    Serial.println("---------------- forwarding the mqtt messages ----------------- ");
    while(file.available()){
        
        char dataIn = (char)file.read() ;
        
        if (dataIn != '\n'){
          msg += dataIn; 
        }else{

          first_slash  = msg.indexOf('-');
          topic   = msg.substring (0 , first_slash);
          payload = msg.substring (first_slash+1, msg.length()); 

          Serial.print("Message forwarding on topic");
          
          if( !send_mqtt(topic,payload,false) ){
            Serial.println("falll");
            failMsg += msg + "\n";
          }
          
          msg     = "";
          

        }

    }
    Serial.println("----------------------------------- ");
    file.close();
    deleteFile(SD,SD_path_user);
    Serial.println(failMsg);
    //writeFile(SD,SD_path_user, failMsg.c_str());

  }
}
