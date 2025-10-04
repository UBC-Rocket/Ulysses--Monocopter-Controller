/*
 * barometer.c
 *
 *  Created on: Jul 28, 2025
 *      Author: ak915
 */

#include "barometer.h"

static uint8_t write_command_to_baro(uint8_t command_tx, MS5611_t* baro_interface);
static uint8_t trigger_prom_read_command(MS5611_t* baro_interface);
static void stitch_baro_data(MS5611_t* baro_interface);

uint32_t hal_delay_adc = 30; //units in ms

MS5611_t* config_MS5611_barometer(uint32_t config_flags, SPI_HandleTypeDef* hspi) {

	//allocate memory for the barometer interface
	MS5611_t* baro_interface = (MS5611_t*)malloc(sizeof(MS5611_t));
	baro_interface->bus_handle = hspi;
	baro_interface->baro_status_flags = 0;

	//allocates memory if pressure reading is enabled
	if(config_flags & (1 << enable_pressure_reading)) {
		baro_interface->baro_pressure_data = (uint8_t*)malloc(BAROMETER_ADC_BUFFER_SIZE*sizeof(uint8_t));

		//sets the sttaus flag within the barometer interface to enable pressure readings
		baro_interface->baro_status_flags |= (1 << pressure_reading_enabled);
	} else {
		baro_interface->baro_pressure_data = NULL;
	}

	//allocates memory if temperature reading is enabled
	if(config_flags & (1 << enable_temperature_reading)) {
		baro_interface->baro_temperature_data =
				(int*)malloc(BAROMETER_ADC_BUFFER_SIZE*sizeof(int));

		//sets the status flag within the barometer interface to enable temperature readings
		baro_interface->baro_status_flags |= (1 << temperature_reading_enabled);
	} else {
		baro_interface->baro_temperature_data = NULL;
	}

	//initializes the barometer buffer iterator to zero
	baro_interface->baro_buffer_iterator = 0;

	//reset the barometer
	if(baro_interface != NULL) {

		if(!write_command_to_baro(RESET_COMMAND, baro_interface)) {
			//if the barometer can't be reset, reboot the system
			Error_Handler();
		}

		baro_interface->prom_initialization_data =
				(int*)malloc(PROM_TOTAL_ADDRESS*sizeof(int));

		baro_interface->baro_pressure_data_corrected = 0;
//		if(!trigger_prom_read_command(baro_interface)){
//			Error_Handler();
//		}

	}

	return baro_interface;
}

void read_from_barometer(MS5611_t* baro_interface) {

	if(baro_interface->baro_status_flags & (1 << pressure_reading_enabled)) {

		if(!write_command_to_baro(CONVERT_D1_OSR_256, baro_interface)) {

		}

		HAL_Delay(4);

		if(!write_command_to_baro(ADC_READ_COMMAND, baro_interface) ||
				baro_interface->baro_pressure_data == 0)
		{
			//hal_delay_adc += 30;

			if(hal_delay_adc >= 500) {
				//Error_Handler();
			}
		}

		stitch_baro_data(baro_interface);

	}
}


static uint8_t write_command_to_baro(uint8_t command_tx, MS5611_t* baro_interface) {
	//drive the chip select pin to low
	BARO1_SELECT();

	uint8_t tx = command_tx;

	//this variable indicates whether the write command was successful or not
	uint8_t write_command_status = 1;

	switch(command_tx) {
		case RESET_COMMAND:

			if(HAL_SPI_Transmit(baro_interface->bus_handle, &tx, 1, HAL_MAX_DELAY) != HAL_OK) {
				write_command_status = 0;
			}

			HAL_Delay(5);
		break;

		case ADC_READ_COMMAND:

			if(HAL_SPI_Transmit(baro_interface->bus_handle, &tx, 1, HAL_MAX_DELAY) != HAL_OK) {
				write_command_status = 0;
			}

			if(HAL_SPI_Receive(baro_interface->bus_handle,
					baro_interface->baro_pressure_data,
					BAROMETER_ADC_BUFFER_SIZE, HAL_MAX_DELAY) != HAL_OK)
			{
				write_command_status = 0;
			}

		break;

		default:
			if(HAL_SPI_Transmit(baro_interface->bus_handle, &tx, 1, HAL_MAX_DELAY) != HAL_OK) {
				write_command_status = 0;
			}
	}

	BARO1_DESELECT();

	return write_command_status;
}

static uint8_t trigger_prom_read_command(MS5611_t* baro_interface) {

	uint8_t prom_address_tx = PROM_READ_COMMAND;
	uint8_t prom_buffer_index = 0;
	uint8_t prom_read_successful = 1;
	uint8_t* rx = (uint8_t*)malloc(2*sizeof(uint8_t));

	while(prom_address_tx <= 0xAE) {

		BARO1_SELECT();

		if(baro_interface->bus_handle->State != HAL_SPI_STATE_READY) {
			prom_read_successful = 0;
			break;
		}

		__HAL_LOCK(baro_interface->bus_handle);

		//configure the SPI communication direction
		SPI_2LINES_TX(baro_interface->bus_handle);

		//set the number of bytes for current transfer for TX
		MODIFY_REG(baro_interface->bus_handle->Instance->CR2, SPI_CR2_TSIZE, 2);

		//enable the SPI peripheral
		__HAL_SPI_ENABLE(baro_interface->bus_handle);

		SET_BIT(baro_interface->bus_handle->Instance->CR1, SPI_CR1_CSTART);

		baro_interface->bus_handle->pTxBuffPtr = &prom_address_tx;

		//check the TXP Flag in the SPI peripheral and send out 8 bits of command instruction
		if((__HAL_SPI_GET_FLAG(baro_interface->bus_handle, SPI_FLAG_TXP))) {
			*((__IO uint8_t *)&baro_interface->bus_handle->Instance->TXDR) =
					*((const uint8_t *)baro_interface->bus_handle->pTxBuffPtr);
		}

//		if(HAL_SPI_Transmit(baro_interface->bus_handle, &prom_address_tx, 1, HAL_MAX_DELAY) != HAL_OK) {
//			prom_read_successful = 0;
//			break;
//		}
//
//		if(HAL_SPI_Receive(baro_interface->bus_handle, rx, 2, HAL_MAX_DELAY) != HAL_OK) {
//			prom_read_successful = 0;
//			break;
//		}

		BARO1_DESELECT();

		baro_interface->prom_initialization_data[prom_buffer_index] =
				(*rx) << 16 | *(rx + 1);

		prom_address_tx += 2;

		prom_buffer_index += 1;
	}

	free(rx);

	return prom_read_successful;
}

static void stitch_baro_data(MS5611_t* baro_interface) {
	baro_interface->baro_pressure_data_corrected =
			*(baro_interface->baro_pressure_data) << 16 |
			*(baro_interface->baro_pressure_data+1) << 8 |
			*(baro_interface->baro_pressure_data+2);
}
