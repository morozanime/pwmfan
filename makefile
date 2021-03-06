
PLATFORM = M328

TARGET = PWM
ASRC = 
CSRC = 
CSRC += main.c

MCU	=	atmega328p

#--------------------------------------------------------------------
OBJ	= $(CSRC:.c=.obj)
DEP	= $(CSRC:.c=.d)

MCUToolsBaseDirEnv = c:/work/AVR/gcc
UploadPort = COM3

INCLUDE_DIRS += |"$(MCUToolsBaseDirEnv)/avr/include"

DEFINES =  |$(PLATFORM)=1

CFLAGS = 	-Wall
CFLAGS +=	-x c
CFLAGS +=	-Os
CFLAGS +=	-fpack-struct
CFLAGS +=	-fshort-enums
CFLAGS +=	-ffunction-sections
CFLAGS +=	-fdata-sections
CFLAGS +=	-std=gnu99
CFLAGS +=	-funsigned-char
CFLAGS +=	-funsigned-bitfields
CFLAGS +=	-mmcu=$(MCU)
CFLAGS +=	-c
CFLAGS +=   $(subst |,-I,$(INCLUDE_DIRS))
CFLAGS +=   $(subst |,-D,$(DEFINES))

LDFLAGS = 	-Wl,-Map,build/$(PLATFORM)/$(TARGET).map
LDFLAGS +=	-Wl,-gc-sections
LDFLAGS +=	-mmcu=$(MCU)

REMOVE =		rm -f
CC =			"$(MCUToolsBaseDirEnv)/bin/avr-gcc.exe"
LD =			"$(MCUToolsBaseDirEnv)/bin/avr-gcc.exe"
OBJDUMP =		"$(MCUToolsBaseDirEnv)/bin/avr-objdump.exe"
OBJCOPY =		"$(MCUToolsBaseDirEnv)/bin/avr-objcopy.exe"
SIZE =			"$(MCUToolsBaseDirEnv)/bin/avr-size.exe"
PROG = 			"$(MCUToolsBaseDirEnv)/bin/avrdude.exe"

all: sizebefore	compile sizeafter

compile: elf lss hex

ELFSIZE	= $(SIZE) -A build/$(PLATFORM)/$(TARGET).elf

sizebefore:
	@if test -f build/$(PLATFORM)/$(TARGET).elf; then $(ELFSIZE); echo; fi

sizeafter:
	@if test -f build/$(PLATFORM)/$(TARGET).elf; then $(ELFSIZE); echo; fi

obj: $(OBJ)

%.obj : %.c
	$(CC) $(CFLAGS) -o $@ $< 2>&1
	
elf: obj
	@if ! test -d build; then mkdir build; fi
	@if ! test -d build/$(PLATFORM); then mkdir build/$(PLATFORM); fi 
	$(LD) $(LDFLAGS) -o build/$(PLATFORM)/$(TARGET).elf $(OBJ)
	  
lss: elf
	$(OBJDUMP) -h -S build/$(PLATFORM)/$(TARGET).elf > build/$(PLATFORM)/$(TARGET).lss	

hex: elf
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex build/$(PLATFORM)/$(TARGET).elf build/$(PLATFORM)/$(TARGET).hex

clean:
	$(REMOVE) $(OBJ)
	$(REMOVE) $(DEP)
	$(REMOVE) $(CSRC_)

fuse:
	$(PROG)	-c usbasp -p $(MCU) -B 100  -U efuse:w:0xFD:m -U hfuse:w:0xD9:m -U lfuse:w:0xFF:m
flash:
	$(PROG)	-c usbasp -p $(MCU) -U flash:w:build/$(PLATFORM)/$(TARGET).hex
lock:
	$(PROG)	-c usbasp -p $(MCU) -U lock:w:0xFC:m
reset:
	$(PROG)	-c usbasp -p $(MCU)
upload:
	$(PROG) -C$(MCUToolsBaseDirEnv)/bin/avrdude.conf -v -p$(MCU) -carduino -P$(UploadPort) -b57600 -D -Uflash:w:build/$(PLATFORM)/$(TARGET).hex

.PHONY: clean size hex elf lss all c c_ s s_ build compile
