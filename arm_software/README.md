# SoC Audio Player - ARM Software

Software para el procesador ARM Cortex-A9 (HPS) de la plataforma Altera DE-SoC1 (Cyclone V).

Este es parte del **Proyecto 2: Diseño de SoC mínimo para reproductor de audio** del curso CE-1113 Sistemas Empotrados, TEC.

## 📋 Descripción

Este software corre en el ARM Cortex-A9 (Hard Processor System) y es responsable de:

- ✅ Leer archivos WAV desde SD card
- ✅ Enviar muestras de audio al FIFO en FPGA (comunicación HPS-FPGA)
- ✅ Manejar botones mediante interrupciones (play/pause, next, prev, stop)
- ✅ Actualizar displays de 7 segmentos con tiempo de reproducción (MM:SS)
- ✅ Servir interfaz web con información de canción actual
- ✅ Controlar playlist y flujo de reproducción

## 🏗️ Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────┐
│ SD Card (música en formato WAV 48kHz, 16-bit, mono)    │
│   ↓                                                      │
│ ARM Cortex-A9 (HPS) [ESTE SOFTWARE]                     │
│   • Lee archivos WAV                                     │
│   • Controla reproducción                                │
│   • Maneja interfaz de usuario                           │
│   • Escribe muestras a FIFO (memory-mapped I/O)         │
│   ↓                                                      │
│ FIFO en FPGA (8KB buffer)                                │
│   ↓                                                      │
│ NIOS II (soft processor en FPGA)                         │
│   • Lee del FIFO                                         │
│   • Aplica filtros de audio                              │
│   • Envía a Audio Codec                                  │
│   ↓                                                      │
│ Audio Codec WM8731 → Salida de audio                     │
└─────────────────────────────────────────────────────────┘
```

## 📁 Estructura del Proyecto

```
arm_software/
├── src/                      # Código fuente
│   ├── main.c                # Aplicación principal
│   ├── wav_reader.c          # Lector de archivos WAV
│   ├── fifo_writer.c         # Escritor a FIFO en FPGA
│   ├── audio_controller.c    # Controlador de audio
│   ├── button_handler.c      # Manejador de botones
│   ├── display_driver.c      # Driver para displays 7-seg
│   └── web_interface.c       # Servidor web embebido
├── include/                  # Headers
│   ├── config.h              # Configuración del sistema
│   ├── wav_reader.h
│   ├── fifo_writer.h
│   ├── audio_controller.h
│   ├── button_handler.h
│   ├── display_driver.h
│   └── web_interface.h
├── build/                    # Archivos compilados (generado)
├── scripts/                  # Scripts auxiliares
├── www/                      # Archivos web (si necesario)
├── Makefile                  # Build system
└── README.md                 # Este archivo
```

## 🛠️ Requisitos Previos

### Software

1. **Intel SoC EDS (Embedded Design Suite)**
   - Incluye toolchain ARM (`arm-linux-gnueabihf-gcc`)
   - Descarga: [Intel FPGA Software](https://www.intel.com/content/www/us/en/products/details/fpga/development-tools/quartus-prime.html)

2. **Linux precompilado para DE-SoC1**
   - Imagen de Linux para Cyclone V SoC
   - Incluido en SoC EDS o disponible en recursos de Altera/Terasic

3. **SSH client** para acceso remoto a la tarjeta

### Hardware

- Altera DE-SoC1 (Cyclone V SoC FPGA)
- Tarjeta SD con:
  - Linux booteable
  - Archivos WAV en `/mnt/sd/music/`
- Conexión Ethernet (para interfaz web y deploy)

## 🚀 Compilación

### 1. Configurar Toolchain

```bash
# Si usas SoC EDS de Intel/Altera
source /path/to/intelFPGA/20.1/embedded/embedded_command_shell.sh

# Verificar toolchain
arm-linux-gnueabihf-gcc --version
```

### 2. Actualizar Direcciones de Memoria

**IMPORTANTE**: Antes de compilar, actualiza las direcciones físicas en `include/config.h` según tu diseño en Platform Designer:

```c
/* Ejemplo de direcciones (ACTUALIZAR según tu Platform Designer) */
#define HPS_TO_FPGA_LW_BASE     0xFF200000  /* Lightweight bridge */
#define HPS_TO_FPGA_BASE        0xC0000000  /* Main bridge */

#define FIFO_BASE_OFFSET        0x00001000  /* Tu offset del FIFO */
#define DISPLAY_BASE_OFFSET     0x00002000  /* Tu offset de displays */
#define BUTTONS_BASE_OFFSET     0x00004000  /* Tu offset de botones */
```

### 3. Compilar

```bash
cd arm_software
make

# Para limpiar
make clean

# Para ver configuración de build
make info
```

## 📦 Despliegue a la Tarjeta

### Opción 1: Via SCP (Red)

```bash
# Asumiendo que la tarjeta tiene IP 192.168.1.100
make deploy TARGET_IP=192.168.1.100

# O manualmente:
scp audio_player root@192.168.1.100:/root/
```

### Opción 2: Via SD Card

```bash
# Copiar ejecutable a SD card montada
cp audio_player /media/user/SD_CARD/root/

# Desmontar SD, insertar en tarjeta, bootear
```

## ▶️ Ejecución

### 1. Conectarse a la Tarjeta

```bash
ssh root@<IP_DE_LA_TARJETA>
# O via UART/Serial console
```

### 2. Preparar Archivos WAV

```bash
# Crear directorio de música
mkdir -p /mnt/sd/music

# Copiar archivos WAV (formato requerido: 48kHz, 16-bit, mono o estéreo)
# Si tienes estéreo, el software lo convierte a mono automáticamente
```

### 3. Ejecutar la Aplicación

```bash
# Dar permisos de ejecución
chmod +x audio_player

