################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../so-exercises/Ejercicio_04/Problema/src/Cliente.c \
../so-exercises/Ejercicio_04/Problema/src/Factura.c \
../so-exercises/Ejercicio_04/Problema/src/Item.c \
../so-exercises/Ejercicio_04/Problema/src/Leaks.c 

OBJS += \
./so-exercises/Ejercicio_04/Problema/src/Cliente.o \
./so-exercises/Ejercicio_04/Problema/src/Factura.o \
./so-exercises/Ejercicio_04/Problema/src/Item.o \
./so-exercises/Ejercicio_04/Problema/src/Leaks.o 

C_DEPS += \
./so-exercises/Ejercicio_04/Problema/src/Cliente.d \
./so-exercises/Ejercicio_04/Problema/src/Factura.d \
./so-exercises/Ejercicio_04/Problema/src/Item.d \
./so-exercises/Ejercicio_04/Problema/src/Leaks.d 


# Each subdirectory must supply rules for building sources it contributes
so-exercises/Ejercicio_04/Problema/src/%.o: ../so-exercises/Ejercicio_04/Problema/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


