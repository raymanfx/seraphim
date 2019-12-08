set(OPENCV_SAMPLES
    dnn_face_detector
    lbp_face_detector
    lbf_facemark_detector)

set(DLIB_SAMPLES
    hog_face_detector
    kazemi_facemark_detector)

# OpenCV
find_package(OpenCV COMPONENTS opencv_core opencv_face opencv_imgproc opencv_videoio)
if (OpenCV_FOUND)
    foreach (SAMPLE ${OPENCV_SAMPLES})
      add_executable(${SAMPLE} ${SAMPLE}.cpp)
      target_link_libraries(${SAMPLE} common)
      target_link_libraries(${SAMPLE} opencv_core opencv_face opencv_imgproc opencv_videoio)
      target_link_libraries(${SAMPLE} seraphim::face seraphim::gui seraphim::iop seraphim::object)
    endforeach ()
endif ()

# dlib
find_library(DLIB_LIBRARY dlib)
if (DLIB_LIBRARY_FOUND)
    foreach (SAMPLE ${DLIB_SAMPLES})
      add_executable(${SAMPLE} ${SAMPLE}.cpp)
      target_link_libraries(${SAMPLE} common)
      target_link_libraries(${SAMPLE} dlib)
      target_link_libraries(${SAMPLE} seraphim::face seraphim::gui seraphim::iop seraphim::object)
    endforeach ()
endif ()
