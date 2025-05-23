#include "Moto_Ctrl_V5_esp32/lib/ota.h"
#include "Moto_Ctrl_V5_esp32/lib/connect_to_wifi.h"
#include "Moto_Ctrl_V5_esp32/lib/config.h"
#include "Moto_Ctrl_V5_esp32/lib/_params.h"
#include "Moto_Ctrl_V5_esp32/lib/dependencies.h"
#include "Moto_Ctrl_V5_esp32/lib/http_event_handler.h"

#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "OTA";
#define JSON_BUFFER_SIZE 2048
#define NVS_NAMESPACE "firmware"
#define NVS_KEY_VERSION "version"

char current_version[32] = {0};  // Holds version from NVS

// Store firmware version in NVS
void store_firmware_version_in_nvs(const char *version_str) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(nvs_handle, NVS_KEY_VERSION, version_str);
    if (err == ESP_OK) {
        nvs_commit(nvs_handle);
        ESP_LOGI(TAG, "Firmware version saved: %s", version_str);
    } else {
        ESP_LOGE(TAG, "Failed to save version: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
}

// Load firmware version from NVS into current_version
void load_current_version_from_nvs(void) {
    nvs_handle_t nvs_handle;
    size_t required_size = sizeof(current_version);

    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, NVS_KEY_VERSION, current_version, &required_size);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Loaded firmware version from NVS: %s", current_version);
        } else {
            ESP_LOGW(TAG, "No version found in NVS.");
            strcpy(current_version, "0.0.0"); // default fallback
        }
        nvs_close(nvs_handle);
    } else {
        ESP_LOGW(TAG, "Failed to open NVS for reading.");
        strcpy(current_version, "0.0.0"); // fallback
    }
}

void perform_ota(const char *firmware_url, const char *new_version) {
    connect_or_reconnect_to_wifi();

    esp_http_client_config_t http_config = {
        .url = firmware_url,
        .timeout_ms = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    ESP_LOGI(TAG, "Starting OTA from: %s", firmware_url);

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Update successful.");

        // 🔐 Store version before restart
        store_firmware_version_in_nvs(new_version);
        strcpy(current_version, new_version);

        ESP_LOGI(TAG, "Restarting now...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA Update failed: %s", esp_err_to_name(ret));
    }
}

void check_for_updates(void) {
    char *buffer = malloc(JSON_BUFFER_SIZE);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation failed");
        return;
    }

    esp_http_client_config_t config = {
        .url = SUPABASE_API_URL "/rest/v1/rpc/get_latest_moto_ctrl_firmware",
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
        .cert_pem = (const char *)_binary_superbase_root_crt_start,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = 2048,
        .buffer_size_tx = 1024,
        .event_handler = _http_event_handler,
        .user_data = buffer
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "apikey", SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Authorization", "bearer " SUPABASE_API_KEY);

    ESP_LOGI(TAG, "Connecting to URL: %s", config.url);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK && esp_http_client_get_status_code(client) == 200) {
        ESP_LOGI(TAG, "Response: %s", buffer);

        cJSON *root = cJSON_Parse(buffer);
        if (!root || !cJSON_IsArray(root)) {
            ESP_LOGE(TAG, "Invalid JSON response");
            free(buffer);
            cJSON_Delete(root);
            return;
        }

        cJSON *firstItem = cJSON_GetArrayItem(root, 0);
        cJSON *version = cJSON_GetObjectItem(firstItem, "version");
        cJSON *url = cJSON_GetObjectItem(firstItem, "firmware_url");

        if (!version || !url) {
            ESP_LOGE(TAG, "Missing version or firmware_url");
        } else {
            if (strcmp(current_version, version->valuestring) != 0) {
                ESP_LOGI(TAG, "New version available: %s", version->valuestring);
                perform_ota(url->valuestring, version->valuestring);
            } else {
                ESP_LOGI(TAG, "Already up-to-date: %s", current_version);
            }
        }

        free(buffer);
        cJSON_Delete(root);
    } else {
        ESP_LOGE(TAG, "Failed to connect to Supabase, error: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
