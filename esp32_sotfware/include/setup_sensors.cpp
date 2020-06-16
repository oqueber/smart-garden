#include <SparkFunBME280.h> //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include <Adafruit_Si7021.h>
#include <Adafruit_TCS34725.h>

// Created the object CCS811
CCS811 myCCS811(CCS811_ADDR);
// Created the object BME280
BME280 myBME280;
// Created the object Si7021
Adafruit_Si7021 sensor = Adafruit_Si7021();
// Created the object TCS34725 
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

bool Setup_CCS811 (){
  uint8_t reintentos = 0;
  while (reintentos <= 5 && !myCCS811.begin() ) {
    reintentos++;
    delay(500);
  }
  return (myCCS811.begin());
}
bool Setup_Si7021 (){
  uint8_t reintentos = 0;
  while (reintentos <= 5 && !sensor.begin()) {
    reintentos++;
    delay(500);
  }

  return (sensor.begin());
}
bool Setup_TCS34725 (){

  uint8_t reintentos = 0;
  while (reintentos <= 5 && !tcs.begin()) {
    reintentos++;
    delay(500);
  }
  return (tcs.begin());
}
bool Setup_BME280 (){
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x76;

  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x76;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  //myBME280.begin();

  uint8_t reintentos = 0;
  while (reintentos <= 5 && !myBME280.beginI2C()) {
    reintentos++;
    delay(500);
  }

  return (myBME280.beginI2C());
}
void printSensorError(){
  uint8_t error = myCCS811.getErrorRegister();

  if ( error == 0xFF ) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}
