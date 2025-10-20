#include "unity.h"
#include "esp_log.h"

static const char *TAG = "hidra_tests";

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

// External test function declarations
extern void test_protocol_constants(void);
extern void test_hidra_master_api(void);
extern void test_usb_descriptors(void);
extern void test_config_management(void);
extern void test_i2c_protocol(void);
extern void test_status_register(void);
extern void test_hid_reports(void);

void app_main(void)
{
    ESP_LOGI(TAG, "Starting HIDra Unit Tests");
    
    UNITY_BEGIN();
    
    // Protocol tests
    RUN_TEST(test_protocol_constants);
    
    // Master component tests
    RUN_TEST(test_hidra_master_api);
    
    // USB descriptor tests
    RUN_TEST(test_usb_descriptors);
    
    // Configuration tests
    RUN_TEST(test_config_management);
    
    // I2C protocol tests
    RUN_TEST(test_i2c_protocol);
    
    // Status register tests
    RUN_TEST(test_status_register);
    
    // HID report tests
    RUN_TEST(test_hid_reports);
    
    UNITY_END();
}
