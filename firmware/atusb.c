/*
 * fw/atusb.c - ATUSB initialization and main loop
 *
 * Written 2008-2011 by Werner Almesberger
 * Copyright 2008-2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usb.h"

#include "board.h"
#include "sernum.h"
#include "ep0.h"

#include "defines.h"

char x[64];

void send_again (void *user);
void send_again (void *user)
{
	uint8_t i;
	LED_B_DDR |= _BV(LED_B);
	x[0]++;
	i = 2;
	/* i = (x[0] & 0x3) + 1; */
	/* x[1] = i; */
	usb_send(&eps[1], x, 16 * i, send_again, NULL);

}

int main(void)
{
	LED_G_DDR |= _BV(LED_G);

	board_init();
	board_app_init();

	user_get_descriptor = sernum_get_descr;

	/* now we should be at 8 MHz */

	usb_init();
	ep0_init();

	/* move interrupt vectors to 0 */
	MCUCR = 1 << IVCE;
	MCUCR = 0;

	sei();

	memset(x, 0, 64);

	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);

	while (eps[1].state);

	send_again(x);

	while (1) {
		sleep_mode();
	}
}
