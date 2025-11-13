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
