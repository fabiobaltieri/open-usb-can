#include "defines.h"

#include <avr/io.h>
#include <util/delay.h>

#include "can.h"
#include "spi.h"
#include "mcp2515.h"

static uint8_t mcp2515_read_reg (uint8_t addr)
{
  uint8_t ret;
  
  can_cs_l();
  
  spi_io(INSTRUCTION_READ);
  
  spi_io(addr);
  
  ret = spi_io(0xff);
  
  can_cs_h();

  return ret;
}

static void mcp2515_write_reg (uint8_t addr, uint8_t data)
{
  can_cs_l();
  
  spi_io(INSTRUCTION_WRITE);
  
  spi_io(addr);
  
  spi_io(data);
  
  can_cs_h();
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

  i = 0;
  while (mcp2515_read_reg(TXBCTRL(0)) & TXBCTRL_TXREQ) {
    _delay_us(1);
    if (i++ > 100) {
      return -1;
    }
  }

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

uint8_t mcp2515_has_data (void)
{
  uint8_t ret;
  
  ret = mcp2515_read_reg(CANINTF);
  
  return (ret & (CANINTF_RX0IF | CANINTF_RX1IF));
}

void mcp2515_init (void)
{
  mcp2515_reset();

  _delay_ms(10);

  mcp2515_write_reg(CNF1, 0x41);
  mcp2515_write_reg(CNF2, 0xac);
  mcp2515_write_reg(CNF3, 0x03);

  mcp2515_write_reg(CANCTRL, 0x00);

  _delay_ms(10);
}
