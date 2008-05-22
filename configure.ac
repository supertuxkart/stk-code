dnl Process this file with autogen.sh to produce a configure script.

AC_INIT(supertuxkart, SVN)
AC_CONFIG_SRCDIR([src/main.cpp])

dnl Require at least automake 2.59
AC_PREREQ(2.59)

dnl ================================================
dnl Initialize the automake stuff
dnl set the $host variable based on local machine/os
dnl ================================================
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([tar-ustar])


dnl =====================================================
dnl Set the default CFLAGS/CXXFLAGS
dnl If these were not set, the AC_PROG_CC and AC_PROG_CXX
dnl scripts set them to "-g -O2".
dnl =====================================================
CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-prototypes"
CXXFLAGS="$CXXFLAGS -Wall -DHAS_GETTEXT"

dnl ===================
dnl Check for compilers
dnl ===================
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_LANG(C++)

# Do a quick check to ensure the c++ compiler works
AC_RUN_IFELSE([
int main()  { return 0; }
],[],
[
   echo
   echo "Compiling a simple program failed, so something is broken with"
   echo "the building process. Please check that your system is setup"
   echo "so it can compile C++ code."
   echo
   echo "Configure aborted."
   exit
])

dnl =============================================
dnl Define debug which enables some cheats
dnl This should be DISABLED when doing a release!
dnl =============================================
SUMMARY="\nSummary of optional components:\n==============================="

if test "x$with_plib" != "x" ; then
   echo "plib prefix is $with_plib"
   EXTRA_DIRS="${EXTRA_DIRS} $with_plib"
fi

AC_ARG_ENABLE(debug, [  --enable-debug    Enable debugging info])
if test x$enable_debug = xyes; then
    AC_DEFINE([DEBUG], [],  [Enable debugging cheats and shortcuts])
    CFLAGS="$CFLAGS -g -pg"
    CXXFLAGS="$CXXFLAGS -g -pg"
    SUMMARY="$SUMMARY\nEnabled debugging, please disable for a release!!"
else
    CFLAGS="$CFLAGS -DNDEBUG"
    CXXFLAGS="$CXXFLAGS -DNDEBUG"
    SUMMARY="$SUMMARY\nDisabled debugging, asserts have been turned off"
fi

AC_ARG_ENABLE(optimization, [  --disable-optimization    Disable compiler optimizations],,
    enable_optimization=yes)
if test x$enable_optimization = xyes; then
    CFLAGS="$CFLAGS -O2"
    CXXFLAGS="$CXXFLAGS -O2"
else
    SUMMARY="$SUMMARY\nDisabled compiler optimizations."
fi

dnl ===================
dnl Checks for programs.
dnl ===================
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL

dnl ===============================
dnl Check for MS Windows environment
dnl ================================
AC_CHECK_HEADER(windows.h)


dnl ================================================
dnl Add special compiler flags for certain platforms
dnl ================================================
case "${host}" in
*darwin*|*macosx*)
  LDFLAGS="$LDFLAGS -lintl"
esac


dnl ==================================
dnl check for OpenGL related libraries
dnl ==================================
dnl We check for openal headers here, too - since otherwise
dnl the test further down is even more complicated. This test
dnl depends on the platform, since Macs are using OpenAL/al.h
dnl instead of AL/al.h.
case "${host}" in
*darwin*|*macosx*)
    opengl_LIBS="$LIBS -framework Carbon -framework OpenGL -framework IOKit -framework AGL"
    ;;

*-*-cygwin* | *-*-mingw32*)
    dnl CygWin under Windoze.

    echo Win32 specific hacks...
    AC_DEFINE([WIN32], 1, [Define for Win32 platforms])
    AC_DEFINE([NOMINMAX], 1, [Define for Win32 platforms])

    opengl_LIBS="-lglut32 -lglu32 -lopengl32 -luser32 -lgdi32 -lm -lwinmm"
    joystick_LIBS="$joystick_LIBS -lwinmm"

    echo "Will link apps with $LIBS"
    ;;

*)
   dnl X-Windows based machines

   save_LIBS=$LIBS
   AC_SEARCH_LIBS(glNewList, [ GL GLcore MesaGL ])
   opengl_LIBS="$LIBS"
   LIBS=$save_LIBS
   if test "x$ac_cv_search_glNewList" = "x-lGLcore"; then
       dnl if GLcore found, then also check for GL
       AC_SEARCH_LIBS(glXCreateContext, GL)
   fi
   ;;
