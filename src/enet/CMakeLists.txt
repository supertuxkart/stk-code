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
