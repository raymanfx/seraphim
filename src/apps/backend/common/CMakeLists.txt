set(MODULE_NAME backend_common)

set(SOURCES
    utils.cpp)

set(HEADERS
    utils.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${MODULE_NAME} PUBLIC seraphim_iop seraphim_ipc)

# OpenCV dep
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc opencv_imgcodecs REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_imgproc opencv_imgcodecs)
