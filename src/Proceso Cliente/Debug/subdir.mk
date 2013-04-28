################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Proceso\ Cliente.c \
../main.c 

OBJS += \
./Proceso\ Cliente.o \
./main.o 

C_DEPS += \
./Proceso\ Cliente.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
Proceso\ Cliente.o: ../Proceso\ Cliente.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"Proceso Cliente.d" -MT"Proceso\ Cliente.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


