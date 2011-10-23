#include "defines.h"

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

void spi_init (void)
{
  SPI_DDR  |= _BV(SPI_SCK) | _BV(SPI_MOSI) | _BV(SPI_SS);
  SPI_PORT |= _BV(SPI_SS);
  
  SPCR = ( (0 << SPIE) |
	   (1 << SPE)  |
	   (0 << DORD) |
	   (1 << MSTR) |
	   (0 << CPOL) | (0 << CPHA) |
	   (0 << SPR1) | (0 << SPR0) );

  SPSR = ( (1 << SPI2X) );
}

uint8_t spi_io (uint8_t data)
{
  uint8_t ret;

  SPDR = data;
  ret = 0xff;
  
  while (!(SPSR & _BV(SPIF)))
    ;

  ret = SPDR;

  return ret;
}
