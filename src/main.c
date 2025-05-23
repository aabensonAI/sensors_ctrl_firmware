#include "Moto_Ctrl_V5_esp32/lib/dependencies.h"
#include "Moto_Ctrl_V5_esp32/lib/ota.h"
#include "Moto_Ctrl_V5_esp32/lib/connect_to_wifi.h"
#include "Moto_Ctrl_V5_esp32/lib/superbase_functions.h"
#include "Moto_Ctrl_V5_esp32/lib/_params.h"
#include "Moto_Ctrl_V5_esp32/lib/moto_functions.h"
#include "Moto_Ctrl_V5_esp32/lib/supabase_login.h"




static const char *TAG = "MAIN";

// Global state
int duration = 0;
int motorSpeed = 0;
int user_decision = 0;
int user_execution = 0;
char automation_controller[32] = "";

// char current_version[16] = "v1.0.0";

// === Main entry point ===
void app_main()
{

    bool should_update = true;
    
    ESP_LOGI(TAG, "🔌 Booted — evaluating OTA update policy...");

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 3. Event Loop Init
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    load_current_version_from_nvs();

    

    // Get current time since boot
    // int64_t now_us = esp_timer_get_time();

    // ✅ Set Wi-Fi module log level to DEBUG
    esp_log_level_set("wifi", ESP_LOG_DEBUG);

    // // Open NVS to check last OTA
    // nvs_handle_t nvs_handle;
    // err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "❌ Failed to open NVS");
    // } else {
    //     uint64_t last_check_us = 0;
    //     nvs_get_u64(nvs_handle, NVS_KEY_LAST_CHECK, &last_check_us);

    //     bool should_update = false;

    //     if (last_check_us == 0) {
    //         ESP_LOGW(TAG, "⚠️ No previous OTA check found — assuming fresh boot.");
    //         should_update = true;
    //     } else if ((now_us - last_check_us) > THREE_DAYS_US) {
    //         ESP_LOGI(TAG, "📆 More than 3 days since last OTA check — triggering update.");
    //         should_update = true;
    //     } else {
    //         ESP_LOGI(TAG, "🕒 Last OTA check was recent. Skipping update.");
    //     }

    //     if (should_update) {
    //         connect_or_reconnect_to_wifi();

    //         while (!wifi_connected) {
    //             ESP_LOGI(TAG, "Waiting for Wi-Fi...");
    //             vTaskDelay(pdMS_TO_TICKS(500));
    //         }

    //         check_for_updates();  // ✅ Your OTA function
    //         nvs_set_u64(nvs_handle, NVS_KEY_LAST_CHECK, now_us);
    //         nvs_commit(nvs_handle);
    //     }

    //     nvs_close(nvs_handle);
    // }

    if (should_update) 
    {
        connect_or_reconnect_to_wifi();

        while (!wifi_connected) {
            ESP_LOGI(TAG, "Waiting for Wi-Fi...");
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        // login_to_supabase();


        check_for_updates();  // ✅ Your OTA function
    }

    init_gpio_and_pwm();
    
    while(true)
    {
        connect_or_reconnect_to_wifi();
        

        while (!wifi_connected) {
            ESP_LOGI(TAG, "Waiting for Wi-Fi...");
            vTaskDelay(pdMS_TO_TICKS(500));
            // connect_or_reconnect_to_wifi();
        }

        // bool dns_ok = manual_resolve_with_retry("vdoohbjzydtvaqpqsngz.supabase.co");

        motor_task();
        
    }
    // xTaskCreate(motor_task, "motor_task", 8192, NULL, 5, NULL);
}
