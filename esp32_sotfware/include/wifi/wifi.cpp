#include <WiFi.h>
#include <HTTPClient.h>

#define MAX_CNX_WIFI          10
#define MAX_TIMEOUT_WIFI      10
WiFiClient espClient;

/*
* if the wifi is disconnect, it wil try to connect.
* if the wifi already connected, it returns a true.
*/
bool wifi_status (){
  byte iConect    = MAX_CNX_WIFI;
  byte cnxTimeout = 0;

  while ( (iConect--) && (WiFi.status() != WL_CONNECTED) )
  {
    Serial.printf("\n Conectando al wifi");

    // Recargamos el tiempo maximo de timeout por conexion
    cnxTimeout = MAX_TIMEOUT_WIFI;

    if ( WiFi.status() != WL_CONNECTED ) { //Check the current connection status
      
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

      while ((cnxTimeout--) && (WiFi.status() != WL_CONNECTED)) {
        delay(1000);
        Serial.printf(".");
      }

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