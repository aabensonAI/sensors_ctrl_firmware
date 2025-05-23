#include "Moto_Ctrl_V5_esp32/lib/connect_to_wifi.h"
#include "Moto_Ctrl_V5_esp32/lib/superbase_functions.h"
#include "Moto_Ctrl_V5_esp32/lib/_params.h"
#include "Moto_Ctrl_V5_esp32/lib/dependencies.h"

static const char *TAG = "moto";

// === Blink Helper ===
void blink(int pin)
{
    for (int i = 0; i < 4; i++) {
        gpio_set_level(pin, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(pin, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// === Motor Task (formerly loop) ===
void motor_task(void)
{
    // Connect once at the beginning
    connect_or_reconnect_to_wifi();
    int64_t  wakeStart = esp_timer_get_time() / 1000;
    get_supabase();  // Pull initial state
    send_to_supabase(user_decision);  
    

    

    if (strcmp(automation_controller, "User") == 0) {

        if (user_execution > 0) 
        {
            uint64_t start_time = esp_timer_get_time() / 1000;
            send_to_supabase(user_decision);

            while ((esp_timer_get_time() / 1000) - start_time <= user_decision * 60 * 1000 &&
                    strcmp(automation_controller, "User") == 0 && user_execution > 0) 
            {
                ESP_LOGI(TAG, "Master Override motor operation");

                motorSpeed = 170;
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, motorSpeed);
                ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

                send_to_supabase(user_decision - ((esp_timer_get_time() / 1000 - start_time) / 60000));
                get_supabase();
                ESP_LOGI(TAG, "Motor ON");
                ESP_LOGI(TAG, "Motor Speed: %d", motorSpeed);

                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            send_to_supabase_user_execusion(user_decision);
        } 

        else if(user_execution == 0 && user_decision == 0)
        {
            ESP_LOGI(TAG,"Operation Complete, Motor OFF");
            motorSpeed = 0;
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, motorSpeed);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

            send_to_supabase(user_decision);
            get_supabase();
            ESP_LOGI(TAG, "Motor OFF");
        }
    }

    else if (strcmp(automation_controller, "Remote Node") == 0) 
    {
        if (duration == 0) {
            get_supabase();
            motorSpeed = 0;
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, motorSpeed);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

            send_to_supabase(0);
        } 
        else 
        {
            while ((esp_timer_get_time() / 1000) - wakeStart < duration * 60 * 1000) {
                motorSpeed = 170;
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, motorSpeed);
                ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

                send_to_supabase(0);
                get_supabase();

                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }

    vTaskDelay(pdMS_TO_TICKS(5000));  // Delay before next evaluation
}


// === GPIO and PWM Setup ===
void init_gpio_and_pwm()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN) | (1ULL << HTTP_STATUS_LED_PIN) | (1ULL << FEEDBACK_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = PWM_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = MOTOR_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel);
}