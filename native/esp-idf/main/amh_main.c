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

    ESP_ERROR_CHECK(amh_kernel_start());

    /*
     * ESP-IDF Wi-Fi uses NVS for its persistent configuration.
     * Initialize the existing NVS partition before starting Wi-Fi.
     * Deliberately do not erase NVS automatically: preserving existing
     * device data is safer than silently clearing it on startup.
     */
    ESP_LOGI(TAG, "Initializing NVS for Wi-Fi...");
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s",
                 esp_err_to_name(nvs_err));
        ESP_LOGE(TAG, "Wi-Fi startup aborted; NVS was not erased.");
        return;
    }

    ESP_ERROR_CHECK(amh_wifi_start());
    ESP_ERROR_CHECK(amh_ui_start());

    ESP_LOGI(TAG, "AMH-OS startup completed");
    ESP_LOGI(TAG, "Connect to Wi-Fi AMH-OS-SETUP (password: amhsetup)");
    ESP_LOGI(TAG, "Then open http://192.168.4.1");
}
