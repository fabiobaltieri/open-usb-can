/*
 * fw/board_app.c - Board-specific functions (for the application)
 *
 * Written 2011 by Werner Almesberger
 * Copyright 2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "defines.h"

#include "usb.h"
#include "board.h"

void reset_cpu(void)
{
	WDTCSR = 1 << WDE;
}

int gpio(uint8_t port, uint8_t data, uint8_t dir, uint8_t mask, uint8_t *res)
{
	EIMSK = 0; /* recover INT_RF to ATUSB_GPIO_CLEANUP or an MCU reset */

	switch (port) {
	case 1:
		DDRB = (DDRB & ~mask) | dir;
		PORTB = (PORTB & ~mask) | data;
		break;
	case 2:
		DDRC = (DDRC & ~mask) | dir;
		PORTC = (PORTC & ~mask) | data;
		break;
	case 3:
		DDRD = (DDRD & ~mask) | dir;
		PORTD = (PORTD & ~mask) | data;
		break;
	default:
		return 0;
	}

	switch (port) {
	case 1:
		res[0] = PINB;
		res[1] = PORTB;
		res[2] = DDRB;
		break;
	case 2:
		res[0] = PINC;
		res[1] = PORTC;
		res[2] = DDRC;
		break;
	case 3:
		res[0] = PIND;
		res[1] = PORTD;
		res[2] = DDRD;
		break;
	}

	return 1;
}

void board_app_init(void)
{
}
