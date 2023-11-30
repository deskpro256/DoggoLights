void preset1() {
  button.tick();
  RGB rgb1 = hslToRgb(preset1Color1);
  RGB rgb2 = hslToRgb(preset1Color2);
  switch (preset1Effect) {
    case 1:
      staticColor(rgb1, rgb2);
      break;
    case 2:
      breathing(rgb1, rgb2);
      break;
    case 3:
      flipFlop(rgb1, rgb2);
      break;
    case 4:
      rainbow();
      break;
  }
  button.tick();
}

void preset2() {
  button.tick();
  RGB rgb1 = hslToRgb(preset2Color1);
  RGB rgb2 = hslToRgb(preset2Color2);
  switch (preset2Effect) {
    case 1:
      staticColor(rgb1, rgb2);
      break;
    case 2:
      breathing(rgb1, rgb2);
      break;
    case 3:
      flipFlop(rgb1, rgb2);
      break;
    case 4:
      rainbow();
      break;
  }
  button.tick();
}
void preset3() {
  button.tick();
  RGB rgb1 = hslToRgb(preset3Color1);
  RGB rgb2 = hslToRgb(preset3Color2);
  switch (preset3Effect) {
    case 1:
      staticColor(rgb1, rgb2);
      break;
    case 2:
      breathing(rgb1, rgb2);
      break;
    case 3:
      flipFlop(rgb1, rgb2);
      break;
    case 4:
      rainbow();
      break;
  }
  button.tick();
}

void staticColor(RGB rgb1, RGB rgb2) {
  button.tick();
  int r1 = invertPwmSignal(rgb1.r);
  button.tick();
  int g1 = invertPwmSignal(rgb1.g);
  button.tick();
  int b1 = invertPwmSignal(rgb1.b);
  button.tick();
  int r2 = invertPwmSignal(rgb2.r);
  button.tick();
  int g2 = invertPwmSignal(rgb2.g);
  button.tick();
  int b2 = invertPwmSignal(rgb2.b);
  button.tick();
  ledcWrite(ledChannelR1, r1);
  button.tick();
  ledcWrite(ledChannelG1, g1);
  button.tick();
  ledcWrite(ledChannelB1, b1);
  button.tick();
  ledcWrite(ledChannelR2, r2);
  button.tick();
  ledcWrite(ledChannelG2, g2);
  button.tick();
  ledcWrite(ledChannelB2, b2);
  button.tick();
}

int invertPwmSignal(int invertedValue) {
  // Ensure the invertedValue is in the range [0, 255]
  invertedValue = (invertedValue < 0) ? 0 : (invertedValue > 255) ? 255
                                                                  : invertedValue;
  // Convert the inverted PWM signal to the standard PWM signal
  int standardValue = 255 - invertedValue;
  return standardValue;
}

RGB hslToRgb(double h) {
  // Ensure h is in the range [0, 360), s and l are in the range [0, 1]
  h = fmod((h + 360), 360);
  double s = 1;    //fmax(0, fmin(1, s));
  double l = 0.5;  //fmax(0, fmin(1, l));

  // Convert HSL to RGB
  double c = (1 - fabs(2 * l - 1)) * s;
  double x = c * (1 - fabs(fmod(h / 60, 2) - 1));
  double m = l - c / 2;

  int red, green, blue;

  if (h >= 0 && h < 60) {
    red = (int)((c + m) * 255);
    green = (int)((x + m) * 255);
    blue = (int)(m * 255);
  } else if (h >= 60 && h < 120) {
    red = (int)((x + m) * 255);
    green = (int)((c + m) * 255);
    blue = (int)(m * 255);
  } else if (h >= 120 && h < 180) {
    red = (int)(m * 255);
    green = (int)((c + m) * 255);
    blue = (int)((x + m) * 255);
  } else if (h >= 180 && h < 240) {
    red = (int)(m * 255);
    green = (int)((x + m) * 255);
    blue = (int)((c + m) * 255);
  } else if (h >= 240 && h < 300) {
    red = (int)((x + m) * 255);
    green = (int)(m * 255);
    blue = (int)((c + m) * 255);
  } else {
    red = (int)((c + m) * 255);
    green = (int)(m * 255);
    blue = (int)((x + m) * 255);
  }

  RGB result = { red, green, blue };
  return result;
}

