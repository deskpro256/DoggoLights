void readPreferences() {
  preferences.begin("DogLights", true);
  String defaultSsid = "DogLights-" + getMacLastHalf();
  ssid = preferences.getString("ssid", defaultSsid);
  password = preferences.getString("password", "");

  currentFirmwareVersion = preferences.getString("firmwareVersion", currentFirmwareVersion);

  homeWifiSet = preferences.getBool("homeWifiSet", false);

  homeSsid = preferences.getString("homeSsid", "HomeNetworkName");
  homePass = preferences.getString("homePassword", "");

  preset1Effect = preferences.getInt("pre1Eff", 1);
  preset1Color1 = preferences.getDouble("pre1Col1", 0);
  preset1Color2 = preferences.getDouble("pre1Col2", 0);

  preset2Effect = preferences.getInt("pre2Eff", 2);
  preset2Color1 = preferences.getDouble("pre2Col1", 120);
  preset2Color2 = preferences.getDouble("pre2Col2", 120);

  preset3Effect = preferences.getInt("pre3Eff", 4);
  preset3Color1 = preferences.getDouble("pre3Col1", 0);
  preset3Color2 = preferences.getDouble("pre3Col2", 0);

  preferences.end();
}

void writeDeviceApPreferences(String name, String pass) {
  preferences.begin("DogLights", false);
  preferences.putString("ssid", name);
  preferences.putString("password", pass);
  preferences.end();
}

void writeDeviceHomeWiFiPreferences(String homeName, String homePass) {
  preferences.begin("DogLights", false);
  preferences.putString("homeSsid", homeName);
  preferences.putString("homePassword", homePass);
  preferences.putBool("homeWifiSet", homeWifiSet);
  preferences.end();
}

void readDeviceHomeWiFiPreferences(){
  preferences.begin("DogLights", true);
  homeWifiSet = preferences.getBool("homeWifiSet", false);
  homeSsid = preferences.getString("homeSsid", "HomeNetworkName");
  homePass = preferences.getString("homePassword", "");
  preferences.end();
}

void writeDeviceFirmwarePreferences(String fwVersion) {
  preferences.begin("DogLights", false);
  preferences.putString("firmwareVersion", fwVersion);
  preferences.end();
}

void readDeviceFirmwarePreferences() {
  preferences.begin("DogLights", true);
  preferences.getString("firmwareVersion", currentFirmwareVersion);
  preferences.end();
}

void writeLEDConfig(int preset, int effect, double colorHue1, double colorHue2) {
  preferences.begin("DogLights", false);
  if (preset == 0) {
    preferences.putInt("pre1Eff", effect);
    preferences.putDouble("pre1Col1", colorHue1);
    preferences.putDouble("pre1Col2", colorHue2);
  }
  if (preset == 1) {
    preferences.putInt("pre2Eff", effect);
    preferences.putDouble("pre2Col1", colorHue1);
    preferences.putDouble("pre2Col2", colorHue2);
  }
  if (preset == 2) {
    preferences.putInt("pre3Eff", effect);
    preferences.putDouble("pre3Col1", colorHue1);
    preferences.putDouble("pre3Col2", colorHue2);
  }
  preferences.end();
}

void clearPreferences() {
  preferences.begin("DogLights", false);
  preferences.clear();
  preferences.end();
}