esac

dnl ========================================
dnl check for OpenAL libraries
dnl ========================================
dnl FIXME: this all appears too complicated, doing unnecesary
dnl        tests etc --> needs cleanup

dnl The check for OpenAL headers depends on the OS, since 
dnl Apple has the headers in OpenAL/al.h instead of AL/al.h 
AC_SEARCH_LIBS(alGenBuffers, [openal alut], have_al_lib=yes)   

dnl We also check for ogg vorbis support now.
dnl we have 4 sound libraries to check, first we check for OpenAL   
if test x$have_al_lib = xyes; then
  case "${host}" in
    *darwin*|*macosx*)
       AC_CHECK_HEADER(OpenAL/al.h, have_al_hdr=yes)
       ;;
    *)
       AC_CHECK_HEADER(AL/al.h,     have_al_hdr=yes)
       ;;
  esac
fi

case "${host}" in
*)
   dnl default unix style machines

   dnl check for libraries
   save_LIBS=$LIBS

   dnl search for the libraries needed to use ogg vorbis
   AC_SEARCH_LIBS(ov_open, vorbisfile, have_vorbisfile_lib=yes)

   if test x$have_vorbisfile_lib = xyes; then       
       dnl check for the headers needed by ogg vorbis
       AC_CHECK_HEADER(vorbis/vorbisfile.h, have_vorbisfile_hdr=yes)
       AC_CHECK_HEADER(ogg/ogg.h, have_ogg_hdr=yes)
   fi

   dnl create OpenAL defines and a summary of the optional sound components
   SUMMARY="$SUMMARY\n"
   if test x$have_al_hdr != xyes; then
       SUMMARY="$SUMMARY You DO NOT have OpenAL."
   fi

   if test x$have_al_hdr = xyes ; then
       openal_LIBS="$LIBS"
       AC_DEFINE([HAVE_OPENAL], 1, [Defined when OpenAL is available])
       SUMMARY="$SUMMARY\nUsing OpenAL for sound."

       if test x$have_vorbisfile_hdr = xyes -a x$have_ogg_hdr = xyes; then
           AC_DEFINE([HAVE_OGGVORBIS], 1, [Defined when Ogg Vorbis is available])
           SUMMARY="$SUMMARY\nWe have Ogg Vorbis support."
           # ov_read for ogg music needs endianness flag. This macro defines
           # WORDS_BIGENDIAN if it's a big endian machine. See music_ogg.cpp
	   AC_C_BIGENDIAN
       else
           SUMMARY="$SUMMARY\nNo Ogg Vorbis support - the game will play with sound effects, but without music."
           SUMMARY="$SUMMARY\nPlease install ogg vorbis!"
       fi
   else
       SUMMARY="$SUMMARY\nInstall OpenAL for music!"
   fi

   LIBS=$save_LIBS
esac

# The following are C++ items that need to be tested for with the c++
# compiler
AC_LANG_PUSH(C++)


# ==============================================
# Check for "plib" without which we cannot go on
# ==============================================
# specify the plib location
AC_ARG_WITH(plib, [  --with-plib=PREFIX      Specify the prefix path to plib])
if test "x$with_plib" != "x" ; then
   echo "plib prefix is $with_plib"
   EXTRA_DIRS="${EXTRA_DIRS} $with_plib"
fi

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
#  include <plib/ul.h>
 
#  define MIN_PLIB_VERSION 184
   int main() {
   if ( PLIB_VERSION < MIN_PLIB_VERSION ) {
      return -1;
    }
   return 0;
   }
 ],
  AC_MSG_RESULT(yes),
  [ AC_MSG_RESULT(wrong version);
    AC_MSG_ERROR([Install plib 1.8.4 or later first...])],
  AC_MSG_RESULT(yes)
)



