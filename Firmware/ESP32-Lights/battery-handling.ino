int readBatteryValue() {
  adcValue = analogRead(BAT_ADC);
  voltage = (adcValue / 4095.0) * 3.3;
  if (voltage > 4.20) {
    voltage = 4.20;
  } else if (voltage < 2.90) {
    voltage = 2.90;
  }
  voltage = 4.20;
  return map(voltage, 2.90, 4.20, 0, 100);
}

/*
75%-100% = Green
50%-74%  = Yellow
25%-49%  = Yellow-purple
0%-24%   = Red
*/
void blinkBateryPercentColor(int battPercent) {
  Serial.print("Battery percent:");
  Serial.println(String(battPercent));
  if (battPercent <= 100 && battPercent >= 75) {
    //Green
    blink(OFF, ON, OFF, OFF, ON, OFF);
  } else if (battPercent <= 74 && battPercent >= 50) {
    //Yellow
    blink(ON, ON, OFF, ON, ON, OFF);
  } else if (battPercent <= 49 && battPercent >= 25) {
    //Yellow-purple
    blink(ON, ON, OFF, ON, OFF, ON);
  } else if (battPercent <= 24 && battPercent >= 0) {
    //Red
    blink(ON, OFF, OFF, ON, OFF, OFF);
  }
}