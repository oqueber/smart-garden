#include <PubSubClient.h>

PubSubClient client(espClient);

// Try connecting to the Mqtt server
void reconnect() {
  uint8_t intentos = 0;

  // Loop until we're reconnected
  while (!client.connected() && intentos < 10){
    const String clientId = "device/"+getMac();
    // Attempt to connect
      if( client.connect(clientId.c_str()) ){
        client.subscribe("device/get/task");
        
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
bool send_mqtt(String msg_topic, String msg_payload){

  bool sendMsg = false;  
  uint8_t intentos = 0;
  String json = "";

  time_t now;
  time(&now);

  DynamicJsonDocument doc(2048);
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

  //Core 0 is already doing it.
  //reconnect();

  if (client.connected()) {
    while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
      intentos++;
      Serial.print("try send number: ");
      Serial.println(intentos);
      delay(1000);
    }

    if( intentos <= 10){ sendMsg = true;}
  }

  // the message can not sent. so we store
  if ( intentos > 10  || !client.connected() ){
    String dataSave = msg_topic + "-" + json + "\n";
    if(debugging_mqtt ){
      Serial.println("-----------Store message---------------");
      Serial.print("data:");
      Serial.println(dataSave);
      Serial.println("----------------------------------------------------------");
    }

    //if ( SD.exists(SD_path_measure) ){
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
          
          if( !send_mqtt(topic,payload) ){
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
