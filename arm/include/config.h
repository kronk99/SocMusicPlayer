/**
 * @file config.h
 * @brief System configuration and hardware definitions
 *
 * Config for Soc Audio Player in Altera DE-SoC1
 */

#ifndef CONFIG_H
#define CONFIG_H


// TODO: Hardware memory map





/* Some macros */
#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define INFO_PRINT(fmt, ...) \
    printf("[INFO] " fmt "\n", ##__VA_ARGS__)



#endif /* CONFIG_H */

