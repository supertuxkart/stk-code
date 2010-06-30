# Process this file with autogen.sh to produce a configure script.

AC_INIT(supertuxkart, SVN)
AC_CONFIG_SRCDIR([src/main.cpp])

AC_PREREQ(2.61)

# ================================================
# Initialize the automake stuff
# set the $host variable based on local machine/os
# ================================================
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([foreign tar-ustar])


# =====================================================
# Set the default CFLAGS/CXXFLAGS
# If these were not set, the AC_PROG_CC and AC_PROG_CXX
# scripts set them to "-g -O2".
# =====================================================
CFLAGS="$CFLAGS -Wall -Wstrict-prototypes -Wmissing-prototypes"
CXXFLAGS="$CXXFLAGS -Wall"

# ===================
# Check for compilers
# ===================
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

# This macro defines WORDS_BIGENDIAN if building for a big endian machine.
AC_C_BIGENDIAN


AC_LANG(C++)


# ===========================
# Check for `configure' flags
# ===========================
SUMMARY="\nSummary of optional components:\n==============================="

AC_ARG_ENABLE(debug, [AS_HELP_STRING(--enable-debug,
                                     [enable debugging info])])
if test x$enable_debug = xyes; then
    AC_DEFINE([DEBUG], [],  [enable debugging cheats and shortcuts])
    CFLAGS="$CFLAGS -g -pg"
    CXXFLAGS="$CXXFLAGS -g -pg"
    SUMMARY="$SUMMARY\nEnabled debugging, please disable for a release!!"
else
    CFLAGS="$CFLAGS -DNDEBUG"
    CXXFLAGS="$CXXFLAGS -DNDEBUG"
    SUMMARY="$SUMMARY\nDisabled debugging, asserts have been turned off"
fi

AC_ARG_ENABLE(optimization, [AS_HELP_STRING(--disable-optimization,
                                            [disable compiler optimizations])],,
    enable_optimization=yes)
if test x$enable_optimization = xyes; then
    CFLAGS="$CFLAGS -O2"
    CXXFLAGS="$CXXFLAGS -O2"
else
    SUMMARY="$SUMMARY\nDisabled compiler optimizations."
fi

# ===================
# Checks for programs.
# ===================
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL

# ===============================
# Check for MS Windows environment
# ================================
AC_CHECK_HEADER(windows.h)

# ==================================
# check for OpenGL related libraries
# ==================================
case "${host}" in
*darwin*|*macosx*)
    opengl_LIBS="$LIBS -framework Carbon -framework OpenGL -framework IOKit -framework AGL"
    ;;

*-*-cygwin* | *-*-mingw32*)
    # CygWin under Windoze.

    echo Win32 specific hacks...
    AC_DEFINE([WIN32], 1, [Define for Win32 platforms])
    AC_DEFINE([NOMINMAX], 1, [Define for Win32 platforms])

    opengl_LIBS="-lopengl32 -luser32 -lgdi32 -lm -lwinmm"
    joystick_LIBS="$joystick_LIBS -lwinmm"
    LDFLAGS="$LDFLAGS -mwindows"

    echo "Will link apps with $LIBS"
    ;;

*)
   # X-Windows based machines

   save_LIBS=$LIBS
   AC_SEARCH_LIBS(glNewList, [ GL GLcore MesaGL ])
   opengl_LIBS="$LIBS"
   LIBS=$save_LIBS
   if test "x$ac_cv_search_glNewList" = "x-lGLcore"; then
       # if GLcore found, then also check for GL
       AC_SEARCH_LIBS(glXCreateContext, GL)
   fi
   ;;
esac

# ========================================
# check for OpenAL libraries
# ========================================
save_LIBS=$LIBS

case "${host}" in
*darwin*|*macosx*)
    # search for OpenAL installed in a UNIX way first. This is necessary since OS X 10.4
    # comes with an OpenAL framework that's too old, so to retain 10.4 support we need to
    # build OpenAL from source, and can't use Apple's framework.
    AC_SEARCH_LIBS(alGenBuffers, [openal alut], have_al_lib=yes)
    
    # We couldn't find OpenAL installed the UNIX way, so fall back to using frameworks
    if test x$have_al_lib != xyes; then
        LIBS="-framework OpenAL $LIBS"
        have_al_lib=yes
    fi
    ;;
