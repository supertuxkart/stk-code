# data/gui/

pkgdatadir = $(datadir)/games/$(PACKAGE)/data/gui

nobase_dist_pkgdata_DATA =  $(shell find $(srcdir) -name "*.png"      ) \
                            $(shell find $(srcdir) -name "*.jpg"      ) \
                            $(shell find $(srcdir) -name "*.stkgui"   ) \
                            $(shell find $(srcdir) -name "*.xml"      ) \
                            $(shell find $(srcdir) -name "License.txt") \
                            $(shell find $(srcdir) -name "*.stkskin"  ) 

EXTRA_DIST = $(nobase_pkgdata_DATA) 
