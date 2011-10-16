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

#ifndef BOARD_H
#define	BOARD_H

#include <stdint.h>


#define	USB_VENDOR	0x20b7	/* Qi Hardware */
#define	USB_PRODUCT	0x1540	/* ben-wpan atusb */

#define	DFU_USB_VENDOR	USB_VENDOR
#define	DFU_USB_PRODUCT	USB_PRODUCT


#define	BOARD_MAX_mA	40

#ifdef BOOT_LOADER
#define	NUM_EPS	1
#else
#define	NUM_EPS	2
#endif

#define	HAS_BOARD_SERNUM

extern uint8_t board_sernum[42];
extern uint8_t irq_serial;


void reset_rf(void);
void reset_cpu(void);
uint8_t read_irq(void);
void slp_tr(void);

void led(int on);
void panic(void);

uint64_t timer_read(void);
void timer_init(void);

int gpio(uint8_t port, uint8_t data, uint8_t dir, uint8_t mask, uint8_t *res);
void gpio_cleanup(void);

void board_init(void);
void board_app_init(void);

#endif /* !BOARD_H */
