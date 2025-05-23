#include <Moto_Ctrl_V5_esp32/lib/dependencies.h>
#include <Moto_Ctrl_V5_esp32/lib/config.h>
#include <Moto_Ctrl_V5_esp32/lib/connect_to_wifi.h>
#include <Moto_Ctrl_V5_esp32/lib/_params.h>

/* FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t wifi_event_group;
static esp_netif_t *sta_netif = NULL;

const int CONNECTED_BIT = BIT0;
volatile bool wifi_connected = false;
static bool wifi_initialized = false;


static const char *TAG = "wifi";

// === Wi-Fi Event Handler ===
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        wifi_connected = true;
    }
}

void initialise_wifi()
{
    if (wifi_initialized) return;

    wifi_initialized = true;
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();

    // Only call once globally
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            // .password = WIFI_PASSWORD
        },
    };

    ESP_LOGI(TAG, "Connecting to SSID: %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // === Enterprise Auth: PEAP/MSCHAPv2 ===
    ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
    ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
    ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD)));

    // Start enterprise and Wi-Fi connection
    ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
    ESP_ERROR_CHECK(esp_wifi_start());
}

void connect_or_reconnect_to_wifi()
{
    initialise_wifi();

    esp_netif_ip_info_t ip;
    memset(&ip, 0, sizeof(esp_netif_ip_info_t));

    if (esp_netif_get_ip_info(sta_netif, &ip) == 0) {
        ESP_LOGI(TAG, "~~~~~~~~~~~");
        ESP_LOGI(TAG, "IP:   " IPSTR, IP2STR(&ip.ip));
        ESP_LOGI(TAG, "MASK: " IPSTR, IP2STR(&ip.netmask));
        ESP_LOGI(TAG, "GW:   " IPSTR, IP2STR(&ip.gw));
        ESP_LOGI(TAG, "~~~~~~~~~~~");
    }
}
