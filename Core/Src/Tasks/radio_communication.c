#include <stdbool.h>
#include "cmsis_os2.h"

void radio_communication_task_start(void *argument)
{
    while (true) {
        osDelay(1);
    }
}
