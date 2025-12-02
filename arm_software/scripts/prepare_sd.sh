#!/bin/bash
#
# prepare_sd.sh - Script helper para preparar SD card para DE-SoC1
#
# Este script ayuda a verificar y preparar la SD card
# NO escribe la imagen automáticamente (por seguridad)
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  SD Card Preparation Helper - DE-SoC1 ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Function to check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        echo -e "${RED}Error: This script must be run as root${NC}"
        echo "Usage: sudo $0"
        exit 1
    fi
}

# Function to list available disks
list_disks() {
    echo -e "${YELLOW}Available disks:${NC}"
    lsblk -d -o NAME,SIZE,TYPE,MODEL | grep -v "loop"
    echo ""
}

# Function to show disk info
show_disk_info() {
    local disk=$1
    echo -e "${BLUE}Disk information for $disk:${NC}"
    sudo fdisk -l /dev/$disk 2>/dev/null || echo "Cannot read disk"
    echo ""
}

# Function to verify it's likely a SD card
verify_sd_card() {
    local disk=$1
    local size=$(lsblk -d -n -o SIZE /dev/$disk 2>/dev/null)

    echo -e "${YELLOW}Verifying /dev/$disk...${NC}"
    echo "  Size: $size"

    if lsblk -d -n -o TYPE /dev/$disk | grep -q "disk"; then
        echo -e "  ${GREEN}✓ Device type: disk${NC}"
    else
        echo -e "  ${RED}✗ Not a disk device${NC}"
        return 1
    fi

    # Warn if disk seems large
    local size_gb=$(lsblk -d -n -o SIZE /dev/$disk | sed 's/G.*//')
    if [ "$size_gb" -gt 64 ] 2>/dev/null; then
        echo -e "  ${RED}⚠ WARNING: Disk seems large ($size), might not be SD card!${NC}"
        return 1
    fi

    return 0
}

# Function to check if image file exists
check_image() {
    local image=$1
    if [ ! -f "$image" ]; then
        echo -e "${RED}Error: Image file not found: $image${NC}"
        return 1
    fi

    echo -e "${GREEN}✓ Image file found: $image${NC}"
    local size=$(ls -lh "$image" | awk '{print $5}')
    echo "  Size: $size"
    return 0
}

# Function to show write command
show_write_command() {
    local image=$1
    local disk=$2

    echo ""
    echo -e "${YELLOW}╔════════════════════════════════════════╗${NC}"
    echo -e "${YELLOW}║  READY TO WRITE IMAGE                 ║${NC}"
    echo -e "${YELLOW}╚════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${RED}⚠ WARNING: This will ERASE ALL DATA on /dev/$disk${NC}"
    echo ""
    echo "Verify the information above is correct, then run:"
    echo ""
    echo -e "${GREEN}sudo dd if=$image of=/dev/$disk bs=4M status=progress conv=fsync${NC}"
    echo ""
    echo "This will take 10-20 minutes depending on SD card speed."
}

# Function to mount and prepare partitions
prepare_partitions() {
    local disk=$1

    echo -e "${BLUE}Preparing mounted partitions...${NC}"

    # Create mount points
    mkdir -p /mnt/de_soc_boot
    mkdir -p /mnt/de_soc_root

    # Check if partitions exist
    if [ -b "/dev/${disk}1" ] && [ -b "/dev/${disk}2" ]; then
        echo -e "${GREEN}✓ Partitions found${NC}"

        # Mount
        echo "Mounting partitions..."
        mount /dev/${disk}1 /mnt/de_soc_boot 2>/dev/null || echo "Boot partition already mounted or error"
        mount /dev/${disk}2 /mnt/de_soc_root 2>/dev/null || echo "Root partition already mounted or error"

        echo ""
        echo -e "${YELLOW}Partition contents:${NC}"
        echo "Boot partition:"
        ls -lh /mnt/de_soc_boot/ 2>/dev/null || echo "Cannot list"
        echo ""
        echo "Root partition:"
        ls -lh /mnt/de_soc_root/ 2>/dev/null | head -15
        echo ""

        echo -e "${GREEN}Partitions mounted at:${NC}"
        echo "  /mnt/de_soc_boot"
        echo "  /mnt/de_soc_root"
        echo ""
        echo "You can now:"
        echo "  - Copy audio_player to /mnt/de_soc_root/root/"
        echo "  - Copy .wav files to /mnt/de_soc_root/mnt/sd/music/"
        echo "  - Modify boot files in /mnt/de_soc_boot/"
        echo ""
        echo "When done:"
        echo "  sudo umount /mnt/de_soc_boot"
        echo "  sudo umount /mnt/de_soc_root"
    else
        echo -e "${RED}✗ Partitions not found on /dev/$disk${NC}"
        echo "Write the image first"
    fi
}

# Main menu
main_menu() {
    while true; do
        echo ""
        echo -e "${BLUE}═══════════════════════════════════════${NC}"
        echo "What would you like to do?"
        echo "═══════════════════════════════════════"
        echo "1) List available disks"
        echo "2) Show disk information"
        echo "3) Verify SD card and show write command"
        echo "4) Mount existing SD card partitions"
        echo "5) Unmount partitions"
        echo "6) Exit"
        echo ""
        read -p "Choose option [1-6]: " choice

        case $choice in
            1)
                list_disks
                ;;
            2)
                read -p "Enter disk name (e.g., sdb): " disk
                show_disk_info "$disk"
                ;;
            3)
                read -p "Enter disk name (e.g., sdb): " disk
                read -p "Enter image file path: " image

                if verify_sd_card "$disk" && check_image "$image"; then
                    show_write_command "$image" "$disk"
                fi
                ;;
            4)
                read -p "Enter disk name (e.g., sdb): " disk
                prepare_partitions "$disk"
                ;;
            5)
                echo "Unmounting..."
                umount /mnt/de_soc_boot 2>/dev/null && echo "✓ Boot partition unmounted" || true
                umount /mnt/de_soc_root 2>/dev/null && echo "✓ Root partition unmounted" || true
                ;;
            6)
                echo "Goodbye!"
                exit 0
                ;;
            *)
                echo -e "${RED}Invalid option${NC}"
                ;;
        esac
    done
}

# Entry point
check_root
main_menu
