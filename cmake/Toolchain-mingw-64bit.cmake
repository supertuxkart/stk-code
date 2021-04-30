# Usage:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw-64bit.cmake 

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# figure out folder to look in
execute_process(COMMAND sh -c "ls /usr/lib/gcc/x86_64-w64-mingw32/ | grep posix | tr -d '\n'" OUTPUT_VARIABLE MINGW_DEPS_FOLDER)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32 /usr/lib/gcc/x86_64-w64-mingw32/${MINGW_DEPS_FOLDER}/ ${PROJECT_SOURCE_DIR}/dependencies-win-x86_64)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ALWAYS)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
