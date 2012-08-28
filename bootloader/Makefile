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
BOOT_ADDR=0x7000
F_CPU=8000000

# Options
CFLAGS	= -Os -Iusb -Wall -mmcu=$(MCU) -DBOOT_ADDR=$(BOOT_ADDR) -DF_CPU=$(F_CPU)
ASFLAGS	= 
LDFLAGS	= -Wl,--section-start=.text=$(BOOT_ADDR)

AVRDUDEFLAGS = -cusbasp -p$(MCU)
DFU_FLAGS = at90usb162
DFU_UTIL_FLAGS = -R

# for ATxx
LFUSE=0xd0
HFUSE=0xd8

LOCK = -U lock:w:0x2f:m

targets	= main.elf main_text.hex main_text.bin main_eeprom.hex main_eeprom.bin
objects	= boot.o board.o sernum.o flash.o spi.o usb/dfu.o usb/dfu_common.o usb/usb.o usb/atu2.o
deps	= Makefile defines.h

include Makefile.targets
