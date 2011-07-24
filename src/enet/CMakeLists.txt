# CMakeLists.txt - enet

include_directories("include")

add_definitions(-DHAS_SOCKLEN_T)

add_library(enet
	callbacks.c  host.c      list.c    peer.c      unix.c
	compress.c   install-sh  packet.c  protocol.c  win32.c
	)
