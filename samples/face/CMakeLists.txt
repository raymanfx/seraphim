set(SAMPLES
    hog_face_detector
    dnn_face_detector
    kazemi_facemark_detector
    lbp_face_detector
    lbf_facemark_detector)

# OpenCV dependency
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc opencv_videoio REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

foreach(SAMPLE ${SAMPLES})
  add_executable(${SAMPLE} ${SAMPLE}.cpp)
  target_link_libraries(${SAMPLE} common)
  target_link_libraries(${SAMPLE} opencv_core opencv_imgproc opencv_videoio)
  target_link_libraries(${SAMPLE} seraphim::face seraphim::gui seraphim::iop seraphim::object)
endforeach()
