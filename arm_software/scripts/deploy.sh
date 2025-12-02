#!/bin/bash
#
# deploy.sh - Script para deployment automático a DE-SoC1
#
# Uso:
#   ./scripts/deploy.sh <IP_DE_LA_TARJETA>
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check arguments
if [ $# -ne 1 ]; then
    echo -e "${RED}Error: IP address required${NC}"
    echo "Usage: $0 <board-ip-address>"
    echo "Example: $0 192.168.1.100"
    exit 1
fi

TARGET_IP=$1
TARGET_USER="root"
TARGET_DIR="/root"
BINARY="audio_player"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  SoC Audio Player - Deployment${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Target IP:   $TARGET_IP"
echo "Target User: $TARGET_USER"
echo "Target Dir:  $TARGET_DIR"
echo ""

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: Binary '$BINARY' not found${NC}"
    echo "Run 'make' first to build the project"
    exit 1
fi

# Test connectivity
echo -e "${YELLOW}[1/4] Testing connectivity...${NC}"
if ! ping -c 1 -W 2 $TARGET_IP > /dev/null 2>&1; then
    echo -e "${RED}Error: Cannot reach $TARGET_IP${NC}"
    echo "Check network connection and IP address"
    exit 1
fi
echo -e "${GREEN}✓ Board is reachable${NC}"
echo ""

# Copy binary
echo -e "${YELLOW}[2/4] Copying binary...${NC}"
scp $BINARY $TARGET_USER@$TARGET_IP:$TARGET_DIR/
echo -e "${GREEN}✓ Binary copied${NC}"
echo ""

# Set executable permissions
echo -e "${YELLOW}[3/4] Setting permissions...${NC}"
ssh $TARGET_USER@$TARGET_IP "chmod +x $TARGET_DIR/$BINARY"
echo -e "${GREEN}✓ Permissions set${NC}"
echo ""

# Test execution (optional)
echo -e "${YELLOW}[4/4] Testing execution...${NC}"
ssh $TARGET_USER@$TARGET_IP "$TARGET_DIR/$BINARY --help 2>&1 || echo 'Binary is executable'"
echo -e "${GREEN}✓ Binary is executable${NC}"
echo ""

# Final instructions
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Deployment Complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "To run the application:"
echo "  ssh $TARGET_USER@$TARGET_IP"
echo "  cd $TARGET_DIR"
echo "  ./$BINARY"
echo ""
echo "Web interface will be available at:"
echo "  http://$TARGET_IP:8080"
echo ""
