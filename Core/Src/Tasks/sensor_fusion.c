#include <stdbool.h>
#include "cmsis_os2.h"
#include "spi_drivers/SPI_queue.h"
#include "spi_drivers/SPI_device_interactions.h"
#include "stm32u5xx_hal.h"
#include "main.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"

#define FUSION_VECTOR_SAMPLE_SIZE 32

// External variable declarations
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart3;

extern bmi088_accel_t accel;
extern bmi088_gyro_t gyro;

void sensor_fusion_task_start(void *argument)
{

    uint32_t primask = __get_PRIMASK();  // save current state
    __disable_irq();

    uint8_t accel_status = bmi088_accel_init(&hspi1, BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, &accel);
    
    if(accel_status == 0){
        bmi088_acc_sample_ring.head = 0;
        bmi088_acc_sample_ring.tail = 0;
    }else {
        ;
    }

    uint8_t gyro_status = bmi088_gyro_init(&hspi1, BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, &gyro);

    if(gyro_status == 0){
        bmi088_gyro_sample_ring.head = 0;
        bmi088_gyro_sample_ring.tail = 0;
    }else {
        ;
    }


    // ms5611_t baro;
    // if(!initialize_baro(&baro, &spi_handle) && 
    //    ring_buffer_init(&accel_baro_buffer, sizeof(ms5611_baro_sample_t), 32)){
    //   HAL_NVIC_SystemReset();
    // }


    __set_PRIMASK(primask);

    static bmi088_accel_sample_t accel_samples[16];

    for (;;) {

        int8_t num_accel_samples = 0;
        bmi088_accel_sample_t sample;
        while(num_accel_samples < FUSION_VECTOR_SAMPLE_SIZE){
            if(!bmi088_acc_sample_dequeue(&bmi088_acc_sample_ring, &sample)) break;
            
            accel_samples[num_accel_samples] = sample;

            num_accel_samples++;

        }
        delay_us(10000);
    }
}
