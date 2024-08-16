void singleClick() {
  Serial.println("SHORT PRESS");
  clicked = true;
  mode++;
  if (mode > 2) {
    mode = 0;
  }
}

void doubleClick() {
  Serial.println("DOUBLE CLICK");
  if (!wifiRunning) {
    wifiRunning = true;
    Serial.println("Starting WiFi server");
    setupWifiAP();
    blink(OFF, OFF, ON, OFF, ON, OFF);
  } else {
    shutdownWifiAP();
  }
}

void longPress() {
  allOff();
  Serial.println("Go to sleep");
  if (wifiRunning) {
    server.end();
    wifiRunning = false;
  }
  blink(ON, OFF, ON, OFF, ON, ON);
  running = false;
}


void waitForButtonPress() {
    unsigned long buttonPressStartTime = 0;
    bool isButtonPressed = false;

    while (true) {
        int buttonState = digitalRead(BUTTON_PIN);

        if (buttonState == LOW && !isButtonPressed) {
            // Button just pressed
            isButtonPressed = true;
            buttonPressStartTime = millis();  // Record the time the button was pressed
        } 
        else if (buttonState == HIGH && isButtonPressed) {
            // Button was released before holding long enough
            isButtonPressed = false;
            goToSleep();  // Go to sleep if the button wasn't held long enough
        } 
        else if (buttonState == LOW && isButtonPressed) {
            // Button is being held down
            if (millis() - buttonPressStartTime >= HOLD_TIME) {
                // Button has been held long enough
                break;  // Exit the loop and proceed with setup
            }
        }

        delay(10);  // Small delay to debounce the button
    }
}

void goToSleep() {
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