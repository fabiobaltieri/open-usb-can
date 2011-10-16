/*
 * boot/dfu.h - DFU protocol constants and data structures
 *
 * Written 2008, 2011 by Werner Almesberger
 * Copyright 2008, 2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef DFU_H
#define DFU_H

#include <stdint.h>

#include "usb.h"


enum dfu_request {
	DFU_DETACH,
	DFU_DNLOAD,
	DFU_UPLOAD,
	DFU_GETSTATUS,
	DFU_CLRSTATUS,
	DFU_GETSTATE,
	DFU_ABORT,
};


enum dfu_status {
	OK,
	errTARGET,
	errFILE,
	errWRITE,
	errERASE,
	errCHECK_ERASED,
	errPROG,
	errVERIFY,
	errADDRESS,
	errNOTDONE,
	errFIRMWARE,
	errVENDOR,
	errUSBR,
	errPOR,
	errUNKNOWN,
	errSTALLEDPKT,
};


enum dfu_state {
	appIDLE,
	appDETACH,
	dfuIDLE,
	dfuDNLOAD_SYNC,
	dfuDNBUSY,
	dfuDNLOAD_IDLE,
	dfuMANIFEST_SYNC,
	dfuMANIFEST,
	dfuMANIFEST_WAIT_RESET,
	dfuUPLOAD_IDLE,
	dfuERROR
};

enum dfu_itf_proto {
	dfu_proto_runtime	= 1,	/* Runtime protocol */
	dfu_proto_dfu		= 2,	/* DFU mode protocol */
};


#define DFU_DT_FUNCTIONAL	0x21	/* DFU FUNCTIONAL descriptor type */


#define	DFU_TO_DEV(req)		(0x21 | (req) << 8)
#define	DFU_FROM_DEV(req)	(0xa1 | (req) << 8)


struct dfu {
	uint8_t status;		/* bStatus */
	uint8_t toL, toM, toH;	/* bwPollTimeout */
	uint8_t state;		/* bState */
	uint8_t iString;
};


#define	DFU_ITF_DESCR(itf, proto)					     \
	9,			/* bLength */				     \
	USB_DT_INTERFACE,	/* bDescriptorType */			     \
	(itf),			/* bInterfaceNumber */			     \
	0,			/* bAlternateSetting */			     \
	0,			/* bNumEndpoints */			     \
	0xfe,			/* bInterfaceClass (application specific) */ \
	0x01,			/* bInterfaceSubClass (device fw upgrade) */ \
	(proto),		/* bInterfaceProtocol (dfu_proto_*) */	     \
	0,			/* iInterface */


extern struct dfu dfu;


void flash_start(void);
int flash_can_write(uint16_t size);
void flash_write(const uint8_t *buf, uint16_t size);
void flash_end_write(void);
uint16_t flash_read(uint8_t *buf, uint16_t size);

int dfu_setup_common(const struct setup_request *setup);
int dfu_my_descr(uint8_t type, uint8_t index, const uint8_t **reply,
    uint8_t *size);

void dfu_init(void);

#endif /* !DFU_H */
