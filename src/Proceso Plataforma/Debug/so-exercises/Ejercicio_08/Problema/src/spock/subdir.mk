################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../so-exercises/Ejercicio_08/Problema/src/spock/Mascota.c \
../so-exercises/Ejercicio_08/Problema/src/spock/Mision.c \
../so-exercises/Ejercicio_08/Problema/src/spock/Spock.c \
../so-exercises/Ejercicio_08/Problema/src/spock/Villano.c 

OBJS += \
./so-exercises/Ejercicio_08/Problema/src/spock/Mascota.o \
./so-exercises/Ejercicio_08/Problema/src/spock/Mision.o \
./so-exercises/Ejercicio_08/Problema/src/spock/Spock.o \
./so-exercises/Ejercicio_08/Problema/src/spock/Villano.o 

C_DEPS += \
./so-exercises/Ejercicio_08/Problema/src/spock/Mascota.d \
./so-exercises/Ejercicio_08/Problema/src/spock/Mision.d \
./so-exercises/Ejercicio_08/Problema/src/spock/Spock.d \
./so-exercises/Ejercicio_08/Problema/src/spock/Villano.d 


# Each subdirectory must supply rules for building sources it contributes
so-exercises/Ejercicio_08/Problema/src/spock/%.o: ../so-exercises/Ejercicio_08/Problema/src/spock/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


