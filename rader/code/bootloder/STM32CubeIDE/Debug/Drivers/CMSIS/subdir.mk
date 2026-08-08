################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/system_stm32c0xx.c 

OBJS += \
./Drivers/CMSIS/system_stm32c0xx.o 

C_DEPS += \
./Drivers/CMSIS/system_stm32c0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/system_stm32c0xx.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/system_stm32c0xx.c Drivers/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Drivers/CMSIS/system_stm32c0xx.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS

clean-Drivers-2f-CMSIS:
	-$(RM) ./Drivers/CMSIS/system_stm32c0xx.cyclo ./Drivers/CMSIS/system_stm32c0xx.d ./Drivers/CMSIS/system_stm32c0xx.o ./Drivers/CMSIS/system_stm32c0xx.su

.PHONY: clean-Drivers-2f-CMSIS

