#!/bin/bash

DIR_NAME="$(dirname $0)"

# デフォルト値
BUILD_TYPE="Release"
BUILD_DIR="."
USE_NINJA=false

# コマンドライン引数の解析
while getopts "Db:n" opt; do
    case "$opt" in
        D)
            BUILD_TYPE="Debug"
            ;;
        b)
            BUILD_DIR="$OPTARG"
            ;;
        n)
            USE_NINJA=true
            ;;
        \?)
            echo "Usage: $0 [-D] [-b build_directory] [-n]"
            exit 1
            ;;
        *)
            echo "Usage: $0 [-D] [-b build_directory] [-n]"
            exit 1
            ;;
    esac
done

conan profile detect --name=blackbox_cpp --force

if $USE_NINJA; then
    echo -e "\n[conf]\ntools.cmake.cmaketoolchain:generator=Ninja" >> ~/.conan2/profiles/blackbox_cpp
fi

echo "Build type: $BUILD_TYPE"
echo "Build directory: $BUILD_DIR"
echo "Using Ninja: $USE_NINJA"

conan install $DIR_NAME --profile blackbox_cpp --build=missing -sbuild_type="$BUILD_TYPE" -of="$BUILD_DIR"
