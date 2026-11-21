#include <stdio.h>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_board.h"
#include "kernel.h"
#include "wifi_service.h"
#include "ui_service.h"
#include "storage_service.h"

static const char *TAG = "AMH-BOOT";
amh_hw_info_t g_amh_hw;

static esp_err_t stage_hardware(void)
{
    esp_err_t err = amh_hal_detect(&g_amh_hw);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG, "Chip %s rev %u, %u cores, %u MHz",
             g_amh_hw.chip_model, g_amh_hw.chip_revision,
             g_amh_hw.cores, (unsigned)g_amh_hw.cpu_mhz);
    ESP_LOGI(TAG, "Flash %lu bytes; PSRAM %s (%lu bytes)",
             (unsigned long)g_amh_hw.flash_size,
             g_amh_hw.psram_present ? "initialized" : "not initialized",
             (unsigned long)g_amh_hw.psram_size);
    ESP_LOGI(TAG, "Internal heap free %lu / %lu bytes",
             (unsigned long)g_amh_hw.internal_free,
             (unsigned long)g_amh_hw.internal_total);
    ESP_LOGI(TAG, "Reset reason: %s; ESP-IDF: %s",
             amh_hal_reset_reason(g_amh_hw.reset_reason), g_amh_hw.idf_version);
    return ESP_OK;
}

static esp_err_t stage_memory(void)
{
    return g_amh_hw.internal_free >= 48 * 1024 ? ESP_OK : ESP_ERR_NO_MEM;
}

static esp_err_t stage_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGE(TAG, "NVS needs recovery (%s); refusing automatic erase to preserve data",
                 esp_err_to_name(err));
    }
    return err;
}

static esp_err_t stage_storage(void) { return amh_storage_mount(); }
static esp_err_t stage_wifi(void) { return amh_wifi_start(); }
static esp_err_t stage_ui(void) { return amh_ui_start(); }

static const amh_boot_stage_t BOOT_STAGES[] = {
    {"hardware-detect", stage_hardware, true, false},
    {"memory-check", stage_memory, true, false},
    {"nvs-init", stage_nvs, true, true},
    {"storage", stage_storage, false, true},
    {"wifi-ap", stage_wifi, true, true},
    {"desktop-http", stage_ui, true, true}
};

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "          AMH-OS NATIVE 3.0.0");
    ESP_LOGI(TAG, "     ESP32-S3 desktop + kernel services");
    ESP_LOGI(TAG, "========================================");

    esp_err_t err = amh_kernel_boot(BOOT_STAGES,
        sizeof(BOOT_STAGES) / sizeof(BOOT_STAGES[0]));

    if (err == ESP_OK) {
        const esp_partition_t *running = esp_ota_get_running_partition();
        if (running) {
            esp_err_t ota_err = esp_ota_mark_app_valid_cancel_rollback();
            if (ota_err != ESP_OK && ota_err != ESP_ERR_NOT_SUPPORTED && ota_err != ESP_ERR_NOT_FOUND)
                ESP_LOGW(TAG, "OTA validation note: %s", esp_err_to_name(ota_err));
        }
        ESP_LOGI(TAG, "AMH-OS 3.0 desktop is online");
    } else {
        ESP_LOGE(TAG, "Boot did not complete (%s). Recovery loop active; no automatic data erase.",
                 esp_err_to_name(err));
    }

    for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
}
