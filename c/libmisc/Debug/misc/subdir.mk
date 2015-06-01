################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../misc/Base64.cpp \
../misc/Date.cpp \
../misc/Logger.cpp \
../misc/Misc.cpp \
../misc/Net.cpp \
../misc/tinyxml2.cpp 

OBJS += \
./misc/Base64.o \
./misc/Date.o \
./misc/Logger.o \
./misc/Misc.o \
./misc/Net.o \
./misc/tinyxml2.o 

CPP_DEPS += \
./misc/Base64.d \
./misc/Date.d \
./misc/Logger.d \
./misc/Misc.d \
./misc/Net.d \
./misc/tinyxml2.d 


# Each subdirectory must supply rules for building sources it contributes
misc/%.o: ../misc/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DLIBMISC -DLINUX -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


