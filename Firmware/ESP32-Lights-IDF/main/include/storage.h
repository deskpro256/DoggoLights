#pragma once

#include <stdbool.h>

#include "app_state.h"

void storage_load_or_default(device_config_t *cfg);
void storage_save_config(const device_config_t *cfg);
void storage_factory_reset(device_config_t *cfg_out);
bool storage_load_mfg_passed(void);
void storage_save_mfg_passed(bool passed);
