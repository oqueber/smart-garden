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

//#define A10 12
//#define A11 13
//#define A12 0 // Can not used only with out way
//#define A12 15
//#define A13 2
//#define A12 4

//SPI
//#define cs 2
//#define mosi 23
//#define miso 19
//#define sclk 18

//I2C
//#define SCL 22
//#define SDA 21

//buttom Touch
#define touchPin 15
// Switchs
#define SW1 16
// Light pin
#define neoPin 17
// led pin
#define LED_GREEN 1

// Water pin
#define waterPin0 4
#define waterPin1 5
#define waterPin2 12
#define waterPin3 13

// ---------------------------------------------------------------------
// --------------------  SD in ESP32  --------------------------
// --------------------------------------------------------------------- 

const char* SD_path_user = "/userData.txt";
const char* SD_path_measure = "/measures.txt";


// ---------------------------------------------------------------------
// --------------------  I2C in ESP32  --------------------------
// --------------------------------------------------------------------- 

//Alternate I2C Address for CCS811
#define CCS811_ADDR 0x5A 

// ---------------------------------------------------------------------
// ------------------------ HTTP connected  -----------------------------
// --------------------------------------------------------------------- 

// API url to Users finding
const String IP = "34.105.251.62"; // google server
//const String IP = "192.168.2.132"; //Server local
const String urlGetUser = "http://"+ IP +":3000/Users/GetData/";

// ---------------------------------------------------------------------
// ------------------ watchDog (Deep Sleep) -----------------------------
// --------------------------------------------------------------------- 

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define S_TO_M_FACTOR  60  /* Conversion factor for  seconds to minuts */
#define TIME_TO_SLEEP  (30*S_TO_M_FACTOR* uS_TO_S_FACTOR)         /* Time ESP32 will go to sleep (in Min) */

// Sleep time while the micro does nothing
const unsigned int time_1S = 1000000;
const unsigned int sleepTime_reconnect= 60 * time_1S; // 1 Min
const unsigned int sleepTime = 30*60 * time_1S; // 30 Min
bool itsHardReboot = false;
// ---------------------------------------------------------------------
// ------------------ NeoPixel (Led Strip) -----------------------------
// --------------------------------------------------------------------- 

// Which pin on the Arduino is connected to the NeoPixels?
#define pin_pixel 17 
// How many NeoPixels are attached to the Arduino?
#define num_pixels 27 
#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels


// ---------------------------------------------------------------------
// ----------------------  NTP Time -----------------------------
// --------------------------------------------------------------------- 

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
// ---------------------------------------------------------------------
// ----------------------  Debugging flag  -----------------------------
// --------------------------------------------------------------------- 

// Debugging flag for printing in monitor serial
const bool debugging = false;
const bool debugging_mqtt = false;
const bool debugging_SD = false;