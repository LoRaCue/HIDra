#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "hidra";

void app_main(void) {
    ESP_LOGI(TAG, "Hidra firmware starting...");
    ESP_LOGI(TAG, "I2C slave mode - Bluetooth and WiFi disabled");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
