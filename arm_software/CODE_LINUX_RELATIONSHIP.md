# 🔗 Relación Exacta: Tu Código ↔ Linux ↔ Hardware

## 📖 Resumen Ejecutivo

Tu código que escribimos es una **aplicación de userspace de Linux** que:
- Corre como un proceso normal en Linux
- Usa syscalls de Linux para acceder a hardware
- No es parte del kernel, no es un driver
- Interactúa con FPGA usando memoria mapeada

## 🎯 Flujo Completo: De Código a Hardware

### Ejemplo 1: Escribir al FIFO

#### Tu Código (src/fifo_writer.c:53-71)
```c
int fifo_init(fifo_context_t *fifo, uint32_t physical_base, uint32_t size) {
    // 1. Abrir /dev/mem (archivo especial del kernel)
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    //         ↓
    //    Esta es una syscall de Linux
    //    Requiere permisos de root
```

#### Lo que pasa en Linux:
```
Tu app                  Linux Kernel              Hardware
─────────────────────────────────────────────────────────────
open("/dev/mem")   →   Kernel: "OK, tienes
                       permiso (eres root)"
                       Devuelve file
                       descriptor: fd=3

mmap(..., fd, ...)  →  Kernel: "Mapear          → Configurar MMU
                       0xC0000000 físico           para que
                       a espacio virtual           0xC0000000
                       del proceso"                sea accesible

*fifo->data = val   →  Kernel MMU traduce       → Bus HPS-FPGA
                       dirección virtual           transporta write
                                                → FIFO en FPGA
                                                   recibe dato
```

#### En Bajo Nivel:
```
1. Dirección física: 0xC0000000 (configurada en Platform Designer)
   ↓
2. Kernel mapea a dirección virtual (ej: 0xb6f00000)
   ↓
3. Tu código escribe: *fifo->data = 0x1234
   ↓
4. CPU genera bus transaction
   ↓
5. MMU traduce virtual → física
   ↓
6. HPS-FPGA bridge captura transacción
   ↓
7. FPGA FIFO recibe dato en registro 0xC0000000
```

### Ejemplo 2: Leer Archivo WAV

#### Tu Código (src/wav_reader.c:105-120)
```c
int wav_open(wav_file_t *wav, const char *filename) {
    // Abrir archivo desde filesystem
    wav->file = fopen(filename, "rb");
    //              ↓
    // filename = "/mnt/sd/music/song.wav"
```

#### Lo que pasa en Linux:
```
Tu app                 Linux Kernel               Hardware
───────────────────────────────────────────────────────────────
fopen(...)        →   Kernel: VFS (Virtual
                      File System) layer
                      ↓
                      ext4 filesystem driver
                      ↓
                      SD card driver
                      ↓
                      SD/MMC controller        → SD card física
                      ↓                           en slot
                      Lee sector del SD
                      ↓
                      Copia a buffer kernel
                      ↓
                      Devuelve FILE* a tu app

fread(buffer,     →   Kernel: Copia datos      → Lee más sectores
      size, ...)      desde buffer kernel         del SD si necesario
                      a tu buffer userspace
```

### Ejemplo 3: Servidor Web

#### Tu Código (src/web_interface.c:143-155)
```c
int web_interface_init(...) {
    // Crear socket TCP
    web->server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Bind a puerto 8080
    bind(web->server_socket, ...);

    // Listen por conexiones
    listen(web->server_socket, 5);
```

#### Lo que pasa en Linux:
```
Tu app               Linux Kernel                 Red
──────────────────────────────────────────────────────────
socket()        →   Kernel: TCP/IP stack
                    Crea socket interno
                    Devuelve fd

bind(..., 8080) →   Kernel: "Reservar
                    puerto 8080 para
                    este proceso"

listen()        →   Kernel: "Poner socket
                    en modo escucha"

accept()        →   Kernel: "Esperar          → Ethernet PHY
                    conexión en puerto           recibe paquete
                    8080"                        TCP SYN
                    ↓                         → Kernel procesa
                    Devuelve nuevo fd            y acepta conexión
                    cuando llega conexión
```

