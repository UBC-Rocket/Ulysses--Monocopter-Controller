// In Core/Inc/drivers/radio.h

#ifndef RADIO_H
#define RADIO_H

#include "main.h" // This is essential. It includes all the STM32 HAL definitions.

// --- Function Prototypes ---

/**
 * @brief Initializes the radio driver with the specific UART peripheral.
 * @param huart Pointer to the UART_HandleTypeDef for the radio (e.g., &huart5).
 * @retval None
 */
void radio_init(UART_HandleTypeDef* huart);

/**
 * @brief Transmits a block of data over the radio using DMA.
 * @param data Pointer to the data buffer to send.
 * @param length The number of bytes to send.
 * @retval HAL_StatusTypeDef HAL_OK on success, or an error code.
 */
HAL_StatusTypeDef radio_send(uint8_t* data, uint16_t length);

#endif // RADIO_H