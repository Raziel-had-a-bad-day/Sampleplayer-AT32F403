################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middlewares/3rd_party/fatfs/source/diskio.c \
../middlewares/3rd_party/fatfs/source/ff.c \
../middlewares/3rd_party/fatfs/source/ffsystem.c \
../middlewares/3rd_party/fatfs/source/ffunicode.c 

OBJS += \
./middlewares/3rd_party/fatfs/source/diskio.o \
./middlewares/3rd_party/fatfs/source/ff.o \
./middlewares/3rd_party/fatfs/source/ffsystem.o \
./middlewares/3rd_party/fatfs/source/ffunicode.o 

C_DEPS += \
./middlewares/3rd_party/fatfs/source/diskio.d \
./middlewares/3rd_party/fatfs/source/ff.d \
./middlewares/3rd_party/fatfs/source/ffsystem.d \
./middlewares/3rd_party/fatfs/source/ffunicode.d 


# Each subdirectory must supply rules for building sources it contributes
middlewares/3rd_party/fatfs/source/%.o: ../middlewares/3rd_party/fatfs/source/%.c middlewares/3rd_party/fatfs/source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -lm -O0 -ffunction-sections  -g -DAT_START_F403A_V1 -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DAT32F403AVGT7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"./middlewares" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