*)
    AC_SEARCH_LIBS(alGenBuffers, [openal alut], have_al_lib=yes)
    ;;
esac

if test x$have_al_lib = xyes; then
    # Apple has the headers in OpenAL/al.h instead of AL/al.h 
    AC_CHECK_HEADERS([AL/al.h OpenAL/al.h], have_al_hdr=yes)
fi

if test x$have_al_hdr != xyes; then
    AC_MSG_ERROR([[cannot find useable OpenAL installation. You need to install OpenAL.]])
fi

openal_LIBS="$LIBS"
LIBS="$save_LIBS"

# ====================
# check for Ogg Vorbis
# ====================

save_LIBS=$LIBS

# also check in /usr/local
CPPFLAGS="${CPPFLAGS} -I/usr/local/include"
LDFLAGS="$LDFLAGS -L/usr/local/lib"

# search for the libraries needed to use ogg vorbis
AC_SEARCH_LIBS(ov_open, [vorbisfile], have_vorbisfile_lib=yes, have_vorbisfile_lib=no, [-lvorbis])

if test x$have_vorbisfile_lib = xyes; then       
    # check for the headers needed by ogg vorbis
    AC_CHECK_HEADER(vorbis/vorbisfile.h, have_vorbisfile_hdr=yes)
    AC_CHECK_HEADER(ogg/ogg.h, have_ogg_hdr=yes)
fi
SUMMARY="$SUMMARY\n"

if test x$have_vorbisfile_hdr = xyes -a x$have_ogg_hdr = xyes; then
    AC_DEFINE([HAVE_OGGVORBIS], 1, [Defined when Ogg Vorbis is available])
    SUMMARY="$SUMMARY\nWe have Ogg Vorbis support."
else
    AC_MSG_ERROR([[cannot find useable Ogg Vorbis installation. You need to install libogg and libvorbis.]])
    #SUMMARY="$SUMMARY\nNo Ogg Vorbis support - the game will play with sound effects, but without music."
    #SUMMARY="$SUMMARY\nPlease install ogg vorbis!"
fi

oggvorbis_LIBS="$LIBS"
LIBS=$save_LIBS

# The following are C++ items that need to be tested for with the c++
# compiler
AC_LANG_PUSH(C++)


# ==================
# Check for irrlicht
# ==================
AC_ARG_WITH(irrlicht, [AS_HELP_STRING(--with-irrlicht=PREFIX,
                                  [specify the prefix path to irrlicht])])
