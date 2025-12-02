/**
 * @file shared_mem_writer.c
 * @brief Shared Memory Writer Implementation
 */

#include "shared_mem_writer.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

int shared_mem_init(shared_mem_context_t *shmem, uint32_t physical_base) {
    if (!shmem) {
        ERROR_PRINT("Invalid shared_mem context");
        return -1;
    }

    memset(shmem, 0, sizeof(shared_mem_context_t));

    /* Open /dev/mem with O_SYNC (unbuffered, forces direct writes) */
    int fd = open("/dev/mem", O_RDWR | O_SYNC | O_DSYNC);
    if (fd < 0) {
        ERROR_PRINT("Failed to open /dev/mem. Are you running as root?");
        return -1;
    }

    /* Align to page boundary */
    uint32_t page_aligned = physical_base & ~PAGE_MASK;
    uint32_t offset_in_page = physical_base & PAGE_MASK;

    /* Calculate map size (round up to pages) */
    uint32_t map_size = ((SHARED_MEM_TOTAL_SIZE + offset_in_page + PAGE_MASK) & ~PAGE_MASK);

    /* Memory map with MAP_SHARED and cache-coherent flags */
    /* MAP_SHARED ensures writes go through to device */
    /* MAP_LOCKED prevents swapping, improves real-time performance */
    /* O_SYNC on fd helps with cache coherency */
    void *mapped = mmap(NULL, map_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_NORESERVE | MAP_LOCKED,
                        fd, page_aligned);
    close(fd);

    if (mapped == MAP_FAILED) {
        ERROR_PRINT("Failed to mmap shared memory at 0x%08X", physical_base);
        perror("mmap");
        return -1;
    }

    /* CRITICAL: Configure memory for cache coherency with NIOS */
    /* MADV_DONTFORK: Don't copy this mapping on fork() */
    /* MADV_DONTDUMP: Don't include in core dumps */
    if (madvise(mapped, map_size, MADV_DONTFORK | MADV_DONTDUMP) != 0) {
        perror("madvise warning (non-fatal)");
        /* Not fatal - continue anyway */
    }

    /* Setup pointers */
    uint8_t *base = (uint8_t *)mapped + offset_in_page;
    shmem->base_addr = mapped;
    shmem->map_size = map_size;
    shmem->write_index = (volatile uint32_t *)(base + SHARED_MEM_OFFSET_WRITE_IDX);
    shmem->read_index = (volatile uint32_t *)(base + SHARED_MEM_OFFSET_READ_IDX);
    shmem->buffer_size = (volatile uint32_t *)(base + SHARED_MEM_OFFSET_BUFFER_SIZE);
    shmem->flags = (volatile uint32_t *)(base + SHARED_MEM_OFFSET_FLAGS);
    shmem->samples = (volatile int32_t *)(base + SHARED_MEM_OFFSET_SAMPLES);
    shmem->samples_written = 0;

    /* Initialize shared memory */
    *shmem->write_index = 0;
    *shmem->read_index = 0;
    *shmem->buffer_size = SHARED_MEM_BUFFER_SIZE;
    *shmem->flags = 0;

    /* Clear buffer */
    memset((void *)shmem->samples, 0, SHARED_MEM_BUFFER_SIZE * sizeof(int32_t));

    /* CRITICAL: Flush all initialization to physical memory */
    /* Without this, NIOS may see uninitialized/cached values */
    __sync_synchronize();  /* Memory barrier */
    msync(shmem->base_addr, shmem->map_size, MS_SYNC);  /* Flush entire region */

    INFO_PRINT("Shared memory initialized at 0x%08X", physical_base);
    DEBUG_PRINT("  Mapped base: %p", mapped);
    DEBUG_PRINT("  Buffer size: %u samples", SHARED_MEM_BUFFER_SIZE);
    DEBUG_PRINT("  Total size: %u bytes", SHARED_MEM_TOTAL_SIZE);
    DEBUG_PRINT("  Cache flushed to physical memory");

    return 0;
}

