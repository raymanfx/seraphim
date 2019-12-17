# protobuf messages
add_subdirectory(data)

set(MODULE_NAME face)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    )

set(HEADERS
    include/seraphim/face.h
    include/seraphim/face/face_detector.h
    include/seraphim/face/face_recognizer.h
    include/seraphim/face/facemark_detector.h)

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

# OpenCV
find_package(OpenCV COMPONENTS opencv_core opencv_face)
if (OpenCV_FOUND)
    target_sources(${MODULE_NAME} PRIVATE
        lbp_face_detector.cpp
        lbp_face_recognizer.cpp
        lbf_facemark_detector.cpp
        utils.cpp
        include/seraphim/face/lbf_facemark_detector.h
        include/seraphim/face/lbp_face_detector.h
        include/seraphim/face/lbp_face_recognizer.h
        include/seraphim/face/utils.h)
    target_compile_definitions(${MODULE_NAME} PUBLIC "-DWITH_OPENCV")
    target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_face)
endif ()

# dlib
find_library(DLIB_LIBRARY dlib)
if (DLIB_LIBRARY_FOUND)
    target_sources(${MODULE_NAME} PRIVATE
        hog_face_detector.cpp
        kazemi_facemark_detector.cpp
        include/seraphim/face/hog_face_detector.h
        include/seraphim/face/kazemi_facemark_detector.h)
    target_compile_definitions(${MODULE_NAME} PUBLIC "-DWITH_DLIB")
    target_link_libraries(${MODULE_NAME} PUBLIC dlib)
endif ()

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core seraphim::iop)

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
