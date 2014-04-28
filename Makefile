# Pulse & Bloom
# Author: Samuel Clay & Rohan Dixit
# Date: 2014-04-27
# Copyright: (c) 2014

DEVICE      = attiny84
DEVICE_MCCU = attiny84     # See http://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
PROGRAMMER ?= usbtiny
F_CPU       = 8000000   # in Hz
FUSE_L      = 0xFF
FUSE_H      = 0xDF

AVRDUDE     = avrdude -v -v -v -v -c $(PROGRAMMER) -p $(DEVICE) -P usb

LIBS        = -I./tiny 
LIBS        += -I./SoftwareSerial 
LIBS        += -I./avr -I.

CFLAGS      = $(LIBS) \
              -fno-exceptions -ffunction-sections -fdata-sections \
              -Wl,--gc-sections

C_SRC   := tiny/pins_arduino.o \
           tiny/wiring.o \
           tiny/wiring_analog.o \
           tiny/wiring_digital.o \
           tiny/wiring_pulse.o \
           tiny/wiring_shift.o
CPP_SRC := tiny/main.o \
           tiny/Print.o \
           tiny/Tone.o \
           tiny/WMath.o \
           tiny/WString.o \
           SoftwareSerial/SoftwareSerial.o
SRC     := pulse.o sampler.o

OBJECTS = $(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o) $(SRC:.cpp=.o)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE_MCCU)

all: program

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
	$(AVRDUDE) -e -Uefuse:w:0xFF:m -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

# rule for uploading firmware:
flash: pulse.hex
	$(AVRDUDE) -U flash:w:pulse.hex:i

read:
	$(AVRDUDE) -U lfuse:r:lfuse.txt:h -U hfuse:r:hfuse.txt:h -U efuse:r:efuse.txt:h -U lock:r:lock.txt:h 

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f pulse.hex pulse.lst pulse.obj pulse.cof pulse.list pulse.map pulse.eep.hex pulse.elf pulse.s core.a *.o **/*.o **/**/*.o *.d **/*.d **/**/*.d

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -g -c $< -o $@

.cpp.o:
	$(COMPILE) -g -c $< -o $@

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

core:
	avr-ar rcs core.a $(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o)

# file targets:
pulse.elf: $(OBJECTS) core
	$(COMPILE) -o pulse.elf $(SRC:.cpp=.o) core.a -L. -lm

pulse.hex: pulse.elf
	rm -f pulse.hex pulse.eep.hex
	avr-objcopy -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 -O ihex pulse.elf pulse.hex
	avr-objcopy -O ihex -R .eeprom pulse.elf pulse.hex
	avr-size pulse.hex

disasm: pulse.elf
	avr-objdump -d pulse.elf

