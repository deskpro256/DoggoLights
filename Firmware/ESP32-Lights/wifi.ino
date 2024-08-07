char HTML1[] PROGMEM = "<!DOCTYPE html>\n"
                       "<html>\n"
                       "   <head>\n"
                       "      <title>DoggyLights</title>\n"
                       "      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
                       "      <style>\n"
                       "         body {\n"
                       "         font-family: Trebuchet MS;\n"
                       "         text-align: center;\n"
                       "         color: #e7e8eb;\n"
                       "         background-color: #1b1b1b;\n"
                       "         }\n"
                       "         /* Style the tab */\n"
                       "         .tab {\n"
                       "         overflow: hidden;\n"
                       "         border: 1px solid #eee;\n"
                       "         color: #363636;\n"
                       "         background-color: #121212;\n"
                       "         }\n"
                       "         form {\n"
                       "         display: inline-block;\n"
                       "         }\n"
                       "         input[type=\"submit\"] {\n"
                       "         background-color: #04aa6d;\n"
                       "         border: none;\n"
                       "         color: white;\n"
                       "         padding: 16px 32px;\n"
                       "         text-decoration: none;\n"
                       "         margin: 4px 2px;\n"
                       "         cursor: pointer;\n"
                       "         }\n"
                       "         /* Style the buttons inside the tab */\n"
                       "         .tab button {\n"
                       "         background-color: inherit;\n"
                       "         color: #eee;\n"
                       "         float: left;\n"
                       "         border: none;\n"
                       "         outline: none;\n"
                       "         cursor: pointer;\n"
                       "         width: 50%;\n"
                       "         padding: 14px;\n"
                       "         transition: 0.3s;\n"
                       "         font-size: 17px;\n"
                       "         }\n"
                       "         /* Change background color of buttons on hover */\n"
                       "         .tab button:hover {\n"
                       "         background-color: #3d3d3d;\n"
                       "         color: #eee;\n"
                       "         }\n"
                       "         /* Create an active/current tablink class */\n"
                       "         .tab button.active {\n"
                       "         background-color: #282828;\n"
                       "         color: #eee;\n"
                       "         }\n"
                       "         /* Style the tab content */\n"
                       "         .tabcontent {\n"
                       "         display: none;\n"
                       "         padding: 6px 12px;\n"
                       "         border: 1px solid #ccc;\n"
                       "         border-top: none;\n"
                       "         }\n"
                       "         .spectrum {\n"
                       "         display: flex;\n"
                       "         width: 80vw;\n"
                       "         height: 30px;\n"
                       "         margin: 0 0 -3px 7px;\n"
                       "         }\n"
                       "         .color-stop {\n"
                       "         flex: 1;\n"
                       "         height: 100%;\n"
                       "         }\n"
                       "         input[type=\"range\"] {\n"
                       "         width: 82vw;\n"
                       "         margin-top: -5px;\n"
                       "         }\n"
                       "         .color-display {\n"
                       "         width: 80vw;\n"
                       "         height: 50px;\n"
                       "         margin: 10px 7px;\n"
                       "         border: 1px solid #000;\n"
                       "         }\n"
                       "         .led-button {\n"
                       "         width: 50px;\n"
                       "         height: 50px;\n"
                       "         margin: 20px;\n"
                       "         border: none;\n"
                       "         cursor: pointer;\n"
                       "         border-radius: 80%;\n"
                       "         box-shadow: 0 0 20px rgba(0, 0, 0, 0.5);\n"
                       "         }\n"
                       "      </style>\n"
                       "   </head>\n"
                       "   <body>\n"
                       "      <h3>Dog light configurator</h3>\n"
                       "      <p>Battery: ";

