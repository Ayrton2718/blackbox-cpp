#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Default values
BUILD_DIR="build"
FORCE_REBUILD=false
BUILD_TYPE="Release"
DEBUG_MACRO=""

# Parse command-line arguments
while getopts "BDV" opt; do
    case "$opt" in
        B) FORCE_REBUILD=true ;;
        D) 
            BUILD_TYPE="Debug"
            ;;
        V) 
            DEBUG_MACRO="-DCMAKE_VERBOSE_MAKEFILE=ON"
            ;;
        \?) echo "Usage: $0 [-B] [-D] [-V]"; exit 1 ;;
        *) echo "Usage: $0 [-B] [-D] [-V]"; exit 1 ;;
    esac
done

BUILD_TYPE_DIR="$BUILD_DIR/$BUILD_TYPE"
# Conan 2.x places generators in a nested structure
TOOLCHAIN_FILE="$BUILD_TYPE_DIR/build/$BUILD_TYPE/generators/conan_toolchain.cmake"

# Remove build directory if -B option is given
if [ "$FORCE_REBUILD" = true ] && [ -d "$BUILD_TYPE_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_TYPE_DIR"
fi

# Run conan install if toolchain file doesn't exist
if [ ! -f "$TOOLCHAIN_FILE" ]; then
    echo "Conan toolchain not found, running conan install..."
    if [ "$BUILD_TYPE" = "Debug" ]; then
        ./conan_install.sh -D
    else
        ./conan_install.sh
    fi
fi

# Run CMake with the selected build type and optional debug macro
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
      $DEBUG_MACRO \
      -S . -B "$BUILD_TYPE_DIR"

# Build project
cmake --build "$BUILD_TYPE_DIR" --parallel 12
