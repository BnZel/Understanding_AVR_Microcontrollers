CC = avr-gcc 
OBJCPY = avr-objcopy
SIZE = avr-size
MCU = atmega328p
F_CPU = 16000000
U8G2_SRC = ./u8g2/csrc
CFLAGS = \
	-mmcu=$(MCU) \
	-DF_CPU=$(F_CPU)UL \
	-Os \
	-std=gnu99 \
	-Werror \
	-ffunction-sections \
	-fdata-sections \
	-I$(U8G2_SRC)/ \
	-I./u8g2/sys/avr/avr-libc/lib/ \
	-DAVR_USE_HW_I2C 
LDFLAGS = \
	-Wl,--gc-sections \
	-mmcu=$(MCU)
AVRDUDE=avrdude
PORT=$(shell pavr2cmd --prog-port)

SRC = $(shell ls $(U8G2_SRC)/*.c) \
	  $(shell ls ./u8g2/sys/avr/avr-libc/lib/*.c) \
	  $(shell ls ./u8g2/sys/avr/avr-libc/lib/avr-hw-i2c/*.c) \
	  $(shell ls ./*.c) \

OBJ = $(SRC:.c=.o)

main.hex: main.elf
	$(OBJCPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature main.elf main.hex

main.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $@

size: main.elf
	$(SIZE) --mcu=$(MCU) --format=avr main.elf

clean:
	rm -f $(OBJ) main.elf main.hex
	rm -f cmd.txt

upload: main.hex
	avrdude -c stk500v2 -P "$(PORT)" -p $(MCU) -U flash:w:$<:i -B 0.5 

memory:
	avr-size -C --mcu=atmega328p -f 16000000 main.elf

auto: clean
	make && make upload
	avr-size -C --mcu=atmega328p main.elf