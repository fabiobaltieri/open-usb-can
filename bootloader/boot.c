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

#include "defines.h"

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "usb.h"
#include "dfu.h"
#include "spi.h"

#include "board.h"

static void (*run_payload)(void) = 0;

int main(void)
{
	/*
	 * pgm_read_byte gets cached and there doesn't seem to be any other
	 * way to dissuade gcc from doing this.
	 */
	volatile int zero = 0;
	uint32_t loop = 0;

	board_init();
	spi_init();

	_delay_ms(10);

	/* set MCP2515 clkdiv to /2  for 8MHz */
	can_cs_l();
	spi_io(INSTRUCTION_WRITE);
	spi_io(CANCTRL);
	spi_io(0x85);
	can_cs_h();

	led_init();

	usb_init();
	dfu_init();

	/* move interrupt vectors to the boot loader */
	MCUCR = 1 << IVCE;
	MCUCR = 1 << IVSEL;

	sei();

	while (loop < 5) {
		led_a_on();
		led_b_on();
		_delay_ms(50);
		led_a_off();
		led_b_off();
		_delay_ms(400);

		if (dfu.state == dfuIDLE && pgm_read_byte(zero) != 0xff)
			loop++;
		else
			loop = 0;
	}

	cli();

	usb_reset();
	run_payload();

	while (1);
}
