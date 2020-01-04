set(MODULE_NAME gui)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    window_factory.cpp)

set(HEADERS
    include/seraphim/gui.hpp
    include/seraphim/gui/window.hpp
    include/seraphim/gui/window_factory.hpp)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME seraphim_${MODULE_NAME})

# set the shared lib version
set_target_properties(${MODULE_NAME} PROPERTIES VERSION
                      ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set_target_properties(${MODULE_NAME} PROPERTIES SOVERSION
                      ${MODULE_VERSION_MAJOR})

# GLFW dependency
find_package(glfw3)
if (glfw3_FOUND)
    target_sources(${MODULE_NAME} PRIVATE
        glad.c
        glfw_window.cpp
        include/KHR/khrplatform.hpp
        include/seraphim/gui/glad/glad.h
        include/seraphim/gui/glfw.hpp
        include/seraphim/gui/glfw_window.hpp)
    target_compile_definitions(${MODULE_NAME} PRIVATE "-DWITH_GLFW")
    target_link_libraries(${MODULE_NAME} PUBLIC glfw)
endif ()

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_highgui)
if (OpenCV_FOUND)
    target_sources(${MODULE_NAME} PRIVATE
        opencv_window.cpp
        include/seraphim/gui/opencv_window.hpp)
    target_compile_definitions(${MODULE_NAME} PRIVATE "-DWITH_OPENCV")
    target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_highgui seraphim::iop)
endif ()

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core)

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
