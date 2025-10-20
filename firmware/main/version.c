#include "version.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "version";

const char* firmware_get_version(void) {
    return FIRMWARE_VERSION_SEMVER;
}

const char* firmware_get_version_full(void) {
    return FIRMWARE_VERSION_FULL;
}

const char* firmware_get_build_info(void) {
    static char build_info[512];
    snprintf(build_info, sizeof(build_info), 
             "%s v%s (Build: %s, Commit: %.8s, Branch: %s, ESP-IDF: %s)",
             FIRMWARE_NAME,
             FIRMWARE_VERSION_SEMVER,
             FIRMWARE_BUILD_TIMESTAMP,
             FIRMWARE_GIT_COMMIT,
             FIRMWARE_GIT_BRANCH,
             FIRMWARE_ESP_IDF_VERSION);
    return build_info;
}

void firmware_print_version_info(void) {
    ESP_LOGI(TAG, "=== %s ===", FIRMWARE_NAME);
    ESP_LOGI(TAG, "Version: %s", FIRMWARE_VERSION_SEMVER);
    ESP_LOGI(TAG, "Full Version: %s", FIRMWARE_VERSION_FULL);
    ESP_LOGI(TAG, "Description: %s", FIRMWARE_DESCRIPTION);
    ESP_LOGI(TAG, "Build Time: %s", FIRMWARE_BUILD_TIMESTAMP);
    ESP_LOGI(TAG, "Git Commit: %s", FIRMWARE_GIT_COMMIT);
    ESP_LOGI(TAG, "Git Branch: %s", FIRMWARE_GIT_BRANCH);
    ESP_LOGI(TAG, "Build Config: %s", FIRMWARE_BUILD_CONFIG);
    ESP_LOGI(TAG, "ESP-IDF Version: %s", FIRMWARE_ESP_IDF_VERSION);
    ESP_LOGI(TAG, "=======================================");
}
