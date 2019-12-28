set(BENCHMARK_NAME benchmark_core)

set(SOURCES
    matrix_ops.cpp)

add_executable(${BENCHMARK_NAME} ${SOURCES})
target_link_libraries(${BENCHMARK_NAME} seraphim::core)
