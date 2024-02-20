char HTML1[] PROGMEM = "<!DOCTYPE html>\n"
                       "<html>\n"
                       "  <head>\n"
                       "    <title>DoggyLights</title>\n"
                       "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
                       "    <style>\n"
                       "      body {\n"
                       "        font-family: Trebuchet MS;\n"
                       "        text-align: center;\n"
                       "        color: #e7e8eb;\n"
                       "        background-color: #1b1b1b;\n"
                       "      }\n"
                       "      /* Style the tab */\n"
                       "      .tab {\n"
                       "        overflow: hidden;\n"
                       "        border: 1px solid #eee;\n"
                       "        color: #363636;\n"
                       "        background-color: #121212;\n"
                       "      }\n"
                       "      form {\n"
                       "        display: inline-block;\n"
                       "      }\n"
                       "      input[type=\"submit\"] {\n"
                       "        background-color: #04aa6d;\n"
                       "        border: none;\n"
                       "        color: white;\n"
                       "        padding: 16px 32px;\n"
                       "        text-decoration: none;\n"
                       "        margin: 4px 2px;\n"
                       "        cursor: pointer;\n"
                       "      }\n"
                       "      /* Style the buttons inside the tab */\n"
                       "      .tab button {\n"
                       "        background-color: inherit;\n"
                       "        color: #eee;\n"
                       "        float: left;\n"
                       "        border: none;\n"
                       "        outline: none;\n"
                       "        cursor: pointer;\n"
                       "        width: 50%;\n"
                       "        padding: 14px;\n"
                       "        transition: 0.3s;\n"
                       "        font-size: 17px;\n"
                       "      }\n"
                       "      /* Change background color of buttons on hover */\n"
                       "      .tab button:hover {\n"
                       "        background-color: #3d3d3d;\n"
                       "        color: #eee;\n"
                       "      }\n"
                       "      /* Create an active/current tablink class */\n"
                       "      .tab button.active {\n"
                       "        background-color: #282828;\n"
                       "        color: #eee;\n"
                       "      }\n"
                       "      /* Style the tab content */\n"
                       "      .tabcontent {\n"
                       "        display: none;\n"
                       "        padding: 6px 12px;\n"
                       "        border: 1px solid #ccc;\n"
                       "        border-top: none;\n"
                       "      }\n"
                       "      .spectrum {\n"
                       "        display: block;\n"
                       "        width: 350px;\n"
                       "        height: 50px;\n"
                       "        margin: 0 0 -3px 7px;\n"
                       "        background: -webkit-linear-gradient(left, hsl(0, 100%, 50%), hsl(60, 100%, 50%), hsl(120, 100%, 50%), hsl(180, 100%, 50%), hsl(240, 100%, 50%), hsl(300, 100%, 50%), hsl(360, 100%, 50%) 100%);\n"
                       "      }\n"
                       "      input[type=\"range\"] {\n"
                       "        width: 360px;\n"
                       "        margin-top: -5px;\n"
                       "      }\n"
                       "    </style>\n"
                       "  </head>\n"
                       "  <body>\n"
                       "    <h3>Dog light configurator</h3>\n"
                       "    <p>Battery: ";

