#include "SPI_device_interactions.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

bmi088_accel_sample_queue_t bmi088_acc_sample_ring;
bmi088_gyro_sample_queue_t bmi088_gyro_sample_ring;

bmi088_accel_t accel;
bmi088_gyro_t gyro;

spi_job_queue_t jobq_spi_1;
spi_job_queue_t jobq_spi_2;

uint8_t bmi088_accel_init(SPI_HandleTypeDef *hspi,
                       GPIO_TypeDef *cs_port,
                       uint16_t cs_pin,
                       bmi088_accel_t *dev)
{
    uint8_t tx[16], rx[16];

    /* --- 1. Soft reset --- */
    size_t n = bmi088_accel_build_softreset(tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(600000);

    /* --- 2. Verify chip ID --- */
    n = bmi088_accel_build_read_reg(BMI088_ACC_CHIP_ID_REG, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    if (rx[2] != BMI088_ACC_CHIP_ID_VALUE)
        return 1;

    /* --- 3. Power on accelerometer --- */
    n = bmi088_accel_build_power_conf(BMI088_ACC_PWR_ACTIVE, tx, dev);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, 2, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, &tx[2], 2, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);
    
    n = bmi088_accel_build_read_reg(BMI088_ACC_PWR_CONF, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    if(rx[2] != 0)
        return 2;

    n = bmi088_accel_build_read_reg(BMI088_ACC_PWR_CTRL, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    if(rx[2] != 4)
        return 3;

    /* --- 4. Set measurement range (±24 g) --- */
    n = bmi088_accel_build_range(BMI088_ACC_RANGE_3G, tx, dev);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    /* --- 5. Configure ODR = 800 Hz, normal bandwidth --- */
    n = bmi088_accel_build_odr_bw(BMI088_ACC_ODR_800_HZ,
                                  BMI088_ACC_BWP_NORMAL,
                                  tx, dev);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    n = bmi088_accel_build_read_reg(BMI088_ACC_CONF, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    n = bmi088_accel_build_read_reg(BMI088_ACC_RANGE, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, 3, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    /* --- 6. Configure INT1 electrical behaviour --- */
    n = bmi088_accel_build_int_pin_conf(BMI088_ACC_INT1,
                                        BMI088_ACC_INT_PUSH_PULL,
                                        BMI088_ACC_INT_ACTIVE_HIGH,
                                        false, // pulse (not latched)
                                        tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

    /* --- 7. Map data-ready event to INT1 --- */
    n = bmi088_accel_build_int_event_map(BMI088_ACC_INT1,
                                         BMI088_ACC_INT_EVENT_DATA_READY,
                                         tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

    /* --- 8. (Optional) small startup delay --- */

    n = bmi088_accel_build_read_reg(BMI088_ACC_STATUS_REG, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    if((rx[2] & 0x80) == 0)
        return 4;

    n = bmi088_accel_build_read(BMI088_ACC_READ_DATA_XYZ, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    delay_us(1000);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(50000);

    bmi088_accel_sample_t sample;
    bmi088_accel_parse_data_xyz(&rx[2], &sample, dev);

    return 0;
}


/* -------------------------------------------------------------------------- */
/* Done callback: called after SPI DMA completes                              */
/* -------------------------------------------------------------------------- */
static void bmi088_accel_done(spi_job_t *job,
                              const uint8_t *rx_buf,
                              void *arg)
{
    (void)job;
    (void)arg;

    // Parse accelerometer XYZ data (skip address byte)
    bmi088_accel_sample_t sample;
    if (bmi088_accel_parse_data_xyz(&rx_buf[1], &sample, &accel)) {
        bmi088_acc_sample_queue(&bmi088_acc_sample_ring,  &sample);
    }
}

/* -------------------------------------------------------------------------- */
/* Accelerometer interrupt entry point                                        */
/* -------------------------------------------------------------------------- */
void bmi088_accel_interrupt(void)
{
    const uint32_t now = timestamp_us();

    spi_job_t job;

    job.cs_port  = BMI_ACC_Chip_Select_GPIO_Port;
    job.cs_pin   = BMI_ACC_Chip_Select_Pin;

    job.len      = bmi088_accel_build_read(BMI088_ACC_READ_DATA_XYZ, job.tx); 
    job.t_sample = now;
    job.type     = SPI_XFER_TXRX;
    job.done     = bmi088_accel_done;
    job.done_arg = NULL;
    job.sensor   = SENSOR_ID_ACCEL;

    spi_submit_job(job, &jobq_spi_1);

}


bmi088_gyro_t gyro_dev;
uint8_t gyro_tx[8];
bmi088_gyro_sample_t gyro_sample_ring; // You can expand to a queue later

/* -------------------------------------------------------------------------- */
/* Gyroscope Initialization                                                   */
/* -------------------------------------------------------------------------- */
uint8_t bmi088_gyro_init(SPI_HandleTypeDef *hspi,
                         GPIO_TypeDef *cs_port,
                         uint16_t cs_pin,
                         bmi088_gyro_t *dev)
{
    uint8_t tx[16], rx[16];
    size_t n;

    /* --- 1. Soft reset --- */
    n = bmi088_gyro_build_softreset(tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    delay_us(1000);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(30000); // per datasheet ≥30 ms

    /* --- 2. Verify chip ID --- */
    n = bmi088_gyro_build_read(BMI088_GYRO_READ_CHIP_ID, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(5000);

    if (rx[1] != BMI088_GYRO_CHIP_ID_VALUE)
        return 1;

    /* --- 3. Power on gyroscope --- */
    n = bmi088_gyro_build_power_conf(BMI088_GYRO_PWR_NORMAL, tx, dev);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(80000); // PLL lock time ≥80 ms

    /* --- 4. Configure range ±500 dps --- */
    n = bmi088_gyro_build_range(BMI088_GYRO_RANGE_500DPS, tx, dev);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(5000);

    /* --- 5. Configure ODR 400 Hz / BW 116 Hz --- */
    n = bmi088_gyro_build_odr(BMI088_GYRO_ODR_400_HZ_BW_116, tx, dev);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(5000);

    /* --- 6. Configure INT3 electrical behavior (push-pull, active high) --- */
    n = bmi088_gyro_build_int_pin_conf(BMI088_GYRO_INT3,
                                       BMI088_GYRO_INT_PUSH_PULL,
                                       BMI088_GYRO_INT_ACTIVE_HIGH,
                                       tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(5000);

    /* --- 7. Map data-ready event to INT3 --- */
    n = bmi088_gyro_build_int_event_map(BMI088_GYRO_INT3,
                                        BMI088_GYRO_INT_EVENT_DATA_READY,
                                        tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(hspi, tx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    delay_us(5000);

    /* --- 8. Verify first reading --- */
    n = bmi088_gyro_build_read(BMI088_GYRO_READ_DATA_XYZ, tx);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(hspi, tx, rx, n, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

    bmi088_gyro_sample_t s;
    if (!bmi088_gyro_parse_data_xyz(rx, &s, dev))
        return 2;

    return 0;
}

/* -------------------------------------------------------------------------- */
/* Gyroscope DMA done callback                                                */
/* -------------------------------------------------------------------------- */
static void bmi088_gyro_done(spi_job_t *job,
                             const uint8_t *rx_buf,
                             void *arg)
{
    (void)job;
    (void)arg;

    bmi088_gyro_sample_t s;
    if (bmi088_gyro_parse_data_xyz(&rx_buf[1], &s, &gyro)) {
        // TODO: push to ring buffer once implemented
        gyro_sample_ring = s;
    }
}

/* -------------------------------------------------------------------------- */
/* Gyroscope interrupt entry point                                            */
/* -------------------------------------------------------------------------- */
void bmi088_gyro_interrupt(void)
{
    const uint32_t now = timestamp_us();

    spi_job_t job;

    job.cs_port  = BMI_GYRO_Chip_Select_GPIO_Port;
    job.cs_pin   = BMI_GYRO_Chip_Select_Pin;

    job.len      = bmi088_gyro_build_read(BMI088_GYRO_READ_DATA_XYZ, job.tx); 
    job.t_sample = now;
    job.type     = SPI_XFER_TXRX;
    job.done     = bmi088_gyro_done;
    job.done_arg = NULL;
    job.sensor   = SENSOR_ID_GYRO;

    spi_submit_job(job, &jobq_spi_1);
}
