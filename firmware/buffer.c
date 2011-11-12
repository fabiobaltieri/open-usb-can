#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "usb.h"
#include "ep0.h"
#include "can.h"
#include "buffer.h"
#include "spi.h"
#include "mcp2515.h"
#include "blink.h"

#include "defines.h"

#define TXBUFSZ 32
static struct can_frame tx_buf[TXBUFSZ];
static volatile uint8_t tx_buf_w;
static volatile uint8_t tx_buf_r;
static volatile uint8_t tx_buf_count;

static struct usb_rx_buffer rx_buf[2];
static volatile uint8_t rx_slot;

static volatile uint8_t update_needed;

static void buffer_tx_prepare (void);

static void buffer_tx_done (void *user)
{
	tx_buf_w = (tx_buf_w + 1) % TXBUFSZ;
	tx_buf_count++;

	update_needed = 1;

	/* prepare for another frame */
	buffer_tx_prepare();
}

static void buffer_tx_prepare (void)
{
	if (tx_buf_count < TXBUFSZ && eps[1].state == EP_IDLE) {
		usb_recv(&eps[1], (uint8_t *)&tx_buf[tx_buf_w],
			 sizeof(struct can_frame),
			 buffer_tx_done, NULL);
	}
}

void buffer_tx_process (void)
{
	cli();
	buffer_tx_prepare();
	sei();

	cli();
	if (tx_buf_count > 0 && mcp2515_txbuf_empty()) {
		mcp2515_tx(&tx_buf[tx_buf_r]);

		tx_buf_r = (tx_buf_r + 1) % TXBUFSZ;
		tx_buf_count--;

		update_needed = 1;

		blink_tx();
	}
	sei();
}

static void buffer_rx_done (void *user)
{
	struct usb_rx_buffer *rxb;

	rxb = user;

	memset(rxb, 0, sizeof(struct usb_rx_buffer));
}

void buffer_rx_process (void)
{
	cli();
	if (mcp2515_has_data()) {
		uint8_t *offset;

		offset = &rx_buf[rx_slot].hdr.frame_count;

		if (*offset < RX_MAX_FRAMES) {
			mcp2515_rx(&rx_buf[rx_slot].frames[*offset]);

			(*offset)++;

			blink_rx();
		}
	}
	sei();

	cli();
	if (eps[2].state == EP_IDLE &&
	    (update_needed || rx_buf[rx_slot].hdr.frame_count > 0)) {
		uint8_t size;

		size = sizeof(struct usb_header) +
			sizeof(struct can_frame) * rx_buf[rx_slot].hdr.frame_count;

		rx_buf[rx_slot].hdr.free_slots = TXBUFSZ - tx_buf_count;

		usb_send(&eps[2], (uint8_t *)&rx_buf[rx_slot],
			 size, buffer_rx_done, &rx_buf[rx_slot]);

		update_needed = 0;

		rx_slot ^= 1;
	}
	sei();
}

void buffer_reset (void)
{
	tx_buf_r = tx_buf_w;
	tx_buf_count = 0;

	memset(rx_buf, 0, sizeof(rx_buf));
	rx_slot = 0;

	update_needed = 0;
}
