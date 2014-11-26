# Author : Weipeng He <heweipeng@gmail.com>
# Copyright (c) 2014, All rights reserved.

add_library(gvalstft SHARED gval_stft.c gval_utils.c)
target_link_libraries(gvalstft ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES} ${FFTW_LIBRARIES})

add_library(gvalmfcc SHARED gval_mfcc.c gval_utils.c)
target_link_libraries(gvalmfcc ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES} ${FFTW_LIBRARIES})

install(TARGETS gvalstft gvalmfcc LIBRARY DESTINATION lib/gstreamer-1.0)

