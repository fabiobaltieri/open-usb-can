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
#include "descr.h"
#include "ep0.h"

#include "defines.h"


static uint8_t x[64];
static uint8_t lock;

void send_again (void *user)
{
	/* uint8_t i; */

	led_b_toggle();

	x[0]++;
	/* i = 2; */
	/* i = (x[0] & 0x3) + 1; */
	/* x[1] = i; */
	/* usb_send(&eps[2], x, 16 * i, send_again, NULL); */
}

void recv_done (void *user)
{
	led_a_toggle();
	usb_recv(&eps[1], x+1, 16, recv_done, NULL);

	lock++;
}

int main(void)
{
	board_init();

	LED_A_DDR |= _BV(LED_A);
	LED_B_DDR |= _BV(LED_B);
	led_a_off();
	led_b_off();

	board_app_init();

	user_get_descriptor = strings_get_descr;

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

	led_a_on();
	led_b_on();

	lock = 0;

	usb_recv(&eps[1], x, 16, recv_done, NULL);

	/* send_again(x); */

	while (1) {
		cli();
		if (lock && eps[2].state == EP_IDLE) {
			usb_send(&eps[2], x, 32, send_again, NULL);
			lock--;
		}
		sei();

		sleep_mode();
	}
}