void shared_mem_close(shared_mem_context_t *shmem) {
    if (shmem && shmem->base_addr) {
        /* Clear playing flag */
        if (shmem->flags) {
            *shmem->flags = 0;
        }

        munmap(shmem->base_addr, shmem->map_size);
        INFO_PRINT("Shared memory closed. Total samples written: %u", shmem->samples_written);
        memset(shmem, 0, sizeof(shared_mem_context_t));
    }
}

int shared_mem_write_samples(shared_mem_context_t *shmem, const int16_t *samples, uint32_t count) {
    if (!shmem || !samples) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    /* CRÍTICO: Esperar a que NIOS esté listo antes de escribir */
    uint32_t flags = *shmem->flags;
    if (!(flags & SHMEM_FLAG_NIOS_READY)) {
        /* NIOS no está listo aún, no escribir */
        return 0;
    }

    uint32_t write_idx = *shmem->write_index;
    uint32_t read_idx = *shmem->read_index;
    uint32_t buf_size = *shmem->buffer_size;

    uint32_t written = 0;

    for (uint32_t i = 0; i < count; i++) {
        /* Calculate next write position */
        uint32_t next_write = (write_idx + 1) % buf_size;

        /* Check if buffer full (write would catch read) */
        if (next_write == read_idx) {
            /* Buffer full, stop writing */
            break;
        }

        /* Write sample (convert 16-bit to 32-bit) */
        shmem->samples[write_idx] = (int32_t)samples[i];

        /* Update write index */
        write_idx = next_write;
        written++;
    }

    /* Update shared write index */
    *shmem->write_index = write_idx;
    shmem->samples_written += written;

    /* CRITICAL: Force cache flush to physical memory SYNCHRONOUSLY */
    /* This ensures NIOS sees the updated data immediately */
    __sync_synchronize();  /* Memory barrier */

    /* CAMBIO CRÍTICO: MS_SYNC en CADA write para eliminar cache coherency issues */
    msync(shmem->base_addr, shmem->map_size, MS_SYNC);  /* Synchronous flush */

    return (int)written;
}

uint32_t shared_mem_get_free_space(shared_mem_context_t *shmem) {
    if (!shmem) {
        return 0;
    }

    /* Invalidate cache before reading (NIOS updates read_index) */
    __sync_synchronize();

    uint32_t write_idx = *shmem->write_index;
    uint32_t read_idx = *shmem->read_index;
    uint32_t buf_size = *shmem->buffer_size;

    if (write_idx >= read_idx) {
        return buf_size - (write_idx - read_idx) - 1;
    } else {
        return read_idx - write_idx - 1;
    }
}

uint32_t shared_mem_get_available(shared_mem_context_t *shmem) {
    if (!shmem) {
        return 0;
    }

    /* Invalidate cache before reading */
    __sync_synchronize();

    uint32_t write_idx = *shmem->write_index;
    uint32_t read_idx = *shmem->read_index;
    uint32_t buf_size = *shmem->buffer_size;

    if (write_idx >= read_idx) {
        return write_idx - read_idx;
    } else {
        return buf_size - read_idx + write_idx;
    }
}

void shared_mem_print_stats(shared_mem_context_t *shmem) {
    if (!shmem) {
        return;
    }

    printf("========================================\n");
    printf("Shared Memory Statistics (ARM)\n");
    printf("========================================\n");
    printf("Buffer Size:     %u samples\n", *shmem->buffer_size);
    printf("Write Index:     %u\n", *shmem->write_index);
    printf("Read Index:      %u\n", *shmem->read_index);
    printf("Available:       %u samples\n", shared_mem_get_available(shmem));
    printf("Free Space:      %u samples\n", shared_mem_get_free_space(shmem));
    printf("Total Written:   %u samples\n", shmem->samples_written);
    printf("========================================\n");
}
