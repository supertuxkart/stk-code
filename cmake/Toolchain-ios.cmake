# Usage:
# cmake .. -DCCTOOLS_PREFIX=/path/to/cctools -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-ios.cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DRT=/path/to/libclang_rt.ios.a

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Darwin)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${CCTOOLS_PREFIX}/bin/arm-apple-darwin11-clang)
SET(CMAKE_CXX_COMPILER ${CCTOOLS_PREFIX}/bin/arm-apple-darwin11-clang++)

SET(CMAKE_C_FLAGS "-mios-version-min=9.0.0 -fembed-bitcode")
SET(CMAKE_CXX_FLAGS "-mios-version-min=9.0.0 -fembed-bitcode")
SET(CMAKE_EXE_LINKER_FLAGS "-mios-version-min=9.0.0 -fembed-bitcode ${RT}")
SET(CMAKE_OSX_SYSROOT ${CCTOOLS_PREFIX}/SDK/iPhoneOS13.2.sdk)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH ${PROJECT_SOURCE_DIR}/dependencies-ios ${CCTOOLS_PREFIX}/SDK/iPhoneOS13.2.sdk)

# manaully set the values which failed to auto detect
set(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies-ios/include/freetype2)
set(CURL_LIBRARY "${PROJECT_SOURCE_DIR}/dependencies-ios/lib/libcurl.a;${PROJECT_SOURCE_DIR}/dependencies-ios/lib/libssl.a")
set(PNG_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies-ios/lib/libpng16.a)
set(SDL2_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies-ios/lib/libSDL2.a)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ALWAYS)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(USE_WIIUSE FALSE CACHE BOOL "")
set(USE_SQLITE3 FALSE CACHE BOOL "")
set(IOS TRUE CACHE BOOL "")
# comment it out to disable logging to debug console
add_definitions(-DENABLE_IOS_DEBUG_PRINT)
