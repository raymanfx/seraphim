set(TEST_NAME core_tests)

set(SOURCES
    image.cpp
    image_converter.cpp
    main.cpp
    matrix.cpp
    matrix_ops.cpp
    memory.cpp
    point.cpp
    polygon.cpp
    threading.cpp)

add_executable(${TEST_NAME} ${SOURCES})

target_link_libraries(${TEST_NAME} seraphim::core)

# Include threads
find_package(Threads REQUIRED)
target_link_libraries(${TEST_NAME} Threads::Threads)
