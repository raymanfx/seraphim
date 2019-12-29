set(TEST_NAME core_tests)

set(SOURCES
    image.cpp
    image_converter.cpp
    main.cpp
    matrix_core.cpp
    matrix_core_conv.cpp
    matrix_core_ops.cpp
    memory.cpp
    pixel.cpp
    point.cpp
    polygon.cpp
    threading.cpp)

add_executable(${TEST_NAME} ${SOURCES})

target_link_libraries(${TEST_NAME} seraphim::core)

# Include threads
find_package(Threads REQUIRED)
target_link_libraries(${TEST_NAME} Threads::Threads)
