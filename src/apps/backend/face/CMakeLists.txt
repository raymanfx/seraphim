set(MODULE_NAME face_service)

set(SOURCES
    detector_lbp_service.cpp
    recognizer_service.cpp)

set(HEADERS
    detector_lbp_service.h
    recognizer_service.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${MODULE_NAME} PUBLIC backend_common)
target_link_libraries(${MODULE_NAME} PUBLIC seraphim_face)
target_link_libraries(${MODULE_NAME} PUBLIC seraphim_messages)
