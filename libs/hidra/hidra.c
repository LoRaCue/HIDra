#include "hidra.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "hidra_master";

esp_err_t hidra_master_bus_init(i2c_port_num_t i2c_port, int sda_io_num, int scl_io_num, hidra_bus_handle_t* bus_handle_out)
{
    if (!bus_handle_out) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = i2c_port,
        .scl_io_num = scl_io_num,
        .sda_io_num = sda_io_num,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&i2c_bus_config, bus_handle_out);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C master bus initialized on port %d", i2c_port);
    }
    return ret;
}

esp_err_t hidra_master_bus_deinit(hidra_bus_handle_t bus_handle)
{
    if (!bus_handle) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = i2c_del_master_bus(bus_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C master bus deinitialized");
    }
    return ret;
}

esp_err_t hidra_add_device_to_bus(hidra_bus_handle_t bus_handle, uint8_t i2c_address, hidra_device_handle_t* device_handle_out)
{
    if (!bus_handle || !device_handle_out) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = i2c_address,
        .scl_speed_hz = 100000, // 100kHz
    };

    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, device_handle_out);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "HIDra device added at address 0x%02X", i2c_address);
    }
    return ret;
}

esp_err_t hidra_remove_device_from_bus(hidra_device_handle_t device_handle)
{
    if (!device_handle) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = i2c_master_bus_rm_device(device_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "HIDra device removed from bus");
    }
    return ret;
}

esp_err_t hidra_send_generic_report(hidra_device_handle_t device, uint8_t hid_register, const uint8_t* report, size_t report_size, int timeout_ms)
{
    if (!device || !report || report_size == 0 || report_size > MAX_REPORT_SIZE) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[MAX_REPORT_SIZE + 1];
    buffer[0] = hid_register;
    memcpy(&buffer[1], report, report_size);

    esp_err_t ret = i2c_master_transmit(device, buffer, report_size + 1, timeout_ms);
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Sent HID report to register 0x%02X, size: %d", hid_register, report_size);
    } else {
        ESP_LOGE(TAG, "Failed to send HID report: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t hidra_read_status(hidra_device_handle_t device, uint8_t* status_out, int timeout_ms)
{
    if (!device || !status_out) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reg_addr = STATUS_REG;
    esp_err_t ret = i2c_master_transmit_receive(device, &reg_addr, 1, status_out, 1, timeout_ms);
    
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Read status: 0x%02X", *status_out);
    } else {
        ESP_LOGE(TAG, "Failed to read status: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t hidra_set_composite_device_config(hidra_device_handle_t device, uint16_t device_bitmap, int timeout_ms)
{
    if (!device) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[3];
    buffer[0] = CONFIG_COMPOSITE_DEVICE_REG;
    buffer[1] = device_bitmap & 0xFF;        // LSB
    buffer[2] = (device_bitmap >> 8) & 0xFF; // MSB

    esp_err_t ret = i2c_master_transmit(device, buffer, 3, timeout_ms);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Set composite device config: 0x%04X", device_bitmap);
    } else {
        ESP_LOGE(TAG, "Failed to set composite device config: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t hidra_set_usb_ids(hidra_device_handle_t device, uint16_t vid, uint16_t pid, int timeout_ms)
{
    if (!device) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[5];
    buffer[0] = CONFIG_USB_IDS_REG;
    buffer[1] = vid & 0xFF;        // VID LSB
    buffer[2] = (vid >> 8) & 0xFF; // VID MSB
    buffer[3] = pid & 0xFF;        // PID LSB
    buffer[4] = (pid >> 8) & 0xFF; // PID MSB

    esp_err_t ret = i2c_master_transmit(device, buffer, 5, timeout_ms);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Set USB IDs - VID: 0x%04X, PID: 0x%04X", vid, pid);
    } else {
        ESP_LOGE(TAG, "Failed to set USB IDs: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t hidra_set_usb_string(hidra_device_handle_t device, uint8_t config_register, const char* str, int timeout_ms)
{
    if (!device || !str) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t str_len = strlen(str);
    if (str_len > MAX_STRING_LENGTH) {
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t buffer[MAX_STRING_LENGTH + 2];
    buffer[0] = config_register;
    memcpy(&buffer[1], str, str_len);
    buffer[str_len + 1] = '\0'; // Null terminator

    esp_err_t ret = i2c_master_transmit(device, buffer, str_len + 2, timeout_ms);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Set USB string (reg 0x%02X): %s", config_register, str);
    } else {
        ESP_LOGE(TAG, "Failed to set USB string: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t hidra_reconfigure_address(hidra_device_handle_t* device_handle_ptr, uint8_t new_address, int timeout_ms)
{
    if (!device_handle_ptr || !*device_handle_ptr) {
        return ESP_ERR_INVALID_ARG;
    }

    hidra_device_handle_t old_device = *device_handle_ptr;
    
    // Send address change command
    uint8_t buffer[2];
    buffer[0] = CONFIG_I2C_ADDR_REG;
    buffer[1] = new_address;

    esp_err_t ret = i2c_master_transmit(old_device, buffer, 2, timeout_ms);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send address change command: %s", esp_err_to_name(ret));
        return ret;
    }

    // Wait for device to reboot (typical reboot time)
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Note: Cannot retrieve bus handle from device in ESP-IDF v5.5
    // User must provide the bus handle for re-adding the device
    ESP_LOGW(TAG, "Device address change sent. Device should reboot with new address 0x%02X", new_address);
    ESP_LOGW(TAG, "Please remove and re-add device with new address using hidra_add_device_to_bus()");
    
    // Remove old device
    hidra_remove_device_from_bus(old_device);
    *device_handle_ptr = NULL;

    return ESP_OK;
}
