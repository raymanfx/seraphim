# protobuf messages
add_subdirectory(data)

set(MODULE_NAME face)

set(SOURCES
    hog_face_detector.cpp
    kazemi_facemark_detector.cpp
    lbp_face_detector.cpp
    lbp_face_recognizer.cpp
    lbf_facemark_detector.cpp
    utils.cpp)

set(HEADERS
    include/seraphim/face.h
    include/seraphim/face/face_detector.h
    include/seraphim/face/face_recognizer.h
    include/seraphim/face/facemark_detector.h
    include/seraphim/face/hog_face_detector.h
    include/seraphim/face/kazemi_facemark_detector.h
    include/seraphim/face/lbf_facemark_detector.h
    include/seraphim/face/lbp_face_detector.h
    include/seraphim/face/lbp_face_recognizer.h
    include/seraphim/face/utils.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_face opencv_imgproc REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_face opencv_imgproc)

# dlib dependency
find_library(dlib REQUIRED)
target_link_libraries(${MODULE_NAME} PUBLIC cblas lapack dlib)

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core seraphim::iop)

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# install headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
