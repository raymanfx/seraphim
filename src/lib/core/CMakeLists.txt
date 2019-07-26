set(MODULE_NAME seraphim_core)

set(SOURCES
    src/dummy.cpp)

set(HEADERS
    include/seraphim/core/computable.h)

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
find_package(OpenCV COMPONENTS opencv_core REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PRIVATE opencv_core)

# install shared lib and headers
install(FILES ${HEADERS} DESTINATION include/seraphim/core)
install(TARGETS ${MODULE_NAME}_shared DESTINATION lib)
