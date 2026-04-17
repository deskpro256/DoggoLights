#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"

esp_err_t ota_pull_start(const char *url);

// Resolve OTA target using an optional latest.txt manifest and compare versions.
// If a newer version is available, out_has_update=true and out_bin_url is set.
// If same/older version, out_has_update=false and OTA should be skipped.
esp_err_t ota_pull_resolve_target(
	const char *input_url,
	const char *current_version,
	char *out_bin_url,
	size_t out_bin_url_len,
	char *out_new_version,
	size_t out_new_version_len,
	bool *out_has_update
);
