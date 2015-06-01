################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../pb/Gproto.cpp 

OBJS += \
./pb/Gproto.o 

CPP_DEPS += \
./pb/Gproto.d 


# Each subdirectory must supply rules for building sources it contributes
pb/%.o: ../pb/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DLIBGMISC -DGSC_REQ_PDU_MTU=1024 -DLINUX -I/usr/local/extra/protobuf-2.5.0/include -O3 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


