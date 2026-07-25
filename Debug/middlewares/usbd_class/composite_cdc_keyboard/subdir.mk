################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middlewares/usbd_class/composite_cdc_keyboard/cdc_keyboard_class.c \
../middlewares/usbd_class/composite_cdc_keyboard/cdc_keyboard_desc.c 

OBJS += \
./middlewares/usbd_class/composite_cdc_keyboard/cdc_keyboard_class.o \
./middlewares/usbd_class/composite_cdc_keyboard/cdc_keyboard_desc.o 

C_DEPS += \
./middlewares/usbd_class/composite_cdc_keyboard/cdc_keyboard_class.d \
./middlewares/usbd_class/composite_cdc_keyboard/cdc_keyboard_desc.d 


# Each subdirectory must supply rules for building sources it contributes
middlewares/usbd_class/composite_cdc_keyboard/%.o: ../middlewares/usbd_class/composite_cdc_keyboard/%.c middlewares/usbd_class/composite_cdc_keyboard/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -lm -O0 -ffunction-sections  -g -DAT_START_F403A_V1 -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DAT32F403AVGT7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"./middlewares" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


