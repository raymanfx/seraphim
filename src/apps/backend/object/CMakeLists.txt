set(MODULE_NAME object_service)

set(SOURCES
    classifier_service.cpp)

set(HEADERS
    classifier_service.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${MODULE_NAME} PRIVATE seraphim_messages)
target_link_libraries(${MODULE_NAME} PRIVATE seraphim_object)
