idf_component_register(
  SRCS
    "main.c"
    "Moto_Ctrl_V5_esp32/source/connect_to_wifi.c"
    "Moto_Ctrl_V5_esp32/source/superbase_functions.c"
    "Moto_Ctrl_V5_esp32/source/ota.c"
    "Moto_Ctrl_V5_esp32/source/moto_functions.c"
    "Moto_Ctrl_V5_esp32/source/http_event_handler.c"
    # Add any additional source files here
  INCLUDE_DIRS
    "."
    "Moto_Ctrl_V5_esp32/lib"
    # Embed CA, certificate & key directly into binary
  EMBED_TXTFILES
    superbase_root.crt

    
)

