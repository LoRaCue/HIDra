#include "unity.h"
#include "hidra.h"
#include "esp_err.h"
#include <string.h>

// Mock I2C handles for testing
static hidra_bus_handle_t mock_bus_handle = (hidra_bus_handle_t)0x12345678;
static hidra_device_handle_t mock_device_handle = (hidra_device_handle_t)0x87654321;

void test_hidra_master_api(void)
{
    // Test parameter validation
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_master_bus_init(I2C_NUM_0, 4, 5, NULL));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_master_bus_deinit(NULL));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_add_device_to_bus(NULL, 0x70, &mock_device_handle));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_add_device_to_bus(mock_bus_handle, 0x70, NULL));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_remove_device_from_bus(NULL));
    
    // Test HID report validation
    uint8_t test_report[8] = {0};
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_send_generic_report(NULL, HIDRA_REG_KEYBOARD, test_report, 8, 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_send_generic_report(mock_device_handle, HIDRA_REG_KEYBOARD, NULL, 8, 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_send_generic_report(mock_device_handle, HIDRA_REG_KEYBOARD, test_report, 0, 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_send_generic_report(mock_device_handle, HIDRA_REG_KEYBOARD, test_report, MAX_REPORT_SIZE + 1, 1000));
    
    // Test status read validation
    uint8_t status;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_read_status(NULL, &status, 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_read_status(mock_device_handle, NULL, 1000));
    
    // Test configuration validation
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_set_composite_device_config(NULL, LAYOUT_KEYBOARD, 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_set_usb_ids(NULL, 0x1234, 0x5678, 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_set_usb_string(NULL, CONFIG_MANUFACTURER_STR_REG, "test", 1000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_set_usb_string(mock_device_handle, CONFIG_MANUFACTURER_STR_REG, NULL, 1000));
    
    // Test string length validation
    char long_string[MAX_STRING_LENGTH + 2];
    memset(long_string, 'A', sizeof(long_string) - 1);
    long_string[sizeof(long_string) - 1] = '\0';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_SIZE, hidra_set_usb_string(mock_device_handle, CONFIG_MANUFACTURER_STR_REG, long_string, 1000));
    
    // Test address reconfiguration validation
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_reconfigure_address(NULL, 0x42, 1000));
    
    hidra_device_handle_t null_device = NULL;
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, hidra_reconfigure_address(&null_device, 0x42, 1000));
}
