#include "unity.h"
#include "usb_descriptors.h"
#include "hidra_protocol.h"

// Mock configuration for testing
static hidra_config_t test_config = {
    .i2c_addr = 0x42,
    .usb_vid = 0x1234,
    .usb_pid = 0x5678,
    .manufacturer = "Test Manufacturer",
    .product = "Test Product", 
    .serial = "TEST123456",
    .composite_layout = LAYOUT_KEYBOARD | LAYOUT_MOUSE
};

void test_usb_descriptors(void)
{
    // Test HID report descriptor constants
    TEST_ASSERT_GREATER_THAN(0, hid_report_descriptor_keyboard_len);
    TEST_ASSERT_GREATER_THAN(0, hid_report_descriptor_mouse_len);
    TEST_ASSERT_GREATER_THAN(0, hid_report_descriptor_gamepad_len);
    TEST_ASSERT_GREATER_THAN(0, hid_report_descriptor_consumer_len);
    
    // Test HID report descriptors exist
    TEST_ASSERT_NOT_NULL(hid_report_descriptor_keyboard);
    TEST_ASSERT_NOT_NULL(hid_report_descriptor_mouse);
    TEST_ASSERT_NOT_NULL(hid_report_descriptor_gamepad);
    TEST_ASSERT_NOT_NULL(hid_report_descriptor_consumer);
    
    // Test descriptor initialization
    TEST_ASSERT_EQUAL(ESP_OK, usb_descriptors_init(&test_config));
    
    // Test interface management
    TEST_ASSERT_TRUE(usb_is_interface_enabled(HIDRA_REG_KEYBOARD));
    TEST_ASSERT_TRUE(usb_is_interface_enabled(HIDRA_REG_MOUSE));
    TEST_ASSERT_FALSE(usb_is_interface_enabled(HIDRA_REG_GAMEPAD));
    
    uint8_t kbd_instance = usb_get_hid_instance_for_register(HIDRA_REG_KEYBOARD);
    TEST_ASSERT_NOT_EQUAL(0xFF, kbd_instance);
    
    uint8_t invalid_instance = usb_get_hid_instance_for_register(HIDRA_REG_GAMEPAD);
    TEST_ASSERT_EQUAL_UINT8(0xFF, invalid_instance);
    
    // Test cleanup
    usb_descriptors_deinit();
}
