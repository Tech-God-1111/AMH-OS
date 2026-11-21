#include "ui_service.h"
#include <stdint.h>
#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "hal_board.h"
#include "kernel.h"
#include "storage_service.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
static const char *TAG = "AMH-UI";
extern amh_hw_info_t g_amh_hw;

static esp_err_t root_handler(httpd_req_t *req)
{
    size_t size = (size_t)(index_html_end - index_html_start);
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, (const char *)index_html_start, size);
}

static esp_err_t status_handler(httpd_req_t *req)
{
    char body[512];
    int n = snprintf(body, sizeof(body),
        "{\"name\":\"AMH-OS\",\"version\":\"3.0.0\",\"chip\":\"%s\","
        "\"cores\":%u,\"free_heap\":%lu,\"uptime_ms\":%llu,"
        "\"flash_bytes\":%lu,\"psram_bytes\":%lu,\"internal_free\":%lu,"
        "\"safe_mode\":%s,\"storage_ready\":%s,\"kernel\":true}",
        g_amh_hw.chip_model, (unsigned)g_amh_hw.cores,
        (unsigned long)esp_get_free_heap_size(),
        (unsigned long long)(esp_timer_get_time() / 1000ULL),
        (unsigned long)g_amh_hw.flash_size, (unsigned long)g_amh_hw.psram_size,
        (unsigned long)g_amh_hw.internal_free,
        amh_kernel_safe_mode() ? "true" : "false",
        amh_storage_ready() ? "true" : "false");
    if (n < 0 || n >= sizeof(body)) return ESP_FAIL;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, body, n);
}

esp_err_t amh_ui_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 16;
    httpd_handle_t server = NULL;
    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK) return err;

    const httpd_uri_t root = {.uri="/", .method=HTTP_GET, .handler=root_handler};
    const httpd_uri_t status = {.uri="/api/status", .method=HTTP_GET, .handler=status_handler};
    err = httpd_register_uri_handler(server, &root);
    if (err != ESP_OK) return err;
    err = httpd_register_uri_handler(server, &status);
    if (err != ESP_OK) return err;
    err = amh_storage_register_handlers(server);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG, "Desktop web server ready at http://192.168.4.1");
    return ESP_OK;
}
