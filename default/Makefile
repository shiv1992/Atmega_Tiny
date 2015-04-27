###############################################################################
# Makefile for the project Atmega128_MT
###############################################################################

## General Flags
PROJECT = Atmega128_MT
MCU = atmega128
TARGET = Atmega128_MT.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2                     -DF_CPU=16000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=Atmega128_MT.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\Users\sanjay trivedi\Desktop\Backup_spray\Atmega_new\Atmega_new\." -I"C:\Users\sanjay trivedi\Desktop\Backup_spray\Atmega_new\Atmega_new\avr" 

## Objects that must be built in order to link
OBJECTS = Atmega128_MT.o lcd.o system.o delay.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) Atmega128_MT.hex Atmega128_MT.eep Atmega128_MT.lss size

## Compile
Atmega128_MT.o: ../Atmega128_MT.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

lcd.o: ../lcd.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

system.o: ../system.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

delay.o: ../delay.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) Atmega128_MT.elf dep/* Atmega128_MT.hex Atmega128_MT.eep Atmega128_MT.lss Atmega128_MT.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

