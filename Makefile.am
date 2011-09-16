ACLOCAL_AMFLAGS = -I m4

SUBDIRS = $(BULLETTREE) $(ENETTREE) src doc data tools

pkgdatadir=$(datadir)/doc/$(PACKAGE)
dist_pkgdata_DATA=AUTHORS ChangeLog COPYING README
EXTRA_DIST=autogen.sh m4

# NEWS only contains "see changelog"
# CHANGES only contains "see NEW"
