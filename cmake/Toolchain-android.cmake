# Cross-compiling requires CMake 2.6 or newer. Example:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=../XCompile-Android.txt -DHOST=arm-linux-androideabi
# Where 'arm-linux-androideabi' is the host prefix for the cross-compiler. If
# you already have a toolchain file setup, you may use that instead of this
# file. Make sure to set CMAKE_FIND_ROOT_PATH to where the NDK toolchain was
# installed (e.g. "$ENV{HOME}/toolchains/arm-linux-androideabi-r10c-21").

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER "${HOST}-gcc")
SET(CMAKE_CXX_COMPILER "${HOST}-g++")
SET(CMAKE_RC_COMPILER "${HOST}-windres")

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH $ENV{NDK_TOOLCHAIN_PATH})

# here is where stuff gets installed to
SET(CMAKE_INSTALL_PREFIX "${CMAKE_FIND_ROOT_PATH}" CACHE STRING "Install path prefix, prepended onto install directories." FORCE)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# set env vars so that pkg-config will look in the appropriate directory for
# .pc files (as there seems to be no way to force using ${HOST}-pkg-config)
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
