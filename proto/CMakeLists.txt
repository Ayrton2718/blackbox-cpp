find_package(foxglove-schemas-protobuf REQUIRED)

# Set directories for proto files and generated code
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

message(STATUS "PROTO_DIR: ${PROTO_DIR}")
message(STATUS "GEN_DIR: ${GEN_DIR}")

# Collect all .proto files from the proto directory
file(GLOB PROTO_FILES 
    "${PROTO_DIR}/simpleproto/*.proto"
    "${PROTO_DIR}/builtin_interfaces/*.proto"
    "${PROTO_DIR}/std_msgs/*.proto"
    "${PROTO_DIR}/geometry_msgs/*.proto"
)

# Ensure the directory for generated files exists
file(MAKE_DIRECTORY ${GEN_DIR})

# Get protoc executable path based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PROTOC_EXECUTABLE "${protobuf_PACKAGE_FOLDER_DEBUG}/bin/protoc")
else()
    set(PROTOC_EXECUTABLE "${protobuf_PACKAGE_FOLDER_RELEASE}/bin/protoc")
endif()

# Set up protoc target if not already available
if(NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc IMPORTED)
endif()
set_target_properties(protobuf::protoc PROPERTIES IMPORTED_LOCATION "${PROTOC_EXECUTABLE}")

# Create proto_generated library with a placeholder source
# CMake requires at least one source file for add_library
add_library(proto_generated STATIC)

# Generate C++ source files from the .proto files using protobuf_generate
protobuf_generate(
    TARGET proto_generated
    LANGUAGE cpp
    PROTOC_OUT_DIR ${GEN_DIR}
    IMPORT_DIRS ${PROTO_DIR}
    PROTOS ${PROTO_FILES}
)

target_include_directories(proto_generated PUBLIC ${GEN_DIR})
target_link_libraries(proto_generated
    PUBLIC
        foxglove-schemas-protobuf::foxglove-schemas-protobuf
        protobuf::libprotobuf
)
