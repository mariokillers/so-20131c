################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Proceso\ Server.c \
../main.c 

OBJS += \
./Proceso\ Server.o \
./main.o 

C_DEPS += \
./Proceso\ Server.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
Proceso\ Server.o: ../Proceso\ Server.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib/commons" -O0 -g3 -c -fmessage-length=0 -MMD -MP -MF"Proceso Server.d" -MT"Proceso\ Server.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Escritorio/TP/tp-20131c-mario-killers/lib/commons" -O0 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


