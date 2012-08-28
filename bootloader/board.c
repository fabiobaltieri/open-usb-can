/*
 * Copyright 2011 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * Based on the original ben-wpan code written by
 *   Werner Almesberger, Copyright 2011
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include <util/delay.h>

#include "usb.h"
#include "board.h"

#include "defines.h"

uint8_t board_sernum[42] = { 42, USB_DT_STRING };

void panic(void)
{
	cli();
	while (1) {
		led_off();
		_delay_ms(100);
		led_on();
		_delay_ms(100);
	}
}


static char hex(uint8_t nibble)
{
	return nibble < 10 ? '0'+nibble : 'a'+nibble-10;
}

static void get_sernum(void)
{
	uint8_t sig;
	int i;

	for (i = 0; i != 10; i++) {
		sig = boot_signature_byte_get(i+0xe);
		board_sernum[(i << 2)+2] = hex(sig >> 4);
		board_sernum[(i << 2)+4] = hex(sig & 0xf);
	}
}

void board_init(void)
{
	/* Disable the watchdog timer */

	MCUSR = 0;		/* Remove override */
	WDTCSR |= 1 << WDCE;	/* Enable change */
	WDTCSR = 1 << WDCE;	/* Disable watchdog while still enabling
				   change */

	/* We start with a 1 MHz/8 clock. Disable the prescaler. */

	CLKPR = 1 << CLKPCE;
	CLKPR = 0;

	get_sernum();
}
