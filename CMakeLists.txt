cmake_minimum_required(VERSION 3.15)
project(blackbox_cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BLACKBOX_SCHEMA_DEBUG "Enable schema debug output" OFF)

# Find dependencies
find_package(mcap REQUIRED)
find_package(protobuf REQUIRED)

# Add proto subdirectory
add_subdirectory(proto)

# Define source files explicitly
set(BLACKBOX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bb_blackbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bb_file.cpp
)

# Create blackbox library
add_library(blackbox ${BLACKBOX_SOURCES})
target_include_directories(blackbox
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(blackbox PUBLIC mcap::mcap proto_generated)

if(BLACKBOX_SCHEMA_DEBUG)
    target_compile_definitions(blackbox PRIVATE BLACKBOX_SCHEMA_DEBUG)
endif()

# Create example executable
add_executable(example.out ${CMAKE_CURRENT_SOURCE_DIR}/example/example.cpp)
target_link_libraries(example.out PRIVATE blackbox)
