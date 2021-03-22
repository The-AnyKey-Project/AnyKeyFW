/*
 * usb.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
 *      This code was heavily inspired by:
 *      https://git.drak.xyz/flabbergast/chibios-projects/src/branch/master/projects/keyb
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/hal/usb.h"

/*
 * Include dependencies
 */
#include <string.h>

/*
 * Forward declarations of static functions
 */
static void _usb_init_hal(void);
static void _usb_init_module(void);
static bool _usb_hid_request_hook(USBDriver *usbp);
static void _usb_hid_kbd_report_pool_next(void);
static bool _usb_hid_kbd_send_report(bool is_in_cb);
static bool _usb_hid_raw_start_receive(void);
static void _usb_hid_raw_sof_hook(void);
static void _usb_hid_raw_configured_hook(void);
static void _usb_event_cb(USBDriver *usbp, usbevent_t event);
static const USBDescriptor *_usb_get_descriptor_cb(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                                   uint16_t lang);
static bool _usb_request_hook_cb(USBDriver *usbp);
static void _usb_sof_cb(USBDriver *usbp);
static void _usb_hid_kbd_idle_timer_cb(void *arg);
static void _usb_hid_kbd_report_timer_cb(void *arg);
static void _usb_hid_raw_out_cb(USBDriver *usbp, usbep_t ep);
static void _usb_hid_raw_in_cb(USBDriver *usbp, usbep_t ep);
static void _usb_hid_raw_ibnotify_cb(io_buffers_queue_t *bqp);
static void _usb_hid_raw_obnotify_cb(io_buffers_queue_t *bqp);
static USB_CALLBACK_STUB(_usb_hid_kbd_in_cb);
static USB_CALLBACK_STUB(_usb_hid_kbdext_in_cb);

/*
 * Static variables
 */

static uint8_t _usb_hid_kbd_idle = 0;
static uint8_t _usb_hid_kbd_protocol = 1;
static virtual_timer_t _usb_hid_kbd_idle_timer;
static virtual_timer_t _usb_hid_kbd_report_timer;
static usb_hid_kbd_report_t _usb_hid_kbd_report_pool[USB_HID_KBD_REPORT_POOLSIZE];
static usb_hid_kbd_report_t *_usb_hid_kbd_report_prepare = (usb_hid_kbd_report_t *)NULL;
static usb_hid_kbd_report_t *_usb_hid_kbd_report_inflight = (usb_hid_kbd_report_t *)NULL;
static uint8_t _usb_hid_report_payload_idx = 0;
static uint8_t _usb_hid_report_idx = 0;
static uint8_t _usb_hid_report_retry_counter = 0;
static uint8_t
    _usb_hid_raw_input_buffer[BQ_BUFFER_SIZE(USB_HID_RAW_INPUT_BUFFER_ENTRIES, USB_HID_RAW_EPSIZE)];
static uint8_t _usb_hid_raw_output_buffer[BQ_BUFFER_SIZE(USB_HID_RAW_OUTPUT_BUFFER_ENTRIES,
                                                         USB_HID_RAW_EPSIZE)];
static input_buffers_queue_t _usb_hid_raw_input_queue;
static input_buffers_queue_t _usb_hid_raw_output_queue;

/*
 * USB Device Descriptor.
 */
static const uint8_t usb_device_descriptor_data[] = {
    USB_DESC_DEVICE(0x0200,          // bcdUSB (1.1)
                    0xef,            // bDeviceClass (misc)
                    0x02,            // bDeviceSubClass (common)
                    0x01,            // bDeviceProtocol (IAD)
                    64,              // bMaxPacketSize (64 bytes) (the driver didn't work with 32)
                    USB_VENDOR_ID,   // idVendor
                    USB_PRODUCT_ID,  // idProduct
                    USB_DEVICE_VER,  // bcdDevice
                    1,               // iManufacturer
                    2,               // iProduct
                    3,               // iSerialNumber
                    1)               // bNumConfigurations
};

/*
 * Device Descriptor wrapper
 */
static const USBDescriptor _usb_device_descriptor = {sizeof usb_device_descriptor_data,
                                                     usb_device_descriptor_data};

/*
 * HID Report Descriptor
 *
 * See "Device Class Definition for Human Interface Devices (HID)"
 * (http://www.usb.org/developers/hidpage/HID1_11.pdf) for the
 * detailed descrition of all the fields
 */

/* Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60 */
static const uint8_t _usb_hid_kbd_report_desc_data[] = {
    0x05, 0x01,                     // Usage Page (Generic Desktop),
    0x09, 0x06,                     // Usage (Keyboard),
    0xA1, 0x01,                     // Collection (Application),
    0x75, 0x01,                     //   Report Size (1),
    0x95, 0x08,                     //   Report Count (8),
    0x05, 0x07,                     //   Usage Page (Key Codes),
    0x19, 0xE0,                     //   Usage Minimum (224),
    0x29, 0xE7,                     //   Usage Maximum (231),
    0x15, 0x00,                     //   Logical Minimum (0),
    0x25, 0x01,                     //   Logical Maximum (1),
    0x81, 0x02,                     //   Input (Data, Variable, Absolute), ;Modifier byte
    0x95, 0x01,                     //   Report Count (1),
    0x75, 0x08,                     //   Report Size (8),
    0x81, 0x03,                     //   Input (Constant),                 ;Reserved byte
    0x95, 0x05,                     //   Report Count (5),
    0x75, 0x01,                     //   Report Size (1),
    0x05, 0x08,                     //   Usage Page (LEDs),
    0x19, 0x01,                     //   Usage Minimum (1),
    0x29, 0x05,                     //   Usage Maximum (5),
    0x91, 0x02,                     //   Output (Data, Variable, Absolute), ;LED report
    0x95, 0x01,                     //   Report Count (1),
    0x75, 0x03,                     //   Report Size (3),
    0x91, 0x03,                     //   Output (Constant),                 ;LED report padding
    0x95, USB_HID_KBD_REPORT_KEYS,  //   Report Count (),
    0x75, 0x08,                     //   Report Size (8),
    0x15, 0x00,                     //   Logical Minimum (0),
    0x25, 0xFF,                     //   Logical Maximum(255),
    0x05, 0x07,                     //   Usage Page (Key Codes),
    0x19, 0x00,                     //   Usage Minimum (0),
    0x29, 0xFF,                     //   Usage Maximum (255),
    0x81, 0x00,                     //   Input (Data, Array),
    0xc0                            // End Collection
};

