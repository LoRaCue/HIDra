#include "unity.h"
#include "hidra_protocol.h"
#include <string.h>

void test_config_management(void)
{
    // Test default configuration values
    TEST_ASSERT_EQUAL_STRING("HIDra Project", DEFAULT_MANUFACTURER);
    TEST_ASSERT_EQUAL_STRING("HIDra Composite HID", DEFAULT_PRODUCT);
    
    // Test NVS key definitions
    TEST_ASSERT_EQUAL_STRING("hidra", NVS_NAMESPACE);
    TEST_ASSERT_EQUAL_STRING("i2c.addr", NVS_KEY_I2C_ADDR);
    TEST_ASSERT_EQUAL_STRING("usb.vid", NVS_KEY_USB_VID);
    TEST_ASSERT_EQUAL_STRING("usb.pid", NVS_KEY_USB_PID);
    TEST_ASSERT_EQUAL_STRING("usb.manuf", NVS_KEY_MANUFACTURER);
    TEST_ASSERT_EQUAL_STRING("usb.prod", NVS_KEY_PRODUCT);
    TEST_ASSERT_EQUAL_STRING("usb.serial", NVS_KEY_SERIAL);
    TEST_ASSERT_EQUAL_STRING("usb.layout", NVS_KEY_COMPOSITE_LAYOUT);
    
    // Test protocol limits
    TEST_ASSERT_EQUAL_UINT8(63, MAX_STRING_LENGTH);
    TEST_ASSERT_EQUAL_UINT8(64, MAX_REPORT_SIZE);
    TEST_ASSERT_EQUAL_UINT8(0, FACTORY_RESET_GPIO);
    
    // Test string length validation
    char test_string[MAX_STRING_LENGTH + 1];
    memset(test_string, 'A', MAX_STRING_LENGTH);
    test_string[MAX_STRING_LENGTH] = '\0';
    TEST_ASSERT_EQUAL_UINT8(MAX_STRING_LENGTH, strlen(test_string));
    
    // Test configuration bitmap operations
    uint16_t layout = LAYOUT_KEYBOARD | LAYOUT_MOUSE;
    TEST_ASSERT_TRUE(layout & LAYOUT_KEYBOARD);
    TEST_ASSERT_TRUE(layout & LAYOUT_MOUSE);
    TEST_ASSERT_FALSE(layout & LAYOUT_GAMEPAD);
    TEST_ASSERT_FALSE(layout & LAYOUT_CONSUMER);
    
    // Test all layout bits
    uint16_t all_layouts = LAYOUT_KEYBOARD | LAYOUT_MOUSE | LAYOUT_JOYSTICK | 
                          LAYOUT_GAMEPAD | LAYOUT_CONSUMER | LAYOUT_PEN | 
                          LAYOUT_TOUCHSCREEN | LAYOUT_TOUCHPAD;
    TEST_ASSERT_EQUAL_HEX16(0xFF, all_layouts);
}
