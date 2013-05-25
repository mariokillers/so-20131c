################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Proceso\ Nivel.c \
../nivel_library.c 

OBJS += \
./Proceso\ Nivel.o \
./nivel_library.o 

C_DEPS += \
./Proceso\ Nivel.d \
./nivel_library.d 


# Each subdirectory must supply rules for building sources it contributes
Proceso\ Nivel.o: ../Proceso\ Nivel.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib/DibujoNivelLib" -I/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"Proceso Nivel.d" -MT"Proceso\ Nivel.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib/DibujoNivelLib" -I/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


