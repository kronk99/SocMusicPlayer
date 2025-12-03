/**
 * @file tone_generator_2k.c
 * @brief Generador de tonos de prueba LIMITADO a 2 kHz
 *
 * Versión ultra-conservadora para sistemas con bandwidth muy limitado
 * Sweeps logarítmicos LENTOS para evitar artefactos
 * Rango: 100 Hz - 2000 Hz
 *
 * Compilar: gcc tone_generator_2k.c -o tone_generator_2k -lm
 * Uso: ./tone_generator_2k
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* WAV file header structure */
typedef struct {
    char riff_tag[4];
    uint32_t riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    uint32_t fmt_length;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_tag[4];
    uint32_t data_length;
} __attribute__((packed)) wav_header_t;

#define SAMPLE_RATE 48000
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 1
#define MAX_AMPLITUDE 32000

void write_wav_header(FILE *fp, uint32_t num_samples) {
    wav_header_t header;

    memcpy(header.riff_tag, "RIFF", 4);
    memcpy(header.wave_tag, "WAVE", 4);
    memcpy(header.fmt_tag, "fmt ", 4);
    memcpy(header.data_tag, "data", 4);

    header.fmt_length = 16;
    header.audio_format = 1;
    header.num_channels = NUM_CHANNELS;
    header.sample_rate = SAMPLE_RATE;
    header.bits_per_sample = BITS_PER_SAMPLE;
    header.byte_rate = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    header.block_align = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    header.data_length = num_samples * header.block_align;
    header.riff_length = 36 + header.data_length;

    fwrite(&header, sizeof(wav_header_t), 1, fp);
}

void generate_sine_tone(const char *filename, float frequency, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float sample = MAX_AMPLITUDE * sinf(2.0f * M_PI * frequency * t);
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (%.1f Hz, %.1f sec)\n", filename, frequency, duration_sec);
}

void generate_log_sweep(const char *filename, float f_start, float f_end, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    float k = powf(f_end / f_start, 1.0f / duration_sec);

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float phase = 2.0f * M_PI * f_start * (powf(k, t) - 1.0f) / logf(k);
        float sample = MAX_AMPLITUDE * sinf(phase);
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (%.1f Hz → %.1f Hz LOG, %.1f sec)\n",
           filename, f_start, f_end, duration_sec);
}

void generate_multitone(const char *filename, float *frequencies, int num_freqs, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float sample = 0.0f;

        for (int j = 0; j < num_freqs; j++) {
            sample += sinf(2.0f * M_PI * frequencies[j] * t);
        }

        sample = (MAX_AMPLITUDE / num_freqs) * sample;
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (Multi-tone: ", filename);
    for (int j = 0; j < num_freqs; j++) {
        printf("%.1f%s", frequencies[j], (j < num_freqs - 1) ? "+" : " Hz");
    }
    printf(", %.1f sec)\n", duration_sec);
}

int main(void) {
    printf("========================================\n");
    printf("Generador de Tonos - LIMITADO a 2 kHz\n");
    printf("48kHz 16-bit Mono (Ultra-conservador)\n");
    printf("========================================\n\n");

    /* 1. Tonos puros de referencia (hasta 2 kHz) */
    printf("1. Tonos puros (100 Hz - 2 kHz):\n");
    generate_sine_tone("tone_100hz_2k.wav", 100.0f, 5.0f);
    generate_sine_tone("tone_440hz_2k.wav", 440.0f, 5.0f);
    generate_sine_tone("tone_1khz_2k.wav", 1000.0f, 5.0f);
    generate_sine_tone("tone_1500hz_2k.wav", 1500.0f, 5.0f);
    generate_sine_tone("tone_2khz_2k.wav", 2000.0f, 5.0f);
    printf("\n");

    /* 2. Barridos logarítmicos LENTOS (sin artefactos) */
    printf("2. Barridos logarítmicos LENTOS:\n");

    /* Ultra-lentos (30 segundos) */
    generate_log_sweep("sweep_log_100hz_2khz_30s.wav", 100.0f, 2000.0f, 30.0f);
    generate_log_sweep("sweep_log_100hz_1500hz_30s.wav", 100.0f, 1500.0f, 30.0f);

    /* Lentos (20 segundos) */
    generate_log_sweep("sweep_log_100hz_2khz_20s.wav", 100.0f, 2000.0f, 20.0f);
    generate_log_sweep("sweep_log_200hz_2khz_20s.wav", 200.0f, 2000.0f, 20.0f);

    /* Moderados (15 segundos) */
    generate_log_sweep("sweep_log_100hz_2khz_15s.wav", 100.0f, 2000.0f, 15.0f);
    generate_log_sweep("sweep_log_500hz_2khz_15s.wav", 500.0f, 2000.0f, 15.0f);

    /* Rápidos (10 segundos) - para comparación */
    generate_log_sweep("sweep_log_100hz_2khz_10s.wav", 100.0f, 2000.0f, 10.0f);
    printf("\n");

    /* 3. Multi-tonos */
    printf("3. Multi-tonos (hasta 2 kHz):\n");
    float freqs_low[] = {100.0f, 200.0f, 300.0f};
    generate_multitone("multitone_low_2k.wav", freqs_low, 3, 5.0f);

    float freqs_mid[] = {440.0f, 880.0f, 1320.0f};
    generate_multitone("multitone_mid_2k.wav", freqs_mid, 3, 5.0f);

    float freqs_high[] = {1000.0f, 1500.0f, 2000.0f};
    generate_multitone("multitone_high_2k.wav", freqs_high, 3, 5.0f);
    printf("\n");

    /* 4. Escalera de frecuencias (pasos discretos) */
    printf("4. Escalera de frecuencias:\n");
    FILE *fp = fopen("frequency_steps_2k.wav", "wb");
    uint32_t total_samples = SAMPLE_RATE * 20; // 20 segundos
    write_wav_header(fp, total_samples);

    float frequencies[] = {100, 200, 400, 600, 800, 1000, 1200, 1500, 1800, 2000};
    int num_steps = 10;
    uint32_t samples_per_step = total_samples / num_steps;

    for (int step = 0; step < num_steps; step++) {
        float freq = frequencies[step];
        for (uint32_t i = 0; i < samples_per_step; i++) {
            float t = (float)i / SAMPLE_RATE;
            float sample = MAX_AMPLITUDE * sinf(2.0f * M_PI * freq * t);
            int16_t sample_int = (int16_t)sample;
            fwrite(&sample_int, sizeof(int16_t), 1, fp);
        }
    }
    fclose(fp);
    printf("Generated: frequency_steps_2k.wav (100-2000 Hz en 10 pasos)\n");
    printf("\n");

    printf("========================================\n");
    printf("Generación completa!\n");
    printf("========================================\n");
    printf("\nTodos los archivos limitados a 100 Hz - 2000 Hz\n");
    printf("Sweeps logarítmicos LENTOS para evitar artefactos\n\n");
    printf("Archivos recomendados para prueba:\n");
    printf("  sweep_log_100hz_2khz_30s.wav   - Más lento (sin artefactos)\n");
    printf("  sweep_log_100hz_2khz_20s.wav   - Lento\n");
    printf("  sweep_log_100hz_2khz_15s.wav   - Moderado\n");
    printf("  frequency_steps_2k.wav         - Pasos discretos\n");
    printf("\n");

    return 0;
}