if test "x$with_irrlicht" != "x" ; then
   CPPFLAGS="${CPPFLAGS} -I $with_irrlicht/include -I $with_irrlicht/include/irrlicht"
   AC_CHECK_HEADER(irrlicht.h, have_irrlicht_hdr=yes)
   if test x$have_irrlicht_hdr != xyes; then
      AC_MSG_ERROR([Can't find irrlicht in $with_irrlicht.])
   fi
else
   CPPFLAGS_Save=$CPPFLAGS

   # check in /usr
   echo " in /usr/ : "
   
   CPPFLAGS="${CPPFLAGS_Save} -I/usr/include/irrlicht"
   AC_CHECK_HEADER(irrlicht.h, irrlicht_found_in_usr=yes, irrlicht_found_in_usr=no)
   
   # if it's not found there, try /usr/local instead
   if test x$irrlicht_found_in_usr == xyes; then
       with_irrlicht="/usr"
   else
   
       # I'd like to fall back to check /usr/local second, but stupid autotools does not support
       # checking for a header in 2 directories because it caches results
       #AC_MSG_ERROR([Can't find irrlicht installation in /usr, use --with-irrlicht...])
       
       # erase cached result
       unset ac_cv_header_irrlicht_h
       
       echo " in /usr/local/ : "
       CPPFLAGS="${CPPFLAGS_Save} -I/usr/local/include/irrlicht"
       AC_CHECK_HEADER(irrlicht.h, irrlicht_found_in_usr_local=yes, irrlicht_found_in_usr_local=no)
       
       if test x$irrlicht_found_in_usr_local == xyes; then
          with_irrlicht="/usr/local"
       else
          AC_MSG_ERROR([Can't find irrlicht installation in standard prefixes, use --with-irrlicht...])
       fi
   fi
   
   CPPFLAGS="${CPPFLAGS_Save} -I ${with_irrlicht}/include/irrlicht"

fi

case "${host}" in
*-*-linux* )
  irrlicht_LIBS="-L/$with_irrlicht/lib/Linux -L/$with_irrlicht/lib -lIrrlicht"
  ;;
*darwin*|*macosx*)
  irrlicht_LIBS="-L/$with_irrlicht/lib/ -lIrrlicht -framework Cocoa"
  ;;
*freebsd*)
  irrlicht_LIBS="-L/$with_irrlicht/lib/ -lIrrlicht -lpthread"
esac

# ---- Check these flags do work by building a small irrLicht sample
echo "checking for irrlicht libraries..."
save_LIBS=$LIBS
LIBS="$LIBS $irrlicht_LIBS $opengl_LIBS"

irrlicht_sample_built_fine="false"

AC_TRY_RUN(
    [
        #  include "irrlicht.h"
        int main() {
        irr::createDevice(irr::video::EDT_NULL);
        return 0;
        }
    ],
    irrlicht_sample_built_fine="true",
    irrlicht_sample_built_fine="false"
)
  
case "${host}" in
*-*-linux* )
  # If base irrLicht flags failed, try again with Xxf86vm
  # FIXME: this program when run prints out two lines (version numbers) to stdout
  if test x$irrlicht_sample_built_fine = xfalse; then
    LIBS="$LIBS -lXxf86vm"
    works_with_Xxf86vm="false"
      AC_TRY_RUN([
     #  include "irrlicht.h"
     int main() {
     irr::createDevice(irr::video::EDT_NULL);
     return 0;
     }
    ],
    works_with_Xxf86vm="true",
    )
    
    if test x$works_with_Xxf86vm = xtrue; then
      irrlicht_LIBS="$irrlicht_LIBS -lXxf86vm"
      irrlicht_sample_built_fine="true"
    fi
  fi
  ;;
esac
  