char HTML2[] PROGMEM = "%</p>\n"  // battery percent split
                       "    <div class=\"tab\">\n"
                       "      <button class=\"tablinks\" onclick=\"openTab(event, 'Lights')\" id=\"defaultOpen\">Lights</button>\n"
                       "      <button class=\"tablinks\" onclick=\"openTab(event, 'Device')\">Device</button>\n"
                       "    </div>\n"
                       "    <div id=\"Lights\" class=\"tabcontent\">\n"
                       "      <canvas id=\"myCanvas\" width=\"200\" height=\"100\"></canvas>\n"
                       "      <br />\n"
                       "      <form action=\"/get\">\n"
                       "        <br />\n"
                       "        <label for=\"preset\">Preset:</label>\n"
                       "        <select id=\"preset\" name=\"preset\">\n"
                       "          <option value=\"0\">Preset 1</option>\n"
                       "          <option value=\"1\">Preset 2</option>\n"
                       "          <option value=\"2\">Preset 3</option>\n"
                       "        </select>\n"
                       "        <br />\n"
                       "        <br />\n"
                       "        <label for=\"effect\">Effect:</label>\n"
                       "        <select id=\"effect\" name=\"effect\" onchange=\"effectChange()\">\n"
                       "          <option value=\"1\">Static</option>\n"
                       "          <option value=\"2\">Breathing</option>\n"
                       "          <option value=\"3\">FlipFlop</option>\n"
                       "          <option value=\"4\">Rainbow</option>\n"
                       "        </select>\n"
                       "        <br />\n"
                       "        <br />\n"
                       "        <div id=\"colorInputs\">\n"
                       "          <input type=\"checkbox\" id=\"bothLEDsCheck\" onclick=\"sameLEDSliders()\" /> Both LEDs same color.\n"
                       "          <p>LED 1 Color:</p>\n"
                       "          <span class=\"spectrum\"></span>\n"
                       "          <input name=\"colorInput1\" id=\"colorInput1\" type=\"range\" min=\"0\" max=\"360\" step=\"1\" oninput=\"sameLEDSliders()\" />\n"
                       "          <br />\n"
                       "          <div id=\"LED2\">\n"
                       "            <p>LED 2 Color:</p>\n"
                       "            <span class=\"spectrum\"></span>\n"
                       "            <input name=\"colorInput2\" id=\"colorInput2\" type=\"range\" min=\"0\" max=\"360\" step=\"1\" />\n"
                       "          </div>\n"
                       "        </div>\n"
                       "        <br />\n"
                       "        <br />\n"
                       "        <input id=\"save\" type=\"submit\" value=\"Save\" />\n"
                       "      </form>\n"
                       "      <script>\n"
                       "        const c = document.getElementById(\"myCanvas\");\n"
                       "        const ctx = c.getContext(\"2d\");\n"
                       "        var color1 = document.getElementById(\"colorInput1\").value;\n"
                       "        var color2 = document.getElementById(\"colorInput2\").value;\n"
                       "        var effect = document.getElementById(\"effect\").value;\n"
                       "        let rainbow = 0;\n"
                       "\n"
                       "        function getRandomColor() {\n"
                       "          min = Math.ceil(0);\n"
                       "          max = Math.floor(360);\n"
                       "          return Math.floor(Math.random() * (max - min + 1) + min); // The maximum is inclusive and the minimum is inclusive\n"
                       "        }\n"
                       "\n"
                       "        function myFunc() {\n"
                       "          ctx.clearRect(0, 0, c.width, c.height);\n"
                       "          effect = document.getElementById(\"effect\").value;\n"
                       "          switch (effect) {\n"
                       "            case \"1\": //static\n"
                       "              color1 = document.getElementById(\"colorInput1\").value;\n"
                       "              color2 = document.getElementById(\"colorInput2\").value;\n"
                       "              glowAlphaRight = 1;\n"
                       "              glowAlphaLeft = 1;\n"
                       "              break;\n"
                       "\n"
                       "            case \"2\": //breathing\n"
                       "              color1 = document.getElementById(\"colorInput1\").value;\n"
                       "              color2 = document.getElementById(\"colorInput2\").value;\n"
                       "              glowAlphaRight = Math.abs(Math.sin(Date.now() * 0.001));\n"
                       "              glowAlphaLeft = glowAlphaRight;\n"
                       "              break;\n"
                       "\n"
                       "            case \"3\": //flipflop\n"
                       "              color1 = document.getElementById(\"colorInput1\").value;\n"
                       "              color2 = document.getElementById(\"colorInput2\").value;\n"
                       "              glowAlphaRight = Math.abs(Math.sin(Date.now() * 0.002));\n"
                       "              glowAlphaLeft = Math.abs(Math.cos(Date.now() * 0.002));\n"
                       "              break;\n"
                       "\n"
                       "            case \"4\": //rainbow\n"
                       "              rainbow += 0.5;\n"
                       "              if (rainbow > 360) {\n"
                       "                rainbow = 0;\n"
                       "              }\n"
                       "              color1 = rainbow;\n"
                       "              color2 = rainbow;\n"
                       "              glowAlphaRight = 1;\n"
                       "              glowAlphaLeft = 1;\n"
                       "              break;\n"
                       "          }\n"
                       "\n"
                       "          ctx.beginPath();\n"
                       "          ctx.fillStyle = \"#333\";\n"
                       "          ctx.fillRect(0, 0, 200, 100); // Case\n"
                       "          ctx.rect(0, 0, 200, 100);\n"
                       "          ctx.stroke();\n"
                       "          ctx.closePath();\n"
                       "\n"
                       "          ctx.beginPath();\n"
                       "          ctx.shadowBlur = 50;\n"
                       "          ctx.fillStyle = \"hsla(\" + color1 + \",100%, 50%, 1.0)\";\n"
                       "          ctx.shadowColor = \"hsla(\" + color1 + \",100%, 50%, 0.8)\";\n"
                       "          ctx.fillRect(30, 30, 40, 40); // LED1\n"
                       "          ctx.globalAlpha = glowAlphaLeft;\n"
                       "          ctx.closePath();\n"
                       "\n"
                       "          ctx.beginPath();\n"
                       "          ctx.shadowBlur = 50;\n"
                       "          ctx.fillStyle = \"hsla(\" + color2 + \",100%, 50%, 1.0)\";\n"
                       "          ctx.shadowColor = \"hsla(\" + color2 + \",100%, 50%, 0.8)\";\n"
                       "          ctx.fillRect(130, 30, 40, 40); // LED2\n"
                       "          ctx.globalAlpha = glowAlphaRight;\n"
                       "          ctx.closePath();\n"
                       "\n"
                       "          // Request the next frame\n"
                       "          requestAnimationFrame(myFunc);\n"
                       "        }\n"
                       "\n"
                       "        // Start the animation loop\n"
                       "        myFunc();\n"
                       "      </script>\n"
                       "    </div>\n"
                       "    <div id=\"Device\" class=\"tabcontent\">\n"
                       "      <p>Change the device name and password. This will reset the device. You'll need to connect again using the new credentials.</p>\n"
                       "      <form action=\"/get\">\n"
                       "        <label for=\"devicename\">Device name: </label>\n"
                       "        <br />\n"
                       // device name split
                       "        <input type=\"text\" name=\"devicename\" placeholder=\"";