/* audio controls & system controls
 * http://www.microsoft.com/whdc/archive/w2kbd.mspx */
static const uint8_t _usb_hid_kbdext_report_desc_data[] = {
    /* system control */
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x80,                      // USAGE (System Control)
    0xa1, 0x01,                      // COLLECTION (Application)
    0x85, USB_HID_REPORT_ID_SYSTEM,  //   REPORT_ID (2)
    0x15, 0x01,                      //   LOGICAL_MINIMUM (0x1)
    0x25, 0xb7,                      //   LOGICAL_MAXIMUM (0xb7)
    0x19, 0x01,                      //   USAGE_MINIMUM (0x1)
    0x29, 0xb7,                      //   USAGE_MAXIMUM (0xb7)
    0x75, 0x10,                      //   REPORT_SIZE (16)
    0x95, 0x01,                      //   REPORT_COUNT (1)
    0x81, 0x00,                      //   INPUT (Data,Array,Abs)
    0xc0,                            // END_COLLECTION
    /* consumer */
    0x05, 0x0c,                        // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,                        // USAGE (Consumer Control)
    0xa1, 0x01,                        // COLLECTION (Application)
    0x85, USB_HID_REPORT_ID_CONSUMER,  //   REPORT_ID (3)
    0x15, 0x01,                        //   LOGICAL_MINIMUM (0x1)
    0x26, 0x9c, 0x02,                  //   LOGICAL_MAXIMUM (0x29c)
    0x19, 0x01,                        //   USAGE_MINIMUM (0x1)
    0x2a, 0x9c, 0x02,                  //   USAGE_MAXIMUM (0x29c)
    0x75, 0x10,                        //   REPORT_SIZE (16)
    0x95, 0x01,                        //   REPORT_COUNT (1)
    0x81, 0x00,                        //   INPUT (Data,Array,Abs)
    0xc0,                              // END_COLLECTION
};

static const uint8_t _usb_hid_raw_report_desc_data[] = {
    0x06, 0x60,
    0xFF,                       // Usage Page 0xFF60 (vendor defined)
    0x09, 0x61,                 // Usage 0x61
    0xA1, 0x01,                 // Collection 0x01
    0x09, 0x62,                 //   Usage 0x62
    0x15, 0x00,                 //   logical minimum = 0
    0x25, 0xFF,                 //   logical maximum = 255
    0x95, USB_HID_RAW_EPSIZE,   //   report count
    0x75, 0x08,                 //   REPORT_SIZE (8)
    0x81, USB_HID_RAW_IOF_IN,   //   I/O Feature descriptor
    0x09, 0x63,                 //   Usage 0x63
    0x15, 0x00,                 //   logical minimum = 0
    0x25, 0xFF,                 //   logical maximum = 255
    0x95, USB_HID_RAW_EPSIZE,   //   report count
    0x75, 0x08,                 //   REPORT_SIZE (8)
    0x91, USB_HID_RAW_IOF_OUT,  //   I/O Feature descriptor
    0xC0                        // end collection
};

/*
 * wrapper
 */
static const USBDescriptor _usb_hid_kbd_report_descriptor = {sizeof _usb_hid_kbd_report_desc_data,
                                                             _usb_hid_kbd_report_desc_data};
static const USBDescriptor _usb_hid_kbdext_report_descriptor = {
    sizeof _usb_hid_kbdext_report_desc_data, _usb_hid_kbdext_report_desc_data};
static const USBDescriptor _usb_hid_raw_report_descriptor = {sizeof _usb_hid_raw_report_desc_data,
                                                             _usb_hid_raw_report_desc_data};

/*
 * Configuration Descriptor tree for a HID device
 *
 * The HID Specifications version 1.11 require the following order:
 * - Configuration Descriptor
 * - Interface Descriptor
 * - HID Descriptor
 * - Endpoints Descriptors
 */

