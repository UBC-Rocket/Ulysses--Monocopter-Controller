/*
 * RFD900x.c
 *
 *  Created on: Aug 9, 2025
 *      Author: ak915
 */

#include "RFD900x.h"

RFD900_t* init_rfd900(UART_HandleTypeDef* uart) {
	RFD900_t* radio = (RFD900_t*)malloc(sizeof(RFD900_t));

	radio->uart_handle = uart;

	return radio;
}
