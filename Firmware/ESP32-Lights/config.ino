void readPreferences() {
  preferences.begin("DogLights", true);

  ssid = preferences.getString("ssid", "DogLights");
  password = preferences.getString("password", "");

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

void writeDevPreferences(String name, String pass) {
  preferences.begin("DogLights", false);
  preferences.putString("ssid", name);
  preferences.putString("password", pass);
  preferences.end();
}

void writeLEDConfig(int preset, int effect, double colorHue1, double colorHue2) {
  preferences.begin("DogLights", false);
  if (preset == 1) {
    preferences.putInt("pre1Eff", effect);
    preferences.putDouble("pre1Col1", colorHue1);
    preferences.putDouble("pre1Col2", colorHue2);
  }
  if (preset == 2) {
    preferences.putInt("pre2Eff", effect);
    preferences.putDouble("pre2Col1", colorHue1);
    preferences.putDouble("pre2Col2", colorHue2);
  }
  if (preset == 3) {
    preferences.putInt("pre3Eff", effect);
    preferences.putDouble("pre3Col1", colorHue1);
    preferences.putDouble("pre3Col2", colorHue2);
  }
  preferences.end();
}