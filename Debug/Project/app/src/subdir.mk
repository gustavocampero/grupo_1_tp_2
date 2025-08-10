################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Project/app/src/app.c \
../Project/app/src/linked_list.c \
../Project/app/src/memory_pool.c \
../Project/app/src/task_button.c \
../Project/app/src/task_led.c \
../Project/app/src/task_ui.c 

OBJS += \
./Project/app/src/app.o \
./Project/app/src/linked_list.o \
./Project/app/src/memory_pool.o \
./Project/app/src/task_button.o \
./Project/app/src/task_led.o \
./Project/app/src/task_ui.o 

C_DEPS += \
./Project/app/src/app.d \
./Project/app/src/linked_list.d \
./Project/app/src/memory_pool.d \
./Project/app/src/task_button.d \
./Project/app/src/task_led.d \
./Project/app/src/task_ui.d 


# Each subdirectory must supply rules for building sources it contributes
Project/app/src/%.o Project/app/src/%.su Project/app/src/%.cyclo: ../Project/app/src/%.c Project/app/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/app/inc" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/services/uart" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/ports/uart" -I"C:/Users/nacho/Desktop/Postgrado/RTOS II/grupo_1_tp_2/Project/utils/logger" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Project-2f-app-2f-src

clean-Project-2f-app-2f-src:
	-$(RM) ./Project/app/src/app.cyclo ./Project/app/src/app.d ./Project/app/src/app.o ./Project/app/src/app.su ./Project/app/src/linked_list.cyclo ./Project/app/src/linked_list.d ./Project/app/src/linked_list.o ./Project/app/src/linked_list.su ./Project/app/src/memory_pool.cyclo ./Project/app/src/memory_pool.d ./Project/app/src/memory_pool.o ./Project/app/src/memory_pool.su ./Project/app/src/task_button.cyclo ./Project/app/src/task_button.d ./Project/app/src/task_button.o ./Project/app/src/task_button.su ./Project/app/src/task_led.cyclo ./Project/app/src/task_led.d ./Project/app/src/task_led.o ./Project/app/src/task_led.su ./Project/app/src/task_ui.cyclo ./Project/app/src/task_ui.d ./Project/app/src/task_ui.o ./Project/app/src/task_ui.su

.PHONY: clean-Project-2f-app-2f-src

