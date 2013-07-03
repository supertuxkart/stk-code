# Build type STKRelease is similar to Release provided by CMake,
# but it uses a lower optimization level

set(CMAKE_CXX_FLAGS_STKRELEASE "-O2 -DNDEBUG" CACHE STRING
    "Flags used by the C++ compiler during STK release builds."
    FORCE)
set(CMAKE_C_FLAGS_STKRELEASE "-O2 -DNDEBUG" CACHE STRING
    "Flags used by the C compiler during STK release builds."
    FORCE)
set(CMAKE_EXE_LINKER_FLAGS_STKRELEASE
    "" CACHE STRING
    "Flags used for linking binaries during STK release builds."
    FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_STKRELEASE
    "" CACHE STRING
    "Flags used by the shared libraries linker during STK release builds."
    FORCE)

mark_as_advanced(
    CMAKE_CXX_FLAGS_STKRELEASE
    CMAKE_C_FLAGS_STKRELEASE
    CMAKE_EXE_LINKER_FLAGS_STKRELEASE
    CMAKE_SHARED_LINKER_FLAGS_STKRELEASE)

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel STKRelease."
    FORCE)
