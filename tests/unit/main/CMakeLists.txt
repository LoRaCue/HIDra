idf_component_register(SRCS "test_main.c" 
                              "test_protocol.c"
                              "test_hidra_master.c"
                              "test_usb_descriptors.c"
                              "test_config.c"
                              "test_i2c_protocol.c"
                              "test_status_register.c"
                              "test_hid_reports.c"
                              "../../../firmware/main/usb_descriptors.c"
                    INCLUDE_DIRS "." "../../../firmware/main"
                    REQUIRES unity hidra
                    PRIV_REQUIRES tinyusb)
