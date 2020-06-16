// ---------------------------------------------------------------------
// -------------------- pinout on ESP32  --------------------------
// --------------------------------------------------------------------- 

#define A0 36 //GPIO36
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
//#define A12 0 // Can not used
#define A12 15
#define A13 2
#define A14 4

// Switchs
#define SW1 23
// Led on board
#define LED_BUILTIN 2
#define LED_GREEN 5
#define LED_BLUE 16
#define LED_YELLOW 17
#define LED_RED 18

// ---------------------------------------------------------------------
// --------------------  I2C in ESP32  --------------------------
// --------------------------------------------------------------------- 

//Alternate I2C Address for CCS811
#define CCS811_ADDR 0x5A 

// ---------------------------------------------------------------------
// ------------------------ HTTP connected  -----------------------------
// --------------------------------------------------------------------- 

// API url to Users finding
const String IP = "35.224.59.94"; // google server
//const String IP = "192.168.1.136"; //Server local
const String urlGetUser = "http://"+ IP +":3000/Users/GetData/";

// ---------------------------------------------------------------------
// ------------------ watchDog (Deep Sleep) -----------------------------
// --------------------------------------------------------------------- 

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define S_TO_M_FACTOR  60  /* Conversion factor for  seconds to minuts */
#define TIME_TO_SLEEP  (1*S_TO_M_FACTOR* uS_TO_S_FACTOR)         /* Time ESP32 will go to sleep (in Min) */

// Sleep time while the micro does nothing
const unsigned int time_1S = 1000000;
const unsigned int sleepTime_reconnect= 60 * time_1S; // 1 Min
const unsigned int sleepTime = 30*60 * time_1S; // 30 Min

// ---------------------------------------------------------------------
// ------------------ NeoPixel (Led Strip) -----------------------------
// --------------------------------------------------------------------- 

// Which pin on the Arduino is connected to the NeoPixels?
#define pin_pixel 6 
// How many NeoPixels are attached to the Arduino?
#define num_pixels 16 
#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels



// ---------------------------------------------------------------------
// ----------------------  Debugging flag  -----------------------------
// --------------------------------------------------------------------- 

// Debugging flag for printing in monitor serial
const bool debugging = true;
const bool debugging_mqtt = true;
