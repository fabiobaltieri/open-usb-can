/*
 * boot/dfu_common.c - DFU protocol engine parts common to App/DFU
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
 * http://www.usb.org/developers/devclass_docs/DFU_1.1.pdf
 */

/*
 * A few, erm, shortcuts:
 *
 * - we don't bother with the app* states since DFU is all this firmware does
 * - after DFU_DNLOAD, we just block until things are written, so we never
 *   enter dfuDNLOAD_SYNC or dfuDNBUSY
 * - no dfuMANIFEST_SYNC, dfuMANIFEST, or dfuMANIFEST_WAIT_RESET
 * - to keep our buffers small, we only accept EP0-sized blocks
 */


#include <stdint.h>

#include "usb.h"
#include "dfu.h"

#include "../board.h"
#include "../sernum.h"


#ifndef NULL
#define NULL 0
#endif

#define debug(...)
#define error(...)


static const uint8_t functional_descriptor[] = {
	9,			/* bLength */
	DFU_DT_FUNCTIONAL,	/* bDescriptorType */
	0xf,			/* bmAttributes (claim omnipotence :-) */
	LE(0xffff),		/* wDetachTimeOut (we're very patient) */
	LE(EP0_SIZE),		/* wTransferSize */
	LE(0x101),		/* bcdDFUVersion */
};


/*
 * The worst-case activity would be flashing a one page and erasing another
 * one, would should take less than 10 ms. A 100 ms timeout ought to be plenty.
 */

struct dfu dfu = {
	OK,			/* bStatus */
	LE(100), 0,		/* bwPollTimeout, 100 ms */
	dfuIDLE,		/* bState */
	0,			/* iString */
};


int dfu_setup_common(const struct setup_request *setup)
{
	switch (setup->bmRequestType | setup->bRequest << 8) {
	case DFU_FROM_DEV(DFU_GETSTATUS):
		debug("DFU_GETSTATUS\n");
		usb_send(&eps[0], (uint8_t *) &dfu, sizeof(dfu), NULL, NULL);
		return 1;
	case DFU_TO_DEV(DFU_CLRSTATUS):
		debug("DFU_CLRSTATUS\n");
		dfu.state = dfuIDLE;
		dfu.status = OK;
		return 1;
	case DFU_FROM_DEV(DFU_GETSTATE):
		debug("DFU_GETSTATE\n");
		usb_send(&eps[0], &dfu.state, 1, NULL, NULL);
		return 1;
	default:
		error("DFU rt %x, rq%x ?\n",
		    setup->bmRequestType, setup->bRequest);
		return 0;
	}
}


int dfu_my_descr(uint8_t type, uint8_t index, const uint8_t **reply,
    uint8_t *size)
{
	if (type != DFU_DT_FUNCTIONAL)
		return sernum_get_descr(type, index, reply, size);
	*reply = functional_descriptor;
	*size = sizeof(functional_descriptor);
	return 1;
}
