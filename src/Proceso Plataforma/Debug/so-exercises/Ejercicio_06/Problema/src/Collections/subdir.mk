################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../so-exercises/Ejercicio_06/Problema/src/Collections/list.c \
../so-exercises/Ejercicio_06/Problema/src/Collections/queue.c 

OBJS += \
./so-exercises/Ejercicio_06/Problema/src/Collections/list.o \
./so-exercises/Ejercicio_06/Problema/src/Collections/queue.o 

C_DEPS += \
./so-exercises/Ejercicio_06/Problema/src/Collections/list.d \
./so-exercises/Ejercicio_06/Problema/src/Collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
so-exercises/Ejercicio_06/Problema/src/Collections/%.o: ../so-exercises/Ejercicio_06/Problema/src/Collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


