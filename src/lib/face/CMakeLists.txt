set(MODULE_NAME seraphim_face)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/module.cpp
    src/lbp_face_detector.cpp
    src/lbp_face_recognizer.cpp
    src/lbf_facemark_detector.cpp
    src/utils.cpp)

set(HEADERS
    include/seraphim/face.h
    include/seraphim/face/module.h
    include/seraphim/face/face_detector.h
    include/seraphim/face/face_recognizer.h
    include/seraphim/face/facemark_detector.h
    include/seraphim/face/lbf_facemark_detector.h
    include/seraphim/face/lbp_face_detector.h
    include/seraphim/face/lbp_face_recognizer.h
    include/seraphim/face/utils.h)

add_library(${MODULE_NAME} SHARED ${SOURCES} ${HEADERS})

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
find_package(OpenCV COMPONENTS opencv_core opencv_face opencv_imgproc REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_face opencv_imgproc)

target_link_libraries(${MODULE_NAME} PUBLIC seraphim_core)

# install shared lib and headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${MODULE_NAME} EXPORT ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# add cmake install target
install(EXPORT ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/seraphim)
