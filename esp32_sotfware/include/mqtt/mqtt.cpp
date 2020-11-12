#define MQTT_MAX_PACKET_SIZE = 500
#include <PubSubClient.h>

PubSubClient client(espClient);
time_t now;
  
// Try connecting to the Mqtt server
void reconnect() {
  uint8_t intentos = 0;

  // Loop until we're reconnected
  while (!client.connected() && intentos < 10){
    const String clientId = "device/"+getMac();
    // Attempt to connect
      if( client.connect(clientId.c_str()) ){
        
        if (fl_manual)
        {
          client.subscribe("action/user/on");
        }
        
        if(debugging_mqtt){
          Serial.println("connected");
          Serial.println(clientId);
          Serial.print("State, rc=");
          Serial.print(client.state());
        }
      }else{
        // Wait 5 seconds before retrying
        intentos++;
        Serial.println("Mqtt try to connecte to mqtt server");
        delay(5000);
      }
  }
  
  if(!client.connected()){ sisError(6); }
}
// Try to send the message to the Mqtt server
bool send_mqtt(String msg_topic, String msg_payload, bool update){
  DynamicJsonDocument doc(2048);
  bool sendMsg = false;  
  uint8_t intentos = 0;
  String json = "";


  if(update){
    json = msg_payload;
  }else{
    auto error = deserializeJson(doc, msg_payload);

    if( error == DeserializationError::Ok ){
      doc["device"] = getMac();
      doc["timestamps"] = now;
      serializeJson(doc, json);

      if(debugging_mqtt ){
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
    }
  }

  //Core 0 is already doing it.
  reconnect();
  delay(100);
  if (client.connected()) {
    while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
      intentos++;
      Serial.print("try send number: ");
      Serial.println(intentos);
      delay(2000);
    }

    if( intentos <= 10){ sendMsg = true;}
  }

  // the message can not sent. reconnect() so we store
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

/*
    if(!SD.begin()){
      Serial.println("Card Mount Failed");
      sisError(5);
    }else{
      listDir(SD, "/", 1);
      writeFile(SD,pathSave.c_str(), dataSave.c_str() );
    }
    SD.end();
*/
    //if ( SD.exists("/db/"+SD_path_measure) ){
    //  appendFile(SD, SD_path_measure, dataSave.c_str() );
    //  //Serial.println("Store message old: ");
    //  //Serial.println(readFile(SD,SD_path_measure));
    //}else{
    //  writeFile(SD, SD_path_measure, dataSave.c_str() );
    //}

  }
  return sendMsg;
}
// Recived the packages from the server. 
void callback(char* topic, byte* payload, unsigned int length) {
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i]; 
  }

  if (debugging_mqtt){
    Serial.print("Message arrived on topic[");
    Serial.print(topic);
    Serial.print("] : ");
    Serial.print(message);
    Serial.println();
  }

  //receptionSystem( String(topic) , message);
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
