set(MODULE_NAME seraphim_face)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/lbp_detector.cpp
    src/lbp_recognizer.cpp
    src/utils.cpp)

set(HEADERS
    include/seraphim/face.h
    include/seraphim/face/detector.h
    include/seraphim/face/lbp_detector.h
    include/seraphim/face/lbp_recognizer.h
    include/seraphim/face/recognizer.h
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
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_face opencv_imgproc REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_face opencv_imgproc)

# install shared lib and headers
install(FILES ${HEADERS} DESTINATION include/seraphim/face)
install(TARGETS ${MODULE_NAME} DESTINATION lib)
