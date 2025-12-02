# 🚨 NOTAS CRÍTICAS - PARA MAÑANA CON LA TARJETA

## ⚠️ ANTES DE COMPILAR - OBLIGATORIO

### 1. Actualizar Direcciones de Memoria en `include/config.h`

**ESTE ES EL PASO MÁS CRÍTICO**. Las direcciones actuales son PLACEHOLDERS:

```c
// En include/config.h - ACTUALIZAR ESTAS LÍNEAS:

/* Base addresses (VERIFICAR con Platform Designer) */
#define HPS_TO_FPGA_LW_BASE     0xFF200000  // ← Lightweight bridge
#define HPS_TO_FPGA_BASE        0xC0000000  // ← Main bridge

/* Offsets (OBTENER de Platform Designer) */
#define FIFO_BASE_OFFSET        0x00001000  // ← Tu offset del FIFO
#define DISPLAY_BASE_OFFSET     0x00002000  // ← Tu offset de 7-seg
#define SWITCHES_BASE_OFFSET    0x00003000  // ← Tu offset de switches
#define BUTTONS_BASE_OFFSET     0x00004000  // ← Tu offset de botones
#define LEDS_BASE_OFFSET        0x00005000  // ← Tu offset de LEDs
```

**Cómo obtener las direcciones correctas:**
1. Abrir Platform Designer
2. En "System Contents", ver la columna "Base Address" de cada componente
3. Para componentes en FPGA: base = HPS_TO_FPGA_LW_BASE + offset
4. Para FIFO grande: base = HPS_TO_FPGA_BASE + offset

## 📝 CHECKLIST PARA MAÑANA

### Fase 1: Hardware Setup (30 min)
- [ ] Conectar tarjeta DE-SoC1 a power
- [ ] Conectar Ethernet
- [ ] Conectar UART/USB console (para ver output)
- [ ] Insertar SD card con Linux
- [ ] Encender y verificar que bootea Linux

### Fase 2: Platform Designer (2-3 horas)
- [ ] Crear proyecto Quartus para DE-SoC1
- [ ] Abrir Platform Designer
- [ ] Agregar componentes:
  - [ ] HPS (ARM Cortex-A9) con SDRAM controller
  - [ ] NIOS II/e (o /f según performance necesario)
  - [ ] **FIFO Avalon-ST** (8KB mínimo, 16KB recomendado)
  - [ ] Audio Codec Controller (para WM8731)
  - [ ] PIO para displays 7-seg (32-bit output)
  - [ ] PIO para botones (4-bit input con interrupts)
  - [ ] PIO para switches (10-bit input)
  - [ ] PIO para LEDs (debug, opcional)
- [ ] Configurar puentes HPS-FPGA:
  - [ ] Lightweight HPS-to-FPGA bridge (para PIOs)
  - [ ] HPS-to-FPGA bridge (para FIFO grande)
- [ ] **ANOTAR TODAS LAS DIRECCIONES BASE**
- [ ] Generar HDL
- [ ] Compilar diseño en Quartus (1-2 horas)
- [ ] Programar FPGA

### Fase 3: Software ARM (1 hora)
- [ ] Actualizar direcciones en `include/config.h`
- [ ] Compilar: `make`
- [ ] Copiar binario a SD card
- [ ] Copiar archivos WAV de prueba a `/mnt/sd/music/`

### Fase 4: Primera Prueba (30 min)
- [ ] Bootear Linux en ARM
- [ ] Via SSH o UART, ejecutar: `./audio_player`
- [ ] Verificar que no hay errores de mmap
- [ ] Presionar botones para probar callbacks
- [ ] Verificar displays 7-seg se actualizan
- [ ] Abrir web browser a http://IP:8080

## 🔍 DEBUGGING INICIAL

### Si falla "Failed to mmap FIFO"

1. Verificar que FPGA está programada:
   ```bash
   # Ver si hay componentes FPGA visibles
   ls -l /sys/class/fpga_manager/
   ```

2. Verificar dirección en Platform Designer

3. Probar lectura manual:
   ```bash
   devmem2 0xC0001000  # Tu dirección de FIFO
   ```

### Si no responden botones

1. Verificar PIO está configurado con interrupts
2. Probar polling manual:
   ```c
   // En button_handler.c, ya está implementado polling mode
   ```

### Si displays no muestran nada

1. Verificar que 7-seg está conectado correctamente en Platform Designer
2. Probar escribir manualmente:
   ```bash
   devmem2 0xFF202000 w 0x3F3F3F3F  # Tu dirección + patrón test
   ```

