#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Default values
BUILD_DIR="build"
INSTALL_DIR="install"
FORCE_REBUILD=false

# Parse command-line arguments
while getopts "B" opt; do
    case "$opt" in
        B) FORCE_REBUILD=true ;;
        *) echo "Usage: $0 [-B]"; exit 1 ;;
    esac
done

# Remove build and install directories if -B option is given
if [ "$FORCE_REBUILD" = true ]; then
    if [ -d "$BUILD_DIR" ]; then
        echo "Cleaning previous build..."
        rm -rf "$BUILD_DIR"
    fi

    if [ -d "$INSTALL_DIR" ]; then
        echo "Cleaning previous install..."
        rm -rf "$INSTALL_DIR"
    fi
fi

# Ensure build directory exists
mkdir -p "$BUILD_DIR"
mkdir -p "$INSTALL_DIR"

# Install dependencies using Conan
conan install . --build=missing

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=Release/generators/conan_toolchain.cmake -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_INSTALL_PREFIX="../$INSTALL_DIR"
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="../$INSTALL_DIR"

# Build project
cmake --build . -j4