dnl =======================
dnl check for SDL libraries
dnl =======================
dnl Potentially sdl-config could be used, but apparently that might not
dnl exist on all platforms, so doing the 'classical' test is safer :(
case "${host}" in
*)
   dnl default unix style machines

   dnl check for libraries
   save_LIBS=$LIBS 
   AC_SEARCH_LIBS(SDL_Init, SDL, have_SDL_lib=yes)
   sdl_LIBS="$LIBS"
   LIBS=$save_LIBS

   dnl check for header files
   AC_CHECK_HEADER(SDL/SDL.h, have_SDL_hdr=yes)
   
   if test x$have_SDL_hdr != xyes -o x$have_SDL_lib != xyes; then
      AC_MSG_ERROR([Install SDL first...])
   fi

    AC_TRY_RUN([
    #  include <SDL/SDL.h>
       int main() {
       if ( SDL_VERSION_ATLEAST(1,2,10)) {
          return 0;
        }
       return -1;
       }
     ],
      :,
        AC_MSG_WARN([Your SDL version uses deprecated
    headers update to version 1.2.10 or higher if possible. Currently this
    doesn't affects the game at all.])
    )

esac

dnl =======================================================
dnl Add special compiler flags for SDL on certain platforms
dnl =======================================================
case "${host}" in
*darwin*|*macosx*)
  dnl SDL on Macs needs to link in SDLmain, since it includes the
  dnl actual main program (on Macs #include "SDL/SDL.h" renames
  dnl main into SDL_main, which then gets called from the new
  dnl main program included in SDLmain. This is necessary to 
  dnl install a proper framework for SDL).
  dnl This can't be done earlier, since all SDL tests program
  dnl from configure do NOT include SDL/SDL.h, therefore the 
  dnl main program gets not replaced, and then getting an linking
  dnl error about main defined twice :((
  LDFLAGGS="$LDFLAGS -framework OpenAL"
  AC_CHECK_PROGS(have_sdl_config,[sdl-config],"no")
  if test x$have_sdl_config != xno; then
    LDFLAGS="$LDFLAGS `sdl-config --libs`"
    CXXFLAGS="$CXXFLAGS `sdl-config --cflags`"
  fi
esac

dnl ======================================================
dnl Try to supply the SVN revision number for the compiler
dnl ======================================================
AC_CHECK_PROGS(have_svnversion,[svnversion],"no")
if test x$have_svnversion != xno; then
  SVNVERSION="`svnversion .`"
  AC_DEFINE_UNQUOTED([SVNVERSION],"$SVNVERSION",["SVN revision number"])
fi


# =========================
# subst bars in Makefile.am
# =========================
AC_SUBST(bullet_LIBS)
AC_SUBST(opengl_LIBS)
AC_SUBST(openal_LIBS)
AC_SUBST(sdl_LIBS)

AC_MSG_CHECKING([for gcc -fno-regmove option ])

dnl =========================
dnl test regmove compiler bug
dnl =========================
dnl Test if -fno-regmove is supported, since -fregmove (which is part of -O2 on
dnl g++) triggers an optimisation problem with static_ssg:
ORIGCXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -fno-regmove"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
	          AC_MSG_RESULT(yes); NOREGMOVE=-fno-regmove,
                  AC_MSG_RESULT(no);  NOREGMOVE="")
CXXFLAGS="$ORIGCXXFLAGS"
AC_SUBST(NOREGMOVE)

dnl ==============
dnl Bullet physics
dnl ==============
AC_DEFINE([BT_NO_PROFILE], [], [Disable bullet internal profiling])
CXXFLAGS="$CXXFLAGS -Ibullet/src"
SUMMARY="$SUMMARY\nUsing bullet physics."
BULLETTREE="src/bullet"

case "${host}" in
*darwin*|*macosx*)
    bullet_LIBS="-Lbullet/Demos/OpenGL -lbulletopenglsupport -Lbullet/src -lbulletdynamics -lbulletcollision -lbulletmath -framework GLUT"
    ;;
*)
    bullet_LIBS="-Lbullet/Demos/OpenGL -lbulletopenglsupport -Lbullet/src -lbulletdynamics -lbulletcollision -lbulletmath -lglut"
    ;;
esac

AC_SUBST(BULLETTREE)

dnl ================
dnl Create makefiles
dnl ================
AC_CONFIG_FILES([                   \
 Makefile                           \
 data/Makefile                      \
 data/fonts/Makefile                \
 data/herrings/Makefile             \
 data/karts/Makefile                \
 data/models/Makefile               \
 data/music/Makefile                \
 data/po/Makefile                   \
 data/sfx/Makefile                  \
 data/textures/Makefile             \
 data/tracks/Makefile               \
 doc/Makefile                       \
 doc/players_manual/Makefile        \
 src/Makefile                       \
 src/robots/Makefile                \
 src/bullet/Makefile                \
 src/bullet/src/Makefile            \
 src/bullet/Demos/OpenGL/Makefile   \
])
AC_OUTPUT
echo -e $SUMMARY
