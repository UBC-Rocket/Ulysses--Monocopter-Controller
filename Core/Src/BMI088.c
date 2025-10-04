/*
 * BMI088.c
 *
 *  Created on: Jul 20, 2025
 *      Author: ak915
 */

#include "BMI088.h"

//writes and reads 1 byte of data to BMI088 IMU through SPI
static uint8_t write_BMI088(BMI088_t* imu_interface, uint8_t* reg_addr, uint8_t* tx, BMI088_chip_select_mode cs_mode);
static uint8_t read_BMI088(uint8_t* rx, BMI088_t* imu_interface, uint8_t* reg_addr, BMI088_chip_select_mode cs_mode);
static void update_BMI088_stream_acc(BMI088_t* imu_struct);
static void update_BMI088_stream_gyro(BMI088_t* imu_struct);
static void BMI088_convert_LSB_to_G(BMI088_t* imu_interface);
static void BMI088_convert_LSB_to_angular_vel(BMI088_t* imu_interface);


/*	CONFIG FLAGS
 *  Bit 0 : accelerometer enable (accelerometer in suspend mode)
 *  Bit 1 : Gyro enable (Gyroscope in deep suspend mode)
 *  Bit 2 : Gyro Interrupt Set
 *  Bit 3 : Accelerometer FIFO Enable
 *
 *  Bit 4 : Accelerometer Range Set Bit 1
 *  Bit 5 : Accelerometer Range Set Bit 2			default accelerometer range = 6g
 *  Bit 6 : FIFO Interrupt Enable
 *  Bit 7 : FIFO Interrupt Setting bit 1
 *
 *  Bit 8 : FIFO Interrupt Setting bit 2
 *  Bit 9 : Gyro FIFO Enable
 *  Bit 10: Gyro FIFO Mode bit 1
 *  Bit 11: Gyro FIFO Mode bit 2
 *
 *  Bit 12: SPI DMA Enable
 *  Bit 13: SPI DMA Global Interrupt Enable
 *
 *
 *
 */
BMI088_t* config_BMI088(uint32_t config_flags, SPI_HandleTypeDef* SPI_Handle) {

	//allocate all necessary handles and IMU interfaces
	BMI088_t* imu = (BMI088_t*)malloc(sizeof(BMI088_t));
	imu->bus_handle = SPI_Handle;
	imu->config_flags = config_flags;
	imu->status_flags = 0;

	//check gyroscope product id first
	uint8_t bmi088_rx;
	uint8_t bmi088_tx;
	uint8_t bmi088_reg_addr = CHIP_ID_ADDR | READ_MASK_SPI;

	//if gyroscope product id cannot be verified, call error handler and reset
	if((!read_BMI088(&bmi088_rx, imu, &bmi088_reg_addr, gyro)) || bmi088_rx != GYRO_CHIP_ID) {
		Error_Handler();
	}

	//initialize temporary gyro circular buffer
	imu->gyro_x = (int*)malloc(GYRO_BUFFER_SIZE*sizeof(int));
	imu->gyro_y = (int*)malloc(GYRO_BUFFER_SIZE*sizeof(int));
	imu->gyro_z = (int*)malloc(GYRO_BUFFER_SIZE*sizeof(int));

	//initialize gyroscope iterator
	imu->gyro_iterator = 0;

	//allocate multi-dimensional array for gyroscope to hold all calculated values
	imu->gyro_calculated_values = (float**)malloc(GYRO_DOF*sizeof(float*));
	for(int i = 0; i < GYRO_DOF; i++) {
		imu->gyro_calculated_values[i] = (float*)malloc(GYRO_BUFFER_SIZE*sizeof(float));
	}

	imu->status_flags |= (1 << gyroscope_enabled);

	//if gyroscope fifo bit is enabled,
	if(config_flags & (1 << gyro_fifo_enable)) {
		bmi088_tx = GYRO_ENABLE_FIFO;
		bmi088_reg_addr = GYRO_FIFO_ENABLE_ADDR | WRITE_MASK_SPI;

		if(!write_BMI088(imu, &bmi088_reg_addr, &bmi088_tx, gyro)) {

		}
	}

	//////////////////////////////////////////////////////////////////////////////

	//enter this sequence if accelerometer is enabled for use
	uint8_t accelerometer_is_enabled = 0;
	if(config_flags & (1 << accelerometer_enable)) {

		//data to be transmitted in order to enable accelerometer. Write 0x04
		bmi088_tx = 0x04 | WRITE_MASK_SPI;

		//need to write to the accelerometer power control register
		bmi088_reg_addr = ACC_PWR_CTL_ADDR | WRITE_MASK_SPI;

		//enable the accelerometer by writing 0x04 to the power control register
		if(!write_BMI088(imu, &bmi088_reg_addr, &bmi088_tx, accelerometer)) {
			Error_Handler();
		}

		//set accelerometer to SPI Mode
		HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_SET);
		HAL_Delay(100);

		//verify accelerometer chip id
		bmi088_reg_addr = CHIP_ID_ADDR | READ_MASK_SPI;
		if(!read_BMI088(&bmi088_rx, imu, &bmi088_reg_addr, accelerometer) || bmi088_rx != ACC_CHIP_ID) {
			Error_Handler();
		}

		//declare all the integer buffers to hold the SPI data input
		imu->acc_x = (int*)malloc(ACC_BUFFER_SIZE*sizeof(int));
		imu->acc_y = (int*)malloc(ACC_BUFFER_SIZE*sizeof(int));
		imu->acc_z = (int*)malloc(ACC_BUFFER_SIZE*sizeof(int));
		imu->acc_iterator = 0;

		//declare multi-dimensional array to store calculated accelerometer values
		imu->acc_calculated_values = (float**)malloc(ACC_DOF*sizeof(float*));

		for(int i = 0; i < ACC_DOF; i++) {
			*(imu->acc_calculated_values + i) = (float*)malloc(ACC_BUFFER_SIZE*sizeof(float));
		}

		accelerometer_is_enabled = 1;
		imu->status_flags |= (1 << accelerometer_enabled);

		imu->sensor_time = (int*)malloc(3*sizeof(int));
//		bmi088_reg_addr =
//		if(!write_BMI088(imu, &))
	} else {

		//make sure that all pointers point to NULL when not using.
		imu->acc_x = NULL;
		imu->acc_y = NULL;
		imu->acc_z = NULL;
		imu->acc_calculated_values = NULL;
		imu->acc_iterator = 0;
	}

	//////////////////////////////////////////////////////////////////////////////

	//BELOW FEATURE IS NOT IMPLEMENTED

	//accelerometer fifo mode select
