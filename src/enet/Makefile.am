noinst_LIBRARIES = libenet.a
EXTRA_DIST = ChangeLog configure design.txt Doxyfile LICENSE tutorial.txt
libenet_a_SOURCES = host.c list.c callbacks.c packet.c peer.c protocol.c unix.c win32.c \
	include/enet/types.h \
	include/enet/list.h \
	include/enet/utility.h \
	include/enet/time.h \
	include/enet/callbacks.h \
	include/enet/unix.h \
	include/enet/win32.h \
	include/enet/protocol.h \
	include/enet/enet.h 

AM_CPPFLAGS = -I$(srcdir)/include/

