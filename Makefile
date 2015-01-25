# Chip settings
MCU=atmega328p
F_CPU=16000000L

# Toolchain
CC=avr-gcc
CFLAGS=-Wall -O3 -DF_CPU=$(F_CPU) -mmcu=$(MCU)
OBJCOPY=avr-objcopy

# Flashing settings
PROTOCOL=arduino
AVRDUDE=avrdude -F -V
flash-u2: PORT:=/dev/ttyACM0
flash-u2: BAUD=115200
flash-ftdi: PORT:=/dev/ttyUSB0
flash-ftdi: BAUD=57600

# Other
BIN_FORMAT=ihex
RM=rm -f
AVRSIZE=avr-size -A

.PHONY: all
all:
	@echo "Available targets:"
	@echo "ftdi, flash-ftdi:"
	@echo "  For the Arduinos with an FTDI"
	@echo "  compatible flashrom arguments: flashrom -p serprog:dev=/dev/ttyUSB0:2000000"
	@echo "  Other boards using an hardware USB<->Serial converter might work too."
	@echo "u2, flash-u2:"
	@echo "  For the Arduino with a 8u2 or a 16u2"
	@echo "  compatible flashrom arguments: flashrom -p serprog:dev=/dev/ttyACM0:115200"

%:
	mkdir -p bin/
	$(CC) $(CFLAGS) -Iconfig/$@/ -S -o bin/$@.s src/serprog.c
	$(CC) $(CFLAGS) -s -o bin/$@.elf bin/$@.s
	$(AVRSIZE) bin/$@.elf
	$(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom bin/$@.elf bin/$@.hex

flash-%:
	$(AVRDUDE) -c $(PROTOCOL) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:bin/$(subst flash-,,$@).hex

clean:
	$(RM) -rf bin/*
