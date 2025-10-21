#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "driver/i2c_slave.h"
#include "tinyusb.h"
#include "tusb.h"
#include "hidra_protocol.h"
#include "usb_descriptors.h"
#include "version.h"

static const char *TAG = "hidra_slave";

// Device configuration structure
typedef struct {
    uint8_t i2c_addr;
    uint16_t usb_vid;
    uint16_t usb_pid;
    char manufacturer[MAX_STRING_LENGTH + 1];
    char product[MAX_STRING_LENGTH + 1];
    char serial[MAX_STRING_LENGTH + 1];
    uint16_t composite_layout;
} hidra_config_t;

// Global variables
static hidra_config_t g_config;
static uint8_t g_status_register = 0;
static i2c_slave_dev_handle_t g_i2c_slave_handle = NULL;
static QueueHandle_t g_hid_queue = NULL;

// HID report structure
typedef struct {
    uint8_t hid_register;
    uint8_t report[MAX_REPORT_SIZE];
    size_t report_size;
} hid_report_t;

// Function prototypes
static void load_config_from_nvs(void);
static void save_config_to_nvs(void);
static void generate_serial_from_mac(char *serial_out);
static void factory_reset_check(void);
static void i2c_task(void *pvParameters);
static void usb_task(void *pvParameters);
static void handle_hid_report(uint8_t reg_addr, const uint8_t *data, size_t len);
static void handle_config_command(uint8_t reg_addr, const uint8_t *data, size_t len);
static void handle_i2c_command(uint8_t reg_addr, const uint8_t *data, size_t len);
static void set_status_bit(uint8_t bit);
static void clear_status_bit(uint8_t bit);
static esp_err_t init_usb_system(void);

void app_main(void)
{
    // Print version information first
    firmware_print_version_info();
    
    ESP_LOGI(TAG, "HIDra Slave Firmware Starting");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Check for factory reset
    factory_reset_check();

    // Load configuration from NVS
    load_config_from_nvs();

    // Initialize USB system with dynamic descriptors
    ESP_ERROR_CHECK(init_usb_system());

    // Create HID report queue
    g_hid_queue = xQueueCreate(10, sizeof(hid_report_t));
    if (g_hid_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create HID queue");
        return;
    }

    // Initialize I2C slave
    i2c_slave_config_t i2c_slv_config = {
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .send_buf_depth = 256,
        .scl_io_num = GPIO_NUM_5,
        .sda_io_num = GPIO_NUM_4,
        .slave_addr = g_config.i2c_addr,
    };

    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &g_i2c_slave_handle));

    // Create tasks
    xTaskCreate(i2c_task, "i2c_task", 4096, NULL, 5, NULL);
    xTaskCreate(usb_task, "usb_task", 4096, NULL, 4, NULL);

    ESP_LOGI(TAG, "HIDra Slave initialized - I2C addr: 0x%02X, VID: 0x%04X, PID: 0x%04X, Layout: 0x%04X", 
             g_config.i2c_addr, g_config.usb_vid, g_config.usb_pid, g_config.composite_layout);
}

static void load_config_from_nvs(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    
    // Set defaults first
    g_config.i2c_addr = DEFAULT_I2C_ADDR;
    g_config.usb_vid = DEFAULT_USB_VID;
    g_config.usb_pid = DEFAULT_USB_PID;
    strcpy(g_config.manufacturer, DEFAULT_MANUFACTURER);
    strcpy(g_config.product, DEFAULT_PRODUCT);
    generate_serial_from_mac(g_config.serial);
    g_config.composite_layout = DEFAULT_COMPOSITE_LAYOUT;

    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS not found, using defaults");
        return;
    }

    // Load values from NVS, keeping defaults if not found
    size_t required_size;
    
    nvs_get_u8(nvs_handle, NVS_KEY_I2C_ADDR, &g_config.i2c_addr);
    nvs_get_u16(nvs_handle, NVS_KEY_USB_VID, &g_config.usb_vid);
    nvs_get_u16(nvs_handle, NVS_KEY_USB_PID, &g_config.usb_pid);
    nvs_get_u16(nvs_handle, NVS_KEY_COMPOSITE_LAYOUT, &g_config.composite_layout);
    
    required_size = sizeof(g_config.manufacturer);
    nvs_get_str(nvs_handle, NVS_KEY_MANUFACTURER, g_config.manufacturer, &required_size);
    
    required_size = sizeof(g_config.product);
    nvs_get_str(nvs_handle, NVS_KEY_PRODUCT, g_config.product, &required_size);
    
    required_size = sizeof(g_config.serial);
    nvs_get_str(nvs_handle, NVS_KEY_SERIAL, g_config.serial, &required_size);

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Configuration loaded from NVS");
}

