/*
 * descr.h - ATUSB serial number
 *
 * Written 2011 by Werner Almesberger
 * Copyright 2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define	USB_VENDOR	0x03eb
#define	USB_PRODUCT	0xcab5

#define	BOARD_MAX_mA	40

enum {
	STRING_UNDEF = 0,
	STRING_VENDOR,
	STRING_PRODUCT,
};

int strings_get_descr(uint8_t type, uint8_t index, const uint8_t **reply, uint8_t *size);
