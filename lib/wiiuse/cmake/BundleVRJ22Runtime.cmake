# - Include the VR Juggler runtime files in an installation or built package.
#
#  VRJUGGLERRUNTIME_BUNDLE
#  VRJUGGLERRUNTIME_BUNDLE_DEBUG - set to yes to include debug dll's as well
#
# Requires these CMake modules:
#  FindVRJuggler22 and its dependencies
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

if(WIN32)
	option(VRJUGGLERRUNTIME_BUNDLE
		"Install a local copy of the VR Juggler runtime files with the project."
		on)
	option(VRJUGGLERRUNTIME_BUNDLE_DEBUG
		"Install the VR Juggler debug runtime files as well."
		off)
	mark_as_advanced(VRJUGGLERRUNTIME_BUNDLE_DEBUG)
else()
	# TODO - how to handle when not on Windows?
	#option(VRJUGGLERRUNTIME_BUNDLE "Install a local copy of the VR Juggler runtime files with the project." off)
endif()

mark_as_advanced(VRJUGGLERRUNTIME_BUNDLE VRJUGGLERRUNTIME_BUNDLE_DEBUG)

if(VRJUGGLERRUNTIME_BUNDLE AND VRJUGGLER22_FOUND)
	if(WIN32)
		get_filename_component(_vrjlibdir "${VRJ22_LIBRARY_RELEASE}" PATH)
		get_filename_component(_vrjroot "${_vrjlibdir}/../" ABSOLUTE)

		# TODO - make sure gadgeteer and sonix can find their DSO's at runtime...

		foreach(_dir bin lib)
			if(VRJUGGLERRUNTIME_BUNDLE_DEBUG)
				install(DIRECTORY "${_vrjroot}/${_dir}/"
						DESTINATION bin
						PATTERN "*.lib" EXCLUDE		# exclude static and link libraries
						PATTERN "*.exe" EXCLUDE		# exclude unneeded executables
						PATTERN "*.py" EXCLUDE		# exclude unneeded python executables
						PATTERN "*.pyc" EXCLUDE		# exclude unneeded python executables
				)
			else()
				install(DIRECTORY ${_vrjroot}/${_dir}/
						DESTINATION bin
						PATTERN "*.lib" EXCLUDE		# exclude static and link libraries
						PATTERN "*.exe" EXCLUDE		# exclude unneeded executables
						PATTERN "*.py" EXCLUDE		# exclude unneeded python executables
						PATTERN "*.pyc" EXCLUDE		# exclude unneeded python executables

						PATTERN "*d.dll" EXCLUDE	# exclude debug dll's
						PATTERN "*-gd-*.dll" EXCLUDE	# exclude Boost debug dll's
				)
			endif()

		endforeach()

		install(DIRECTORY ${_vrjroot}/share/
				DESTINATION share
				FILES_MATCHING

				# Runtime files
				PATTERN "*.dll"
				PATTERN "*.jar"

				# Data files
				PATTERN "*.wav"
				PATTERN "*.xml"
				PATTERN "*.xsl"
				PATTERN "*.xsd"
				PATTERN "*.flt"
				PATTERN "*.dat"
				PATTERN "*.table"


				# Config files
				PATTERN "*.jdef"
				PATTERN "*.jconf"
				PATTERN "*.cfg"
				PATTERN "hosts.allow"

				# Other Files
				PATTERN "*.txt"
				PATTERN "COPYING*"
				PATTERN "ChangeLog"
		)

	endif()



endif()
