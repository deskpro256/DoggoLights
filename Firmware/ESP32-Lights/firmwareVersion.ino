// Function to compare two version strings
int compareVersions(String version1, String version2) {
  int vnum1 = 0, vnum2 = 0;

  for (int i = 0, j = 0; (i < version1.length() || j < version2.length());) {
    while (i < version1.length() && version1[i] != '.') {
      vnum1 = vnum1 * 10 + (version1[i] - '0');
      i++;
    }
    while (j < version2.length() && version2[j] != '.') {
      vnum2 = vnum2 * 10 + (version2[j] - '0');
      j++;
    }

    if (vnum1 > vnum2) {
      return 1;
    }
    if (vnum2 > vnum1) {
      return -1;
    }

    // Reset variables for next iteration
    vnum1 = vnum2 = 0;
    i++;
    j++;
  }

  return 0;
}

String getLatestFirmwareVersion() {
  String firmwareVersion = "";
  WiFiClientSecure client;
  client.setInsecure();  // Set client to allow insecure connections

  if (client.connect(HOST, PORT)) {
    Serial.println("Connected to server");

    client.print(String("GET ") + VERSION_PATH + " HTTP/1.1\r\n");
    client.print(String("Host: ") + HOST + "\r\n");
    client.println("Connection: close\r\n");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;  // Headers finished
      }
    }

    // Read the body of the response which should contain the version string
    firmwareVersion = client.readStringUntil('\n');
    firmwareVersion.trim();  // Remove any extra whitespace/newline characters
  } else {
    Serial.println("Connection to server failed.");
  }

  client.stop();
  return firmwareVersion;
}
