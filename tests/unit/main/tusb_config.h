#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU                OPT_MCU_ESP32S3
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_FREERTOS
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG              0
#endif

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENABLED
#define CFG_TUD_ENABLED             1
#endif

#define CFG_TUD_MAX_SPEED           OPT_MODE_DEFAULT_SPEED

#define CFG_TUD_HID                 4
#define CFG_TUD_CDC                 0
#define CFG_TUD_MSC                 0
#define CFG_TUD_MIDI                0
#define CFG_TUD_VENDOR              0

#define CFG_TUD_HID_EP_BUFSIZE      64

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
