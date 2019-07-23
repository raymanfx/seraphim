set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(COMPONENT_NAME seraphim-client-face)

set(SOURCES
    main.cpp
    FaceStorage/FaceStorage.cpp
    ui/MainWindow.cpp)

set(HEADERS
    FaceStorage/FaceStorage.h
    ui/MainWindow.h)

# add resources
set(SOURCES
    ${SOURCES}
    qml.qrc)

add_executable(${COMPONENT_NAME} ${SOURCES} ${HEADERS})

# Include threads
find_package(Threads REQUIRED)
target_link_libraries(${COMPONENT_NAME} PRIVATE Threads::Threads)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc REQUIRED)
target_link_libraries(${COMPONENT_NAME} PRIVATE opencv_core opencv_imgproc)

# Qt components
find_package(Qt5 COMPONENTS Core Quick QuickControls2 REQUIRED)
target_link_libraries(${COMPONENT_NAME} PRIVATE Qt5::Core Qt5::Quick Qt5::QuickControls2)

target_link_libraries(${COMPONENT_NAME} PRIVATE frontend_common)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_core)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_ipc)

install(TARGETS ${COMPONENT_NAME} DESTINATION bin)
