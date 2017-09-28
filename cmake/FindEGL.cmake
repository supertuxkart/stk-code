# - Find EGL
# Find the EGL includes and libraries
#
# Following variables are provided:
# EGL_FOUND
#     True if EGL has been found
# EGL_INCLUDE_DIR
#     The include directory of EGL
# EGL_LIBRARY
#     EGL library list

find_path(EGL_INCLUDE_DIR EGL/egl.h)
find_library(EGL_LIBRARY NAMES EGL)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EGL DEFAULT_MSG EGL_LIBRARY EGL_INCLUDE_DIR)

mark_as_advanced(EGL_LIBRARY EGL_INCLUDE_DIR)
