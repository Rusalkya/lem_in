#!/bin/bash

# ANSI Color Codes
GREEN="\033[92m"
RED="\033[91m"
YELLOW="\033[93m"
BLUE="\033[94m"
RESET="\033[0m"

echo -e "${BLUE}=== LEM-IN TEST RUNNER ===${RESET}"

# Build the project
echo -e "\n${BLUE}[1/3] Building executables...${RESET}"
make bonus > /dev/null 2>&1
if [ ! -f "./lem-in" ] || [ ! -f "./visu-hex" ]; then
    echo -e "${RED}Error: Build failed. Please check your Makefile.${RESET}"
    exit 1
fi
echo -e "${GREEN}Build successful (lem-in and visu-hex).${RESET}"

# Test valid maps
echo -e "\n${BLUE}[2/3] Running tests on VALID maps...${RESET}"
for map in maps/valid/*.map; do
    basename=$(basename "$map")
    # We use a 2-second timeout to prevent infinite loops
    output=$(timeout 2s ./lem-in < "$map" 2>/dev/null)
    exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo -e "  [${RED}TIMEOUT${RESET}] $basename (inf loop detected)"
    elif [ $exit_code -eq 0 ]; then
        echo -e "  [${GREEN}PASS${RESET}] $basename"
    else
        echo -e "  [${RED}FAIL${RESET}] $basename (Exit code: $exit_code)"
    fi
done

# Test invalid maps
echo -e "\n${BLUE}[3/3] Running tests on INVALID maps...${RESET}"
for map in maps/invalid/*.map; do
    basename=$(basename "$map")
    # Redirect stderr to stdout to catch "ERROR"
    output=$(timeout 2s ./lem-in < "$map" 2>&1)
    exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo -e "  [${RED}TIMEOUT${RESET}] $basename (inf loop/hang detected)"
    elif [ $exit_code -ne 0 ] || echo "$output" | grep -q "ERROR"; then
        echo -e "  [${GREEN}PASS${RESET}] $basename (Correctly rejected/handled)"
    else
        echo -e "  [${YELLOW}WARNING${RESET}] $basename (Allowed through! Exit: $exit_code)"
    fi
done

echo -e "\n${BLUE}=== Bonus Visualization Demo ===${RESET}"
echo -e "To view the bonus visualizer output, you can run:"
echo -e "  ${YELLOW}./visu-hex < maps/valid/00_simple.map${RESET}"
echo -e "Or for a multi-path map:"
echo -e "  ${YELLOW}./visu-hex < maps/valid/04_disjoint_multi.map${RESET}"
echo -e ""
