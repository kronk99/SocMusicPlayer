/**
 * @file tone_generator.c
 * @brief Generador de tonos y barridos para pruebas de audio
 *
 * Genera archivos WAV con diferentes señales para testing:
 * - Tonos puros (sine waves)
 * - Barridos lineales (linear chirp)
 * - Barridos logarítmicos (log chirp)
 * - Ruido blanco
 * - Ondas cuadradas, triangulares, diente de sierra
 *
 * Compilar: gcc tone_generator.c -o tone_generator -lm
 * Uso: ./tone_generator
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
    char riff_tag[4];           // "RIFF"
    uint32_t riff_length;       // File size - 8
    char wave_tag[4];           // "WAVE"
    char fmt_tag[4];            // "fmt "
    uint32_t fmt_length;        // 16 for PCM
    uint16_t audio_format;      // 1 = PCM
    uint16_t num_channels;      // 1 = Mono, 2 = Stereo
    uint32_t sample_rate;       // 48000 Hz
    uint32_t byte_rate;         // sample_rate * num_channels * bits_per_sample / 8
    uint16_t block_align;       // num_channels * bits_per_sample / 8
    uint16_t bits_per_sample;   // 16
    char data_tag[4];           // "data"
    uint32_t data_length;       // Audio data size
} __attribute__((packed)) wav_header_t;

#define SAMPLE_RATE 48000
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 1
#define MAX_AMPLITUDE 32000  // Dejar margen para evitar clipping

/**
 * @brief Write WAV header
 */
void write_wav_header(FILE *fp, uint32_t num_samples) {
    wav_header_t header;

    memcpy(header.riff_tag, "RIFF", 4);
    memcpy(header.wave_tag, "WAVE", 4);
    memcpy(header.fmt_tag, "fmt ", 4);
    memcpy(header.data_tag, "data", 4);

    header.fmt_length = 16;
    header.audio_format = 1; // PCM
    header.num_channels = NUM_CHANNELS;
    header.sample_rate = SAMPLE_RATE;
    header.bits_per_sample = BITS_PER_SAMPLE;
    header.byte_rate = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    header.block_align = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    header.data_length = num_samples * header.block_align;
    header.riff_length = 36 + header.data_length;

    fwrite(&header, sizeof(wav_header_t), 1, fp);
}

/**
 * @brief Generate pure sine tone
 */
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

/**
 * @brief Generate linear frequency sweep (chirp)
 */
void generate_linear_sweep(const char *filename, float f_start, float f_end, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    float chirp_rate = (f_end - f_start) / duration_sec;

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        // Instantaneous frequency: f(t) = f_start + chirp_rate * t
        // Phase: phi(t) = 2*pi * (f_start*t + 0.5*chirp_rate*t^2)
        float phase = 2.0f * M_PI * (f_start * t + 0.5f * chirp_rate * t * t);
        float sample = MAX_AMPLITUDE * sinf(phase);
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (%.1f Hz → %.1f Hz, %.1f sec)\n",
           filename, f_start, f_end, duration_sec);
}

/**
 * @brief Generate logarithmic frequency sweep
 */
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
        // Instantaneous frequency: f(t) = f_start * k^t
        float freq = f_start * powf(k, t);
        // Phase integral
        float phase = 2.0f * M_PI * f_start * (powf(k, t) - 1.0f) / logf(k);
        float sample = MAX_AMPLITUDE * sinf(phase);
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (%.1f Hz → %.1f Hz LOG, %.1f sec)\n",
           filename, f_start, f_end, duration_sec);
}

/**
 * @brief Generate square wave
 */
void generate_square_wave(const char *filename, float frequency, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float phase = fmodf(frequency * t, 1.0f);
        float sample = (phase < 0.5f) ? MAX_AMPLITUDE : -MAX_AMPLITUDE;
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (Square %.1f Hz, %.1f sec)\n", filename, frequency, duration_sec);
}

/**
 * @brief Generate triangle wave
 */
void generate_triangle_wave(const char *filename, float frequency, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float phase = fmodf(frequency * t, 1.0f);
        // Triangle: 4*|phase - 0.5| - 1
        float sample = MAX_AMPLITUDE * (4.0f * fabsf(phase - 0.5f) - 1.0f);
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (Triangle %.1f Hz, %.1f sec)\n", filename, frequency, duration_sec);
}

/**
 * @brief Generate sawtooth wave
 */
