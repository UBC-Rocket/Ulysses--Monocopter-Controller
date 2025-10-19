// In Core/Src/drivers/radio.c

#include "Drivers/radio.h"

// --- Private Variables ---

// This static variable will hold a pointer to the UART peripheral we are using.
// It's "static" so it's only visible inside this file, enforcing encapsulation.
static UART_HandleTypeDef* radio_uart_handle = NULL;

// --- Function Implementations ---


void radio_init(UART_HandleTypeDef* huart) {
    radio_uart_handle = huart;
    // You can add any other one-time setup here if needed in the future.
}

HAL_StatusTypeDef radio_send(uint8_t* data, uint16_t length) {
    // A "guard clause" to make sure the driver was initialized.
    if (radio_uart_handle == NULL) {
        return HAL_ERROR;
    }
    
    // This is the core of the driver.
    // We are calling the STM32 HAL function to start a UART transmission using DMA.
    // The CPU gives this command and is immediately free to do other things.
    // The DMA hardware handles the rest in the background.
    return HAL_UART_Transmit_DMA(radio_uart_handle, data, length);
}

// --- Interrupt and DMA Callbacks ---

/**
  * @brief  Tx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    // Check if the callback is from our radio's UART
    if (huart->Instance == radio_uart_handle->Instance) {
        // This function is called automatically by the HAL when the DMA transfer is finished.
        // For now, it can be empty. In the future, you could put logic here,
        // for example, to set a flag that a transmission has completed.
    }
}