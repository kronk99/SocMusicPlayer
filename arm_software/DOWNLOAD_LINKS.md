# 📥 Enlaces de Descarga Directos - Para Mañana

## 🎯 Lo que Necesitas Descargar Esta Noche/Mañana Temprano

### 1️⃣ PRIORIDAD ALTA: Imagen Linux para DE-SoC1

#### Opción A: Terasic (Más Rápido - RECOMENDADO)

```
Producto: DE1-SoC (Cyclone V SoC Development Kit)
URL Principal: https://www.terasic.com.tw/cgi-bin/page/archive.pl?No=836

En la página:
1. Hacer scroll hasta "Design Examples"
2. Buscar "Linux Console" o "Linux SD Card Image"
3. Click en "Download"

Archivo esperado: ~500MB-1GB
Nombre: DE1_SoC_SD_Card_Image.zip o similar
```

**Links alternativos del CD-ROM:**
- Terasic FTP (si disponible): ftp.terasic.com
- Solicitar en foros de Terasic si no encuentras el link

#### Opción B: Golden Hardware Reference Design (GHRD)

```
Intel/Altera GitHub:
https://github.com/altera-opensource/

Buscar: "ghrd" y "cyclone5" y "socfpga"

Específicamente:
- socfpga-ghrd (repositorio)
- Releases section
- Descargar prebuilt images si disponibles
```

### 2️⃣ PRIORIDAD MEDIA: Intel SoC EDS (Toolchain)

```
Producto: Intel SoC FPGA Embedded Development Suite
URL: https://www.intel.com/content/www/us/en/software-kit/785086/

Versión recomendada: 23.4 o más reciente

Opciones de descarga:
- SoC EDS Pro Edition (completo, ~10GB)
- SoC EDS Standard Edition (más ligero)

Incluye:
✓ arm-linux-gnueabihf-gcc (toolchain)
✓ Linux kernel source
✓ U-Boot
✓ Device Tree tools
✓ Ejemplos

NOTA: Si ya tienes toolchain ARM funcional, esto es opcional.
```

### 3️⃣ PRIORIDAD BAJA: Quartus Prime (Si no lo tienes)

```
URL: https://www.intel.com/content/www/us/en/software-kit/825278/

Versión: Quartus Prime Lite 23.1std (gratuita)
Tamaño: ~5GB

Necesario para:
- Platform Designer
- Compilar diseño FPGA
- Programar la tarjeta

Incluye:
- Quartus Prime
- ModelSim (simulación)
- Platform Designer
```

## 🛠️ Toolchain ARM Alternativo (Si SoC EDS es muy pesado)

### Linaro ARM Toolchain

```
URL: https://www.linaro.org/downloads/

Buscar: "GNU-A" toolchain
Arquitectura: arm-linux-gnueabihf

Ejemplo de descarga:
wget https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz

Extraer:
tar xf gcc-linaro-*.tar.xz
export PATH=$PATH:/path/to/gcc-linaro-.../bin
```

### Ubuntu/Debian Package Manager

```bash
# En tu PC Linux:
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf
sudo apt-get install g++-arm-linux-gnueabihf

# Verificar:
arm-linux-gnueabihf-gcc --version
```

## 📦 Otros Recursos Útiles

### Documentación Oficial

1. **DE1-SoC User Manual (PDF)**
   ```
   URL: https://www.terasic.com.tw/cgi-bin/page/archive.pl?No=836
   Sección: Documents
   Archivo: DE1-SoC_User_Manual.pdf (~20MB)
   ```

2. **Cyclone V HPS Technical Reference Manual**
   ```
   URL: https://www.intel.com/content/www/us/en/docs/programmable/683126/current/
   O buscar en: intel.com/programmable
   Buscar: "Cyclone V Hard Processor System Technical Reference Manual"
   ```

3. **Linux Device Tree Specification**
   ```
   URL: https://www.devicetree.org/
   Útil para entender .dtb files
   ```

### Ejemplos y Código de Referencia

1. **RocketBoards.org**
   ```
   URL: https://rocketboards.org/foswiki/Documentation/CycloneVSoCDevelopmentKit

   Incluye:
   - Linux BSP
   - U-Boot configs
   - Device tree examples
   - Tutoriales
   ```

2. **Intel FPGA University Program**
   ```
   URL: https://www.intel.com/content/www/us/en/programmable/support/training/university.html

   Buscar: DE1-SoC materials
   Incluye ejemplos de laboratorio
   ```

## 🔧 Software Adicional Útil

### Terminal Serial

**Linux:**
```bash
# Minicom
sudo apt-get install minicom

# O Screen (usualmente pre-instalado)
screen --version
```

**Windows:**
```
PuTTY: https://www.putty.org/
TeraTerm: https://ttssh2.osdn.jp/
```

