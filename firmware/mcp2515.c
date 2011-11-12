#include "defines.h"

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "can.h"
#include "spi.h"
#include "mcp2515.h"
#include "error.h"

struct can_config can_cfg;

static uint8_t ctrl;
static uint8_t status;
static uint8_t tx_empty;
static uint8_t error;

uint8_t mcp2515_read_reg (uint8_t addr)
{
	uint8_t ret;

	can_cs_l();

	spi_io(INSTRUCTION_READ);
	spi_io(addr);
	ret = spi_io(0xff);

	can_cs_h();

	return ret;
}

void mcp2515_write_reg (uint8_t addr, uint8_t data)
{
	can_cs_l();

	spi_io(INSTRUCTION_WRITE);
	spi_io(addr);
	spi_io(data);

	can_cs_h();
}

void mcp2515_write_bits (uint8_t addr, uint8_t mask, uint8_t val)
{
	can_cs_l();

	spi_io(INSTRUCTION_BIT_MODIFY);
	spi_io(addr);
	spi_io(mask);
	spi_io(val);

	can_cs_h();
}

void mcp2515_update_status (void)
{
	status = mcp2515_read_reg(CANINTF);

	status &= CANINTF_RX | CANINTF_TX | CANINTF_ERR;

	if (status & CANINTF_ERR) {
		error = mcp2515_read_reg(EFLG);
		mcp2515_write_bits(EFLG, error, 0x00);
	}

	if (status & CANINTF_TX)
		tx_empty = 1;

	mcp2515_write_bits(CANINTF, status & (CANINTF_ERR | CANINTF_TX), 0x00);
}

static void mcp2515_reset (void)
{
	can_cs_l();

	spi_io(INSTRUCTION_RESET);

	can_cs_h();
}

uint8_t mcp2515_tx (struct can_frame * frame)
{
	uint32_t eid, sid;
	uint8_t rtr, exide;
	uint8_t i;

	tx_empty = 0;

	exide = (frame->can_id & CAN_EFF_FLAG) ? 1 : 0; /* Extended ID Enable */
	if (exide)
		sid = (frame->can_id & CAN_EFF_MASK) >> 18;
	else
		sid = frame->can_id & CAN_SFF_MASK; /* Standard ID */
	eid = frame->can_id & CAN_EFF_MASK; /* Extended ID */
	rtr = (frame->can_id & CAN_RTR_FLAG) ? 1 : 0; /* Remote transmission */

	can_cs_l();

	spi_io(INSTRUCTION_LOAD_TXB(0));
	spi_io(sid >> SIDH_SHIFT); /* TXBnSIDH*/
	spi_io(((sid & SIDL_SID_MASK) << SIDL_SID_SHIFT) |
	       (exide << SIDL_EXIDE_SHIFT) |
	       ((eid >> SIDL_EID_SHIFT) & SIDL_EID_MASK)); /* TXBnSIDL */
	spi_io(GET_BYTE(eid, 1)); /* TXBnEID8 */
	spi_io(GET_BYTE(eid, 0)); /* TXBnEID0 */
	spi_io((rtr << DLC_RTR_SHIFT) | frame->can_dlc); /* TXBnDLC */
	for (i = 0; i < frame->can_dlc; i++) /* TXBnDn */
		spi_io(frame->data[i]);

	can_cs_h();

	mcp2515_write_reg(TXBCTRL(0), TXBCTRL_TXREQ);

	return 0;
}

void mcp2515_rx (struct can_frame * frame)
{
	uint8_t ret;
	uint8_t buf;
	uint8_t i;

	ret = mcp2515_read_reg(CANINTF);

	can_cs_l();
	spi_io(INSTRUCTION_READ_RXB((ret & CANINTF_RX0IF) ? 0 : 1));

	buf = spi_io(0xff); /* RXBnSIDH */
	frame->can_id = buf << RXBSIDH_SHIFT;
	buf = spi_io(0xff); /* RXBnSIDL */
	frame->can_id |= buf >> RXBSIDL_SHIFT;

	if (buf & RXBSIDL_IDE) {
		frame->can_id = frame->can_id << 18;
		frame->can_id |= CAN_EFF_FLAG;
		frame->can_id |= (uint32_t)(buf & RXBSIDL_EID) << 16;
		buf = spi_io(0xff); /* RXBnEID8 */
		frame->can_id |= (uint32_t)buf << 8;
		buf = spi_io(0xff); /* RXBnEID0 */
		frame->can_id |= buf;
		buf = spi_io(0xff); /* RXBnDLC */
		frame->can_id |= (buf & RXBDLC_RTR) ? CAN_RTR_FLAG : 0;
	} else {
		frame->can_id |= (buf & RXBSIDL_SRR) ? CAN_RTR_FLAG : 0;
		spi_io(0xff); /* RXBnEID8 */
		spi_io(0xff); /* RXBnEID0 */
		buf = spi_io(0xff); /* RXBnDLC */
	}
	frame->can_dlc = buf & RXBDLC_LEN_MASK;

	for (i = 0; i < frame->can_dlc; i++) /* TXBnDn */
		frame->data[i] = spi_io(0xff); /* RXBnDn */

	can_cs_h();
}

