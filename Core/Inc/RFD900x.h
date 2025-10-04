/*
 * RFD900x.h
 *
 *  Created on: Aug 9, 2025
 *      Author: ak915
 */

#ifndef INC_RFD900X_H_
#define INC_RFD900X_H_

#include <stdlib.h>
#include <stdint.h>
#include "stm32u5xx_hal.h"
#include "main.h"
#include <string.h>

#define MAX_BUFFER_SIZE_RADIO_TX 100

typedef struct RFD900_t {
	int baud_rate;

	UART_HandleTypeDef* uart_handle;

	uint8_t transfer_failed;

}RFD900_t;

RFD900_t* init_rfd900(UART_HandleTypeDef* uart);

#endif /* INC_RFD900X_H_ */
