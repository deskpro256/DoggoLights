void readPreferences() {
  preferences.begin("DogLights", true);
  ssid = preferences.getString("ssid", "DogLights");
  password = preferences.getString("password", "");
  anim = preferences.getInt("anim", 1);
  red1 = preferences.getChar("red1", 0);
  grn1 = preferences.getChar("grn1", 255);
  blu1 = preferences.getChar("blu1", 255);
  red2 = preferences.getChar("red2", 0);
  grn2 = preferences.getChar("grn2", 255);
  blu2 = preferences.getChar("blu2", 255);
  preferences.end();
}

void writePreferences() {
  preferences.begin("DogLights", false);
  preferences.putString("ssid", "DogLights");
  preferences.putString("password", "");
  preferences.putInt("anim", 1);
  preferences.putChar("red1", 0);
  preferences.putChar("grn1", 255);
  preferences.putChar("blu1", 255);
  preferences.putChar("red2", 0);
  preferences.putChar("grn2", 255);
  preferences.putChar("blu2", 255);
  preferences.end();
}

void writeLEDConfig() {
  preferences.begin("DogLights", false);
  preferences.putInt("anim", mode);
  preferences.putChar("red1", red1);
  preferences.putChar("grn1", grn1);
  preferences.putChar("blu1", blu1);
  preferences.putChar("red2", red2);
  preferences.putChar("grn2", grn2);
  preferences.putChar("blu2", blu2);
  preferences.end();
}