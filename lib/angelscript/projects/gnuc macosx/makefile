# Angelscript makefile for linux (based on MingW makefile)
# Type 'make' then 'make install' to complete the installation of the library

# For 'make install' to work, set LOCAL according to your system configuration
LOCAL = /usr/local

# If you want to build a shared library, then run make with SHARED=1 and VERSION=version
LIB = libangelscript.a
DEVLIB = libangelscript.dylib
BUNDLE = libangelscript.so
INC = angelscript.h

SRCDIR = ../../source
INCDIR = ../../include
OBJDIR = obj
LIBDIR = ../../lib
CXX = c++
AFLAGS= -arch i386 -arch x86_64
CXXFLAGS = -g -Wall -fPIC $(AFLAGS)
CXXBFLAGS = $(CXXFLAGS) -fno-common  

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
BOBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.cpp=.lo)))
TARG = $(LIBDIR)/$(LIB) $(LIBDIR)/$(DEVLIB) $(LIBDIR)/$(BUNDLE) 


all: $(TARG)
$(LIBDIR)/$(LIB): $(OBJDIR) $(LIBDIR) $(OBJ)
	rm -f $(LIBDIR)/$(LIB)
	ar r $(LIBDIR)/$(LIB) $(OBJ)
	ranlib $(LIBDIR)/$(LIB) 
	file $(LIBDIR)/$(LIB) 

$(LIBDIR)/$(DEVLIB): $(OBJDIR) $(LIBDIR) $(OBJ)
	rm -f $(LIBDIR)/$(DEVLIB)
	$(CXX) $(AFLAGS) -dynamiclib -o $(DEVLIB) $(OBJ)
	mv $(DEVLIB) $(LIBDIR)/$(DEVLIB)
	file  $(LIBDIR)/$(DEVLIB) 

$(LIBDIR)/$(BUNDLE): $(OBJDIR) $(LIBDIR) $(BOBJ)
	rm -f $(LIBDIR)/$(BUNDLE)
	$(CXX) $(AFLAGS) -bundle -flat_namespace -undefined suppress -o $(LIBDIR)/$(BUNDLE) $(BOBJ)
	file  $(LIBDIR)/$(BUNDLE) 

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

$(OBJDIR)/%.lo: $(SRCDIR)/%.cpp
	$(CXX) $(CXXBFLAGS) -o $@ -c $<

$(OBJDIR)/%.lo: $(SRCDIR)/%.S
	$(CXX) $(CXXBFLAGS) -o $@ -c $<

$(OBJDIR)/%.lo: $(SRCDIR)/%.s
	$(CXX) $(CXXBFLAGS) -o $@ -c $<

clean:
	$(DELETER) $(OBJ) $(BOBJ) $(TARG) 

install: $(TARG)
	@echo Installing to: $(LOCAL)/lib and $(LOCAL)/include...
	@echo -------------------------------------------------------------------
ifdef SHARED
	$(COPIER) $(LIBDIR)/$(DEVLIB) $(LOCAL)/lib
endif
	$(COPIER) $(TARG) $(LOCAL)/lib
	$(COPIER) $(INCDIR)/$(INC) $(LOCAL)/include
	@echo -------------------------------------------------------------------
	@echo Angelscript library installed. Enjoy!

uninstall:
	$(DELETER) $(LOCAL)/include/$(INC) $(LOCAL)/lib/$(LIB) $(LOCAL)/lib/$(DEVLIB)
	@echo -------------------------------------------------------------------
	@echo Angelscript library uninstalled.

.PHONY: all clean install uninstall
