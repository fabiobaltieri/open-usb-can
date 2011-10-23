/*
 * include/atusb/ep0.h - EP0 extension protocol
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
 * EP0 protocol:
 *
 * 0.0	initial release
 * 0.1  addition of ATUSB_TEST
 */

#define EP0ATUSB_MAJOR	0	/* EP0 protocol, major revision */
#define EP0ATUSB_MINOR	1	/* EP0 protocol, minor revision */

/*
 * bmRequestType:
 *
 * D7 D6..5 D4...0
 * |  |     |
 * direction (0 = host->dev)
 *    type (2 = vendor)
 *          recipient (0 = device)
 */

#define	ATUSB_TO_DEV(req)	(0x40 | (req) << 8)
#define	ATUSB_FROM_DEV(req)	(0xc0 | (req) << 8)

/*
 * Direction	bRequest		wValue		wIndex	wLength
 */

enum atusb_requests {
/* system status/control grp
 *
 * ->host	ATUSB_ID		-		-	2
 * ->host	ATUSB_BUILD		-		-	#bytes
 * host->	ATUSB_RESET		-		-	0
 */
	ATUSB_ID			= 0x00,
	ATUSB_BUILD,
	ATUSB_RESET,

/* debug/test group
 *
 * ->host	ATUSB_GPIO		dir+data	mask+p#	3
 */
	ATUSB_GPIO			= 0x10,

/* transceiver group
 *
 * host->	ATUSB_REG_WRITE		value		addr	0
 * ->host	ATUSB_REG_READ		-		addr	1
 */
	ATUSB_REG_WRITE			= 0x20,
	ATUSB_REG_READ,

/* SPI group
 *
 * host->	ATUSB_SPI_WRITE1	byte0		-	#bytes
 * host->	ATUSB_SPI_WRITE2	byte0		byte1	#bytes
 * ->host	ATUSB_SPI_READ1		byte0		-	#bytes
 * ->host	ATUSB_SPI_READ2		byte0		byte1	#bytes
 */
	ATUSB_SPI_WRITE1		= 0x30,
	ATUSB_SPI_WRITE2,
	ATUSB_SPI_READ1,
	ATUSB_SPI_READ2,
};

void ep0_init(void);
