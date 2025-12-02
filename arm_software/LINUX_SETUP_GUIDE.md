# 🐧 Guía Completa: Linux en DE-SoC1 y Relación con Nuestro Código

## 📖 Entendiendo la Arquitectura Completa

### ¿Cómo se relaciona TODO?

```
┌─────────────────────────────────────────────────────────────┐
│ 1. SD CARD (Tarjeta física)                                 │
│    ├── Partition 1 (FAT32): Boot files                      │
│    │   ├── u-boot.scr          (boot script)                │
│    │   ├── zImage              (Linux kernel)               │
│    │   ├── socfpga.dtb         (device tree)                │
│    │   └── soc_system.rbf      (FPGA configuration)         │
│    └── Partition 2 (ext3/4): Root filesystem                │
│        ├── /bin, /lib, /usr    (Linux userspace)            │
│        ├── /mnt/sd/music/      (Tus archivos WAV)           │
│        └── /root/audio_player  (TU APLICACIÓN ← Esto!)      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. PROCESO DE BOOT                                          │
│    a) Power ON                                              │
│    b) Boot ROM (en chip) → Lee Preloader                    │
│    c) Preloader → Inicializa SDRAM, configura clocks       │
│    d) U-Boot → Bootloader                                   │
│    e) U-Boot programa FPGA con soc_system.rbf              │
│    f) U-Boot carga Linux kernel (zImage)                    │
│    g) Linux boot → monta root filesystem                    │
│    h) Init scripts → /etc/init.d/                           │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. LINUX CORRIENDO (Este es el ambiente de tu app)         │
│    ├── Kernel Linux 4.x/5.x running                         │
│    ├── Drivers cargados (/dev/mem, etc)                     │
│    ├── Shell disponible (via SSH o UART)                    │
│    └── Tu aplicación se ejecuta AQUÍ:                       │
│        $ ./audio_player                                      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 4. TU APLICACIÓN (audio_player)                             │
│    ├── Es un proceso de Linux normal (userspace)            │
│    ├── Usa /dev/mem para acceder a FPGA (privileged)        │
│    ├── Lee archivos WAV del filesystem                      │
│    ├── Abre sockets para web server                         │
│    └── Escribe a memoria mapeada de FPGA (FIFO, PIOs)       │
└─────────────────────────────────────────────────────────────┘
```

## 🔍 ¿Qué es "Linux Precompilado"?

Linux precompilado = **Una imagen completa de SD card lista para usar**, que incluye:

1. **Boot files** (kernel, device tree, FPGA config)
2. **Root filesystem** (sistema operativo básico)
3. **Drivers** necesarios para HPS
4. **Toolchain** (opcional, para compilar en la tarjeta)

## 📥 OPCIÓN 1: Golden Hardware Reference Design (GHRD) - RECOMENDADO

### ¿Qué es GHRD?

Es el diseño de referencia oficial de Intel/Altera que incluye:
- ✅ Linux kernel precompilado
- ✅ U-Boot configurado
- ✅ Imagen de SD card completa
- ✅ Ejemplos funcionando
- ✅ Documentación

### Dónde Obtenerlo

**Fuente Oficial:**
```
Intel/Altera FPGA University Program
URL: https://www.intel.com/content/www/us/en/programmable/support/training/university/materials-software.html
```

**Terasic (Fabricante de DE-SoC1):**
```
URL: https://www.terasic.com.tw/cgi-bin/page/archive.pl?No=836
Sección: "Resources" → "CD-ROM" → "Linux"
```

### Archivos que Buscas

```
DE1_SoC_SD_Card_Image/
├── DE1_SoC_SD.img          (Imagen completa de SD - Easy mode)
└── manual_files/            (Si quieres crear manualmente)
    ├── u-boot.img
    ├── zImage
    ├── socfpga_cyclone5_de1_soc.dtb
    └── rootfs.tar.gz
```

## 📥 OPCIÓN 2: Intel SoC EDS - Más control

### Instalar SoC EDS

1. **Descargar SoC EDS**
   ```
   URL: https://www.intel.com/content/www/us/en/software-kit/785086/intel-soc-fpga-embedded-development-suite-pro-edition-version-23-4.html

   Necesitas:
   - Intel Quartus Prime Lite (gratuito)
   - SoC EDS (incluido)
   ```

