#include "storage_service.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"

#define AMH_FS_BASE "/amh"
#define AMH_FS_LABEL "storage"
#define AMH_FS_MAX_PATH 192
#define AMH_FS_MAX_BODY 4096

static const char *TAG = "AMH-STORAGE";
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static bool s_ready;

bool amh_storage_ready(void) { return s_ready; }

esp_err_t amh_storage_mount(void)
{
    const esp_vfs_fat_mount_config_t config = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 4096
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(
        AMH_FS_BASE, AMH_FS_LABEL, &config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Storage not mounted (%s). No automatic formatting was performed.",
                 esp_err_to_name(err));
        return err;
    }
    s_ready = true;
    ESP_LOGI(TAG, "FAT filesystem mounted at %s", AMH_FS_BASE);
    return ESP_OK;
}

static bool get_path(httpd_req_t *req, char *out, size_t out_size)
{
    char query[256];
    char relative[AMH_FS_MAX_PATH];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
        return false;
    if (httpd_query_key_value(query, "path", relative, sizeof(relative)) != ESP_OK)
        return false;
    if (relative[0] != '/') return false;
    if (strstr(relative, "..") || strchr(relative, '\\') || strchr(relative, ':'))
        return false;
    int n = snprintf(out, out_size, "%s%s", AMH_FS_BASE, relative);
    return n > 0 && (size_t)n < out_size;
}

static esp_err_t send_json(httpd_req_t *req, cJSON *root, const char *status)
{
    char *encoded = cJSON_PrintUnformatted(root);
    if (!encoded) return ESP_ERR_NO_MEM;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_status(req, status);
    esp_err_t err = httpd_resp_sendstr(req, encoded);
    free(encoded);
    return err;
}

static esp_err_t storage_unavailable(httpd_req_t *req)
{
    httpd_resp_set_status(req, "503 Service Unavailable");
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"error\":\"storage_unavailable\",\"hint\":\"Storage is not formatted or could not be mounted. No data was erased.\"}");
}

static esp_err_t list_handler(httpd_req_t *req)
{
    if (!s_ready) return storage_unavailable(req);
    char path[AMH_FS_MAX_PATH + sizeof(AMH_FS_BASE)];
    if (!get_path(req, path, sizeof(path))) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid path");
        return ESP_FAIL;
    }
    DIR *dir = opendir(path);
    if (!dir) {
        httpd_resp_send_err(req, errno == ENOENT ? HTTPD_404_NOT_FOUND : HTTPD_400_BAD_REQUEST,
                            "Directory unavailable");
        return ESP_FAIL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *items = cJSON_AddArrayToObject(root, "items");
    if (!root || !items) {
        closedir(dir);
        cJSON_Delete(root);
        return ESP_ERR_NO_MEM;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        char full[AMH_FS_MAX_PATH + sizeof(AMH_FS_BASE)];
        struct stat st;
        int n = snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);
        if (n <= 0 || (size_t)n >= sizeof(full) || stat(full, &st) != 0) continue;
        cJSON *item = cJSON_CreateObject();
        if (!item) continue;
        cJSON_AddStringToObject(item, "name", entry->d_name);
        cJSON_AddBoolToObject(item, "directory", S_ISDIR(st.st_mode));
        cJSON_AddNumberToObject(item, "size", (double)st.st_size);
        cJSON_AddItemToArray(items, item);
    }
    closedir(dir);
    cJSON_AddStringToObject(root, "path", req->uri);
    esp_err_t err = send_json(req, root, "200 OK");
    cJSON_Delete(root);
    return err;
}

static esp_err_t file_get_handler(httpd_req_t *req)
{
    if (!s_ready) return storage_unavailable(req);
    char path[AMH_FS_MAX_PATH + sizeof(AMH_FS_BASE)];
    if (!get_path(req, path, sizeof(path))) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid path");
        return ESP_FAIL;
    }
    FILE *file = fopen(path, "rb");
    if (!file) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    char buffer[1024];
    size_t count;
    while ((count = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (httpd_resp_send_chunk(req, buffer, count) != ESP_OK) {
            fclose(file);
            return ESP_FAIL;
        }
    }
    fclose(file);
    return httpd_resp_send_chunk(req, NULL, 0);
}

static esp_err_t receive_json(httpd_req_t *req, cJSON **out)
{
    if (req->content_len <= 0 || req->content_len > AMH_FS_MAX_BODY)
        return ESP_ERR_INVALID_SIZE;
    char *body = calloc(1, req->content_len + 1);
    if (!body) return ESP_ERR_NO_MEM;
    size_t received_total = 0;
    while (received_total < (size_t)req->content_len) {
        int received = httpd_req_recv(req, body + received_total,
                                      req->content_len - received_total);
        if (received <= 0) {
            free(body);
            return ESP_FAIL;
        }
        received_total += (size_t)received;
    }
    *out = cJSON_Parse(body);
    free(body);
    return *out ? ESP_OK : ESP_ERR_INVALID_ARG;
}

