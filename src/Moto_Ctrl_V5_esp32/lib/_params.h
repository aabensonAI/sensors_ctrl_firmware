#pragma once

#include "cJSON.h"  // ESP-IDF uses cJSON for JSON parsing
#include <stdint.h> // For fixed-width integer types

// === Motor Control Definitions ===
extern int duration;
extern int motorSpeed;
extern int user_decision;
extern int user_execution;

// Avoid Arduino String, use const char* or char arrays
extern char automation_controller[32];  // Or dynamically allocated char*

// === Motor Driver Configuration ===
#define MOTOR_PIN          12
#define LEDC_CHANNEL       0
#define PWM_FREQUENCY_HZ   10000
#define PWM_RESOLUTION     8

// === Soil Parameters ===
#define INFILTRATION_RATE  0.0028f
#define PLOT_AREA          3167.74f
#define PUMP_FLOW_RATE     (INFILTRATION_RATE * PLOT_AREA)

// === LED Pins ===
#define LED_PIN              2
#define HTTP_STATUS_LED_PIN  4
#define FEEDBACK_LED_PIN     5

// === JSON Document (for cJSON, you will use pointers not fixed documents)
extern cJSON *doc;

// === Deep Sleep Config ===
#define US_TO_S_FACTOR      1000000ULL
#define INACTIVITY_TIMEOUT  60

extern uint64_t lastActivityTime;




// Time interval: 3 days in microseconds
#define THREE_DAYS_US (3LL * 24 * 60 * 60 * 1000000)
#define NVS_NAMESPACE "ota_info"
#define NVS_KEY_LAST_CHECK "last_check_us"


#define MAX_DNS_RETRIES 3
extern const uint8_t _binary_superbase_root_crt_start[]; 
extern const uint8_t _binary_superbase_root_crt_end[];

// === HTTP parameters ===
#define MAX_HTTP_RECV_BUFFER 2048
#define MAX_HTTP_OUTPUT_BUFFER 2048

// Stores number of bytes read

#include "esp_crt_bundle.h"

extern char jwt_access_token[1024];  // This is the actual storage
extern char current_version[32];  // e.g., "v1.0.0"