2. **Instalar en Linux** (Ubuntu recomendado)
   ```bash
   chmod +x SoCEDSProSetup-23.4.0.115-linux.run
   ./SoCEDSProSetup-23.4.0.115-linux.run
   ```

3. **Setup environment**
   ```bash
   source /path/to/intelFPGA/23.4/embedded/embedded_command_shell.sh
   ```

### SoC EDS incluye:

- ✅ Toolchain ARM cross-compiler
- ✅ Linux kernel source y prebuilts
- ✅ U-Boot source y prebuilts
- ✅ Device Tree Generator
- ✅ Ejemplos y templates

## 📥 OPCIÓN 3: RocketBoards.org - Comunidad

```
URL: https://rocketboards.org/
Sección: Cyclone V SoC → Linux Distributions
```

Incluye:
- Buildroot configs (si decides usarlo después)
- Angstrom Linux
- Ubuntu Core
- Debian

## 🛠️ PREPARAR SD CARD - MÉTODO FÁCIL

### Paso 1: Descargar Imagen

```bash
# Ejemplo con imagen de Terasic
wget http://www.terasic.com.tw/.../DE1_SoC_SD.img.zip
unzip DE1_SoC_SD.img.zip
```

### Paso 2: Escribir a SD Card

```bash
# CUIDADO: Verificar que /dev/sdX es TU SD card!
# Usa 'lsblk' para identificar correctamente

# Desmontar si está montada
sudo umount /dev/sdX*

# Escribir imagen (puede tomar 10-20 minutos)
sudo dd if=DE1_SoC_SD.img of=/dev/sdX bs=4M status=progress
sudo sync

# Ejemplo:
# sudo dd if=DE1_SoC_SD.img of=/dev/sdb bs=4M status=progress
```

### Paso 3: Verificar Particiones

```bash
# Ver particiones creadas
sudo fdisk -l /dev/sdX

# Deberías ver:
# /dev/sdX1  FAT32   ~500MB   (Boot partition)
# /dev/sdX2  ext3/4  ~1-2GB   (Root filesystem)
```

### Paso 4: Montar y Personalizar

```bash
# Montar partición 2 (rootfs)
sudo mkdir -p /mnt/sd_root
sudo mount /dev/sdX2 /mnt/sd_root

# Crear directorio para música
sudo mkdir -p /mnt/sd_root/mnt/sd/music

# Copiar tu aplicación (cuando esté compilada)
sudo cp audio_player /mnt/sd_root/root/

# Copiar archivos WAV de prueba
sudo cp *.wav /mnt/sd_root/mnt/sd/music/

# Desmontar
sudo umount /mnt/sd_root
```

## 🛠️ PREPARAR SD CARD - MÉTODO MANUAL (Más control)

Si quieres entender cada paso o la imagen no funciona:

### Paso 1: Particionar SD Card

```bash
# Borrar particiones existentes y crear nuevas
sudo fdisk /dev/sdX

# Comandos en fdisk:
# o    (crear nueva tabla de particiones DOS)
# n    (nueva partición)
# p    (primaria)
# 1    (número de partición)
# [Enter] (primer sector por defecto)
# +500M   (tamaño 500MB)
# t    (tipo)
# b    (FAT32)
# n    (nueva partición 2)
# p    (primaria)
# 2
# [Enter] (resto del espacio)
# w    (escribir cambios)
```

### Paso 2: Formatear Particiones

```bash
# Formatear partición boot (FAT32)
sudo mkfs.vfat -F 32 -n BOOT /dev/sdX1

# Formatear partición root (ext4)
sudo mkfs.ext4 -L rootfs /dev/sdX2
```

### Paso 3: Copiar Boot Files

```bash
# Montar partición boot
sudo mkdir -p /mnt/sd_boot
sudo mount /dev/sdX1 /mnt/sd_boot

# Copiar archivos de boot (obtener de GHRD o SoC EDS)
sudo cp u-boot.img /mnt/sd_boot/
sudo cp zImage /mnt/sd_boot/
sudo cp socfpga_cyclone5_de1_soc.dtb /mnt/sd_boot/socfpga.dtb
sudo cp soc_system.rbf /mnt/sd_boot/  # Tu diseño FPGA

# Crear boot script (u-boot.scr)
# Ver abajo para contenido

sudo umount /mnt/sd_boot
```

### Paso 4: Extraer Root Filesystem

