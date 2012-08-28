/*
 * Copyright 2011 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * Based on the original ben-wpan code written by
 *   Werner Almesberger, Copyright 2008-2011
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdint.h>

#include "usb.h"

#include "board.h"
#include "sernum.h"


static const uint8_t string_descriptor_0[] = {
	4,				/* blength */
	USB_DT_STRING,			/* bDescriptorType */
	LE(USB_LANGID_ENGLISH_US)	/* wLANGID[0]  */
};

static const uint8_t string_descriptor_2[] = {
	16,				/* blength */
	USB_DT_STRING,			/* bDescriptorType */
	'A', '\0',
	'V', '\0',
	'R', '\0',
	' ', '\0',
	'D', '\0',
	'F', '\0',
	'U', '\0',
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

static const uint8_t string_descriptor_4[] = {
	12,				/* blength */
	USB_DT_STRING,			/* bDescriptorType */
	'f', '\0',
	'l', '\0',
	'a', '\0',
	's', '\0',
	'h', '\0',
};

int sernum_get_descr(uint8_t type, uint8_t index, const uint8_t **reply,
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
	case 4:
		*reply = string_descriptor_4;
		*size = sizeof(string_descriptor_4);
		return 1;
	default:
		return 0;
	}
}
