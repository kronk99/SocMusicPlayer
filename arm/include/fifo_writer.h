/**
 * @file fifo_writer.h
 * @brief HPS to FPGA FIFO interface
 *
 * Module to write audio samples into FPGA's FIFO
 * I/O Memory-mapped through HPS-FPGA's bridge
 */

#ifndef FIFO_WRITER_H
#define FIFO_WRITER_H

#include <stdint.h>


/* Status register bits */
#define FIFO_STATUS_FULL (1 << 0)

/**
 * @brief FIFO conext structure
 */
typedef struct {
    volatile uint32_t *data_reg; /* Pointer to FIFO data register */
    void *base_addr;             /* Mapped base address */
    uint32_t map_size;           /* Size of mapped region */
    uint32_t fifo_depth;         /* FIFO depth in words (from Platform Designer) */
    uint32_t words_written;      /* Total words written (stats) */
} fifo_context_t;


/* ====================================
 * Public API
 * ==================================== */

/**
 * @brief Initialize FIFO interface
 * 
 * Maps FIFO register into process memory space using /dev/mem
 * 
 * @param fifo pointer to fifo_context_t structure
 * @param physical_base Physical address of FIFO in FPGA (from Platform Designer)
 * @param size FIFO size in bytes
 * @return 0 on sucess, -1 on erro
 */
int fifo_init(fifo_context_t *fifo, uint32_t physical_base, uint32_t size);

/**
 * @brief Close FIFO interface
 * 
 * Unmaps memory and cleans up resources
 * 
 * @params fifo Pointer to fifo_context_t structure
 */
void fifo_close(fifo_context_t *fifo);

/**
 * @brief 
 * 
 * 
 */
int fifo_write_samples(fifo_context_t *fifo, const int16_t *samples, uint32_t count);

#endif /* FIFO_WRITER_H */

