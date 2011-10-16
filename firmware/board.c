/*
 * fw/board.c - Board-specific functions (for boot loader and application)
 *
 * Written 2011 by Werner Almesberger
 * Copyright 2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include <util/delay.h>

#include "usb.h"
#include "board.h"

#include "defines.h"

void panic(void)
{
	cli();
	while (1) {
		led_a_on();
		led_b_on();
		_delay_ms(100);
		led_a_off();
		led_b_off();
		_delay_ms(100);
	}
}

void board_init(void)
{
	/* Disable the watchdog timer */

	MCUSR = 0;		/* Remove override */
	WDTCSR |= 1 << WDCE;	/* Enable change */
	WDTCSR = 1 << WDCE;	/* Disable watchdog while still enabling change */

	/* We start with a 1 MHz/8 clock. Disable the prescaler. */

	CLKPR = 1 << CLKPCE;
	CLKPR = 0;
}
