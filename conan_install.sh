#!/bin/bash

DIR_NAME="$(dirname $0)"

# Default values
BUILD_TYPE="Release"
USE_NINJA=true

# Parse command-line arguments
while getopts "Dn" opt; do
    case "$opt" in
        D)
            BUILD_TYPE="Debug"
            ;;
        n)
            USE_NINJA=true
            ;;
        \?)
            echo "Usage: $0 [-D] [-n]"
            exit 1
            ;;
        *)
            echo "Usage: $0 [-D] [-n]"
            exit 1
            ;;
    esac
done

# Conan outputs to build/<BuildType>
OUTPUT_DIR="build/$BUILD_TYPE"

conan profile detect --name=blackbox_cpp --force

if $USE_NINJA; then
    echo -e "\n[conf]\ntools.cmake.cmaketoolchain:generator=Ninja" >> ~/.conan2/profiles/blackbox_cpp
fi

echo "Build type: $BUILD_TYPE"
echo "Output directory: $OUTPUT_DIR"
echo "Using Ninja: $USE_NINJA"

conan install "$DIR_NAME" --profile blackbox_cpp --build=missing -s build_type="$BUILD_TYPE" -of="$OUTPUT_DIR"
