bool Setup_wifi (){
 // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint8_t reintentos = 0;
  while ((reintentos <= 60) && (WiFi.status() != WL_CONNECTED)) {
    delay(1000);
  }
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

  // Loop until we're reconnected
  while (!client.connected() && intentos < 10){
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (!client.connect( clientId.c_str(),willTopic, 2, false, willMessage )) {
      if(debugging_mqtt){
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
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
      doc["Device"] = getMac();
      serializeJson(doc, json);
      
      uint8_t intentos = 0;
      while((client.publish(msg_topic.c_str(), json.c_str(),false) != 1)&&(intentos <=10)){
        intentos++;
        delay(1000);
      }
    }else{
      ESP.deepSleep(sleepTime_reconnect * 1000000);
    }
  }else{
    ESP.deepSleep(sleepTime_reconnect * 1000000);
  }

  if (debugging_mqtt){
    Serial.println("----------------------------------------------------------");
    Serial.print("Status mqtt: ");
    Serial.println( client.state() );
    Serial.print("connected wifi: ");
    Serial.println(WiFi.status() == WL_CONNECTED);
    Serial.print("connected mqtt: ");
    Serial.println(client.connected());
    Serial.print("Topic:");
    Serial.println(msg_topic);
    Serial.print("Topic:");
    Serial.println(msg_payload);
    Serial.println("----------------------------------------------------------");
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  if (debugging_mqtt){
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }
 /*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  */
}
