include(/opt/devkitpro/devkita64.cmake)

set (DKA_SWITCH_C_FLAGS "-D__SWITCH__ -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -ftls-model=local-exec -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS   "${DKA_SWITCH_C_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${DKA_SWITCH_C_FLAGS}" CACHE STRING "")
set(CMAKE_ASM_FLAGS "${DKA_SWITCH_C_FLAGS}" CACHE STRING "")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-specs=${DEVKITPRO}/libnx/switch.specs")

set(CMAKE_FIND_ROOT_PATH
  ${DEVKITPRO}/devkitA64
  ${DEVKITPRO}/tools
  ${DEVKITPRO}/portlibs/switch
  ${DEVKITPRO}/libnx
)

# Set pkg-config for the same
find_program(PKG_CONFIG_EXECUTABLE NAMES aarch64-none-elf-pkg-config HINTS "${DEVKITPRO}/portlibs/switch/bin")
if (NOT PKG_CONFIG_EXECUTABLE)
   message(WARNING "Could not find aarch64-none-elf-pkg-config: try installing switch-pkg-config")
endif()

set(NSWITCH TRUE)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(NX_ROOT ${DEVKITPRO}/libnx)

set(NX_STANDARD_LIBRARIES "${NX_ROOT}/lib/libnx.a")
set(CMAKE_C_STANDARD_LIBRARIES "${NX_STANDARD_LIBRARIES}" CACHE STRING "")
set(CMAKE_CXX_STANDARD_LIBRARIES "${NX_STANDARD_LIBRARIES}" CACHE STRING "")
set(CMAKE_ASM_STANDARD_LIBRARIES "${NX_STANDARD_LIBRARIES}" CACHE STRING "")

#for some reason cmake (3.14.3) doesn't appreciate having \" here
set(NX_STANDARD_INCLUDE_DIRECTORIES "${NX_ROOT}/include")
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${NX_STANDARD_INCLUDE_DIRECTORIES}" CACHE STRING "")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "${NX_STANDARD_INCLUDE_DIRECTORIES}" CACHE STRING "")
set(CMAKE_ASM_STANDARD_INCLUDE_DIRECTORIES "${NX_STANDARD_INCLUDE_DIRECTORIES}" CACHE STRING "")
