#include <WiFi.h>
#include <HTTPClient.h>

WiFiClient espClient;

/*
* if the wifi is disconnect, it wil try to connect.
* if the wifi already connected, it returns a true.
*/
bool wifi_status (){

  if ( WiFi.status() != WL_CONNECTED ) { //Check the current connection status
    // connect to wifi.
    //IPAddress local_IP(192, 168, 1, 200);
    //IPAddress gateway(192, 168, 1, 1);
    //IPAddress subnet(255, 255, 255, 0);
    //WiFi.config(local_IP, gateway, subnet);
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