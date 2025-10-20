#include "unity.h"
#include "hidra_protocol.h"
#include <string.h>

void test_hid_reports(void)
{
    // Test standard HID report sizes
    uint8_t keyboard_report[8] = {0}; // Standard keyboard report
    uint8_t mouse_report[4] = {0};    // Standard mouse report
    uint8_t gamepad_report[6] = {0};  // Standard gamepad report
    
    TEST_ASSERT_EQUAL_UINT8(8, sizeof(keyboard_report));
    TEST_ASSERT_EQUAL_UINT8(4, sizeof(mouse_report));
    TEST_ASSERT_EQUAL_UINT8(6, sizeof(gamepad_report));
    
    // Test keyboard report structure
    // [modifier, reserved, key1, key2, key3, key4, key5, key6]
    keyboard_report[0] = 0x02; // Left Shift
    keyboard_report[2] = 0x04; // 'A' key
    
    TEST_ASSERT_EQUAL_UINT8(0x02, keyboard_report[0]); // Modifier
    TEST_ASSERT_EQUAL_UINT8(0x00, keyboard_report[1]); // Reserved
    TEST_ASSERT_EQUAL_UINT8(0x04, keyboard_report[2]); // Key code
    
    // Test mouse report structure
    // [buttons, x_movement, y_movement, wheel]
    mouse_report[0] = 0x01; // Left button
    mouse_report[1] = 10;   // X movement
    mouse_report[2] = 246;  // Y movement (-10 in signed byte)
    mouse_report[3] = 1;    // Wheel scroll
    
    TEST_ASSERT_EQUAL_UINT8(0x01, mouse_report[0]); // Buttons
    TEST_ASSERT_EQUAL_UINT8(10, mouse_report[1]);   // X
    TEST_ASSERT_EQUAL_UINT8(246, mouse_report[2]);  // Y (signed -10)
    TEST_ASSERT_EQUAL_UINT8(1, mouse_report[3]);    // Wheel
    
    // Test report size validation
    TEST_ASSERT_TRUE(sizeof(keyboard_report) <= MAX_REPORT_SIZE);
    TEST_ASSERT_TRUE(sizeof(mouse_report) <= MAX_REPORT_SIZE);
    TEST_ASSERT_TRUE(sizeof(gamepad_report) <= MAX_REPORT_SIZE);
    
    // Test maximum report size
    uint8_t max_report[MAX_REPORT_SIZE];
    memset(max_report, 0xAA, sizeof(max_report));
    TEST_ASSERT_EQUAL_UINT8(MAX_REPORT_SIZE, sizeof(max_report));
    TEST_ASSERT_EQUAL_UINT8(0xAA, max_report[0]);
    TEST_ASSERT_EQUAL_UINT8(0xAA, max_report[MAX_REPORT_SIZE - 1]);
    
    // Test report data integrity
    uint8_t test_report[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t copy_report[8];
    
    memcpy(copy_report, test_report, sizeof(test_report));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_report, copy_report, sizeof(test_report));
    
    // Test report clearing
    memset(test_report, 0, sizeof(test_report));
    for (size_t i = 0; i < sizeof(test_report); i++) {
        TEST_ASSERT_EQUAL_UINT8(0, test_report[i]);
    }
    
    // Test HID register to interface mapping concept
    struct {
        uint8_t hid_register;
        const char* interface_name;
        size_t expected_report_size;
    } interface_map[] = {
        {HIDRA_REG_KEYBOARD, "Keyboard", 8},
        {HIDRA_REG_MOUSE, "Mouse", 4},
        {HIDRA_REG_GAMEPAD, "Gamepad", 6},
        {HIDRA_REG_CONSUMER, "Consumer", 2}
    };
    
    size_t map_count = sizeof(interface_map) / sizeof(interface_map[0]);
    for (size_t i = 0; i < map_count; i++) {
        TEST_ASSERT_TRUE(interface_map[i].expected_report_size <= MAX_REPORT_SIZE);
        TEST_ASSERT_NOT_NULL(interface_map[i].interface_name);
    }
}
