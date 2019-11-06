# protobuf messages
add_subdirectory(data)

set(MODULE_NAME object)

set(SOURCES
    dnn_classifier.cpp)

set(HEADERS
    include/seraphim/object.h
    include/seraphim/object/classifier.h
    include/seraphim/object/dnn_classifier.h)

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_dnn)
if (${OpenCV_VERSION} GREATER_EQUAL 4)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_VULKAN")
endif ()

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core seraphim::iop)

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# install headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
