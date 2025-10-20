#pragma once

#include <stdint.h>

// HID Data Registers (Write-Only)
// Register address = (Usage Page << 4) | Usage ID
#define HIDRA_REG_KEYBOARD      0x16  // Generic Desktop (0x01) | Keyboard (0x06)
#define HIDRA_REG_MOUSE         0x12  // Generic Desktop (0x01) | Mouse (0x02)
#define HIDRA_REG_JOYSTICK      0x14  // Generic Desktop (0x01) | Joystick (0x04)
#define HIDRA_REG_GAMEPAD       0x15  // Generic Desktop (0x01) | Gamepad (0x05)
#define HIDRA_REG_CONSUMER      0xC1  // Consumer (0x0C) | Consumer Control (0x01)
#define HIDRA_REG_PEN           0xD2  // Digitizers (0x0D) | Pen/Stylus (0x02)
#define HIDRA_REG_TOUCHSCREEN   0xD4  // Digitizers (0x0D) | Touch Screen (0x04)
#define HIDRA_REG_TOUCHPAD      0xD5  // Digitizers (0x0D) | Touch Pad (0x05)

// Configuration Registers (Write-Only)
#define CONFIG_USB_IDS_REG          0xF0  // 4 bytes: [VID_LSB, VID_MSB, PID_LSB, PID_MSB]
#define CONFIG_MANUFACTURER_STR_REG 0xF1  // Variable length, null-terminated UTF-8 (max 63 chars)
#define CONFIG_PRODUCT_STR_REG      0xF2  // Variable length, null-terminated UTF-8 (max 63 chars)
#define CONFIG_SERIAL_STR_REG       0xF3  // Variable length, null-terminated UTF-8 (max 63 chars)
#define CONFIG_COMPOSITE_DEVICE_REG 0xF4  // 2 bytes (uint16_t): bitmap of enabled HID interfaces
#define CONFIG_I2C_ADDR_REG         0xFE  // 1 byte: new 7-bit I2C slave address

// Status Register (Read-Only)
#define STATUS_REG                  0xFF  // 1 byte: bitmask of internal state

// Status Register Bit Definitions
#define STATUS_OK                   0x01  // Last command successful (cleared on read)
#define ERROR_UNKNOWN_REGISTER      0x02  // Write to undefined register
#define ERROR_PAYLOAD_TOO_LARGE     0x04  // More data than expected
#define ERROR_INTERFACE_DISABLED    0x08  // HID report for disabled interface
#define ERROR_NVS_WRITE_FAILED      0x10  // Failed to save config to NVS

// Default Configuration Values
#define DEFAULT_I2C_ADDR            0x70
#define DEFAULT_USB_VID             0x413D
#define DEFAULT_USB_PID             0x0001
#define DEFAULT_MANUFACTURER        "HIDra Project"
#define DEFAULT_PRODUCT             "HIDra Composite HID"
#define DEFAULT_COMPOSITE_LAYOUT    0x000B  // Keyboard | Mouse | Gamepad

// Composite Layout Bitmap
#define LAYOUT_KEYBOARD             (1 << 0)
#define LAYOUT_MOUSE                (1 << 1)
#define LAYOUT_JOYSTICK             (1 << 2)
#define LAYOUT_GAMEPAD              (1 << 3)
#define LAYOUT_CONSUMER             (1 << 4)
#define LAYOUT_PEN                  (1 << 5)
#define LAYOUT_TOUCHSCREEN          (1 << 6)
#define LAYOUT_TOUCHPAD             (1 << 7)

// NVS Keys
#define NVS_NAMESPACE               "hidra"
#define NVS_KEY_I2C_ADDR            "i2c.addr"
#define NVS_KEY_USB_VID             "usb.vid"
#define NVS_KEY_USB_PID             "usb.pid"
#define NVS_KEY_MANUFACTURER        "usb.manuf"
#define NVS_KEY_PRODUCT             "usb.prod"
#define NVS_KEY_SERIAL              "usb.serial"
#define NVS_KEY_COMPOSITE_LAYOUT    "usb.layout"

// Protocol Limits
#define MAX_STRING_LENGTH           63
#define MAX_REPORT_SIZE             64
#define FACTORY_RESET_GPIO          0  // GPIO pin for factory reset
