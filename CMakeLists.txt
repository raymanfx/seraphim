cmake_minimum_required(VERSION 3.3)
include(GNUInstallDirs)

project(Seraphim VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# global compiler flags
string(CONCAT CMAKE_CXX_FLAGS
              "${CMAKE_CXX_FLAGS}"
              " -Werror=switch"
              " -Werror=implicit-fallthrough")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(samples)
add_subdirectory(deploy)

# Generate package config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/seraphim.pc.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/seraphim.pc @ONLY)

# Install cmake config
install(FILES seraphim-config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim)
