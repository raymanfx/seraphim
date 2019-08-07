set(MODULE_NAME seraphim_core)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/image.cpp
    src/image_buffer.cpp)

set(HEADERS
    include/seraphim/core/computable.h
    include/seraphim/core/image.h
    include/seraphim/core/image_buffer.h
    include/seraphim/core/module.h)

# utilities that depend on 3rdparty libs
find_package(OpenCV COMPONENTS opencv_core)
if (OpenCV_FOUND)
    set(HEADERS ${HEADERS} include/seraphim/core/image_utils_opencv.h)
endif ()
find_package(Qt5 COMPONENTS Multimedia)
if (Qt5_FOUND)
    set(HEADERS ${HEADERS} include/seraphim/core/image_utils_qt.h)
endif ()

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})

# utilities that depend on 3rdparty libs
find_package(OpenCV COMPONENTS opencv_core)
if (OpenCV_FOUND)
    target_link_libraries(${MODULE_NAME} PUBLIC opencv_core)
endif ()
find_package(Qt5 COMPONENTS Multimedia)
if (Qt5_FOUND)
    target_link_libraries(${MODULE_NAME} PUBLIC Qt5::Multimedia)
endif ()

# set the shared lib name
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME})

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

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core)

# install shared lib and headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# add cmake install target
install(EXPORT ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim)
