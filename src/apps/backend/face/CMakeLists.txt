set(MODULE_NAME face_service)

set(SOURCES
    face_detector_service.cpp
    face_recognizer_service.cpp
    facemark_detector_service.cpp)

set(HEADERS
    face_detector_service.hpp
    face_recognizer_service.hpp
    facemark_detector_service.hpp)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${MODULE_NAME} PUBLIC backend_common)
target_link_libraries(${MODULE_NAME} PUBLIC seraphim::face seraphim::ipc seraphim::face_messages)
