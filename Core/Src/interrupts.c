#include "stm32u5xx_hal.h"
#include "SPI_queue.h"
#include "BMI088_accel.h"
#include "main.h"
#include "SPI_device_interactions.h"

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BMI_ACC_INT1_Pin) {
        bmi088_accel_interrupt();
    }else if (GPIO_Pin == BMI_GYRO_INT1_Pin){
        bmi088_gyro_interrupt();
    }
    
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    (void)GPIO_Pin;
}
