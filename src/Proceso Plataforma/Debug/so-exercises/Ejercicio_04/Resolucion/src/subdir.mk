################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../so-exercises/Ejercicio_04/Resolucion/src/Cliente.c \
../so-exercises/Ejercicio_04/Resolucion/src/Factura.c \
../so-exercises/Ejercicio_04/Resolucion/src/Item.c \
../so-exercises/Ejercicio_04/Resolucion/src/Leaks.c 

OBJS += \
./so-exercises/Ejercicio_04/Resolucion/src/Cliente.o \
./so-exercises/Ejercicio_04/Resolucion/src/Factura.o \
./so-exercises/Ejercicio_04/Resolucion/src/Item.o \
./so-exercises/Ejercicio_04/Resolucion/src/Leaks.o 

C_DEPS += \
./so-exercises/Ejercicio_04/Resolucion/src/Cliente.d \
./so-exercises/Ejercicio_04/Resolucion/src/Factura.d \
./so-exercises/Ejercicio_04/Resolucion/src/Item.d \
./so-exercises/Ejercicio_04/Resolucion/src/Leaks.d 


# Each subdirectory must supply rules for building sources it contributes
so-exercises/Ejercicio_04/Resolucion/src/%.o: ../so-exercises/Ejercicio_04/Resolucion/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