static const uint8_t _usb_configuration_descriptor_data[] = {
    /* Configuration Descriptor (9 bytes) USB spec 9.6.3, page 264-266, Table 9-10 */
    USB_DESC_CONFIGURATION(USB_CONFIG_DESC_DATA_SIZE,  // wTotalLength
                           USB_NUM_INTERFACES,         // bNumInterfaces
                           1,                          // bConfigurationValue
                           0,                          // iConfiguration
                           0xA0,                       // bmAttributes (RESERVED|REMOTEWAKEUP)
                           250),                       // bMaxPower (500mA)

    USB_DESC_INTERFACE_ASSOCIATION(USB_HID_KBD_INTERFACE,   /* bFirstInterface.         */
                                   3,                       /* bInterfaceCount.         */
                                   USB_CONFIG_HID_CLASS,    /* bFunctionClass.          */
                                   USB_CONFIG_HID_SUBCLASS, /* bFunctionSubClass.       */
                                   1,                       /* bFunctionProcotol.       */
                                   0                        /* iInterface.              */
                                   ),

    /* Interface Descriptor (9 bytes) USB spec 9.6.5, page 267-269, Table 9-12 */
    USB_DESC_INTERFACE(USB_HID_KBD_INTERFACE,    // bInterfaceNumber
                       0,                        // bAlternateSetting
                       1,                        // bNumEndpoints
                       USB_CONFIG_HID_CLASS,     // bInterfaceClass: HID
                       USB_CONFIG_HID_SUBCLASS,  // bInterfaceSubClass: Boot
                       0x01,                     // bInterfaceProtocol: Keyboard
                       0),                       // iInterface

    /* HID descriptor (9 bytes) HID 1.11 spec, section 6.2.1 */
    USB_DESC_BYTE(9),                                      // bLength
    USB_DESC_BYTE(USB_DESCRIPTOR_HID),                     // bDescriptorType (HID class)
    USB_DESC_BCD(0x0111),                                  // bcdHID: HID version 1.11
    USB_DESC_BYTE(0),                                      // bCountryCode
    USB_DESC_BYTE(1),                                      // bNumDescriptors
    USB_DESC_BYTE(USB_DESCRIPTOR_HID_REPORT),              // bDescriptorType (report desc)
    USB_DESC_WORD(sizeof(_usb_hid_kbd_report_desc_data)),  // wDescriptorLength

    /* Endpoint Descriptor (7 bytes) USB spec 9.6.6, page 269-271, Table 9-13 */
    USB_DESC_ENDPOINT(USB_HID_KBD_EP | 0x80,  // bEndpointAddress
                      USB_EP_MODE_TYPE_INTR,  // bmAttributes (Interrupt)
                      USB_HID_KBD_EPSIZE,     // wMaxPacketSize
                      10),                    // bInterval

    /* Interface Descriptor (9 bytes) USB spec 9.6.5, page 267-269, Table 9-12 */
    USB_DESC_INTERFACE(USB_HID_KBDEXT_INTERFACE,  // bInterfaceNumber
                       0,                         // bAlternateSetting
                       1,                         // bNumEndpoints
                       USB_CONFIG_HID_CLASS,      // bInterfaceClass: HID
                       0x00,                      // bInterfaceSubClass: None
                       0x00,                      // bInterfaceProtocol: None
                       0),                        // iInterface

    /* HID descriptor (9 bytes) HID 1.11 spec, section 6.2.1 */
    USB_DESC_BYTE(9),                                         // bLength
    USB_DESC_BYTE(USB_DESCRIPTOR_HID),                        // bDescriptorType (HID class)
    USB_DESC_BCD(0x0111),                                     // bcdHID: HID version 1.11
    USB_DESC_BYTE(0),                                         // bCountryCode
    USB_DESC_BYTE(1),                                         // bNumDescriptors
    USB_DESC_BYTE(USB_DESCRIPTOR_HID_REPORT),                 // bDescriptorType (report desc)
    USB_DESC_WORD(sizeof(_usb_hid_kbdext_report_desc_data)),  // wDescriptorLength

    /* Endpoint Descriptor (7 bytes) USB spec 9.6.6, page 269-271, Table 9-13 */
    USB_DESC_ENDPOINT(USB_HID_KBDEXT_EP | 0x80,  // bEndpointAddress
                      USB_EP_MODE_TYPE_INTR,     // bmAttributes (Interrupt)
                      USB_HID_KBDEXT_EPSIZE,     // wMaxPacketSize
                      1),                        // bInterval

    /* Interface Descriptor (9 bytes) USB spec 9.6.5, page 267-269, Table 9-12 */
    USB_DESC_INTERFACE(USB_HID_RAW_INTERFACE,  // bInterfaceNumber
                       0,                      // bAlternateSetting
                       2,                      // bNumEndpoints
                       USB_CONFIG_HID_CLASS,   // bInterfaceClass: HID
                       0x00,                   // bInterfaceSubClass: None
                       0x00,                   // bInterfaceProtocol: None
                       0),                     // iInterface

    /* HID descriptor (9 bytes) HID 1.11 spec, section 6.2.1 */
    USB_DESC_BYTE(9),                                      // bLength
    USB_DESC_BYTE(USB_DESCRIPTOR_HID),                     // bDescriptorType (HID class)
    USB_DESC_BCD(0x0111),                                  // bcdHID: HID version 1.11
    USB_DESC_BYTE(0),                                      // bCountryCode
    USB_DESC_BYTE(1),                                      // bNumDescriptors
    USB_DESC_BYTE(USB_DESCRIPTOR_HID_REPORT),              // bDescriptorType (report desc)
    USB_DESC_WORD(sizeof(_usb_hid_raw_report_desc_data)),  // wDescriptorLength

    /* Endpoint Descriptor (7 bytes) USB spec 9.6.6, page 269-271, Table 9-13 */
    USB_DESC_ENDPOINT(USB_HID_RAW_EP,         // bEndpointAddress
                      USB_EP_MODE_TYPE_INTR,  // bmAttributes (Interrupt)
                      USB_HID_RAW_EPSIZE,     // wMaxPacketSize
                      10),                    // bInterval
    /* Endpoint Descriptor (7 bytes) USB spec 9.6.6, page 269-271, Table 9-13 */
    USB_DESC_ENDPOINT(USB_HID_RAW_EP | 0x80,  // bEndpointAddress
                      USB_EP_MODE_TYPE_INTR,  // bmAttributes (Interrupt)
                      USB_HID_RAW_EPSIZE,     // wMaxPacketSize
                      10),                    // bInterval

    USB_DESC_INTERFACE_ASSOCIATION(USB_CDC_INT_INTERFACE,             /* bFirstInterface.         */
                                   2,                                 /* bInterfaceCount.         */
                                   CDC_COMMUNICATION_INTERFACE_CLASS, /* bFunctionClass.          */
                                   CDC_ABSTRACT_CONTROL_MODEL,        /* bFunctionSubClass.       */
                                   1,                                 /* bFunctionProcotol.       */
                                   0                                  /* iInterface.              */
                                   ),

    /* Interface Descriptor.*/
    USB_DESC_INTERFACE(USB_CDC_INT_INTERFACE,             /* bInterfaceNumber.                */
                       0x00,                              /* bAlternateSetting.               */
                       0x01,                              /* bNumEndpoints.                   */
                       CDC_COMMUNICATION_INTERFACE_CLASS, /* bInterfaceClass (Communications */
                                                          /* Interface Class, CDC section */
                                                          /* 4.2).                            */
                       CDC_ABSTRACT_CONTROL_MODEL,        /* bInterfaceSubClass (Abstract */
                                                          /* Control Model, CDC section 4.3).   */
                       0x01,                              /* bInterfaceProtocol (AT commands,
                                                             CDC section 4.4).                */
                       0),                                /* iInterface.                      */

    /* Header Functional Descriptor (CDC section 5.2.3).*/
    USB_DESC_BYTE(5),                /* bLength.                         */
    USB_DESC_BYTE(CDC_CS_INTERFACE), /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(CDC_HEADER),       /* bDescriptorSubtype (Header
                                  Functional Descriptor.           */

    USB_DESC_BCD(0x0110), /* bcdCDC.                          */

    /* Call Management Functional Descriptor. */
    USB_DESC_BYTE(5),                      /* bFunctionLength.                 */
    USB_DESC_BYTE(CDC_CS_INTERFACE),       /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(CDC_CALL_MANAGEMENT),    /* bDescriptorSubtype (Call Management
                               Functional Descriptor).          */
    USB_DESC_BYTE(0x03),                   /* bmCapabilities (D0+D1).          */
    USB_DESC_BYTE(USB_CDC_DATA_INTERFACE), /* bDataInterface.                  */

    /* ACM Functional Descriptor.*/
    USB_DESC_BYTE(4),                               /* bFunctionLength.                 */
    USB_DESC_BYTE(CDC_CS_INTERFACE),                /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(CDC_ABSTRACT_CONTROL_MANAGEMENT), /* bDescriptorSubtype (Abstract
                            Control Management Descriptor).  */
    USB_DESC_BYTE(0x02),                            /* bmCapabilities.                  */

    /* Union Functional Descriptor.*/
    USB_DESC_BYTE(5),                      /* bFunctionLength.                 */
    USB_DESC_BYTE(CDC_CS_INTERFACE),       /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(CDC_UNION),              /* bDescriptorSubtype (Union
                                         Functional Descriptor).          */
    USB_DESC_BYTE(USB_CDC_INT_INTERFACE),  /* bMasterInterface (Communication
                             Class Interface).                */
    USB_DESC_BYTE(USB_CDC_DATA_INTERFACE), /* bSlaveInterface0 (Data Class
                            Interface).                      */

    /* Endpoint 2 Descriptor. in only.*/
    USB_DESC_ENDPOINT(USB_CDC_INT_REQUEST_EP | 0x80, /* bEndpointAddress */
                      USB_EP_MODE_TYPE_INTR,         /* bmAttributes (Interrupt). */
                      USB_CDC_INT_EPSIZE,            /* wMaxPacketSize.                  */
                      0xff),                         /* bInterval.                       */
    /* Interface Descriptor.*/
    USB_DESC_INTERFACE(USB_CDC_DATA_INTERFACE,   /* bInterfaceNumber.                */
                       0x00,                     /* bAlternateSetting.               */
                       0x02,                     /* bNumEndpoints.                   */
                       CDC_DATA_INTERFACE_CLASS, /* bInterfaceClass (Data Class
                                Interface, CDC section 4.5).     */
                       0x00,                     /* bInterfaceSubClass (CDC section
                                                    4.6).                            */
                       0x00,                     /* bInterfaceProtocol (CDC section
                                                    4.7).                            */
                       0x00),                    /* iInterface.                      */
    /* Endpoint 1 Descriptor.*/
    USB_DESC_ENDPOINT(USB_CDC_DATA_AVAILABLE_EP, /* bEndpointAddress.*/
                      USB_EP_MODE_TYPE_BULK,     /* bmAttributes (Bulk).             */
                      USB_CDC_DATA_EPSIZE,       /* wMaxPacketSize.                  */
                      0x00),                     /* bInterval.                       */
    /* Endpoint 1 Descriptor.*/
    USB_DESC_ENDPOINT(USB_CDC_DATA_REQUEST_EP | 0x80, /* bEndpointAddress.*/
                      USB_EP_MODE_TYPE_BULK,          /* bmAttributes (Bulk).             */
                      USB_CDC_DATA_EPSIZE,            /* wMaxPacketSize.                  */
                      0x00),                          /* bInterval.                       */
};

