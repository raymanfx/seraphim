set(MODULE_NAME car_service)

set(SOURCES
    lane_detector_service.cpp)

set(HEADERS
    lane_detector_service.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${MODULE_NAME} PUBLIC backend_common)
target_link_libraries(${MODULE_NAME} PUBLIC seraphim::car seraphim::ipc seraphim::car_messages)
