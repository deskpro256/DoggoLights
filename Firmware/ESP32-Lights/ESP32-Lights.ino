#include <Preferences.h>
#include <OneButton.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "driver/adc.h"

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

bool clicked = false;
bool running = true;
bool wifiRunning = false;
int mode = 0;
int batteryPercentage;
float voltage;
int temp;
int sensorValue;

//WiFi settings
String ssid;
String password;

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

  readPreferences();
  //setupBatteryReading();
  setupLEDPins();
  blinkBateryPercentColor(readBatteryValue());

  while (running) {
    //Serial.println("while (running)");
    button.tick();
    doTheAnimation();
  }
  allOff();  // turn off LEDs

  constexpr auto DEFAULT_WAKEUP_PIN = BUTTON_PIN;
  constexpr auto DEFAULT_WAKEUP_LEVEL = ESP_GPIO_WAKEUP_GPIO_LOW;
  const gpio_config_t config = {
    .pin_bit_mask = BIT(DEFAULT_WAKEUP_PIN),
    .mode = GPIO_MODE_INPUT,
  };
  ESP_ERROR_CHECK(gpio_config(&config));
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(DEFAULT_WAKEUP_PIN), DEFAULT_WAKEUP_LEVEL));
  delay(100);
  esp_deep_sleep_start();  // Enter deep sleep mode
}

void loop() {
}
