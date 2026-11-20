#include "amh_ui.h"

#include <stdio.h>

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_timer.h"

static const char *TAG = "AMH-UI";

static const char INDEX_HTML[] =
"<!doctype html><html lang='en'><head><meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<meta name='theme-color' content='#0a1220'><title>AMH-OS</title>"
"<style>"
":root{color-scheme:dark;--bg:#08111f;--panel:#111f32;--line:#293c55;--mint:#73e2c2;--muted:#91a5bd;--white:#edf5ff}"
"*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at 15% 0%,#1a3850,transparent 42%),var(--bg);color:var(--white);font:15px/1.5 'Segoe UI',Arial,sans-serif;min-height:100vh}"
".bar{height:68px;border-bottom:1px solid #ffffff18;display:flex;align-items:center;justify-content:space-between;padding:0 5vw;background:#07101dcc}"
".brand{display:flex;align-items:center;gap:12px;font-weight:800;letter-spacing:.15em}.logo{width:38px;height:38px;border-radius:12px;background:var(--mint);color:#09251e;display:grid;place-items:center;font-size:22px}"
".online{font-size:11px;color:var(--mint);letter-spacing:.12em}.online:before{content:'';display:inline-block;width:7px;height:7px;border-radius:50%;background:var(--mint);margin-right:9px;box-shadow:0 0 12px var(--mint)}"
"main{max-width:1120px;margin:auto;padding:52px 24px}.tag{color:var(--mint);font-size:10px;font-weight:800;letter-spacing:.2em}h1{font-size:clamp(38px,6vw,66px);line-height:1.02;letter-spacing:-.05em;margin:12px 0}h1 span{color:#8faac7}.intro{color:var(--muted);max-width:580px;margin:18px 0 32px}"
".row{display:grid;grid-template-columns:repeat(3,1fr);gap:14px}.card{background:linear-gradient(145deg,#172941,#101b2c);border:1px solid var(--line);border-radius:16px;padding:20px}.card small{color:var(--muted);font-size:10px;letter-spacing:.14em;font-weight:800}.value{font-size:28px;font-weight:750;margin:8px 0 2px}.unit{color:var(--muted);font-size:11px}"
".section{margin-top:24px;display:grid;grid-template-columns:1.2fr .8fr;gap:14px}.panel{background:#101c2d;border:1px solid var(--line);border-radius:16px;padding:22px}.panel h2{font-size:17px;margin:0 0 16px}.line{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #ffffff10;font-size:12px;gap:15px}.line:last-child{border:0}.line span:first-child{color:var(--muted)}.line span:last-child{text-align:right;overflow-wrap:anywhere}"
".apps{display:grid;grid-template-columns:repeat(2,1fr);gap:10px}.app{border:1px solid #ffffff12;background:#0b1727;border-radius:12px;padding:15px;color:var(--white);font-size:12px}.app b{display:block;color:var(--mint);font-size:20px;margin-bottom:7px}footer{margin-top:28px;color:#7388a2;font-size:10px;display:flex;justify-content:space-between;gap:10px}"
"@media(max-width:700px){main{padding:35px 16px}.row{grid-template-columns:1fr}.section{grid-template-columns:1fr}.bar{padding:0 16px}footer{flex-direction:column}}"
"</style></head><body><header class='bar'><div class='brand'><span class='logo'>A</span> AMH-OS</div><div class='online'>DEVICE ONLINE</div></header>"
"<main><div class='tag'>NATIVE ESP32-S3 EDITION / 0.1.0</div><h1>Your system.<br><span>Your workspace.</span></h1>"
"<p class='intro'>This interface is being served directly by the AMH-OS firmware running on your ESP32-S3. The board is the system host; this browser is its display.</p>"
"<section class='row'><article class='card'><small>FREE HEAP</small><div class='value' id='heap'>---</div><div class='unit'>bytes available</div></article>"
"<article class='card'><small>CPU CORES</small><div class='value' id='cores'>--</div><div class='unit'>ESP32-S3 processor</div></article>"
"<article class='card'><small>UPTIME</small><div class='value' id='uptime'>---</div><div class='unit'>seconds since boot</div></article></section>"
"<section class='section'><article class='panel'><h2>System information</h2>"
"<div class='line'><span>Operating system</span><span>AMH-OS Native</span></div><div class='line'><span>Firmware</span><span>0.1.0</span></div>"
"<div class='line'><span>Chip</span><span id='chip'>ESP32-S3</span></div><div class='line'><span>Wi-Fi network</span><span>AMH-OS-SETUP</span></div>"
"<div class='line'><span>Control address</span><span>192.168.4.1</span></div><div class='line'><span>Kernel</span><span id='kernel'>Running</span></div></article>"
"<article class='panel'><h2>Workspace</h2><div class='apps'><div class='app'><b>▣</b>System</div><div class='app'><b>⌘</b>Terminal <small>COMING NEXT</small></div>"
"<div class='app'><b>▤</b>Files <small>COMING NEXT</small></div><div class='app'><b>⚙</b>Settings <small>COMING NEXT</small></div></div></article></section>"
"<footer><span>AMH-OS · NATIVE FIRMWARE</span><span>RUNNING LOCALLY ON YOUR ESP32-S3</span></footer></main>"
"<script>async function update(){try{let r=await fetch('/api/status',{cache:'no-store'});let d=await r.json();document.getElementById('heap').textContent=(d.free_heap||0).toLocaleString();document.getElementById('cores').textContent=d.cores||'--';document.getElementById('uptime').textContent=Math.floor((d.uptime_ms||0)/1000);document.getElementById('chip').textContent=d.chip||'ESP32-S3';document.getElementById('kernel').textContent=d.kernel?'Running':'Starting'}catch(e){document.querySelector('.online').textContent='RECONNECTING' }}update();setInterval(update,2500);</script>"
"</body></html>";

static esp_err_t root_handler(httpd_req_t *request)
{
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    return httpd_resp_send(request, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
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

    ESP_LOGI(TAG, "AMH-OS graphical interface is online at http://192.168.4.1");
    return ESP_OK;
}
