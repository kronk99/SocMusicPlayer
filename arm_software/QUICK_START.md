# ⚡ Quick Start - Comandos para Mañana

## 🚀 Comandos Rápidos - Copy & Paste

### ESTA NOCHE: Preparación

```bash
# 1. Instalar toolchain ARM (5 minutos)
sudo apt-get update
sudo apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# 2. Verificar instalación
arm-linux-gnueabihf-gcc --version

# 3. Instalar herramientas útiles
sudo apt-get install -y minicom sox

# 4. Test de compilación de nuestro código
cd ~/Documents/Cursos/TEC/Embebidos/SocMusicPlayer/arm_software
make
# Debería compilar sin errores (aunque las direcciones son placeholder)
```

### MAÑANA CON LA TARJETA: Linux Setup

```bash
# 1. Identificar SD card
lsblk
# Buscar tu SD (ej: /dev/sdb - VERIFICA QUE SEA CORRECTO!)

# 2. Escribir imagen Linux (REEMPLAZAR sdX con tu device!)
# ⚠️ PELIGRO: Esto BORRA TODO en el device!
sudo dd if=DE1_SoC_SD.img of=/dev/sdX bs=4M status=progress conv=fsync
sudo sync

# 3. Montar particiones para personalizarlas
sudo mkdir -p /mnt/de_soc_root
sudo mount /dev/sdX2 /mnt/de_soc_root

# 4. Crear directorio de música
sudo mkdir -p /mnt/de_soc_root/mnt/sd/music

# 5. Copiar archivos WAV de prueba
sudo cp *.wav /mnt/de_soc_root/mnt/sd/music/

# 6. Desmontar
sudo umount /mnt/de_soc_root

# 7. Insertar SD en tarjeta DE-SoC1 y bootear
```

### MAÑANA: Conexión a la Tarjeta

```bash
# Opción A: Via UART/Serial
sudo minicom -D /dev/ttyUSB0 -b 115200
# Login: root
# Password: (enter) o "root"

# Opción B: Via SSH (si configuraste red)
ssh root@192.168.1.100
# Password: (enter) o "root"
```

### MAÑANA: Después de Platform Designer

```bash
# 1. Abrir config.h y actualizar direcciones
# (Usar las que anotaste de Platform Designer)
nano arm_software/include/config.h

# 2. Recompilar
cd arm_software
make clean
make

# 3. Verificar que es binario ARM
file audio_player
# Debe decir: "ARM"

# 4. Copiar a tarjeta
scp audio_player root@192.168.1.100:/root/
# O via SD card:
sudo mount /dev/sdX2 /mnt/de_soc_root
sudo cp audio_player /mnt/de_soc_root/root/
sudo umount /mnt/de_soc_root
```

### MAÑANA: Testing en la Tarjeta

```bash
# En la tarjeta (via SSH o UART):

# 1. Dar permisos
chmod +x /root/audio_player

# 2. Ejecutar
cd /root
./audio_player

# Deberías ver:
# [INFO] SoC Audio Player - ARM HPS
# [INFO] Initializing FIFO...
# etc.

# 3. Si hay error de /dev/mem:
# - Verificar que eres root
# - Verificar que FPGA está programada

# 4. Test web interface (desde tu PC):
firefox http://192.168.1.100:8080

# 5. Presionar botones físicos en la tarjeta
# Deberías ver logs de botones en la consola
```

## 🐛 Troubleshooting Rápido

```bash
# Error: "Failed to open /dev/mem"
# Solución:
sudo ./audio_player  # Correr como root

# Error: "Failed to mmap FIFO"
# Causa: Direcciones incorrectas o FPGA no programada
# Solución:
# 1. Verificar FPGA programada
# 2. Verificar direcciones en config.h match Platform Designer

# Error: "No songs found"
# Solución:
ls -l /mnt/sd/music/*.wav  # Verificar archivos existen

# Error: Botones no responden
# El código usa polling, debería funcionar siempre
# Verificar dirección de BUTTON_BASE_OFFSET en config.h

# Error: Displays no muestran nada
# Verificar dirección de DISPLAY_BASE_OFFSET en config.h

# Error: Web no carga
# Verificar:
ifconfig  # Ver IP de la tarjeta
netstat -tuln | grep 8080  # Verificar puerto abierto
```

## 📝 Template para Anotar Direcciones de Platform Designer

```
Componente         | Base Address | Offset desde bridge
─────────────────────────────────────────────────────────
HPS-FPGA Bridge    | 0xC0000000   | -
FIFO               | 0x________   | 0x________
Audio Codec        | 0x________   | 0x________

Lightweight Bridge | 0xFF200000   | -
7-Seg Display      | 0x________   | 0x________
Buttons            | 0x________   | 0x________
Switches           | 0x________   | 0x________
LEDs               | 0x________   | 0x________
```

Copiar esto en un txt y llenar mañana cuando diseñes en Platform Designer.

## ⚡ Super Quick Reference

```bash
# Compilar
cd arm_software && make

# Deploy
make deploy TARGET_IP=192.168.1.100

# Run en tarjeta
ssh root@192.168.1.100 "/root/audio_player"

# Ver logs en tiempo real
ssh root@192.168.1.100 "tail -f /var/log/messages"

# Test memoria FPGA (sin app)
ssh root@192.168.1.100 "devmem 0xC0000000"
```

## 📞 Ayuda Rápida

Si algo no funciona:

1. **Lee los logs**: El código imprime mucha info de debug
2. **Verifica paso a paso**:
   - ¿Linux bootea? ✓
   - ¿/dev/mem accesible? ✓
   - ¿FPGA programada? ✓
   - ¿Direcciones correctas? ✓
3. **Consulta**: `CRITICAL_NOTES.md`, `LINUX_SETUP_GUIDE.md`
4. **Pregunta al profesor**: Si es tema de hardware específico

## ✅ Mini Checklist Mañana

```
FASE 1: Linux (1-2h)
  [ ] SD preparada con imagen
  [ ] Tarjeta bootea Linux
  [ ] Login funciona
  [ ] Red configurada (opcional)

FASE 2: Platform Designer (2-3h)
  [ ] Proyecto Quartus creado
  [ ] SoC diseñado (HPS+NIOS+FIFO+PIOs)
  [ ] Direcciones anotadas
  [ ] Compilado y programado

FASE 3: Software (30min)
  [ ] config.h actualizado
  [ ] Compilado exitoso
  [ ] Copiado a tarjeta

FASE 4: Test (2h)
  [ ] App ejecuta sin errores
  [ ] FIFO funciona
  [ ] Botones detectados
  [ ] Displays actualizan
  [ ] Web accesible

Al final del día: Sistema básico funcionando! 🎉
```

¡Éxito mañana! 🚀