//	if(accelerometer_is_enabled &&
//			(config_flags & (1 << accelerometer_fifo_enable))) {
//
//		bmi088_tx = ACC_FIFO_ENABLE;
//		bmi088_reg_addr = ACC_FIFO_CONFIG0_ADDR | WRITE_MASK_SPI;
//
//		//set the accelerometer to fifo mode instead of stream mode
//		if(!write_BMI088(imu, &bmi088_reg_addr, &bmi088_tx, accelerometer)) {
//
//		}
//	}
	return imu;
}

void update_BMI088_stream(BMI088_t* imu_interface) {

	update_BMI088_stream_gyro(imu_interface);

	if((imu_interface->status_flags) & (1 << accelerometer_enabled)) {
		update_BMI088_stream_acc(imu_interface);
	}
}

void read_timer_BMI088(BMI088_t* imu_interface) {
	uint8_t rx;
	uint8_t tx = SENSOR_TIME_BYTE0 | READ_MASK_SPI;

	for(int i = 0; i < 3; i++) {

		read_BMI088(&rx, imu_interface, &tx, accelerometer);

		tx += 1;

		*(imu_interface->sensor_time) |= rx << (8*i);
	}

}

void update_BMI088_int(BMI088_t* imu_struct) {

}

static void update_BMI088_stream_acc(BMI088_t* imu_struct) {

	uint8_t bmi088_rx;
	uint8_t bmi088_reg_addr;


	bmi088_reg_addr = ACC_STATUS_ADDR | READ_MASK_SPI;


	if(!read_BMI088(&bmi088_rx, imu_struct,&bmi088_reg_addr, accelerometer)) {

	}

	if((bmi088_rx & (1 << ACC_STATUS_DATA_READY_MASK))) {

		//read accelerometer data from the registers
		uint8_t bmi088_reg_addr_acceleration_values[6] = {
				ACC_Z_MSB_ADDR | READ_MASK_SPI,
				ACC_Z_LSB_ADDR | READ_MASK_SPI,
				ACC_Y_MSB_ADDR | READ_MASK_SPI,
				ACC_Y_LSB_ADDR | READ_MASK_SPI,
				ACC_X_MSB_ADDR | READ_MASK_SPI,
				ACC_X_LSB_ADDR | READ_MASK_SPI
		};

		for(int i = 0; i < 6; i++) {

			if(!read_BMI088(&bmi088_rx, imu_struct,
					&bmi088_reg_addr_acceleration_values[i], accelerometer)) {
				continue;
			}

			switch(bmi088_reg_addr_acceleration_values[i]) {
				case ACC_Z_MSB_ADDR | READ_MASK_SPI:
					*(imu_struct->acc_z + imu_struct->acc_iterator) = (0x0000 | bmi088_rx) << 8;
				break;

				case ACC_Z_LSB_ADDR | READ_MASK_SPI:
					*(imu_struct->acc_z + imu_struct->acc_iterator) =
							*(imu_struct->acc_z + imu_struct->acc_iterator) | bmi088_rx;
				break;
				case ACC_Y_MSB_ADDR | READ_MASK_SPI:
					*(imu_struct->acc_y + imu_struct->acc_iterator) = (0x0000 | bmi088_rx) << 8;
				break;
				case ACC_Y_LSB_ADDR | READ_MASK_SPI:
					*(imu_struct->acc_y + imu_struct->acc_iterator) =
							*(imu_struct->acc_y + imu_struct->acc_iterator) | bmi088_rx;
				break;

				case ACC_X_MSB_ADDR | READ_MASK_SPI:
					*(imu_struct->acc_x + imu_struct->acc_iterator) = (0x0000 | bmi088_rx) << 8;
				break;

				case ACC_X_LSB_ADDR | READ_MASK_SPI:
					*(imu_struct->acc_x + imu_struct->acc_iterator) =
							*(imu_struct->acc_x + imu_struct->acc_iterator) | bmi088_rx;
				break;
			}

		}

	} else {
		return;
	}

	if(imu_struct->acc_iterator == ACC_BUFFER_SIZE) {
		imu_struct->acc_buffer_full = 1;
	} else {
		imu_struct->acc_buffer_full = 0;
	}

	if(imu_struct->acc_buffer_full) {
		//BMI088_convert_LSB_to_G(imu_struct);
	}

	imu_struct->acc_iterator = (imu_struct->acc_iterator + 1) % ACC_BUFFER_SIZE;
}

