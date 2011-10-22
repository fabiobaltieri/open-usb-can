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

#include "defines.h"

#include "usb.h"
#include "board.h"
#include "descr.h"
#include "ep0.h"
#include "can.h"

static uint8_t x[8];
static uint16_t lock;

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

	//lock++;
}

struct can_frame cf[4];

int main(void)
{

	board_init();
	led_init();
	usb_init();
	ep0_init();

	user_get_descriptor = strings_get_descr;

	/* move interrupt vectors to 0 */
	MCUCR = 1 << IVCE;
	MCUCR = 0;

	sei();

	memset(x, 0, 1);

	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);

	led_a_on();
	led_b_on();

	lock = 0;

	usb_recv(&eps[1], x, 16, recv_done, NULL);

	/* send_again(x); */
	memset(&cf, 0, sizeof(cf));
	cf[0].can_id = 0xcf0 | CAN_EFF_FLAG;
	cf[0].can_dlc = 8;
	cf[0].data[0] = 0;
	cf[1].can_id = 0xcf1 | CAN_EFF_FLAG;
	cf[1].can_dlc = 1;
	cf[1].data[0] = 0;
	cf[2].can_id = 0xcf2 | CAN_EFF_FLAG;
	cf[2].can_dlc = 1;
	cf[2].data[0] = 0;
	cf[3].can_id = 0xcf3 | CAN_EFF_FLAG;
	cf[3].can_dlc = 1;
	cf[3].data[0] = 0;
	while (1) {
		cli();
		if (lock == 0 && eps[2].state == EP_IDLE) {
			usb_send(&eps[2], cf, sizeof(struct can_frame),  send_again, NULL);
			cf[0].data[0]++;
		}
		lock++;
		sei();

		//sleep_mode();
	}
}
