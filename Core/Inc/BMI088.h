/*
 * BMI088.h
 *
 *  Created on: Jul 20, 2025
 *      Author: ak915
 */

#ifndef INC_BMI088_H_
#define INC_BMI088_H_

#include <stdlib.h>
#include <stdint.h>
#include "stm32u5xx_hal.h"
#include "main.h"

#define USING_BMI088
#define USING_SPI
#define GYRO_DOF 3
#define ACC_DOF 3

// Config Flag Table

#define READ_MASK_SPI 0x80
#define WRITE_MASK_SPI 0x00

#define CHIP_ID_ADDR 0x00
#define GYRO_CHIP_ID 0x0f

#define ACC_CHIP_ID 0x1E
#define ACC_PWR_CTL_ADDR 0x7D
#define ACC_RANGE_CTL_ADDR 0x41
#define ACC_FIFO_CONFIG0_ADDR 0x48
#define ACC_FIFO_ENABLE	0x03
#define ACC_INTERRUPT_CONFIG_ADDR 0x58
#define ACC_STATUS_ADDR 0x03
#define ACC_STATUS_DATA_READY_MASK 7
#define ACC_Z_MSB_ADDR 0x17
#define ACC_Z_LSB_ADDR 0x16
#define ACC_Y_MSB_ADDR 0x15
#define ACC_Y_LSB_ADDR 0x14
#define ACC_X_MSB_ADDR 0x13
#define ACC_X_LSB_ADDR 0x12

//Accelerometer buffer size define
#define ACC_BUFFER_SIZE 10
#define GYRO_BUFFER_SIZE 10

#define GYRO_RANGE_CTL_ADDR 0x0F
#define GYRO_FIFO_ENABLE_ADDR 0x3E
#define GYRO_ENABLE_FIFO 0x40
#define GYRO_RATE_Z_MSB_ADDR 0x07
#define GYRO_RATE_Z_LSB_ADDR 0x06
#define GYRO_RATE_Y_MSB_ADDR 0x05
#define GYRO_RATE_Y_LSB_ADDR 0x04
#define GYRO_RATE_X_MSB_ADDR 0x03
#define GYRO_RATE_X_LSB_ADDR 0x02

#define SENSOR_TIME_BYTE2 0x1A
#define SENSOR_TIME_BYTE1 0x19
#define SENSOR_TIME_BYTE0 0x18

//interrupt config bit masks
#define acc_data_ready_interrupt_io2_mask 6
#define acc_fifo_watermark_interrupt_io2_mask 5
#define acc_fifo_full_interrupt_io2_mask 4
#define acc_data_ready_interrupt_io1_mask 2
#define acc_fifo_watermark_interrupt_io1_mask 1
#define acc_fifo_full_interrupt_io1_mask 0

#define ACC_SENSITIVITY_6G 5460 // units: LSB/g

//enumeration to wrap the chip select options while using SPI. The gyro and accelerometer use
//different chip select pins so hardware needs to control separately.
typedef enum BMI088_chip_select_mode {
	gyro,
	accelerometer

} BMI088_chip_select_mode;

//configuration flag masks to configure the BMI088 with various settings and options laid out below
typedef enum BMI088_config_flag_mask {

	accelerometer_enable = 0,
	gyro_interrupt_set,
	accelerometer_fifo_enable,
	accelerometer_fifo_mode_bit1,
	accelerometer_fifo_mode_bit2,
	fifo_interrupt_enable,
	fifo_interrupt_setting_bit1,
	fifo_interrupt_setting_bit2,
	gyro_fifo_enable,
	gyro_fifo_mode_bit1,
	gyro_fifo_mode_bit2

}BMI088_config_flag_mask;

typedef enum BMI088_status_flag_mask {
	accelerometer_enabled = 0,
	gyroscope_enabled
}BMI088_status_flag_mask;


//The BMI088 IMU Interface
typedef struct BMI088_t {

    uint32_t config_flags;
    uint32_t status_flags;

    SPI_HandleTypeDef* bus_handle;

    int* acc_z;
    int* acc_y;
    int* acc_x;

    float** acc_calculated_values;

    uint8_t acc_iterator;
    uint8_t acc_buffer_full;

    int* gyro_z;
    int* gyro_y;
    int* gyro_x;

    float** gyro_calculated_values;

    int* sensor_time;

    uint8_t gyro_iterator;
    uint8_t gyro_buffer_full;

} BMI088_t;


BMI088_t* config_BMI088(uint32_t config_flags, SPI_HandleTypeDef* SPI_Handle);

void update_BMI088_int(BMI088_t* imu_struct);

void update_BMI088_stream(BMI088_t* imu_struct);

void read_timer_BMI088(BMI088_t* imu_interface);



#endif /* INC_BMI088_H_ */
