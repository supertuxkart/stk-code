dnl Process this file with autogen.sh to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([src/main.cpp])

dnl Require at least automake 2.52
AC_PREREQ(2.52)


dnl Initialize the automake stuff
dnl set the $host variable based on local machine/os
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE(supertuxkart, 0.1rc1)

dnl Check for compilers.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

dnl Set compiler flags
CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-prototypes"
CXXFLAGS="$CXXFLAGS -Wall"

dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL


# specify the plib location
AC_ARG_WITH(plib, [  --with-plib=PREFIX      Specify the prefix path to plib])
if test "x$with_plib" != "x" ; then
   echo "plib prefix is $with_plib"
   EXTRA_DIRS="${EXTRA_DIRS} $with_plib"
fi


# Check for MS Windows environment
AC_CHECK_HEADER(windows.h)


dnl check for OpenGL related libraries
case "${host}" in
*-*-cygwin* | *-*-mingw32*)
    dnl CygWin under Windoze.

    echo Win32 specific hacks...
    AC_DEFINE([WIN32], 1, [Define for Win32 platforms])
    AC_DEFINE([NOMINMAX], 1, [Define for Win32 platforms])

    LIBS="$LIBS -lglut32 -lglu32 -lopengl32 -luser32 -lgdi32 -lm -lwinmm"
    joystick_LIBS="$joystick_LIBS -lwinmm"

    echo "Will link apps with $LIBS"
    ;;

*)
   dnl X-Windows based machines

   AC_SEARCH_LIBS(glNewList, [ GL GLcore MesaGL ])
   if test "x$ac_cv_search_glNewList" = "x-lGLcore"; then
       dnl if GLcore found, then also check for GL
       AC_SEARCH_LIBS(glXCreateContext, GL)
   fi

esac


# The following are C++ items that need to be tested for with the c++
# compiler

AC_LANG_PUSH(C++)

# Check for "plib" without which we cannot go on
AC_CHECK_HEADER(plib/ul.h)
if test "x$ac_cv_header_plib_ul_h" != "xyes"; then
   echo
   echo "You *must* have the plib library installed on your system to build!"
   echo
   echo "Please see README for more details."
   echo
   echo "configure aborted."
   exit
fi

AC_MSG_CHECKING([for plib 1.8.4 or newer])
AC_TRY_RUN([
#include <plib/ul.h>

#define MIN_PLIB_VERSION 184

int main() {
 int major, minor, micro;

 if ( PLIB_VERSION < MIN_PLIB_VERSION ) {
   return -1;
 }

 return 0;
}

],
 AC_MSG_RESULT(yes),
 [AC_MSG_RESULT(wrong version);
  AC_MSG_ERROR([Install plib 1.8.4 or later first...])],
 AC_MSG_RESULT(yes)
)


AC_CONFIG_FILES([      \
 Makefile             \
 data/Makefile        \
 fonts/Makefile       \
 images/Makefile      \
 models/Makefile      \
 mods/Makefile        \
 src/Makefile         \
 wavs/Makefile        \
 wavs/radio/Makefile  \
 wavs/tintagel/Makefile
])
AC_OUTPUT
