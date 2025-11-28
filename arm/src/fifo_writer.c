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

    void *mapped_base = mmap(NULL, 
                             map_size, 
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             fd,
                             page_aligned_base);

    close(fd); // Can be closed after mmap

    if (mapped_base == MAP_FAILED) {
        ERROR_PRINT("Failed to mmap FIFO register at 0x%08X", physical_base);
        return -1;
    }

    /* Calculate actual register base */
    fifo->base_addr = mapped_base;
    fifo->regs = (volatile fifo_regs_t *)((uint8_t *)mapped_base + offset_in_page);
    fifo->fifo_size = size / sizeof(uint32_t); /* Size in words */
    fifo->words_written = 0;
    fifo->overflow_count = 0;

    INFO_PRINT("FIFO initialized at physical address 0x%08X", physical_base);
    DEBUG_PRINT("  Mapped base: %p", mapped_base);
    DEBUG_PRINT("  Register base: %p", fifo->regs);
    DEBUG_PRINT("  FIFO size: %u words", fifo->fifo_size);

    /* Test read status to verify mapping works */
    uint32_t status = fifo->regs->status;
    DEBUG_PRINT("  Initial status: 0x%08X", status);

    return 0;
}

void fifo_close(fifo_context_t *fifo) {
    if (fifo && fifo->base_addr) {
        /* "Reverse engineer" the original map size for munmap */
        uint32_t map_size = ((fifo->fifo_size * sizeof(uint32_t) + PAGE_MASK) & ~PAGE_MASK);
        munmap(fifo->base_addr, map_size);

        INFO_PRINT("FIFO closed. Total words written: %u, Overflows: %u",
                    fifo->words_written, fifo->overflow_count);

        memset(fifo, 0, sizeof(fifo_context_t));
    }
}

int fifo_write_samples(fifo_context_t *fifo, const int16_t *samples, uint32_t count) {
    if (!fifo || !fifo->regs || !samples) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    uint32_t written = 0;

    for (uint32_t i = 0; i < count; i++) {
        /* Check available space in FIFO */
        uint32_t free_space = fifo_get_free_space(fifo);
        if (free_space == 0) {
            DEBUG_PRINT("FIFO full after writing %u/%u samples", written, count);
            break;
        }

        /* Write sample */
        fifo->regs->data = (uint32_t)samples[i];
        fifo->words_written++;
        written++;
    }

    return 0;
}

uint32_t fifo_get_free_space(fifo_context_t *fifo) {
    if (!fifo || !fifo->regs) {
        return 0;
    }

    uint32_t fill_level = fifo->regs->fill_level;
    
    if (fill_level >= fifo->fifo_size) {
        return 0;
    }

    return fifo->fifo_size - fill_level;
}

