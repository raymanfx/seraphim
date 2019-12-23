set(SAMPLES
    grayscale
    matrix_ops)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_videoio REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

foreach(SAMPLE ${SAMPLES})
  add_executable(${SAMPLE} ${SAMPLE}.cpp)
  target_link_libraries(${SAMPLE} common)
  target_link_libraries(${SAMPLE} opencv_videoio)
  target_link_libraries(${SAMPLE} seraphim::core seraphim::gui seraphim::iop)
endforeach()
