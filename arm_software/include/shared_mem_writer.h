/**
 * @file shared_mem_writer.h
 * @brief Shared Memory Writer - ARM to NIOS communication via SDRAM
 *
 * Usa memoria compartida (SDRAM) en lugar de FIFO para comunicación ARM→NIOS.
 * Mucho más simple y sin problemas de timing.
 */

#ifndef SHARED_MEM_WRITER_H
#define SHARED_MEM_WRITER_H

#include <stdint.h>

/* ============================================================================
 * SHARED MEMORY LAYOUT
 * ============================================================================
 *
 * Base física ARM:  0xC0100000 (SDRAM + 1MB offset)
 * Base física NIOS: 0x00100000 (mismo offset desde perspectiva NIOS)
 *
 * Estructura:
 *   Offset 0x00: write_index (uint32_t) - ARM actualiza
 *   Offset 0x04: read_index (uint32_t)  - NIOS actualiza
 *   Offset 0x08: buffer_size (uint32_t) - Fijo: 65536 samples
 *   Offset 0x0C: flags (uint32_t)       - Control flags
 *   Offset 0x10: Reserved (padding)
 *   Offset 0x20: audio_samples[65536]   - Circular buffer (256KB)
 */

#define SHARED_MEM_OFFSET_WRITE_IDX     0x00
#define SHARED_MEM_OFFSET_READ_IDX      0x04
#define SHARED_MEM_OFFSET_BUFFER_SIZE   0x08
#define SHARED_MEM_OFFSET_FLAGS         0x0C
#define SHARED_MEM_OFFSET_SAMPLES       0x20

#define SHARED_MEM_BUFFER_SIZE          65536  /* 64K samples (256KB) */
#define SHARED_MEM_TOTAL_SIZE           (0x20 + (SHARED_MEM_BUFFER_SIZE * sizeof(int32_t)))

/* Flags */
#define SHMEM_FLAG_NIOS_READY           (1 << 0)  /* NIOS listo para recibir */
#define SHMEM_FLAG_PLAYING              (1 << 1)
#define SHMEM_FLAG_RESET                (1 << 2)

/**
 * @brief Shared memory context
 */
typedef struct {
    volatile uint32_t *write_index;     /* Pointer to write index */
    volatile uint32_t *read_index;      /* Pointer to read index */
    volatile uint32_t *buffer_size;     /* Pointer to buffer size */
    volatile uint32_t *flags;           /* Pointer to flags */
    volatile int32_t *samples;          /* Pointer to sample buffer */
    void *base_addr;                    /* Mapped base address */
    uint32_t map_size;                  /* Size of mapped region */
    uint32_t samples_written;           /* Statistics */
} shared_mem_context_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize shared memory for audio streaming
 *
 * @param shmem Pointer to shared_mem_context_t structure
 * @param physical_base Physical address of shared memory (ARM view)
 * @return 0 on success, -1 on error
 */
int shared_mem_init(shared_mem_context_t *shmem, uint32_t physical_base);

/**
 * @brief Close shared memory
 *
 * @param shmem Pointer to shared_mem_context_t structure
 */
void shared_mem_close(shared_mem_context_t *shmem);

/**
 * @brief Write samples to shared memory circular buffer
 *
 * @param shmem Pointer to shared_mem_context_t structure
 * @param samples Buffer of samples to write
 * @param count Number of samples
 * @return Number of samples written (may be less if buffer full)
 */
int shared_mem_write_samples(shared_mem_context_t *shmem, const int16_t *samples, uint32_t count);

/**
 * @brief Get available space in buffer
 *
 * @param shmem Pointer to shared_mem_context_t structure
 * @return Number of samples that can be written
 */
uint32_t shared_mem_get_free_space(shared_mem_context_t *shmem);

/**
 * @brief Get number of samples available to read
 *
 * @param shmem Pointer to shared_mem_context_t structure
 * @return Number of samples available
 */
uint32_t shared_mem_get_available(shared_mem_context_t *shmem);

/**
 * @brief Print shared memory statistics
 *
 * @param shmem Pointer to shared_mem_context_t structure
 */
void shared_mem_print_stats(shared_mem_context_t *shmem);

#endif /* SHARED_MEM_WRITER_H */
