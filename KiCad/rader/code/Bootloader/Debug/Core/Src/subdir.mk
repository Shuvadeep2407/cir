################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/blink_driver.c \
../Core/Src/bootloader.c \
../Core/Src/can_driver.c \
../Core/Src/gsm_driver.c \
../Core/Src/i2c_driver.c \
../Core/Src/i2s_audio_driver.c \
../Core/Src/led_driver.c \
../Core/Src/main.c \
../Core/Src/ota.c \
../Core/Src/spi_radio_driver.c \
../Core/Src/stepper_driver.c \
../Core/Src/stm32c0xx_hal_msp.c \
../Core/Src/stm32c0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32c0xx.c 

OBJS += \
./Core/Src/blink_driver.o \
./Core/Src/bootloader.o \
./Core/Src/can_driver.o \
./Core/Src/gsm_driver.o \
./Core/Src/i2c_driver.o \
./Core/Src/i2s_audio_driver.o \
./Core/Src/led_driver.o \
./Core/Src/main.o \
./Core/Src/ota.o \
./Core/Src/spi_radio_driver.o \
./Core/Src/stepper_driver.o \
./Core/Src/stm32c0xx_hal_msp.o \
./Core/Src/stm32c0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32c0xx.o 

C_DEPS += \
./Core/Src/blink_driver.d \
./Core/Src/bootloader.d \
./Core/Src/can_driver.d \
./Core/Src/gsm_driver.d \
./Core/Src/i2c_driver.d \
./Core/Src/i2s_audio_driver.d \
./Core/Src/led_driver.d \
./Core/Src/main.d \
./Core/Src/ota.d \
./Core/Src/spi_radio_driver.d \
./Core/Src/stepper_driver.d \
./Core/Src/stm32c0xx_hal_msp.d \
./Core/Src/stm32c0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32c0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../Core/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/blink_driver.cyclo ./Core/Src/blink_driver.d ./Core/Src/blink_driver.o ./Core/Src/blink_driver.su ./Core/Src/bootloader.cyclo ./Core/Src/bootloader.d ./Core/Src/bootloader.o ./Core/Src/bootloader.su ./Core/Src/can_driver.cyclo ./Core/Src/can_driver.d ./Core/Src/can_driver.o ./Core/Src/can_driver.su ./Core/Src/gsm_driver.cyclo ./Core/Src/gsm_driver.d ./Core/Src/gsm_driver.o ./Core/Src/gsm_driver.su ./Core/Src/i2c_driver.cyclo ./Core/Src/i2c_driver.d ./Core/Src/i2c_driver.o ./Core/Src/i2c_driver.su ./Core/Src/i2s_audio_driver.cyclo ./Core/Src/i2s_audio_driver.d ./Core/Src/i2s_audio_driver.o ./Core/Src/i2s_audio_driver.su ./Core/Src/led_driver.cyclo ./Core/Src/led_driver.d ./Core/Src/led_driver.o ./Core/Src/led_driver.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/ota.cyclo ./Core/Src/ota.d ./Core/Src/ota.o ./Core/Src/ota.su ./Core/Src/spi_radio_driver.cyclo ./Core/Src/spi_radio_driver.d ./Core/Src/spi_radio_driver.o ./Core/Src/spi_radio_driver.su ./Core/Src/stepper_driver.cyclo ./Core/Src/stepper_driver.d ./Core/Src/stepper_driver.o ./Core/Src/stepper_driver.su ./Core/Src/stm32c0xx_hal_msp.cyclo ./Core/Src/stm32c0xx_hal_msp.d ./Core/Src/stm32c0xx_hal_msp.o ./Core/Src/stm32c0xx_hal_msp.su ./Core/Src/stm32c0xx_it.cyclo ./Core/Src/stm32c0xx_it.d ./Core/Src/stm32c0xx_it.o ./Core/Src/stm32c0xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32c0xx.cyclo ./Core/Src/system_stm32c0xx.d ./Core/Src/system_stm32c0xx.o ./Core/Src/system_stm32c0xx.su

.PHONY: clean-Core-2f-Src

