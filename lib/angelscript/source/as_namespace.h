/*
   AngelCode Scripting Library
   Copyright (c) 2013 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


#ifndef AS_NAMESPACE_H
#define AS_NAMESPACE_H

#include "as_string.h"

BEGIN_AS_NAMESPACE

struct asSNameSpace
{
	asCString name;

	// TODO: namespace: A namespace should have access masks. The application should be
	//                  able to restrict specific namespaces from access to specific modules
};


struct asSNameSpaceNamePair
{
	const asSNameSpace *ns;
	asCString           name;

	asSNameSpaceNamePair() : ns(0) {}
	asSNameSpaceNamePair(const asSNameSpace *_ns, const asCString &_name) : ns(_ns), name(_name) {}

	asSNameSpaceNamePair &operator=(const asSNameSpaceNamePair &other)
	{
		ns   = other.ns;
		name = other.name;
		return *this;
	}

	bool operator==(const asSNameSpaceNamePair &other) const
	{
		return (ns == other.ns && name == other.name);
	}

	bool operator<(const asSNameSpaceNamePair &other) const
	{
		return (ns < other.ns || (ns == other.ns && name < other.name));
	}
};

END_AS_NAMESPACE

#endif