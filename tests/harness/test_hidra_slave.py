#!/usr/bin/env python3
"""
HIDra Slave Test Harness

This script tests the HIDra slave firmware using a USB-to-I2C adapter.
Requires a USB-to-I2C bridge device (e.g., FT232H, CH341A, etc.)
"""

import time
import sys
from typing import Optional

# Protocol constants (must match hidra_protocol.h)
HIDRA_REG_KEYBOARD = 0x16
HIDRA_REG_MOUSE = 0x12
HIDRA_REG_GAMEPAD = 0x15
CONFIG_USB_IDS_REG = 0xF0
CONFIG_COMPOSITE_DEVICE_REG = 0xF4
CONFIG_I2C_ADDR_REG = 0xFE
STATUS_REG = 0xFF

STATUS_OK = 0x01
ERROR_UNKNOWN_REGISTER = 0x02
ERROR_PAYLOAD_TOO_LARGE = 0x04
ERROR_INTERFACE_DISABLED = 0x08
ERROR_NVS_WRITE_FAILED = 0x10

DEFAULT_I2C_ADDR = 0x70

class HidraTestHarness:
    def __init__(self, i2c_adapter):
        """
        Initialize test harness with I2C adapter
        
        Args:
            i2c_adapter: I2C adapter object with read/write methods
        """
        self.i2c = i2c_adapter
        self.device_addr = DEFAULT_I2C_ADDR
        self.test_results = []

    def write_register(self, reg_addr: int, data: bytes) -> bool:
        """Write data to HIDra register"""
        try:
            payload = bytes([reg_addr]) + data
            self.i2c.write(self.device_addr, payload)
            return True
        except Exception as e:
            print(f"Write failed: {e}")
            return False

    def read_status(self) -> Optional[int]:
        """Read status register"""
        try:
            self.i2c.write(self.device_addr, bytes([STATUS_REG]))
            status = self.i2c.read(self.device_addr, 1)
            return status[0] if status else None
        except Exception as e:
            print(f"Status read failed: {e}")
            return None

    def test_status_register(self) -> bool:
        """Test status register functionality"""
        print("Testing status register...")
        
        status = self.read_status()
        if status is None:
            print("❌ Failed to read status register")
            return False
            
        print(f"✅ Status register read: 0x{status:02X}")
        return True

    def test_keyboard_report(self) -> bool:
        """Test keyboard HID report"""
        print("Testing keyboard report...")
        
        # Send keyboard report (press 'A' key)
        kbd_report = bytes([0, 0, 0x04, 0, 0, 0, 0, 0])  # 'A' key
        
        if not self.write_register(HIDRA_REG_KEYBOARD, kbd_report):
            print("❌ Failed to send keyboard report")
            return False
            
        # Check status
        time.sleep(0.1)
        status = self.read_status()
        if status is None:
            print("❌ Failed to read status after keyboard report")
            return False
            
        if status & STATUS_OK:
            print("✅ Keyboard report successful")
            return True
        else:
            print(f"❌ Keyboard report failed, status: 0x{status:02X}")
            return False

    def test_mouse_report(self) -> bool:
        """Test mouse HID report"""
        print("Testing mouse report...")
        
        # Send mouse report (move cursor)
        mouse_report = bytes([0, 10, 246, 0])  # Move right 10, up 10 (246 = -10 in signed byte)
        
        if not self.write_register(HIDRA_REG_MOUSE, mouse_report):
            print("❌ Failed to send mouse report")
            return False
            
        # Check status
        time.sleep(0.1)
        status = self.read_status()
        if status is None:
            print("❌ Failed to read status after mouse report")
            return False
            
        if status & STATUS_OK:
            print("✅ Mouse report successful")
            return True
        else:
            print(f"❌ Mouse report failed, status: 0x{status:02X}")
            return False

    def test_unknown_register(self) -> bool:
        """Test error handling for unknown register"""
        print("Testing unknown register error...")
        
        # Write to invalid register
        if not self.write_register(0x99, bytes([0x01])):
            print("❌ Failed to write to unknown register")
            return False
            
        # Check status
        time.sleep(0.1)
        status = self.read_status()
        if status is None:
            print("❌ Failed to read status after unknown register write")
            return False
            
        if status & ERROR_UNKNOWN_REGISTER:
            print("✅ Unknown register error detected correctly")
            return True
        else:
            print(f"❌ Expected unknown register error, got status: 0x{status:02X}")
            return False

    def test_payload_too_large(self) -> bool:
        """Test error handling for oversized payload"""
        print("Testing payload too large error...")
        
        # Send oversized keyboard report (normal is 8 bytes)
        large_payload = bytes([0] * 65)  # 65 bytes, exceeds MAX_REPORT_SIZE
        
        if not self.write_register(HIDRA_REG_KEYBOARD, large_payload):
            print("❌ Failed to send large payload")
            return False
            
        # Check status
        time.sleep(0.1)
        status = self.read_status()
        if status is None:
            print("❌ Failed to read status after large payload")
            return False
            
        if status & ERROR_PAYLOAD_TOO_LARGE:
            print("✅ Payload too large error detected correctly")
            return True
        else:
            print(f"❌ Expected payload too large error, got status: 0x{status:02X}")
            return False

    def test_usb_id_configuration(self) -> bool:
        """Test USB ID configuration"""
        print("Testing USB ID configuration...")
        
        # Set new VID/PID (this will cause device reboot)
        vid = 0x1234
        pid = 0x5678
        usb_ids = bytes([
            vid & 0xFF, (vid >> 8) & 0xFF,  # VID LSB, MSB
            pid & 0xFF, (pid >> 8) & 0xFF   # PID LSB, MSB
        ])
        
        print(f"Setting VID: 0x{vid:04X}, PID: 0x{pid:04X}")
        print("⚠️  Device will reboot after this command")
        
        if not self.write_register(CONFIG_USB_IDS_REG, usb_ids):
            print("❌ Failed to send USB ID configuration")
            return False
            
        print("✅ USB ID configuration sent (device should reboot)")
        return True

    def run_all_tests(self) -> bool:
        """Run all tests"""
        print("=" * 50)
        print("HIDra Slave Test Harness")
        print("=" * 50)
        
        tests = [
            ("Status Register", self.test_status_register),
            ("Keyboard Report", self.test_keyboard_report),
            ("Mouse Report", self.test_mouse_report),
            ("Unknown Register Error", self.test_unknown_register),
            ("Payload Too Large Error", self.test_payload_too_large),
            ("USB ID Configuration", self.test_usb_id_configuration),
        ]
        
        passed = 0
        total = len(tests)
        
        for test_name, test_func in tests:
            print(f"\n--- {test_name} ---")
            try:
                if test_func():
                    passed += 1
                    self.test_results.append((test_name, True, None))
                else:
                    self.test_results.append((test_name, False, "Test failed"))
            except Exception as e:
                print(f"❌ Test exception: {e}")
                self.test_results.append((test_name, False, str(e)))
            
            time.sleep(0.5)  # Brief pause between tests
        
        print("\n" + "=" * 50)
        print("TEST RESULTS")
        print("=" * 50)
        
        for test_name, passed_test, error in self.test_results:
            status = "✅ PASS" if passed_test else f"❌ FAIL ({error})"
            print(f"{test_name:<30} {status}")
        
        print(f"\nOverall: {passed}/{total} tests passed")
        return passed == total


def main():
    """Main test function"""
    print("HIDra Slave Test Harness")
    print("This script requires a USB-to-I2C adapter")
    print("Connect the adapter to the HIDra slave device")
    print()
    
    # TODO: Initialize your specific I2C adapter here
    # Example adapters:
    # - FT232H with pyftdi
    # - CH341A with ch341dll
    # - Arduino with custom firmware
    
    print("❌ I2C adapter not configured")
    print("Please modify this script to use your specific USB-to-I2C adapter")
    print()
    print("Required adapter methods:")
    print("  - write(device_addr, data)")
    print("  - read(device_addr, length)")
    
    return 1

if __name__ == "__main__":
    sys.exit(main())