# Ejecutar (requiere root para acceso a /dev/mem)
./audio_player

# O
sudo ./audio_player
```

### 4. Salida Esperada

```
╔════════════════════════════════════════╗
║   SoC Audio Player - ARM Cortex-A9    ║
║   Plataforma: Altera DE-SoC1           ║
╚════════════════════════════════════════╝

[INFO] ========================================
[INFO] SoC Audio Player - ARM HPS
[INFO] ========================================
[INFO] Initializing FIFO...
[INFO] FIFO initialized at physical address 0xC0001000
[INFO] Initializing audio controller...
[INFO] Audio controller initialized
[INFO] Loading playlist from /mnt/sd/music...
[INFO] Loaded 5 songs
[INFO] Initializing buttons...
[INFO] Button handler initialized at 0xFF204000
[INFO] Initializing display...
[INFO] Display driver initialized at 0xFF202000
[INFO] Initializing web interface on port 8080...
[INFO] Web interface initialized on port 8080
[INFO] ========================================
[INFO] System initialized successfully!
[INFO] Web interface: http://<board-ip>:8080
[INFO] ========================================
```

## 🎮 Uso

### Controles con Botones

- **KEY0 (Play/Pause)**: Inicia o pausa la reproducción
- **KEY1 (Next)**: Siguiente canción
- **KEY2 (Previous)**: Canción anterior
- **KEY3 (Stop)**: Detener reproducción

### Interfaz Web

1. Abrir navegador en tu computadora
2. Navegar a: `http://<IP_DE_LA_TARJETA>:8080`
3. Ver información en tiempo real:
   - Estado de reproducción
   - Nombre de la canción
   - Artista y álbum
   - Tiempo actual y total

La página se actualiza automáticamente cada 2 segundos.

### Displays 7 Segmentos

Muestran el tiempo de reproducción en formato `MM:SS`:
- Ejemplo: `03:45` = 3 minutos, 45 segundos

## 🐛 Debugging y Troubleshooting

### Problema: "Failed to open /dev/mem"

**Solución**: Ejecutar como root
```bash
sudo ./audio_player
```

### Problema: "Failed to initialize FIFO"

**Causas posibles**:
1. Dirección física incorrecta en `config.h`
2. FPGA no programada
3. Diseño en Platform Designer no incluye el FIFO

**Solución**: Verificar direcciones en Platform Designer y actualizar `config.h`

### Problema: "No songs found in playlist"

**Solución**:
```bash
# Verificar que existen archivos WAV
ls -l /mnt/sd/music/

# Verificar formato de archivos WAV
file /mnt/sd/music/*.wav
```

### Problema: Audio con glitches o interrupciones

**Causas posibles**:
1. FIFO muy pequeño (aumentar a 16KB o 32KB)
2. SD card muy lenta
3. Buffer underruns

**Solución**: Revisar logs y aumentar tamaño de FIFO en Platform Designer

### Habilitar Debug Detallado

En `include/config.h`:
```c
#define DEBUG_ENABLED           1
#define DEBUG_FIFO_STATUS       1  /* Debug de FIFO */
```

Recompilar y ejecutar para ver logs detallados.

## 📝 Configuración Avanzada

### Cambiar Puerto del Servidor Web

En `include/config.h`:
```c
#define WEB_SERVER_PORT     8080  /* Cambiar a tu puerto */
```

### Cambiar Directorio de Música

En `include/config.h`:
```c
#define MUSIC_DIRECTORY     "/mnt/sd/music"  /* Tu path */
```

### Ajustar Tamaño de Buffer de Reproducción

En `include/config.h`:
```c
#define PLAYBACK_BUFFER_SIZE    4096  /* Aumentar si hay underruns */
```

## 🔧 Integración con NIOS II

Este software en ARM escribe las muestras de audio al FIFO. El NIOS II debe:

1. **Leer del FIFO** usando polling o interrupciones
2. **Aplicar filtros** según estado de switches
3. **Enviar al Audio Codec** (WM8731) via I2S

Ver documentación del proyecto NIOS II para la otra mitad del sistema.

## 📚 Referencias

- [DE1-SoC User Manual](https://www.terasic.com.tw/cgi-bin/page/archive.pl?No=836)
- [Cyclone V HPS Technical Reference Manual](https://www.intel.com/content/www/us/en/docs/programmable/683126/current/hard-processor-system-technical-reference.html)
- [SoC EDS User Guide](https://www.intel.com/content/www/us/en/docs/programmable/683187/current/introduction-to-intel-soc-fpga-embedded.html)

## ✅ Checklist de Requisitos del Proyecto

- [x] Lee archivos WAV desde SD card
- [x] Escribe muestras al FIFO en FPGA (HPS-FPGA)
- [x] Maneja botones por interrupciones (play/pause/next/prev)
- [x] Actualiza displays 7 segmentos con tiempo (MM:SS)
- [x] Interfaz externa (servidor web) con info de canción
- [x] Código mínimo y eficiente (sin SO complejo visible)
- [x] Sistema operativo mínimo (Linux precompilado)
- [x] Aplicación inicia automáticamente (agregar a init scripts)

## 🚀 Próximos Pasos (Para Mañana)

1. **Programar FPGA** con diseño de Platform Designer
2. **Actualizar direcciones** en `config.h` según tu diseño
3. **Compilar** este software
4. **Bootear Linux** en la tarjeta
5. **Copiar ejecutable** y archivos WAV a SD
6. **Probar** funcionamiento básico
7. **Integrar con NIOS II** (la otra mitad del proyecto)

## 👥 Contribuciones

Proyecto realizado por el equipo de 4 personas para CE-1113.

## 📄 Licencia

Proyecto académico - TEC 2025
