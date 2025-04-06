#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Default values
BUILD_TYPE="Release"

# Parse command-line arguments
while getopts "D" opt; do
    case "$opt" in
        D) 
            BUILD_TYPE="Debug"
            ;;
        \?) echo "Usage: $0 [-D]"; exit 1 ;;
        *) echo "Usage: $0 [-D]"; exit 1 ;;
    esac
done

conan install . --build=missing -sbuild_type="$BUILD_TYPE"
