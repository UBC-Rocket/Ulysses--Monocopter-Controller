#include "SPI_queue.h"
#include "SPI_device_interactions.h"
#include "stm32u5xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @file SPI_queue.h
 * @brief SPI job queue with DMA chaining.
 *
 * Supports multiple interrupt-driven devices on a shared SPI bus.
 * Each job describes a DMA transfer (CS pin, Tx buffer, Rx buffer, length,
 * timestamp, transfer type, and completion callback). The queue ensures safe,
 * serialized DMA usage. Device-specific handling is done in job callbacks.
 *
 *  UBC Rocket, Benedikt Howard, Sept 29th, 2025
 */

/* -------------------------------------------------------------------------- */
/* Core job runner                                                            */
/* -------------------------------------------------------------------------- */

/**
 * @brief Start a new SPI job immediately.
 * @note Assumes bus is idle. DMA completion will chain the next job.
 */
static void start_job(spi_job_t *job, spi_job_queue_t *q) {
    q->spi_busy = true;
    CS_ASSERT(job->cs_port, job->cs_pin);

    switch (job->type) {
    case SPI_XFER_TX:
        HAL_SPI_Transmit_DMA(q->spi_bus, job->tx, job->len);
        break;
    case SPI_XFER_RX:
        HAL_SPI_Receive_DMA(q->spi_bus, (uint8_t*)q->spi_rx_staging, job->len);
        break;
    case SPI_XFER_TXRX:
    default:
        HAL_SPI_TransmitReceive_DMA(q->spi_bus, job->tx,
                                    (uint8_t*)q->spi_rx_staging,
                                    job->len);
        break;
    }
}

/**
 * @brief Submit an SPI job to the shared bus queue.
 *        Safe to call from both ISR and task contexts.
 *
 * @param job Pointer to job descriptor (copied into queue if not started immediately).
 * @param q Pointer to the queue that the job is for. describes enquing behaviour, and bus.
 * @return true  if the job was started immediately
 *         false if it was queued (will run later)
 */
bool spi_submit_job( spi_job_t job, spi_job_queue_t *q)
{
    bool started = false;

    uint32_t primask = __get_PRIMASK(); 
    __disable_irq(); //critical seciton to check and set busy
    if (!q->spi_busy) {
        q->spi_busy = true;
        q->current_job = job;
        __set_PRIMASK(primask);//end critical section
        start_job(&job, q);// safe to call outside crit sect
        started = true;
    } else {
        __set_PRIMASK(primask);
        // Copy into queue (lock-free, single-byte head/tail)
        uint8_t next_head = (q->head + 1U) % SPI_JOB_QUEUE_SIZE;
        if (next_head != q->tail) {
            q->jobs[q->head] = job;
            q->head = next_head;
        }
        // TODO: queue full → silently drop or set error flag if you we need to
    }

    return started;
}

/* -------------------------------------------------------------------------- */
/* DMA completion callbacks                                                   */
/* -------------------------------------------------------------------------- */

/**
 * @brief Common handler called at end of DMA transfer.
 */
static void spi_dma_complete_common(SPI_HandleTypeDef *hspi) {

    BaseType_t xWoken = pdFALSE;

    spi_job_queue_t *jobq;
    if (hspi == jobq_spi_1.spi_bus){
        jobq = &jobq_spi_1;
    }else if (hspi == jobq_spi_2.spi_bus){
            jobq = &jobq_spi_2;
    }else{
        return;
    }

    CS_DEASSERT(jobq->current_job.cs_port, jobq->current_job.cs_pin);

    if(jobq->current_job.done) {
        jobq->current_job.done(&jobq->current_job, (const uint8_t*)jobq->spi_rx_staging, jobq->current_job.done_arg);
    }

    spi_job_t next;
    UBaseType_t s = taskENTER_CRITICAL_FROM_ISR();
    if (dequeue_job(jobq, &next)) {
        jobq->current_job = next;
        jobq->spi_busy = true;
    } else {
        jobq->spi_busy = false;
    }

    if (jobq->spi_busy) {
       start_job(&jobq->current_job, jobq); 
    }

    taskEXIT_CRITICAL_FROM_ISR(s);
    portYIELD_FROM_ISR(xWoken);
}

/**
 * @brief HAL weak callback for TxRx DMA complete.
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    spi_dma_complete_common(hspi);
}

/**
 * @brief HAL weak callback for Tx-only DMA complete.
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    spi_dma_complete_common(hspi);
}

/**
 * @brief HAL weak callback for Rx-only DMA complete.
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    spi_dma_complete_common(hspi);
}
