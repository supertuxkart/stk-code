# AngelScript makefile for Linux.
# Type 'make' then 'make install' to complete
# the installation of the library. You no
# longer have to specify SHARED=1 VERSION=x.y.z
# (the Makefile automatically determines it
# and builds it and the static library).
# See README for how to use the shared library
# instead of the static. The README also
# contains other information and in particular
# specifies on how to override the install
# location should you desire this (you don't
# have to - nor should you - edit this
# file).
#
# One note: I don't have a way to test
# the phone builds. I am an old-timer
# and I _still_ miss customer-owned
# coin-operated telephones. In fact
# I still _miss_ the rotary telephone!


# Use export CXXFLAGS="{options}" before running make to set type of compilation:
#
# For debugging I suggest using: -O0 -ggdb
# For release I suggest using: -O2


## library names and versions
LIBNAME=libangelscript
AS_VER:=$(shell awk -F\" '/\#define ANGELSCRIPT_VERSION_STRING/{print $$2}' ../../include/angelscript.h | cut -d" " -f1)
SHLIB=$(LIBNAME).so.$(AS_VER)
ARLIB=$(LIBNAME).a


## install directories
ifeq ($(PREFIX),)
PREFIX=/usr/local
endif
INCLUDEDIR_DEST=$(PREFIX)/include
LIBDIR_DEST=$(PREFIX)/lib
DOCDIR_BASEDIR=$(PREFIX)/share/doc/angelscript-$(AS_VER)
DOXYGEN_DEST=$(DOCDIR_BASEDIR)/html
SAMPLES_DEST=$(DOCDIR_BASEDIR)/samples


## install commands
INSTALL = install
INSTALL_DIR = $(INSTALL) -d
INSTALL_SHLIB = $(INSTALL) -m 755
INSTALL_ARLIB = $(INSTALL) -m 644
INSTALL_HEADER = $(INSTALL) -m 644
CP_SYMLINK = cp --no-dereference --preserve=links
CP_R = cp -R


HEADER = angelscript.h
SRCDIR = ../../source
INCDIR = ../../include


## platform specific settings
ifeq ($(TARGETPLATFORM), iphone)
	IPHONEBIN =  /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin
	OBJDIR = obj-iphone
	LIBDIR = ../../lib-iphone
	CXX ?= $(IPHONEBIN)/clang++
	CXXFLAGS += -Wall -fPIC -fno-strict-aliasing -arch armv7 -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.0.sdk -miphoneos-version-min=3.0
else ifeq ($(TARGETPLATFORM), iphonesimulator)
	IPHONEBIN =  /Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin
	OBJDIR = obj-iphone
	LIBDIR = ../../lib-iphone
	CXX ?= $(IPHONEBIN)/clang++
	CXXFLAGS += -Wall -fPIC -fno-strict-aliasing -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.0.sdk -miphoneos-version-min=3.0
else ifeq ($(TARGETPLATFORM), android)
	ANDROIDNDKROOT = /cygdrive/c/android/android-ndk-1.6_r1
	ANDROIDBIN = $(ANDROIDNDKROOT)/build/prebuilt/windows/arm-eabi-4.2.1/bin
	SYSROOT = $(ANDROIDNDKROOT)/build/platforms/android-4/arch-arm
	OBJDIR = obj-android
	LIBDIR = ../../lib-android
	CXX ?= $(ANDROIDBIN)/arm-eabi-gcc
	CXXFLAGS += -I$(SYSROOT)/usr/include \
	-Wall \
	-DANDROID \
	-fno-exceptions \
	-march=armv6 -mthumb-interwork \
	-mfloat-abi=softfp -fno-rtti
else
	OBJDIR = obj
	LIBDIR = ../../lib
	CXX ?= g++
	# On i686 architecture you may need to add -march=i686 if you get 
	# an undefined symbol for __sync_sub_and_fetch_4 in as_atomic.cpp.
	CXXFLAGS += -Wall -fPIC -fno-strict-aliasing
endif

## Detect if targetting ARM CPU and if so tell assembler to accept implicit IT constructs in thumb mode
GCC_ARCH := $(shell $(CXX) -dumpmachine)
$(info GCC ARCH: $(GCC_ARCH))
ifneq (,$(findstring arm-,$(GCC_ARCH)))
	CXXFLAGS += -Wa,-mimplicit-it=thumb
else ifneq (,$(findstring armv7-, $(GCC_ARCH)))
	CXXFLAGS += -Wa,-mimplicit-it=thumb
endif

## toolchain
AR ?= ar
RANLIB ?= ranlib


SRCNAMES = \
  as_atomic.cpp \
  as_builder.cpp  \
  as_bytecode.cpp \
  as_callfunc.cpp \
  as_callfunc_arm.cpp \
  as_callfunc_arm64.cpp \
  as_callfunc_mips.cpp \
  as_callfunc_ppc.cpp \
  as_callfunc_ppc_64.cpp \
  as_callfunc_riscv64.cpp \
  as_callfunc_sh4.cpp \
  as_callfunc_x86.cpp \
  as_callfunc_x64_gcc.cpp \
  as_callfunc_x64_mingw.cpp \
  as_compiler.cpp \
  as_context.cpp \
  as_configgroup.cpp \
  as_datatype.cpp \
  as_generic.cpp \
  as_gc.cpp \
  as_globalproperty.cpp \
  as_memory.cpp \
  as_module.cpp \
  as_objecttype.cpp \
  as_outputbuffer.cpp \
  as_parser.cpp \
  as_restore.cpp \
  as_scriptcode.cpp \
  as_scriptengine.cpp \
  as_scriptfunction.cpp \
  as_scriptnode.cpp \
  as_scriptobject.cpp \
  as_string.cpp \
  as_string_util.cpp \
  as_thread.cpp \
  as_tokenizer.cpp \
  as_typeinfo.cpp \
  as_variablescope.cpp \

OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.o)))