if test x$irrlicht_sample_built_fine = xfalse; then
AC_MSG_ERROR([Can't build test irrLicht program. Check config.log to see the errors])
fi

# ---- Check we have the minimum required irrLicht version
echo -n "checking irrlicht version is recent enough... "

irrlicht_min_version_met="false"

AC_TRY_RUN(
    [
        #  include "irrlicht.h"
        int main() {
        #if (IRRLICHT_VERSION_MAJOR > 1) || (IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR >= 7)
            // ok
        #else
        #error "You need irrLicht 1.7+ to build STK"
        #endif
        return 0;
        }
    ],
    irrlicht_min_version_met="true",
    irrlicht_min_version_met="false"
)

if test x$irrlicht_min_version_met = xfalse; then
    AC_MSG_ERROR([Your irrLicht is too old, please update irrLicht.])
else
    echo "yes"
fi

LIBS=$save_LIBS

# ==================
# Check for Fribidi
# ==================

save_LIBS=$LIBS

# also check in /usr/local
CPPFLAGS="${CPPFLAGS} -I/usr/local/include"
LDFLAGS="$LDFLAGS -L/usr/local/lib"

# search for the libraries needed to use ogg vorbis
AC_SEARCH_LIBS(fribidi_log2vis, [fribidi], have_fribidi=yes, have_fribidi=no)

if test x$have_fribidi = xyes; then       
    AC_CHECK_HEADER(fribidi/fribidi.h, have_fribidi_hdr=yes)
fi
SUMMARY="$SUMMARY\n"

if test x$have_fribidi = xyes -a x$have_fribidi_hdr = xyes; then
    AC_DEFINE([ENABLE_BIDI], 1, [Defined when Fribidi is available])
    SUMMARY="$SUMMARY\nFribidi (RTL languages support) support enabled."
    fribidi_LIBS="$LIBS"
else
    SUMMARY="$SUMMARY\nFribidi (RTL languages support) support disabled."
    fribidi_LIBS=""
fi

LIBS=$save_LIBS


# ======================================================
# Try to supply the SVN revision number for the compiler
# ======================================================
AC_CHECK_PROGS(have_svnversion,[svnversion],"no")
if test x$have_svnversion != xno; then
  SVNVERSION="`svnversion $srcdir`"
  AC_DEFINE_UNQUOTED([SVNVERSION],"$SVNVERSION",["SVN revision number"])
fi

# ==============
# Bullet physics
# ==============
AC_DEFINE([BT_NO_PROFILE], [], [Disable bullet internal profiling])
SUMMARY="$SUMMARY\nUsing bullet physics."
BULLETTREE="src/bullet"
bullet_LIBS="-Lbullet/src -lbulletdynamics -lbulletcollision -lbulletmath"


# ====
# enet
# ====
AC_CHECK_FUNC(gethostbyaddr_r, [AC_DEFINE(HAS_GETHOSTBYADDR_R)])
AC_CHECK_FUNC(gethostbyname_r, [AC_DEFINE(HAS_GETHOSTBYNAME_R)])
AC_CHECK_FUNC(poll, [AC_DEFINE(HAS_POLL)])
AC_CHECK_FUNC(fcntl, [AC_DEFINE(HAS_FCNTL)])
AC_CHECK_FUNC(inet_pton, [AC_DEFINE(HAS_INET_PTON)])
AC_CHECK_FUNC(inet_ntop, [AC_DEFINE(HAS_INET_NTOP)])

AC_CHECK_MEMBER(struct msghdr.msg_flags, [AC_DEFINE(HAS_MSGHDR_FLAGS)], , [#include <sys/socket.h>])

AC_CHECK_TYPE(socklen_t, [AC_DEFINE(HAS_SOCKLEN_T)], , 
              #include <sys/types.h>
              #include <sys/socket.h>
)

AC_EGREP_HEADER(MSG_MAXIOVLEN, /usr/include/sys/socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))
AC_EGREP_HEADER(MSG_MAXIOVLEN, socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))

case "${host}" in
*-*-cygwin* | *-*-mingw32*)
  enet_LIBS="-Lenet -lenet -lws2_32"
  ;;
*)
  enet_LIBS="-Lenet -lenet"
  ;;
esac
SUMMARY="$SUMMARY\nUsing enet network support."
ENETTREE="src/enet"

# Check for gettext
CHECK_GETTEXT(external)
#disable/enable addons support
AC_ARG_WITH(addons,[AS_HELP_STRING([--with-addons],
              [This option is used to enable/disable addons support])])
if test "x$with_addons" == "xyes" ; then
    PKG_CHECK_MODULES(LIBCURL, libcurl >= 0.1)
    echo "STK will compile with addons support"
    AC_DEFINE(ADDONS_MANAGER)
fi
# =========================
# subst bars in Makefile.am
# =========================

AC_SUBST(irrlicht_LIBS)
AC_SUBST(fribidi_LIBS)
AC_SUBST(bullet_LIBS)
AC_SUBST(enet_LIBS)
AC_SUBST(opengl_LIBS)
AC_SUBST(openal_LIBS)
AC_SUBST(oggvorbis_LIBS)
AC_SUBST(BULLETTREE)
AC_SUBST(ENETTREE)

# ================
# Create makefiles
# ================
AC_CONFIG_FILES([                   \
 Makefile                           \
 data/Makefile                      \
 data/fonts/Makefile                \
 data/grandprix/Makefile            \
 data/gui/Makefile                  \
 data/karts/Makefile                \
 data/models/Makefile               \
 data/music/Makefile                \
 data/po/Makefile                   \
 data/sfx/Makefile                  \
 data/textures/Makefile             \
 data/tracks/Makefile               \
 doc/Makefile                       \
 src/Makefile                       \
 src/ide/Makefile                   \
 src/bullet/Makefile                \
 src/bullet/src/Makefile            \
 src/enet/Makefile                  \
])
AC_OUTPUT
echo -e $SUMMARY
