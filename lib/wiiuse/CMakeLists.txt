# CMakeLists.txt - wiiuse

cmake_minimum_required(VERSION 2.8.1)

# libbluetooth is required on Unix platforms
if(UNIX AND (NOT APPLE))
    include(FindPkgConfig)
    pkg_check_modules(BLUETOOTH bluez)
    if(NOT BLUETOOTH_FOUND)
        message(FATAL_ERROR "Bluetooth library not found. "
            "Either install libbluetooth or disable wiiuse support with -DUSE_WIIUSE=0")
    endif()
endif()

set(WIIUSE_SOURCES
    classic.c
    dynamics.c
    events.c
    guitar_hero_3.c
    io.c
    ir.c
    motion_plus.c
    nunchuk.c
    os_nix.c
    os_win.c
    util.c
    wiiboard.c
    wiiuse.c
)

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -F/Library/Frameworks")
    set(WIIUSE_SOURCES ${WIIUSE_SOURCES}
        os_mac/os_mac_find.m
        os_mac/os_mac_interface.m
        os_mac/os_mac.m
    )
endif()

if(WIN32)
    add_definitions("/DWIIUSE_STATIC")
    add_library(wiiuse STATIC ${WIIUSE_SOURCES})
else()
    add_library(wiiuse ${WIIUSE_SOURCES})
endif()

if(MSVC)
    if(MSVC90)
        set(WIIUSE_VS_INCLUDE_DIR "C:\\Program\ Files\ (x86)\\Microsoft\ Visual\ Studio\ 9.0\\VC\\include"
            CACHE STRING "VS 9.0 include directory, see lib/wiiuse/README for details")
    elseif(MSVC10)
        set(WIIUSE_VS_INCLUDE_DIR "C:\\Program\ Files\ (x86)\\Microsoft\ Visual\ Studio\ 10.0\\VC\\include"
            CACHE STRING "VS 10.0 include directory, see lib/wiiuse/README for details")
    endif()
    mark_as_advanced(WIIUSE_VS_INCLUDE_DIR)
 
    if(MSVC90 OR MSVC10)
         # VS 9 does not have the windows device driver kit
         # So the driver must be installed additionally. Also, 
         # in order to avoid a compilation error, you have to
         # specify the visual studio include path FIRST (google
         # it, known issue). So in this case add appropriate
         # variables that can be set.
         set(WIIUSE_WINDDK_ROOT "C:/WinDDK" CACHE 
             STRING "Install directory of Windows Driver Kits")
         mark_as_advanced(WIIUSE_WINDDK_ROOT)
         include_directories(${WIIUSE_VS_INCLUDE_DIR} ${WIIUSE_WINDDK_ROOT}/inc/api)
    endif()
    add_library(setupapi.lib STATIC IMPORTED)
    add_library(hid.lib STATIC IMPORTED)
    set_target_properties(setupapi.lib PROPERTIES 
                          IMPORTED_LOCATION ${WIIUSE_WINDDK_ROOT}/lib/win7/i386/setupapi.lib)
    set_target_properties(hid.lib PROPERTIES 
                          IMPORTED_LOCATION ${WIIUSE_WINDDK_ROOT}/lib/win7/i386/hid.lib)
    # This doesn't work for me with VS 11, which apparently finds its own
    # copy of the libs
    if(MSVC90 OR MSVC10)
        set_target_properties(wiiuse PROPERTIES STATIC_LIBRARY_FLAGS 
            "${WIIUSE_WINDDK_ROOT}/lib/win7/i386/setupapi.lib ${WIIUSE_WINDDK_ROOT}/lib/win7/i386/hid.lib")
    else()
        set_target_properties(wiiuse PROPERTIES STATIC_LIBRARY_FLAGS "setupapi.lib hid.lib")
    endif()
elseif(MINGW)
    target_link_libraries(wiiuse hid setupapi)
endif()