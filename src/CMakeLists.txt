# Author : Weipeng He <heweipeng@gmail.com>
# Copyright (c) 2014, All rights reserved.

add_library(gval SHARED 
  gval_plugin.c gval_utils.c gval_cv.cpp gval_stft.c
  gval_mfcc.c gval_keypoints.c gval_sift.c)
target_link_libraries(gval
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_AUDIO_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}
  ${FFTW_LIBRARIES}
  ${OpenCV_LIBS})

install(TARGETS gval
  LIBRARY DESTINATION lib/gstreamer-1.0)

add_executable(display display_image.cpp)
target_link_libraries(display ${OpenCV_LIBS})

add_executable(matwr_test matwr_test.cpp gval_cv.cpp gval_utils.c)
target_link_libraries(matwr_test ${OpenCV_LIBS} ${FFTW_LIBRARIES})

