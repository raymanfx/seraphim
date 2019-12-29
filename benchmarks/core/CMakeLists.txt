set(BENCHMARK_NAME benchmark_core)

set(SOURCES
    main.cpp
    matrix_core_conv.hpp
    matrix_core_ops.hpp)

add_executable(${BENCHMARK_NAME} ${SOURCES})
target_link_libraries(${BENCHMARK_NAME} seraphim::core)
