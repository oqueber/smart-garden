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

    Serial.print("-> Error: ");
    switch (numberError)
    {
    case 1:
      Serial.println("Wifi connection error");
      digitalWrite(LED_RED,HIGH);
      delay(5000);
      toSleep(TIME_TO_SLEEP);
      break;
    case 2:
      Serial.printf("[HTTP] GET... failed, error: %s\n", text.c_str());
      break;
    case 3:
      Serial.print("User don't exist");
      break;
    case 4:
      Serial.print("Bad comunication in http");
      break;
    case 5:
      break;
    default:
      break;
    }
    Serial.println();
  }
}