ifeq ($(TARGETPLATFORM), iphone)
	OBJ += $(OBJDIR)/as_callfunc_arm_xcode.o
else
	OBJ += $(OBJDIR)/as_callfunc_arm_gcc.o
	OBJ += $(OBJDIR)/as_callfunc_arm64_gcc.o
	OBJ += $(OBJDIR)/as_callfunc_riscv64_gcc.o
endif


default: all

all: shared static

shared: $(LIBDIR)/$(SHLIB) $(LIBDIR)/$(LIBNAME).so

static: $(LIBDIR) $(OBJDIR) $(LIBDIR)/$(ARLIB)

$(OBJDIR):
	mkdir -p "$(OBJDIR)"

$(LIBDIR):
	mkdir -p "$(LIBDIR)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.S
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(LIBDIR)/$(SHLIB): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -Wl,-soname,$(SHLIB) -o "$(LIBDIR)"/$(SHLIB) $(OBJ)

$(LIBDIR)/$(LIBNAME).so: $(LIBDIR)/$(SHLIB)
	@( cd "$(LIBDIR)" && ln -s $(SHLIB) $(LIBNAME).so )

$(LIBDIR)/$(ARLIB): $(OBJ)
	$(AR) r "$(LIBDIR)"/$(ARLIB) $(OBJ)
	$(RANLIB) "$(LIBDIR)"/$(ARLIB)


## install rules
install_header: $(INCDIR)/$(HEADER)
	$(INSTALL_DIR) "$(DESTDIR)/$(INCLUDEDIR_DEST)"
	$(INSTALL_HEADER) "$(INCDIR)"/$(HEADER) "$(DESTDIR)/$(INCLUDEDIR_DEST)/"$(HEADER)

install_shared:
	$(INSTALL_DIR) "$(DESTDIR)/$(LIBDIR_DEST)"
	$(INSTALL_SHLIB) "$(LIBDIR)"/$(SHLIB) "$(DESTDIR)/$(LIBDIR_DEST)/"$(SHLIB)
	$(CP_SYMLINK) "$(LIBDIR)"/$(LIBNAME).so "$(DESTDIR)/$(LIBDIR_DEST)/"$(LIBNAME).so

install_static: $(LIBDIR)/$(ARLIB) $(LIBDIR)/$(SHLIB)
	$(INSTALL_DIR) "$(DESTDIR)/$(LIBDIR_DEST)"
	$(INSTALL_ARLIB) "$(LIBDIR)"/$(ARLIB) "$(DESTDIR)/$(LIBDIR_DEST)/"$(ARLIB)

install_docs: ./../../../docs
	$(INSTALL_DIR) "$(DESTDIR)/$(DOCDIR_BASEDIR)"
	$(CP_R) ./../../../docs "$(DESTDIR)/$(DOXYGEN_DEST)"

install_samples: ./../../../samples
	$(INSTALL_DIR) "$(DESTDIR)/$(DOCDIR_BASEDIR)"
	$(CP_R) ./../../../samples "$(DESTDIR)/$(SAMPLES_DEST)"

install_all: install_docs install_samples install

install: install_header install_shared install_static

uninstall:
	rm -f "$(DESTDIR)/$(INCLUDEDIR_DEST)/$(HEADER)" "$(DESTDIR)/$(LIBDIR_DEST)"/$(LIBNAME)*

help:
	@echo -------------------------------------------------------------------
	@echo 'BUILDING:'
	@echo '  make all: build shared and static libs'
	@echo '  make shared: build shared lib only'
	@echo '  make static: build static lib only'
	@echo
	@echo 'INSTALLING:'
	@echo '  make install: install headers, shared and static libs'
	@echo '  make install_header: install only the headers'
	@echo '  make install_shared: install only the shared libs'
	@echo '  make install_static: install only the static libs'
	@echo '  make install_docs: install only the documentation'
	@echo '  make install_samples: install only the samples'
	@echo '  make install_all: install everything, including docs and samples'
	@echo
	@echo 'PARAMETERS (pass to make, as in PARAM=value):'
	@echo '  PREFIX: installation prefix (default /usr/local)'
	@echo '  INCLUDEDIR_DEST: where to install headers (default PREFIX/include)'
	@echo '  LIBDIR_DEST: where to install libraries (default PREFIX/lib)'
	@echo '  DOCDIR_BASEDIR: where the basedir of the documentation lies'
	@echo '                  (default PREFIX/share/doc/angelscript-AS_VER)'
	@echo '  DOXYGEN_DEST: where to install doxygen documentation'
	@echo '                (default DOCDIR_BASEDIR/html)'
	@echo '  SAMPLES_DEST: where to install samples'
	@echo '                (default DOCDIR_BASEDIR/samples)'
	@echo '  DESTDIR: destination, prepended to PREFIX, usually used by'
	@echo '           package managers (default empty)'
	@echo -------------------------------------------------------------------

clean:
	rm -f $(OBJ) "$(LIBDIR)"/$(ARLIB) "$(LIBDIR)"/$(SHLIB) "$(LIBDIR)"/$(LIBNAME).so

.PHONY: all clean install install_all install_docs install_header install_samples install_shared install_static shared static uninstall
