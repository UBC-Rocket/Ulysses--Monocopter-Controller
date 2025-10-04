/*
 * barometer.h
 *
 *  Created on: Jul 23, 2025
 *      Author: ak915
 */

#ifndef INC_BAROMETER_H_
#define INC_BAROMETER_H_

#include <stdlib.h>
#include <stdint.h>
#include "stm32u5xx_hal.h"
#include "main.h"

//SPI Command Macros
#define RESET_COMMAND 		0x1E

#define CONVERT_D1_OSR_256	0x40	//D1 conversion is uncompensated pressure
#define CONVERT_D1_OSR_512	0x42
#define CONVERT_D1_OSR_1024	0x44
#define CONVERT_D1_OSR_2048	0x46
#define CONVERT_D1_OSR_4096	0x48

#define CONVERT_D2_OSR_256	0x50	//D2 conversion is uncompensated temperature
#define CONVERT_D2_OSR_512	0x52
#define CONVERT_D2_OSR_1024	0x54
#define CONVERT_D2_OSR_2048	0x56
#define CONVERT_D2_OSR_4096	0x58

#define ADC_READ_COMMAND	0x00
#define PROM_READ_COMMAND	0xA0	//command ranges from 0xA0 to 0xAE depending on the memory region of the RPOM
#define PROM_TOTAL_ADDRESS	8
#define BAROMETER_ADC_BUFFER_SIZE	3

#define BARO1_SELECT()		HAL_GPIO_WritePin(BARO1_Chip_Select_GPIO_Port, BARO1_Chip_Select_Pin, GPIO_PIN_RESET);
#define BARO1_DESELECT()	HAL_GPIO_WritePin(BARO1_Chip_Select_GPIO_Port, BARO1_Chip_Select_Pin, GPIO_PIN_SET);

typedef enum MS5611_status_mask {

	barometer_reset = 0,
	conversion_error,
	pressure_reading_enabled,
	temperature_reading_enabled

}MS5611_Status_mask;

typedef enum MS5611_config_mask{

	enable_pressure_reading = 0,
	enable_temperature_reading

}MS5611_config_mask;

typedef struct MS5611_t {

	uint32_t baro_status_flags;
	uint8_t* baro_pressure_data;
	int* baro_temperature_data;

	int* prom_initialization_data;

	uint8_t baro_buffer_iterator;

	SPI_HandleTypeDef* bus_handle;

	int baro_pressure_data_corrected;

}MS5611_t;

MS5611_t* config_MS5611_barometer(uint32_t config_flags, SPI_HandleTypeDef* hspi);

void read_from_barometer(MS5611_t* baro_interface);

#endif /* INC_BAROMETER_H_ */
