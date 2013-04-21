################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Proceso\ Cliente.c 

OBJS += \
./Proceso\ Cliente.o 

C_DEPS += \
./Proceso\ Cliente.d 


# Each subdirectory must supply rules for building sources it contributes
Proceso\ Cliente.o: ../Proceso\ Cliente.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"Proceso Cliente.d" -MT"Proceso\ Cliente.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


