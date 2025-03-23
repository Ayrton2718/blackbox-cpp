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

add_executable(example.out ${CMAKE_SOURCE_DIR}/example/example.cpp)
# target_include_directories(example.out PRIVATE ${CMAKE_INSTALL_PREFIX}/proto)
target_link_libraries(example.out PUBLIC mcap::mcap proto_generated)

# set_target_properties(example.out PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/.)

# Link the executable against the Protobuf libraries
# target_link_libraries(example.out PUBLIC mcap::mcap ${Protobuf_LIBRARIES})