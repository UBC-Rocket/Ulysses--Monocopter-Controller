#include <stdbool.h>
#include "cmsis_os2.h"

void start_motor_control_task(void *argument)
{
    while (true) {
        osDelay(1);
    }
}