static void save_config_to_nvs(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        set_status_bit(ERROR_NVS_WRITE_FAILED);
        return;
    }

    nvs_set_u8(nvs_handle, NVS_KEY_I2C_ADDR, g_config.i2c_addr);
    nvs_set_u16(nvs_handle, NVS_KEY_USB_VID, g_config.usb_vid);
    nvs_set_u16(nvs_handle, NVS_KEY_USB_PID, g_config.usb_pid);
    nvs_set_u16(nvs_handle, NVS_KEY_COMPOSITE_LAYOUT, g_config.composite_layout);
    nvs_set_str(nvs_handle, NVS_KEY_MANUFACTURER, g_config.manufacturer);
    nvs_set_str(nvs_handle, NVS_KEY_PRODUCT, g_config.product);
    nvs_set_str(nvs_handle, NVS_KEY_SERIAL, g_config.serial);

    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    if (err != ESP_OK) {
        set_status_bit(ERROR_NVS_WRITE_FAILED);
    } else {
        ESP_LOGI(TAG, "Configuration saved to NVS");
    }
}

static void generate_serial_from_mac(char *serial_out)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(serial_out, MAX_STRING_LENGTH, "HIDra-%02X%02X%02X%02X%02X%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void factory_reset_check(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << FACTORY_RESET_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    if (gpio_get_level(FACTORY_RESET_GPIO) == 0) {
        ESP_LOGW(TAG, "Factory reset requested");
        nvs_flash_erase_partition(NVS_NAMESPACE);
        esp_restart();
    }
}

static void i2c_task(void *pvParameters)
{
    uint8_t buffer[MAX_REPORT_SIZE + 1]; // +1 for register address
    
    while (1) {
        size_t size = 0;
        esp_err_t ret = i2c_slave_receive(g_i2c_slave_handle, buffer, sizeof(buffer), &size, portMAX_DELAY);
        
        if (ret == ESP_OK && size >= 1) {
            uint8_t reg_addr = buffer[0];
            
            // Handle status register read
            if (reg_addr == STATUS_REG && size == 1) {
                // This is a status register read request
                uint8_t status = g_status_register;
                g_status_register = 0; // Clear on read
                
                ret = i2c_slave_transmit(g_i2c_slave_handle, &status, 1, 1000);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to send status: %s", esp_err_to_name(ret));
                }
            } else if (size > 1) {
                // This is a write command
                handle_i2c_command(reg_addr, &buffer[1], size - 1);
            }
        }
    }
}

