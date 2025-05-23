#include "Moto_Ctrl_V5_esp32/lib/dependencies.h"

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
extern volatile bool wifi_connected;;

void connect_or_reconnect_to_wifi();
void initialise_wifi();

#ifdef __cplusplus
}
#endif
