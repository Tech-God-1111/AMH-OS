#include "amh_ui.h"

#include <stdio.h>
#include <stdint.h>

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_timer.h"

/* The desktop is embedded into the firmware image at build time. */
extern const uint8_t index_html_start[] asm("_binary_web_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_web_index_html_end");

static const char *TAG = "AMH-UI";

static esp_err_t root_handler(httpd_req_t *request)
{
    const size_t page_size = (size_t)(index_html_end - index_html_start);
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    return httpd_resp_send(request, (const char *)index_html_start, page_size);
}

static esp_err_t status_handler(httpd_req_t *request)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    char response[256];
    int length = snprintf(
        response,
        sizeof(response),
        "{\"name\":\"AMH-OS\",\"version\":\"0.1.0\",\"chip\":\"ESP32-S3\","
        "\"cores\":%d,\"free_heap\":%lu,\"uptime_ms\":%llu,\"kernel\":true}",
        chip_info.cores,
        (unsigned long)esp_get_free_heap_size(),
        (unsigned long long)(esp_timer_get_time() / 1000ULL)
    );

    httpd_resp_set_type(request, "application/json");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    return httpd_resp_send(request, response, length);
}

esp_err_t amh_ui_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 8;

    httpd_handle_t server = NULL;
    esp_err_t result = httpd_start(&server, &config);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Could not start HTTP server: %s", esp_err_to_name(result));
        return result;
    }

    const httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL
    };
    const httpd_uri_t status_uri = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &status_uri));

    ESP_LOGI(TAG, "AMH-OS desktop is online at http://192.168.4.1");
    ESP_LOGI(TAG, "Desktop assets are embedded in the ESP32-S3 firmware");
    return ESP_OK;
}
