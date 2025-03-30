
# 親のCMakeLists.txtから受け取った変数を表示
message(STATUS "TARGET_VALUE in sub_project: ${TARGET_VALUE}")

# Set directories for proto files and generated code
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GEN_DIR "${CMAKE_INSTALL_PREFIX}/proto")

message(STATUS "PROTO_DIR in sub_project: ${PROTO_DIR}")
message(STATUS "GEN_DIR in sub_project: ${GEN_DIR}")

# Collect all .proto files from the proto directory
file(GLOB PROTO_FILES "${PROTO_DIR}/foxglove/*.proto")

# Ensure the directory for generated files exists
file(MAKE_DIRECTORY ${GEN_DIR})

add_library(${TARGET_VALUE} STATIC)

if(TARGET protobuf::protoc)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION ${protobuf_PACKAGE_FOLDER_DEBUG}/bin/protoc)
    else()
        set_property(TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION ${protobuf_PACKAGE_FOLDER_RELEASE}/bin/protoc)
    endif()
else()
    add_executable(protobuf::protoc IMPORTED)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION ${protobuf_PACKAGE_FOLDER_DEBUG}/bin/protoc)
    else()
        set_property(TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION ${protobuf_PACKAGE_FOLDER_RELEASE}/bin/protoc)
    endif()
endif()

# Generate C++ source files from the .proto files using protobuf_generate
protobuf_generate(
    TARGET ${TARGET_VALUE}
    LANGUAGE cpp
    PROTOC_OUT_DIR ${GEN_DIR}
    IMPORT_DIRS ${PROTO_DIR}
    PROTOS ${PROTO_FILES}
)

target_include_directories(${TARGET_VALUE} PUBLIC ${GEN_DIR})
target_link_libraries(${TARGET_VALUE} PUBLIC protobuf::libprotobuf)

