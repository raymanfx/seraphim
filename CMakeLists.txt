cmake_minimum_required(VERSION 3.6)
include(GNUInstallDirs)

project(Seraphim VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(samples)
add_subdirectory(deploy)

# Generate package config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/seraphim.pc.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/seraphim.pc @ONLY)

# Install cmake config
install(FILES seraphim-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim
    COMPONENT libraries)

# CPack setup
set(CPACK_PACKAGE_NAME "seraphim")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "raymanfx@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Computer Vision interoperability platform")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")
# Components
set(CPACK_COMPONENTS_ALL applications libraries headers)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Seraphim applications")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS libraries)
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)
# RPM
set(CPACK_RPM_COMPONENT_INSTALL ON)

include(CPack)