char HTML2[] PROGMEM = "%</p>\n"
                       "      <div class=\"tab\">\n"
                       "         <button class=\"tablinks\" onclick=\"openTab(event, 'Lights')\" id=\"defaultOpen\">Lights</button>\n"
                       "         <button class=\"tablinks\" onclick=\"openTab(event, 'Device')\">Device</button>\n"
                       "      </div>\n"
                       "      <div id=\"Lights\" class=\"tabcontent\">\n"
                       "         <div id=\"ledContainer\">\n"
                       "            <button id=\"led1\" class=\"led-button\" onclick=\"selectLED(1)\" style=\"background-color: hsl(0, 100%, 50%);\"></button>\n"
                       "            <button id=\"led2\" class=\"led-button\" onclick=\"selectLED(2)\" style=\"background-color: hsl(0, 100%, 50%);\"></button>\n"
                       "         </div>\n"
                       "         <br />\n"
                       "         <form action=\"/get\">\n"
                       "            <br />\n"
                       "            <label for=\"preset\">Preset:</label>\n"
                       "            <select id=\"preset\" name=\"preset\">\n"
                       "               <option value=\"0\">Preset 1</option>\n"
                       "               <option value=\"1\">Preset 2</option>\n"
                       "               <option value=\"2\">Preset 3</option>\n"
                       "            </select>\n"
                       "            <br />\n"
                       "            <br />\n"
                       "            <label for=\"effect\">Effect:</label>\n"
                       "            <select id=\"effect\" name=\"effect\" onchange=\"effectChange()\">\n"
                       "               <option value=\"1\">Static</option>\n"
                       "               <option value=\"2\">Breathing</option>\n"
                       "               <option value=\"3\">FlipFlop</option>\n"
                       "               <option value=\"4\">Rainbow</option>\n"
                       "            </select>\n"
                       "            <br />\n"
                       "            <br />\n"
                       "            <div id=\"colorInputs\">\n"
                       "               <input type=\"checkbox\" id=\"bothLEDsCheck\" onclick=\"sameLEDSliders()\" /> Both LEDs same color.\n"
                       "               <p>LED Color:</p>\n"
                       "               <div class=\"spectrum\">\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(0, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(15, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(30, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(45, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(60, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(75, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(90, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(105, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(120, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(135, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(150, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(165, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(180, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(195, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(210, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(225, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(240, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(255, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(270, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(285, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(300, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(315, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(330, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(345, 100%, 50%);\"></div>\n"
                       "                  <div class=\"color-stop\" style=\"background-color: hsl(360, 100%, 50%);\"></div>\n"
                       "               </div>\n"
                       "               <input name=\"colorInput1\" id=\"colorInput1\" type=\"range\" min=\"0\" max=\"360\" step=\"15\" value=\"0\" oninput=\"updateSelectedLEDColor()\" />\n"
                       "               <input name=\"colorInput2\" id=\"colorInput2\" type=\"range\" min=\"0\" max=\"360\" step=\"15\" value=\"0\" oninput=\"updateSelectedLEDColor()\" style=\"display: none;\" />\n"
                       "            </div>\n"
                       "            <br />\n"
                       "            <input id=\"save\" type=\"submit\" value=\"Save\" />\n"
                       "         </form>\n"
                       "      </div>\n"
                       "      <div id=\"Device\" class=\"tabcontent\">\n"
                       "         <p>Change the device name and password. This will reset the device. You'll need to connect again using the new\n"
                       "            credentials.\n"
                       "         </p>\n"
                       "         <form action=\"/get\">\n"
                       "            <label for=\"devicename\">Device name: </label>\n"
                       "            <br />\n"
                       "            <input type=\"text\" name=\"devicename\" placeholder=\"";