void setupLEDPins() {
  // configure LED PWM functionalitites
  ledcSetup(ledChannelR1, freq, resolution);
  ledcSetup(ledChannelG1, freq, resolution);
  ledcSetup(ledChannelB1, freq, resolution);
  ledcSetup(ledChannelR2, freq, resolution);
  ledcSetup(ledChannelG2, freq, resolution);
  ledcSetup(ledChannelB2, freq, resolution);

  ledcAttachPin(RED_LED1, ledChannelR1);
  ledcAttachPin(GREEN_LED1, ledChannelG1);
  ledcAttachPin(BLUE_LED1, ledChannelB1);
  ledcAttachPin(RED_LED2, ledChannelR2);
  ledcAttachPin(GREEN_LED2, ledChannelG2);
  ledcAttachPin(BLUE_LED2, ledChannelB2);
}

void solid(int r1, int g1, int b1, int r2, int g2, int b2) {
  ledcWrite(ledChannelR1, r1);
  ledcWrite(ledChannelG1, g1);
  ledcWrite(ledChannelB1, b1);
  ledcWrite(ledChannelR2, r2);
  ledcWrite(ledChannelG2, g2);
  ledcWrite(ledChannelB2, b2);
}

void solidPulsing(bool r1, bool g1, bool b1, bool r2, bool g2, bool b2) {  //RGB rgb1, RGB rgb2
                                                                           // increase the LED brightness
  button.tick();
  allOff();
  button.tick();
  for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {

    if (clicked) {
      break;
    }
    if (!running) {
      break;
    }
    // changing the LED brightness with PWM
    if (r1) {
      ledcWrite(ledChannelR1, dutyCycle);
    }
    if (g1) {
      ledcWrite(ledChannelG1, dutyCycle);
    }
    if (b1) {
      ledcWrite(ledChannelB1, dutyCycle);
    }
    if (r2) {
      ledcWrite(ledChannelR2, dutyCycle);
    }
    if (g2) {
      ledcWrite(ledChannelG2, dutyCycle);
    }
    if (b2) {
      ledcWrite(ledChannelB2, dutyCycle);
    }
    button.tick();
    delay(10);
    button.tick();
  }

  // decrease the LED brightness
  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    if (clicked) {
      clicked = false;
      break;
    }
    if (!running) {
      break;
    }
    // changing the LED brightness with PWM
    if (r1) { ledcWrite(ledChannelR1, dutyCycle); }
    if (g1) { ledcWrite(ledChannelG1, dutyCycle); }
    if (b1) { ledcWrite(ledChannelB1, dutyCycle); }
    if (r2) { ledcWrite(ledChannelR2, dutyCycle); }
    if (g2) { ledcWrite(ledChannelG2, dutyCycle); }
    if (b2) { ledcWrite(ledChannelB2, dutyCycle); }
    button.tick();
    delay(10);
    button.tick();
  }
  allOff();
}

void blink(int r1, int g1, int b1, int r2, int g2, int b2) {
  for (int i = 0; i < 4; i++) {
    ledcWrite(ledChannelR1, r1);
    ledcWrite(ledChannelG1, g1);
    ledcWrite(ledChannelB1, b1);
    ledcWrite(ledChannelR2, r2);
    ledcWrite(ledChannelG2, g2);
    ledcWrite(ledChannelB2, b2);
    delay(100);
    ledcWrite(ledChannelR1, OFF);
    ledcWrite(ledChannelG1, OFF);
    ledcWrite(ledChannelB1, OFF);
    ledcWrite(ledChannelR2, OFF);
    ledcWrite(ledChannelG2, OFF);
    ledcWrite(ledChannelB2, OFF);
    delay(100);
  }
}

void allOff() {
  ledcWrite(ledChannelR1, OFF);
  ledcWrite(ledChannelG1, OFF);
  ledcWrite(ledChannelB1, OFF);
  ledcWrite(ledChannelR2, OFF);
  ledcWrite(ledChannelG2, OFF);
  ledcWrite(ledChannelB2, OFF);
}