static void update_BMI088_stream_gyro(BMI088_t* imu_struct) {

	uint8_t bmi088_rx;
	uint8_t bmi088_reg_addr[6] = {
			GYRO_RATE_Z_MSB_ADDR | READ_MASK_SPI,
			GYRO_RATE_Z_LSB_ADDR | READ_MASK_SPI,
			GYRO_RATE_Y_MSB_ADDR | READ_MASK_SPI,
			GYRO_RATE_Y_LSB_ADDR | READ_MASK_SPI,
			GYRO_RATE_X_MSB_ADDR | READ_MASK_SPI,
			GYRO_RATE_X_LSB_ADDR | READ_MASK_SPI
	};

	for(int i = 0; i < 6; i++) {

		if(!read_BMI088(&bmi088_rx, imu_struct,
							&bmi088_reg_addr[i], gyro)) {
			continue;
		}

		switch(bmi088_reg_addr[i]) {

			case GYRO_RATE_Z_MSB_ADDR | READ_MASK_SPI:
				*(imu_struct->gyro_z + imu_struct->gyro_iterator) = (0x00 | bmi088_rx) << 8;
				break;
			case GYRO_RATE_Z_LSB_ADDR | READ_MASK_SPI:
				*(imu_struct->gyro_z + imu_struct->gyro_iterator) =
						*(imu_struct->gyro_z + imu_struct->gyro_iterator) | bmi088_rx;
				break;
			case GYRO_RATE_Y_MSB_ADDR | READ_MASK_SPI:
				*(imu_struct->gyro_y + imu_struct->gyro_iterator) = (0x00 | bmi088_rx) << 8;
				break;
			case GYRO_RATE_Y_LSB_ADDR | READ_MASK_SPI:
				*(imu_struct->gyro_y + imu_struct->gyro_iterator) =
						*(imu_struct->gyro_y + imu_struct->gyro_iterator) | bmi088_rx;
				break;
			case GYRO_RATE_X_MSB_ADDR | READ_MASK_SPI:
				*(imu_struct->gyro_x + imu_struct->gyro_iterator) = (0x00 | bmi088_rx) << 8;
				break;
			case GYRO_RATE_X_LSB_ADDR | READ_MASK_SPI:
				*(imu_struct->gyro_x + imu_struct->gyro_iterator) =
						*(imu_struct->gyro_x + imu_struct->gyro_iterator) | bmi088_rx;
				break;

		}
	}

	if(imu_struct->gyro_iterator == GYRO_BUFFER_SIZE) {
		imu_struct->gyro_buffer_full = 1;
	} else {
		imu_struct->gyro_buffer_full = 0;
	}

	if(imu_struct->gyro_buffer_full) {
		//BMI088_convert_LSB_to_angular_vel(imu_struct);
	}

	imu_struct->gyro_iterator = (imu_struct->gyro_iterator + 1) % GYRO_BUFFER_SIZE;
}

