/*
 * list.h
 *
 *  Created on: Jul 27, 2025
 *      Author: ak915
 */

#ifndef INC_LIST_H_
#define INC_LIST_H_

#include <stdlib.h>
#include <stdint.h>
#include "stm32u5xx_hal.h"
#include "main.h"

//library for implementing abstract data types for various peripherals
#define MAX_LIST_SIZE 20

//doubly linked list of IMU data
typedef struct sensor_data {

	float* data_array; //in the order of gyroscope, accelerometer, sensor time, and barometer
	struct sensor_data* next;
	struct sensor_data* prev;

}sensor_data;

typedef struct sensor_data_list {
	sensor_data* front_node;

	uint8_t list_size;
	uint8_t list_iterator;

}sensor_data_list;

//initializes one empty node as the first node
sensor_data_list* initialize_list();

uint8_t append_list(sensor_data_list* list);

uint8_t pop(sensor_data_list* list);

uint8_t remove_element(sensor_data_list* list, uint8_t index);

uint8_t insert_element(sensor_data_list* list, uint8_t index);

void destroy_list();



#endif /* INC_LIST_H_ */
