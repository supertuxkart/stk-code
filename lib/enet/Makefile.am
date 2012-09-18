noinst_LIBRARIES = libenet.a
EXTRA_DIST = ChangeLog configure design.txt Doxyfile LICENSE tutorial.txt CMakeLists.txt\
             $(shell find $(srcdir) -maxdepth 2 -name "*.dox")
libenet_a_SOURCES = callbacks.c compress.c host.c list.c  packet.c peer.c \
        protocol.c unix.c win32.c \
	include/enet/callbacks.h \
	include/enet/enet.h \
	include/enet/list.h \
	include/enet/protocol.h \
	include/enet/time.h \
	include/enet/types.h \
	include/enet/unix.h \
	include/enet/utility.h \
	include/enet/win32.h

AM_CPPFLAGS = -I$(srcdir)/include/
