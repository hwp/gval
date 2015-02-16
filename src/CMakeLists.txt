# Author : Weipeng He <heweipeng@gmail.com>
# Copyright (c) 2014, All rights reserved.

add_library(gval SHARED 
  gval_plugin.c gval_utils.c gval_cv.cpp gval_stft.c
  gval_mfcc.c gval_keypoints.c gval_sift.c gval_bow.c)
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

add_executable(build_voc build_voc.cpp gval_cv.cpp gval_utils.c)
target_link_libraries(build_voc ${OpenCV_LIBS} ${FFTW_LIBRARIES}
  ${Boost_LIBRARIES})

add_executable(gval_voc_size gval_voc_size.cpp gval_cv.cpp)
target_link_libraries(gval_voc_size ${OpenCV_LIBS})

add_executable(gval_merge gval_merge.c)

add_executable(gval_sum gval_sum.c)

install(TARGETS gval_voc_size gval_merge gval_sum
  DESTINATION bin) 
