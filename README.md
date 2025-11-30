# ♫ SocMusicPlayer ــــــــﮩ٨ـ
Proyecto 2 del curso sistemas empotrados: reproductor de musica

## 🛠️ Flujo de Trabajo con Ramas

Este proyecto utiliza una estrategia de branching con tres niveles:

### 🪵 Estructura de Ramas

- **`main`**: Rama principal de producción. Contiene código estable y probado.
- **`Development`**: Rama de desarrollo. Integra todas las features antes de pasar a producción.
- **`feature/<task>*`**: Ramas de trabajo para nuevas funcionalidades o correcciones.

### 🌊 Flujo correcto de trabajo

1. **Crear una rama feature**
   ```bash
   git checkout Development
   git pull origin Development
   git checkout -b feature/<nombre-de-tarea>
   ```

2. **Abrir Pull Request hacia Development**
   - Desde GitHub, abrir un PR de `feature/<nombre-de-tarea>` → `Development`
   - Esperar revisión y aprobación
   - Fusionar mediante el PR

3. **Integrar a main**
   - Abrir un PR de `Development` → `main`
   - Esperar revisión y aprobación
   - Fusionar mediante el PR

### 🚧 Restricciones

- No se permiten push directos a `main` ni `Development`
- No se permiten PRs de `feature/*` directamente a `main`
- Solo ramas `feature/*` pueden fusionarse a `Development`
- Solo `Development` puede fusionarse a `main`
  
Sección de pruebas
En esta sección se describen las pruebas realizadas en simulación con ModelSim para verificar el correcto funcionamiento del sistema de interrupciones del Nios II y los periféricos PIO.

Prueba 1 – Inicialización del sistema

<img width="1915" height="910" alt="Screenshot 2025-11-30 041857" src="https://github.com/user-attachments/assets/6084c760-91fd-4ab6-b770-0aef65107c35" />


En la primera simulación se ejecutó el testbench sin forzar entradas externas, únicamente con el reloj y el reset generados por los BFMs del sistema.
En la ventana Transcript de ModelSim se observa la secuencia de mensajes de los BFMs de reloj y reset, seguida por la salida del programa en C:

=== Sistema inicializado ===

Interrupciones habilitadas.

Esto confirma que:

El procesador Nios II arranca correctamente tras la desaserción de reset.

El software configura los PIO de botones y switches, limpia los registros EDGE_CAP y habilita las interrupciones asociadas antes de entrar en el bucle principal.

Prueba 2 – Interrupciones por botones y switches (0001)

<img width="1915" height="699" alt="Screenshot 2025-11-30 042214" src="https://github.com/user-attachments/assets/657c52a6-4a51-4cf6-ae7c-674accb9864f" />


En la segunda simulación se forzaron las entradas de los BFMs de botones y switches al valor binario 0001.
En el waveform se observan las señales de los BFMs soc_inst_buttons_bfm_conduit_export y soc_inst_switches_bfm_conduit_export en 0001, mientras que el reloj del sistema se mantiene activo de forma continua.

Al ejecutar la simulación durante 0.5 ms con estos valores, en la Transcript aparecen los mensajes:

Botón presionado: 1

Switch cambiado: 1

Esto indica que:

El PIO de botones detecta un flanco en el bit 0, captura el evento en su registro EDGE_CAP y genera una interrupción hacia el Nios II.

La ISR de botones lee EDGE_CAP, limpia el registro y actualiza la variable global buttons_flag, lo que provoca que el bucle principal imprima Botón presionado: 1 y actualice los LEDs.

De forma análoga, el PIO de switches genera una interrupción cuando el bit 0 pasa a 1, la ISR de switches actualiza switches_flag y el bucle principal imprime Switch cambiado: 1, reflejando también el valor en los LEDs.
