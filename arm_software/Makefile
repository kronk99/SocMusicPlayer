# ============================================================================
# Makefile for SoC Audio Player (ARM HPS)
# ============================================================================
#
# Para compilación cruzada para ARM Cortex-A9 (Cyclone V SoC)
#
# Uso:
#   make              # Compilar
#   make clean        # Limpiar
#   make deploy       # Copiar a tarjeta (requiere TARGET_IP configurado)
#

# ============================================================================
# TOOLCHAIN CONFIGURATION
# ============================================================================

# Para compilación cruzada con SoC EDS de Intel/Altera
CROSS_COMPILE ?= arm-linux-gnueabihf-

# O para toolchain de Linaro (alternativa)
# CROSS_COMPILE ?= arm-linux-gnueabihf-

CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AR      = $(CROSS_COMPILE)ar
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE    = $(CROSS_COMPILE)size

# ============================================================================
# PROJECT STRUCTURE
# ============================================================================

PROJECT_NAME = audio_player
TARGET       = $(PROJECT_NAME)

SRC_DIR     = src
INC_DIR     = include
BUILD_DIR   = build
SCRIPTS_DIR = scripts

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
DEPENDS = $(OBJECTS:.o=.d)

# ============================================================================
# COMPILER FLAGS
# ============================================================================

# Include paths
INCLUDES = -I$(INC_DIR)

# Warning flags
WARNINGS = -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual \
           -Wwrite-strings -Wconversion -Wmissing-prototypes

# Optimization flags
# Para desarrollo: -O0 -g
# Para producción: -O2 -DNDEBUG
OPTFLAGS = -O2 -g

# Architecture-specific flags for ARM Cortex-A9
ARCHFLAGS = -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard

# C standard
CSTD = -std=gnu11

# All CFLAGS
CFLAGS = $(CSTD) $(ARCHFLAGS) $(OPTFLAGS) $(WARNINGS) $(INCLUDES)

# Dependency generation
CFLAGS += -MMD -MP

# ============================================================================
# LINKER FLAGS
# ============================================================================

# Libraries
LIBS = -lpthread -lm

# Linker flags
LDFLAGS = $(ARCHFLAGS) -static

# ============================================================================
# BUILD RULES
# ============================================================================

.PHONY: all clean deploy info help

all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	@echo "Creating build directory..."
	@mkdir -p $(BUILD_DIR)

# Link executable
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete!"
	@echo ""
	@$(SIZE) $@
	@echo ""

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Include dependencies
-include $(DEPENDS)

# ============================================================================
# UTILITY RULES
# ============================================================================

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete!"

# Deploy to target board (set TARGET_IP environment variable)
# Example: make deploy TARGET_IP=192.168.1.100
deploy: $(TARGET)
ifndef TARGET_IP
	@echo "ERROR: TARGET_IP not set"
	@echo "Usage: make deploy TARGET_IP=<board-ip-address>"
	@exit 1
endif
	@echo "Deploying to $(TARGET_IP)..."
	scp $(TARGET) root@$(TARGET_IP):/root/
	@echo "Deploy complete!"
	@echo "Connect to board: ssh root@$(TARGET_IP)"

# Show build info
info:
	@echo "Project:        $(PROJECT_NAME)"
	@echo "Target:         $(TARGET)"
	@echo "Cross Compile:  $(CROSS_COMPILE)"
	@echo "CC:             $(CC)"
	@echo "Sources:        $(SOURCES)"
	@echo "Objects:        $(OBJECTS)"
	@echo "CFLAGS:         $(CFLAGS)"
	@echo "LDFLAGS:        $(LDFLAGS)"
	@echo "LIBS:           $(LIBS)"

# Show help
help:
	@echo "SoC Audio Player - Build System"
	@echo "================================"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the application (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  deploy  - Copy binary to target board (requires TARGET_IP)"
	@echo "  info    - Show build configuration"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                                  # Build"
	@echo "  make clean                            # Clean"
	@echo "  make deploy TARGET_IP=192.168.1.100  # Deploy"
	@echo ""
	@echo "Environment Variables:"
	@echo "  CROSS_COMPILE - Toolchain prefix (default: arm-linux-gnueabihf-)"
	@echo "  TARGET_IP     - Target board IP address (for deploy)"
	@echo ""
