
void setupWifiAP() {
  resetWiFiAPTimer();
  checkWiFiAPTimer();
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("IP address: ");
  Serial.print(WiFi.softAPIP());
  Serial.println(" or doglights.local");

  if (!MDNS.begin("doglights")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(10);
    }
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = String(HTML1) + String(readBatteryValue()) + String(HTML2) + String(ssid) + String(HTML3) + currentFirmwareVersion + String(HTML4) + homeSsid + String(HTML5) + String(HTML6) + String(HTML7);
    resetWiFiAPTimer();
    readBatteryValue();
    Serial.print("homeWifiSet:");
    Serial.println(homeWifiSet);
    if (!homeWifiSet) {
      html = String(HTML1) + String(readBatteryValue()) + String(HTML2) + String(ssid) + String(HTML3) + currentFirmwareVersion + String(HTML4) + homeSsid + String(HTML5) + String(HTML7);
    }
    request->send(200, "text/html", html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    resetWiFiAPTimer();
    readBatteryValue();

    int params = request->params();

    AsyncWebParameter *firstParam = request->getParam(0);
    String paramName = firstParam->name();
    //device name
    if (paramName == "devicename") {
      // Handle device settings
      String ssid = request->getParam("devicename")->value();
      String password = request->getParam("password")->value();
      writeDeviceApPreferences(ssid, password);
      blink(ON, OFF, OFF, OFF, OFF, ON);
      ESP.restart();
      //home wifi
    } else if (paramName == "homeSsid") {
      // Handle home network settings
      String homeSsid = request->getParam("homeSsid")->value();
      String homePass = request->getParam("homePassword")->value();
      homeWifiSet = true;
      writeDeviceHomeWiFiPreferences(homeSsid, homePass);
      readDeviceHomeWiFiPreferences();
      // OTA update
    } else if (paramName == "update") {
      shutdownWifiAP();
      connectToWiFi();
      delay(1);

      String latestFirmware = getLatestFirmwareVersion();
      int comparisonResult = compareVersions(currentFirmwareVersion, latestFirmware);

      if (comparisonResult == 0) {
        Serial.println("The device is up-to-date.");
        delay(1);
        setupWifiAP();
        // Flash LEDs green
      } else if (comparisonResult < 0) {
        Serial.println("A newer firmware is available.");
        getFileFromServer();
        delay(1);
        performOTAUpdateFromSPIFFS();
        delay(1);
      } else {
        Serial.println("The device has a newer or same version.");
        delay(1);
        setupWifiAP();
      }
      //LED stuff
    } else if (paramName == "preset") {
      // Handle LED settings
      int presetVal = request->getParam("preset")->value().toInt();
      int effectVal = request->getParam("effect")->value().toInt();
      double hue1 = request->getParam("colorInput1")->value().toDouble();
      double hue2 = request->getParam("colorInput2")->value().toDouble();

      writeLEDConfig(presetVal, effectVal, hue1, hue2);
      mode = presetVal;
      clicked = true;
      readPreferences();
    }
    String html = String(HTML1) + String(readBatteryValue()) + String(HTML2) + String(ssid) + String(HTML3) + currentFirmwareVersion + String(HTML4) + homeSsid + String(HTML5) + String(HTML6) + String(HTML7);
    // Always respond to the client with something!
    Serial.print("homeWifiSet:");
    Serial.println(homeWifiSet);
    if (!homeWifiSet) {
      html = String(HTML1) + String(readBatteryValue()) + String(HTML2) + String(ssid) + String(HTML3) + currentFirmwareVersion + String(HTML4) + homeSsid + String(HTML5) + String(HTML7);
    }
    request->send(200, "text/html", html);
  });

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void shutdownWifiAP() {
  resetWiFiAPTimer();
  Serial.println("Stopping WiFi server");
  blink(OFF, OFF, ON, ON, OFF, OFF);
  server.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiRunning = false;
}

void checkWiFiAPTimer() {
  // Check if 2 minutes have elapsed
  currentWiFiMillis = millis();
  //Serial.print("currentWiFiMillis: ");
  //Serial.println(String(currentWiFiMillis));
  if (currentWiFiMillis - previousWifiMillis >= WiFiInterval) {
    if (wifiRunning) {
      Serial.println("Wifi should turn off");
      // Reset the timer
      previousWifiMillis = currentWiFiMillis;
      shutdownWifiAP();
    }
  }
}

void resetWiFiAPTimer() {
  previousWifiMillis = millis();
}
