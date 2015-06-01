################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../net/Gworker.cpp \
../net/Lx.cpp \
../net/WebSocket.cpp 

OBJS += \
./net/Gworker.o \
./net/Lx.o \
./net/WebSocket.o 

CPP_DEPS += \
./net/Gworker.d \
./net/Lx.d \
./net/WebSocket.d 


# Each subdirectory must supply rules for building sources it contributes
net/%.o: ../net/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -D__cplusplus=201103L -DLINUX -DLIBGSC -DGSC_PDU=8192 -I/root/c/gs/libmisc -I/usr/local/extra/protobuf-2.6.1/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


