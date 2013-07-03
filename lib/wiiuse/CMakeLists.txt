# CMakeLists.txt - wiiuse

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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -F/Library/Frameworks")
    set(WIIUSE_SOURCES ${WIIUSE_SOURCES}
        os_mac/os_mac_find.m
        os_mac/os_mac_interface.m
        os_mac/os_mac.m
    )
endif()

add_library(wiiuse ${WIIUSE_SOURCES})
