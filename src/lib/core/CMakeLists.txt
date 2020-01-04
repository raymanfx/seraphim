set(MODULE_NAME core)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    image.cpp
    image_converter.cpp)

set(HEADERS
    include/seraphim/computable.hpp
    include/seraphim/core.hpp
    include/seraphim/except.hpp
    include/seraphim/image.hpp
    include/seraphim/image_converter.hpp
    include/seraphim/matrix.hpp
    include/seraphim/matrix/matrix_core.hpp
    include/seraphim/matrix/matrix_core_conv.hpp
    include/seraphim/matrix/matrix_core_ops.hpp
    include/seraphim/memory.hpp
    include/seraphim/module.hpp
    include/seraphim/pixel.hpp
    include/seraphim/pixelformat.hpp
    include/seraphim/point.hpp
    include/seraphim/polygon.hpp
    include/seraphim/size.hpp
    include/seraphim/threading.hpp)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME seraphim_${MODULE_NAME})

# set the shared lib version
set_target_properties(${MODULE_NAME} PROPERTIES VERSION
                      ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION
                      ${MODULE_VERSION_MAJOR})

# BLAS acceleration
find_package(BLAS)
if (BLAS_FOUND)
    target_compile_definitions(${MODULE_NAME} PUBLIC "-DWITH_BLAS")
    target_link_libraries(${MODULE_NAME} PUBLIC ${BLAS_LIBRARIES})
endif ()

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# install shared lib and headers
install(DIRECTORY include/seraphim
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries)

# add cmake install target
install(EXPORT ${MODULE_NAME} NAMESPACE seraphim::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim
    COMPONENT libraries)
