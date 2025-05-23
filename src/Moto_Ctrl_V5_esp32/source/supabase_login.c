#include <Moto_Ctrl_V5_esp32/lib/dependencies.h>
#include <Moto_Ctrl_V5_esp32/lib/connect_to_wifi.h>
#include <Moto_Ctrl_V5_esp32/lib/config.h>
#include <Moto_Ctrl_V5_esp32/lib/_params.h>
#include <Moto_Ctrl_V5_esp32/lib/http_event_handler.h>


static const char *TAG = "SUPABASE_AUTH";
char jwt_access_token[1024] = {0};


int login_to_supabase() {

    char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    


    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return ESP_ERR_NO_MEM;
    }

    char post_data[512];
    snprintf(post_data, sizeof(post_data),
    "{\"email\":\"%s\", \"password\":\"%s\", \"grant_type\":\"password\"}",
    SUPABASE_AUTH_EMAIL, SUPABASE_AUTH_PASSWORD
    );


    esp_http_client_config_t config = {
        .url = SUPABASE_AUTH_URL,
        .cert_pem = (const char *) _binary_superbase_root_crt_start,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = 10000,
        .buffer_size = 4098,
        .buffer_size_tx = 1024,
        .event_handler = _http_event_handler,
        .user_data = buffer,  // <== Pass buffer to handler
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "apikey", SUPABASE_API_KEY);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Login request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int status = esp_http_client_get_status_code(client);
    if (status != 200) {
        ESP_LOGE(TAG, "Login failed, status code: %d", status);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

   
    ESP_LOGI(TAG, "Login response: %s", buffer);

    cJSON *root = cJSON_Parse(buffer);
    cJSON *access_token = cJSON_GetObjectItem(root, "access_token");

    if (access_token && cJSON_IsString(access_token)) {
        strncpy(jwt_access_token, access_token->valuestring, sizeof(jwt_access_token) - 1);
        ESP_LOGI(TAG, "✅ Logged in, token: %.30s...", jwt_access_token);  // Show beginning of token only
    } else {
        ESP_LOGE(TAG, "Failed to parse access token");
        cJSON_Delete(root);
        free(buffer);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    cJSON_Delete(root);
    free(buffer);
    esp_http_client_cleanup(client);
    return ESP_OK;
}