```bash
# Montar partición root
sudo mount /dev/sdX2 /mnt/sd_root

# Extraer rootfs (de GHRD o build propio)
sudo tar -xzf rootfs.tar.gz -C /mnt/sd_root/

# Configurar
sudo chown -R root:root /mnt/sd_root/*

sudo umount /mnt/sd_root
```

## 🚀 BOOTEAR LINUX EN LA TARJETA

### Paso 1: Hardware Setup

```
1. Configurar switches de boot en DE-SoC1:
   SW10: todos hacia abajo (boot from SD card)

2. Insertar SD card en slot de la tarjeta

3. Conectar:
   - Cable USB (UART console)
   - Cable Ethernet (opcional, para SSH)
   - Power supply

4. Abrir terminal serial:
   - Baudrate: 115200
   - Data bits: 8
   - Parity: None
   - Stop bits: 1
```

### Paso 2: Conectar a Console

```bash
# Linux
sudo minicom -D /dev/ttyUSB0 -b 115200

# O con screen
sudo screen /dev/ttyUSB0 115200

# Windows: usar PuTTY
```

### Paso 3: Encender y Observar Boot

Verás algo como:

```
U-Boot 2021.04 (Altera)
SOCFPGA: Cyclone V
...
Loading kernel...
Starting kernel...

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 5.10.0-altera
...
[   OK   ] Started System Logging Service
[   OK   ] Reached target Multi-User System

DE1-SoC login: root
Password: (normalmente sin password o "root")
```

### Paso 4: Verificar Sistema

```bash
# Login
root@DE1-SoC:~# uname -a
Linux DE1-SoC 5.10.0 #1 SMP ARM

# Ver memoria
root@DE1-SoC:~# free -h

# Ver montajes
root@DE1-SoC:~# df -h

# Verificar acceso a /dev/mem
root@DE1-SoC:~# ls -l /dev/mem
crw-r----- 1 root kmem 1, 1 Jan  1 00:00 /dev/mem

# Verificar red (si conectada)
root@DE1-SoC:~# ifconfig
root@DE1-SoC:~# ip addr show
```

## 🔗 CÓMO SE RELACIONA CON TU CÓDIGO

### 1. Compilación (En tu PC con toolchain)

```bash
cd arm_software/

# El Makefile usa el cross-compiler
make CROSS_COMPILE=arm-linux-gnueabihf-

# Genera: audio_player (binario ARM)
file audio_player
# Output: audio_player: ELF 32-bit LSB executable, ARM, ...
```

### 2. Deployment (Copiar a tarjeta)

```bash
# Opción A: Via SCP (si hay red)
scp audio_player root@192.168.1.100:/root/

# Opción B: Via SD card
sudo mount /dev/sdX2 /mnt/sd_root
sudo cp audio_player /mnt/sd_root/root/
sudo umount /mnt/sd_root
```

### 3. Ejecución (En la tarjeta)

```bash
# En la tarjeta DE-SoC1 (via SSH o UART)
root@DE1-SoC:~# cd /root
root@DE1-SoC:~# chmod +x audio_player
root@DE1-SoC:~# ./audio_player

# Tu aplicación inicia y:
# - Abre /dev/mem para acceder a FPGA
# - Lee archivos WAV del filesystem
# - Escribe a periféricos FPGA via memory mapping
# - Inicia servidor web en puerto 8080
```

### 4. Relación con FPGA

```
Tu código:          Linux:              Hardware:
─────────────────────────────────────────────────────
fifo_init()   →   open(/dev/mem)  →  Kernel permite
                  mmap(0xC0000000) →  Acceso a bus HPS-FPGA
                                    ↓
*fifo->data   →   Write to         →  FPGA recibe datos
= sample          0xC0000000           en el FIFO

button_read() →   Read from        →  FPGA PIO envía estado
                  0xFF204000           de botones

display_write()→  Write to         →  FPGA 7-seg muestra
                  0xFF202000           dígitos
```

## 🎯 TESTING PASO A PASO - MAÑANA

### Test 1: Boot básico (10 min)

```bash
# Objetivo: Verificar que Linux bootea
1. Insertar SD con imagen
2. Encender tarjeta
3. Ver output en UART console
4. Login exitoso

✓ Si ves el prompt "root@DE1-SoC:~#" → ÉXITO
```

### Test 2: Acceso a /dev/mem (5 min)