## 📊 MAPA DE MEMORIA TÍPICO (DE-SoC1)

Estos son valores TÍPICOS, pero **DEBES VERIFICAR CON TU PLATFORM DESIGNER**:

```
┌─────────────────────────────────────────────┐
│ SDRAM: 0x00000000 - 0x3FFFFFFF (1GB)       │  ARM access
│                                             │
│ HPS Peripherals: 0xFF200000 - 0xFFFFFFFF   │  ARM internal
│                                             │
│ HPS-FPGA Lightweight: 0xFF200000+          │  PIOs, small IPs
│   - Buttons PIO:      0xFF200000 (ejemplo) │
│   - 7-Seg PIO:        0xFF201000 (ejemplo) │
│   - Switches PIO:     0xFF202000 (ejemplo) │
│                                             │
│ HPS-FPGA Bridge: 0xC0000000 - 0xDFFFFFFF   │  Large transfers
│   - FIFO:             0xC0000000 (ejemplo) │
└─────────────────────────────────────────────┘
```

## 🎵 ARCHIVOS WAV DE PRUEBA

Para pruebas iniciales, usa archivos WAV SIMPLES:

**Especificaciones:**
- Sample Rate: 48000 Hz (o 44100 Hz)
- Bits per sample: 16
- Channels: 1 (mono) o 2 (estéreo, se convierte a mono)
- Formato: PCM sin comprimir

**Crear archivo de prueba con `sox`:**
```bash
# Generar tono de prueba de 5 segundos
sox -n -r 48000 -c 1 -b 16 test_tone.wav synth 5 sine 440

# Convertir cualquier audio a formato correcto
sox input.mp3 -r 48000 -c 1 -b 16 output.wav
```

## 🚀 ORDEN DE TESTING RECOMENDADO

1. **Test 1: FIFO Write**
   - Escribir datos dummy al FIFO
   - Verificar que no hay errores de mmap
   - Ver fill_level del FIFO

2. **Test 2: WAV Reader**
   - Leer un archivo WAV simple
   - Verificar header parse correcto
   - Leer algunas muestras

3. **Test 3: Integración FIFO + WAV**
   - Leer WAV y escribir a FIFO
   - Verificar flujo continuo

4. **Test 4: Buttons**
   - Presionar botones
   - Verificar callbacks se ejecutan

5. **Test 5: Displays**
   - Actualizar displays con tiempo
   - Verificar patrón correcto

6. **Test 6: Web Interface**
   - Acceder desde browser
   - Verificar que muestra info

7. **Test 7: Sistema Completo**
   - Reproducir música de verdad
   - Verificar comunicación con NIOS II

## ⏰ ESTIMACIÓN DE TIEMPO

- Platform Designer setup: **2-3 horas**
- Compilación Quartus: **1-2 horas**
- Actualizar y compilar software ARM: **30 min**
- Testing y debug inicial: **2-3 horas**
- **TOTAL DÍA 1: 6-9 horas**

## 🆘 SI ALGO SALE MAL

### Backup Plan 1: Usar Ejemplo de Altera
Si Platform Designer es muy complicado, usa ejemplos de Altera:
- Golden Hardware Reference Design (GHRD)
- Tiene HPS configurado correctamente
- Agrega solo el FIFO y PIOs que necesitas

### Backup Plan 2: Simplificar Arquitectura
Si no funciona ARM + FIFO + NIOS:
- Considera usar solo NIOS II para todo (más simple)
- O solo ARM sin NIOS (pero no cumple requisitos del profesor)

### Backup Plan 3: Usar Polling en Lugar de Interrupciones
- El button_handler.c ya tiene polling mode implementado
- Funciona sin configuración de interrupciones
- Menos eficiente pero más fácil de debuggear

## 📞 CONTACTOS Y RECURSOS

- **Profesor**: Dr.-Ing. Jeferson González Gómez
- **Deadline**: 2 de Diciembre de 2025
- **Manual DE-SoC1**: https://www.terasic.com.tw/cgi-bin/page/archive.pl?No=836
- **Intel SoC EDS**: https://www.intel.com/content/www/us/en/products/details/fpga/development-tools.html

## 💡 TIP FINAL

**Documenta TODO lo que hagas mañana:**
- Screenshots de Platform Designer
- Direcciones de memoria usadas
- Errores que encuentres y cómo los resolviste
- Esto será invaluable para el informe final (20% de la nota)

¡Éxito mañana! 🚀