static bool json_path(cJSON *root, char *out, size_t size)
{
    cJSON *path = cJSON_GetObjectItemCaseSensitive(root, "path");
    if (!cJSON_IsString(path) || !path->valuestring || path->valuestring[0] != '/')
        return false;
    const char *p = path->valuestring;
    if (strstr(p, "..") || strchr(p, '\\') || strchr(p, ':')) return false;
    int n = snprintf(out, size, "%s%s", AMH_FS_BASE, p);
    return n > 0 && (size_t)n < size;
}

static esp_err_t file_put_handler(httpd_req_t *req)
{
    if (!s_ready) return storage_unavailable(req);
    cJSON *root = NULL;
    esp_err_t err = receive_json(req, &root);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON body");
        return err;
    }
    char path[AMH_FS_MAX_PATH + sizeof(AMH_FS_BASE)];
    cJSON *content = cJSON_GetObjectItemCaseSensitive(root, "content");
    if (!json_path(root, path, sizeof(path)) || !cJSON_IsString(content)) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Expected path and string content");
        return ESP_ERR_INVALID_ARG;
    }
    FILE *file = fopen(path, "wb");
    if (!file) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Cannot create file");
        return ESP_FAIL;
    }
    size_t length = strlen(content->valuestring);
    size_t written = fwrite(content->valuestring, 1, length, file);
    int close_result = fclose(file);
    bool ok = written == length && close_result == 0;
    cJSON_Delete(root);
    if (!ok) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Write failed");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"saved\":true}");
}

static esp_err_t mkdir_handler(httpd_req_t *req)
{
    if (!s_ready) return storage_unavailable(req);
    cJSON *root = NULL;
    esp_err_t err = receive_json(req, &root);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON body");
        return err;
    }
    char path[AMH_FS_MAX_PATH + sizeof(AMH_FS_BASE)];
    bool valid = json_path(root, path, sizeof(path));
    cJSON_Delete(root);
    if (!valid || mkdir(path, 0775) != 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Cannot create directory");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"created\":true}");
}

static esp_err_t format_handler(httpd_req_t *req)
{
    cJSON *root = NULL;
    esp_err_t err = receive_json(req, &root);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Confirmation required");
        return err;
    }
    cJSON *confirm = cJSON_GetObjectItemCaseSensitive(root, "confirm");
    bool approved = cJSON_IsString(confirm) &&
        strcmp(confirm->valuestring, "FORMAT AMH-OS STORAGE") == 0;
    cJSON_Delete(root);
    if (!approved) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Exact format confirmation required");
        return ESP_ERR_INVALID_ARG;
    }

    if (s_ready) {
        err = esp_vfs_fat_spiflash_unmount_rw_wl(AMH_FS_BASE, s_wl_handle);
        if (err != ESP_OK) return err;
        s_ready = false;
        s_wl_handle = WL_INVALID_HANDLE;
    }

    err = esp_vfs_fat_spiflash_format_rw_wl(AMH_FS_BASE, AMH_FS_LABEL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Explicit storage format failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Format failed");
        return err;
    }
    err = amh_storage_mount();
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Formatted, but remount failed");
        return err;
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\\"formatted\\":true}");
}

static esp_err_t delete_handler(httpd_req_t *req)
{
    if (!s_ready) return storage_unavailable(req);
    char path[AMH_FS_MAX_PATH + sizeof(AMH_FS_BASE)];
    if (!get_path(req, path, sizeof(path)) || !strcmp(path, AMH_FS_BASE)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid path");
        return ESP_FAIL;
    }
    struct stat st;
    if (stat(path, &st) != 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Path not found");
        return ESP_FAIL;
    }
    int result = S_ISDIR(st.st_mode) ? rmdir(path) : unlink(path);
    if (result != 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Delete failed; directory may not be empty");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"deleted\":true}");
}

esp_err_t amh_storage_register_handlers(httpd_handle_t server)
{
    const httpd_uri_t list = {.uri="/api/files", .method=HTTP_GET, .handler=list_handler};
    const httpd_uri_t get = {.uri="/api/file", .method=HTTP_GET, .handler=file_get_handler};
    const httpd_uri_t put = {.uri="/api/file", .method=HTTP_PUT, .handler=file_put_handler};
    const httpd_uri_t mkdir = {.uri="/api/mkdir", .method=HTTP_POST, .handler=mkdir_handler};
    const httpd_uri_t del = {.uri="/api/file", .method=HTTP_DELETE, .handler=delete_handler};
    const httpd_uri_t format = {.uri="/api/storage/format", .method=HTTP_POST, .handler=format_handler};
    esp_err_t err = httpd_register_uri_handler(server, &list);
    if (err != ESP_OK) return err;
    err = httpd_register_uri_handler(server, &get);
    if (err != ESP_OK) return err;
    err = httpd_register_uri_handler(server, &put);
    if (err != ESP_OK) return err;
    err = httpd_register_uri_handler(server, &mkdir);
    if (err != ESP_OK) return err;
    err = httpd_register_uri_handler(server, &del);
    if (err != ESP_OK) return err;
    return httpd_register_uri_handler(server, &format);
}
