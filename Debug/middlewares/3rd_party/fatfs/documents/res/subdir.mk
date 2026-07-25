################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middlewares/3rd_party/fatfs/documents/res/app1.c \
../middlewares/3rd_party/fatfs/documents/res/app2.c \
../middlewares/3rd_party/fatfs/documents/res/app3.c \
../middlewares/3rd_party/fatfs/documents/res/app4.c \
../middlewares/3rd_party/fatfs/documents/res/app5.c \
../middlewares/3rd_party/fatfs/documents/res/app6.c 

OBJS += \
./middlewares/3rd_party/fatfs/documents/res/app1.o \
./middlewares/3rd_party/fatfs/documents/res/app2.o \
./middlewares/3rd_party/fatfs/documents/res/app3.o \
./middlewares/3rd_party/fatfs/documents/res/app4.o \
./middlewares/3rd_party/fatfs/documents/res/app5.o \
./middlewares/3rd_party/fatfs/documents/res/app6.o 

C_DEPS += \
./middlewares/3rd_party/fatfs/documents/res/app1.d \
./middlewares/3rd_party/fatfs/documents/res/app2.d \
./middlewares/3rd_party/fatfs/documents/res/app3.d \
./middlewares/3rd_party/fatfs/documents/res/app4.d \
./middlewares/3rd_party/fatfs/documents/res/app5.d \
./middlewares/3rd_party/fatfs/documents/res/app6.d 


# Each subdirectory must supply rules for building sources it contributes
middlewares/3rd_party/fatfs/documents/res/%.o: ../middlewares/3rd_party/fatfs/documents/res/%.c middlewares/3rd_party/fatfs/documents/res/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -lm -O0 -ffunction-sections  -g -DAT_START_F403A_V1 -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DAT32F403AVGT7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"./middlewares" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


