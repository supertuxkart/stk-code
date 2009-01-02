ACLOCAL_AMFLAGS = -I m4

SUBDIRS = $(BULLETTREE) $(ENETTREE) src doc data

EXTRA_DIST = \
 README.cygwin \
 README.macosx \
 README.windows

pkgdatadir=$(datadir)/doc/$(PACKAGE)
dist_pkgdata_DATA=AUTHORS ChangeLog COPYING README
# NEWS only contains "see changelog"
# CHANGES only contains "see NEW"
