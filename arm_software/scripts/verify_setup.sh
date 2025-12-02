#!/bin/bash
#
# verify_setup.sh - Verificar que todo está listo para mañana
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  Setup Verification - SoC Audio Player║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0
WARNINGS=0

# Test functions
test_pass() {
    echo -e "${GREEN}✓ $1${NC}"
    ((TESTS_PASSED++))
}

test_fail() {
    echo -e "${RED}✗ $1${NC}"
    ((TESTS_FAILED++))
}

test_warn() {
    echo -e "${YELLOW}⚠ $1${NC}"
    ((WARNINGS++))
}

echo "Verificando herramientas necesarias..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check ARM toolchain
if command -v arm-linux-gnueabihf-gcc &> /dev/null; then
    VERSION=$(arm-linux-gnueabihf-gcc --version | head -n1)
    test_pass "ARM toolchain instalado: $VERSION"
else
    test_fail "ARM toolchain NO encontrado"
    echo "      Instalar con: sudo apt-get install gcc-arm-linux-gnueabihf"
fi

# Check make
if command -v make &> /dev/null; then
    test_pass "make disponible"
else
    test_fail "make NO encontrado"
fi

# Check minicom/screen
if command -v minicom &> /dev/null; then
    test_pass "minicom disponible (para UART console)"
elif command -v screen &> /dev/null; then
    test_pass "screen disponible (para UART console)"
else
    test_warn "minicom/screen no encontrado (útil para console serial)"
    echo "      Instalar con: sudo apt-get install minicom"
fi

# Check sox (audio tools)
if command -v sox &> /dev/null; then
    test_pass "sox disponible (para convertir audio)"
else
    test_warn "sox no encontrado (útil para crear WAV de prueba)"
    echo "      Instalar con: sudo apt-get install sox"
fi

echo ""
echo "Verificando estructura del proyecto..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check directories
if [ -d "src" ] && [ -d "include" ]; then
    test_pass "Directorios src/ e include/ existen"
else
    test_fail "Estructura de directorios incorrecta"
fi

# Check key files
REQUIRED_FILES=(
    "src/main.c"
    "src/wav_reader.c"
    "src/fifo_writer.c"
    "src/audio_controller.c"
    "include/config.h"
    "Makefile"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        test_pass "Archivo $file existe"
    else
        test_fail "Archivo $file NO encontrado"
    fi
done

echo ""
echo "Intentando compilar código..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Try to compile
if make clean > /dev/null 2>&1 && make > /dev/null 2>&1; then
    test_pass "Compilación exitosa"

    # Check if binary is ARM
    if file audio_player | grep -q "ARM"; then
        test_pass "Binario es ARM (correcto para DE-SoC1)"
    else
        test_fail "Binario NO es ARM"
    fi
else
    test_fail "Compilación falló"
    echo "      Ver errores con: make"
fi

echo ""
echo "Verificando documentación..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

DOCS=(
    "README.md"
    "CRITICAL_NOTES.md"
    "LINUX_SETUP_GUIDE.md"
    "CODE_LINUX_RELATIONSHIP.md"
    "DOWNLOAD_LINKS.md"
    "QUICK_START.md"
)

for doc in "${DOCS[@]}"; do
    if [ -f "$doc" ]; then
        test_pass "$doc disponible"
    else
        test_warn "$doc no encontrado"
    fi
done

echo ""
echo "Verificando permisos..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check if user can access /dev/mem (will need root)
if [ -c "/dev/mem" ]; then
    test_pass "/dev/mem existe (la app lo necesitará)"
    if [ -r "/dev/mem" ]; then
        test_pass "Puedes leer /dev/mem (tienes permisos)"
    else
        test_warn "/dev/mem existe pero sin permisos (OK, usarás sudo)"
    fi
else
    test_warn "/dev/mem no encontrado (normal si no estás en DE-SoC1)"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "RESUMEN"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "Pruebas exitosas: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Pruebas fallidas:  ${RED}$TESTS_FAILED${NC}"
echo -e "Advertencias:      ${YELLOW}$WARNINGS${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ TODO LISTO PARA MAÑANA!            ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    echo ""
    echo "Próximos pasos:"
    echo "1. Lee CRITICAL_NOTES.md esta noche"
    echo "2. Descarga imagen Linux (ver DOWNLOAD_LINKS.md)"
    echo "3. Mañana: bootea Linux en la tarjeta"
    echo "4. Actualiza config.h con direcciones de Platform Designer"
    echo "5. Recompila y prueba!"
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ⚠ FALTAN ALGUNAS COSAS               ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════╝${NC}"
    echo ""
    echo "Revisa los errores de arriba y corrígelos."
    echo "Lo más crítico es tener:"
    echo "  - ARM toolchain instalado"
    echo "  - Código compilando correctamente"
    exit 1
fi
