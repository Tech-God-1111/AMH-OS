#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_system.h"

typedef struct {
    char chip_model[16];
    uint8_t chip_revision;
    uint8_t cores;
    bool has_wifi;
    bool has_ble;
    uint32_t cpu_mhz;
    uint32_t flash_size;
    bool psram_present;
    uint32_t psram_size;
    size_t internal_total;
    size_t internal_free;
    uint8_t mac[6];
    esp_reset_reason_t reset_reason;
    const char *idf_version;
} amh_hw_info_t;

esp_err_t amh_hal_detect(amh_hw_info_t *out);
const char *amh_hal_reset_reason(esp_reset_reason_t reason);
