# Pulse & Bloom
# Author: Samuel Clay & Rohan Dixit
# Date: 2014-04-27
# Copyright: (c) 2014

DEVICE      = attiny84
DEVICE_MCCU = attiny84     # See http://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
PROGRAMMER ?= usbtiny
F_CPU       = 16000000
FUSE_L      = 0xFF
FUSE_H      = 0xD7

AVRDUDE     = avrdude -v -v -v -v -c $(PROGRAMMER) -p $(DEVICE) -P usb

LIBS        =  -I./libs/tiny 
LIBS        += -I./libs/avr
LIBS        += -I./libs/SoftwareSerial
LIBS        += -I./libs
LIBS        += -I./src
LIBS        += -I.

CFLAGS      = $(LIBS) \
              -fno-exceptions -ffunction-sections -fdata-sections \
              -Wl,--gc-sections

C_SRC   := libs/tiny/pins_arduino.o \
           libs/tiny/wiring.o \
           libs/tiny/wiring_analog.o \
           libs/tiny/wiring_digital.o \
           libs/tiny/wiring_pulse.o \
           libs/tiny/wiring_shift.o
CPP_SRC := libs/tiny/main.o \
           libs/tiny/Print.o \
           libs/tiny/Tone.o \
           libs/tiny/WMath.o \
           libs/tiny/WString.o \
           libs/SoftwareSerial/SoftwareSerial.o
SRC     := src/pulse.o \
           src/smooth.o \
           src/sampler.o

OBJECTS = $(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o) $(SRC:.cpp=.o)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE_MCCU)

all: clean fuse program

# symbolic targets:
help:
	@echo "Use one of the following:"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make hex ....... to build pulse.hex"
	@echo "make clean ..... to delete objects and hex file"

hex: pulse.hex

# Add fuse to program once you're in production
program: flash

# rule for programming fuse bits:
fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U lfuse:w:$(FUSE_L):m -U hfuse:w:$(FUSE_H):m -U efuse:w:0xFF:m

# rule for uploading firmware:
flash: pulse.hex
	$(AVRDUDE) -U flash:w:build/pulse.hex:i

read:
	$(AVRDUDE) -U lfuse:r:lfuse.txt:h -U hfuse:r:hfuse.txt:h -U efuse:r:efuse.txt:h -U lock:r:lock.txt:h 

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f build/* *.o **/*.o **/**/*.o *.d **/*.d **/**/*.d **/**/*.lst

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -g -c $< -o $@

.cpp.o:
	$(COMPILE) -g -c $< -o $@

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

core:
	avr-ar rcs build/core.a $(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o)

# file targets:
pulse.elf: $(OBJECTS) core
	$(COMPILE) -o build/pulse.elf $(SRC:.cpp=.o) build/core.a -L. -lm

pulse.hex: pulse.elf
	rm -f build/pulse.hex build/pulse.eep.hex
	avr-objcopy -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 -O ihex build/pulse.elf build/pulse.hex
	avr-objcopy -O ihex -R .eeprom build/pulse.elf build/pulse.hex
	avr-size build/pulse.hex

disasm: pulse.elf
	avr-objdump -d build/pulse.elf

