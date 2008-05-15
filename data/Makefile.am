# data/

SUBDIRS      = fonts herrings karts models music po sfx textures tracks
pkgdatadir = $(datadir)/games/@PACKAGE@/data

pkgdata_DATA =  $(shell find $(srcdir) -name "*.data")        \
                $(shell find $(srcdir) -name "*.herring")     \
                $(shell find $(srcdir) -name "*.projectile")  \
                $(shell find $(srcdir) -name "*.cup")         \
                $(shell find $(srcdir) -name "*.collectable") \
                $(shell find $(srcdir) -name "*.projectile")  \
                CREDITS licenses                              \
                supertuxkart_32.xpm supertuxkart_64.xpm       \
                supertuxkart_desktop.template

desktopdir = $(prefix)/share/applications
desktop_DATA = supertuxkart.desktop

icondir = $(prefix)/share/pixmaps
icon_DATA = supertuxkart_32.xpm supertuxkart_64.xpm

EXTRA_DIST = $(pkgdata_DATA)

# The desktop file needs the absolute path to the binary
# since e.g. /usr/games might not be in the standard path

supertuxkart.desktop: supertuxkart_desktop.template
	cat supertuxkart_desktop.template | sed 's#PREFIX#@prefix@#' >supertuxkart.desktop

