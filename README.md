# Simple-IGC-Logger
 Simple android based IGC flight logger, see wiki for details.
 
 ##Build instructions
 1. clone the repository
 2. open the folder in Visual Studio Code with Platform IO extension installed
 3. build
 4. upload firmware via USB cable

```
Processing megaatmega2560 (platform: atmelavr; board: megaatmega2560; framework: arduino)
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/atmelavr/megaatmega2560.html
PLATFORM: Atmel AVR (3.4.0) > Arduino Mega or Mega 2560 ATmega2560 (Mega 2560)     
HARDWARE: ATMEGA2560 16MHz, 8KB RAM, 248KB Flash
DEBUG: Current (avr-stub) On-board (avr-stub, simavr)
PACKAGES:
 - framework-arduino-avr 5.1.0
 - toolchain-atmelavr 1.70300.191015 (7.3.0)
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 13 compatible libraries
Scanning dependencies...
Dependency Graph
|-- <Adafruit BMP280 Library> 2.5.0
|   |-- <Adafruit Unified Sensor> 1.1.4
|   |-- <Adafruit BusIO> 1.9.9
|   |   |-- <Wire> 1.0
|   |   |-- <SPI> 1.0
|   |-- <Wire> 1.0
|   |-- <SPI> 1.0
|-- <TinyGPSPlus> 1.0.2
|-- <SD> 1.2.4
|   |-- <SPI> 1.0
|-- <IniFile> 1.3.0
|   |-- <SD> 1.2.4
|   |   |-- <SPI> 1.0
|-- <MD5>
|-- <DateTime>
|-- <SPI> 1.0
|-- <Wire> 1.0
Building in release mode
Compiling .pio\build\megaatmega2560\src\config.cpp.o
Compiling .pio\build\megaatmega2560\src\igc_file_writer.cpp.o
Compiling .pio\build\megaatmega2560\src\index.cpp.o
Compiling .pio\build\megaatmega2560\src\logger.cpp.o
Compiling .pio\build\megaatmega2560\src\main.cpp.o
Compiling .pio\build\megaatmega2560\src\utils.cpp.o
Compiling .pio\build\megaatmega2560\lib3e3\Adafruit Unified Sensor\Adafruit_Sensor.cpp.o
Compiling .pio\build\megaatmega2560\libf2c\Wire\Wire.cpp.o
Compiling .pio\build\megaatmega2560\libf2c\Wire\utility\twi.c.o
Archiving .pio\build\megaatmega2560\lib3e3\libAdafruit Unified Sensor.a
Compiling .pio\build\megaatmega2560\libcec\SPI\SPI.cpp.o
Compiling .pio\build\megaatmega2560\lib7ed\Adafruit BusIO\Adafruit_BusIO_Register.cpp.o
Compiling .pio\build\megaatmega2560\lib7ed\Adafruit BusIO\Adafruit_I2CDevice.cpp.o
Compiling .pio\build\megaatmega2560\lib7ed\Adafruit BusIO\Adafruit_SPIDevice.cpp.o
Compiling .pio\build\megaatmega2560\lib854\Adafruit BMP280 Library\Adafruit_BMP280.cpp.o
Compiling .pio\build\megaatmega2560\libf37\TinyGPSPlus\TinyGPS++.cpp.o
Compiling .pio\build\megaatmega2560\lib468\SD\File.cpp.o
Archiving .pio\build\megaatmega2560\libcec\libSPI.a
Compiling .pio\build\megaatmega2560\lib468\SD\SD.cpp.o
Compiling .pio\build\megaatmega2560\lib468\SD\utility\Sd2Card.cpp.o
Archiving .pio\build\megaatmega2560\libf2c\libWire.a
Archiving .pio\build\megaatmega2560\lib7ed\libAdafruit BusIO.a
Archiving .pio\build\megaatmega2560\lib854\libAdafruit BMP280 Library.a
Archiving .pio\build\megaatmega2560\libf37\libTinyGPSPlus.a
Compiling .pio\build\megaatmega2560\lib468\SD\utility\SdFile.cpp.o
Compiling .pio\build\megaatmega2560\lib468\SD\utility\SdVolume.cpp.o
Compiling .pio\build\megaatmega2560\libb1b\IniFile\IniFile.cpp.o
Compiling .pio\build\megaatmega2560\lib7d4\MD5\MD5.cpp.o
Compiling .pio\build\megaatmega2560\lib107\DateTime\DateTime.cpp.o
Archiving .pio\build\megaatmega2560\libFrameworkArduinoVariant.a
Compiling .pio\build\megaatmega2560\FrameworkArduino\CDC.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\HardwareSerial.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\HardwareSerial0.cpp.o
Archiving .pio\build\megaatmega2560\lib107\libDateTime.a
Compiling .pio\build\megaatmega2560\FrameworkArduino\HardwareSerial1.cpp.o
Archiving .pio\build\megaatmega2560\lib468\libSD.a
Archiving .pio\build\megaatmega2560\lib7d4\libMD5.a
Archiving .pio\build\megaatmega2560\libb1b\libIniFile.a
Compiling .pio\build\megaatmega2560\FrameworkArduino\HardwareSerial2.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\HardwareSerial3.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\IPAddress.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\PluggableUSB.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\Print.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\Stream.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\Tone.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\USBCore.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\WInterrupts.c.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\WMath.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\WString.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\abi.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\hooks.c.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\main.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\new.cpp.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\wiring.c.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\wiring_analog.c.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\wiring_digital.c.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\wiring_pulse.S.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\wiring_pulse.c.o
Compiling .pio\build\megaatmega2560\FrameworkArduino\wiring_shift.c.o
Archiving .pio\build\megaatmega2560\libFrameworkArduino.a
Linking .pio\build\megaatmega2560\firmware.elf
Checking size .pio\build\megaatmega2560\firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [====      ]  42.6% (used 3490 bytes from 8192 bytes)
Flash: [==        ]  20.8% (used 52924 bytes from 253952 bytes)
Building .pio\build\megaatmega2560\firmware.hex
=========================== [SUCCESS] Took 7.69 seconds ================================
```

