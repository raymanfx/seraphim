set(MODULE_NAME frontend_common)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    QCameraCaptureStream/QCameraCaptureStream.cpp
    QImageProvider/QImageProvider.cpp
    QVideoCaptureStream/QVideoCaptureStream.cpp)

set(HEADERS
    ICaptureStream/ICaptureStream.h
    QCameraCaptureStream/QCameraCaptureStream.h
    QImageProvider/QImageProvider.h
    QVideoCaptureStream/QVideoCaptureStream.h)

# Linux specific classes
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SOURCES
        ${SOURCES}
        V4L2CaptureStream/V4L2CaptureStream.cpp
        V4L2CaptureStream/v4l2_device.cpp)
    set(HEADERS
        ${HEADERS}
        V4L2CaptureStream/V4L2CaptureStream.h
        V4L2CaptureStream/v4l2_device.h)
endif()

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Qt components
find_package(Qt5 COMPONENTS Core Multimedia Quick REQUIRED)
target_link_libraries(${MODULE_NAME} PRIVATE Qt5::Core Qt5::Multimedia Qt5::Quick)

target_link_libraries(${MODULE_NAME} PRIVATE seraphim::core)
