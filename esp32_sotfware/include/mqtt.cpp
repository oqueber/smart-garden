/* Error Definitions:
* 1 --> wifi connection error 
* 2 --> server connection error
* 3 --> 
*
*/
void sisError(unsigned int numberError){
  
  if (debugging){
    Serial.print("Error: ");
    if(numberError == 1){ Serial.print("Wifi connection error");}
    if(numberError == 2){ Serial.print("Server connection error");}
    Serial.println();
  }
}

/*
* if the wifi is disconnect, it wil try to connect.
* if the wifi already connected, it returns a true.
*/
bool wifi_status (){

  if ( WiFi.status() != WL_CONNECTED ) { //Check the current connection status
    // connect to wifi.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint8_t reintentos = 0;
    while ((reintentos <= 10) && (WiFi.status() != WL_CONNECTED)) {
      delay(1500);
      reintentos++;
    }
  }

  if ( WiFi.status() != WL_CONNECTED ){ sisError(1); }
  return(WiFi.status() == WL_CONNECTED);
}
// return a string with the MAC Device AA:AA:AA:AA:AA:AA
String getMac(){
  byte mac[6];
  WiFi.macAddress(mac);
  return (String(mac[5])+':'+String(mac[4])+':'+String(mac[3])+':'+String(mac[2])+':'+String(mac[1])+':'+String(mac[0]));
}
// Try connecting to the Mqtt server
void reconnect() {
  uint8_t intentos = 0;
  const String clientId = "ESP8266Client-"+String(random(0xffff), HEX);

  // Loop until we're reconnected
  while (!client.connected() && intentos < 10){
    // Attempt to connect
      if( client.connect(clientId.c_str()) ){
        client.subscribe("esp32/MAC/system");
        
        if(debugging_mqtt){
          Serial.println("connected");
          Serial.println(clientId);
          Serial.print("State, rc=");
          Serial.print(client.state());
        }
      }else{
        // Wait 5 seconds before retrying
        intentos++;
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

      doc["Device"] = getMac();
      serializeJson(doc, json);
      
      if(debugging_mqtt){
        Serial.println("-----------Enviando a mqtt---------------");
        Serial.print("Status mqtt: ");
        Serial.println( client.state() );
        Serial.print("connected wifi: ");
        Serial.println(WiFi.status() == WL_CONNECTED);
        Serial.print("connected mqtt: ");
        Serial.println(client.connected());
        Serial.print("Topic:");
        Serial.println(msg_topic);
        Serial.print("Msg:");
        Serial.println(msg_payload);
        Serial.println("----------------------------------------------------------");
      }else{
        while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
          intentos++;
          delay(1000);
        }
      }
    }else{
      ESP.deepSleep(sleepTime_reconnect);
    }
  }else{
    ESP.deepSleep(sleepTime_reconnect);
  }

  
}


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

  if (String(topic) == "esp32/MAC/system") {
    
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
  }
}
