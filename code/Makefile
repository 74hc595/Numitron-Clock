# By default, use the first connected FTDI USB-serial cable we find. (macOS)
# May be overridden by setting the $FTDI_DEV environment variable in your shell
# configuration.
FTDI_DEV ?= $(firstword $(wildcard /dev/cu.usbserial-*))

PROGRAM_NAME = numitron-clock
LINKER_SCRIPT = lpc811.ld
OBJ = delay.o \
      pins.o \
      spi.o \
      display.o \
      beeper.o \
      button.o \
      bcd.o \
      dst_dates.o \
      date.o \
      rtc.o \
      app.o \
      main.o \
      system_LPC8xx.o \
      gcc_startup_lpc8xx.o \

ELF = $(PROGRAM_NAME).elf
HEX = $(PROGRAM_NAME).hex
DISASM = $(PROGRAM_NAME).s

CROSS = arm-none-eabi-
AS = $(CROSS)gcc
CC = $(CROSS)gcc
CXX = $(CROSS)g++
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
SIZE = $(CROSS)size
LPC21ISP = lpc21isp

# use these on the $(LD) line, as needed:
LIBM = "$(shell $(CC) $(CFLAGS) --print-file-name=libm.a)"
LIBC = "$(shell $(CC) $(CFLAGS) --print-file-name=libc.a)"
LIBGCC = "$(shell $(CC) $(CFLAGS) --print-libgcc-file-name)"

CFLAGS_COMMON += -Os -Wall -I./cmsis -D__USE_CMSIS -DINLINE=inline
CFLAGS_COMMON += -mthumb -mcpu=cortex-m0plus -MMD -MP --specs=nano.specs -ffunction-sections -fdata-sections -fno-builtin -fno-common -ffreestanding
CFLAGS += $(CFLAGS_COMMON) -std=gnu11 -Dstatic_assert=_Static_assert -Dauto=__auto_type
CXXFLAGS += $(CFLAGS_COMMON) -std=gnu++11 -fno-exceptions -fno-non-call-exceptions -fno-rtti -fno-use-cxa-atexit
ASFLAGS += $(CFLAGS) -D__ASSEMBLY__ -x assembler-with-cpp
LDFLAGS += --gc-sections -T $(LINKER_SCRIPT)
DEPS = $(OBJ:.o=.d)


.PHONY: all cable_check clean flash run

all: $(ELF) $(HEX) $(DISASM)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

$(ELF): $(OBJ)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBGCC)
	@$(SIZE) $@

$(HEX): $(ELF)
	$(OBJCOPY) --strip-unneeded -O ihex $< $@

$(DISASM): $(ELF)
	$(OBJDUMP) -d $< > $@

clean:
	rm -f *.o *.d
	rm -f $(ELF) $(HEX) $(DISASM)

cable_check:
	@echo "Checking for FTDI cable at '$(FTDI_DEV)'"
	@test -n "$(FTDI_DEV)"
	@test -e "$(FTDI_DEV)"

flash: $(HEX) cable_check
	$(LPC21ISP) -wipe -verify $(HEX) $(FTDI_DEV) 115200 30000

run: $(HEX) cable_check
	$(LPC21ISP) -term -wipe -verify $(HEX) $(FTDI_DEV) 115200 30000

-include $(DEPS)
