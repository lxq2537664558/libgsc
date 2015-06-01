################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../core/Cb.cpp \
../core/Cfg.cpp \
../core/EvnCb.cpp \
../core/Gh2ntrans.cpp \
../core/Gn2h.cpp \
../core/Gn2htrans.cpp \
../core/Gsc.cpp \
../core/Gstat.cpp 

OBJS += \
./core/Cb.o \
./core/Cfg.o \
./core/EvnCb.o \
./core/Gh2ntrans.o \
./core/Gn2h.o \
./core/Gn2htrans.o \
./core/Gsc.o \
./core/Gstat.o 

CPP_DEPS += \
./core/Cb.d \
./core/Cfg.d \
./core/EvnCb.d \
./core/Gh2ntrans.d \
./core/Gn2h.d \
./core/Gn2htrans.d \
./core/Gsc.d \
./core/Gstat.d 


# Each subdirectory must supply rules for building sources it contributes
core/%.o: ../core/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -D__cplusplus=201103L -DLINUX -DLIBGSC -DGSC_PDU=8192 -I/root/c/gs/libmisc -I/usr/local/extra/protobuf-2.6.1/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


