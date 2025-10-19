#include <stdbool.h>
#include "cmsis_os2.h"

void sensor_fusion_task_start(void *argument)
{
    while (true) {
        osDelay(1);
    }
}
