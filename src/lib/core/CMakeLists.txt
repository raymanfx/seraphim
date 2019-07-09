set(MODULE_NAME seraphim_core)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)

set(SOURCES
    src/dummy.cpp)

set(HEADERS
    include/seraphim/core/computable.h
    include/seraphim/core/module.h)

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
find_package(OpenCV COMPONENTS opencv_core REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core)

# install shared lib and headers
install(FILES ${HEADERS} DESTINATION include/seraphim/core)
install(TARGETS ${MODULE_NAME} DESTINATION lib)
