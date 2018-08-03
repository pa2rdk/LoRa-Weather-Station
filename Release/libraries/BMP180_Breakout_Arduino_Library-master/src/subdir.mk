################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/Users/robertdekok/Library/Mobile\ Documents/com~apple~CloudDocs/ARDUINO/libraries/BMP180_Breakout_Arduino_Library-master/src/SFE_BMP180.cpp 

LINK_OBJ += \
./libraries/BMP180_Breakout_Arduino_Library-master/src/SFE_BMP180.cpp.o 

CPP_DEPS += \
./libraries/BMP180_Breakout_Arduino_Library-master/src/SFE_BMP180.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
libraries/BMP180_Breakout_Arduino_Library-master/src/SFE_BMP180.cpp.o: /Users/robertdekok/Library/Mobile\ Documents/com~apple~CloudDocs/ARDUINO/libraries/BMP180_Breakout_Arduino_Library-master/src/SFE_BMP180.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"/Applications/Eclipse/arduino-oxygen/Eclipse.app/Contents/MacOS//arduinoPlugin/packages/arduino/tools/avr-gcc/4.9.2-atmel3.5.4-arduino2/bin/avr-g++" -c -g -Os -Wall -Wextra -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -flto -mmcu=atmega32u4 -DF_CPU=8000000L -DARDUINO=10802 -DARDUINO_AVR_FEATHER32U4 -DARDUINO_ARCH_AVR -DUSB_VID=0x239A -DUSB_PID=0x800C '-DUSB_MANUFACTURER="Adafruit"' '-DUSB_PRODUCT="Feather 32u4"'  -I"/Applications/Eclipse/arduino-oxygen/Eclipse.app/Contents/MacOS/arduinoPlugin/packages/arduino/hardware/avr/1.6.20/cores/arduino" -I"/Users/robertdekok/Documents/Arduino/hardware/avr/variants/lora32u4II" -I"/Applications/Eclipse/arduino-oxygen/Eclipse.app/Contents/MacOS/arduinoPlugin/packages/arduino/hardware/avr/1.6.20/libraries/SPI" -I"/Applications/Eclipse/arduino-oxygen/Eclipse.app/Contents/MacOS/arduinoPlugin/packages/arduino/hardware/avr/1.6.20/libraries/SPI/src" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/DHT-sensor-library-master" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/Adafruit_Sensor-master" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/Low-Power-master" -I"/Users/robertdekok/Documents/Arduino/hardware/avr/libraries/Wire" -I"/Users/robertdekok/Documents/Arduino/hardware/avr/libraries/Wire/utility" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/arduino-lmic-master" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/arduino-lmic-master/src" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/BMP180_Breakout_Arduino_Library-master" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/BMP180_Breakout_Arduino_Library-master/src" -I"/Users/robertdekok/Library/Mobile Documents/com~apple~CloudDocs/ARDUINO/libraries/Grove_Barometer_Sensor-master" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '


