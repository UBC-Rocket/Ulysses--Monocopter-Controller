/*
    Driver for the MS5611-01BA03 barometer.
    This driver is only used to parse and create register access packages.
    It is completely bus-agnostic.

    The core functionalities of the device are implemented, as required by the TVR team,
    but some more specific functionalities are not.

    @ UBC Rocket, Sept 29th 2025, Benedikt Howard
*/

#ifndef MS5611_H
#define MS5611_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Command set (SPI/I2C)                                                      */
/* -------------------------------------------------------------------------- */
#define MS5611_CMD_RESET       0x1E  ///< Reset command, wait ~2.8 ms
#define MS5611_CMD_ADC_READ    0x00  ///< Read ADC result (24 bits)
#define MS5611_CMD_CONV_D1     0x40  ///< Convert pressure (add OSR bits)
#define MS5611_CMD_CONV_D2     0x50  ///< Convert temperature (add OSR bits)
#define MS5611_CMD_PROM_BASE   0xA0  ///< Base address for PROM reads

/* PROM addresses (16-bit words) */
#define MS5611_PROM_FACTORY    0xA0
#define MS5611_PROM_C1         0xA2  ///< Pressure sensitivity
#define MS5611_PROM_C2         0xA4  ///< Pressure offset
#define MS5611_PROM_C3         0xA6  ///< Temp coeff of pressure sensitivity
#define MS5611_PROM_C4         0xA8  ///< Temp coeff of pressure offset
#define MS5611_PROM_C5         0xAA  ///< Reference temperature
#define MS5611_PROM_C6         0xAC  ///< Temp coeff of temperature
#define MS5611_PROM_CRC        0xAE  ///< CRC + serial bits

/* -------------------------------------------------------------------------- */
/* Device state struct                                                        */
/* -------------------------------------------------------------------------- */
typedef struct {
    uint16_t C[8];        ///< Calibration coefficients C0..C7 (C0=factory, C7=CRC)
    uint8_t osr;          ///< Selected OSR setting (256–4096)
    uint32_t D1_raw;      ///< Last raw pressure conversion
    uint32_t D2_raw;      ///< Last raw temperature conversion
    int32_t TEMP_centi;   ///< Last compensated temperature (0.01 °C units)
    int32_t P_mbar;       ///< Last compensated pressure (Pa or mbar*100)
} ms5611_t;

/* -------------------------------------------------------------------------- */
/* OSR options                                                                */
/* -------------------------------------------------------------------------- */
typedef enum {
    MS5611_OSR_256  = 0x00, ///< 0.6 ms conversion
    MS5611_OSR_512  = 0x02, ///< 1.2 ms conversion
    MS5611_OSR_1024 = 0x04, ///< 2.3 ms conversion
    MS5611_OSR_2048 = 0x06, ///< 4.6 ms conversion
    MS5611_OSR_4096 = 0x08  ///< 9.1 ms conversion
} ms5611_osr_t;

/* -------------------------------------------------------------------------- */
/* Command builders (bus-agnostic)                                            */
/* -------------------------------------------------------------------------- */

/**
 * @brief Build reset command.
 * @param[out] tx_buf Buffer to hold the 1-byte command.
 * @return Number of bytes to send (1).
 */
size_t ms5611_build_reset(uint8_t *tx_buf);

/**
 * @brief Build PROM read command.
 * @param index PROM index [0..7] (C0..C7).
 * @param[out] tx_buf Buffer to hold the command.
 * @return Number of bytes to send (1).
 */
size_t ms5611_build_prom_read(uint8_t index, uint8_t *tx_buf);

/**
 * @brief Build convert D1 (pressure) command with chosen OSR.
 */
size_t ms5611_build_conv_d1(ms5611_osr_t osr, uint8_t *tx_buf);

/**
 * @brief Build convert D2 (temperature) command with chosen OSR.
 */
size_t ms5611_build_conv_d2(ms5611_osr_t osr, uint8_t *tx_buf);

/**
 * @brief Build ADC read command (returns 24-bit result).
 */
size_t ms5611_build_adc_read(uint8_t *tx_buf);

/* -------------------------------------------------------------------------- */
/* Parsing helpers                                                            */
/* -------------------------------------------------------------------------- */

/**
 * @brief Parse PROM coefficient (2-byte response).
 * @param[in] rx_buf 2-byte raw PROM data.
 * @param[out] coeff Decoded coefficient.
 * @return True if parse OK.
 */
bool ms5611_parse_prom(const uint8_t *rx_buf, uint16_t *coeff);

/**
 * @brief Parse ADC 24-bit result.
 * @param[in] rx_buf 3-byte raw ADC data.
 * @param[out] value Decoded 24-bit unsigned value.
 * @return True if parse OK.
 */
bool ms5611_parse_adc(const uint8_t *rx_buf, uint32_t *value);

/**
 * @brief Compute compensated pressure and temperature from raw values.
 * @param[in,out] dev Device state with coefficients + D1/D2.
 * @return True if computation OK.
 */
bool ms5611_compute(ms5611_t *dev);

/**
 * @brief Validate PROM CRC-4.
 * @param[in] coeff Array of 8 PROM words.
 * @return True if CRC valid.
 */
bool ms5611_check_crc(const uint16_t *coeff);

#endif /* MS5611_H */
