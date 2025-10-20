#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hidra.h"

static const char *TAG = "hidra_example";

void app_main(void)
{
    ESP_LOGI(TAG, "HIDra Master Example Starting");

    // 1. Initialize I2C bus
    hidra_bus_handle_t bus_handle;
    esp_err_t ret = hidra_master_bus_init(I2C_NUM_0, GPIO_NUM_4, GPIO_NUM_5, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
        return;
    }

    // 2. Add HIDra device at default address
    hidra_device_handle_t device;
    ret = hidra_add_device_to_bus(bus_handle, DEFAULT_I2C_ADDR, &device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device: %s", esp_err_to_name(ret));
        hidra_master_bus_deinit(bus_handle);
        return;
    }

    // 3. Test status reading
    uint8_t status;
    ret = hidra_read_status(device, &status, 1000);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Device status: 0x%02X", status);
    }

    // 4. Send test keyboard report (press 'A' key)
    uint8_t kbd_report[8] = {0}; // Standard keyboard report
    kbd_report[2] = 0x04; // 'A' key scancode
    
    ret = hidra_send_generic_report(device, HIDRA_REG_KEYBOARD, kbd_report, sizeof(kbd_report), 1000);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Sent keyboard report");
        
        // Check status after command
        ret = hidra_read_status(device, &status, 1000);
        if (ret == ESP_OK) {
            if (status & STATUS_OK) {
                ESP_LOGI(TAG, "Command successful");
            } else {
                ESP_LOGW(TAG, "Command failed, status: 0x%02X", status);
            }
        }
    }

    // 5. Send key release
    memset(kbd_report, 0, sizeof(kbd_report));
    hidra_send_generic_report(device, HIDRA_REG_KEYBOARD, kbd_report, sizeof(kbd_report), 1000);

    // 6. Test mouse report (move cursor)
    uint8_t mouse_report[4] = {0}; // Standard mouse report
    mouse_report[1] = 10;  // X movement
    mouse_report[2] = -5;  // Y movement
    
    ret = hidra_send_generic_report(device, HIDRA_REG_MOUSE, mouse_report, sizeof(mouse_report), 1000);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Sent mouse report");
    }

    // 7. Example of reconfiguring device to new address
    ESP_LOGI(TAG, "Reconfiguring device address from 0x%02X to 0x42", DEFAULT_I2C_ADDR);
    ret = hidra_reconfigure_address(&device, 0x42, 1000);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Device successfully reconfigured to address 0x42");
        
        // Test communication with new address
        ret = hidra_read_status(device, &status, 1000);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Communication confirmed with new address, status: 0x%02X", status);
        }
    } else {
        ESP_LOGE(TAG, "Failed to reconfigure device address");
    }

    // 8. Cleanup
    hidra_remove_device_from_bus(device);
    hidra_master_bus_deinit(bus_handle);
    
    ESP_LOGI(TAG, "Example completed");
}
