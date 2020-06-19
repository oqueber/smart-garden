#include <PubSubClient.h>

PubSubClient client(espClient);

// Try connecting to the Mqtt server
void reconnect() {
  uint8_t intentos = 0;
  const String clientId = "device/"+getMac();

  // Loop until we're reconnected
  while (!client.connected() && intentos < 10){
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

}
// Try to send the message to the Mqtt server
void send_mqtt(String msg_topic, String msg_payload){
  if (!client.connected()) {
    reconnect();
  }

  if (client.connected()) {
    DynamicJsonDocument doc(2048);
    auto error = deserializeJson(doc, msg_payload);

    if( error == DeserializationError::Ok ){
      String json = "";
      uint8_t intentos = 0;

      doc["device"] = getMac();
      serializeJson(doc, json);
      
      while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
        intentos++;
        Serial.print("try send number: ");
        Serial.println(intentos);
        delay(1000);
      }

      if(debugging_mqtt && (intentos < 10 )){
        Serial.println("-----------Se envio a mqtt---------------");
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
      
    }else{
      ESP.deepSleep(sleepTime_reconnect);
    }
  }else{
    ESP.deepSleep(sleepTime_reconnect);
  }

  
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

  receptionSystem( String(topic) , message);
}

