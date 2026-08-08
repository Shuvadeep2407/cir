################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/cc1101.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/client_node.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/fdcan.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/gpio.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/lib/SENSORS/gps_compass.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/i2c.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/i2s.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/lib/AUDIO/i2s_mic.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/main.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/spi.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/stm32c0xx_hal_msp.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/stm32c0xx_it.c \
../Application/User/Core/syscalls.c \
../Application/User/Core/sysmem.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/tim.c \
E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/usart.c 

OBJS += \
./Application/User/Core/cc1101.o \
./Application/User/Core/client_node.o \
./Application/User/Core/fdcan.o \
./Application/User/Core/gpio.o \
./Application/User/Core/gps_compass.o \
./Application/User/Core/i2c.o \
./Application/User/Core/i2s.o \
./Application/User/Core/i2s_mic.o \
./Application/User/Core/main.o \
./Application/User/Core/spi.o \
./Application/User/Core/stm32c0xx_hal_msp.o \
./Application/User/Core/stm32c0xx_it.o \
./Application/User/Core/syscalls.o \
./Application/User/Core/sysmem.o \
./Application/User/Core/tim.o \
./Application/User/Core/usart.o 

C_DEPS += \
./Application/User/Core/cc1101.d \
./Application/User/Core/client_node.d \
./Application/User/Core/fdcan.d \
./Application/User/Core/gpio.d \
./Application/User/Core/gps_compass.d \
./Application/User/Core/i2c.d \
./Application/User/Core/i2s.d \
./Application/User/Core/i2s_mic.d \
./Application/User/Core/main.d \
./Application/User/Core/spi.d \
./Application/User/Core/stm32c0xx_hal_msp.d \
./Application/User/Core/stm32c0xx_it.d \
./Application/User/Core/syscalls.d \
./Application/User/Core/sysmem.d \
./Application/User/Core/tim.d \
./Application/User/Core/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Core/cc1101.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/cc1101.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/cc1101.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/client_node.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/client_node.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/client_node.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/fdcan.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/fdcan.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/fdcan.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/gpio.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/gpio.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/gpio.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/gps_compass.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/lib/SENSORS/gps_compass.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/gps_compass.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/i2c.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/i2c.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/i2c.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/i2s.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/i2s.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/i2s.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/i2s_mic.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/lib/AUDIO/i2s_mic.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/i2s_mic.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/main.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/main.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/main.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/spi.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/spi.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/spi.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/stm32c0xx_hal_msp.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/stm32c0xx_hal_msp.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/stm32c0xx_hal_msp.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/stm32c0xx_it.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/stm32c0xx_it.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/stm32c0xx_it.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/%.o Application/User/Core/%.su Application/User/Core/%.cyclo: ../Application/User/Core/%.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/tim.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/tim.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/tim.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Application/User/Core/usart.o: E:/drone/c_uas/drone\ control/rader/code/bootloder/Core/Src/usart.c Application/User/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DHAS_AUDIO_NODE -DUSE_HAL_DRIVER -DSTM32C092xx -c -I../../Core/Inc -I../../lib/AUDIO -I../../lib/SENSORS -I../../Drivers/STM32C0xx_HAL_Driver/Inc -I../../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/User/Core/usart.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-User-2f-Core

clean-Application-2f-User-2f-Core:
	-$(RM) ./Application/User/Core/cc1101.cyclo ./Application/User/Core/cc1101.d ./Application/User/Core/cc1101.o ./Application/User/Core/cc1101.su ./Application/User/Core/client_node.cyclo ./Application/User/Core/client_node.d ./Application/User/Core/client_node.o ./Application/User/Core/client_node.su ./Application/User/Core/fdcan.cyclo ./Application/User/Core/fdcan.d ./Application/User/Core/fdcan.o ./Application/User/Core/fdcan.su ./Application/User/Core/gpio.cyclo ./Application/User/Core/gpio.d ./Application/User/Core/gpio.o ./Application/User/Core/gpio.su ./Application/User/Core/gps_compass.cyclo ./Application/User/Core/gps_compass.d ./Application/User/Core/gps_compass.o ./Application/User/Core/gps_compass.su ./Application/User/Core/i2c.cyclo ./Application/User/Core/i2c.d ./Application/User/Core/i2c.o ./Application/User/Core/i2c.su ./Application/User/Core/i2s.cyclo ./Application/User/Core/i2s.d ./Application/User/Core/i2s.o ./Application/User/Core/i2s.su ./Application/User/Core/i2s_mic.cyclo ./Application/User/Core/i2s_mic.d ./Application/User/Core/i2s_mic.o ./Application/User/Core/i2s_mic.su ./Application/User/Core/main.cyclo ./Application/User/Core/main.d ./Application/User/Core/main.o ./Application/User/Core/main.su ./Application/User/Core/spi.cyclo ./Application/User/Core/spi.d ./Application/User/Core/spi.o ./Application/User/Core/spi.su ./Application/User/Core/stm32c0xx_hal_msp.cyclo ./Application/User/Core/stm32c0xx_hal_msp.d ./Application/User/Core/stm32c0xx_hal_msp.o ./Application/User/Core/stm32c0xx_hal_msp.su ./Application/User/Core/stm32c0xx_it.cyclo ./Application/User/Core/stm32c0xx_it.d ./Application/User/Core/stm32c0xx_it.o ./Application/User/Core/stm32c0xx_it.su ./Application/User/Core/syscalls.cyclo ./Application/User/Core/syscalls.d ./Application/User/Core/syscalls.o ./Application/User/Core/syscalls.su ./Application/User/Core/sysmem.cyclo ./Application/User/Core/sysmem.d ./Application/User/Core/sysmem.o ./Application/User/Core/sysmem.su ./Application/User/Core/tim.cyclo ./Application/User/Core/tim.d ./Application/User/Core/tim.o ./Application/User/Core/tim.su ./Application/User/Core/usart.cyclo ./Application/User/Core/usart.d ./Application/User/Core/usart.o ./Application/User/Core/usart.su

.PHONY: clean-Application-2f-User-2f-Core

