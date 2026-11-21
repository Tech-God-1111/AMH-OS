idf_component_register(
    SRCS "main.c" "wifi_service.c" "ui_service.c" "storage_service.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "web/index.html"
    REQUIRES kernel hal nvs_flash app_update esp_wifi esp_event esp_netif esp_http_server esp_timer fatfs wear_levelling json
)
