# Blueprint: The HIDra System

**A Multi-headed, I2C-Controlled USB HID System for ESP32-S3**

### **1. System Overview**

The HIDra system enables a single **Master** ESP32-S3 to control multiple **Slave** ESP32-S3 boards over a shared I2C bus. Each Slave board connects to a host computer via USB and acts as a dynamically configurable composite Human Interface Device (HID).  
The architecture is designed for "enterprise-grade" use. A slave's core identity—including its **I2C address**, **USB identity (VID, PID, strings)**, and the **specific combination of HID interfaces** it exposes—can be configured dynamically by the master and is persisted on the slave's non-volatile storage (NVS). When no custom configuration is found, the device reverts to a known, functional set of default values.  
The master component is designed to coexist peacefully with other I2C peripherals by decoupling I2C bus management from the library itself.  
**Core Components:**

1. **I2C Slave Firmware**: A standalone ESP-IDF project for the ESP32-S3.  
2. **I2C Master Component (hidra)**: A reusable ESP-IDF component that acts as a client on a shared I2C bus.  
3. **Protocol Definition**: A shared header file (hidra\_protocol.h) providing a single source of truth for all protocol constants.  
4. **Example Firmware**: An ESP-IDF project demonstrating the use of the hidra master component.

### **2\. Part A: The I2C Slave Firmware**

This firmware turns an ESP32-S3 into a configurable I2C-to-USB bridge.

#### **2.1. Project Setup (ESP-IDF)**

* **Target**: esp32s3  
* **Key Components (idf.py menuconfig)**:  
  * Enable **TinyUSB** for the USB stack.  
  * Enable the **I2C** peripheral driver.  
  * Enable the **NVS (Non-Volatile Storage)** library.  
* **Driver Usage**: This firmware will use the modern **i2c\_slave** driver component.

#### **2.2. RTOS Architecture (FreeRTOS)**

To ensure robust, non-blocking operation, the slave firmware must be built on a multi-task architecture.

* **i2c\_task**: This high-priority task handles all I2C communication. It will block waiting for data from the master. When a valid command arrives, it places the data into a dedicated FreeRTOS queue for the relevant HID device and updates the internal status register.  
* **usb\_task**: This task runs the main TinyUSB stack loop (tud\_task()). In the TinyUSB callbacks (e.g., when the host is ready for a new report), it checks the appropriate queue for data. If a report is available, it dequeues it and sends it to the host.

This architecture decouples the I2C and USB stacks, preventing I2C bus timeouts if the USB host is busy and ensuring the slave can accept new commands rapidly.

#### **2.3. Default Device Configuration**

This table defines the factory-default state of a HIDra slave device, used when no configuration is found in NVS.

| Configuration Item | Default Value | NVS Key (Example) | Notes |
| :---- | :---- | :---- | :---- |
| **I2C Slave Address** | 0x70 | hidra.i2c.addr | This is the initial provisioning address. |
| **USB Vendor ID (VID)** | 0x413D | hidra.usb.vid |  |
| **USB Product ID (PID)** | 0x0001 | hidra.usb.pid | Default PID for the HIDra project. |
| **Manufacturer String** | "HIDra Project" | hidra.usb.manuf |  |
| **Product String** | "HIDra Composite HID" | hidra.usb.prod |  |
| **Serial Number String** | Derived from MAC address | hidra.usb.serial | Ensures a unique serial number for each device. |
| **Composite Layout** | 0x000B | hidra.usb.layout | Enables Keyboard, Mouse, and Gamepad by default. |

* **Composite Layout Default**: The value 0x000B corresponds to (1 \<\< 0\) | (1 \<\< 1\) | (1 \<\< 3), enabling the **Keyboard**, **Mouse**, and **Gamepad** interfaces.

#### **2.4. Dynamic USB Implementation**

The firmware will dynamically construct the USB descriptors at startup. It will use memcpy to build the final descriptor in RAM from a static "pool" of all possible interface descriptors, based on the configuration loaded from NVS. This strategy avoids dynamic memory allocation (malloc).

#### **2.5. The HIDra I2C Protocol**

The protocol is defined in a shared header (hidra\_protocol.h).  
HID Data Registers (Write-Only):  
The I2C register address is a single byte formed by (Usage Page \<\< 4\) | Usage ID.

