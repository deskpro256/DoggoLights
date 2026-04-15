#include "battery.h"

#include "app_config.h"
#include "driver/adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;
static bool s_has_cali;

// Average multiple ADC samples; the 100nF cap on the pin helps settling.
static int read_adc_mv_avg(void) {
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        int raw = 0;
        int mv  = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc, ADC_CHANNEL_3, &raw));
        if (s_has_cali) {
            adc_cali_raw_to_voltage(s_cali, raw, &mv);
        } else {
            // 12 dB attenuation: full-scale ~3100 mV
            mv = (raw * 3100) / 4095;
        }
        sum += mv;
    }
    return sum / 8;
}

// Voltage at the ADC pin, before divider compensation.
int battery_read_adc_mv(void) {
    return read_adc_mv_avg();
}

// Actual battery voltage, compensated for the resistor divider.
int battery_read_millivolts(void) {
    int adc_mv = read_adc_mv_avg();
    // V_bat = V_adc * (R_top + R_bot) / R_bot
    return adc_mv * (BAT_R_TOP_KOHM + BAT_R_BOT_KOHM) / BAT_R_BOT_KOHM;
}

void battery_init(void) {
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &s_adc));

    adc_oneshot_chan_cfg_t cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, ADC_CHANNEL_3, &cfg));

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL_3,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_cali) == ESP_OK) {
        s_has_cali = true;
    }
#endif
}

int battery_read_percent(void) {
    int mv = battery_read_millivolts();
    int percent = (mv - BAT_MV_EMPTY) * 100 / (BAT_MV_FULL - BAT_MV_EMPTY);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
}
