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

// TODO: Adjust according to design in Platform Designer

/**
 * @brief FIFO register offsets
 */
typedef struct {
    volatile uint32_t data;       /* Offset 0x00: Data register */
    volatile uint32_t fill_level; /* Offset 0x04: Fill level (words used) */
    volatile uint32_t status;     /* Offset 0x08: Status register */
    volatile uint32_t control;    /* Offset 0x0C: Control register */
} fifo_regs_t;

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




#endif /* FIFO_WRITER_H */