void generate_sawtooth_wave(const char *filename, float frequency, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    for (uint32_t i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float phase = fmodf(frequency * t, 1.0f);
        // Sawtooth: 2*phase - 1
        float sample = MAX_AMPLITUDE * (2.0f * phase - 1.0f);
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (Sawtooth %.1f Hz, %.1f sec)\n", filename, frequency, duration_sec);
}

/**
 * @brief Generate white noise
 */
void generate_white_noise(const char *filename, float duration_sec) {
    uint32_t num_samples = (uint32_t)(SAMPLE_RATE * duration_sec);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return;
    }

    write_wav_header(fp, num_samples);

    for (uint32_t i = 0; i < num_samples; i++) {
        // Random value between -1 and 1
        float random = 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
        float sample = MAX_AMPLITUDE * random;
        int16_t sample_int = (int16_t)sample;
        fwrite(&sample_int, sizeof(int16_t), 1, fp);
    }

    fclose(fp);
    printf("Generated: %s (White Noise, %.1f sec)\n", filename, duration_sec);
}

/**
 * @brief Generate multi-tone signal (sum of multiple frequencies)
 */
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

        // Sum all frequencies
        for (int j = 0; j < num_freqs; j++) {
            sample += sinf(2.0f * M_PI * frequencies[j] * t);
        }

        // Normalize by number of tones
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

/**
 * @brief Main function
 */
int main(void) {
    printf("========================================\n");
    printf("Generador de Tonos de Prueba - 48kHz 16-bit Mono\n");
    printf("========================================\n\n");

    // 1. Tonos puros de referencia
    printf("1. Tonos puros de referencia:\n");
    generate_sine_tone("tone_100hz.wav", 100.0f, 5.0f);
    generate_sine_tone("tone_440hz.wav", 440.0f, 5.0f);
    generate_sine_tone("tone_1khz.wav", 1000.0f, 5.0f);
    generate_sine_tone("tone_5khz.wav", 5000.0f, 5.0f);
    generate_sine_tone("tone_10khz.wav", 10000.0f, 5.0f);
    generate_sine_tone("tone_15khz.wav", 15000.0f, 5.0f);
    printf("\n");

    // 2. Barridos lineales (sweeps)
    printf("2. Barridos lineales:\n");
    generate_linear_sweep("sweep_20hz_20khz_10s.wav", 20.0f, 20000.0f, 10.0f);
    generate_linear_sweep("sweep_100hz_10khz_5s.wav", 100.0f, 10000.0f, 5.0f);
    generate_linear_sweep("sweep_20hz_1khz_5s.wav", 20.0f, 1000.0f, 5.0f);
    generate_linear_sweep("sweep_1khz_20khz_5s.wav", 1000.0f, 20000.0f, 5.0f);
    printf("\n");

    // 3. Barridos logarítmicos (mejor para análisis de frecuencia)
    printf("3. Barridos logarítmicos:\n");
    generate_log_sweep("sweep_log_20hz_20khz_10s.wav", 20.0f, 20000.0f, 10.0f);
    generate_log_sweep("sweep_log_100hz_10khz_5s.wav", 100.0f, 10000.0f, 5.0f);
    printf("\n");

    // 4. Ondas no-sinusoidales
    printf("4. Formas de onda (waveforms):\n");
    generate_square_wave("square_440hz.wav", 440.0f, 3.0f);
    generate_triangle_wave("triangle_440hz.wav", 440.0f, 3.0f);
    generate_sawtooth_wave("sawtooth_440hz.wav", 440.0f, 3.0f);
    printf("\n");

    // 5. Ruido blanco
    printf("5. Ruido:\n");
    generate_white_noise("white_noise_5s.wav", 5.0f);
    printf("\n");

    // 6. Multi-tonos (para probar intermodulación)
    printf("6. Multi-tonos:\n");
    float freqs_low[] = {100.0f, 200.0f, 300.0f};
    generate_multitone("multitone_low.wav", freqs_low, 3, 5.0f);

    float freqs_mid[] = {440.0f, 880.0f, 1320.0f};
    generate_multitone("multitone_mid.wav", freqs_mid, 3, 5.0f);

    float freqs_high[] = {5000.0f, 7500.0f, 10000.0f};
    generate_multitone("multitone_high.wav", freqs_high, 3, 5.0f);
    printf("\n");

    printf("========================================\n");
    printf("Generación completa!\n");
    printf("========================================\n");
    printf("\nUso recomendado:\n");
    printf("  - sweep_*.wav: Para análisis de respuesta en frecuencia\n");
    printf("  - tone_*.wav: Para calibración y pruebas básicas\n");
    printf("  - square/triangle/sawtooth: Para probar armónicos\n");
    printf("  - white_noise: Para probar ruido de fondo\n");
    printf("  - multitone: Para probar distorsión e intermodulación\n");
    printf("\nCopiar a la board:\n");
    printf("  scp *.wav root@<board-ip>:/home/root/sounds/\n");
    printf("\n");

    return 0;
}
