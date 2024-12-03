export MCU = atmega8u2
export TARGET_ARCH = -mmcu=$(MCU)

export CFLAGS =  -Wall -I. -DF_CPU=16000000 -Os #-g
export LDFLAGS = -g $(TARGET_ARCH) -lm -Wl,--gc-sections #	-Os


TARGET = at90usb82-bl-usb-1_0_5
TERM = /dev/ttyACM0
#TERM = /dev/ttyACM0
CPPFLAGS = -mmcu=$(MCU)
PGMER = -c stk500v1 -b 19200 -P $(TERM)
ARVDUDECONF= -C /usr/local/arduino/arduino-0021/hardware/tools/avrdude.conf
export DUDE = /usr/bin/avrdude -F -v -p $(MCU) $(AVRDUDECONF)

C_SRC = $(wildcard *.c)
OBJS = $(C_SRC:.c=.o)


all:

reset:
	avrdude -c stk500v1 -p atmega8u2 -P $(TERM) -b 19200 -t
	#reset puis quit

fuses:
	avrdude -c avrisp -p atmega8u2 -P $(TERM) -b 19200 -U lfuse:w:0xde:m -U hfuse:w:0xd9:m -U efuse:w:0xf4:m
	#fuses pour horloge 16mhz externe

rex:
	stty -F $(TERM) hupcl # reset
	$(DUDE) $(PGMER) -U flash:w:$(TARGET).hex


# fichier bootloader d'une arduino UNO
arduino:
	avrdude -p at90usb82 -F $(PGMER) -U flash:w:UNO-dfu_and_usbserial_combined.hex -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

