# Overview
The firmware contained in this repository is for the EEG-to-ECG adapter that I developed as part of my Master of Science in Biomedical Engineering thesis. If you're curious and want to know more about why such a thing is useful, you can read my thesis in its entirety [here](http://urn.fi/URN:NBN:fi:tty-201104111176).

# Developer PC setup
|Software                     					|Version 	|Source                                                               |
|-----------------------------------------------|-----------|---------------------------------------------------------------------|
|AVR Studio 4               					|4.18.685	|https://www.microchip.com/en-us/tools-resources/archives/avr-sam-mcus|
|WinAVR                                         |20100110   |https://sourceforge.net/projects/winavr/files/                       |

# ATMEGA1284P Fuse Settings
## Fuse High Byte
|Bit Name|Bit #|Description                                           |Default|Setting|
|--------|-----|------------------------------------------------------|-------|-------|
|OCDEN   |7    |Enable OCD (0 = disabled, 1 = enabled)                |1      |1      |
|JTAGEN  |6    |Enable JTAG (0 = enabled, 1 = disabled)               |0      |1      |
|SPIEN   |5    |Enable SPI Serial (0 = enabled, 1 = disabled)         |0      |0      |
|CKOPT   |4    |Oscillator options (1 = internal RC oscillator)       |1      |1      |
|EESAVE  |3    |EEPROM memory preservation (1 = EEPROM not preserved) |1      |1      |
|BOOTSZ1 |2    |Select Boot Size (0 = max. boot size)                 |0      |0      |
|BOOTSZ0 |1    |Select Boot Size (0 = max. boot size)                 |0      |0      |
|BOOTRST |0    |Select reset vector                                   |1      |1      |

## Fuse Low Byte
|Bit Name|Bit #|Description                                           |Default|Setting|
|--------|-----|------------------------------------------------------|-------|-------|
|BODLEVEL|7    |Brown-out Detector Level                              |1      |1      |
|BODEN   |6    |Brown-out Detector (1 = disabled)                     |1      |1      |
|SUT1    |5    |Select start-up time                                  |1      |1      |
|SUT0    |4    |Select start-up time                                  |0      |0      |
|CKSEL3  |3    |Select Clock source (0 for 1 MHz clock)               |0      |0      |
|CKSEL2  |2    |Select Clock source	(0 for 1 MHz clock)               |0      |0      |
|CKSEL1  |1    |Select Clock source	(0 for 1 MHz clock)               |0      |0      |
|CKSEL0  |0    |Select Clock source	(1 for 1 MHz clock)               |1      |1      |