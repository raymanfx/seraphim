set(MODULE_NAME seraphim_face)

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

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS})

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(${MODULE_NAME}_shared SHARED $<TARGET_OBJECTS:${MODULE_NAME}>)
#add_library(${MODULE_NAME}_static STATIC $<TARGET_OBJECTS:${MODULE_NAME}>)

# set the shared lib name
set_target_properties(${MODULE_NAME}_shared PROPERTIES OUTPUT_NAME ${MODULE_NAME})

# set the shared lib version
set_target_properties(${LIB_NAME} PROPERTIES SOVERSION 0.0.0)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_face opencv_imgproc REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PRIVATE opencv_core opencv_face opencv_imgproc)

# install shared lib and headers
install(FILES ${HEADERS} DESTINATION include/seraphim/face)
install(TARGETS ${MODULE_NAME}_shared DESTINATION lib)