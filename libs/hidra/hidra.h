#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "hidra_protocol.h"
#include "version.h"

// Opaque handles
typedef i2c_master_bus_handle_t hidra_bus_handle_t;
typedef i2c_master_dev_handle_t hidra_device_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

// --- Bus Management ---
esp_err_t hidra_master_bus_init(i2c_port_num_t i2c_port, int sda_io_num, int scl_io_num, hidra_bus_handle_t* bus_handle_out);
esp_err_t hidra_master_bus_deinit(hidra_bus_handle_t bus_handle);

// --- Device Management ---
esp_err_t hidra_add_device_to_bus(hidra_bus_handle_t bus_handle, uint8_t i2c_address, hidra_device_handle_t* device_handle_out);
esp_err_t hidra_remove_device_from_bus(hidra_device_handle_t device_handle);

// --- HID Reporting & Status ---
esp_err_t hidra_send_generic_report(hidra_device_handle_t device, uint8_t hid_register, const uint8_t* report, size_t report_size, int timeout_ms);
esp_err_t hidra_read_status(hidra_device_handle_t device, uint8_t* status_out, int timeout_ms);

// --- Device Configuration ---
esp_err_t hidra_set_composite_device_config(hidra_device_handle_t device, uint16_t device_bitmap, int timeout_ms);
esp_err_t hidra_set_usb_ids(hidra_device_handle_t device, uint16_t vid, uint16_t pid, int timeout_ms);
esp_err_t hidra_set_usb_string(hidra_device_handle_t device, uint8_t config_register, const char* str, int timeout_ms);
esp_err_t hidra_reconfigure_address(hidra_device_handle_t* device_handle_ptr, uint8_t new_address, int timeout_ms);

#ifdef __cplusplus
}
#endif
