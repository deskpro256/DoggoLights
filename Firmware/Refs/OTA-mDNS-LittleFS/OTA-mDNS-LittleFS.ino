/**
   @file OTA-mDNS-LittleFS.ino

   @author Pascal Gollor (http://www.pgollor.de/cms/)
   @date 2015-09-18

   changelog:
   2015-10-22:
   - Use new ArduinoOTA library.
   - loadConfig function can handle different line endings
   - remove mDNS studd. ArduinoOTA handle it.

*/

#ifndef APSSID
#define APSSID "DoggoLights"
#define APPSK "Asshole!23"
#endif

// includes
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>


#define BLUE_LED1 14
#define RED_LED1 12
#define GREEN_LED1 13

#define BLUE_LED2 0
#define RED_LED2 4
#define GREEN_LED2 5

#define BUTTON 2

int delayTime = 20;  // Speed control. Higher number = slower fades.
bool buttonPressed = false;
bool wifiEnabled = false;
volatile int mode = 0;

/**
   @brief mDNS and OTA Constants
   @{
*/
#define HOSTNAME "DoggoLight"  ///< Hostname. The setup function adds the Chip ID at the end.
/// @}

/**
   @brief Default WiFi connection information.
   @{
*/
const char *ap_default_ssid = APSSID;  ///< Default SSID.
const char *ap_default_psk = APPSK;    ///< Default PSK.
/// @}

/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

/**
   @brief Read WiFi connection information from file system.
   @param ssid String pointer for storing SSID.
   @param pass String pointer for storing PSK.
   @return True or False.

   The config file has to contain the WiFi SSID in the first line
   and the WiFi PSK in the second line.
   Line separator can be \r\n (CR LF) \r or \n.
*/
bool loadConfig(String *ssid, String *pass) {
  // open file for reading.
  File configFile = LittleFS.open("/cl_conf.txt", "r");
  if (!configFile) {
    Serial.println("Failed to open cl_conf.txt.");

    return false;
  }

  // Read content from config file.
  String content = configFile.readString();
  configFile.close();

  content.trim();

  // Check if there is a second line available.
  int8_t pos = content.indexOf("\r\n");
  uint8_t le = 2;
  // check for linux and mac line ending.
  if (pos == -1) {
    le = 1;
    pos = content.indexOf("\n");
    if (pos == -1) { pos = content.indexOf("\r"); }
  }

  // If there is no second line: Some information is missing.
  if (pos == -1) {
    Serial.println("Infvalid content.");
    Serial.println(content);

    return false;
  }

  // Store SSID and PSK into string vars.
  *ssid = content.substring(0, pos);
  *pass = content.substring(pos + le);

  ssid->trim();
  pass->trim();

  Serial.println("----- file content -----");
  Serial.println(content);
  Serial.println("----- file content -----");
  Serial.println("ssid: " + *ssid);
  Serial.println("psk:  " + *pass);

  return true;
}  // loadConfig


/**
   @brief Save WiFi SSID and PSK to configuration file.
   @param ssid SSID as string pointer.
   @param pass PSK as string pointer,
   @return True or False.
*/
bool saveConfig(String *ssid, String *pass) {
  // Open config file for writing.
  File configFile = LittleFS.open("/cl_conf.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open cl_conf.txt for writing");

    return false;
  }

  // Save SSID and PSK.
  configFile.println(*ssid);
  configFile.println(*pass);

  configFile.close();

  return true;
}  // saveConfig


/**
   @brief Arduino setup function.
*/
void doTheWiFiStuff() {

  wifiEnabled = true;
  yield();
  digitalWrite(BLUE_LED1, LOW);
  delay(100);
  digitalWrite(GREEN_LED1, LOW);
  delay(100);
  digitalWrite(RED_LED1, LOW);
  delay(100);
  digitalWrite(BLUE_LED1, HIGH);
  digitalWrite(GREEN_LED1, HIGH);
  digitalWrite(RED_LED1, HIGH);

  String station_ssid = "";
  String station_psk = "";


  Serial.println("\r\n");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);

  // Set Hostname.
  String hostname(HOSTNAME);
  //hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  // Print hostname.
  Serial.println("Hostname: " + hostname);
  // Serial.println(WiFi.hostname());


  // Initialize file system.
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Go into software AP mode.
  WiFi.mode(WIFI_AP);

  delay(10);

  WiFi.softAP(ap_default_ssid, ap_default_psk);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void ICACHE_RAM_ATTR isr() {
  mode += 1;
}


void fadeUp(int fadePin1, int fadePin2, int onPin1, int onPin2, int offPin1, int offPin2) {
  //set constant colors
  digitalWrite(onPin1, HIGH);
  digitalWrite(onPin2, HIGH);
  digitalWrite(offPin1, LOW);
  digitalWrite(offPin2, LOW);
  //set current brightness out of 1000
  for (int bright = 1; bright < 1000; bright = bright + 10) {
    yield();
    //set PWM lengths
    int on = bright;
    int off = 1000 - bright;
    //software PWM for 'time' ms
    for (int run = 0; run < delayTime; run++) {
      yield();
      digitalWrite(fadePin1, HIGH);
      digitalWrite(fadePin2, HIGH);
      delayMicroseconds(on);
      digitalWrite(fadePin1, LOW);
      digitalWrite(fadePin2, LOW);
      delayMicroseconds(off);
    }
  }
}

