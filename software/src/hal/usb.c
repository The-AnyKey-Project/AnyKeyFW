/*
 * usb.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
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

/*
 * Forward declarations of static functions
 */
static void _usb_init_hal(void);
static void _usb_init_module(void);
static const USBDescriptor *get_descriptor(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                           uint16_t lang);
static void usb_event(USBDriver *usbp, usbevent_t event);
static void sof_handler(USBDriver *usbp);

/*
 * Static variables
 */
/*
 * USB Device Descriptor.
 */
static const uint8_t _usb_device_descriptor_data[18] = {
    USB_DESC_DEVICE(0x0110, /* bcdUSB (1.1).                    */
                    0x00,   /* bDeviceClass (CDC).              */
                    0x00,   /* bDeviceSubClass.                 */
                    0x00,   /* bDeviceProtocol.                 */
                    0x40,   /* bMaxPacketSize.                  */
                    0x0483, /* idVendor (ST).                   */
                    0x5740, /* idProduct.                       */
                    0x0200, /* bcdDevice.                       */
                    1,      /* iManufacturer.                   */
                    2,      /* iProduct.                        */
                    3,      /* iSerialNumber.                   */
                    1)      /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor _usb_device_descriptor = {sizeof _usb_device_descriptor_data,
                                                     _usb_device_descriptor_data};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t _usb_configuration_descriptor_data[92] = {
    /* Configuration Descriptor.*/
    USB_DESC_CONFIGURATION(67,   /* wTotalLength.                    */
                           0x02, /* bNumInterfaces.                  */
                           0x01, /* bConfigurationValue.             */
                           0,    /* iConfiguration.                  */
                           0xC0, /* bmAttributes (self powered).     */
                           250), /* bMaxPower (100mA).               */
    /* Interface Descriptor.*/
    USB_DESC_INTERFACE(USB_VCOM_INTERFACE, /* bInterfaceNumber.                */
                       0x00,               /* bAlternateSetting.               */
                       0x01,               /* bNumEndpoints.                   */
                       0x02,               /* bInterfaceClass (Communications
                                              Interface Class, CDC section
                                              4.2).                            */
                       0x02,               /* bInterfaceSubClass (Abstract
                                            Control Model, CDC section 4.3).   */
                       0x01,               /* bInterfaceProtocol (AT commands,
                                              CDC section 4.4).                */
                       0),                 /* iInterface.                      */
    /* Header Functional Descriptor (CDC section 5.2.3).*/
    USB_DESC_BYTE(5),     /* bLength.                         */
    USB_DESC_BYTE(0x24),  /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(0x00),  /* bDescriptorSubtype (Header
                             Functional Descriptor.           */
    USB_DESC_BCD(0x0110), /* bcdCDC.                          */
    /* Call Management Functional Descriptor. */
    USB_DESC_BYTE(5),    /* bFunctionLength.                 */
    USB_DESC_BYTE(0x24), /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(0x01), /* bDescriptorSubtype (Call Management
                            Functional Descriptor).          */
    USB_DESC_BYTE(0x00), /* bmCapabilities (D0+D1).          */
    USB_DESC_BYTE(0x01), /* bDataInterface.                  */
    /* ACM Functional Descriptor.*/
    USB_DESC_BYTE(4),    /* bFunctionLength.                 */
    USB_DESC_BYTE(0x24), /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(0x02), /* bDescriptorSubtype (Abstract
                            Control Management Descriptor).  */
    USB_DESC_BYTE(0x02), /* bmCapabilities.                  */
    /* Union Functional Descriptor.*/
    USB_DESC_BYTE(5),    /* bFunctionLength.                 */
    USB_DESC_BYTE(0x24), /* bDescriptorType (CS_INTERFACE).  */
    USB_DESC_BYTE(0x06), /* bDescriptorSubtype (Union
                            Functional Descriptor).          */
    USB_DESC_BYTE(0x00), /* bMasterInterface (Communication
                            Class Interface).                */
    USB_DESC_BYTE(0x01), /* bSlaveInterface0 (Data Class
                            Interface).                      */
    /* Endpoint 2 Descriptor. in only.*/
    USB_DESC_ENDPOINT(USB_ENDPOINT_IN(USB_VCOM_INTERRUPT_REQUEST_EP),
                      0x03,   /* bmAttributes (Interrupt). */
                      0x0008, /* wMaxPacketSize.                  */
                      0xFF),  /* bInterval.                       */
    /* Interface Descriptor.*/
    USB_DESC_INTERFACE(0x01,  /* bInterfaceNumber.                */
                       0x00,  /* bAlternateSetting.               */
                       0x02,  /* bNumEndpoints.                   */
                       0x0A,  /* bInterfaceClass (Data Class
                                 Interface, CDC section 4.5).     */
                       0x00,  /* bInterfaceSubClass (CDC section
                                 4.6).                            */
                       0x00,  /* bInterfaceProtocol (CDC section
                                 4.7).                            */
                       0x00), /* iInterface.                      */
    /* Endpoint 1 Descriptor.*/
    USB_DESC_ENDPOINT(USB_ENDPOINT_OUT(USB_VCOM_DATA_AVAILABLE_EP), /* bEndpointAddress.*/
                      0x02,   /* bmAttributes (Bulk).             */
                      0x0040, /* wMaxPacketSize.                  */
                      0x00),  /* bInterval.                       */
    /* Endpoint 1 Descriptor.*/
    USB_DESC_ENDPOINT(USB_ENDPOINT_IN(USB_VCOM_DATA_REQUEST_EP), /* bEndpointAddress.*/
                      0x02,   /* bmAttributes (Bulk).             */
                      0x0040, /* wMaxPacketSize.                  */
                      0x00),  /* bInterval.                       */
    /* Interface Descriptor (9 bytes) USB spec 9.6.5, page 267-269, Table 9-12 */
    USB_DESC_INTERFACE(USB_HID_KBD_INTERFACE, /* bInterfaceNumber */
                       0,                     /* bAlternateSetting */
                       1,                     /* bNumEndpoints */
                       0x03,                  /* bInterfaceClass: HID */
                       0x01,                  /* bInterfaceSubClass: Boot */
                       0x01,                  /* bInterfaceProtocol: Keyboard */
                       0),                    /* iInterface */

    /* HID descriptor (9 bytes) HID 1.11 spec, section 6.2.1 */
    USB_DESC_BYTE(9),     /* bLength */
    USB_DESC_BYTE(0x21),  /* bDescriptorType (HID class) */
    USB_DESC_BCD(0x0111), /* bcdHID: HID version 1.11 */
    USB_DESC_BYTE(0),     /* bCountryCode */
    USB_DESC_BYTE(1),     /* bNumDescriptors */
    USB_DESC_BYTE(0x22),  /* bDescriptorType (report desc) */
    USB_DESC_WORD(0xff),  // sizeof(keyboard_hid_report_desc_data)), /* wDescriptorLength */

    /* Endpoint Descriptor (7 bytes) USB spec 9.6.6, page 269-271, Table 9-13 */
    USB_DESC_ENDPOINT(USB_HID_KBD_EP | 0x80, /* bEndpointAddress */
                      0x03,                  /* bmAttributes (Interrupt) */
                      USB_HID_KBD_EP_SIZE,   /* wMaxPacketSize */
                      10),                   /* bInterval */

};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor vcom_configuration_descriptor = {
    sizeof _usb_configuration_descriptor_data, _usb_configuration_descriptor_data};

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
    USB_DESC_BYTE(44),                    /* bLength.                         */
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
    USB_DESC_CHAR_PADDED('A'),
    USB_DESC_CHAR_PADDED('n'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED('k'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('M'),
    USB_DESC_CHAR_PADDED('a'),
    USB_DESC_CHAR_PADDED('c'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED(' '),
    USB_DESC_CHAR_PADDED('K'),
    USB_DESC_CHAR_PADDED('e'),
    USB_DESC_CHAR_PADDED('y'),
    USB_DESC_CHAR_PADDED('b'),
    USB_DESC_CHAR_PADDED('o'),
    USB_DESC_CHAR_PADDED('a'),
    USB_DESC_CHAR_PADDED('r'),
    USB_DESC_CHAR_PADDED('d'),
};

/*
 * Device Description string.
 */
static const uint8_t _usb_desc_string2[] = {
    USB_DESC_BYTE(74),                    /* bLength.                         */
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
    0};

/*
 * Strings wrappers array.
 */
static const USBDescriptor vcom_strings[] = {{sizeof _usb_desc_string0, _usb_desc_string0},
                                             {sizeof _usb_desc_string1, _usb_desc_string1},
                                             {sizeof _usb_desc_string2, _usb_desc_string2},
                                             {sizeof _usb_desc_string3, _usb_desc_string3}};

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config = {USB_EP_MODE_TYPE_BULK,
                                            NULL,
                                            sduDataTransmitted,
                                            sduDataReceived,
                                            0x0040,
                                            0x0040,
                                            &ep1instate,
                                            &ep1outstate,
                                            2,
                                            NULL};

/**
 * @brief   IN EP2 state.
 */
static USBInEndpointState ep2instate;

/**
 * @brief   EP2 initialization structure (IN only).
 */
static const USBEndpointConfig ep2config = {USB_EP_MODE_TYPE_INTR,
                                            NULL,
                                            sduInterruptTransmitted,
                                            NULL,
                                            0x0010,
                                            0x0000,
                                            &ep2instate,
                                            NULL,
                                            1,
                                            NULL};

/*
 * Global variables
 */
SerialUSBDriver USB_VCOM_DRIVER_HANDLE;

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {usb_event, get_descriptor, sduRequestsHook, sof_handler};

/*
 * Serial over USB driver configuration.
 */
const SerialUSBConfig serusbcfg = {&USB_DRIVER_HANDLE, USB_VCOM_DATA_REQUEST_EP,
                                   USB_VCOM_DATA_AVAILABLE_EP, USB_VCOM_INTERRUPT_REQUEST_EP};

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
  sduObjectInit(&(USB_VCOM_DRIVER_HANDLE));
  sduStart(&(USB_VCOM_DRIVER_HANDLE), &serusbcfg);
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
  usbStart(&USB_DRIVER_HANDLE, &usbcfg);
  usbConnectBus(&USB_DRIVER_HANDLE);
}

static const USBDescriptor *get_descriptor(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                           uint16_t lang)
{
  (void)usbp;
  (void)lang;
  switch (dtype)
  {
    case USB_DESCRIPTOR_DEVICE:
      return &_usb_device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
      return &vcom_configuration_descriptor;
    case USB_DESCRIPTOR_STRING:
      if (dindex < 4) return &vcom_strings[dindex];
  }
  return NULL;
}

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event)
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
      usbInitEndpointI(usbp, USB_VCOM_DATA_REQUEST_EP, &ep1config);
      usbInitEndpointI(usbp, USB_VCOM_INTERRUPT_REQUEST_EP, &ep2config);

      /* Resetting the state of the CDC subsystem.*/
      sduConfigureHookI(&USB_VCOM_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_RESET:
      /* Falls into.*/
    case USB_EVENT_UNCONFIGURED:
      /* Falls into.*/
    case USB_EVENT_SUSPEND:
      chSysLockFromISR();

      /* Disconnection event on suspend.*/
      sduSuspendHookI(&USB_VCOM_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_WAKEUP:
      chSysLockFromISR();

      /* Connection event on wakeup.*/
      sduWakeupHookI(&USB_VCOM_DRIVER_HANDLE);

      chSysUnlockFromISR();
      return;
    case USB_EVENT_STALLED:
      return;
  }
  return;
}

/*
 * Handles the USB driver global events.
 */
static void sof_handler(USBDriver *usbp)
{
  (void)usbp;

  osalSysLockFromISR();
  sduSOFHookI(&USB_VCOM_DRIVER_HANDLE);
  osalSysUnlockFromISR();
}

/*
 * Callback functions
 */

/*
 * Shell functions
 */

/*
 * API functions
 */
void usb_init(void)
{
  _usb_init_hal();
  _usb_init_module();
}
