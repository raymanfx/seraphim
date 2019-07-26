# We include OpenCV headers in some classes, so make sure we include their
# locations globally during compilation.
find_package(OpenCV COMPONENTS opencv_core opencv_face REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(COMPONENT_NAME seraphim_services)

set(SOURCES
    dummy.cpp)

set(HEADERS
    )

add_subdirectory(car)
add_subdirectory(face)
add_subdirectory(object)

add_library(${COMPONENT_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${COMPONENT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${COMPONENT_NAME} PRIVATE car_service)
target_link_libraries(${COMPONENT_NAME} PRIVATE face_service)
target_link_libraries(${COMPONENT_NAME} PRIVATE object_service)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_messages)

set(COMPONENT_NAME seraphim-server)

set(SOURCES
    config_store.cpp
    main.cpp
    shm_server.cpp
    tcp_server.cpp)

set(HEADERS
    config_store.h
    server.h
    service.h
    shm_server.h
    tcp_server.h)

add_executable(${COMPONENT_NAME} ${SOURCES} ${HEADERS})

# Include threads
find_package(Threads REQUIRED)
target_link_libraries(${COMPONENT_NAME} PRIVATE Threads::Threads)

target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_core)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_car)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_face)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_ipc)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_messages)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_object)
target_link_libraries(${COMPONENT_NAME} PRIVATE seraphim_services)

install(TARGETS ${COMPONENT_NAME} DESTINATION bin)
