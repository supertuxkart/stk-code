# Defines a series of preprocessor variables based on the current platform.
#
# Usage: define_platform_macros(PREFIX)
#
#  where PREFIX is the macro prefix (i.e., if PREFIX is XYZZY then the macros
#  will be named XYZZY_LINUX, XYZZY_WINDOWS, etc.).
#
# Author:
#   Kevin M. Godby <kevin@godby.org>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

function(_define_platform_macros_impl prefix platform_string variable_name)
	if (${CMAKE_SYSTEM_NAME} MATCHES "${platform_string}")
		string(TOUPPER "${prefix}_${variable_name}" _varname)
		set(${_varname} TRUE PARENT_SCOPE)
	endif()
endfunction()

macro(define_platform_macros _prefix)
	_define_platform_macros_impl(${_prefix} "AIX" AIX)
	_define_platform_macros_impl(${_prefix} "Android" ANDROID)
	_define_platform_macros_impl(${_prefix} "BS/DOS" BSDOS)
	_define_platform_macros_impl(${_prefix} "FreeBSD" FREEBSD)
	_define_platform_macros_impl(${_prefix} "HP-UX" HPUX)
	_define_platform_macros_impl(${_prefix} "IRIX" IRIX)
	_define_platform_macros_impl(${_prefix} "Linux" LINUX)
	_define_platform_macros_impl(${_prefix} "GNU/kFreeBSD" KFREEBSD)
	_define_platform_macros_impl(${_prefix} "NetBSD" NETBSD)
	_define_platform_macros_impl(${_prefix} "OpenBSD" OPENBSD)
	_define_platform_macros_impl(${_prefix} "OFS1" OFS1)
	_define_platform_macros_impl(${_prefix} "SCO_SV" SCO_SV)
	_define_platform_macros_impl(${_prefix} "UnixWare" UNIXWARE)
	_define_platform_macros_impl(${_prefix} "Xenix" XENIX)
	_define_platform_macros_impl(${_prefix} "SunOS" SUNOS)
	_define_platform_macros_impl(${_prefix} "Tru64" TRU64)
	_define_platform_macros_impl(${_prefix} "ULTRIX" ULTRIX)
	_define_platform_macros_impl(${_prefix} "CYGWIN" CYGWIN)
	_define_platform_macros_impl(${_prefix} "Darwin" MACOSX)
	_define_platform_macros_impl(${_prefix} "Windows" WINDOWS)
endmacro()

