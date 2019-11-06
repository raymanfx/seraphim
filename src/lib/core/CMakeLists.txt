set(MODULE_NAME core)

set(SOURCES
    image.cpp
    image_converter.cpp)

set(HEADERS
    include/seraphim/computable.h
    include/seraphim/except.h
    include/seraphim/image.h
    include/seraphim/image_converter.h
    include/seraphim/matrix.h
    include/seraphim/memory.h
    include/seraphim/module.h
    include/seraphim/pixelformat.h
    include/seraphim/point.h
    include/seraphim/polygon.h
    include/seraphim/threading.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# install headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
