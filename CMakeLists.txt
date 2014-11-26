# Author : Weipeng He <heweipeng@gmail.com>
# Copyright (c) 2014, All rights reserved.

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(gval)

set(PACKAGE "gval")
set(PACKAGE_NAME "GVAL")
set(PACKAGE_VERSION "1.0.1")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_URL "http://www.github.com/hwp/gval")

add_definitions(-DVERSION="${PACKAGE_VERSION}")

find_package(GStreamer REQUIRED)
include_directories(${GSTREAMER_INCLUDE_DIRS} ${GSTREAMER_BASE_INCLUDE_DIRS})

find_package(GLIB REQUIRED COMPONENTS gobject)
include_directories(${GLIB_INCLUDE_DIRS})

pkg_check_modules(FFTW REQUIRED fftw3)
include_directories(${FFTW_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "-Wall")

add_subdirectory(src)

