set(MODULE_NAME iop)

set(HEADERS
    include/seraphim/iop.h)

add_library(${MODULE_NAME} STATIC ${HEADERS})
add_library(seraphim::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(${MODULE_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# OpenCV
find_package(OpenCV COMPONENTS opencv_core)
if (OpenCV_FOUND)
    target_sources(${MODULE_NAME} PRIVATE
        opencv/mat.cpp
        include/seraphim/iop/opencv/mat.h)
    target_link_libraries(${MODULE_NAME} PUBLIC opencv_core opencv_imgproc)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_OPENCV")
endif ()

target_link_libraries(${MODULE_NAME} PUBLIC seraphim::core)

# shared libraries need PIC
set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

# install headers
install(DIRECTORY include/seraphim DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