void fadeDown(int fadePin1, int fadePin2, int onPin1, int onPin2, int offPin1, int offPin2) {
  //set constant colors
  digitalWrite(onPin1, HIGH);
  digitalWrite(onPin2, HIGH);
  digitalWrite(offPin1, LOW);
  digitalWrite(offPin2, LOW);
  //set current brightness out of 1000
  for (int bright = 1; bright < 1000; bright = bright + 10) {
    yield();
    //set inverse PWM lengths
    int on = 1000 - bright;
    int off = bright;
    //software PWM for 'time' ms
    for (int run = 0; run < delayTime; run++) {
      yield();
      digitalWrite(fadePin1, HIGH);
      digitalWrite(fadePin2, HIGH);
      delayMicroseconds(on);
      digitalWrite(fadePin1, LOW);
      digitalWrite(fadePin2, LOW);
      delayMicroseconds(off);
    }
  }
}

void red() {
  digitalWrite(RED_LED1, LOW);
  digitalWrite(RED_LED2, LOW);
  digitalWrite(BLUE_LED1, HIGH);
  digitalWrite(BLUE_LED2, HIGH);
  digitalWrite(GREEN_LED1, HIGH);
  digitalWrite(GREEN_LED2, HIGH);
}
void green() {
  digitalWrite(GREEN_LED1, LOW);
  digitalWrite(GREEN_LED2, LOW);
  digitalWrite(RED_LED1, HIGH);
  digitalWrite(RED_LED2, HIGH);
  digitalWrite(BLUE_LED1, HIGH);
  digitalWrite(BLUE_LED2, HIGH);
}
void blue() {
  digitalWrite(BLUE_LED1, LOW);
  digitalWrite(BLUE_LED2, LOW);
  digitalWrite(GREEN_LED1, HIGH);
  digitalWrite(GREEN_LED2, HIGH);
  digitalWrite(RED_LED1, HIGH);
  digitalWrite(RED_LED2, HIGH);
}
void white() {
  digitalWrite(RED_LED1, LOW);
  digitalWrite(RED_LED2, LOW);
  digitalWrite(BLUE_LED1, LOW);
  digitalWrite(BLUE_LED2, LOW);
  digitalWrite(GREEN_LED1, LOW);
  digitalWrite(GREEN_LED2, LOW);
}
void allOff() {
  digitalWrite(RED_LED1, HIGH);
  digitalWrite(RED_LED2, HIGH);
  digitalWrite(BLUE_LED1, HIGH);
  digitalWrite(BLUE_LED2, HIGH);
  digitalWrite(GREEN_LED1, HIGH);
  digitalWrite(GREEN_LED2, HIGH);
}

void rainbow() {
  fadeUp(GREEN_LED1, GREEN_LED2, RED_LED1, RED_LED2, BLUE_LED1, BLUE_LED2);    //red to yellow
  fadeDown(RED_LED1, RED_LED2, GREEN_LED1, GREEN_LED2, BLUE_LED1, BLUE_LED2);  //yellow to green
  fadeUp(BLUE_LED1, BLUE_LED2, GREEN_LED1, GREEN_LED2, RED_LED1, RED_LED2);    //green to cyan
  fadeDown(GREEN_LED1, GREEN_LED2, BLUE_LED1, BLUE_LED2, RED_LED1, RED_LED2);  //cyan to blue
  fadeUp(RED_LED1, RED_LED2, BLUE_LED1, BLUE_LED2, GREEN_LED1, GREEN_LED2);    //blue to purple
  fadeDown(BLUE_LED1, BLUE_LED2, RED_LED1, RED_LED2, GREEN_LED1, GREEN_LED2);  //purple to red
}


void setup() {

  pinMode(BUTTON, INPUT);
  attachInterrupt(BUTTON, isr, FALLING);

  pinMode(BLUE_LED1, OUTPUT);
  pinMode(GREEN_LED1, OUTPUT);
  pinMode(RED_LED1, OUTPUT);

  pinMode(BLUE_LED2, OUTPUT);
  pinMode(GREEN_LED2, OUTPUT);
  pinMode(RED_LED2, OUTPUT);

  digitalWrite(BLUE_LED1, HIGH);
  digitalWrite(GREEN_LED1, HIGH);
  digitalWrite(RED_LED1, HIGH);

  digitalWrite(BLUE_LED2, HIGH);
  digitalWrite(GREEN_LED2, HIGH);
  digitalWrite(RED_LED2, HIGH);

  Serial.begin(115200);
  ESP.wdtDisable();
  ESP.deepSleep(0);
}

/**
   @brief Arduino loop function.
*/
void loop() {
  Serial.println(mode);
  switch (mode) {
    case 1:
      red();
      break;
    case 2:
      green();
      break;
    case 3:
      blue();
      break;
    case 4:
      white();
      break;
    case 5:
      rainbow();
      break;
    case 6:
      if (!wifiEnabled) {
        Serial.println("Turning on WiFi AP");
        doTheWiFiStuff();
      } else {
        Serial.println("Serve web page");
      }
      break;
    default:
      allOff();
      break;
  }
  if (mode > 6) { mode = 0; }
}