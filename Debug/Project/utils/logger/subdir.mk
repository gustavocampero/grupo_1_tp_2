################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Project/utils/logger/logger.c \
../Project/utils/logger/logger_sink_uart.c 

OBJS += \
./Project/utils/logger/logger.o \
./Project/utils/logger/logger_sink_uart.o 

C_DEPS += \
./Project/utils/logger/logger.d \
./Project/utils/logger/logger_sink_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Project/utils/logger/%.o Project/utils/logger/%.su Project/utils/logger/%.cyclo: ../Project/utils/logger/%.c Project/utils/logger/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/app/inc" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/services/uart" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/ports/uart" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/utils/logger" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Project-2f-utils-2f-logger

clean-Project-2f-utils-2f-logger:
	-$(RM) ./Project/utils/logger/logger.cyclo ./Project/utils/logger/logger.d ./Project/utils/logger/logger.o ./Project/utils/logger/logger.su ./Project/utils/logger/logger_sink_uart.cyclo ./Project/utils/logger/logger_sink_uart.d ./Project/utils/logger/logger_sink_uart.o ./Project/utils/logger/logger_sink_uart.su

.PHONY: clean-Project-2f-utils-2f-logger

