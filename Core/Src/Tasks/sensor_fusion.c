#include <stdbool.h>
#include "cmsis_os2.h"

void start_sensor_fusion_task(void *argument)
{
    while (true) {
        osDelay(1);
    }
}
