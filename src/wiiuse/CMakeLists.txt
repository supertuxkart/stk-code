# CMakeLists.txt - wiiuse

#include_directories(".")

add_library(wiiuse
	classic.c   events.c         io.c      io_win.c  nunchuk.c
	dynamics.c  guitar_hero_3.c  io_nix.c  ir.c      wiiuse.c
	)
