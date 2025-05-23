// #include <Moto_Ctrl_V5_esp32/lib/dependencies.h>


#pragma once  // Prevent double inclusion

// === Wi-Fi (WPA2-Enterprise) Credentials ===
// #define WIFI_SSID     "KSU Wireless"
// #define WIFI_USERNAME "aabenson"
// #define WIFI_PASSWORD "Win#Trebla@2024"

#define WIFI_SSID     "KSU Housing"
#define WIFI_USERNAME "aabenson"
#define WIFI_PASSWORD "Win#Trebla@2024"

// You may want to make this mutable in .cpp


// === Supabase Credentials ===
// These are constants used in HTTP headers
#define SUPABASE_API_URL "https://vdoohbjzydtvaqpqsngz.supabase.co"
#define SUPABASE_API_KEY "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InZkb29oYmp6eWR0dmFxcHFzbmd6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDM5ODI5OTYsImV4cCI6MjA1OTU1ODk5Nn0.pfl5qBL3kn9sbX0vs7JVMTf7yPH-QF0tGDyRSAX1RUQ"
#define SUPABASE_AUTH_EMAIL      "albertansahbenson@gmail.com"
#define SUPABASE_AUTH_PASSWORD   "Winner"
#define SUPABASE_AUTH_URL "https://vdoohbjzydtvaqpqsngz.supabase.co/auth/v1/token"




// === Supabase Table Names ===
#define SUPABASE_TABLE_MAIN          "mainTable"
#define SUPABASE_TABLE_IRRIGATION    "irrigation_status_log"

// === HTTPS Port ===
#define HTTPS_PORT 443



