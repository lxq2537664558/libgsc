################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../com/Actor.cpp \
../com/ActorAuto.cpp \
../com/ActorIdpd.cpp \
../com/ActorMgr.cpp \
../com/ActorTrans.cpp \
../com/Gsc.cpp \
../com/GscEvent.cpp \
../com/GscMisc.cpp \
../com/Lx.cpp 

OBJS += \
./com/Actor.o \
./com/ActorAuto.o \
./com/ActorIdpd.o \
./com/ActorMgr.o \
./com/ActorTrans.o \
./com/Gsc.o \
./com/GscEvent.o \
./com/GscMisc.o \
./com/Lx.o 

CPP_DEPS += \
./com/Actor.d \
./com/ActorAuto.d \
./com/ActorIdpd.d \
./com/ActorMgr.d \
./com/ActorTrans.d \
./com/Gsc.d \
./com/GscEvent.d \
./com/GscMisc.d \
./com/Lx.d 


# Each subdirectory must supply rules for building sources it contributes
com/%.o: ../com/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DLINUX -DLIBGSC -DGSC_CCC=1000 -DGSC_WORKER=16 -DGSC_MTU=2048 -DGSC_SENDBUF=1 -I"/root/c/mirx/libgmisc" -I/usr/local/extra/protobuf-2.5.0/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