void mcp2515_err (struct can_frame * frame)
{
	frame->can_id = CAN_ERR_FLAG;
	frame->can_dlc = CAN_ERR_DLC;
	memset(&frame->data[0], 0, frame->can_dlc);

	if (error & EFLG_TXBO) {
		frame->can_id |= CAN_ERR_BUSOFF;
	} else if (error & EFLG_TXEP) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_TX_PASSIVE;
	} else if (error & EFLG_RXEP) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_RX_PASSIVE;
	} else if (error & EFLG_TXWAR) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_TX_WARNING;
	} else if (error & EFLG_RXWAR) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_RX_WARNING;
	}

	if (error & (EFLG_RX0OVR | EFLG_RX1OVR)) {
		frame->can_id |= CAN_ERR_CRTL;
		frame->data[1] |= CAN_ERR_CRTL_RX_OVERFLOW;
	}
}

uint8_t mcp2515_txbuf_empty (void)
{
	if (can_cfg.mode & CAN_CTRLMODE_ONE_SHOT)
		return 1;
	else
		return tx_empty;
}

uint8_t mcp2515_has_data (void)
{
	return (status & CANINTF_RX);
}

uint8_t mcp2515_has_errors (void)
{
	return (status & CANINTF_ERR);
}

uint8_t mcp2515_start (void)
{
	uint8_t cnf1, cnf2, cnf3;

	cnf1 = ((can_cfg.sjw - 1) << CNF1_SJW_SHIFT) | (can_cfg.brp / 2 - 1);

	cnf2 = CNF2_BTLMODE |
		(can_cfg.mode & CAN_CTRLMODE_3_SAMPLES ? CNF2_SAM : 0) |
		((can_cfg.phase_seg1 - 1) << CNF2_PS1_SHIFT) |
		(can_cfg.prop_seg - 1);

	cnf3 = can_cfg.phase_seg2 - 1;

	ctrl &= ~CANCTRL_REQOP_MASK;

	if (can_cfg.mode & CAN_CTRLMODE_ONE_SHOT)
	  ctrl |= CANCTRL_OSM;

	if (can_cfg.mode & CAN_CTRLMODE_LOOPBACK)
		ctrl |= CANCTRL_REQOP_LOOPBACK;
	else if (can_cfg.mode & CAN_CTRLMODE_LISTENONLY)
		ctrl |= CANCTRL_REQOP_LISTEN_ONLY;
	else
		ctrl |= CANCTRL_REQOP_NORMAL;

	mcp2515_write_reg(CNF1, cnf1);
	mcp2515_write_reg(CNF2, cnf2);
	mcp2515_write_reg(CNF3, cnf3);
	mcp2515_write_reg(CANCTRL, ctrl);

	tx_empty = 1;

	return 0;
}

uint8_t mcp2515_stop (void)
{
	ctrl &= ~(CANCTRL_REQOP_MASK | CANCTRL_OSM);
	ctrl |= CANCTRL_REQOP_CONF;

	mcp2515_write_reg(CANCTRL, ctrl);
	mcp2515_write_reg(TXBCTRL(0), 0x00);
	mcp2515_write_reg(CNF1, 0x00);
	mcp2515_write_reg(CNF2, 0x00);
	mcp2515_write_reg(CNF3, 0x00);

	return 0;
}

void mcp2515_init (uint8_t clkpre)
{
	mcp2515_reset();

	_delay_ms(10);

	ctrl = CANCTRL_REQOP_CONF | CANCTRL_CLKEN | (clkpre & 0x03);

	mcp2515_write_reg(CANCTRL, ctrl);

	_delay_ms(10);

	memset(&can_cfg, 0, sizeof(can_cfg));
}
