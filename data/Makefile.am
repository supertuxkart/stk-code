# data/

pkgdatadir = $(datadir)/games/@PACKAGE@/data

pkgdata_DATA = $(wildcard *.dat) $(wildcard *.data) $(wildcard *.herring)    \
	       $(wildcard *.loc) $(wildcard *.drv?) $(wildcard *.projectile) \
	       $(wildcard *.cup) $(wildcard *.tkkf) $(wildcard *.collectable)\
	       $(wildcard *.track) CREDITS 				     \
	       supertuxkart_32.xpm supertuxkart_64.xpm  supertuxkart.desktop

desktopdir = $(prefix)/share/applications
desktop_DATA = supertuxkart.desktop

icondir = $(prefix)/share/pixmaps
icon_DATA = supertuxkart_32.xpm supertuxkart_64.xpm

EXTRA_DIST = $(pkgdata_DATA)
