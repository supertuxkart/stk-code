# - Find Wayland
# Find the Wayland includes and libraries
#
# Following variables are provided:
# WAYLAND_FOUND
#     True if Wayland has been found
# WAYLAND_INCLUDE_DIRS
#     The include directories of Wayland
# WAYLAND_LIBRARIES
#     Wayland library list

find_package(PkgConfig REQUIRED)

if(NOT PKGCONFIG_FOUND)
    message(FATAL_ERROR "Pkg-config not found.")
endif()

pkg_check_modules(PKG_WAYLAND QUIET wayland-client)

if(PKG_WAYLAND_FOUND)
    set(WAYLAND_VERSION ${PKG_WAYLAND_VERSION})
else()
    set(WAYLAND_VERSION 0)
endif()

find_path(WAYLAND_CLIENT_INCLUDE_DIR wayland-client.h)
find_path(WAYLAND_CURSOR_INCLUDE_DIR wayland-cursor.h)
find_path(WAYLAND_EGL_INCLUDE_DIR wayland-egl.h)
find_path(XKBCOMMON_INCLUDE_DIR xkbcommon/xkbcommon.h)

find_library(WAYLAND_CLIENT_LIBRARY NAMES wayland-client)
find_library(WAYLAND_CURSOR_LIBRARY NAMES wayland-cursor)
find_library(WAYLAND_EGL_LIBRARY NAMES wayland-egl)
find_library(XKBCOMMON_LIBRARY NAMES xkbcommon)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Wayland DEFAULT_MSG WAYLAND_CLIENT_LIBRARY 
                                                      WAYLAND_CURSOR_LIBRARY
                                                      WAYLAND_EGL_LIBRARY
                                                      XKBCOMMON_LIBRARY
                                                      WAYLAND_CLIENT_INCLUDE_DIR
                                                      WAYLAND_CURSOR_INCLUDE_DIR
                                                      WAYLAND_EGL_INCLUDE_DIR
                                                      XKBCOMMON_INCLUDE_DIR)

set(WAYLAND_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIR} 
                         ${WAYLAND_CURSOR_INCLUDE_DIR} 
                         ${WAYLAND_EGL_INCLUDE_DIR} 
                         ${XKBCOMMON_INCLUDE_DIR})

set(WAYLAND_LIBRARIES ${WAYLAND_CLIENT_LIBRARY} 
                      ${WAYLAND_CURSOR_LIBRARY} 
                      ${WAYLAND_EGL_LIBRARY} 
                      ${XKBCOMMON_LIBRARY})
                      
list(REMOVE_DUPLICATES WAYLAND_INCLUDE_DIRS)
list(REMOVE_DUPLICATES WAYLAND_LIBRARIES)

mark_as_advanced(WAYLAND_CLIENT_LIBRARY 
                 WAYLAND_CURSOR_LIBRARY
                 WAYLAND_EGL_LIBRARY
                 XKBCOMMON_LIBRARY
                 WAYLAND_CLIENT_INCLUDE_DIR
                 WAYLAND_CURSOR_INCLUDE_DIR
                 WAYLAND_EGL_INCLUDE_DIR
                 XKBCOMMON_INCLUDE_DIR)