## 🔍 Dependencias Específicas de Linux

### 1. Syscalls que Usa Tu Código

| Syscall | Archivo | Línea | Para qué |
|---------|---------|-------|----------|
| `open()` | fifo_writer.c | 53 | Abrir /dev/mem |
| `mmap()` | fifo_writer.c | 67 | Mapear memoria FPGA |
| `munmap()` | fifo_writer.c | 96 | Desmapear memoria |
| `fopen()` | wav_reader.c | 107 | Abrir archivo WAV |
| `fread()` | wav_reader.c | 182 | Leer datos WAV |
| `socket()` | web_interface.c | 143 | Crear socket TCP |
| `bind()` | web_interface.c | 158 | Bind a puerto |
| `listen()` | web_interface.c | 165 | Escuchar conexiones |
| `pthread_create()` | audio_controller.c | 58 | Crear threads |
| `gettimeofday()` | button_handler.c | 28 | Obtener tiempo |

### 2. Archivos del Sistema que Usa

| Archivo | Propósito | Código |
|---------|-----------|--------|
| `/dev/mem` | Acceso a memoria física | fifo_writer.c:53 |
| `/mnt/sd/music/*.wav` | Archivos de audio | audio_controller.c:252 |
| Socket en puerto 8080 | Web server | web_interface.c:158 |

### 3. Bibliotecas de Linux que Necesita

Tu Makefile (línea 59) enlaza estas librerías:
```makefile
LIBS = -lpthread -lm
```

- **libpthread**: Para `pthread_create()`, `pthread_mutex_lock()`, etc.
- **libm**: Para funciones matemáticas (si usas filtros más adelante)
- **libc** (implícita): Para `fopen()`, `malloc()`, `printf()`, etc.

Estas bibliotecas vienen incluidas en el Linux precompilado.

## 🛠️ ¿Qué Provee el Linux Precompilado?

### 1. Kernel Linux (~5MB)
```
Archivo: zImage en SD card
Contiene:
  - Drivers para Cyclone V SoC
  - Driver /dev/mem
  - SD/MMC controller driver
  - Ethernet driver
  - USB driver
  - Filesystem support (ext4, FAT)
  - Memory management (MMU)
  - Scheduler
  - TCP/IP stack
```

### 2. Root Filesystem (~500MB-2GB)
```
Directorio raíz montado en /
Contiene:
  /bin/        - Comandos básicos (ls, cat, sh)
  /lib/        - Bibliotecas compartidas (libc.so, libpthread.so)
  /usr/        - Programas y librerías adicionales
  /etc/        - Archivos de configuración
  /dev/        - Nodos de dispositivos (/dev/mem, /dev/null, etc)
  /proc/       - Información del kernel
  /sys/        - Sysfs (info de hardware)
  /mnt/        - Puntos de montaje
  /root/       - Home del usuario root (aquí va tu app)
```

### 3. Device Tree Blob (~50KB)
```
Archivo: socfpga.dtb en SD card
Describe el hardware al kernel:
  - Direcciones de memoria de periféricos
  - IRQ numbers
  - Clocks
  - Configuración de pines
```

### 4. U-Boot Bootloader (~500KB)
```
Archivo: u-boot.img en SD card
Tareas:
  - Inicializar memoria SDRAM
  - Programar FPGA con soc_system.rbf
  - Cargar kernel (zImage)
  - Pasar device tree al kernel
  - Boot kernel
```

## 🔄 Proceso Completo: De Power-On a Tu App Running

### Timeline Detallado

