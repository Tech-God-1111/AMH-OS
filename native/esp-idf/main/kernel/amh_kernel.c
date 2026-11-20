#include "amh_kernel.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "AMH-KERNEL";

static void amh_monitor_task(void *argument)
{
    (void)argument;

    ESP_LOGI(TAG, "Kernel monitor task started");

    while (1) {
        ESP_LOGI(
            TAG,
            "Kernel alive | Free heap: %lu bytes",
            (unsigned long)esp_get_free_heap_size()
        );

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

esp_err_t amh_kernel_start(void)
{
    ESP_LOGI(TAG, "Initializing AMH-OS kernel foundation");

    BaseType_t result = xTaskCreate(
        amh_monitor_task,
        "amh_monitor",
        3072,
        NULL,
        5,
        NULL
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Could not create kernel monitor task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Kernel foundation initialized");
    return ESP_OK;
}
