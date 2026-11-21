#include "kernel.h"
#include <stdio.h>
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BOOT_MAGIC 0xA3B03A00u
#define SAFE_MODE_THRESHOLD 3u

static const char *TAG = "AMH-KERNEL";
static RTC_NOINIT_ATTR uint32_t s_magic;
static RTC_NOINIT_ATTR uint32_t s_failed_boots;
static amh_boot_result_t s_results[AMH_MAX_BOOT_STAGES];
static size_t s_result_count;
static bool s_safe_mode;
static uint32_t s_boot_duration_ms;

bool amh_kernel_safe_mode(void) { return s_safe_mode; }
uint32_t amh_kernel_boot_duration_ms(void) { return s_boot_duration_ms; }

size_t amh_kernel_get_results(const amh_boot_result_t **results)
{
    if (results) *results = s_results;
    return s_result_count;
}

static void boot_counter_begin(void)
{
    if (s_magic != BOOT_MAGIC) {
        s_magic = BOOT_MAGIC;
        s_failed_boots = 0;
    }
    s_failed_boots++;
    s_safe_mode = s_failed_boots > SAFE_MODE_THRESHOLD;
}

esp_err_t amh_kernel_boot(const amh_boot_stage_t *stages, size_t count)
{
    if (!stages || count == 0 || count > AMH_MAX_BOOT_STAGES)
        return ESP_ERR_INVALID_ARG;

    const int64_t boot_start = esp_timer_get_time();
    boot_counter_begin();
    s_result_count = 0;

    ESP_LOGI(TAG, "%s %s boot sequence", AMH_OS_NAME, AMH_OS_VERSION);
    if (s_safe_mode)
        ESP_LOGW(TAG, "Safe mode active after %lu unsuccessful boot attempts",
                 (unsigned long)(s_failed_boots - 1));

    for (size_t i = 0; i < count; ++i) {
        const amh_boot_stage_t *stage = &stages[i];
        amh_boot_result_t *result = &s_results[s_result_count++];
        result->name = stage->name;
        result->error = ESP_OK;
        result->duration_us = 0;
        result->skipped = false;

        if (s_safe_mode && stage->skip_in_safe_mode) {
            result->skipped = true;
            ESP_LOGW(TAG, "Stage %s skipped in safe mode", stage->name);
            continue;
        }

        int64_t start = esp_timer_get_time();
        result->error = stage->init ? stage->init() : ESP_OK;
        result->duration_us = (uint32_t)(esp_timer_get_time() - start);
        ESP_LOGI(TAG, "[%u/%u] %s: %s (%lu us)",
                 (unsigned)(i + 1), (unsigned)count, stage->name,
                 result->error == ESP_OK ? "OK" : esp_err_to_name(result->error),
                 (unsigned long)result->duration_us);

        if (result->error != ESP_OK && stage->critical) {
            if (!s_safe_mode) {
                ESP_LOGE(TAG, "Critical stage failed; one controlled restart will advance recovery counter");
                vTaskDelay(pdMS_TO_TICKS(1500));
                esp_restart();
            }
            ESP_LOGE(TAG, "Critical stage still failing in safe mode; restart loop prevented");
            s_boot_duration_ms = (uint32_t)((esp_timer_get_time() - boot_start) / 1000);
            return result->error;
        }
    }

    if (!s_safe_mode) s_failed_boots = 0;
    s_boot_duration_ms = (uint32_t)((esp_timer_get_time() - boot_start) / 1000);
    ESP_LOGI(TAG, "Boot sequence completed in %lu ms%s",
             (unsigned long)s_boot_duration_ms, s_safe_mode ? " (safe mode)" : "");
    return ESP_OK;
}
