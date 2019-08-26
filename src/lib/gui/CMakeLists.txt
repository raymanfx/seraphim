set(MODULE_NAME gui)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/module.cpp
    src/gl_window.cpp)

set(HEADERS
    include/seraphim/gui.h
    include/seraphim/gui/module.h
    include/seraphim/gui/window.h
    include/seraphim/gui/glfw.h
    include/seraphim/gui/gl_window.h)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME seraphim_${MODULE_NAME})

# set the shared lib version
set_target_properties(${MODULE_NAME} PROPERTIES VERSION
                      ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION
                      ${MODULE_VERSION_MAJOR})

# GLEW loader
find_package(GLEW REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC GLEW)

# GLFW dependency
find_package(glfw3 REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC glfw)

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# install shared lib and headers
install(DIRECTORY include/seraphim DESTINATION include)
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# add cmake install target
install(EXPORT ${MODULE_NAME} NAMESPACE seraphim:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim)
