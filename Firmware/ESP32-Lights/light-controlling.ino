
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
// t=interval
void solidPulsing(bool r1, bool g1, bool b1, bool r2, bool g2, bool b2, int t) {
  // increase the LED brightness
  allOff();
  red1 = OFF;
  grn1 = OFF;
  blu1 = OFF;
  red2 = OFF;
  grn2 = OFF;
  blu2 = OFF;

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
      red1 = ON;
    }
    if (g1) {
      ledcWrite(ledChannelG1, dutyCycle);
      grn1 = ON;
    }
    if (b1) {
      ledcWrite(ledChannelB1, dutyCycle);
      blu1 = ON;
    }
    if (r2) {
      ledcWrite(ledChannelR2, dutyCycle);
      red2 = ON;
    }
    if (g2) {
      ledcWrite(ledChannelG2, dutyCycle);
      grn2 = ON;
    }
    if (b2) {
      ledcWrite(ledChannelB2, dutyCycle);
      blu2 = ON;
    }
    button.tick();
    delay(t);
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
    delay(t);
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
  red1 = ON;
  grn1 = OFF;
  blu1 = OFF;
  red2 = ON;
  grn2 = OFF;
  blu2 = OFF;
  ledcWrite(ledChannelR1, ON);
  ledcWrite(ledChannelG1, OFF);
  ledcWrite(ledChannelB1, OFF);
  ledcWrite(ledChannelR2, ON);
  ledcWrite(ledChannelG2, OFF);
  ledcWrite(ledChannelB2, OFF);
}
void green() {
  red1 = OFF;
  grn1 = ON;
  blu1 = OFF;
  red2 = OFF;
  grn2 = ON;
  blu2 = OFF;
  ledcWrite(ledChannelR1, OFF);
  ledcWrite(ledChannelG1, ON);
  ledcWrite(ledChannelB1, OFF);
  ledcWrite(ledChannelR2, OFF);
  ledcWrite(ledChannelG2, ON);
  ledcWrite(ledChannelB2, OFF);
}
void blue() {
  red1 = OFF;
  grn1 = OFF;
  blu1 = ON;
  red2 = OFF;
  grn2 = OFF;
  blu2 = ON;
  ledcWrite(ledChannelR1, OFF);
  ledcWrite(ledChannelG1, OFF);
  ledcWrite(ledChannelB1, ON);
  ledcWrite(ledChannelR2, OFF);
  ledcWrite(ledChannelG2, OFF);
  ledcWrite(ledChannelB2, ON);
}
void white() {
  red1 = ON;
  grn1 = ON;
  blu1 = ON;
  red2 = ON;
  grn2 = ON;
  blu2 = ON;
  ledcWrite(ledChannelR1, ON);
  ledcWrite(ledChannelG1, ON);
  ledcWrite(ledChannelB1, ON);
  ledcWrite(ledChannelR2, ON);
  ledcWrite(ledChannelG2, ON);
  ledcWrite(ledChannelB2, ON);
}

void yellow() {
  red1 = ON;
  grn1 = ON;
  blu1 = OFF;
  red2 = ON;
  grn2 = ON;
  blu2 = OFF;
  ledcWrite(ledChannelR1, ON);
  ledcWrite(ledChannelG1, ON);
  ledcWrite(ledChannelB1, OFF);
  ledcWrite(ledChannelR2, ON);
  ledcWrite(ledChannelG2, ON);
  ledcWrite(ledChannelB2, OFF);
}

void cyan() {
  red1 = OFF;
  grn1 = ON;
  blu1 = ON;
  red2 = OFF;
  grn2 = ON;
  blu2 = ON;
  ledcWrite(ledChannelR1, OFF);
  ledcWrite(ledChannelG1, ON);
  ledcWrite(ledChannelB1, ON);
  ledcWrite(ledChannelR2, OFF);
  ledcWrite(ledChannelG2, ON);
  ledcWrite(ledChannelB2, ON);
}

