#include "wifi_service.h"
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

static const char *TAG = "AMH-WIFI";
#define AMH_AP_SSID "AMH-OS-SETUP"
#define AMH_AP_PASSWORD "amhsetup"

esp_err_t amh_wifi_start(void)
{
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    if (!esp_netif_create_default_wifi_ap()) return ESP_FAIL;

    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&init);
    if (err != ESP_OK && err != ESP_ERR_WIFI_INIT_STATE) return err;

    wifi_config_t config = {0};
    memcpy(config.ap.ssid, AMH_AP_SSID, sizeof(AMH_AP_SSID) - 1);
    memcpy(config.ap.password, AMH_AP_PASSWORD, sizeof(AMH_AP_PASSWORD) - 1);
    config.ap.ssid_len = sizeof(AMH_AP_SSID) - 1;
    config.ap.channel = 1;
    config.ap.max_connection = 4;
    config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    config.ap.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
    err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STOPPED) return err;

    ESP_LOGI(TAG, "AP ready: %s; browse to http://192.168.4.1", AMH_AP_SSID);
    ESP_LOGW(TAG, "Development AP credentials must be changed before public deployment");
    return ESP_OK;
}