void rainbow() {
  RGB rgb1;
  int i;
  for (i = 0; i <= 360; i++) {
    if (clicked) {
      break;
    }
    if (!running) {
      break;
    }
    button.tick();
    rgb1 = hslToRgb(i);
    button.tick();
    int r1 = invertPwmSignal(rgb1.r);
    button.tick();
    int g1 = invertPwmSignal(rgb1.g);
    button.tick();
    int b1 = invertPwmSignal(rgb1.b);
    button.tick();
    int r2 = invertPwmSignal(rgb1.r);
    button.tick();
    int g2 = invertPwmSignal(rgb1.g);
    button.tick();
    int b2 = invertPwmSignal(rgb1.b);
    button.tick();
    ledcWrite(ledChannelR1, r1);
    button.tick();
    ledcWrite(ledChannelG1, g1);
    button.tick();
    ledcWrite(ledChannelB1, b1);
    button.tick();
    ledcWrite(ledChannelR2, r2);
    button.tick();
    ledcWrite(ledChannelG2, g2);
    button.tick();
    ledcWrite(ledChannelB2, b2);
    button.tick();
    delay(10);
    button.tick();
  }
  if (i >= 360) {
    i = 0;
  };
  button.tick();
}

void breathing(RGB rgb1, RGB rgb2) {
  button.tick();
  solidPulsing(1, 1, 0, 0, 1, 0);
  button.tick();
}

/*
  int r1 = invertPwmSignal(rgb1.r);
  int g1 = invertPwmSignal(rgb1.g);
  int b1 = invertPwmSignal(rgb1.b);
  int r2 = invertPwmSignal(rgb2.r);
  int g2 = invertPwmSignal(rgb2.g);
  int b2 = invertPwmSignal(rgb2.b);
  ledcWrite(ledChannelR1, r1);
  ledcWrite(ledChannelG1, g1);
  ledcWrite(ledChannelB1, b1);
  ledcWrite(ledChannelR2, r2);
  ledcWrite(ledChannelG2, g2);
  ledcWrite(ledChannelB2, b2);
*/

void flipFlop(RGB rgb1, RGB rgb2) {
  button.tick();
  int r1 = invertPwmSignal(rgb1.r);
  int g1 = invertPwmSignal(rgb1.g);
  int b1 = invertPwmSignal(rgb1.b);
  int r2 = invertPwmSignal(rgb2.r);
  int g2 = invertPwmSignal(rgb2.g);
  int b2 = invertPwmSignal(rgb2.b);
  button.tick();
  unsigned long previousMillis = 0;  // will store last time LED was updated
  const long interval = 300;         // interval at which to blink (milliseconds)
  unsigned long currentMillis = millis();
  int led1State = LOW;  // ledState used to set the LED
  int led2State = LOW;  // ledState used to set the LED
  allOff();
  while (running) {
    button.tick();
    if (clicked) {
      clicked = false;
      break;
    }
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (led1State == OFF) {
        button.tick();
        led1State = ON;
        led2State = OFF;
        ledcWrite(ledChannelR1, r1);
        ledcWrite(ledChannelG1, g1);
        ledcWrite(ledChannelB1, b1);
        ledcWrite(ledChannelR2, OFF);
        ledcWrite(ledChannelG2, OFF);
        ledcWrite(ledChannelB2, OFF);
      } else {
        button.tick();
        led1State = OFF;
        led2State = ON;
        ledcWrite(ledChannelR1, OFF);
        ledcWrite(ledChannelG1, OFF);
        ledcWrite(ledChannelB1, OFF);
        ledcWrite(ledChannelR2, r2);
        ledcWrite(ledChannelG2, g2);
        ledcWrite(ledChannelB2, b2);
      }
    }
  }
}

void doTheAnimation() {
  button.tick();
  Serial.println(mode);
  switch (mode) {
    case 0:
      button.tick();
      preset1();
      button.tick();
      break;
    case 1:
      button.tick();
      preset2();
      button.tick();
      break;
    case 2:
      button.tick();
      preset3();
      button.tick();
      break;
  }
  button.tick();
}
