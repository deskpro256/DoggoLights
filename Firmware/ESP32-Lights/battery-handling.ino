int readBatteryValue() {
  analogReadResolution(12);  // 12-bit ADC resolution for battery voltage read
  sensorValue = analogRead(BAT_ADC);
  //Serial.print("Analog value: ");
  //Serial.println(String(sensorValue));
  voltage = sensorValue * (4.2 / 4095.0);
  //Serial.print("Batt voltage: ");
  //Serial.println(String(voltage));
  batteryPercentage = map(voltage, 3.0, 4.2, 0, 100);
  batteryPercentage = constrain(batteryPercentage, 0, 100);
  //Serial.print("Batt %: ");
  //Serial.println(String(batteryPercentage));
  
  return batteryPercentage;
}

/*
75%-100% = Green
50%-74%  = Yellow
25%-49%  = Yellow-purple
0%-24%   = Red
*/
void blinkBateryPercentColor(int battPercent) {
  //Serial.print("Battery %: ");
  //Serial.println(String(battPercent));
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