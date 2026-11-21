#include "hal_board.h"
#include <stdio.h>
#include <string.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_mac.h"
#include "esp_psram.h"
#include "sdkconfig.h"

esp_err_t amh_hal_detect(amh_hw_info_t *out)
{
    if (out == NULL) return ESP_ERR_INVALID_ARG;
    memset(out, 0, sizeof(*out));

    esp_chip_info_t chip;
    esp_chip_info(&chip);
    snprintf(out->chip_model, sizeof(out->chip_model), "%s",
             chip.model == CHIP_ESP32S3 ? "ESP32-S3" : "ESP32-family");
    out->chip_revision = (uint8_t)chip.revision;
    out->cores = chip.cores;
    out->has_wifi = (chip.features & CHIP_FEATURE_WIFI_BGN) != 0;
    out->has_ble = (chip.features & CHIP_FEATURE_BLE) != 0;
    out->cpu_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;

    uint32_t flash_bytes = 0;
    if (esp_flash_get_size(NULL, &flash_bytes) == ESP_OK)
        out->flash_size = flash_bytes;

    out->psram_present = esp_psram_is_initialized();
    out->psram_size = out->psram_present ? (uint32_t)esp_psram_get_size() : 0;
    out->internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    out->internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    esp_read_mac(out->mac, ESP_MAC_WIFI_STA);
    out->reset_reason = esp_reset_reason();
    out->idf_version = esp_get_idf_version();
    return ESP_OK;
}

const char *amh_hal_reset_reason(esp_reset_reason_t r)
{
    switch (r) {
    case ESP_RST_POWERON: return "power-on";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt-watchdog";
    case ESP_RST_TASK_WDT: return "task-watchdog";
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_DEEPSLEEP: return "deep-sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_EXT: return "external";
    default: return "unknown";
    }
}
