/*
 * Copyright 2011 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * Based on the original ben-wpan code written by
 *   Werner Almesberger, Copyright 2011
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef SERNUM_H
#define	SERNUM_H

#include <stdint.h>

#include "board.h"

int sernum_get_descr(uint8_t type, uint8_t index, const uint8_t **reply,
		uint8_t *size);

#endif /* !SERNUM_H */
