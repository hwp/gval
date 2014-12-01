# Author : Weipeng He <heweipeng@gmail.com>
# Copyright (c) 2014, All rights reserved.

add_library(gvalstft SHARED gval_stft.c gval_utils.c)
target_link_libraries(gvalstft ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES} ${GSTREAMER_AUDIO_LIBRARIES}
  ${FFTW_LIBRARIES})

add_library(gvalmfcc SHARED gval_mfcc.c gval_utils.c)
target_link_libraries(gvalmfcc ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES} ${GSTREAMER_AUDIO_LIBRARIES}
  ${FFTW_LIBRARIES})

add_library(gvalkeypoints SHARED gval_keypoints.c gval_cv.cpp)
target_link_libraries(gvalkeypoints ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES} ${GSTREAMER_VIDEO_LIBRARIES}
  ${OPENCV_LIBRARIES})

add_library(gvalsift SHARED gval_sift.c gval_cv.cpp)
target_link_libraries(gvalsift ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES} ${GSTREAMER_VIDEO_LIBRARIES}
  ${OPENCV_LIBRARIES})

install(TARGETS gvalstft gvalmfcc gvalkeypoints gvalsift
  LIBRARY DESTINATION lib/gstreamer-1.0)

add_executable(display display_image.cpp)
target_link_libraries(display ${OPENCV_LIBRARIES})

