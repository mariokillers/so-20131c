################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Connections/Client.c \
../Connections/Mensajes.c \
../Connections/Server.c 

OBJS += \
./Connections/Client.o \
./Connections/Mensajes.o \
./Connections/Server.o 

C_DEPS += \
./Connections/Client.d \
./Connections/Mensajes.d \
./Connections/Server.d 


# Each subdirectory must supply rules for building sources it contributes
Connections/%.o: ../Connections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


