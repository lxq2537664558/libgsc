################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../actor/Actor.cpp \
../actor/ActorBlocking.cpp \
../actor/ActorGusrDb.cpp \
../actor/ActorNet.cpp \
../actor/Gusr.cpp \
../actor/H2N.cpp \
../actor/HttpH2n.cpp \
../actor/N2H.cpp 

OBJS += \
./actor/Actor.o \
./actor/ActorBlocking.o \
./actor/ActorGusrDb.o \
./actor/ActorNet.o \
./actor/Gusr.o \
./actor/H2N.o \
./actor/HttpH2n.o \
./actor/N2H.o 

CPP_DEPS += \
./actor/Actor.d \
./actor/ActorBlocking.d \
./actor/ActorGusrDb.d \
./actor/ActorNet.d \
./actor/Gusr.d \
./actor/H2N.d \
./actor/HttpH2n.d \
./actor/N2H.d 


# Each subdirectory must supply rules for building sources it contributes
actor/%.o: ../actor/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -D__cplusplus=201103L -DLINUX -DLIBGSC -DGSC_PDU=8192 -I/root/c/gs/libmisc -I/usr/local/extra/protobuf-2.6.1/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


