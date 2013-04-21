################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../so-exercises/Ejercicio_08/Resuelto/src/spock/Mascota.c \
../so-exercises/Ejercicio_08/Resuelto/src/spock/Mision.c \
../so-exercises/Ejercicio_08/Resuelto/src/spock/Spock.c \
../so-exercises/Ejercicio_08/Resuelto/src/spock/Stream.c \
../so-exercises/Ejercicio_08/Resuelto/src/spock/Villano.c 

OBJS += \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Mascota.o \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Mision.o \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Spock.o \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Stream.o \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Villano.o 

C_DEPS += \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Mascota.d \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Mision.d \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Spock.d \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Stream.d \
./so-exercises/Ejercicio_08/Resuelto/src/spock/Villano.d 


# Each subdirectory must supply rules for building sources it contributes
so-exercises/Ejercicio_08/Resuelto/src/spock/%.o: ../so-exercises/Ejercicio_08/Resuelto/src/spock/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


