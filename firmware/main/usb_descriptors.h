#pragma once

#include "tusb.h"
#include "hidra_protocol.h"

// Forward declaration
typedef struct {
    uint8_t i2c_addr;
    uint16_t usb_vid;
    uint16_t usb_pid;
    char manufacturer[64];
    char product[64];
    char serial[64];
    uint16_t composite_layout;
} hidra_config_t;

// USB descriptor builder interface
esp_err_t usb_descriptors_init(const hidra_config_t *config);
void usb_descriptors_deinit(void);

// TinyUSB descriptor callbacks
uint8_t const *tud_descriptor_device_cb(void);
uint8_t const *tud_descriptor_configuration_cb(uint8_t index);
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid);
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance);

// HID interface management
uint8_t usb_get_hid_instance_for_register(uint8_t hid_register);
bool usb_is_interface_enabled(uint8_t hid_register);

// HID report descriptors
extern const uint8_t hid_report_descriptor_keyboard[];
extern const uint8_t hid_report_descriptor_mouse[];
extern const uint8_t hid_report_descriptor_gamepad[];
extern const uint8_t hid_report_descriptor_consumer[];

extern const size_t hid_report_descriptor_keyboard_len;
extern const size_t hid_report_descriptor_mouse_len;
extern const size_t hid_report_descriptor_gamepad_len;
extern const size_t hid_report_descriptor_consumer_len;
