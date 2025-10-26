#include <stdbool.h>
#include "cmsis_os2.h"

void motor_control_task_start(void *argument)
{
    while (true) {
        osDelay(1);
    }
}