cmake_minimum_required(VERSION 3.15)
project(mcap_serializer)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/Release/generators/conan_toolchain.cmake)  # Conanのツールチェーンを読み込む

# Find dependencies
find_package(mcap REQUIRED)
find_package(protobuf REQUIRED)

set(TARGET_VALUE proto_generated)
add_subdirectory(proto)

# set source directory and files
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
message(STATUS "SOURCE_DIR: ${SOURCE_DIR}")
file(GLOB SOURCES ${SOURCE_DIR}/*.cpp)

add_library(blackbox ${SOURCES})
target_include_directories(blackbox PUBLIC ${SOURCE_DIR})
target_link_libraries(blackbox PUBLIC mcap::mcap proto_generated)

add_executable(example.out ${CMAKE_SOURCE_DIR}/example/example.cpp)
target_link_libraries(example.out PUBLIC mcap::mcap proto_generated)