static void usb_task(void *pvParameters)
{
    while (1) {
        tud_task();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void handle_hid_report(uint8_t reg_addr, const uint8_t *data, size_t len)
{
    // Check if interface is enabled
    uint8_t interface_bit = 0;
    switch (reg_addr) {
        case HIDRA_REG_KEYBOARD: interface_bit = LAYOUT_KEYBOARD; break;
        case HIDRA_REG_MOUSE: interface_bit = LAYOUT_MOUSE; break;
        case HIDRA_REG_GAMEPAD: interface_bit = LAYOUT_GAMEPAD; break;
        case HIDRA_REG_JOYSTICK: interface_bit = LAYOUT_JOYSTICK; break;
        case HIDRA_REG_CONSUMER: interface_bit = LAYOUT_CONSUMER; break;
        case HIDRA_REG_PEN: interface_bit = LAYOUT_PEN; break;
        case HIDRA_REG_TOUCHSCREEN: interface_bit = LAYOUT_TOUCHSCREEN; break;
        case HIDRA_REG_TOUCHPAD: interface_bit = LAYOUT_TOUCHPAD; break;
    }

    if (!(g_config.composite_layout & interface_bit)) {
        set_status_bit(ERROR_INTERFACE_DISABLED);
        return;
    }

    if (len > MAX_REPORT_SIZE) {
        set_status_bit(ERROR_PAYLOAD_TOO_LARGE);
        return;
    }

    // Queue HID report
    hid_report_t report = {
        .hid_register = reg_addr,
        .report_size = len
    };
    memcpy(report.report, data, len);

    if (xQueueSend(g_hid_queue, &report, 0) == pdTRUE) {
        set_status_bit(STATUS_OK);
    }
}

static void handle_config_command(uint8_t reg_addr, const uint8_t *data, size_t len)
{
    switch (reg_addr) {
        case CONFIG_USB_IDS_REG:
            if (len == 4) {
                g_config.usb_vid = (data[1] << 8) | data[0];
                g_config.usb_pid = (data[3] << 8) | data[2];
                save_config_to_nvs();
                esp_restart();
            } else {
                set_status_bit(ERROR_PAYLOAD_TOO_LARGE);
            }
            break;

        case CONFIG_MANUFACTURER_STR_REG:
        case CONFIG_PRODUCT_STR_REG:
        case CONFIG_SERIAL_STR_REG:
            if (len <= MAX_STRING_LENGTH) {
                char *target = NULL;
                switch (reg_addr) {
                    case CONFIG_MANUFACTURER_STR_REG: target = g_config.manufacturer; break;
                    case CONFIG_PRODUCT_STR_REG: target = g_config.product; break;
                    case CONFIG_SERIAL_STR_REG: target = g_config.serial; break;
                }
                memcpy(target, data, len);
                target[len] = '\0';
                save_config_to_nvs();
                esp_restart();
            } else {
                set_status_bit(ERROR_PAYLOAD_TOO_LARGE);
            }
            break;

        case CONFIG_COMPOSITE_DEVICE_REG:
            if (len == 2) {
                g_config.composite_layout = (data[1] << 8) | data[0];
                save_config_to_nvs();
                esp_restart();
            } else {
                set_status_bit(ERROR_PAYLOAD_TOO_LARGE);
            }
            break;

        case CONFIG_I2C_ADDR_REG:
            if (len == 1) {
                g_config.i2c_addr = data[0];
                save_config_to_nvs();
                esp_restart();
            } else {
                set_status_bit(ERROR_PAYLOAD_TOO_LARGE);
            }
            break;

        default:
            set_status_bit(ERROR_UNKNOWN_REGISTER);
            break;
    }
}

static void handle_i2c_command(uint8_t reg_addr, const uint8_t *data, size_t len)
{
    // Clear previous status
    g_status_register = 0;

    if (reg_addr >= HIDRA_REG_KEYBOARD && reg_addr <= HIDRA_REG_TOUCHPAD) {
        handle_hid_report(reg_addr, data, len);
    } else if (reg_addr >= CONFIG_USB_IDS_REG && reg_addr <= CONFIG_I2C_ADDR_REG) {
        handle_config_command(reg_addr, data, len);
    } else {
        set_status_bit(ERROR_UNKNOWN_REGISTER);
    }
}

static void set_status_bit(uint8_t bit)
{
    g_status_register |= bit;
}

static void clear_status_bit(uint8_t bit)
{
    g_status_register &= ~bit;
}

// TinyUSB callbacks (minimal implementation)
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "USB mounted");
}

void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "USB unmounted");
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}
