/*
 * Copyright 2011 Fabio Baltieri <fabio.baltieri@gmail.com>
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

#include <stdint.h>
#include <string.h>
#include <avr/io.h>

#include "usb.h"
#include "dfu.h"
#include "ep0.h"
#include "version.h"
#include "board.h"
#include "descr.h"
#include "spi.h"
#include "can.h"
#include "mcp2515.h"
#include "buffer.h"

#include "defines.h"

#define debug(...)
#define error(...)

static const uint8_t id[] = { EP0ATUSB_MAJOR, EP0ATUSB_MINOR };
static uint8_t buf[EP0_SIZE];
static uint8_t size;

static void do_buf_write(void *user)
{
	uint8_t i;

	can_cs_l();
	for (i = 0; i != size; i++)
		spi_io(buf[i]);
	can_cs_h();
}

#define	BUILD_OFFSET	7	/* '#' plus "65535" plus ' ' */

static int my_setup(const struct setup_request *setup)
{
	uint16_t req = setup->bmRequestType | setup->bRequest << 8;
	unsigned tmp;
	uint8_t i;

	switch (req) {

	case ATUSB_FROM_DEV(ATUSB_ID):
		debug("ATUSB_ID\n");
		if (setup->wLength > 2)
			return 0;
		usb_send(&eps[0], id, setup->wLength, NULL, NULL);
		return 1;

	case ATUSB_FROM_DEV(ATUSB_BUILD):
		debug("ATUSB_BUILD\n");
		tmp = build_number;
		for (i = BUILD_OFFSET-2; tmp; i--) {
			buf[i] = (tmp % 10)+'0';
			tmp /= 10;
		}
		buf[i] = '#';
		buf[BUILD_OFFSET-1] = ' ';
		for (size = 0; build_date[size]; size++)
			buf[BUILD_OFFSET+size] = build_date[size];
		size += BUILD_OFFSET-i;
		if (size > setup->wLength)
			return 0;
		usb_send(&eps[0], buf+i, size, NULL, NULL);
		return 1;

	case ATUSB_TO_DEV(ATUSB_RESET):
		debug("ATUSB_RESET\n");
		reset_cpu();
		while (1);

	case ATUSB_FROM_DEV(ATUSB_GPIO):
		debug("ATUSB_GPIO\n");
		if (setup->wLength < 3)
			return 0;
		if (!gpio(setup->wIndex, setup->wValue, setup->wValue >> 8,
		    setup->wIndex >> 8, buf))
			return 0;
		usb_send(&eps[0], buf, 3, NULL, NULL);
		return 1;

	case ATUSB_TO_DEV(ATUSB_REG_WRITE):
		debug("ATUSB_REG_WRITE\n");
		mcp2515_write_reg(setup->wIndex, setup->wValue);
		return 1;

	case ATUSB_FROM_DEV(ATUSB_REG_READ):
		debug("ATUSB_REG_READ\n");
		buf[0] = mcp2515_read_reg(setup->wIndex);
		usb_send(&eps[0], buf, 1, NULL, NULL);
		return 1;

	case ATUSB_TO_DEV(ATUSB_SPI_WRITE1):
		size = setup->wLength+1;
		if (size > sizeof(buf))
			return 0;
		buf[0] = setup->wValue;
		if (setup->wLength)
			usb_recv(&eps[0], buf+1, setup->wLength,
			    do_buf_write, NULL);
		else
			do_buf_write(NULL);
		return 1;

	case ATUSB_TO_DEV(ATUSB_SPI_WRITE2):
		size = setup->wLength+2;
		if (size > sizeof(buf))
			return 0;
		buf[0] = setup->wValue;
		buf[1] = setup->wIndex;
		if (setup->wLength)
			usb_recv(&eps[0], buf+2, setup->wLength,
			    do_buf_write, NULL);
		else
			do_buf_write(NULL);
		return 1;

	case ATUSB_FROM_DEV(ATUSB_SPI_READ1):
	case ATUSB_FROM_DEV(ATUSB_SPI_READ2):
		can_cs_l();
		spi_io(setup->wValue);
		if (req == ATUSB_FROM_DEV(ATUSB_SPI_READ2))
			spi_io(setup->wIndex);
		for (i = 0; i != setup->wLength; i++)
			buf[i] = spi_io(0xff);
		can_cs_h();
		usb_send(&eps[0], buf, setup->wLength, NULL, NULL);
		return 1;

	case ATUSB_TO_DEV(ATUSB_CAN_PUT_CONFIG):
		if (setup->wLength != sizeof(can_cfg))
			return 0;
		usb_recv(&eps[0], (uint8_t *)&can_cfg,
			 setup->wLength, NULL, NULL);
		return 1;

	case ATUSB_FROM_DEV(ATUSB_CAN_GET_CONFIG):
		if (setup->wLength != sizeof(can_cfg))
			return 0;
		usb_send(&eps[0], (uint8_t *)&can_cfg,
			 setup->wLength, NULL, NULL);
		return 1;

	case ATUSB_TO_DEV(ATUSB_CAN_START):
		buffer_reset();
		mcp2515_start();
		return 1;

	case ATUSB_TO_DEV(ATUSB_CAN_STOP):
		mcp2515_stop();
		buffer_reset();
		return 1;

	default:
		error("Unrecognized SETUP: 0x%02x 0x%02x ...\n",
		    setup->bmRequestType, setup->bRequest);
		return 0;
	}
}

static int my_dfu_setup(const struct setup_request *setup)
{
	switch (setup->bmRequestType | setup->bRequest << 8) {
	case DFU_TO_DEV(DFU_DETACH):
		/* @@@ should use wTimeout */
		dfu.state = appDETACH;
		return 1;
	default:
		return dfu_setup_common(setup);
	}
}

static void my_set_interface(int nth)
{
	if (nth) {
		user_setup = my_dfu_setup;
		user_get_descriptor = dfu_my_descr;
		dfu.state = appIDLE;
	} else {
		user_setup = my_setup;
		user_get_descriptor = strings_get_descr;
	}
}

static void my_reset(void)
{
	if (dfu.state == appDETACH) {
		reset_cpu();
		while (1);
	}
}

void ep0_init(void)
{
	user_setup = my_setup;
	user_set_interface = my_set_interface;
	my_set_interface(0);
	user_reset = my_reset;
}
