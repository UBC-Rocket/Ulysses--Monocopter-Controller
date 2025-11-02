#include <stdbool.h>
#include "cmsis_os2.h"
#include "main.h"

void motor_control_task_start(void *argument)
{
    while (true) {

        HAL_GPIO_TogglePin(STAT_LED_2_GPIO_Port, STAT_LED_2_Pin);

        osDelay(1);
    }
}