void purple() {
  red1 = ON;
  grn1 = OFF;
  blu1 = ON;
  red2 = ON;
  grn2 = OFF;
  blu2 = ON;
  ledcWrite(ledChannelR1, ON);
  ledcWrite(ledChannelG1, OFF);
  ledcWrite(ledChannelB1, ON);
  ledcWrite(ledChannelR2, ON);
  ledcWrite(ledChannelG2, OFF);
  ledcWrite(ledChannelB2, ON);
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
  fadeUp(GREEN_LED1, GREEN_LED2, RED_LED1, RED_LED2, BLUE_LED1, BLUE_LED2);    //red to yellow
  fadeDown(RED_LED1, RED_LED2, GREEN_LED1, GREEN_LED2, BLUE_LED1, BLUE_LED2);  //yellow to green
  fadeUp(BLUE_LED1, BLUE_LED2, GREEN_LED1, GREEN_LED2, RED_LED1, RED_LED2);    //green to cyan
  fadeDown(GREEN_LED1, GREEN_LED2, BLUE_LED1, BLUE_LED2, RED_LED1, RED_LED2);  //cyan to blue
  fadeUp(RED_LED1, RED_LED2, BLUE_LED1, BLUE_LED2, GREEN_LED1, GREEN_LED2);    //blue to purple
  fadeDown(BLUE_LED1, BLUE_LED2, RED_LED1, RED_LED2, GREEN_LED1, GREEN_LED2);  //purple to red
}
void rainbow2() {
  button.tick();
  solidPulsing(1, 0, 0, 1, 0, 0, 5);  //red
  button.tick();
  solidPulsing(1, 1, 0, 1, 1, 0, 5);  //yellow
  button.tick();
  solidPulsing(0, 1, 0, 0, 1, 0, 5);  //green
  button.tick();
  solidPulsing(0, 1, 1, 0, 1, 1, 5);  //cyan?
  button.tick();
  solidPulsing(0, 0, 1, 0, 0, 1, 5);  //blue
  button.tick();
  solidPulsing(1, 0, 1, 1, 0, 1, 5);  //purple
  button.tick();
}

void pawlice() {
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
        led1State = ON;
        led2State = OFF;
      } else {
        led1State = OFF;
        led2State = ON;
      }
      ledcWrite(ledChannelR1, led1State);
      ledcWrite(ledChannelB2, led2State);
    }
  }
}

void doTheAnimation() {
  switch (mode) {
    case 0:
      red();
      break;
    case 1:
      yellow();
      break;
    case 2:
      green();
      break;
    case 3:
      cyan();
      break;
    case 4:
      blue();
      break;
    case 5:
      purple();
      break;
    case 6:
      white();
      break;
    case 7:
      pawlice();
      break;
    case 8:
      //red
      solidPulsing(1, 0, 0, 1, 0, 0, 10);
      break;
    case 9:
      //yellow
      solidPulsing(1, 1, 0, 1, 1, 0, 10);
      break;
    case 10:
      //green
      solidPulsing(0, 1, 0, 0, 1, 0, 10);
      break;
    case 11:
      //cyan
      solidPulsing(0, 1, 1, 0, 1, 1, 10);
      break;
    case 12:
      //blue
      solidPulsing(0, 0, 1, 0, 0, 1, 10);
      break;
    case 13:
      //purple
      solidPulsing(1, 0, 1, 1, 0, 1, 10);
      break;
    case 14:
      //white
      solidPulsing(1, 1, 1, 1, 1, 1, 10);
      break;
    case 69:
      // saved setting
      defaultMode();
      break;
    default:
      red();
      break;
  }
}

void defaultMode() {
  readPreferences();
  mode = anim;
}

/*
void doTheAnimation() {
  switch (anim) {
    case SOLID:
      solid(red1, grn1, blu1, red2, grn2, blu2);
      break;
    case PULSING:
      //solidPulsing();
      break;
    case RAINBOW:
      rainbow();
      break;
    default:
      allOff();
      break;
  }
}

*/