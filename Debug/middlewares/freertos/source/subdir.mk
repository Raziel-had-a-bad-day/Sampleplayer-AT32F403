################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middlewares/freertos/source/croutine.c \
../middlewares/freertos/source/event_groups.c \
../middlewares/freertos/source/list.c \
../middlewares/freertos/source/queue.c \
../middlewares/freertos/source/stream_buffer.c \
../middlewares/freertos/source/tasks.c \
../middlewares/freertos/source/timers.c 

OBJS += \
./middlewares/freertos/source/croutine.o \
./middlewares/freertos/source/event_groups.o \
./middlewares/freertos/source/list.o \
./middlewares/freertos/source/queue.o \
./middlewares/freertos/source/stream_buffer.o \
./middlewares/freertos/source/tasks.o \
./middlewares/freertos/source/timers.o 

C_DEPS += \
./middlewares/freertos/source/croutine.d \
./middlewares/freertos/source/event_groups.d \
./middlewares/freertos/source/list.d \
./middlewares/freertos/source/queue.d \
./middlewares/freertos/source/stream_buffer.d \
./middlewares/freertos/source/tasks.d \
./middlewares/freertos/source/timers.d 


# Each subdirectory must supply rules for building sources it contributes
middlewares/freertos/source/%.o: ../middlewares/freertos/source/%.c middlewares/freertos/source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -lm -O0 -ffunction-sections  -g -DAT_START_F403A_V1 -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DAT32F403AVGT7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"./middlewares" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


