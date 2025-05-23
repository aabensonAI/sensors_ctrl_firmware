#include "Moto_Ctrl_V5_esp32/lib/dependencies.h"
#include "Moto_Ctrl_V5_esp32/lib/connect_to_wifi.h"
#include "Moto_Ctrl_V5_esp32/lib/config.h"
#include "Moto_Ctrl_V5_esp32/lib/superbase_functions.h"
#include "Moto_Ctrl_V5_esp32/lib/_params.h"
#include "Moto_Ctrl_V5_esp32/lib/http_event_handler.h"

static const char *TAG = "Supabase";
// extern const uint8_t server_root_ca_crt_start[] asm("_binary_superbase_root_crt_start");
// extern const uint8_t server_root_ca_crt_end[]   asm("_binary_superbase_root_crt_end");




void get_supabase(void)
{
    char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }

    esp_http_client_config_t config = {
        .url = "https://vdoohbjzydtvaqpqsngz.supabase.co/rest/v1/rpc/get_irrigation_status_log_data",
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
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        free(buffer);
        return;
    }

    esp_http_client_set_header(client, "apikey", SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Authorization", "Bearer " SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_header(client, "Accept-Encoding", "identity");  // Avoid gzip

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP chunk encoding Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "Response: %s", buffer);
    

    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        return;
    }

    // Assume it's an array of one object
    cJSON *first_item = cJSON_GetArrayItem(root, 0);
    if (first_item == NULL) {
        ESP_LOGE(TAG, "Response JSON array is empty or malformed");
        cJSON_Delete(root);
        return;
    }

    // Extract individual fields
    cJSON *duration_cJSON = cJSON_GetObjectItem(first_item, "duration");
    if (cJSON_IsNumber(duration_cJSON)) {
        duration = duration_cJSON->valueint;
        ESP_LOGI(TAG, "duration: %d", duration);
    }

    cJSON *user_duration_cJSON = cJSON_GetObjectItem(first_item, "user_duration_minutes");
    if (cJSON_IsNumber(user_duration_cJSON)) {
        user_decision = user_duration_cJSON->valueint;
        ESP_LOGI(TAG, "user_duration_minutes: %d", user_decision);

    }

    cJSON *control_source_cJSON = cJSON_GetObjectItem(first_item, "control_source");
    if (cJSON_IsString(control_source_cJSON)) {
        strncpy(automation_controller, control_source_cJSON->valuestring, sizeof(automation_controller) - 1);
        ESP_LOGI(TAG, "control_source: %s", automation_controller);

    }

    cJSON *user_execution_cJSON = cJSON_GetObjectItem(first_item, "user_execution");
    if (cJSON_IsNumber(user_execution_cJSON)) {
        user_execution = user_execution_cJSON->valueint;
        ESP_LOGI(TAG, "user_execution: %d", user_execution);
    }
    

    // Clean up
    esp_http_client_cleanup(client);
    free(buffer);
    cJSON_Delete(root);



    
}



void send_to_supabase(int user_decision_value)
{

    // // char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    // if (buffer == NULL) {
    //     ESP_LOGE(TAG, "Cannot malloc http receive buffer");
    //     return;
    // }

    esp_http_client_config_t config = {
        .url = SUPABASE_API_URL "/rest/v1/rpc/update_latest_user_execution",
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char *) _binary_superbase_root_crt_start,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = 5000,
        .buffer_size = 2048,
        .buffer_size_tx = 1024,
        .event_handler = _http_event_handler,
        // .user_data = buffer
        
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    // ESP_LOGI("CERT", "Embedded Root CA:\n\n%s\n", _binary_superbase_root_crt_start);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_header(client, "apikey", SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Authorization", "Bearer " SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Prefer", "return=minimal");

    char *post_data = malloc(128);
    if (!post_data) {
        ESP_LOGE(TAG, "Failed to allocate post_data");
        esp_http_client_cleanup(client);
        return;
    }

    snprintf(post_data, 128, "{\"exec_value\": %d}", user_decision_value);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    free(post_data);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "POST Success. Status: %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    // free(buffer);
}

void send_to_supabase_user_execusion(int user_execution)
{
    char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }
    char url[256];
    snprintf(url, sizeof(url), "%s/rest/v1/rpc/update_user_decision", SUPABASE_API_URL);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char *) _binary_superbase_root_crt_start,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = 5000,
        .buffer_size = 2048,
        .buffer_size_tx = 1024,
        .event_handler = _http_event_handler,
        .user_data = buffer
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_header(client, "apikey", SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Authorization", "Bearer " SUPABASE_API_KEY);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Prefer", "return=minimal");

    char *post_data = malloc(64);
    if (!post_data) {
        ESP_LOGE(TAG, "Failed to allocate post_data");
        esp_http_client_cleanup(client);
        return;
    }

    snprintf(post_data, 64, "{\"exec_value\":%d}", user_execution);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    free(post_data);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP POST Status = %d", status_code);

        if (status_code != 204)
        {
            int content_length = esp_http_client_get_content_length(client);
            if (content_length > 0 && content_length < 1024) 
            {
                // char *buffer = malloc(content_length + 1);
                ESP_LOGI(TAG, "Response: %s", buffer);
                free(buffer);
                
            }
        }
    } 
    else 
    {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    // free(buffer);
    
}