char HTML3[] PROGMEM = "\" required />\n"
                       "            <br />\n"
                       "            <br />\n"
                       "            <label for=\"password\">Password: </label>\n"
                       "            <br />\n"
                       "            <input type=\"password\" name=\"password\" placeholder=\"Password\" required />\n"
                       "            <br />\n"
                       "            <br />\n"
                       "            <input type=\"submit\" value=\"Save\" />\n"
                       "         </form>\n"
                       "      </div>\n"
                       "      <script>\n"
                       "         document.getElementById(\"defaultOpen\").click();\n"
                       "         let selectedLED = 1;\n"
                       "         selectLED(selectedLED);\n"
                       "         let flipFlopInterval, breathingInterval, rainbowInterval;\n"
                       "         \n"
                       "         function openTab(evt, tabName) {\n"
                       "             const tabcontent = document.getElementsByClassName(\"tabcontent\");\n"
                       "             for (let i = 0; i < tabcontent.length; i++) {\n"
                       "                 tabcontent[i].style.display = \"none\";\n"
                       "             }\n"
                       "             const tablinks = document.getElementsByClassName(\"tablinks\");\n"
                       "             for (let i = 0; i < tablinks.length; i++) {\n"
                       "                 tablinks[i].className = tablinks[i].className.replace(\" active\", \"\");\n"
                       "             }\n"
                       "             document.getElementById(tabName).style.display = \"block\";\n"
                       "             evt.currentTarget.className += \" active\";\n"
                       "         }\n"
                       "         \n"
                       "         function selectLED(ledNumber) {\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "             selectedLED = ledNumber;\n"
                       "             if (selectedLED === 1) {\n"
                       "         led1.style.border = \"3px solid #e7e7e7\"\n"
                       "         led2.style.border = \"\"\n"
                       "                 document.getElementById('colorInput1').style.display = 'block';\n"
                       "                 document.getElementById('colorInput2').style.display = 'none';\n"
                       "             } else {\n"
                       "         led2.style.border = \"3px solid #e7e7e7\"\n"
                       "         led1.style.border = \"\"\n"
                       "                 document.getElementById('colorInput2').style.display = 'block';\n"
                       "                 document.getElementById('colorInput1').style.display = 'none';\n"
                       "             }\n"
                       "             updateSliderValue();\n"
                       "         }\n"
                       "         \n"
                       "         function updateSliderValue() {\n"
                       "             const colorInput1 = document.getElementById('colorInput1');\n"
                       "             const colorInput2 = document.getElementById('colorInput2');\n"
                       "             const bothSameColor = document.getElementById('bothLEDsCheck').checked;\n"
                       "         \n"
                       "             if (bothSameColor) {\n"
                       "                 colorInput2.value = colorInput1.value;\n"
                       "             } else {\n"
                       "                 if (selectedLED === 1) {\n"
                       "                     colorInput1.value = colorInput1.value; // Set slider to LED1's value\n"
                       "                 } else {\n"
                       "                     colorInput2.value = colorInput2.value; // Set slider to LED2's value\n"
                       "                 }\n"
                       "             }\n"
                       "         }\n"
                       "         \n"
                       "         function updateSelectedLEDColor() {\n"
                       "             const colorInput1 = document.getElementById('colorInput1');\n"
                       "             const colorInput2 = document.getElementById('colorInput2');\n"
                       "             const bothSameColor = document.getElementById('bothLEDsCheck').checked;\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "         \n"
                       "             if (bothSameColor) {\n"
                       "                 colorInput2.value = colorInput1.value;\n"
                       "                 led1.style.backgroundColor = `hsl(${colorInput1.value}, 100%, 50%)`;\n"
                       "                 led2.style.backgroundColor = `hsl(${colorInput1.value}, 100%, 50%)`;\n"
                       "             } else {\n"
                       "                 if (selectedLED === 1) {\n"
                       "                     led1.style.backgroundColor = `hsl(${colorInput1.value}, 100%, 50%)`;\n"
                       "                 } else {\n"
                       "                     led2.style.backgroundColor = `hsl(${colorInput2.value}, 100%, 50%)`;\n"
                       "                 }\n"
                       "             }\n"
                       "         }\n"
                       "         \n"
                       "         function effectChange() {\n"
                       "             const effect = document.getElementById(\"effect\").value;\n"
                       "             const colorInputs = document.getElementById(\"colorInputs\");\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "         selectLED(selectedLED);\n"
                       "         \n"
                       "             led1.style.opacity = 1;\n"
                       "             led2.style.opacity = 1;\n"
                       "         \n"
                       "             clearInterval(flipFlopInterval);\n"
                       "             clearInterval(breathingInterval);\n"
                       "             clearInterval(rainbowInterval);\n"
                       "         \n"
                       "             if (effect === \"2\") { // Breathing effect\n"
                       "                 colorInputs.style.display = \"block\";\n"
                       "                 startBreathingEffect();\n"
                       "             } else if (effect === \"3\") { // FlipFlop effect\n"
                       "                 colorInputs.style.display = \"block\";\n"
                       "                 startFlipFlopEffect();\n"
                       "             } else if (effect === \"4\") { // Rainbow effect\n"
                       "                 colorInputs.style.display = \"none\";\n"
                       "                 startRainbowEffect();\n"
                       "             } else {\n"
                       "                 colorInputs.style.display = \"block\";\n"
                       "             }\n"
                       "         }\n"
                       "         \n"
                       "         function sameLEDSliders() {\n"
                       "             const bothSameColor = document.getElementById('bothLEDsCheck').checked;\n"
                       "             const colorInput1 = document.getElementById('colorInput1');\n"
                       "             const colorInput2 = document.getElementById('colorInput2');\n"
                       "         \n"
                       "             if (bothSameColor) {\n"
                       "                 document.getElementById('colorInput1').style.display = 'block';\n"
                       "                 document.getElementById('colorInput2').style.display = 'none';\n"
                       "                 colorInput2.value = colorInput1.value; // Ensure colorInput2 matches colorInput1 when bothSameColor is checked\n"
                       "             }\n"
                       "             updateLEDColors();\n"
                       "         }\n"
                       "         \n"
                       "         function updateLEDColors() {\n"
                       "             const colorInput1 = document.getElementById('colorInput1').value;\n"
                       "             const colorInput2 = document.getElementById('colorInput2').value;\n"
                       "             const colorHSL1 = `hsl(${colorInput1}, 100%, 50%)`;\n"
                       "             const colorHSL2 = `hsl(${colorInput2}, 100%, 50%)`;\n"
                       "             const bothSameColor = document.getElementById('bothLEDsCheck').checked;\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "         let tempColorInput1 = 0;\n"
                       "         \n"
                       "             if (bothSameColor) {\n"
                       "                 led1.style.backgroundColor = colorHSL1;\n"
                       "                 led2.style.backgroundColor = colorHSL1;\n"
                       "                 document.getElementById('colorInput2').value = colorInput1; // Ensure colorInput2 matches colorInput1 when bothSameColor is checked\n"
                       "             } else {\n"
                       "                 if (selectedLED === 1) {\n"
                       "                     led1.style.backgroundColor = colorHSL1;\n"
                       "                 } else {\n"
                       "                     led2.style.backgroundColor = colorHSL2;\n"
                       "                 }\n"
                       "             }\n"
                       "         }\n"
                       "         \n"
                       "         function startBreathingEffect() {\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "             let opacity = 0;\n"
                       "             let increasing = true;\n"
                       "         \n"
                       "             breathingInterval = setInterval(() => {\n"
                       "                 if (increasing) {\n"
                       "                     opacity += 0.05;\n"
                       "                     if (opacity >= 1) {\n"
                       "                         increasing = false;\n"
                       "                     }\n"
                       "                 } else {\n"
                       "                     opacity -= 0.05;\n"
                       "                     if (opacity <= 0) {\n"
                       "                         increasing = true;\n"
                       "                     }\n"
                       "                 }\n"
                       "                 led1.style.opacity = opacity;\n"
                       "                 led2.style.opacity = opacity;\n"
                       "             }, 100); // Adjust the interval and increment for a smoother effect\n"
                       "         }\n"
                       "         \n"
                       "         function startFlipFlopEffect() {\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "             let led1State = true;\n"
                       "         \n"
                       "             flipFlopInterval = setInterval(() => {\n"
                       "                 if (led1State) {\n"
                       "                     led1.style.opacity = 1;\n"
                       "                     led2.style.opacity = 0;\n"
                       "                 } else {\n"
                       "                     led1.style.opacity = 0;\n"
                       "                     led2.style.opacity = 1;\n"
                       "                 }\n"
                       "                 led1State = !led1State;\n"
                       "             }, 300); // Adjust the interval as needed\n"
                       "         }\n"
                       "         \n"
                       "         function startRainbowEffect() {\n"
                       "             const led1 = document.getElementById('led1');\n"
                       "             const led2 = document.getElementById('led2');\n"
                       "         \n"
                       "             rainbowInterval = setInterval(() => {\n"
                       "                 let rainbow = (Date.now() / 50) % 360;\n"
                       "                 const rainbowColor = `hsl(${rainbow}, 100%, 50%)`;\n"
                       "         led1.style.border = \"\"\n"
                       "         led2.style.border = \"\"\n"
                       "                 led1.style.backgroundColor = rainbowColor;\n"
                       "                 led2.style.backgroundColor = rainbowColor;\n"
                       "             }, 25); // Adjust the interval as needed\n"
                       "         }\n"
                       "         \n"
                       "      </script>\n"
                       "   </body>\n"
                       "</html>";

