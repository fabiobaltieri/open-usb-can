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
	0,			/* iManufacturer */
	0,			/* iProduct */
#ifdef HAS_BOARD_SERNUM
	1,			/* iSerialNumber */
#else
	0,			/* iSerialNumber */
#endif
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
#if 0
	LE(9+9+7+7),		/* wTotalLength */
#else
	LE(9+9+7+9),		/* wTotalLength */
#endif
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
	1,			/* bNumEndpoints */
	USB_CLASS_VENDOR_SPEC,	/* bInterfaceClass */
	0,			/* bInterfaceSubClass */
	0,			/* bInterfaceProtocol */
	0,			/* iInterface */

#if 0
	/* EP OUT */

	7,			/* bLength */
	USB_DT_ENDPOINT,	/* bDescriptorType */
	0x01,			/* bEndPointAddress */
	0x02,			/* bmAttributes (bulk) */
	LE(EP1_SIZE),		/* wMaxPacketSize */
	0,			/* bInterval */
#endif

#if 1
	/* EP IN */

	7,			/* bLength */
	USB_DT_ENDPOINT,	/* bDescriptorType */
	0x81,			/* bEndPointAddress */
	0x02,			/* bmAttributes (bulk) */
	LE(EP1_SIZE),		/* wMaxPacketSize */
	0,			/* bInterval */
#endif

	/* Interface #1 */

	DFU_ITF_DESCR(1, dfu_proto_runtime)
};
