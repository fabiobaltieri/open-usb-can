# Apps
CC	= avr-gcc
AS	= avr-as
LD	= avr-ld
OBJCOPY	= avr-objcopy
SIZE	= avr-size
GDB	= avr-gdb
AVRDUDE	= avrdude
DFUPROGRAMMER = dfu-programmer
DFU_UTIL = dfu-util

MCU = atmega32u2
F_CPU = 8000000UL

# Options
CFLAGS	= -Os -Iusb -Wall -mmcu=$(MCU) -DF_CPU=$(F_CPU)
ASFLAGS	= 
LDFLAGS	= 

AVRDUDEFLAGS = -cusbasp -p$(MCU)
DFU_FLAGS = at90usb162
DFU_UTIL_FLAGS = -R

# for ATxx
LFUSE=0xd0
HFUSE=0xd9

targets	= main.elf main_text.hex main_text.bin main_eeprom.hex main_eeprom.bin
objects	= version.o main.o board.o spi.o mcp2515.o buffer.o blink.o descr.o ep0.o usb/dfu_common.o usb/usb.o usb/atu2.o
deps	= Makefile defines.h

include Makefile.targets

.PHONY: version.c

version.c:
	@if [ -f .version ]; then \
		v=`cat .version`; \
		expr $$v + 1 > .version; \
	else \
		echo 0 > .version; \
	fi
	@[ -s .version ] || echo 0 > .version
	@echo '/* MACHINE-GENERATED. DO NOT EDIT ! */' >version.c
	@echo '#include "version.h"' >> version.c
	@echo "const char *build_date = \"`date`\";" >>version.c
	@echo "const uint16_t build_number = `cat .version`;" \
	>> version.c


