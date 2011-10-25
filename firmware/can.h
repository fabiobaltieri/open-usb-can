/* special address description flags for the CAN_ID */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/*
 * Controller Area Network Identifier structure
 *
 * bit 0-28     : CAN identifier (11/29 bit)
 * bit 29       : error frame flag (0 = data frame, 1 = error frame)
 * bit 30       : remote transmission request flag (1 = rtr frame)
 * bit 31       : frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
typedef uint32_t canid_t;

/**
 * struct can_frame - basic CAN frame structure
 * @can_id:  the CAN ID of the frame and CAN_*_FLAG flags, see above.
 * @can_dlc: the data length field of the CAN frame
 * @data:    the CAN frame payload.
 */
struct can_frame {
	canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
	uint8_t can_dlc; /* data length code: 0 .. 8 */
	uint8_t data[8];
};

#define CAN_CTRLMODE_LOOPBACK           0x01    /* Loopback mode */
#define CAN_CTRLMODE_LISTENONLY         0x02    /* Listen-only mode */
#define CAN_CTRLMODE_3_SAMPLES          0x04    /* Triple sampling mode */
#define CAN_CTRLMODE_ONE_SHOT           0x08    /* One-Shot mode */
struct can_config {
	uint8_t mode;		/* Bus mode */
	uint8_t prop_seg;	/* Propagation segment in TQs */
	uint8_t phase_seg1;	/* Phase buffer segment 1 in TQs */
	uint8_t phase_seg2;	/* Phase buffer segment 2 in TQs */
	uint8_t sjw;		/* Synchronisation jump width in TQs */
	uint8_t brp;		/* Bit-rate prescaler */
};
