set(TEST_NAME core_tests)

set(SOURCES
    image.cpp
    image_converter.cpp
    main.cpp
    matrix.cpp
    point.cpp
    polygon.cpp)

add_executable(${TEST_NAME} ${SOURCES})

target_link_libraries(${TEST_NAME} Catch2::Catch2)

target_link_libraries(${TEST_NAME} seraphim::core)
