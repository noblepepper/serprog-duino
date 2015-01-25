CC=avr-gcc
CFLAGS=-Wall -O3 -DF_CPU=$(F_CPU) -mmcu=$(MCU)
MCU=atmega328p
F_CPU=16000000L

OBJCOPY=avr-objcopy
BIN_FORMAT=ihex

PORT=/dev/ttyACM0
BAUD=115200
PROTOCOL=arduino
PART=$(MCU)
AVRDUDE=avrdude -F -V
AVRSIZE=avr-size -A

RM=rm -f

.PHONY: all
all: size

bin/serprog.hex: bin/serprog.elf
	$(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@

bin/serprog.elf: bin/serprog.s
	$(CC) $(CFLAGS) -s -o $@ $<

bin/serprog.s: src/serprog.c
	$(CC) $(CFLAGS) -S -o $@ $<

src/serprog.c: bin

bin:
	mkdir bin/

.PHONY: clean
clean:
	$(RM) bin/serprog.elf bin/serprog.hex bin/serprog.s

.PHONY: upload
upload: bin/serprog.hex
	$(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -U flash:w:$<

.PHONY: size
size: bin/serprog.elf
	$(AVRSIZE) $<
