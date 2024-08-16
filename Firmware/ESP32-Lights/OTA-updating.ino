
void connectToWiFi() {
  // Begin connecting to WiFi using the provided SSID and password
  WiFi.begin(homeSsid, homePass);

  // Display connection progress
  Serial.print("Connecting to WiFi");

  // Wait until WiFi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  // Print confirmation message when WiFi is connected
  Serial.println("WiFi connected");
}

void getFileFromServer() {
  WiFiClientSecure client;
  client.setInsecure();  // Set client to allow insecure connections

  if (client.connect(HOST, PORT)) {  // Connect to the server
    Serial.println("Connected to server");
    client.print("GET " + String(PATH) + " HTTP/1.1\r\n");  // Send HTTP GET request
    client.print("Host: " + String(HOST) + "\r\n");         // Specify the host
    client.println("Connection: close\r\n");                // Close connection after response
    client.println();                                       // Send an empty line to indicate end of request headers

    File file = SPIFFS.open("/" + String(FILE_NAME), FILE_WRITE);  // Open file in SPIFFS for writing
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }

    bool endOfHeaders = false;
    String headers = "";
    String http_response_code = "error";
    const size_t bufferSize = 1024;  // Buffer size for reading data
    uint8_t buffer[bufferSize];

    // Loop to read HTTP response headers
    while (client.connected() && !endOfHeaders) {
      if (client.available()) {
        char c = client.read();
        headers += c;
        if (headers.startsWith("HTTP/1.1")) {
          http_response_code = headers.substring(9, 12);
        }
        if (headers.endsWith("\r\n\r\n")) {  // Check for end of headers
          endOfHeaders = true;
        }
      }
    }

    Serial.println("HTTP response code: " + http_response_code);  // Print received headers

    // Loop to read and write raw data to file
    while (client.connected()) {
      if (client.available()) {
        size_t bytesRead = client.readBytes(buffer, bufferSize);
        file.write(buffer, bytesRead);  // Write data to file
      }
    }
    file.close();   // Close the file
    client.stop();  // Close the client connection
    Serial.println("File saved successfully");
  } else {
    Serial.println("Failed to connect to server");
  }
}

void performOTAUpdateFromSPIFFS() {
  // Open the firmware file in SPIFFS for reading
  File file = SPIFFS.open("/" + String(FILE_NAME), FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("Starting update..");
  size_t fileSize = file.size();  // Get the file size
  Serial.println(fileSize);

  // Begin OTA update process with specified size and flash destination
  if (!Update.begin(fileSize, U_FLASH)) {
    Serial.println("Cannot do the update");
    return;
  }

  // Write firmware data from file to OTA update
  Update.writeStream(file);

  // Complete the OTA update process
  if (Update.end()) {
    Serial.println("Successful update");
  } else {
    Serial.println("Error Occurred:" + String(Update.getError()));
    return;
  }

  file.close();  // Close the file
  writeDeviceFirmwarePreferences(latestFirmware);
  Serial.println("Reset in 4 seconds....");
  delay(4000);
  ESP.restart();  // Restart ESP32 to apply the update
}