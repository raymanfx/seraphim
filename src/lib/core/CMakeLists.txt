set(MODULE_NAME core)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/image.cpp
    src/image_buffer.cpp
    src/image_converter.cpp)

set(HEADERS
    include/seraphim/core/computable.h
    include/seraphim/core/fourcc.h
    include/seraphim/core/image.h
    include/seraphim/core/image_buffer.h
    include/seraphim/core/image_converter.h
    include/seraphim/core/matrix.h
    include/seraphim/core/module.h
    include/seraphim/core/point.h
    include/seraphim/core/polygon.h)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME seraphim_${MODULE_NAME})

# set the shared lib version
set_target_properties(${MODULE_NAME} PROPERTIES VERSION
                      ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION
                      ${MODULE_VERSION_MAJOR})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# install shared lib and headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# add cmake install target
install(EXPORT ${MODULE_NAME} NAMESPACE seraphim:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim)