char HTML3[] PROGMEM = "\" required />\n"
                       "        <br />\n"
                       "        <br />\n"
                       "        <label for=\"password\">Password: </label>\n"
                       "        <input type=\"checkbox\" onclick=\"showPassword()\" /> Show\n"
                       "        <br />\n"
                       "        <input type=\"password\" name=\"password\" id=\"passInput\" required />\n"
                       "        <br />\n"
                       "        <br />\n"
                       "        <br />\n"
                       "        <input type=\"submit\" value=\"Save\" />\n"
                       "      </form>\n"
                       "    </div>\n"
                       "    <script>\n"
                       "      document.getElementById(\"defaultOpen\").click();\n"
                       "      function openTab(evt, tabName) {\n"
                       "        var i, tabcontent, tablinks;\n"
                       "        tabcontent = document.getElementsByClassName(\"tabcontent\");\n"
                       "        for (i = 0; i < tabcontent.length; i++) {\n"
                       "          tabcontent[i].style.display = \"none\";\n"
                       "        }\n"
                       "        tablinks = document.getElementsByClassName(\"tablinks\");\n"
                       "        for (i = 0; i < tablinks.length; i++) {\n"
                       "          tablinks[i].className = tablinks[i].className.replace(\" active\", \"\");\n"
                       "        }\n"
                       "        document.getElementById(tabName).style.display = \"block\";\n"
                       "        evt.currentTarget.className += \" active\";\n"
                       "      }\n"
                       "\n"
                       "      function showPassword() {\n"
                       "        var x = document.getElementById(\"passInput\");\n"
                       "        if (x.type === \"password\") {\n"
                       "          x.type = \"text\";\n"
                       "        } else {\n"
                       "          x.type = \"password\";\n"
                       "        }\n"
                       "      }\n"
                       "      function effectChange() {\n"
                       "        var colorInputsAll = document.getElementById(\"colorInputs\");\n"
                       "        var effect = document.getElementById(\"effect\").value;\n"
                       "        if (effect == \"4\") {\n"
                       "          colorInputsAll.style.display = \"none\";\n"
                       "        } else {\n"
                       "          colorInputsAll.style.display = \"block\";\n"
                       "        }\n"
                       "      }\n"
                       "      function sameLEDSliders() {\n"
                       "        var led2Text = document.getElementById(\"LED2\");\n"
                       "        var spectrum2 = document.getElementById(\"spectrum2\");\n"
                       "        var colorInput2 = document.getElementById(\"colorInput2\");\n"
                       "        var check = document.getElementById(\"bothLEDsCheck\");\n"
                       "\n"
                       "        if (check.checked) {\n"
                       "          var LED1 = document.getElementById(\"colorInput1\");\n"
                       "          var LED2 = document.getElementById(\"colorInput2\");\n"
                       "          LED2.value = LED1.value;\n"
                       "          led2Text.style.display = \"none\";\n"
                       "        } else {\n"
                       "          led2Text.style.display = \"block\";\n"
                       "        }\n"
                       "      }\n"
                       "    </script>\n"
                       "  </body>\n"
                       "</html>\n";

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
  // Check if 5 minutes have elapsed
  currentWiFiMillis = millis();
  //Serial.print("currentWiFiMillis: ");
  //Serial.println(String(currentWiFiMillis));
  if (currentWiFiMillis - previousWifiMillis >= WiFiInterval) {
    if (wifiRunning) {
      //Serial.println("Wifi should turn off");
      // Reset the timer
      previousWifiMillis = currentWiFiMillis;
      shutdownWifi();
    }
  }
}

void resetWiFiTimer() {
  previousWifiMillis = millis();
}
