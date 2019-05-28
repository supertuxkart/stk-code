# - Given a list of libraries with OPTIMIZED, DEBUG, etc.
#
#  split_library_list(_generalvar _releasevar _debugvar)
#
# Requires CMake 2.6 or newer (uses the 'function' command)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


function(split_library_list _generalvar _releasevar _debugvar)
	set(_general)
	set(_debug)
	set(_release)
	set(_dest _general)

	foreach(item ${ARGN})
		if(${item} MATCHES "[dD][eE][bB][uU][gG]")
			set(_dest _debug)
		elseif(${item} MATCHES "[oO][pP][tT][iI][mM][iI][zZ][eE][dD]")
			set(_dest _release)
		elseif(${item} MATCHES "[gG][eE][nN][eE][rR][aA][lL]")
			set(_dest _general)
		else()
			list(APPEND ${_dest} "${item}")
			set(_dest _general)
		endif()
	endforeach()

	set(${_releasevar} ${_release} PARENT_SCOPE)
	set(${_debugvar} ${_debug} PARENT_SCOPE)
	set(${_generalvar} ${_general} PARENT_SCOPE)
endfunction()