```
Tiempo  Componente    Acción                      Resultado
───────────────────────────────────────────────────────────────────
t=0s    Power ON      Usuario presiona botón      Chip energizado

t=0s    Boot ROM      ROM en chip ejecuta         Busca preloader
        (HW)          código hardcoded            en SD card

t=1s    Preloader     Inicializa SDRAM            1GB RAM disponible
        (SD)          Configura PLL/clocks        CPU @ 800MHz

t=2s    U-Boot        Carga de SD card            U-Boot corriendo
        (SD)          Ejecuta                     en SDRAM

t=3s    U-Boot        Lee soc_system.rbf          FPGA programada
                      Programa FPGA               (Tu diseño cargado)

t=5s    U-Boot        Lee zImage y socfpga.dtb    Kernel en memoria
                      Salta a kernel entry point

t=6s    Linux Kernel  Inicializa drivers          /dev/mem disponible
                      Monta root filesystem       / montado

t=8s    Init System   Ejecuta /etc/init.d/*       Servicios iniciados
                      Configura red, etc.

t=10s   Login Prompt  Getty en console            "DE1-SoC login:"

t=11s   Usuario       Escribe: root               Shell iniciado
                      Password: <enter>

t=12s   Shell (Bash)  root@DE1-SoC:~# cd /root

t=13s   Tu App        ./audio_player              Proceso creado (PID)
                      ↓
                      Ejecuta main()              Tu código corriendo!
                      ↓
                      fifo_init()                 mmap() a FPGA
                      ↓
                      audio_controller_init()     Thread iniciado
                      ↓
                      web_interface_init()        Socket abierto
                      ↓
                      Main loop                   Sistema funcionando
```

## 🎮 Interacción en Tiempo de Ejecución

### Ejemplo: Usuario Presiona Botón

```
Hardware         Linux Kernel          Tu Código
──────────────────────────────────────────────────────────
Usuario
presiona KEY0
    ↓
FPGA PIO
detecta falling
edge
    ↓
(En implementación    (Tu polling thread)      button_handler.c:63
polling mode, no      lee periódicamente
usa IRQ de Linux)     *button_base
    ↓                     ↓
                      Detecta cambio           if (pressed & (1<<i))
                          ↓
                      Llama callback           callbacks[0](...)
                          ↓
                      button_play_pause_        main.c:37
                      callback()
                          ↓
                      audio_controller_         audio_controller.c:240
                      play()
                          ↓
                      Playback thread           audio_controller.c:44
                      empieza a leer WAV
                      y escribir a FIFO
                          ↓
                      fifo_write_samples()      fifo_writer.c:145
                          ↓
                      *fifo->data = sample      fifo_writer.c:153
    ↑                     ↓
FIFO en FPGA     ←────────┘
recibe muestra
    ↓
NIOS II lee
del FIFO
    ↓
Aplica filtros
    ↓
Envía a Audio
Codec
    ↓
🔊 SONIDO!
```

## 💻 Compilación: Cross-Compilation Explicada

### ¿Por qué Cross-Compilation?

Tu PC es probablemente x86_64:
```bash
$ uname -m
x86_64
```

La DE-SoC1 es ARM:
```bash
root@DE1-SoC:~# uname -m
armv7l
```

**No puedes ejecutar binario x86_64 en ARM**, y vice versa.

### Proceso de Compilación

```
En tu PC (x86_64):
───────────────────────────────────────────────────────────
$ make
  ↓
Makefile usa:
  CC = arm-linux-gnueabihf-gcc  ← Cross-compiler
  ↓
Compilador genera código ARM (no x86)
  ↓
  audio_player (binario ARM)

$ file audio_player
audio_player: ELF 32-bit LSB executable, ARM, EABI5 version 1...
                                         ^^^
                                         ARM!

Este binario NO corre en tu PC:
$ ./audio_player
bash: cannot execute binary file: Exec format error
        ↓
Pero SÍ corre en la DE-SoC1:
───────────────────────────────────────────────────────────
En la tarjeta (ARM):

root@DE1-SoC:~# ./audio_player
[INFO] SoC Audio Player - ARM HPS
✓ Ejecuta perfectamente
```

## 🔧 Configuración Necesaria del Linux

### Kernel Modules Requeridos

Tu código necesita que el kernel tenga:

```bash
# Verificar en la tarjeta:
root@DE1-SoC:~# zcat /proc/config.gz | grep -i mem
CONFIG_DEVMEM=y          ← /dev/mem support (CRÍTICO)

root@DE1-SoC:~# zcat /proc/config.gz | grep -i mmc
CONFIG_MMC=y             ← SD card support
CONFIG_MMC_DW=y          ← DesignWare MMC controller

root@DE1-SoC:~# zcat /proc/config.gz | grep -i net
CONFIG_NET=y             ← Networking
CONFIG_INET=y            ← TCP/IP
```

