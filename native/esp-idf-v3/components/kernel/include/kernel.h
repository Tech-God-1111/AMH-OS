#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#define AMH_OS_NAME "AMH-OS"
#define AMH_OS_VERSION "3.0.0"
#define AMH_MAX_BOOT_STAGES 16

typedef esp_err_t (*amh_stage_fn)(void);

typedef struct {
    const char *name;
    amh_stage_fn init;
    bool critical;
    bool skip_in_safe_mode;
} amh_boot_stage_t;

typedef struct {
    const char *name;
    esp_err_t error;
    uint32_t duration_us;
    bool skipped;
} amh_boot_result_t;

esp_err_t amh_kernel_boot(const amh_boot_stage_t *stages, size_t count);
bool amh_kernel_safe_mode(void);
uint32_t amh_kernel_boot_duration_ms(void);
size_t amh_kernel_get_results(const amh_boot_result_t **results);
