################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Project/services/uart/ao_uart.c 

OBJS += \
./Project/services/uart/ao_uart.o 

C_DEPS += \
./Project/services/uart/ao_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Project/services/uart/%.o Project/services/uart/%.su Project/services/uart/%.cyclo: ../Project/services/uart/%.c Project/services/uart/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/app/inc" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/services/uart" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/ports/uart" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/utils/logger" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Project-2f-services-2f-uart

clean-Project-2f-services-2f-uart:
	-$(RM) ./Project/services/uart/ao_uart.cyclo ./Project/services/uart/ao_uart.d ./Project/services/uart/ao_uart.o ./Project/services/uart/ao_uart.su

.PHONY: clean-Project-2f-services-2f-uart

