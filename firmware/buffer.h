struct usb_header {
        uint8_t frame_count;
	uint8_t spare0;
	uint8_t spare1;
	uint8_t free_slots;
};

#define RX_MAX_FRAMES 4

struct usb_rx_buffer {
	struct usb_header hdr;
	struct can_frame frames[RX_MAX_FRAMES];
};

void buffer_tx_process (void);
void buffer_rx_process (void);
void buffer_reset (void);