/* Configuration Descriptor wrapper */
static const USBDescriptor _usb_configuration_descriptor = {
    sizeof _usb_configuration_descriptor_data, _usb_configuration_descriptor_data};

/* wrappers */
static const USBDescriptor _usb_hid_kbd_descriptor = {
    USB_CONFIG_DESC_HID_SIZE, &_usb_configuration_descriptor_data[USB_HID_KBD_DESC_OFFSET]};
static const USBDescriptor _usb_hid_kbdext_descriptor = {
    USB_CONFIG_DESC_HID_SIZE, &_usb_configuration_descriptor_data[USB_HID_KBDEXT_DESC_OFFSET]};
static const USBDescriptor _usb_hid_raw_descriptor = {
    USB_CONFIG_DESC_HID_SIZE, &_usb_configuration_descriptor_data[USB_HID_RAW_DESC_OFFSET]};
static const USBDescriptor _usb_cdc_descriptor = {
    USB_CONFIG_DESC_CDC_SIZE, &_usb_configuration_descriptor_data[USB_CDC_DESC_OFFSET]};

/*
 * U.S. English language identifier.
 */
static const uint8_t _usb_desc_string0[] = {
    USB_DESC_BYTE(4),                     /* bLength.                         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
    USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t _usb_desc_string1[] = {
    USB_DESC_BYTE(38),                    /* bLength.                         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
    USB_DESC_CHAR_PADDED('T'),
    USB_DESC_CHAR_PADDED('h'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('A'),
    USB_DESC_CHAR_PADDED('n'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED('k'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('P'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('j'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('c'),
    USB_DESC_CHAR_PADDED('t'),
};

/*
 * Device Description string.
 */
static const uint8_t _usb_desc_string2[] = {
    USB_DESC_BYTE(72),                    /* bLength.                         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
    USB_DESC_CHAR_PADDED('C'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('n'),
    USB_DESC_CHAR_PADDED('f'),
    USB_DESC_CHAR_PADDED('i'),
    USB_DESC_CHAR_PADDED('g'),
    USB_DESC_CHAR_PADDED('u'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('a'),
    USB_DESC_CHAR_PADDED('b'),
    USB_DESC_CHAR_PADDED('l'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('k'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED('b'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('a'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('d'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('w'),
    USB_DESC_CHAR_PADDED('i'),
    USB_DESC_CHAR_PADDED('t'),
    USB_DESC_CHAR_PADDED('h'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('d'),
    USB_DESC_CHAR_PADDED('i'),
    USB_DESC_CHAR_PADDED('s'),
    USB_DESC_CHAR_PADDED('p'),
    USB_DESC_CHAR_PADDED('l'),
    USB_DESC_CHAR_PADDED('a'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED('s'),
};

/*
 * Serial Number string.
 */
static const uint8_t _usb_desc_string3[] = {
    USB_DESC_BYTE(8),                     /* bLength.                         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
    '0' + CH_KERNEL_MAJOR,
    0,
    '0' + CH_KERNEL_MINOR,
    0,
    '0' + CH_KERNEL_PATCH,
    0,
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor _usb_desc_strings[] = {
    {sizeof _usb_desc_string0, _usb_desc_string0},
    {sizeof _usb_desc_string1, _usb_desc_string1},
    {sizeof _usb_desc_string2, _usb_desc_string2},
    {sizeof _usb_desc_string3, _usb_desc_string3},
};

/*
 * EP states
 */
static USBInEndpointState _usb_ep_in_states[USB_NUM_IN_EPS];
static USBOutEndpointState _usb_ep_out_states[USB_NUM_OUT_EPS];

/*
 * EP configs
 */
// clang-format off
static const USBEndpointConfig _usb_ep_configs[] = {
    {USB_EP_MODE_TYPE_INTR, NULL, _usb_hid_kbd_in_cb, NULL, USB_HID_KBD_EPSIZE, 0x0000,
     &_usb_ep_in_states[USB_HID_KBD_EPS_IN], NULL, 2, NULL},
    {USB_EP_MODE_TYPE_INTR, NULL, _usb_hid_kbdext_in_cb, NULL, USB_HID_KBDEXT_EPSIZE, 0x0000,
     &_usb_ep_in_states[USB_HID_KBDEXT_EPS_IN], NULL, 2, NULL},
    {USB_EP_MODE_TYPE_INTR, NULL, _usb_hid_raw_in_cb, _usb_hid_raw_out_cb, USB_HID_RAW_EPSIZE, USB_HID_RAW_EPSIZE,
     &_usb_ep_in_states[USB_HID_RAW_EPS_IN], &_usb_ep_out_states[USB_HID_RAW_EPS_OUT], 2, NULL},
    {USB_EP_MODE_TYPE_INTR, NULL, sduInterruptTransmitted, NULL, 0x0010, 0x0000,
     &_usb_ep_in_states[USB_CDC_INT_REQUEST_EPS_IN], NULL, 2, NULL},
    {USB_EP_MODE_TYPE_BULK, NULL, sduDataTransmitted, sduDataReceived, 0x0040, 0x0040,
     &_usb_ep_in_states[USB_CDC_DATA_REQUEST_EPS_IN], &_usb_ep_out_states[USB_CDC_DATA_AVAILABLE_EPS_OUT], 2, NULL},
};
// clang-format on

/*
 * Global variables
 */
SerialUSBDriver USB_CDC_DRIVER_HANDLE;

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {_usb_event_cb, _usb_get_descriptor_cb, _usb_request_hook_cb, _usb_sof_cb};

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig serusbcfg = {&USB_DRIVER_HANDLE, USB_CDC_DATA_REQUEST_EP,
                                   USB_CDC_DATA_AVAILABLE_EP, USB_CDC_INT_REQUEST_EP};

/*
 * Tasks
 */

/*
 * Static helper functions
 */
static void _usb_init_hal(void)
{
  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&(USB_CDC_DRIVER_HANDLE));
  sduStart(&(USB_CDC_DRIVER_HANDLE), &serusbcfg);
}

static void _usb_init_module(void)
{
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
#if defined(USE_USB_DISC)
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1500);
#endif

  chVTObjectInit(&_usb_hid_kbd_idle_timer);
  chVTObjectInit(&_usb_hid_kbd_report_timer);
  memset(_usb_hid_kbd_report_pool, 0, sizeof(_usb_hid_kbd_report_pool));
  _usb_hid_kbd_report_prepare = &_usb_hid_kbd_report_pool[_usb_hid_report_idx];

  ibqObjectInit(&_usb_hid_raw_input_queue, true, _usb_hid_raw_input_buffer, USB_HID_RAW_EPSIZE,
                USB_HID_RAW_INPUT_BUFFER_ENTRIES, _usb_hid_raw_ibnotify_cb, NULL);
  obqObjectInit(&_usb_hid_raw_output_queue, true, _usb_hid_raw_output_buffer, USB_HID_RAW_EPSIZE,
                USB_HID_RAW_OUTPUT_BUFFER_ENTRIES, _usb_hid_raw_obnotify_cb, NULL);

  usbStart(&USB_DRIVER_HANDLE, &usbcfg);
  usbConnectBus(&USB_DRIVER_HANDLE);
}

static bool _usb_hid_request_hook(USBDriver *usbp)
{
  const USBDescriptor *dp;

  /* usbp->setup fields:
   *  0:   bmRequestType (bitmask)
   *  1:   bRequest
   *  2,3: (LSB,MSB) wValue
   *  4,5: (LSB,MSB) wIndex
   *  6,7: (LSB,MSB) wLength (number of bytes to transfer if there is a data phase) */

  /* Handle HID class specific requests */
  if (((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS) &&
      ((usbp->setup[0] & USB_RTYPE_RECIPIENT_MASK) == USB_RTYPE_RECIPIENT_INTERFACE))
  {
    switch (usbp->setup[0] & USB_RTYPE_DIR_MASK)
    {
      case USB_RTYPE_DIR_DEV2HOST:
        switch (usbp->setup[1])
        { /* bRequest */
          case USB_HID_GET_REPORT:
            switch (usbp->setup[4])
            { /* LSB(wIndex) (check MSB==0?) */
              case USB_HID_KBD_INTERFACE:
                usbSetupTransfer(usbp, (uint8_t *)_usb_hid_kbd_report_inflight,
                                 sizeof(usb_hid_kbd_report_t), NULL);
                return TRUE;
                break;

              case USB_HID_KBDEXT_INTERFACE:
                if (usbp->setup[3] == 1)
                { /* MSB(wValue) [Report Type] == 1 [Input Report] */
                  usb_hid_kbdext_report_t blank_report = {.key = 0};
                  switch (usbp->setup[2])
                  { /* LSB(wValue) [Report ID] */
                    case USB_HID_REPORT_ID_SYSTEM:
                      blank_report.report_id = USB_HID_REPORT_ID_SYSTEM;
                      usbSetupTransfer(usbp, (uint8_t *)&blank_report, sizeof(blank_report), NULL);
                      return TRUE;
                      break;
                    case USB_HID_REPORT_ID_CONSUMER:
                      blank_report.report_id = USB_HID_REPORT_ID_CONSUMER;
                      usbSetupTransfer(usbp, (uint8_t *)&blank_report, sizeof(blank_report), NULL);
                      return TRUE;
                      break;
                    default:
                      return FALSE;
                  }
                }
                else
                {
                  return FALSE;
                }
                break;
              case USB_HID_RAW_INTERFACE:
              {
                uint8_t buf[USB_HID_RAW_EPSIZE] = {0};
                uint8_t i = 0;
                for (i = 0; i < USB_HID_RAW_EPSIZE; i++) buf[i] = i;
                usbSetupTransfer(usbp, buf, sizeof(uint8_t) * USB_HID_RAW_EPSIZE, NULL);
                return TRUE;
                break;
              }
              default:
                usbSetupTransfer(usbp, NULL, 0, NULL);
                return TRUE;
                break;
            }
            break;

          case USB_HID_GET_PROTOCOL:
            if ((usbp->setup[4] == USB_HID_KBD_INTERFACE) && (usbp->setup[5] == 0))
            { /* wIndex */
              usbSetupTransfer(usbp, &_usb_hid_kbd_protocol, 1, NULL);
              return TRUE;
            }
            break;

          case USB_HID_GET_IDLE:
            usbSetupTransfer(usbp, &_usb_hid_kbd_idle, 1, NULL);
            return TRUE;
            break;
        }
        break;

      case USB_RTYPE_DIR_HOST2DEV:
        switch (usbp->setup[1])
        { /* bRequest */
          case USB_HID_SET_PROTOCOL:
            if ((usbp->setup[4] == USB_HID_KBD_INTERFACE) && (usbp->setup[5] == 0))
            {                                                     /* wIndex */
              _usb_hid_kbd_protocol = ((usbp->setup[2]) != 0x00); /* LSB(wValue) */
              if (_usb_hid_kbd_idle)
              {
                /* arm the idle timer if boot protocol & idle */
                chSysLockFromISR();
                chVTSetI(&_usb_hid_kbd_idle_timer, 4 * TIME_MS2I(_usb_hid_kbd_idle),
                         _usb_hid_kbd_idle_timer_cb, (void *)usbp);
                chSysUnlockFromISR();
              }
            }
            usbSetupTransfer(usbp, NULL, 0, NULL);
            return TRUE;
            break;

          case USB_HID_SET_IDLE:
            _usb_hid_kbd_idle = usbp->setup[3]; /* MSB(wValue) */
                                                /* arm the timer */
            if (_usb_hid_kbd_idle)
            {
              chSysLockFromISR();
              chVTSetI(&_usb_hid_kbd_idle_timer, 4 * TIME_MS2I(_usb_hid_kbd_idle),
                       _usb_hid_kbd_idle_timer_cb, (void *)usbp);
              chSysUnlockFromISR();
            }
            usbSetupTransfer(usbp, NULL, 0, NULL);
            return TRUE;
            break;
        }
        break;
    }
  }

  /* Handle the Get_Descriptor Request for HID class (not handled by the default hook) */
  if ((usbp->setup[0] == 0x81) && (usbp->setup[1] == USB_REQ_GET_DESCRIPTOR))
  {
    dp = usbp->config->get_descriptor_cb(usbp, usbp->setup[3], usbp->setup[2],
                                         ((usbp->setup[5] << 8) | usbp->setup[4]));
    if (dp == NULL) return FALSE;
    usbSetupTransfer(usbp, (uint8_t *)dp->ud_string, dp->ud_size, NULL);
    return TRUE;
  }

  return FALSE;
}

static void _usb_hid_kbd_report_pool_next(void)
{
  chSysLock();
  _usb_hid_kbd_report_inflight = _usb_hid_kbd_report_prepare;
  _usb_hid_report_idx = (_usb_hid_report_idx + 1) % USB_HID_KBD_REPORT_POOLSIZE;
  _usb_hid_kbd_report_prepare = &_usb_hid_kbd_report_pool[_usb_hid_report_idx];
  memset(_usb_hid_kbd_report_prepare, 0, sizeof(usb_hid_kbd_report_t));
  _usb_hid_report_payload_idx = 0;
  chSysUnlock();
}

static bool _usb_hid_kbd_send_report(bool is_in_cb)
{
  chSysLockFromISR();
  if (usbGetDriverStateI(&USB_DRIVER_HANDLE) != USB_ACTIVE)
  {
    chSysUnlockFromISR();
    return FALSE;
  }
  chSysUnlockFromISR();

  /* need to wait until the previous packet has made it through */
  /* busy wait, should be short and not very common */
  chSysLockFromISR();
  if (usbGetTransmitStatusI(&USB_DRIVER_HANDLE, USB_HID_KBD_EP))
  {
    /* Need to either suspend, or loop and call unlock/lock during
     * every iteration - otherwise the system will remain locked,
     * no interrupts served, so USB not going through as well.
     * Note: for suspend, need USB_USE_WAIT == TRUE in halconf.h */
    if (is_in_cb == FALSE)
    {
      chThdSuspendS(&(&USB_DRIVER_HANDLE)->epc[USB_HID_KBD_EP]->in_state->thread);
    }
    else
    {
      chSysUnlockFromISR();
      return FALSE;
    }
  }
  usbStartTransmitI(&USB_DRIVER_HANDLE, USB_HID_KBD_EP, (uint8_t *)_usb_hid_kbd_report_inflight,
                    USB_HID_KBD_EPSIZE);
  chSysUnlockFromISR();
  return TRUE;
}

static bool _usb_hid_raw_start_receive(void)
{
  uint8_t *buf;

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(&USB_DRIVER_HANDLE) != USB_ACTIVE))
  {
    return true;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (usbGetReceiveStatusI(&USB_DRIVER_HANDLE, USB_HID_RAW_EP))
  {
    return true;
  }

  /* Checking if there is a buffer ready for incoming data.*/
  buf = ibqGetEmptyBufferI(&_usb_hid_raw_input_queue);
  if (buf == NULL)
  {
    return true;
  }

  /* Buffer found, starting a new transaction.*/
  usbStartReceiveI(&USB_DRIVER_HANDLE, USB_HID_RAW_EP, buf, USB_HID_RAW_EPSIZE);

  return false;
}

static void _usb_hid_raw_sof_hook(void)
{
  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(&USB_DRIVER_HANDLE) != USB_ACTIVE))
  {
    return;
  }

  /* If there is already a transaction ongoing then another one cannot be
     started.*/
  if (usbGetTransmitStatusI(&USB_DRIVER_HANDLE, USB_HID_RAW_EP))
  {
    return;
  }

  //  /* Checking if there only a buffer partially filled, if so then it is
  //     enforced in the queue and transmitted.*/
  //  if (obqTryFlushI(&_usb_hid_raw_output_queue))
  //  {
  //    size_t n;
  //    uint8_t *buf = obqGetFullBufferI(&_usb_hid_raw_output_queue, &n);
  //
  //    osalDbgAssert(buf != NULL, "queue is empty");
  //
  //    usbStartTransmitI(&USB_DRIVER_HANDLE, USB_HID_RAW_EP, buf, n);
  //  }
}