static uint8_t read_BMI088(uint8_t* rx, BMI088_t* imu_interface, uint8_t* reg_addr, BMI088_chip_select_mode cs_mode) {

	uint8_t bytes_read = 1;

	switch(cs_mode) {
		case gyro:
			HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_RESET);
			break;

		case accelerometer:
			HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_RESET);
			bytes_read = 2;
			break;
	}

	if(HAL_SPI_Transmit(imu_interface->bus_handle, reg_addr, 1, HAL_MAX_DELAY) != HAL_OK) {

		if(cs_mode == accelerometer) {
			HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_SET);
		}

		if(cs_mode == gyro) {
			HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_SET);
		}

		return 0;

	}

	for(int i = 0; i < bytes_read; i++) {

		if(HAL_SPI_Receive(imu_interface->bus_handle, rx, 1, HAL_MAX_DELAY) != HAL_OK) {

			HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_SET);
			return 0;
		}

	}

	switch(cs_mode) {
		case gyro:
			HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_SET);
			break;

		case accelerometer:
			HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_SET);
			break;
	}

	return 1;
}

static uint8_t write_BMI088(BMI088_t* imu_interface, uint8_t* reg_addr, uint8_t* tx, BMI088_chip_select_mode cs_mode) {

	switch(cs_mode) {
		case gyro:
			HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_RESET);
			break;

		case accelerometer:
			HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_RESET);
			break;
	}

	if(HAL_SPI_Transmit(imu_interface->bus_handle, reg_addr, 1, HAL_MAX_DELAY) != HAL_OK) {

		HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_SET);
		return 0;

	}

	if(HAL_SPI_Transmit(imu_interface->bus_handle, tx, 1, HAL_MAX_DELAY) != HAL_OK) {

		HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_SET);
		return 0;

	}

	switch(cs_mode) {
		case gyro:
			HAL_GPIO_WritePin(BMI_GYRO_Chip_Select_GPIO_Port, BMI_GYRO_Chip_Select_Pin, GPIO_PIN_RESET);
			break;

		case accelerometer:
			HAL_GPIO_WritePin(BMI_ACC_Chip_Select_GPIO_Port, BMI_ACC_Chip_Select_Pin, GPIO_PIN_RESET);
			break;
	}
	return 1;
}

static void BMI088_convert_LSB_to_G(BMI088_t* imu_interface) {
	uint8_t bmi088_rx;
	uint8_t bmi088_reg_addr = ACC_RANGE_CTL_ADDR | READ_MASK_SPI;
	uint16_t lsb_to_g_conversion_factor = 5460;

	if(!read_BMI088(&bmi088_rx, imu_interface, &bmi088_reg_addr, accelerometer)) {
	}

	switch(bmi088_rx) {
	case 0x00:
		lsb_to_g_conversion_factor = 10920;
		break;
	case 0x01:
		break;
	case 0x02:
		lsb_to_g_conversion_factor = 2730;
		break;
	case 0x03:
		lsb_to_g_conversion_factor = 1365;
	}

	for(int i = 0; i < ACC_BUFFER_SIZE; i++) {

		*( *(imu_interface->acc_calculated_values) +i) =
				(float)(*(imu_interface->acc_x)) / (float)lsb_to_g_conversion_factor;
		*( *(imu_interface->acc_calculated_values + 1) +i) =
				(float)(*(imu_interface->acc_y)) / (float)lsb_to_g_conversion_factor;
		*( *(imu_interface->acc_calculated_values + 2) +i) =
				(float)(*(imu_interface->acc_z)) / (float)lsb_to_g_conversion_factor;

	}
}

static void BMI088_convert_LSB_to_angular_vel(BMI088_t* imu_interface) {
	uint8_t bmi088_rx;
	uint8_t bmi088_reg_addr = GYRO_RANGE_CTL_ADDR | READ_MASK_SPI;
	float lsb_to_angular_vel_conv_factor = 16.384;

	if(!read_BMI088(&bmi088_rx, imu_interface, &bmi088_reg_addr, gyro)) {

	}

	switch(bmi088_rx) {
		case 0x00:
		break;

		case 0x01:
		lsb_to_angular_vel_conv_factor = 32.768;
		break;

		case 0x02:
		lsb_to_angular_vel_conv_factor = 65.356;
		break;

		case 0x03:
		lsb_to_angular_vel_conv_factor = 131.072;
		break;

		case 0x04:
		lsb_to_angular_vel_conv_factor = 262.144;
		break;
	}

	for(int i = 0; i < GYRO_BUFFER_SIZE; i++) {
		*( *(imu_interface->gyro_calculated_values) +i) =
				(float)(*(imu_interface->gyro_x)) / (float)lsb_to_angular_vel_conv_factor;
		*( *(imu_interface->gyro_calculated_values + 1) +i) =
				(float)(*(imu_interface->gyro_y)) / (float)lsb_to_angular_vel_conv_factor;
		*( *(imu_interface->gyro_calculated_values + 2) +i) =
				(float)(*(imu_interface->gyro_z)) / (float)lsb_to_angular_vel_conv_factor;
	}
}
