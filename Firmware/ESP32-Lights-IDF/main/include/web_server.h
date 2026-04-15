#pragma once

#include <stdbool.h>

void        web_server_init(void);
void        web_server_start_ap(void);
void        web_server_stop_ap(void);
bool        web_server_ap_running(void);
const char *web_server_get_runtime_ap_ssid(void);