static void _usb_hid_raw_configured_hook(void)
{
  ibqResetI(&_usb_hid_raw_input_queue);
  bqResumeX(&_usb_hid_raw_input_queue);
  obqResetI(&_usb_hid_raw_output_queue);
  bqResumeX(&_usb_hid_raw_output_queue);

  _usb_hid_raw_start_receive();
}

/*
 * Callback functions
 */
/*
 * Handles the USB driver global events.
 */
static void _usb_event_cb(USBDriver *usbp, usbevent_t event)
{
  switch (event)
  {
    case USB_EVENT_ADDRESS:
      return;
    case USB_EVENT_CONFIGURED:
      chSysLockFromISR();

      /* Enables the endpoints specified into the configuration.
         Note, this callback is invoked from an ISR so I-Class functions
         must be used.*/
      usbInitEndpointI(usbp, USB_HID_KBD_EP, &_usb_ep_configs[USB_HID_KBD_INTERFACE]);
      usbInitEndpointI(usbp, USB_HID_KBDEXT_EP, &_usb_ep_configs[USB_HID_KBDEXT_INTERFACE]);
      usbInitEndpointI(usbp, USB_HID_RAW_EP, &_usb_ep_configs[USB_HID_RAW_INTERFACE]);

      usbInitEndpointI(usbp, USB_CDC_INT_REQUEST_EP, &_usb_ep_configs[USB_CDC_INT_INTERFACE]);
      usbInitEndpointI(usbp, USB_CDC_DATA_REQUEST_EP, &_usb_ep_configs[USB_CDC_DATA_INTERFACE]);

      /* Resetting the state of the CDC subsystem.*/
      sduConfigureHookI(&USB_CDC_DRIVER_HANDLE);

      _usb_hid_raw_configured_hook();

      chSysUnlockFromISR();
      return;
    case USB_EVENT_RESET:
      /* Falls into.*/
    case USB_EVENT_UNCONFIGURED:
      /* Falls into.*/
    case USB_EVENT_SUSPEND:
      chSysLockFromISR();

      /* Disconnection event on suspend.*/
      sduSuspendHookI(&USB_CDC_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_WAKEUP:
      chSysLockFromISR();

      /* Connection event on wakeup.*/
      sduWakeupHookI(&USB_CDC_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_STALLED:
      return;
  }
  return;
}

static const USBDescriptor *_usb_get_descriptor_cb(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                                   uint16_t lang)
{
  (void)usbp;
  (void)lang;
  switch (dtype)
  {
    case USB_DESCRIPTOR_DEVICE:
      return &_usb_device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
      return &_usb_configuration_descriptor;
    case USB_DESCRIPTOR_STRING:
      if (dindex < 4) return &_usb_desc_strings[dindex];
      break;
    case USB_DESCRIPTOR_CDC:
      return &_usb_cdc_descriptor;
    case USB_DESCRIPTOR_HID: /* HID Descriptors */
      switch (lang)
      {
        case USB_HID_KBD_INTERFACE:
          return &_usb_hid_kbd_descriptor;
        case USB_HID_KBDEXT_INTERFACE:
          return &_usb_hid_kbdext_descriptor;
        case USB_HID_RAW_INTERFACE:
          return &_usb_hid_raw_descriptor;
      }
      break;
    case USB_DESCRIPTOR_HID_REPORT: /* HID Descriptors */
      switch (lang)
      {
        case USB_HID_KBD_INTERFACE:
          return &_usb_hid_kbd_report_descriptor;
        case USB_HID_KBDEXT_INTERFACE:
          return &_usb_hid_kbdext_report_descriptor;
        case USB_HID_RAW_INTERFACE:
          return &_usb_hid_raw_report_descriptor;
      }
      break;
    default:
      break;
  }
  return NULL;
}

static bool _usb_request_hook_cb(USBDriver *usbp)
{
  if (((usbp->setup[0] & USB_RTYPE_RECIPIENT_MASK) == USB_RTYPE_RECIPIENT_INTERFACE) &&
      (usbp->setup[1] == USB_REQ_SET_INTERFACE))
  {
    usbSetupTransfer(usbp, NULL, 0, NULL);
    return true;
  }
  if (_usb_hid_request_hook(usbp))
  {
    return TRUE;
  }
  return sduRequestsHook(usbp);
}

static void _usb_sof_cb(USBDriver *usbp)
{
  (void)usbp;

  chSysLockFromISR();
  sduSOFHookI(&USB_CDC_DRIVER_HANDLE);
  _usb_hid_raw_sof_hook();
  chSysUnlockFromISR();
}

static void _usb_hid_kbd_idle_timer_cb(void *arg)
{
  USBDriver *usbp = (USBDriver *)arg;

  chSysLockFromISR();

  /* check that the states of things are as they're supposed to */
  if (usbGetDriverStateI(usbp) != USB_ACTIVE)
  {
    /* do not rearm the timer, should be enabled on IDLE request */
    chSysUnlockFromISR();
    return;
  }

  if (_usb_hid_kbd_idle)
  {
    /* TODO: are we sure we want the KBD_ENDPOINT? */
    if (!usbGetTransmitStatusI(usbp, USB_HID_KBD_EP))
    {
      usbStartTransmitI(usbp, USB_HID_KBD_EP, (uint8_t *)_usb_hid_kbd_report_inflight,
                        sizeof(usb_hid_kbd_report_t));
    }
    /* rearm the timer */
    chVTSetI(&_usb_hid_kbd_idle_timer, 4 * TIME_MS2I(_usb_hid_kbd_idle), _usb_hid_kbd_idle_timer_cb,
             (void *)usbp);
  }

  /* do not rearm the timer if the condition above fails
   * it should be enabled again on either IDLE or SET_PROTOCOL requests */
  chSysUnlockFromISR();
}

static void _usb_hid_kbd_report_timer_cb(void *arg)
{
  bool retry = (bool)arg;
  if (retry == FALSE)
  {
    _usb_hid_kbd_report_pool_next();
  }
  if (_usb_hid_kbd_send_report(TRUE) == FALSE)
  {
    if (_usb_hid_report_retry_counter < USB_HID_KBD_REPORT_RETRY_MAX)
    {
      _usb_hid_report_retry_counter++;
      chVTSetI(&_usb_hid_kbd_report_timer, TIME_MS2I(USB_HID_KBD_REPORT_RETRY_MS),
               _usb_hid_kbd_report_timer_cb, (void *)TRUE);
      return;
    }
  }
  _usb_hid_report_retry_counter = 0;
}

static void _usb_hid_raw_in_cb(USBDriver *usbp, usbep_t ep)
{
  uint8_t *buf;
  size_t n;

  chSysLockFromISR();

  /* Freeing the buffer just transmitted, if it was not a zero size packet.*/
  if (usbp->epc[ep]->in_state->txsize > 0U)
  {
    obqReleaseEmptyBufferI(&_usb_hid_raw_output_queue);
  }

  /* Checking if there is a buffer ready for transmission.*/
  buf = obqGetFullBufferI(&_usb_hid_raw_output_queue, &n);

  if (buf != NULL)
  {
    /* The endpoint cannot be busy, we are in the context of the callback,
       so it is safe to transmit without a check.*/
    usbStartTransmitI(usbp, ep, buf, n);
  }
  else
  {
    /* Nothing to transmit.*/
  }

  chSysUnlockFromISR();
}

static void _usb_hid_raw_out_cb(USBDriver *usbp, usbep_t ep)
{
  size_t size;

  chSysLockFromISR();

  /* Checking for zero-size transactions.*/
  size = usbGetReceiveTransactionSizeX(usbp, ep);
  if (size > (size_t)0)
  {
    /* Posting the filled buffer in the queue.*/
    ibqPostFullBufferI(&_usb_hid_raw_input_queue, size);
  }

  /* The endpoint cannot be busy, we are in the context of the callback,
     so a packet is in the buffer for sure. Trying to get a free buffer
     for the next transaction.*/
  (void)_usb_hid_raw_start_receive();

  chSysUnlockFromISR();
}

static void _usb_hid_raw_ibnotify_cb(io_buffers_queue_t *bqp)
{
  (void)bqp;
  _usb_hid_raw_start_receive();
}

static void _usb_hid_raw_obnotify_cb(io_buffers_queue_t *bqp)
{
  (void)bqp;
  size_t n;

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if (usbGetDriverStateI(&USB_DRIVER_HANDLE) != USB_ACTIVE)
  {
    return;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (!usbGetTransmitStatusI(&USB_DRIVER_HANDLE, USB_HID_RAW_EP))
  {
    /* Getting a full buffer, a buffer is available for sure because this
       callback is invoked when one has been inserted.*/
    uint8_t *buf = obqGetFullBufferI(&_usb_hid_raw_output_queue, &n);
    osalDbgAssert(buf != NULL, "buffer not found");
    usbStartTransmitI(&USB_DRIVER_HANDLE, USB_HID_RAW_EP, buf, n);
  }
}

/*
 * Shell functions
 */
void usb_loop_hid_raw_input(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "usb-loop-hid-raw\r\n");
    return;
  }
  chprintf(chp, "Dumping hid raw ouptut to console:\r\n");
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
    uint8_t line_break = 0;
    if (ibqGetFullBufferTimeoutS(&_usb_hid_raw_input_queue, TIME_MS2I(50)) == MSG_OK)
    {
      uint8_t *msg = _usb_hid_raw_input_queue.ptr;
      size_t len = (size_t)_usb_hid_raw_input_queue.top - (size_t)_usb_hid_raw_input_queue.ptr;
      uint8_t i = 0;
      for (i = 0; i < len; i++)
      {
        line_break++;
        uint8_t rn[3] = {'\r', '\n', 0};
        rn[0] = ((line_break % 64) == 0) ? '\r' : 0;
        chprintf(chp, "%02X %s", msg[i], rn);
      }
    }
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

/*
 * API functions
 */
void usb_init(void)
{
  _usb_init_hal();
  _usb_init_module();
}

void usb_hid_kbd_flush(void)
{
  _usb_hid_kbd_report_pool_next();
  _usb_hid_kbd_send_report(FALSE);
}

void usb_hid_kbd_send_key(uint8_t mods, uint8_t key)
{
  chVTReset(&_usb_hid_kbd_report_timer);
  if (_usb_hid_report_payload_idx && mods != _usb_hid_kbd_report_prepare->mods)
  {
    usb_hid_kbd_flush();
  }
  _usb_hid_kbd_report_prepare->mods = mods;
  _usb_hid_kbd_report_prepare->keys[_usb_hid_report_payload_idx] = key;
  _usb_hid_report_payload_idx++;
  if (_usb_hid_report_payload_idx == USB_HID_KBD_REPORT_KEYS)
  {
    usb_hid_kbd_flush();
  }
  else
  {
    chVTSet(&_usb_hid_kbd_report_timer, TIME_MS2I(USB_HID_KBD_REPORT_TIMEOUT_MS),
            _usb_hid_kbd_report_timer_cb, (void *)FALSE);
  }
}

void usb_hid_kbdext_send_key(usb_hid_report_id_t report_id, uint16_t keyext)
{
  usb_hid_kbdext_report_t report;
  report.report_id = report_id;
  report.key = keyext;
  chSysLock();
  if (usbGetDriverStateI(&USB_DRIVER_HANDLE) != USB_ACTIVE)
  {
    chSysUnlock();
    return;
  }

  if (usbGetTransmitStatusI(&USB_DRIVER_HANDLE, USB_HID_KBD_EP))
  {
    chSysUnlock();
    chThdSuspendS(&(&USB_DRIVER_HANDLE)->epc[USB_HID_KBDEXT_EP]->in_state->thread);
    chSysLock();
  }

  usbStartTransmitI(&USB_DRIVER_HANDLE, USB_HID_KBDEXT_EP, (uint8_t *)&report,
                    sizeof(usb_hid_kbdext_report_t));

  chSysUnlock();
}

size_t usb_hid_raw_send(uint8_t *msg, uint8_t size)
{
  return obqWriteTimeout(&_usb_hid_raw_output_queue, msg, size, TIME_INFINITE);
}

size_t usb_hid_raw_receive(uint8_t *msg, uint8_t size)
{
  return ibqReadTimeout(&_usb_hid_raw_input_queue, msg, size, TIME_INFINITE);
}
