# CMakeLists.txt - enet

include_directories("include")

add_definitions(-DHAS_SOCKLEN_T)

add_library(enet
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

#if(WIN32)
#  find_library(WS2_LIBRARY NAMES "ws2_32" PATHS "C:/Windows/System32")
#  target_link_libraries(enet ${WS2_LIBRARY})
#endif()
