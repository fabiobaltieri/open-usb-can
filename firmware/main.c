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
#include "spi.h"
#include "can.h"
#include "buffer.h"
#include "mcp2515.h"
#include "blink.h"

void hello (void)
{
	uint8_t i;

	_delay_ms(100);

	for (i = 0; i < 5; i++) {
		led_a_on();
		led_b_on();

		_delay_ms(50);
		
		led_a_off();
		led_b_off();
		
		_delay_ms(50);
	}
}

int main(void)
{
	board_init();
	led_init();
	spi_init();
	blink_init();

	_delay_ms(10);

	mcp2515_init(1);

	usb_init();
	ep0_init();

	user_get_descriptor = strings_get_descr;

	/* move interrupt vectors to 0 */
	MCUCR = 1 << IVCE;
	MCUCR = 0;

	sei();

	hello();

	while (1) {
		buffer_tx_process();
		buffer_rx_process();

		//sleep_mode();
	}
}
