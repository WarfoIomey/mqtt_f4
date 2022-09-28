################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middleware/MQTT/Src/esp8266_mqtt.c 

OBJS += \
./Middleware/MQTT/Src/esp8266_mqtt.o 

C_DEPS += \
./Middleware/MQTT/Src/esp8266_mqtt.d 


# Each subdirectory must supply rules for building sources it contributes
Middleware/MQTT/Src/%.o Middleware/MQTT/Src/%.su: ../Middleware/MQTT/Src/%.c Middleware/MQTT/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Project/STM32F4DISCOVERY/mqtt/mqtt/Middleware/MQTT/Inc" -I"C:/Project/STM32F4DISCOVERY/mqtt/mqtt/Middleware/ESP8266/Src" -I"C:/Project/STM32F4DISCOVERY/mqtt/mqtt/Middleware/MQTT/Src" -I"C:/Project/STM32F4DISCOVERY/mqtt/mqtt/Middleware/ESP8266/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middleware-2f-MQTT-2f-Src

clean-Middleware-2f-MQTT-2f-Src:
	-$(RM) ./Middleware/MQTT/Src/esp8266_mqtt.d ./Middleware/MQTT/Src/esp8266_mqtt.o ./Middleware/MQTT/Src/esp8266_mqtt.su

.PHONY: clean-Middleware-2f-MQTT-2f-Src

