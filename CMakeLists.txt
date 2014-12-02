# Author : Weipeng He <heweipeng@gmail.com>
# Copyright (c) 2014, All rights reserved.

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(gval)

set(PACKAGE "gval-package")
set(PACKAGE_NAME "General Video-Audio Learning")
set(PACKAGE_VERSION "1.0.1")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_URL "http://www.github.com/hwp/gval")

add_definitions(-DPACKAGE="${PACKAGE}")
add_definitions(-DPACKAGE_NAME="${PACKAGE_NAME}")
add_definitions(-DVERSION="${PACKAGE_VERSION}")
add_definitions(-DGST_PACKAGE_ORIGIN="${PACKAGE_URL}")

find_package(GStreamer REQUIRED audio video)
include_directories(${GSTREAMER_INCLUDE_DIRS} 
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  ${GSTREAMER_AUDIO_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS})

find_package(GLIB REQUIRED COMPONENTS gobject)
include_directories(${GLIB_INCLUDE_DIRS})

find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})

pkg_check_modules(FFTW REQUIRED fftw3)
include_directories(${FFTW_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "-Wall")

add_subdirectory(src)

