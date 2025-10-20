idf_component_register(
    SRCS "main.c" "usb_descriptors.c" "version.c"
    INCLUDE_DIRS "." "${CMAKE_CURRENT_BINARY_DIR}/../"
    REQUIRES freertos esp_system nvs_flash driver tinyusb hidra
)
