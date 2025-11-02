#include "BMI088_accel.h"
#include "BMI088_gyro.h"
#include "MS5611_baro.h"
#include "SPI_queue.h"
#include "main.h"
#include "stm32u5xx_hal.h"

#ifndef SPI_DEVICE_INTERACTIONS
#define SPI_DEVICE_INTERACTIONS

#define BMI088_ACC_CHIP_ID_VALUE 0x1E
#define BMI088_GYRO_CHIP_ID_VALUE  0x0F

extern bmi088_accel_sample_queue_t bmi088_acc_sample_ring;
extern bmi088_gyro_sample_queue_t bmi088_gyro_sample_ring;

extern spi_job_queue_t jobq_spi_1;
extern spi_job_queue_t jobq_spi_2;


uint8_t bmi088_accel_init(SPI_HandleTypeDef *hspi,
                       GPIO_TypeDef *cs_port,
                       uint16_t cs_pin,
                       bmi088_accel_t *dev);


/* -------------------------------------------------------------------------- */
/* Done callback: called after SPI DMA completes                              */
/* -------------------------------------------------------------------------- */
static void bmi088_accel_done(spi_job_t *job,
                              const uint8_t *rx_buf,
                              void *arg);

/* -------------------------------------------------------------------------- */
/* Accelerometer interrupt entry point                                        */
/* -------------------------------------------------------------------------- */
void bmi088_accel_interrupt(void);

uint8_t bmi088_gyro_init(SPI_HandleTypeDef *hspi,
                         GPIO_TypeDef *cs_port,
                         uint16_t cs_pin,
                         bmi088_gyro_t *dev);

static void bmi088_gyro_done(spi_job_t *job,
                              const uint8_t *rx_buf,
                              void *arg);

void bmi088_gyro_interrupt(void);

#endif //DEVICE_INTERACTIONS
