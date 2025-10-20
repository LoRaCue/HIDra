#include "unity.h"
#include "hidra_protocol.h"

void test_i2c_protocol(void)
{
    // Test HID register address calculation
    // Register address = (Usage Page << 4) | Usage ID
    
    // Keyboard: Generic Desktop (0x01) | Keyboard (0x06) = 0x16
    uint8_t kbd_reg = (0x01 << 4) | 0x06;
    TEST_ASSERT_EQUAL_HEX8(HIDRA_REG_KEYBOARD, kbd_reg);
    
    // Mouse: Generic Desktop (0x01) | Mouse (0x02) = 0x12
    uint8_t mouse_reg = (0x01 << 4) | 0x02;
    TEST_ASSERT_EQUAL_HEX8(HIDRA_REG_MOUSE, mouse_reg);
    
    // Gamepad: Generic Desktop (0x01) | Gamepad (0x05) = 0x15
    uint8_t gamepad_reg = (0x01 << 4) | 0x05;
    TEST_ASSERT_EQUAL_HEX8(HIDRA_REG_GAMEPAD, gamepad_reg);
    
    // Consumer: Consumer (0x0C) | Consumer Control (0x01) = 0xC1
    uint8_t consumer_reg = (0x0C << 4) | 0x01;
    TEST_ASSERT_EQUAL_HEX8(HIDRA_REG_CONSUMER, consumer_reg);
    
    // Test register ranges
    TEST_ASSERT_TRUE(HIDRA_REG_KEYBOARD >= 0x10 && HIDRA_REG_KEYBOARD <= 0xEF);
    TEST_ASSERT_TRUE(HIDRA_REG_MOUSE >= 0x10 && HIDRA_REG_MOUSE <= 0xEF);
    TEST_ASSERT_TRUE(HIDRA_REG_GAMEPAD >= 0x10 && HIDRA_REG_GAMEPAD <= 0xEF);
    
    // Test configuration register range
    TEST_ASSERT_TRUE(CONFIG_USB_IDS_REG >= 0xF0 && CONFIG_USB_IDS_REG <= 0xFE);
    TEST_ASSERT_TRUE(CONFIG_COMPOSITE_DEVICE_REG >= 0xF0 && CONFIG_COMPOSITE_DEVICE_REG <= 0xFE);
    TEST_ASSERT_TRUE(CONFIG_I2C_ADDR_REG >= 0xF0 && CONFIG_I2C_ADDR_REG <= 0xFE);
    
    // Test status register
    TEST_ASSERT_EQUAL_HEX8(0xFF, STATUS_REG);
    
    // Test register uniqueness
    uint8_t registers[] = {
        HIDRA_REG_KEYBOARD, HIDRA_REG_MOUSE, HIDRA_REG_GAMEPAD, HIDRA_REG_CONSUMER,
        CONFIG_USB_IDS_REG, CONFIG_COMPOSITE_DEVICE_REG, CONFIG_I2C_ADDR_REG, STATUS_REG
    };
    
    size_t reg_count = sizeof(registers) / sizeof(registers[0]);
    for (size_t i = 0; i < reg_count; i++) {
        for (size_t j = i + 1; j < reg_count; j++) {
            TEST_ASSERT_NOT_EQUAL(registers[i], registers[j]);
        }
    }
    
    // Test USB ID payload format
    uint16_t test_vid = 0x1234;
    uint16_t test_pid = 0x5678;
    uint8_t usb_ids_payload[4] = {
        test_vid & 0xFF,        // VID LSB
        (test_vid >> 8) & 0xFF, // VID MSB
        test_pid & 0xFF,        // PID LSB
        (test_pid >> 8) & 0xFF  // PID MSB
    };
    
    uint16_t decoded_vid = (usb_ids_payload[1] << 8) | usb_ids_payload[0];
    uint16_t decoded_pid = (usb_ids_payload[3] << 8) | usb_ids_payload[2];
    TEST_ASSERT_EQUAL_HEX16(test_vid, decoded_vid);
    TEST_ASSERT_EQUAL_HEX16(test_pid, decoded_pid);
}
