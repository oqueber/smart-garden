// ---------------------------------------------------------------------
// ---------------------- MQTT connections -----------------------------
// --------------------------------------------------------------------- 

const int mqttPort = 1883;
const char* mqtt_server = IP.c_str();
const char* willTopic = "PW";
const char* willMessage = "Offline";

void reconnect(void);
void send_mqtt(String msg_topic, String msg_payload);
void callback(char* topic, byte* payload, unsigned int length); 