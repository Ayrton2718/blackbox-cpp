#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Default values
BUILD_DIR="build"
INSTALL_DIR="install"
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
conan install . --build=missing -sbuild_type="$BUILD_TYPE"

# Run CMake with the selected build type and optional debug macro
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -S . -B build/$BUILD_TYPE -G "Unix Makefiles"

# Build project
cmake --build /home/sen/git_tmp/blackbox-cpp/build/$BUILD_TYPE --parallel 12
