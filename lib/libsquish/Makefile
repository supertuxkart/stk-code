include config

VER = 1.15
SOVER = 0

SRC = alpha.cpp clusterfit.cpp colourblock.cpp colourfit.cpp colourset.cpp maths.cpp rangefit.cpp singlecolourfit.cpp squish.cpp

HDR = alpha.h clusterfit.h colourblock.h colourfit.h colourset.h maths.h rangefit.h singlecolourfit.h squish.h
HDR += config.h simd.h simd_float.h simd_sse.h simd_ve.h singlecolourlookup.inl

OBJ = $(SRC:%.cpp=%.o)

SOLIB = libsquish.so.$(SOVER)
LIB = $(SOLIB).0
CPPFLAGS += -fPIC
LIBA = libsquish.a

.PHONY: all install uninstall docs tgz clean

all: $(LIB) $(LIBA) docs libsquish.pc

install: $(LIB) $(LIBA) libsquish.pc
	$(INSTALL_DIRECTORY) $(INSTALL_DIR)/include $(INSTALL_DIR)/$(LIB_PATH)
	$(INSTALL_FILE) squish.h $(INSTALL_DIR)/include
	$(INSTALL_FILE) $(LIBA) $(INSTALL_DIR)/$(LIB_PATH)
ifneq ($(USE_SHARED),0)
	$(INSTALL_FILE) $(LIB) $(INSTALL_DIR)/$(LIB_PATH)
	ln -s $(LIB) $(INSTALL_DIR)/$(LIB_PATH)/$(SOLIB)
	ln -s $(LIB) $(INSTALL_DIR)/$(LIB_PATH)/libsquish.so
	$(INSTALL_DIRECTORY) $(INSTALL_DIR)/$(LIB_PATH)/pkgconfig
	$(INSTALL_FILE) libsquish.pc $(INSTALL_DIR)/$(LIB_PATH)/pkgconfig
endif

uninstall:
	$(RM) $(INSTALL_DIR)/include/squish.h
	$(RM) $(INSTALL_DIR)/$(LIB_PATH)/$(LIBA)
	-$(RM) $(INSTALL_DIR)/$(LIB_PATH)/$(LIB)
	-$(RM) $(INSTALL_DIR)/$(LIB_PATH)/$(SOLIB)
	-$(RM) $(INSTALL_DIR)/$(LIB_PATH)/libsquish.so
	-$(RM) $(INSTALL_DIR)/$(LIB_PATH)/pkgconfig/libsquish.pc

$(LIB): $(OBJ)
ifneq ($(USE_SHARED),0)
	$(CXX) $(LDFLAGS) -shared -Wl,-soname,$(SOLIB) -o $@ $(OBJ)
endif

$(LIBA): $(OBJ)
	$(AR) cr $@ $?
	@ranlib $@

docs: $(SRC) $(HDR)
	@if [ -x "`command -v doxygen`" ]; then doxygen; fi

libsquish.pc: libsquish.pc.in
	@sed 's|@PREFIX@|$(PREFIX)|;s|@LIB_PATH@|$(LIB_PATH)|' $@.in > $@

tgz: clean
	tar zcf libsquish-$(VER).tgz $(SRC) $(HDR) Makefile config CMakeLists.txt CMakeModules libSquish.* README.txt LICENSE.txt ChangeLog.txt Doxyfile libsquish.pc.in extra --exclude \*.svn\*

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -I. $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(OBJ) $(LIB) $(LIBA) libsquish.pc
	@-$(RM) -rf docs
