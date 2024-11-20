export CC = avr-gcc

export MCU = atmega8u2
export TARGET_ARCH = -mmcu=$(MCU)

export CFLAGS =  -Wall -I. -DF_CPU=16000000 -Os #-g
export LDFLAGS = -g $(TARGET_ARCH) -lm -Wl,--gc-sections #	-Os


TARGET = blink
TERM = /dev/ttyACM0
#TERM = /dev/ttyACM0
CPPFLAGS = -mmcu=$(MCU)
PGMER = -c stk500v1 -b 19200 -P $(TERM)
PGMERISP = -c stk500v1 -b 115200 -P $(TERM)
ARVDUDECONF= -C /usr/local/arduino/arduino-0021/hardware/tools/avrdude.conf
export DUDE = /usr/bin/avrdude -F -v -p $(MCU) $(AVRDUDECONF)

C_SRC = $(wildcard *.c)
OBJS = $(C_SRC:.c=.o)


all: $(TARGET).hex

ass:$(C_SRC)
	$(CC) -S $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f *.o

%.o:%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@


$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex $(TARGET).elf eeprom.hex

upload: $(TARGET).hex
	stty -F $(TERM) hupcl # reset
	$(DUDE) $(PGMER) -U flash:w:$(TARGET).hex
#	$(DUDE) $(PGMERISP) -U flash:w:$(TARGET).hex

size: $(TARGET).elf
	avr-size --format=avr --mcu=$(MCU) $(TARGET).elf

