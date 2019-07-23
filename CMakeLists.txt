cmake_minimum_required(VERSION 3.1)

project(Seraphim)

add_subdirectory(src)
add_subdirectory(deploy)

# Generate package config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/seraphim.pc.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/seraphim.pc @ONLY)
