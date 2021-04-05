# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindOpenGL
# ----------
#
# FindModule for OpenGL and GLU.
#
# Optional COMPONENTS
# ^^^^^^^^^^^^^^^^^^^
#
# This module respects several optional COMPONENTS: ``EGL``, ``GLX``, and
# ``OpenGL``.  There are corresponding import targets for each of these flags.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the :prop_tgt:`IMPORTED` targets:
#
# ``OpenGL::GL``
#  Defined to the platform-specific OpenGL libraries if the system has OpenGL.
# ``OpenGL::OpenGL``
#  Defined to libOpenGL if the system is GLVND-based.
# ``OpenGL::GLU``
#  Defined if the system has GLU.
# ``OpenGL::GLX``
#  Defined if the system has GLX.
# ``OpenGL::EGL``
#  Defined if the system has EGL.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module sets the following variables:
#
# ``OPENGL_FOUND``
#  True, if the system has OpenGL and all components are found.
# ``OPENGL_XMESA_FOUND``
#  True, if the system has XMESA.
# ``OPENGL_GLU_FOUND``
#  True, if the system has GLU.
# ``OpenGL_OpenGL_FOUND``
#  True, if the system has an OpenGL library.
# ``OpenGL_GLX_FOUND``
#  True, if the system has GLX.
# ``OpenGL_EGL_FOUND``
#  True, if the system has EGL.
# ``OPENGL_INCLUDE_DIR``
#  Path to the OpenGL include directory.
# ``OPENGL_EGL_INCLUDE_DIRS``
#  Path to the EGL include directory.
# ``OPENGL_LIBRARIES``
#  Paths to the OpenGL library, windowing system libraries, and GLU libraries.
#  On Linux, this assumes GLX and is never correct for EGL-based targets.
#  Clients are encouraged to use the ``OpenGL::*`` import targets instead.
#
# Cache variables
# ^^^^^^^^^^^^^^^
#
# The following cache variables may also be set:
#
# ``OPENGL_egl_LIBRARY``
#  Path to the EGL library.
# ``OPENGL_glu_LIBRARY``
#  Path to the GLU library.
# ``OPENGL_glx_LIBRARY``
#  Path to the GLVND 'GLX' library.
# ``OPENGL_opengl_LIBRARY``
#  Path to the GLVND 'OpenGL' library
# ``OPENGL_gl_LIBRARY``
#  Path to the OpenGL library.  New code should prefer the ``OpenGL::*`` import
#  targets.
#
# Linux-specific
# ^^^^^^^^^^^^^^
#
# Some Linux systems utilize GLVND as a new ABI for OpenGL.  GLVND separates
# context libraries from OpenGL itself; OpenGL lives in "libOpenGL", and
# contexts are defined in "libGLX" or "libEGL".  GLVND is currently the only way
# to get OpenGL 3+ functionality via EGL in a manner portable across vendors.
# Projects may use GLVND explicitly with target ``OpenGL::OpenGL`` and either
# ``OpenGL::GLX`` or ``OpenGL::EGL``.
#
# Projects may use the ``OpenGL::GL`` target (or ``OPENGL_LIBRARIES`` variable)
# to use legacy GL interfaces.  These will use the legacy GL library located
# by ``OPENGL_gl_LIBRARY``, if available.  If ``OPENGL_gl_LIBRARY`` is empty or
# not found and GLVND is available, the ``OpenGL::GL`` target will use GLVND
# ``OpenGL::OpenGL`` and ``OpenGL::GLX`` (and the ``OPENGL_LIBRARIES``
# variable will use the corresponding libraries).  Thus, for non-EGL-based
# Linux targets, the ``OpenGL::GL`` target is most portable.
#
# A ``OpenGL_GL_PREFERENCE`` variable may be set to specify the preferred way
# to provide legacy GL interfaces in case multiple choices are available.
# The value may be one of:
#
# ``GLVND``
#  If the GLVND OpenGL and GLX libraries are available, prefer them.
#  This forces ``OPENGL_gl_LIBRARY`` to be empty.
#  This is the default if components were requested (since components
#  correspond to GLVND libraries) or if policy :policy:`CMP0072` is
#  set to ``NEW``.
#
# ``LEGACY``
#  Prefer to use the legacy libGL library, if available.
#  This is the default if no components were requested and
#  policy :policy:`CMP0072` is not set to ``NEW``.
#
# For EGL targets the client must rely on GLVND support on the user's system.
# Linking should use the ``OpenGL::OpenGL OpenGL::EGL`` targets.  Using GLES*
# libraries is theoretically possible in place of ``OpenGL::OpenGL``, but this
# module does not currently support that; contributions welcome.
#
# ``OPENGL_egl_LIBRARY`` and ``OPENGL_EGL_INCLUDE_DIRS`` are defined in the case of
# GLVND.  For non-GLVND Linux and other systems these are left undefined.
#
# macOS-Specific
# ^^^^^^^^^^^^^^
#
# On OSX FindOpenGL defaults to using the framework version of OpenGL. People
# will have to change the cache values of OPENGL_glu_LIBRARY and
# OPENGL_gl_LIBRARY to use OpenGL with X11 on OSX.

set(OPENGL_egl_LIBRARY "/opt/devkitpro/portlibs/switch/lib/libEGL.a")
set(OPENGL_gl_LIBRARY "/opt/devkitpro/portlibs/switch/lib/libglapi.a")
set(OPENGL_EGL_INCLUDE_DIR "/opt/devkitpro/portlibs/switch/include")
set(OPENGL_INCLUDE_DIR "/opt/devkitpro/portlibs/switch/include")

mark_as_advanced(
  OPENGL_INCLUDE_DIR
  OPENGL_xmesa_INCLUDE_DIR
  OPENGL_egl_LIBRARY
  OPENGL_glu_LIBRARY
  OPENGL_glx_LIBRARY
  OPENGL_gl_LIBRARY
  OPENGL_opengl_LIBRARY
  OPENGL_EGL_INCLUDE_DIR
  OPENGL_GLX_INCLUDE_DIR
)
