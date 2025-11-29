# ===========================
# Toolchain configuration
# ===========================
CROSS_COMPILE ?= arm-linux-gnueabihf-

CC      = $(CROSS_COMPILE)gcc
# TODO: Currently we are not using the following. Should we get rid of them? We'll see
LD      = $(CROSS_COMPILE)ld
AR      = $(CROSS_COMPILE)ar
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE    = $(CROSS_COMPILE)size


# ===========================
# Project structure
# ===========================
PROJECT_NAME = audio_player
TARGET       = $(PROJECT_NAME)

SRC_DIR      = src
INC_DIR      = include
BUILD_DIR    = build

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))
DEPENDS = $(OBJECTS:.o=.d)


# ===========================
# Compiler flags
# ===========================
# Include paths
INCLUDES = -I$(INC_DIR)

# Warning flags
WARNINGS = -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings -Wmissing-prototypes

# Optimization flags
OPTFLAGS = -O0 -g

# Architecture-specific flags for ARM Cortex-A9
ARCHFLAGS = -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard

# C standard
CSTD = -std=gnu11

# All flags
CFLAGS = $(CSTD) $(ARCHFLAGS) $(OPTFLAGS) $(WARNINGS) $(INCLUDES) 

# Dependency generation
CFLAGS += -MMD -MP


# ===========================
# Linker flags
# ===========================
# Libraries
LIBS = -lpthread -lm

# Linker flags
LDFLAGS = $(ARCHFLAGS) -static


# ===========================
# Build rules
# ===========================
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	@echo "Creating build directory..."
	@mkdir -p $(BUILD_DIR)

# Link executable
$(TARGET): $(OBJECTS)
	@echo "Linking $@... 🔗"
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete! 🛠️"
	@echo ""
	@$(SIZE) $@
	@echo ""

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling... $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Include dependencies
-include $(DEPENDS)


# ===========================
# Cleaner
# ===========================
clean:
	@echo "Cleaning... 🧹"
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean completed! ✨"


# ===========================
# Shows project's info
# ===========================
#TODO: info:




# ===========================
# Shows help
# ===========================
help:
	@echo "╔═══════════════════════════════════════╗"
	@echo "║   SoC Audio Player - ARM Cortex-A9    ║"	
	@echo "║             BUILD  SYSTEM             ║"	
	@echo "╚═══════════════════════════════════════╝"
	@echo ""
	@echo "🎯 TARGETS"
	@echo "  ├── all (default)    Build the application"
	@echo "  ├── clean            Remove builded artifacts"
	@echo "  ├── info             Show build configuration"
	@echo "  └── help             Show this help message"


.PHONY: all clean info help
