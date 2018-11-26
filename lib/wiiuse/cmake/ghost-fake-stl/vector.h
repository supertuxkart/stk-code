/**	@file
	@brief	Fake header to allow GHOST 4.09 use with MSVC 2005

	@date	2010

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

#pragma once

#include <vector>
using std::vector;

// Disable dll export that depends on the SGI STL implementation
#undef GHOST_EXTRA_TEMPLATE_DECLARATIONS