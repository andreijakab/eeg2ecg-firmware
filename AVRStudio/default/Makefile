###############################################################################
# Makefile for the project EEG-2-ECG
###############################################################################

## General Flags
PROJECT = EEG-2-ECG
MCU = atmega164p
TARGET = EEG-2-ECG.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99  -D_SNS1_SNSK1_SAME_PORT_  -DQT_NUM_CHANNELS=4  -DQT_DELAY_CYCLES=10  -DQTOUCH_STUDIO_MASKS=1  -DNUMBER_OF_PORTS=1  -D_POWER_OPTIMIZATION_=0  -D_QTOUCH_  -DSNS1=B  -DSNSK1=B      -DF_CPU=4000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=EEG-2-ECG.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"D:\User Data\Andrei\Documents\BME\Work\EEGEM\EEG-ECG\Firmware\AVRStudio\..\..\..\..\..\..\..\..\..\Atmel_QTouch_Libraries_4.3\Generic_QTouch_Libraries\include" -I"D:\User Data\Andrei\Documents\BME\Work\EEGEM\EEG-ECG\Firmware\AVRStudio\..\..\..\..\..\..\..\..\..\Atmel_QTouch_Libraries_4.3\Generic_QTouch_Libraries\AVR_Tiny_Mega_XMega\QTouch\common_files" 

## Library Directories
LIBDIRS = -L"D:\Atmel_QTouch_Libraries_4.3\Generic_QTouch_Libraries\AVR_Tiny_Mega_XMega\QTouch\library_files" 

## Libraries
LIBS = -lavr51g1-4qt-k-0rs 

## Objects that must be built in order to link
OBJECTS = acc_check.o alarms.o gain_adjust.o main.o calib_RC_32kHz.o avr_adc.o avr_timer0.o avr_timer1.o avr_timer2.o mma7341lc.o pga112.o qtouch_key.o qt_asm_tiny_mega.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) EEG-2-ECG.hex EEG-2-ECG.eep EEG-2-ECG.lss size

## Compile
acc_check.o: ../../Source/acc_check.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

alarms.o: ../../Source/alarms.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

gain_adjust.o: ../../Source/gain_adjust.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: ../../Source/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

calib_RC_32kHz.o: ../../Source/calibration/calib_RC_32kHz.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

avr_adc.o: ../../Source/drivers/avr_adc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

avr_timer0.o: ../../Source/drivers/avr_timer0.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

avr_timer1.o: ../../Source/drivers/avr_timer1.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

avr_timer2.o: ../../Source/drivers/avr_timer2.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

mma7341lc.o: ../../Source/drivers/mma7341lc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pga112.o: ../../Source/drivers/pga112.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

qtouch_key.o: ../../Source/drivers/qtouch_key.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

qt_asm_tiny_mega.o: ../../../../../../../../../../Atmel_QTouch_Libraries_4.3/Generic_QTouch_Libraries/AVR_Tiny_Mega_XMega/QTouch/common_files/qt_asm_tiny_mega.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

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
	-rm -rf $(OBJECTS) EEG-2-ECG.elf dep/* EEG-2-ECG.hex EEG-2-ECG.eep EEG-2-ECG.lss EEG-2-ECG.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

