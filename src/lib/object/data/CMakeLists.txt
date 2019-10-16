set(MODULE_NAME object_messages)

# compile protobuf code as C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find the protobuf compiler and libraries
find_package(Protobuf REQUIRED)

# Allow imports with relative paths
set(Protobuf_IMPORT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR})

# Find protocol definitions
file(GLOB PROTO_FILES
     ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)

# macOS relies on homebrew to satisfy dependencies.
# Some headers like protobuf "port_def.inc" are not properly exposes at the
# moment, so we have to include them manually for the project.
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    include_directories("/usr/local/opt/protobuf/include")
endif ()

# Generate the .h and .cxx files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_custom_target(${MODULE_NAME}_generated DEPENDS ${PROTO_SRCS} ${PROTO_HDRS})

# Build an object library containing the message classes
add_library(${MODULE_NAME} STATIC ${PROTO_FILES} ${PROTO_SRCS} ${PROTO_HDRS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})
add_dependencies(${MODULE_NAME} ${MODULE_NAME}_generated)
target_link_libraries(${MODULE_NAME} PUBLIC ${PROTOBUF_LIBRARY})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/seraphim/face/data>)

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# Install generated headers
install(FILES ${PROTO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seraphim/face/data)
