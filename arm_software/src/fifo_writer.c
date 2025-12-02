/**
 * @file fifo_writer.c
 * @brief HPS to FPGA FIFO interface implementation
 *
 * Implementación simplificada para Avalon-MM FIFO (AVALONMM_WRITE)
 * ARM solo escribe datos - no tiene acceso a registros de status
 */

#include "fifo_writer.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================ */

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

int fifo_init(fifo_context_t *fifo, uint32_t physical_base, uint32_t size) {
    if (!fifo) {
        ERROR_PRINT("Invalid FIFO context pointer");
        return -1;
    }

    memset(fifo, 0, sizeof(fifo_context_t));

    /* Open /dev/mem for memory-mapped I/O */
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        ERROR_PRINT("Failed to open /dev/mem. Are you running as root?");
        return -1;
    }

    /* Align address to page boundary */
    uint32_t page_aligned_base = physical_base & ~PAGE_MASK;
    uint32_t offset_in_page = physical_base & PAGE_MASK;

    /* Calculate mapping size (must be page-aligned) */
    uint32_t map_size = ((size + offset_in_page + PAGE_MASK) & ~PAGE_MASK);
    if (map_size < PAGE_SIZE) {
        map_size = PAGE_SIZE; /* Minimum one page */
    }

    /* Memory map the FIFO write register */
    void *mapped_base = mmap(NULL,
                             map_size,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             fd,
                             page_aligned_base);

    close(fd); /* Can close fd after mmap */

    if (mapped_base == MAP_FAILED) {
        ERROR_PRINT("Failed to mmap FIFO at 0x%08X", physical_base);
        perror("mmap");
        return -1;
    }

    /* Calculate actual data register pointer */
    fifo->base_addr = mapped_base;
    fifo->map_size = map_size;
    fifo->data_reg = (volatile uint32_t *)((uint8_t *)mapped_base + offset_in_page);
    fifo->fifo_depth = size / sizeof(uint32_t); /* Depth in words */
    fifo->words_written = 0;

    INFO_PRINT("FIFO initialized at physical address 0x%08X", physical_base);
    DEBUG_PRINT("  Mapped base: %p", mapped_base);
    DEBUG_PRINT("  Data register: %p", fifo->data_reg);
    DEBUG_PRINT("  FIFO depth: %u words", fifo->fifo_depth);
    DEBUG_PRINT("  Map size: %u bytes", map_size);

    /* Test write (write a zero, harmless) */
    *fifo->data_reg = 0;
    DEBUG_PRINT("  Test write successful");

    return 0;
}

void fifo_close(fifo_context_t *fifo) {
    if (fifo && fifo->base_addr) {
        munmap(fifo->base_addr, fifo->map_size);

        INFO_PRINT("FIFO closed. Total words written: %u", fifo->words_written);

        memset(fifo, 0, sizeof(fifo_context_t));
    }
}

int fifo_write_sample(fifo_context_t *fifo, int16_t sample) {
    if (!fifo || !fifo->data_reg) {
        ERROR_PRINT("Invalid FIFO context");
        return -1;
    }

    /* Write sample to FIFO data register */
    /* Avalon FIFO expects 32-bit writes, extend 16-bit sample to 32-bit */
    *fifo->data_reg = (uint32_t)(int32_t)sample; /* Sign-extend */
    fifo->words_written++;

    return 0;
}

int fifo_write_samples(fifo_context_t *fifo, const int16_t *samples, uint32_t count) {
    if (!fifo || !fifo->data_reg || !samples) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    /* Write all samples - hardware handles backpressure */
    for (uint32_t i = 0; i < count; i++) {
        *fifo->data_reg = (uint32_t)(int32_t)samples[i];
        fifo->words_written++;
    }

    return count; /* All written (hardware buffers if needed) */
}

/* ============================================================================
 * STATUS FUNCTIONS - NOT AVAILABLE FROM ARM
 * ============================================================================
 * These functions are stubs because ARM side (AVALONMM_WRITE) does not
 * have access to status registers. Status is only available from NIOS
 * side via in_csr register at 0xFF210020.
 * ============================================================================ */

uint32_t fifo_get_fill_level(fifo_context_t *fifo) {
    (void)fifo; /* Unused */
    /* Not accessible from ARM - return 0 */
    return 0;
}

int fifo_is_full(fifo_context_t *fifo) {
    (void)fifo; /* Unused */
    /* Not accessible from ARM - hardware handles backpressure */
    return 0;
}

int fifo_is_empty(fifo_context_t *fifo) {
    (void)fifo; /* Unused */
    /* Not accessible from ARM */
    return 0;
}

int fifo_check_overflow(fifo_context_t *fifo) {
    (void)fifo; /* Unused */
    /* Not accessible from ARM - hardware prevents overflow with backpressure */
    return 0;
}

int fifo_reset(fifo_context_t *fifo) {
    (void)fifo; /* Unused */
    /* Not supported from ARM side */
    ERROR_PRINT("FIFO reset not available from ARM side");
    return -1;
}

uint32_t fifo_get_free_space(fifo_context_t *fifo) {
    if (!fifo) {
        return 0;
    }
    /* Return depth as estimation - actual free space not readable from ARM */
    return fifo->fifo_depth;
}

void fifo_print_stats(fifo_context_t *fifo) {
    if (!fifo || !fifo->data_reg) {
        return;
    }

    printf("========================================\n");
    printf("FIFO Statistics (ARM Side)\n");
    printf("========================================\n");
    printf("FIFO Depth:      %u words (configured)\n", fifo->fifo_depth);
    printf("Data Register:   %p\n", fifo->data_reg);
    printf("Total Written:   %u words\n", fifo->words_written);
    printf("========================================\n");
    printf("NOTE: Fill level and status not accessible from ARM.\n");
    printf("      Check NIOS side (in_csr @ 0xFF210020) for status.\n");
    printf("========================================\n");
}