Si alguno falta, tu código no funcionará correctamente.

**El Linux precompilado de Terasic/Intel ya tiene todo esto configurado.**

### Permisos de /dev/mem

```bash
root@DE1-SoC:~# ls -l /dev/mem
crw-r----- 1 root kmem 1, 1 Jan  1 00:00 /dev/mem
          ↑
          Character device (no es archivo regular)
```

**Por eso tu app debe correr como root** (main.c:315):
```c
if (geteuid() != 0) {
    ERROR_PRINT("This application must be run as root");
    return EXIT_FAILURE;
}
```

## 📚 Comparación: Tu Código vs Driver de Kernel

### Tu Código (Userspace)
```c
// audio_player - Userspace application
// Ventajas:
✓ Más fácil de desarrollar y debuggear
✓ Crash no afecta al sistema
✓ Fácil de actualizar (solo copiar binario)
✓ Puede usar bibliotecas estándar (pthread, sockets)

// Desventajas:
✗ Menos eficiente (context switches)
✗ No puede manejar interrupciones de hardware directamente
✗ Requiere /dev/mem (menos seguro)
```

### Driver de Kernel (Alternativa)
```c
// audio_driver.ko - Kernel module
// Ventajas:
✓ Más eficiente
✓ Puede registrar interrupciones
✓ Acceso directo a hardware

// Desventajas:
✗ Más complejo de desarrollar
✗ Crash puede tumbar el sistema
✗ Más difícil de debuggear
✗ Requiere recompilar kernel o cargar módulo
```

**Para este proyecto, userspace es la mejor opción.**

## ✅ Validación: ¿Está Todo Correcto?

### Test 1: Linux Bootea
```bash
# En UART console, debes ver:
[    0.000000] Booting Linux on physical CPU 0x0
...
DE1-SoC login: root
✓ Linux funciona
```

### Test 2: /dev/mem Accesible
```bash
root@DE1-SoC:~# dd if=/dev/mem of=/dev/null bs=1 count=1 2>&1
1+0 records in
✓ /dev/mem funciona
```

### Test 3: Bibliotecas Disponibles
```bash
root@DE1-SoC:~# ls /lib/libpthread*
/lib/libpthread.so.0
✓ libpthread disponible

root@DE1-SoC:~# ldd ./audio_player
libpthread.so.0 => /lib/libpthread.so.0
✓ Todas las dependencias resueltas
```

### Test 4: Red Funciona
```bash
root@DE1-SoC:~# ifconfig eth0
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>
      inet 192.168.1.100
✓ Red configurada
```

### Test 5: SD Card Mounted
```bash
root@DE1-SoC:~# df -h
Filesystem      Size  Used Avail Use% Mounted on
/dev/mmcblk0p2  3.5G  1.2G  2.1G  37% /
✓ Filesystem montado
```

### Test 6: Tu App Ejecuta
```bash
root@DE1-SoC:~# ./audio_player
[INFO] ========================================
[INFO] SoC Audio Player - ARM HPS
✓ Todo funciona!
```

## 🎯 Conclusión

**Relación resumida:**

1. **Linux precompilado** = Sistema operativo base
2. **Tu código** = Aplicación que corre sobre ese Linux
3. **Linux provee** = Syscalls, drivers, filesystem, red
4. **Tu código usa** = esas facilidades para acceder a FPGA y SD card

**No necesitas:**
- ❌ Modificar el kernel
- ❌ Escribir drivers
- ❌ Compilar Linux desde cero
- ❌ Buildroot (como discutimos antes)

**Solo necesitas:**
- ✅ Linux precompilado funcionando
- ✅ Tu aplicación compilada para ARM
- ✅ Archivos WAV en /mnt/sd/music/
- ✅ Ejecutar como root

¡Eso es todo! 🚀
