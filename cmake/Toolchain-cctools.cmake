# Usage:
# cmake .. -DCCTOOLS_PREFIX=/path/to/cctools -DCCTOOLS_ARCH=arch -DCCTOOLS_PLATFORM=platform \
# -DRT=/path/to/cctools/darwin/libclang_rt.{ios, iossim, osx, tvos, tvossim}.a  -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-cctools.cmake
# Download precompiled cctools at https://github.com/supertuxkart/dependencies/releases/download/preview/cctools.tar.xz
# Compiled in Ubuntu 18.04

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Darwin)

# which compilers to use for C and C++
string(TOLOWER "${CCTOOLS_PLATFORM}" platform_lower)
set(CLANG_ARCH ${CCTOOLS_ARCH})
if(CCTOOLS_ARCH MATCHES ^arm)
  set(CLANG_ARCH arm)
else()
  set(CLANG_ARCH ${CCTOOLS_ARCH})
endif()

SET(CMAKE_C_COMPILER ${CCTOOLS_PREFIX}/bin/${CCTOOLS_ARCH}-${platform_lower}-clang)
SET(CMAKE_CXX_COMPILER ${CCTOOLS_PREFIX}/bin/${CCTOOLS_ARCH}-${platform_lower}-clang++)
SET(CMAKE_RANLIB ${CCTOOLS_PREFIX}/bin/${CLANG_ARCH}-apple-darwin11-ranlib)
SET(CMAKE_AR ${CCTOOLS_PREFIX}/bin/${CLANG_ARCH}-apple-darwin11-ar)
SET(CMAKE_INSTALL_NAME_TOOL ${CCTOOLS_PREFIX}/bin/${CLANG_ARCH}-apple-darwin11-install_name_tool)
SET(CMAKE_SYSTEM_PROCESSOR ${CCTOOLS_ARCH})
SET(CMAKE_MACOSX_RPATH TRUE)
SET(CMAKE_EXE_LINKER_FLAGS ${RT})

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH ${PROJECT_SOURCE_DIR}/dependencies-${platform_lower} ${CCTOOLS_PREFIX}/sdk/${CCTOOLS_PLATFORM}.sdk)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ALWAYS)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# fix statically linking because of order
if (NOT CCTOOLS_PLATFORM MATCHES MacOSX)
  set(CURL_LIBRARY "${PROJECT_SOURCE_DIR}/dependencies-${platform_lower}/lib/libcurl.a;${PROJECT_SOURCE_DIR}/dependencies-${platform_lower}/lib/libmbedtls.a;${PROJECT_SOURCE_DIR}/dependencies-${platform_lower}/lib/libmbedx509.a")
endif()
set(USE_CRYPTO_OPENSSL FALSE CACHE BOOL "")

if (NOT CCTOOLS_PLATFORM MATCHES MacOSX)
set(USE_WIIUSE FALSE CACHE BOOL "")
set(USE_SQLITE3 FALSE CACHE BOOL "")
set(IOS TRUE CACHE BOOL "")
endif()
