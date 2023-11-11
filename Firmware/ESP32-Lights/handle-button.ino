void singleClick() {
  Serial.println("SHORT PRESS");
  clicked = true;
  mode++;
  if (mode > 14) {
    mode = 0;
  }
}

void doubleClick() {
  Serial.println("DOUBLE CLICK");
  Serial.println("Save mode");
  writeLEDConfig();
  blink(ON, ON, OFF, ON, ON, OFF);
}

void longClick() {
  allOff();
  // indicate wifi
  ledcWrite(ledChannelR1, OFF);
  ledcWrite(ledChannelG1, OFF);
  ledcWrite(ledChannelB1, ON);
  ledcWrite(ledChannelR2, OFF);
  ledcWrite(ledChannelG2, OFF);
  ledcWrite(ledChannelB2, OFF);
}

void longPress() {
  allOff();
  if (button.getPressedMs() < 1800 && button.getPressedMs() > 799) {
    if (!wifiRunning) {
      wifiRunning = true;
      Serial.println("Starting WiFi server");
      setupWifi();
      blink(OFF, OFF, ON, OFF, ON, OFF);
    } else {
      Serial.println("Stopping WiFi server");
      blink(OFF, OFF, ON, ON, OFF, OFF);
      server.end();
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      wifiRunning = false;
    }
  } else if (button.getPressedMs() > 2000) {
    Serial.println("Go to sleep");
    if (wifiRunning) {
      server.end();
      wifiRunning = false;
    }
    running = false;
    blink(ON, OFF, ON, OFF, ON, ON);
  }
}
