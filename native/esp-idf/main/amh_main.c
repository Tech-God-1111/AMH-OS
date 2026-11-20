#include "amh_kernel.h"
#include "amh_wifi.h"
#include "amh_ui.h"

#include "esp_chip_info.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

static const char *TAG = "AMH-BOOT";

void app_main(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "          AMH-OS 0.1.0");
    ESP_LOGI(TAG, "      Native ESP32-S3 system");
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "CPU cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Free heap: %lu bytes",
             (unsigned long)esp_get_free_heap_size());

    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs initialization; erasing NVS partition only");
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    ESP_ERROR_CHECK(amh_kernel_start());
    ESP_ERROR_CHECK(amh_wifi_start());
    ESP_ERROR_CHECK(amh_ui_start());

    ESP_LOGI(TAG, "AMH-OS startup completed");
    ESP_LOGI(TAG, "Connect to Wi-Fi AMH-OS-SETUP (password: amhsetup)");
    ESP_LOGI(TAG, "Then open http://192.168.4.1");
}