| Device Type | USB Usage Page | USB Usage ID | HIDra I2C Register Address |
| :---- | :---- | :---- | :---- |
| **Keyboard** | Generic Desktop (0x01) | 0x06 | 0x16 |
| **Mouse** | Generic Desktop (0x01) | 0x02 | 0x12 |
| **Joystick** | Generic Desktop (0x01) | 0x04 | 0x14 |
| **Gamepad** | Generic Desktop (0x01) | 0x05 | 0x15 |
| **Consumer Control** | Consumer (0x0C) | 0x01 | 0xC1 |
| **Pen / Stylus** | Digitizers (0x0D) | 0x02 | 0xD2 |
| **Touch Screen** | Digitizers (0x0D) | 0x04 | 0xD4 |
| **Touch Pad** | Digitizers (0x0D) | 0x05 | 0xD5 |

Configuration Registers (Write-Only):  
Writing to any configuration register saves the new value to NVS and reboots the slave.

| Register Address | Name | Payload Description |
| :---- | :---- | :---- |
| 0xF0 | CONFIG\_USB\_IDS\_REG | 4 bytes: \[VID\_LSB, VID\_MSB, PID\_LSB, PID\_MSB\] |
| 0xF1 | CONFIG\_MANUFACTURER\_STR\_REG | Variable length, null-terminated UTF-8 string (max 63 chars). |
| 0xF2 | CONFIG\_PRODUCT\_STR\_REG | Variable length, null-terminated UTF-8 string (max 63 chars). |
| 0xF3 | CONFIG\_SERIAL\_STR\_REG | Variable length, null-terminated UTF-8 string (max 63 chars). |
| 0xF4 | CONFIG\_COMPOSITE\_DEVICE\_REG | 2 bytes (uint16\_t): A bitmap defining enabled HID interfaces. |
| 0xFE | CONFIG\_I2C\_ADDR\_REG | 1 byte: The new 7-bit I2C slave address. |

**Status Register (Read-Only):**

| Register Address | Name | R/W | Payload Description |
| :---- | :---- | :---- | :---- |
| 0xFF | STATUS\_REG | R | 1 byte: A bitmask representing the internal state of the slave. |

**Status Register Bit Definitions:**

| Bit | Value | Name | Description |
| :---- | :---- | :---- | :---- |
| 0 | 0x01 | STATUS\_OK | Set to 1 if the last command was successful. Cleared on read. |
| 1 | 0x02 | ERROR\_UNKNOWN\_REGISTER | An I2C write was attempted to an undefined register address. |
| 2 | 0x04 | ERROR\_PAYLOAD\_TOO\_LARGE | The master sent more data than expected for a given register. |
| 3 | 0x08 | ERROR\_INTERFACE\_DISABLED | The master sent a HID report for a device not enabled in the layout bitmap. |
| 4 | 0x10 | ERROR\_NVS\_WRITE\_FAILED | The slave failed to save a new configuration to NVS. |

#### **2.6. Boot Sequence and Persistence Logic**

1. **Check for Factory Reset**: On startup, check if a designated GPIO pin is held LOW. If so, erase the hidra NVS partition and reboot.  
2. **Load Configuration**: Read the complete configuration from NVS. For each item, if the NVS key is not found, load the hardcoded default value.  
3. **Build Descriptors**: Dynamically build the USB descriptor array for TinyUSB.  
4. **Initialize USB**: Pass the complete USB identity to TinyUSB for initialization.  
5. **Initialize I2C**: Use the I2C address (from NVS or default 0x70) to initialize the I2C slave peripheral.

#### **2.7. Bulletproof Address Configuration Workflow**

A **one-by-one provisioning workflow** must be used to avoid I2C bus collisions.  
**Procedure:**

1. **Connect ONE** unconfigured slave device to the master's I2C bus.  
2. The master application assigns a new, unique address (e.g., from 0x70 to 0x42).  
3. The slave saves the new address to NVS and reboots.  
4. **Connect the NEXT** unconfigured slave and repeat the process.

### **3\. Part B: The Master ESP-IDF Component (hidra)**

This component provides a clean API for controlling HIDra slaves and is designed to integrate safely into larger applications.

#### **3.1. Public API Design (hidra.h)**

\#pragma once

\#include "driver/i2c\_master.h"  
\#include "esp\_err.h"  
\#include "hidra\_protocol.h" // Includes the shared protocol definitions

// Opaque handles  
typedef i2c\_master\_bus\_handle\_t hidra\_bus\_handle\_t;  
typedef i2c\_master\_dev\_handle\_t hidra\_device\_handle\_t;

// \--- Bus Management \---  
esp\_err\_t hidra\_master\_bus\_init(i2c\_port\_num\_t i2c\_port, int sda\_io\_num, int scl\_io\_num, hidra\_bus\_handle\_t\* bus\_handle\_out);  
esp\_err\_t hidra\_master\_bus\_deinit(hidra\_bus\_handle\_t bus\_handle);

