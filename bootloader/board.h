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

#ifndef BOARD_H
#define	BOARD_H

#include <stdint.h>

/* from Openmoko open registry for community:
 * http://wiki.openmoko.org/wiki/USB_Product_IDs */
#define	USB_VENDOR	0x1d50	/* Openmoko, Inc */
#define	USB_PRODUCT	0x6044	/* open-usb-can (DFU) */

#define	DFU_USB_VENDOR	USB_VENDOR
#define	DFU_USB_PRODUCT	USB_PRODUCT

#define	BOARD_MAX_mA	40

#define	NUM_EPS	1

extern uint8_t board_sernum[42];
extern uint8_t irq_serial;

void reset_cpu(void);

void panic(void);

void board_init(void);
void board_app_init(void);

#endif /* !BOARD_H */
