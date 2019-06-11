set(COMPONENT_NAME seraphim_messages)

# find the protobuf compiler and libraries
find_package(Protobuf REQUIRED)

# Allow imports with relative paths
set(Protobuf_IMPORT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR})

# Find protocol definitions
file(GLOB PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
message("PROTO_FILES = ${PROTO_FILES}")

# Generate the .h and .cxx files
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Print path to generated files
message("PROTO_FILES = ${PROTO_FILES}")
message("PROTO_SRCS = ${PROTO_SRCS}")
message("PROTO_HDRS = ${PROTO_HDRS}")

# Build a static library containing the message classes
add_library(${COMPONENT_NAME} STATIC ${PROTO_FILES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(${COMPONENT_NAME} ${PROTOBUF_LIBRARY})

# Expose message header
target_include_directories(${COMPONENT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# shared libraries need PIC
set_property(TARGET ${COMPONENT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
