/**
 * @file fifo_writer.c
 * @brief HPS to FPGA interface implementation
 */

#include "fifo_writer.h"
#include "config.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>


/* ======================================================
 * PRIVATE DEFINITIONS
 * ====================================================== */
#define PAGE_SIZE 4096 // Tipical size in Linuxa -> 0x1000
#define PAGE_MASK (PAGE_SIZE - 1) // 0xFFF


/* ======================================================
 * PUBLIC DEFINITIONS
 * ====================================================== */
int fifo_init(fifo_context_t *fifo, uint32_t physical_base, uint32_t size) {
    if (!fifo) {
        ERROR_PRINT("Invalid FIFO context pointer");
        return -1;
    }

    memset(fifo, 0, sizeof(fifo_context_t));

    /* Open /dev/mem for memory-mapped I/O */
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        ERROR_PRINT("Failed to open /dev/mem. Are you runnign as root?");
        return -1;
    }

    /* Align address to page boundary for mmap use */ // physical_base = 0xFF200030   // Real direction of FIFO.in
    uint32_t page_aligned_base = physical_base & ~PAGE_MASK; // ~PAGE_MASK = 0xFFFFF000 -> 0xFF200030 & 0xFFFFF000 = 0xFF200000
    uint32_t offset_in_page = physical_base & PAGE_MASK;     // PAGE_MASK  = 0x00000FFF -> 0xFF200030 & 0x00000FFF = 0x00000030

    /* Calculate mapping size (must be page-aligned) */
    uint32_t map_size = ((size + offset_in_page + PAGE_MASK) & ~PAGE_MASK);

    if (map_size < PAGE_SIZE) {
        map_size = PAGE_SIZE; /* Minimum one page */
    }

    void *mapped_base = mmap(NULL, 
                             map_size, 
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             fd,
                             page_aligned_base);

    close(fd); // Can be closed after mmap

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
    DEBUG_PRINT("   Test write successful");

    return 0;
}

void fifo_close(fifo_context_t *fifo) {
    if (fifo && fifo->base_addr) {
        munmap(fifo->base_addr, fifo->map_size);

        INFO_PRINT("FIFO closed. Total words written: %u", fifo->words_written);

        memset(fifo, 0, sizeof(fifo_context_t));
    }
}

int fifo_write_samples(fifo_context_t *fifo, const int16_t *samples, uint32_t count) {
    if (!fifo || !fifo->data_reg || !samples) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    for (uint32_t i = 0; i < count; i++) {
        *fifo->data_reg = (uint32_t)(int32_t)samples[i];
        fifo->words_written++;
    }

    return count;
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

