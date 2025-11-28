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

// TODO: Adjust according to design in Platform Designer. The following is just a place holder
/**
 * @brief FIFO register offsets
 */
typedef struct {
    volatile uint32_t data;       /* Offset 0x00: Data register */
    volatile uint32_t fill_level; /* Offset 0x04: Fill level (words used) */
    volatile uint32_t status;     /* Offset 0x08: Status register */
    volatile uint32_t control;    /* Offset 0x0C: Control register */
} fifo_regs_t;


/* Status register bits */
#define FIFO_STATUS_FULL (1 << 0)

/**
 * @brief FIFO conext structure
 */
typedef struct {
    volatile fifo_regs_t *regs; /* Memory-mapped register */
    void *base_addr;            /* Mapped base address */
    uint32_t fifo_size;         /* FIFO size in words */
    uint32_t words_written;     /* Total words written (stats) */
    uint32_t overflow_count;    /* Overflow events counter */
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
int fifo_write_sample(fifo_context_t *fifo, int16_t sample);

uint32_t fifo_get_free_space(fifo_context_t *fifo);




#endif /* FIFO_WRITER_H */

