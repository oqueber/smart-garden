//Global sensor objects
#define LED_BUILTIN 2

// ADCs on ESP32 
#define gA0 36 //GPIO36
#define A1 39 //GPIO39
#define A2 34
#define A3 35
#define A4 32
#define A5 33
#define A6 25
#define A7 26
#define A8 27
#define A9 14
#define A10 12
#define A11 13

//Alternate I2C Address for CCS811
#define CCS811_ADDR 0x5A 

// API url to Users finding
const String urlGetUser = "http://35.224.59.94:3000/Users/GetData/";
// Switchs
#define SW1 23

// Wifi setting
const char* WIFI_SSID = "MIWIFI_2G_jPek";
const char* WIFI_PASSWORD = "TdvM6Urk";
// Mqtt Setting
const int mqttPort = 1883;
const char* mqtt_server = "35.224.59.94";
const char* willTopic = "PW";
const char* willMessage = "Offline";

// Debugging flag for printing in monitor serial
const bool debugging = false;
const bool debugging_mqtt = true;
// Sleep time while the micro does nothing
const unsigned int time_1S = 1000000;
const int sleepTime_reconnect= 60 * time_1S; // 1 Min
const int sleepTime = 30*60 * time_1S; // 30 Min