```bash
# En la tarjeta:
root@DE1-SoC:~# devmem 0xFF200000
# Debería devolver un valor hexadecimal (o error si dirección inválida)

✓ Si devuelve valor → /dev/mem funciona
✗ Si dice "Operation not permitted" → Problema de permisos
```

### Test 3: Copiar y ejecutar Hello World (10 min)

```bash
# En tu PC, crear test simple:
cat > hello_arm.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello from ARM!\n");
    return 0;
}
EOF

# Compilar para ARM
arm-linux-gnueabihf-gcc hello_arm.c -o hello_arm

# Copiar a tarjeta (via SD o SCP)
scp hello_arm root@<IP>:/root/

# En la tarjeta:
root@DE1-SoC:~# ./hello_arm
Hello from ARM!

✓ Si imprime el mensaje → Toolchain y deployment funcionan
```

### Test 4: Tu aplicación real (30 min)

```bash
# Compilar tu código
cd arm_software/
make

# Copiar a tarjeta
scp audio_player root@<IP>:/root/
scp test.wav root@<IP>:/mnt/sd/music/

# En la tarjeta:
root@DE1-SoC:~# ./audio_player

# Observar output
# Debería ver inicialización de módulos
# Si hay errores de mmap → Actualizar direcciones en config.h
```

## 🔧 CONFIGURACIÓN AVANZADA

### Auto-start al Boot

Crear script en `/etc/init.d/`:

```bash
# En la tarjeta:
cat > /etc/init.d/audio_player << 'EOF'
#!/bin/sh
### BEGIN INIT INFO
# Provides:          audio_player
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: SoC Audio Player
### END INIT INFO

case "$1" in
  start)
    echo "Starting audio player..."
    /root/audio_player &
    ;;
  stop)
    echo "Stopping audio player..."
    killall audio_player
    ;;
  *)
    echo "Usage: /etc/init.d/audio_player {start|stop}"
    exit 1
    ;;
esac
exit 0
EOF

chmod +x /etc/init.d/audio_player
update-rc.d audio_player defaults
```

### Configurar IP Estática

```bash
# Editar /etc/network/interfaces
auto eth0
iface eth0 inet static
    address 192.168.1.100
    netmask 255.255.255.0
    gateway 192.168.1.1
```

## 📚 RECURSOS ADICIONALES

### Documentación Clave

1. **DE1-SoC User Manual**
   - https://www.terasic.com.tw/cgi-bin/page/archive.pl?No=836

2. **SoC EDS User Guide**
   - Incluido en instalación de SoC EDS
   - /intelFPGA/23.4/embedded/documentation/

3. **Linux para Cyclone V**
   - https://rocketboards.org/foswiki/Documentation/CycloneVSoCDevelopmentKit

4. **U-Boot Documentation**
   - https://u-boot.readthedocs.io/

### Videos Útiles

- "Getting Started with DE1-SoC" (YouTube - Terasic)
- "Linux Boot Process on ARM" (Embedded Linux Conference)

## ❓ FAQ

**Q: ¿Necesito compilar Linux yo mismo?**
A: NO. Usa imagen precompilada de Terasic o Intel. Solo compila tu aplicación.

**Q: ¿Qué versión de kernel funciona?**
A: Linux 4.x o 5.x. La versión específica no es crítica, siempre que tenga drivers para Cyclone V.

**Q: ¿Puedo usar otra distribución (Ubuntu, Debian)?**
A: Sí, pero es más complejo. Mejor empieza con imagen de Terasic.

**Q: ¿Cómo actualizo el kernel si necesito?**
A: Reemplaza `zImage` en partición boot y reinicia.

**Q: ¿El Linux ocupa mucho espacio?**
A: ~500MB boot + ~1-2GB root filesystem. SD de 4GB es suficiente.

## ✅ CHECKLIST FINAL

Antes de dormir esta noche, asegúrate de tener:

- [ ] SD card (4GB mínimo, Class 10 recomendado)
- [ ] Imagen de Linux descargada
- [ ] Cable USB para UART console
- [ ] Software de terminal (minicom, screen, o PuTTY)
- [ ] Toolchain ARM instalado y testeado
- [ ] Este código compilado exitosamente (test con `make`)
- [ ] Archivos WAV de prueba preparados

Mañana estarás listo para bootear y probar! 🚀
