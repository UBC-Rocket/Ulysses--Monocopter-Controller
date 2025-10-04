/*
 * BNO8x.c
 *
 *  Created on: Jul 20, 2025
 *      Author: ak915
 */


#include "BNO8x.h"

static void write_BNO8x_SPI(uint16_t* data_buffer);
static void read_BNO8x_SPI(uint16_t* data_buffer);


BNO8x_t* config_BNO8x(uint32_t config_flags) {

    //select SPI by writing the to the pins on the imu

    //verify the product ID
    uint16_t product_id_request_shtp = 0xF9;
    uint16_t* data_buffer_receive = (uint16_t*)malloc(20*sizeof(uint16_t));

    write_BNO8x_SPI(&product_id_request_shtp);
    read_BNO8x_SPI(data_buffer_receive);

    if(*data_buffer_receive != (product_id_request_shtp - 1)) {
        //call error handler
    }

}

void update_BNO8x(BNO8x_t* imu_struct) {

}

static void write_BNO8x_SPI(uint16_t* data_buffer) {

}

static void read_BNO8x_SPI(uint16_t* data_buffer) {

}
