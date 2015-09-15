# - Try to find fftw
# Once done this will define
#  FFTW_FOUND - System has fftw
#  FFTW_INCLUDE_DIRS - The fftw include directories
#  FFTW_LIBRARIES - The libraries needed to use fftw
#  FFTW_DEFINITIONS - Compiler switches required for using fftw

find_package(PkgConfig)
pkg_check_modules(PC_FFTW QUIET fftw3)
set(FFTW_DEFINITIONS ${PC_FFTW_CFLAGS_OTHER})

find_path(FFTW_INCLUDE_DIR fftw3.h
          HINTS ${PC_FFTW_INCLUDEDIR} ${PC_FFTW_INCLUDE_DIRS}
          PATH_SUFFIXES fftw3)

find_library(FFTW_LIBRARY NAMES fftw3 
             HINTS ${PC_FFTW_LIBDIR} ${PC_FFTW_LIBRARY_DIRS})

set(FFTW_LIBRARIES ${FFTW_LIBRARY})
set(FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(fftw DEFAULT_MSG
                                  FFTW_LIBRARY FFTW_INCLUDE_DIR)

mark_as_advanced(FFTW_INCLUDE_DIR FFTW_LIBRARY)

