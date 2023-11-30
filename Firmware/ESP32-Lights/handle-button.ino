void singleClick() {
  Serial.println("SHORT PRESS");
  clicked = true;
  mode++;
  if (mode > 2) {
    mode = 0;
  }
}

void doubleClick() {
  Serial.println("DOUBLE CLICK");
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
}

void longPress() {
  allOff();
  Serial.println("Go to sleep");
  if (wifiRunning) {
    server.end();
    wifiRunning = false;
  }
  blink(ON, OFF, ON, OFF, ON, ON);
  running = false;
}