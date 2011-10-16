/*
 * fw/usb/usb.c - USB hardware setup and standard device requests
 *
 * Written 2008-2011 by Werner Almesberger
 * Copyright 2008-2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Known issues:
 * - no suspend/resume
 * - should support EP clearing and stalling
 */

#include <stdint.h>

#include "usb.h"
#include "../board.h"


#ifndef NULL
#define NULL 0
#endif

#if 1
extern void panic(void);
#define BUG_ON(cond)	do { if (cond) panic(); } while (0)
#else
#define BUG_ON(cond)
#endif

int (*user_setup)(const struct setup_request *setup);
void (*user_set_interface)(int nth);
int (*user_get_descriptor)(uint8_t type, uint8_t index,
    const uint8_t **reply, uint8_t *size);
void (*user_reset)(void);


void usb_io(struct ep_descr *ep, enum ep_state state, uint8_t *buf,
    uint8_t size, void (*callback)(void *user), void *user)
{
	BUG_ON(ep->state);
	ep->state = state;
	ep->buf = buf;
	ep->end = buf+size;
	ep->callback = callback;
	ep->user = user;
	usb_ep_change(ep);
}


static int get_descriptor(uint8_t type, uint8_t index, uint16_t length)
{
	const uint8_t *reply;
	uint8_t size;

	switch (type) {
	case USB_DT_DEVICE:
		reply = device_descriptor;
		size = reply[0];
		break;
	case USB_DT_CONFIG:
		if (index)
			return 0;
		reply = config_descriptor;
		size = reply[2];
		break;
	default:
		if (!user_get_descriptor)
			return 0;
		if (!user_get_descriptor(type, index, &reply, &size))
			return 0;
	}
	if (length < size)
		size = length;
	usb_send(&eps[0], reply, size, NULL, NULL);
	return 1;
}


int handle_setup(const struct setup_request *setup)
{
	switch (setup->bmRequestType | setup->bRequest << 8) {

	/*
	 * Device request
	 *
	 * See http://www.beyondlogic.org/usbnutshell/usb6.htm
	 */

	case FROM_DEVICE(GET_STATUS):
		if (setup->wLength != 2)
			return 0;
		usb_send(&eps[0], "\000", 2, NULL, NULL);
		break;
	case TO_DEVICE(CLEAR_FEATURE):
		break;
	case TO_DEVICE(SET_FEATURE):
		return 0;
	case TO_DEVICE(SET_ADDRESS):
		set_addr(setup->wValue);
		break;
	case FROM_DEVICE(GET_DESCRIPTOR):
		if (!get_descriptor(setup->wValue >> 8, setup->wValue,
		    setup->wLength))
			return 0;
		break;
	case TO_DEVICE(SET_DESCRIPTOR):
		return 0;
	case FROM_DEVICE(GET_CONFIGURATION):
		usb_send(&eps[0], "", 1, NULL, NULL);
		break;
	case TO_DEVICE(SET_CONFIGURATION):
		if (setup->wValue != config_descriptor[5])
			return 0;
		break;

	/*
	 * Interface request
	 */

	case FROM_INTERFACE(GET_STATUS):
		return 0;
	case TO_INTERFACE(CLEAR_FEATURE):
		return 0;
	case TO_INTERFACE(SET_FEATURE):
		return 0;
	case FROM_INTERFACE(GET_INTERFACE):
		return 0;
	case TO_INTERFACE(SET_INTERFACE):
		{
			const uint8_t *interface_descriptor =
			    config_descriptor+9;
			const uint8_t *p;
			int i;

			i = 0;
			for (p = interface_descriptor;
			    p != config_descriptor+config_descriptor[2];
			    p += p[0]) {
				if (p[2] == setup->wIndex &&
				    p[3] == setup->wValue) {
					if (user_set_interface)
						user_set_interface(i);
					return 1;
				}
				i++;
			}
			return 0;
		}
		break;

	/*
	 * Endpoint request
	 */

	case FROM_ENDPOINT(GET_STATUS):
		return 0;
	case TO_ENDPOINT(CLEAR_FEATURE):
		return 0;
	case TO_ENDPOINT(SET_FEATURE):
		return 0;
	case FROM_ENDPOINT(SYNCH_FRAME):
		return 0;

	default:
		if (user_setup)
			return user_setup(setup);
		return 0;
	}

	return 1;
}
