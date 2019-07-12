# CMakeLists.txt - enet

include_directories("include")

if(APPLE AND NOT IOS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -F/Library/Frameworks")
endif()

add_definitions(-DHAS_SOCKLEN_T)

add_library(enet STATIC
	callbacks.c
	compress.c
	host.c
	list.c
	packet.c
	peer.c
	protocol.c
	unix.c
	win32.c
)

if(WIN32)
  target_link_libraries(enet wsock32 ws2_32 winmm)
endif()
