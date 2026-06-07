// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// USB descriptors for the bare-metal Teensy 4.0 CDC device. Reproduces the
// current firmware's USB identity byte-for-byte so existing host drivers bind
// unchanged and the `serialnumber` command returns the same value
// (see docs/migration/usb-identity.md). Written against the TinyUSB descriptor
// API; values are ours.

#include "tusb.h"
#include "MIMXRT1062.h"

#define USB_VID 0x16C0
#define USB_PID 0x0483
#define USB_BCD 0x0200

// --- Device descriptor -----------------------------------------------------
static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    // Use IAD so the CDC comm + data interfaces are grouped.
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

const uint8_t *tud_descriptor_device_cb(void)
{
    return (const uint8_t *)&desc_device;
}

// --- Configuration descriptor ----------------------------------------------
enum { ITF_NUM_CDC = 0, ITF_NUM_CDC_DATA, ITF_NUM_TOTAL };

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 0, EPNUM_CDC_NOTIF, 8,
                       EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_configuration;
}

// --- String descriptors ----------------------------------------------------
// Serial number: decimal of (HW_OCOTP_MAC0 & 0xFFFFFF), x10 if < 1e7
// (OS-X CDC-ACM workaround), base-10 ASCII — identical to the PJRC formula.
static char serial_str[11];

static void build_serial(void)
{
    uint32_t num = OCOTP->MAC0 & 0xFFFFFFu;
    if (num < 10000000u) num *= 10u;
    // base-10 ASCII, no leading zeros
    char tmp[11];
    int i = 0;
    if (num == 0) tmp[i++] = '0';
    while (num) { tmp[i++] = (char)('0' + (num % 10u)); num /= 10u; }
    int j = 0;
    while (i > 0) serial_str[j++] = tmp[--i];
    serial_str[j] = '\0';
}

static const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: language = English (0x0409)
    "TeensyToAny",               // 1: manufacturer (overrides Teensyduino)
    "USB Serial",                // 2: product
    serial_str,                  // 3: serial (filled at runtime)
};

static uint16_t desc_string[32];

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    uint8_t chr_count;

    if (index == 0) {
        desc_string[1] = 0x0409;
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
            return NULL;
        }
        if (index == 3) build_serial();
        const char *str = string_desc_arr[index];
        chr_count = 0;
        while (str[chr_count] && chr_count < 31) {
            desc_string[1 + chr_count] = (uint16_t)str[chr_count];
            chr_count++;
        }
    }

    desc_string[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return desc_string;
}
