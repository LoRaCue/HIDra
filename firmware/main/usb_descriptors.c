#include "usb_descriptors.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "usb_desc";

// USB descriptor constants
#define USB_VID_DEFAULT     0x413D
#define USB_PID_DEFAULT     0x0001
#define USB_BCD_DEVICE      0x0100
#define USB_CONFIG_TOTAL_LEN_BASE  (TUD_CONFIG_DESC_LEN)
#define USB_HID_DESC_LEN    (TUD_HID_DESC_LEN)
#define USB_HID_IN_EP_SIZE  64

// Interface and endpoint tracking
typedef struct {
    uint8_t hid_register;
    uint8_t interface_num;
    uint8_t endpoint_in;
    const uint8_t *report_desc;
    size_t report_desc_len;
    bool enabled;
} hid_interface_t;

// Global descriptor storage
static tusb_desc_device_t *g_device_desc = NULL;
static uint8_t *g_config_desc = NULL;
static uint16_t **g_string_desc = NULL;
static hid_interface_t g_hid_interfaces[8];
static uint8_t g_interface_count = 0;
static uint8_t g_string_count = 0;

// HID Report Descriptors
const uint8_t hid_report_descriptor_keyboard[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

const uint8_t hid_report_descriptor_mouse[] = {
    TUD_HID_REPORT_DESC_MOUSE()
};

const uint8_t hid_report_descriptor_gamepad[] = {
    TUD_HID_REPORT_DESC_GAMEPAD()
};

const uint8_t hid_report_descriptor_consumer[] = {
    TUD_HID_REPORT_DESC_CONSUMER()
};

const size_t hid_report_descriptor_keyboard_len = sizeof(hid_report_descriptor_keyboard);
const size_t hid_report_descriptor_mouse_len = sizeof(hid_report_descriptor_mouse);
const size_t hid_report_descriptor_gamepad_len = sizeof(hid_report_descriptor_gamepad);
const size_t hid_report_descriptor_consumer_len = sizeof(hid_report_descriptor_consumer);

// Helper functions
static uint16_t *create_string_descriptor(const char *str);
static void build_device_descriptor(const hidra_config_t *config);
static void build_configuration_descriptor(const hidra_config_t *config);
static void build_string_descriptors(const hidra_config_t *config);
static void setup_hid_interfaces(const hidra_config_t *config);

esp_err_t usb_descriptors_init(const hidra_config_t *config)
{
    ESP_LOGI(TAG, "Initializing USB descriptors");
    
    // Clean up any existing descriptors
    usb_descriptors_deinit();
    
    // Setup HID interface mapping
    setup_hid_interfaces(config);
    
    // Build descriptors
    build_device_descriptor(config);
    build_configuration_descriptor(config);
    build_string_descriptors(config);
    
    ESP_LOGI(TAG, "USB descriptors initialized - %d interfaces, %d strings", 
             g_interface_count, g_string_count);
    
    return ESP_OK;
}

void usb_descriptors_deinit(void)
{
    // Free device descriptor
    if (g_device_desc) {
        free(g_device_desc);
        g_device_desc = NULL;
    }
    
    // Free configuration descriptor
    if (g_config_desc) {
        free(g_config_desc);
        g_config_desc = NULL;
    }
    
    // Free string descriptors
    if (g_string_desc) {
        for (int i = 0; i < g_string_count; i++) {
            if (g_string_desc[i]) {
                free(g_string_desc[i]);
            }
        }
        free(g_string_desc);
        g_string_desc = NULL;
    }
    
    g_interface_count = 0;
    g_string_count = 0;
    memset(g_hid_interfaces, 0, sizeof(g_hid_interfaces));
}

static void setup_hid_interfaces(const hidra_config_t *config)
{
    uint8_t interface_num = 0;
    uint8_t endpoint_in = 0x81; // Start from EP1 IN
    
    // Define interface mappings
    struct {
        uint16_t layout_bit;
        uint8_t hid_register;
        const uint8_t *report_desc;
        size_t report_desc_len;
    } interface_map[] = {
        {LAYOUT_KEYBOARD, HIDRA_REG_KEYBOARD, hid_report_descriptor_keyboard, hid_report_descriptor_keyboard_len},
        {LAYOUT_MOUSE, HIDRA_REG_MOUSE, hid_report_descriptor_mouse, hid_report_descriptor_mouse_len},
        {LAYOUT_GAMEPAD, HIDRA_REG_GAMEPAD, hid_report_descriptor_gamepad, hid_report_descriptor_gamepad_len},
        {LAYOUT_CONSUMER, HIDRA_REG_CONSUMER, hid_report_descriptor_consumer, hid_report_descriptor_consumer_len},
    };
    
    g_interface_count = 0;
    
    for (int i = 0; i < sizeof(interface_map) / sizeof(interface_map[0]); i++) {
        if (config->composite_layout & interface_map[i].layout_bit) {
            g_hid_interfaces[g_interface_count] = (hid_interface_t){
                .hid_register = interface_map[i].hid_register,
                .interface_num = interface_num++,
                .endpoint_in = endpoint_in++,
                .report_desc = interface_map[i].report_desc,
                .report_desc_len = interface_map[i].report_desc_len,
                .enabled = true
            };
            g_interface_count++;
        }
    }
    
    ESP_LOGI(TAG, "Setup %d HID interfaces", g_interface_count);
}

static void build_device_descriptor(const hidra_config_t *config)
{
    g_device_desc = malloc(sizeof(tusb_desc_device_t));
    if (!g_device_desc) {
        ESP_LOGE(TAG, "Failed to allocate device descriptor");
        return;
    }
    
    *g_device_desc = (tusb_desc_device_t){
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = 0x0200,
        .bDeviceClass       = 0x00,
        .bDeviceSubClass    = 0x00,
        .bDeviceProtocol    = 0x00,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor           = config->usb_vid,
        .idProduct          = config->usb_pid,
        .bcdDevice          = USB_BCD_DEVICE,
        .iManufacturer      = 1,
        .iProduct           = 2,
        .iSerialNumber      = 3,
        .bNumConfigurations = 1
    };
}

static void build_configuration_descriptor(const hidra_config_t *config)
{
    // Calculate total length
    uint16_t total_len = TUD_CONFIG_DESC_LEN + (g_interface_count * TUD_HID_DESC_LEN);
    
    g_config_desc = malloc(total_len);
    if (!g_config_desc) {
        ESP_LOGE(TAG, "Failed to allocate configuration descriptor");
        return;
    }
    
    uint8_t *desc = g_config_desc;
    
    // Configuration descriptor
    *desc++ = 9;                        // bLength
    *desc++ = TUSB_DESC_CONFIGURATION;  // bDescriptorType
    *desc++ = total_len & 0xFF;         // wTotalLength LSB
    *desc++ = (total_len >> 8) & 0xFF;  // wTotalLength MSB
    *desc++ = g_interface_count;        // bNumInterfaces
    *desc++ = 1;                        // bConfigurationValue
    *desc++ = 0;                        // iConfiguration
    *desc++ = 0x80;                     // bmAttributes (bus powered)
    *desc++ = 100;                      // bMaxPower (200mA)
    
    // Add HID interface descriptors
    for (int i = 0; i < g_interface_count; i++) {
        hid_interface_t *hid = &g_hid_interfaces[i];
        
        // Interface descriptor
        *desc++ = 9;                    // bLength
        *desc++ = TUSB_DESC_INTERFACE;  // bDescriptorType
        *desc++ = hid->interface_num;   // bInterfaceNumber
        *desc++ = 0;                    // bAlternateSetting
        *desc++ = 1;                    // bNumEndpoints
        *desc++ = TUSB_CLASS_HID;       // bInterfaceClass
        *desc++ = 0;                    // bInterfaceSubClass
        *desc++ = 0;                    // bInterfaceProtocol
        *desc++ = 0;                    // iInterface
        
        // HID descriptor
        *desc++ = 9;                    // bLength
        *desc++ = HID_DESC_TYPE_HID;    // bDescriptorType
        *desc++ = 0x11;                 // bcdHID LSB
        *desc++ = 0x01;                 // bcdHID MSB
        *desc++ = 0;                    // bCountryCode
        *desc++ = 1;                    // bNumDescriptors
        *desc++ = HID_DESC_TYPE_REPORT; // bDescriptorType
        *desc++ = hid->report_desc_len & 0xFF;        // wDescriptorLength LSB
        *desc++ = (hid->report_desc_len >> 8) & 0xFF; // wDescriptorLength MSB
        
        // Endpoint descriptor
        *desc++ = 7;                    // bLength
        *desc++ = TUSB_DESC_ENDPOINT;   // bDescriptorType
        *desc++ = hid->endpoint_in;     // bEndpointAddress
        *desc++ = TUSB_XFER_INTERRUPT;  // bmAttributes
        *desc++ = USB_HID_IN_EP_SIZE;   // wMaxPacketSize LSB
        *desc++ = 0;                    // wMaxPacketSize MSB
        *desc++ = 10;                   // bInterval
    }
}

static void build_string_descriptors(const hidra_config_t *config)
{
    g_string_count = 4; // Language, Manufacturer, Product, Serial
    g_string_desc = malloc(g_string_count * sizeof(uint16_t*));
    if (!g_string_desc) {
        ESP_LOGE(TAG, "Failed to allocate string descriptor array");
        return;
    }
    
    // Language descriptor
    g_string_desc[0] = malloc(4);
    g_string_desc[0][0] = (TUSB_DESC_STRING << 8) | 4;
    g_string_desc[0][1] = 0x0409; // English (US)
    
    // String descriptors
    g_string_desc[1] = create_string_descriptor(config->manufacturer);
    g_string_desc[2] = create_string_descriptor(config->product);
    g_string_desc[3] = create_string_descriptor(config->serial);
}

static uint16_t *create_string_descriptor(const char *str)
{
    size_t len = strlen(str);
    uint16_t *desc = malloc((len + 1) * 2);
    if (!desc) return NULL;
    
    desc[0] = (TUSB_DESC_STRING << 8) | ((len + 1) * 2);
    for (size_t i = 0; i < len; i++) {
        desc[i + 1] = str[i];
    }
    return desc;
}

// TinyUSB descriptor callbacks
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)g_device_desc;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return g_config_desc;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    if (index >= g_string_count) return NULL;
    return g_string_desc[index];
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    if (instance >= g_interface_count) return NULL;
    return g_hid_interfaces[instance].report_desc;
}

// Interface management functions
uint8_t usb_get_hid_instance_for_register(uint8_t hid_register)
{
    for (uint8_t i = 0; i < g_interface_count; i++) {
        if (g_hid_interfaces[i].hid_register == hid_register) {
            return i;
        }
    }
    return 0xFF; // Not found
}

bool usb_is_interface_enabled(uint8_t hid_register)
{
    for (uint8_t i = 0; i < g_interface_count; i++) {
        if (g_hid_interfaces[i].hid_register == hid_register) {
            return g_hid_interfaces[i].enabled;
        }
    }
    return false;
}