// \--- Device Management \---  
esp\_err\_t hidra\_add\_device\_to\_bus(hidra\_bus\_handle\_t bus\_handle, uint8\_t i2c\_address, hidra\_device\_handle\_t\* device\_handle\_out);  
esp\_err\_t hidra\_remove\_device\_from\_bus(hidra\_device\_handle\_t device\_handle);

// \--- HID Reporting & Status \---  
esp\_err\_t hidra\_send\_generic\_report(hidra\_device\_handle\_t device, uint8\_t hid\_register, const uint8\_t\* report, size\_t report\_size, int timeout\_ms);  
esp\_err\_t hidra\_read\_status(hidra\_device\_handle\_t device, uint8\_t\* status\_out, int timeout\_ms);

// \--- Device Configuration \---  
esp\_err\_t hidra\_set\_composite\_device\_config(hidra\_device\_handle\_t device, uint16\_t device\_bitmap, int timeout\_ms);  
esp\_err\_t hidra\_set\_usb\_ids(hidra\_device\_handle\_t device, uint16\_t vid, uint16\_t pid, int timeout\_ms);  
esp\_err\_t hidra\_set\_usb\_string(hidra\_device\_handle\_t device, uint8\_t config\_register, const char\* str, int timeout\_ms);  
esp\_err\_t hidra\_reconfigure\_address(hidra\_device\_handle\_t\* device\_handle\_ptr, uint8\_t new\_address, int timeout\_ms);

#### **3.2. Enterprise Usage Example (Application Owns Bus)**

This example shows how a main application can manage the I2C bus and allow HIDra to share it.  
// In main\_app.c  
\#include "hidra.h"  
\#include "some\_other\_i2c\_lib.h"

void app\_main(void) {  
    // 1\. Main application creates and owns the I2C bus  
    i2c\_master\_bus\_config\_t i2c\_bus\_config \= { /\* ... \*/ };  
    hidra\_bus\_handle\_t central\_bus\_handle;  
    i2c\_new\_master\_bus(\&i2c\_bus\_config, \&central\_bus\_handle);

    // 2\. Add and use a HIDra device  
    hidra\_device\_handle\_t hidra\_device1;  
    hidra\_add\_device\_to\_bus(central\_bus\_handle, 0x42, \&hidra\_device1);  
    hidra\_send\_generic\_report(hidra\_device1, HIDRA\_REG\_KEYBOARD, kbd\_data, 8, 100);

    // 3\. Check status after command  
    uint8\_t status;  
    hidra\_read\_status(hidra\_device1, \&status, 100);  
    if (\!(status & STATUS\_OK)) {  
        // Handle error...  
    }

    // 4\. Main application de-initializes everything  
    hidra\_remove\_device\_from\_bus(hidra\_device1);  
    i2c\_del\_master\_bus(central\_bus\_handle);  
}

### **4\. Part C: Development & Validation Strategy**

1. **Phase 1: Validate the Slave Firmware**: Develop the slave firmware and test it with a PC-based USB-to-I2C adapter and a Python script. Verify all HID reporting, configuration settings, and status register feedback.  
2. **Phase 2: Develop the Master Component**: Once the slave firmware is verified, develop the hidra component and test it against the known-good slave using the example application.

### **5\. Part D: Monorepo Project Structure**

This structure promotes portability, eliminates protocol errors, and provides a clear framework for automated testing.  
hidra-system/  
├── firmware/                   \# ESP-IDF project for the Slave Firmware  
│   ├── main/  
│   │   └── main.c              \# \<-- \#include "../../protocol/hidra\_protocol.h"  
│   └── CMakeLists.txt  
│  
├── libs/                       \# Platform-agnostic, reusable libraries  
│   └── hidra/                  \# The Master Library  
│       ├── hidra.h  
│       └── hidra.c             \# \<-- \#include "../../protocol/hidra\_protocol.h"  
│  
├── protocol/                   \# SINGLE SOURCE OF TRUTH for the I2C protocol  
│   └── hidra\_protocol.h  
│  
├── examples/  
│   └── master\_test\_app/        \# ESP-IDF project for the Test/Example Firmware  
│       ├── main/  
│       └── CMakeLists.txt      \# \<-- Points to ../../libs and ../../protocol  
│  
├── tests/  
│   ├── unit/                   \# Unit tests for the hidra library (e.g., using Ceedling)  
│   └── harness/                \# Python scripts for the USB-\>I2C adapter  
│  
├── docs/                       \# Documentation  
│   └── HIDra\_Blueprint.md  
│  
└── .gitignore  