### Herramientas SD Card

**Linux:**
```bash
# Usualmente pre-instalado
dd --version
fdisk --version
```

**Windows:**
```
Win32 Disk Imager: https://sourceforge.net/projects/win32diskimager/
Rufus: https://rufus.ie/ (no para imagen .img, usar Win32DiskImager)
```

**macOS:**
```bash
# Balena Etcher (GUI)
https://www.balena.io/etcher/

# O usar dd en terminal
```

## 🎵 Archivos WAV de Prueba

### Convertir Archivos Existentes a Formato Correcto

```bash
# Instalar sox (Swiss Army knife de audio)
sudo apt-get install sox

# Convertir cualquier audio a WAV 48kHz 16-bit mono
sox input.mp3 -r 48000 -c 1 -b 16 output.wav

# Generar tono de prueba
sox -n -r 48000 -c 1 -b 16 test_tone_440hz.wav synth 10 sine 440

# Generar varios tonos
sox -n -r 48000 -c 1 -b 16 test_sweep.wav synth 10 sine 200-2000
```

### Descargar Samples Libres

```
Free Music Archive: https://freemusicarchive.org/
(Descargar y convertir con sox)

Soundbible: https://soundbible.com/
(Efectos de sonido, algunos en WAV)

Internet Archive: https://archive.org/details/audio
(Muchos archivos de dominio público)
```

## 📋 Checklist de Descargas

Antes de ir a dormir, asegúrate de tener:

- [ ] Imagen Linux de Terasic (~500MB-1GB)
- [ ] Toolchain ARM instalado y verificado
- [ ] Quartus Prime Lite (si no lo tienes)
- [ ] Manual de usuario DE1-SoC (PDF)
- [ ] SD card vacía lista (4GB+, Class 10)
- [ ] Archivos WAV de prueba preparados
- [ ] Software de terminal serial instalado

## ⚡ Descarga Rápida - Script Helper

```bash
#!/bin/bash
# quick_setup.sh - Descargar herramientas básicas

echo "Installing ARM toolchain..."
sudo apt-get update
sudo apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

echo "Installing utilities..."
sudo apt-get install -y minicom screen sox

echo "Verifying..."
arm-linux-gnueabihf-gcc --version
sox --version

echo "Done! Now download Linux image from Terasic website manually."
```

## 🆘 Si No Encuentras las Descargas

### Terasic Support

```
Email: support@terasic.com
Subject: DE1-SoC Linux SD Card Image Request

Body:
"Hi, I'm working on a university project with the DE1-SoC board.
Could you please provide the link to download the Linux SD card
image? The download link on your website seems to be unavailable.

Board: DE1-SoC (P/N: ______)
Purpose: Academic project

Thank you!"
```

### Foros y Comunidades

1. **Intel FPGA Forum**
   ```
   URL: https://community.intel.com/t5/Intel-FPGA/bd-p/fpgas
   Buscar: "DE1-SoC Linux" o "Cyclone V SoC Linux"
   ```

2. **RocketBoards Forum**
   ```
   URL: https://forum.rocketboards.org/
   Comunidad muy activa de SoC FPGA
   ```

3. **Reddit**
   ```
   r/FPGA
   r/embedded
   Preguntar por DE1-SoC Linux image
   ```

## 💾 Tamaños de Descarga Estimados

| Item | Tamaño | Tiempo (10 Mbps) | Prioridad |
|------|--------|------------------|-----------|
| Linux Image | ~1 GB | ~15 min | ALTA |
| SoC EDS | ~10 GB | ~2 horas | MEDIA |
| Quartus Lite | ~5 GB | ~1 hora | BAJA* |
| Toolchain (Linaro) | ~200 MB | ~3 min | ALTA |
| Documentación | ~50 MB | ~1 min | MEDIA |

*Si ya tienes Quartus instalado

## ⏰ Plan de Descarga Sugerido

### Esta Noche (Antes de dormir):
1. ⭐ Toolchain ARM (via apt-get, 5 min)
2. ⭐ Linux Image de Terasic (15-30 min)
3. ⭐ Documentación (5 min)

### Mañana (Mientras trabajas en otras cosas):
4. SoC EDS (en background, ~2 horas)
5. Quartus Lite (si falta, en background)

## 🎯 Resumen

**CRÍTICO para mañana:**
- Imagen Linux: ✅ DESCARGAR ESTA NOCHE
- Toolchain ARM: ✅ INSTALAR ESTA NOCHE
- Todo lo demás: Opcional o puede esperar

**Con estos 2 items, mañana puedes:**
1. Bootear Linux en la tarjeta
2. Compilar el software ARM que hicimos hoy
3. Empezar testing inmediatamente

¡Éxito con las descargas! 🚀
