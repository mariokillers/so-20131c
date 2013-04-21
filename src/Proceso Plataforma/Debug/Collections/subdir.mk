################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Collections/list.c \
../Collections/queue.c 

OBJS += \
./Collections/list.o \
./Collections/queue.o 

C_DEPS += \
./Collections/list.d \
./Collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
Collections/%.o: ../Collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