void setupWifi() {
  checkWiFiTimer();
  resetWiFiTimer();
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
    resetWiFiTimer();
    readBatteryValue();
    String html = String(HTML1) + String(readBatteryValue()) + String(HTML2) + String(ssid) + String(HTML3);
    request->send(200, "text/html", html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    resetWiFiTimer();
    readBatteryValue();
    // get the param count.
    int params = request->params();
    // loop over the params.
    if (params < 3) {
      Serial.println("DEVICE SETTINGS");
      AsyncWebParameter *s = request->getParam(0);
      AsyncWebParameter *p = request->getParam(1);
      ssid = s->value().c_str();
      password = p->value().c_str();
      writeDevPreferences(ssid, password);
      blink(ON, OFF, OFF, OFF, OFF, ON);
      ESP.restart();

    } else {
      Serial.println("LED SETTINGS");
      AsyncWebParameter *preset = request->getParam(0);  // int 1-3
      AsyncWebParameter *effect = request->getParam(1);  // int 1-4
      AsyncWebParameter *led1 = request->getParam(2);    // double 0 - 360 translate to rgb
      AsyncWebParameter *led2 = request->getParam(3);    // double 0 - 360 translate to rgb

      int presetVal = atoi(preset->value().c_str());
      int effectVal = atoi(effect->value().c_str());
      double hue1 = atoi(led1->value().c_str());
      double hue2 = atoi(led2->value().c_str());

      writeLEDConfig(presetVal, effectVal, hue1, hue2);
      mode = presetVal;
      clicked = true;
      readPreferences();
    }
    // always respond to the client with something!
    String html = String(HTML1) + String(readBatteryValue()) + String(HTML2) + String(ssid) + String(HTML3);
    request->send(200, "text/html", html);
  });

  server.begin();
  MDNS.addService("http", "tcp", 80);
}
void shutdownWifi() {
  Serial.println("Stopping WiFi server");
  blink(OFF, OFF, ON, ON, OFF, OFF);
  server.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiRunning = false;
}

void checkWiFiTimer() {
  // Check if 2 minutes have elapsed
  currentWiFiMillis = millis();
  //Serial.print("currentWiFiMillis: ");
  //Serial.println(String(currentWiFiMillis));
  if (currentWiFiMillis - previousWifiMillis >= WiFiInterval) {
    if (wifiRunning) {
      Serial.println("Wifi should turn off");
      // Reset the timer
      previousWifiMillis = currentWiFiMillis;
      shutdownWifi();
    }
  }
}

void resetWiFiTimer() {
  previousWifiMillis = millis();
}
