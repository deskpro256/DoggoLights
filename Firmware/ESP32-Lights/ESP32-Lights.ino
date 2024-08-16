#include <Preferences.h>
#include <OneButton.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "driver/adc.h"
#include <SPIFFS.h>
#include "Update.h"
#include <WiFiClientSecure.h>

/*
NOTE:
======
Only RTC IO can be used as a source for external wake
source. They are pins: 0,2,4,12-15,25-27,32-39.
*/

Preferences preferences;

#define ON 0
#define OFF 255

int preset1Effect;
double preset1Color1;
double preset1Color2;

int preset2Effect;
double preset2Color1;
double preset2Color2;

int preset3Effect;
double preset3Color1;
double preset3Color2;

#define BLUE_LED1 6
#define RED_LED1 5
#define GREEN_LED1 4

#define BLUE_LED2 1
#define RED_LED2 2
#define GREEN_LED2 21  // new boards
//#define GREEN_LED2 3 //old prototype

//#define BAT_ADC 10 // old
#define BAT_ADC 3  //new

#define BUTTON_PIN 0
#define HOLD_TIME 500  // Time in milliseconds to detect a long press

bool clicked = false;
bool running = true;
bool wifiRunning = false;
int mode = 0;
int batteryPercentage;
float voltage;
int temp;
int sensorValue;

String currentFirmwareVersion = "1.0.0";
String latestFirmware;

//WiFi settings
//AP
String ssid;
String password;
String homeSsid;
String homePass;
bool homeWifiSet = false;

// Define server details and file path
#define HOST "raw.githubusercontent.com"
#define PATH "/deskpro256/DoggoLights/Firmware/latest/firmware.bin"
#define VERSION_PATH "/deskpro256/DoggoLights/Firmware/latest/latest.txt"
#define PORT 443

// Define the name for the downloaded firmware file
#define FILE_NAME "firmware.bin"

unsigned long previousWifiMillis = 0;  // Variable to store the last time the timer was updated
const long WiFiInterval = 120000;      // Interval for 5 minutes in milliseconds // 10sec = 10000 5min = 300000 2min = 120000
unsigned long currentWiFiMillis;       // Get the current time


// setting PWM properties
int freq = 5000;
int ledChannelR1 = 0;
int ledChannelG1 = 1;
int ledChannelB1 = 2;
int ledChannelR2 = 3;
int ledChannelG2 = 4;
int ledChannelB2 = 5;
int resolution = 8;

typedef struct {
  int r;
  int g;
  int b;
} RGB;

OneButton button(BUTTON_PIN, true, true);
AsyncWebServer server(80);

void setup() {
  // start 1 sec timer to check if button is pressed long enough to turn on
  // otherwise, go back to sleep
  pinMode(BUTTON_PIN, INPUT);  // Set button pin as input with pull-up
  waitForButtonPress();

  button.setPressMs(1000);

  button.attachClick(singleClick);
  button.attachDoubleClick(doubleClick);
  button.attachLongPressStart(longPress);

  pinMode(BAT_ADC, INPUT);

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
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  readPreferences();
  setupLEDPins();
  int battPerc = readBatteryValue();
  blinkBateryPercentColor(battPerc);
  checkForLowBatteryShutdown(battPerc);

  while (running) {
    //Serial.println("while (running)");
    button.tick();
    doTheAnimation();
  }
  goToSleep();
}

void loop() {
}
