/*
 * fw/descr.c - USB descriptors
 *
 * Written 2008-2011 by Werner Almesberger
 * Copyright 2008-2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include "usb.h"
#include "dfu.h"
#include "board.h"


#define LE(x) ((uint16_t) (x) & 0xff), ((uint16_t) (x) >> 8)

/*
 * Device descriptor
 */

const uint8_t device_descriptor[18] = {
	18,			/* bLength */
	USB_DT_DEVICE,		/* bDescriptorType */
	LE(0x200),		/* bcdUSB */
	USB_CLASS_VENDOR_SPEC,	/* bDeviceClass */
	0x00,			/* bDeviceSubClass */
	0x00,			/* bDeviceProtocol */
	EP0_SIZE,		/* bMaxPacketSize */
	LE(USB_VENDOR),		/* idVendor */
	LE(USB_PRODUCT),	/* idProduct */
	LE(0x0001),		/* bcdDevice */
	3,			/* iManufacturer */
	2,			/* iProduct */
	1,			/* iSerialNumber */
	1			/* bNumConfigurations */
};


/*
 * Our configuration
 *
 * We're always bus-powered.
 */

const uint8_t config_descriptor[] = {
	9,			/* bLength */
	USB_DT_CONFIG,		/* bDescriptorType */
	LE(9+9+7+7+9),		/* wTotalLength */
	2,			/* bNumInterfaces */
	1,			/* bConfigurationValue (> 0 !) */
	0,			/* iConfiguration */
	USB_ATTR_BUS_POWERED,	/* bmAttributes */
	((BOARD_MAX_mA)+1)/2,	/* bMaxPower */

	/* Interface #0 */

	9,			/* bLength */
	USB_DT_INTERFACE,	/* bDescriptorType */
	0,			/* bInterfaceNumber */
	0,			/* bAlternateSetting */
	2,			/* bNumEndpoints */
	USB_CLASS_VENDOR_SPEC,	/* bInterfaceClass */
	0,			/* bInterfaceSubClass */
	0,			/* bInterfaceProtocol */
	0,			/* iInterface */

	/* EP OUT */

	7,			/* bLength */
	USB_DT_ENDPOINT,	/* bDescriptorType */
	0x01,			/* bEndPointAddress */
	0x02,			/* bmAttributes (bulk) */
	LE(EP1_SIZE),		/* wMaxPacketSize */
	0,			/* bInterval */

	/* EP IN */

	7,			/* bLength */
	USB_DT_ENDPOINT,	/* bDescriptorType */
	0x82,			/* bEndPointAddress */
	0x02,			/* bmAttributes (bulk) */
	LE(EP2_SIZE),		/* wMaxPacketSize */
	0,			/* bInterval */

	/* Interface #1 */

	DFU_ITF_DESCR(1, dfu_proto_runtime)
};

static const uint8_t string_descriptor_0[] = {
	4,				/* blength */
	USB_DT_STRING,			/* bDescriptorType */
	LE(USB_LANGID_ENGLISH_US)	/* wLANGID[0]  */
};

static const uint8_t string_descriptor_2[] = {
	26,				/* blength */
	USB_DT_STRING,			/* bDescriptorType */
	'O', '\0',
	'p', '\0',
	'e', '\0',
	'n', '\0',
	' ', '\0',
	'U', '\0',
	'S', '\0',
	'B', '\0',
	'-', '\0',
	'C', '\0',
	'A', '\0',
	'N', '\0',
};

static const uint8_t string_descriptor_3[] = {
	12,				/* blength */
	USB_DT_STRING,			/* bDescriptorType */
	'B', '\0',
	'a', '\0',
	'l', '\0',
	't', '\0',
	'o', '\0',
};

int strings_get_descr(uint8_t type, uint8_t index, const uint8_t **reply,
    uint8_t *size)
{
	if (type != USB_DT_STRING)
		return 0;
	switch (index) {
	case 0:
		*reply = string_descriptor_0;
		*size = sizeof(string_descriptor_0);
		return 1;
	case 1:
		*reply = board_sernum;
		*size = sizeof(board_sernum);
		return 1;
	case 2:
		*reply = string_descriptor_2;
		*size = sizeof(string_descriptor_2);
		return 1;
	case 3:
		*reply = string_descriptor_3;
		*size = sizeof(string_descriptor_3);
		return 1;
	default:
		return 0;
	}
}
