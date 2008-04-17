# data/po/

pkgdatadir = $(datadir)/games/@PACKAGE@/data/po

nobase_pkgdata_DATA = $(shell find $(srcdir) -name "*.po") \
                      $(shell find $(srcdir) -name "*.mo") \
                      supertuxkart.pot

EXTRA_DIST = $(nobase_pkgdata_DATA)