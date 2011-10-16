/*
 * fw/sernum.h - ATUSB serial number
 *
 * Written 2011 by Werner Almesberger
 * Copyright 2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef SERNUM_H
#define	SERNUM_H

#include <stdint.h>

#include "board.h"


#ifdef HAS_BOARD_SERNUM

int sernum_get_descr(uint8_t type, uint8_t index, const uint8_t **reply,
    uint8_t *size);

#else /* HAS_BOARD_SERNUM */

static inline int sernum_get_descr(uint8_t type, uint8_t index,
    const uint8_t **reply, uint8_t *size)
{
	return 0;
}

#endif /* !HAS_BOARD_SERNUM */

#endif /* !SERNUM_H */
