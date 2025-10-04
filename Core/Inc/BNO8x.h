/*
 * BNO8x.h
 *
 *  Created on: Jul 20, 2025
 *      Author: ak915
 */

#ifndef INC_BNO8X_H_
#define INC_BNO8X_H_

#include <stdint.h>
#include <stdlib.h>

#define USING_BNO8x

#define PRODUCT_ID_REQUEST 0xF9

typedef struct BNO8x_t {

    uint32_t config_flags;

} BNO8x_t;


BNO8x_t* config_BNO8x(uint32_t config_flags);

void update_BNO8x(BNO8x_t* imu_struct);

#endif /* INC_BNO8X_H_ */
