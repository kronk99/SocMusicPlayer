/**
 * @file fifo_writer.h
 * @brief HPS to FPGA FIFO interface
 *
 * Módulo para escribir muestras de audio al FIFO en FPGA.
 * Usa memory-mapped I/O a través del puente HPS-FPGA.
 */

#ifndef FIFO_WRITER_H
#define FIFO_WRITER_H

#include <stdint.h>

/* ============================================================================
 * FIFO CONTROL REGISTERS - Avalon-MM FIFO (Altera/Intel)
 * ============================================================================
 * Basado en: altera_avalon_fifo IP core (Quartus 18.1)
 * Configuración: AVALONMM_WRITE (write side para ARM)
 *
 * El FIFO Avalon-MM expone solo UN registro para escritura:
 * - Offset 0x00: Data register (write-only para ARM)
 *
 * NOTA: Fill level, status, etc. están en el CSR (in_csr) pero ese
 * registro está mapeado para NIOS, NO para ARM en este diseño.
 * ARM simplemente escribe datos al FIFO.
 * ============================================================================ */

/**
 * @brief FIFO write interface (ARM side - simplified)
 *
 * Para el lado de escritura (ARM), el FIFO Avalon-MM solo expone:
 * - Un registro de datos donde escribir (offset 0x00)
 *
 * El hardware maneja automáticamente:
 * - Backpressure (ARM debe escribir y confiar que no hay overflow)
 * - Flow control interno
 */
typedef struct {
    volatile uint32_t data;         /* Offset 0x00: Write data here */
} fifo_write_regs_t;

/* Para simplificar, ARM escribe directo a la dirección base */
/* No hay status bits accesibles desde ARM en configuración AVALONMM_WRITE */

/**
 * @brief FIFO context structure
 */
typedef struct {
    volatile uint32_t *data_reg;    /* Pointer to FIFO data register */
    void *base_addr;                /* Mapped base address (for munmap) */
    uint32_t map_size;              /* Size of mapped region */
    uint32_t fifo_depth;            /* FIFO depth in words (from Platform Designer) */
    uint32_t words_written;         /* Total words written (stats) */
} fifo_context_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize FIFO interface
 *
 * Maps FIFO registers into process memory space using /dev/mem.
 *
 * @param fifo Pointer to fifo_context_t structure
 * @param physical_base Physical address of FIFO in FPGA (from Platform Designer)
 * @param size FIFO size in bytes
 * @return 0 on success, -1 on error
 */
int fifo_init(fifo_context_t *fifo, uint32_t physical_base, uint32_t size);

/**
 * @brief Close FIFO interface
 *
 * Unmaps memory and cleans up resources.
 *
 * @param fifo Pointer to fifo_context_t structure
 */
void fifo_close(fifo_context_t *fifo);

/**
 * @brief Write a single audio sample to FIFO
 *
 * @param fifo Pointer to fifo_context_t structure
 * @param sample 16-bit audio sample
 * @return 0 on success, -1 if FIFO full
 */
int fifo_write_sample(fifo_context_t *fifo, int16_t sample);

/**
 * @brief Write multiple audio samples to FIFO
 *
 * Writes as many samples as possible without blocking.
 *
 * @param fifo Pointer to fifo_context_t structure
 * @param samples Buffer of 16-bit audio samples
 * @param count Number of samples to write
 * @return Number of samples actually written
 */
int fifo_write_samples(fifo_context_t *fifo, const int16_t *samples, uint32_t count);

/**
 * @brief Get FIFO fill level
 *
 * NOTA: NO DISPONIBLE desde ARM en configuración AVALONMM_WRITE.
 * Los registros de status están mapeados para NIOS (in_csr @ 0xFF210020).
 * Esta función retorna 0 (placeholder para compatibilidad).
 *
 * @param fifo Pointer to fifo_context_t structure
 * @return 0 (no accesible desde ARM)
 */
uint32_t fifo_get_fill_level(fifo_context_t *fifo);

/**
 * @brief Check if FIFO is full
 *
 * NOTA: NO DISPONIBLE desde ARM. Hardware maneja backpressure automáticamente.
 * Esta función siempre retorna 0 (assume FIFO never full).
 *
 * @param fifo Pointer to fifo_context_t structure
 * @return 0 (no accesible desde ARM)
 */
int fifo_is_full(fifo_context_t *fifo);

/**
 * @brief Check if FIFO is empty
 *
 * NOTA: NO DISPONIBLE desde ARM.
 * Esta función siempre retorna 0 (placeholder).
 *
 * @param fifo Pointer to fifo_context_t structure
 * @return 0 (no accesible desde ARM)
 */
int fifo_is_empty(fifo_context_t *fifo);

/**
 * @brief Check for FIFO overflow
 *
 * NOTA: NO DISPONIBLE desde ARM.
 * El hardware FIFO tiene backpressure, no debería haber overflow si
 * ARM no escribe demasiado rápido.
 *
 * @param fifo Pointer to fifo_context_t structure
 * @return 0 (no accesible desde ARM)
 */
int fifo_check_overflow(fifo_context_t *fifo);

/**
 * @brief Reset FIFO
 *
 * NOTA: NO DISPONIBLE desde ARM en configuración AVALONMM_WRITE.
 * No hay registro de control accesible.
 *
 * @param fifo Pointer to fifo_context_t structure
 * @return -1 (not supported)
 */
int fifo_reset(fifo_context_t *fifo);

/**
 * @brief Get available space in FIFO
 *
 * NOTA: NO DISPONIBLE desde ARM.
 * Retorna una estimación basada en depth configurado.
 *
 * @param fifo Pointer to fifo_context_t structure
 * @return Estimación de espacio (fifo_depth)
 */
uint32_t fifo_get_free_space(fifo_context_t *fifo);

/**
 * @brief Print FIFO statistics (debug)
 *
 * @param fifo Pointer to fifo_context_t structure
 */
void fifo_print_stats(fifo_context_t *fifo);

#endif /* FIFO_WRITER_H */
