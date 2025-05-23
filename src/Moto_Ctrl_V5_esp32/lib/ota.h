#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void check_for_updates(void);
void perform_ota(const char *firmware_url, const char *new_version);
void store_firmware_version_in_nvs(const char *version_str);
void load_current_version_from_nvs(void);

#ifdef __cplusplus
}
#endif
