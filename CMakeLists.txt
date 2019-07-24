cmake_minimum_required(VERSION 3.1)

project(Seraphim VERSION 0.0.0)

# global compiler flags
string(CONCAT CMAKE_CXX_FLAGS
              "${CMAKE_CXX_FLAGS}"
              " -Werror=switch"
              " -Werror=implicit-fallthrough")

add_subdirectory(src)
add_subdirectory(deploy)

# Generate package config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/seraphim.pc.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/seraphim.pc @ONLY)
