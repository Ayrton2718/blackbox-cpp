cmake_minimum_required(VERSION 3.15)
project(blackbox_cpp)

# Find dependencies
find_package(mcap REQUIRED)
find_package(protobuf REQUIRED)

add_subdirectory(proto)

# set source directory and files
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB SOURCES ${SOURCE_DIR}/*.cpp)

add_library(blackbox ${SOURCES})
target_include_directories(blackbox PRIVATE ${SOURCE_DIR})
target_include_directories(blackbox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(blackbox PUBLIC mcap::mcap proto_generated)

add_executable(example.out ${CMAKE_CURRENT_SOURCE_DIR}/example/example.cpp)
target_link_libraries(example.out PUBLIC blackbox)