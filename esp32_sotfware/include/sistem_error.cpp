void toSleep(unsigned int time ){
  Serial.println("Going to sleep...");
  Serial.flush();
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Configure the timer to wake us up!
  esp_sleep_enable_timer_wakeup(time);
  delay(100);
  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

/* Error Definitions:
* 1 --> wifi connection error 
* 2 --> server connection error
* 3 --> 
*
*/
void sisError(unsigned int numberError, String text = ""){
  
  if (debugging){

    Serial.println("");
    Serial.println("-> Error: ");
    //digitalWrite(LED_RED,LOW);
    switch (numberError){
    case 0:
      Serial.println("Nothing to do, no wifi or localUser save ");
      toSleep(TIME_TO_SLEEP);
      break;
    case 1:
      Serial.println("Wifi connection error");
      break;
    case 2:
      Serial.printf("[HTTP] GET... failed, error: %s\n", text.c_str());
      break;
    case 3:
      Serial.println("User don't exist");
      break;
    case 4:
      Serial.println("Bad comunication in http");
      break;
    case 5:
        Serial.println("No SD card attached");
      break;
    case 6:
        Serial.println("server unreach");
      break;
    default:
      break;
    }
  }
}