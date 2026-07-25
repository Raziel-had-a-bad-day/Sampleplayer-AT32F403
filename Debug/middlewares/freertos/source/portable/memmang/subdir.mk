################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middlewares/freertos/source/portable/memmang/heap_1.c \
../middlewares/freertos/source/portable/memmang/heap_2.c \
../middlewares/freertos/source/portable/memmang/heap_3.c \
../middlewares/freertos/source/portable/memmang/heap_4.c \
../middlewares/freertos/source/portable/memmang/heap_5.c 

OBJS += \
./middlewares/freertos/source/portable/memmang/heap_1.o \
./middlewares/freertos/source/portable/memmang/heap_2.o \
./middlewares/freertos/source/portable/memmang/heap_3.o \
./middlewares/freertos/source/portable/memmang/heap_4.o \
./middlewares/freertos/source/portable/memmang/heap_5.o 

C_DEPS += \
./middlewares/freertos/source/portable/memmang/heap_1.d \
./middlewares/freertos/source/portable/memmang/heap_2.d \
./middlewares/freertos/source/portable/memmang/heap_3.d \
./middlewares/freertos/source/portable/memmang/heap_4.d \
./middlewares/freertos/source/portable/memmang/heap_5.d 


# Each subdirectory must supply rules for building sources it contributes
middlewares/freertos/source/portable/memmang/%.o: ../middlewares/freertos/source/portable/memmang/%.c middlewares/freertos/source/portable/memmang/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -lm -O0 -ffunction-sections  -g -DAT_START_F403A_V1 -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DAT32F403AVGT7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"./middlewares" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


