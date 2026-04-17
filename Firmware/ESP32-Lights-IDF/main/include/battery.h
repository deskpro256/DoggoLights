#pragma once

// Voltage divider between battery+ and ADC pin.
// BAT+ -> R_TOP(820K) -> ADC -> R_BOT(2M) -> GND
#define BAT_R_TOP_KOHM 820
#define BAT_R_BOT_KOHM 2000
#define BAT_MV_FULL    4200
#define BAT_MV_EMPTY   3000

// Software calibration for already-built boards.
// V_cal = (V_raw * BAT_CAL_GAIN_NUM / BAT_CAL_GAIN_DEN) + BAT_CAL_OFFSET_MV
// Default gain compensates the observed ~7.7% low reading.
#define BAT_CAL_GAIN_NUM   1077
#define BAT_CAL_GAIN_DEN   1000
#define BAT_CAL_OFFSET_MV  0

void battery_init(void);
int  battery_read_adc_mv(void);   // raw voltage at the ADC pin (after divider)
int  battery_read_millivolts(void); // actual battery voltage (divider compensated)
int battery_read_percent(void);
