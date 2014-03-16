# Angelscript makefile for linux (based on MingW makefile)
# Type 'make' then 'make install' to complete the installation of the library

# For 'make install' to work, set LOCAL according to your system configuration
LOCAL ?= /usr/local
INSTALL_LIBDIR ?= lib

# If you want to build a shared library, then run make with SHARED=1 and VERSION=version
ifdef SHARED
	LIB = libangelscript-$(VERSION).so
	DEVLIB = libangelscript.so
else
	LIB = libangelscript.a
	DEVLIB = libangelscript.a
endif
INC = angelscript.h

SRCDIR = ../../source
INCDIR = ../../include
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

AR ?= ar
RANLIB ?= ranlib
DELETER = rm -f
COPIER = cp -a

SRCNAMES = \
  as_atomic.cpp \
  as_builder.cpp  \
  as_bytecode.cpp \
  as_callfunc.cpp \
  as_callfunc_arm.cpp \
  as_callfunc_mips.cpp \
  as_callfunc_ppc.cpp \
  as_callfunc_ppc_64.cpp \
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
BIN = $(LIBDIR)/$(LIB)

ifeq ($(TARGETPLATFORM), iphone)
	OBJ += $(OBJDIR)/as_callfunc_arm_xcode.o
else 
	OBJ += $(OBJDIR)/as_callfunc_arm_gcc.o
endif

all: $(BIN)

$(BIN): $(OBJDIR) $(LIBDIR) $(OBJ)
ifdef SHARED
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -Wl,-soname,$(LIB) -o $(BIN) $(OBJ)
else
	$(AR) r $(BIN) $(OBJ)
	$(RANLIB) $(BIN)
endif
	@echo -------------------------------------------------------------------
	@echo Done. As root, type 'make install' to install the library.

$(OBJDIR):
	mkdir $(OBJDIR)

$(LIBDIR):
	mkdir $(LIBDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.S
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(CXX) $(CXXFLAGS) -o $@ -c $<


clean:
	$(DELETER) $(OBJ) $(BIN) $(LIBDIR)/$(DEVLIB)

install: $(BIN)
	@echo Installing to: $(LOCAL)/$(INSTALL_LIBDIR) and $(LOCAL)/include...
	@echo -------------------------------------------------------------------
ifdef SHARED
	$(COPIER) $(LIBDIR)/$(LIB) $(LOCAL)/$(INSTALL_LIBDIR)
	ln -s $(LOCAL)/$(INSTALL_LIBDIR)/$(LIB) $(LOCAL)/$(INSTALL_LIBDIR)/$(DEVLIB)
	ln -s $(LOCAL)/$(INSTALL_LIBDIR)/$(LIB) $(LOCAL)/$(INSTALL_LIBDIR)/$(DEVLIB).$(VERSION)
endif
	$(COPIER) $(BIN) $(LOCAL)/$(INSTALL_LIBDIR)
	$(COPIER) $(INCDIR)/$(INC) $(LOCAL)/include
	@echo -------------------------------------------------------------------
	@echo Angelscript library installed. Enjoy!

uninstall:
	$(DELETER) $(LOCAL)/include/$(INC) $(LOCAL)/$(INSTALL_LIBDIR)/$(LIB) $(LOCAL)/$(INSTALL_LIBDIR)/$(DEVLIB)
	@echo -------------------------------------------------------------------
	@echo Angelscript library uninstalled.

.PHONY: all clean install uninstall
