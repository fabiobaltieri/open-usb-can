/*
 * fw/board.h - Board-specific functions and definitions
 *
 * Written 2008-2011 by Werner Almesberger
 * Copyright 2008-2011 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

void panic(void);
void reset_cpu(void);
int gpio(uint8_t port, uint8_t data, uint8_t dir, uint8_t mask, uint8_t *res);

void board_init(void);
