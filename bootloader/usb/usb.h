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

#ifndef USB_H
#define USB_H


#include <stdint.h>


/*
 * Descriptor types
 *
 * Reuse libusb naming scheme (/usr/include/usb.h)
 */

#define	USB_DT_DEVICE		1
#define	USB_DT_CONFIG		2
#define	USB_DT_STRING		3
#define	USB_DT_INTERFACE	4
#define	USB_DT_ENDPOINT		5

/*
 * Device classes
 *
 * Reuse libusb naming scheme (/usr/include/usb.h)
 */

#define USB_CLASS_PER_INTERFACE	0xfe
#define USB_CLASS_VENDOR_SPEC	0xff

/*
 * Configuration attributes
 */

#define	USB_ATTR_BUS_POWERED	0x80
#define	USB_ATTR_SELF_POWERED	0x40
#define	USB_ATTR_REMOTE_WAKEUP	0x20

/*
 * Setup request types
 */

#define	TO_DEVICE(req)		(0x00 | (req) << 8)
#define	FROM_DEVICE(req)	(0x80 | (req) << 8)
#define	TO_INTERFACE(req)	(0x01 | (req) << 8)
#define	FROM_INTERFACE(req)	(0x81 | (req) << 8)
#define	TO_ENDPOINT(req)	(0x02 | (req) << 8)
#define	FROM_ENDPOINT(req)	(0x82 | (req) << 8)

/*
 * Setup requests
 */

#define	GET_STATUS		0x00
#define	CLEAR_FEATURE		0x01
#define	SET_FEATURE		0x03
#define	SET_ADDRESS		0x05
#define	GET_DESCRIPTOR		0x06
#define	SET_DESCRIPTOR		0x07
#define	GET_CONFIGURATION	0x08
#define	SET_CONFIGURATION	0x09
#define	GET_INTERFACE		0x0a
#define	SET_INTERFACE		0x0b
#define	SYNCH_FRAME		0x0c

/*
 * USB Language ID codes
 *
 * http://www.usb.org/developers/docs/USB_LANGIDs.pdf
 */

#define	USB_LANGID_ENGLISH_US	0x409


/*
 * Odd. sdcc seems to think "x" assumes the size of the destination, i.e.,
 * uint8_t. Hence the cast.
 */

#define LE(x) ((uint16_t) (x) & 0xff), ((uint16_t) (x) >> 8)

#define LO(x) (((uint8_t *) &(x))[0])
#define HI(x) (((uint8_t *) &(x))[1])


#ifdef LOW_SPEED
#define	EP0_SIZE	8
#else
#define	EP0_SIZE	64
#endif

#define	EP1_SIZE	64	/* simplify */


enum ep_state {
	EP_IDLE,
	EP_RX,
	EP_TX,
	EP_STALL,
};

struct ep_descr {
	enum ep_state state;
	uint8_t *buf;
	uint8_t *end;
	uint8_t size;
	void (*callback)(void *user);
	void *user;
};

struct setup_request {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};


extern const uint8_t device_descriptor[];
extern const uint8_t config_descriptor[];
extern struct ep_descr eps[];

extern int (*user_setup)(const struct setup_request *setup);
extern void (*user_set_interface)(int nth);
extern int (*user_get_descriptor)(uint8_t type, uint8_t index,
    const uint8_t **reply, uint8_t *size);
extern void (*user_reset)(void);


#define	usb_left(ep) ((ep)->end-(ep)->buf)
#define usb_send(ep, buf, size, callback, user) \
	usb_io(ep, EP_TX, (void *) buf, size, callback, user)
#define usb_recv(ep, buf, size, callback, user) \
	usb_io(ep, EP_RX, buf, size, callback, user)

void usb_io(struct ep_descr *ep, enum ep_state state, uint8_t *buf,
    uint8_t size, void (*callback)(void *user), void *user);

int handle_setup(const struct setup_request *setup);
int set_addr(uint8_t addr);
void usb_ep_change(struct ep_descr *ep);
void usb_reset(void);
void usb_init(void);

#endif /* !USB_H */
