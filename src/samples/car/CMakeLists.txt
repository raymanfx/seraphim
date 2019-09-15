set(SAMPLES
    linear_lane_detector)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

foreach(SAMPLE ${SAMPLES})
  add_executable(${SAMPLE} ${SAMPLE}.cpp)
  target_link_libraries(${SAMPLE} opencv_core opencv_imgproc opencv_videoio)
  target_link_libraries(${SAMPLE} seraphim::car seraphim::gui seraphim::iop)
endforeach()
