#include "battery.h"

#include "app_config.h"
#include "driver/adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;
static bool s_has_cali;

typedef struct {
    int mv;
    int pct;
} batt_curve_point_t;

// Non-linear Li-ion state-of-charge curve (resting-voltage oriented).
// This gives a more realistic percentage than a simple 3.0V..4.2V linear map.
static const batt_curve_point_t k_batt_curve[] = {
    {4200, 100},
    {4150, 95},
    {4100, 90},
    {4050, 85},
    {4000, 80},
    {3950, 74},
    {3900, 68},
    {3850, 62},
    {3800, 55},
    {3750, 48},
    {3700, 40},
    {3650, 32},
    {3600, 24},
    {3550, 17},
    {3500, 12},
    {3450, 8},
    {3400, 5},
    {3350, 3},
    {3300, 2},
    {3200, 1},
    {3000, 0},
};

static int battery_percent_from_curve(int mv) {
    const int n = (int)(sizeof(k_batt_curve) / sizeof(k_batt_curve[0]));

    if (mv >= k_batt_curve[0].mv) {
        return k_batt_curve[0].pct;
    }
    if (mv <= k_batt_curve[n - 1].mv) {
        return k_batt_curve[n - 1].pct;
    }

    for (int i = 0; i < (n - 1); i++) {
        const batt_curve_point_t hi = k_batt_curve[i];
        const batt_curve_point_t lo = k_batt_curve[i + 1];
        if (mv <= hi.mv && mv >= lo.mv) {
            const int dv = hi.mv - lo.mv;
            if (dv <= 0) {
                return lo.pct;
            }
            const int dp = hi.pct - lo.pct;
            const int num = (mv - lo.mv) * dp;
            int pct = lo.pct + (num + (dv / 2)) / dv;
            if (pct < 0) pct = 0;
            if (pct > 100) pct = 100;
            return pct;
        }
    }

    return 0;
}

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
    int mv = adc_mv * (BAT_R_TOP_KOHM + BAT_R_BOT_KOHM) / BAT_R_BOT_KOHM;

    // Apply software calibration for board-level ADC/divider tolerance.
    mv = (mv * BAT_CAL_GAIN_NUM + (BAT_CAL_GAIN_DEN / 2)) / BAT_CAL_GAIN_DEN;
    mv += BAT_CAL_OFFSET_MV;

    if (mv < 0) {
        mv = 0;
    }
    return mv;
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
    return battery_percent_from_curve(mv);
}
