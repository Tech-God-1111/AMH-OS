#pragma once
#include <stdbool.h>
#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t amh_storage_mount(void);
bool amh_storage_ready(void);
esp_err_t amh_storage_register_handlers(httpd_handle_t server);
