// ---------------------------------------------------------------------
// ---------------------- MQTT connections -----------------------------
// --------------------------------------------------------------------- 

const int mqttPort = 1883;
const char* mqtt_server = "192.168.1.136";
const char* willTopic = "PW";
const char* willMessage = "Offline";

void reconnect(void);
void send_mqtt(String msg_topic, String msg_payload);
void callback(char* topic, byte* payload, unsigned int length); 