# Usage:
# cmake .. -DLLVM_PREFIX=/path/to/llvm-mingw-prefix -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-llvm-mingw.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${LLVM_PREFIX}/bin/i686-w64-mingw32-clang)
SET(CMAKE_CXX_COMPILER ${LLVM_PREFIX}/bin/i686-w64-mingw32-clang++)
SET(CMAKE_RC_COMPILER ${LLVM_PREFIX}/bin/i686-w64-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH ${LLVM_PREFIX}/generic-w64-mingw32 ${LLVM_PREFIX}/i686-w64-mingw32/bin ${PROJECT_SOURCE_DIR}/dependencies-vs)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ALWAYS)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
