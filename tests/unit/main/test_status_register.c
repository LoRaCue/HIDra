#include "unity.h"
#include "hidra_protocol.h"

void test_status_register(void)
{
    // Test individual status bits
    TEST_ASSERT_EQUAL_HEX8(0x01, STATUS_OK);
    TEST_ASSERT_EQUAL_HEX8(0x02, ERROR_UNKNOWN_REGISTER);
    TEST_ASSERT_EQUAL_HEX8(0x04, ERROR_PAYLOAD_TOO_LARGE);
    TEST_ASSERT_EQUAL_HEX8(0x08, ERROR_INTERFACE_DISABLED);
    TEST_ASSERT_EQUAL_HEX8(0x10, ERROR_NVS_WRITE_FAILED);
    
    // Test bit uniqueness
    uint8_t status_bits[] = {
        STATUS_OK, ERROR_UNKNOWN_REGISTER, ERROR_PAYLOAD_TOO_LARGE,
        ERROR_INTERFACE_DISABLED, ERROR_NVS_WRITE_FAILED
    };
    
    size_t bit_count = sizeof(status_bits) / sizeof(status_bits[0]);
    for (size_t i = 0; i < bit_count; i++) {
        for (size_t j = i + 1; j < bit_count; j++) {
            TEST_ASSERT_NOT_EQUAL(status_bits[i], status_bits[j]);
            TEST_ASSERT_EQUAL_UINT8(0, status_bits[i] & status_bits[j]); // No overlapping bits
        }
    }
    
    // Test bit manipulation operations
    uint8_t status = 0;
    
    // Set STATUS_OK
    status |= STATUS_OK;
    TEST_ASSERT_TRUE(status & STATUS_OK);
    TEST_ASSERT_FALSE(status & ERROR_UNKNOWN_REGISTER);
    
    // Add error bit
    status |= ERROR_PAYLOAD_TOO_LARGE;
    TEST_ASSERT_TRUE(status & STATUS_OK);
    TEST_ASSERT_TRUE(status & ERROR_PAYLOAD_TOO_LARGE);
    TEST_ASSERT_FALSE(status & ERROR_UNKNOWN_REGISTER);
    
    // Clear specific bit
    status &= ~STATUS_OK;
    TEST_ASSERT_FALSE(status & STATUS_OK);
    TEST_ASSERT_TRUE(status & ERROR_PAYLOAD_TOO_LARGE);
    
    // Clear all bits
    status = 0;
    TEST_ASSERT_EQUAL_UINT8(0, status);
    
    // Test multiple error conditions
    status = ERROR_UNKNOWN_REGISTER | ERROR_PAYLOAD_TOO_LARGE | ERROR_NVS_WRITE_FAILED;
    TEST_ASSERT_TRUE(status & ERROR_UNKNOWN_REGISTER);
    TEST_ASSERT_TRUE(status & ERROR_PAYLOAD_TOO_LARGE);
    TEST_ASSERT_TRUE(status & ERROR_NVS_WRITE_FAILED);
    TEST_ASSERT_FALSE(status & STATUS_OK);
    TEST_ASSERT_FALSE(status & ERROR_INTERFACE_DISABLED);
    
    // Test status register read-clear behavior simulation
    uint8_t original_status = STATUS_OK | ERROR_PAYLOAD_TOO_LARGE;
    uint8_t read_status = original_status;
    original_status = 0; // Simulate clear on read
    
    TEST_ASSERT_TRUE(read_status & STATUS_OK);
    TEST_ASSERT_TRUE(read_status & ERROR_PAYLOAD_TOO_LARGE);
    TEST_ASSERT_EQUAL_UINT8(0, original_status);
}
