/*
   AngelCode Scripting Library
   Copyright (c) 2003-2014 Andreas Jonsson

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




#include "as_config.h"
#include "as_property.h"
#include "as_scriptengine.h"

BEGIN_AS_NAMESPACE

asCGlobalProperty::asCGlobalProperty() 
{ 
	memory          = &storage; 
	memoryAllocated = false; 
	realAddress     = 0; 
	initFunc        = 0;
	accessMask      = 0xFFFFFFFF;

	refCount.set(1);
}

asCGlobalProperty::~asCGlobalProperty()
{ 
#ifndef WIP_16BYTE_ALIGNED
	if( memoryAllocated ) { asDELETEARRAY(memory); } 
#else
	if( memoryAllocated ) { asDELETEARRAYALIGNED(memory); } 
#endif

	if( initFunc )
		initFunc->ReleaseInternal();
}

void asCGlobalProperty::AddRef()
{
	refCount.atomicInc();
}

void asCGlobalProperty::Release()
{
	if( refCount.atomicDec() == 0 )
		asDELETE(this, asCGlobalProperty);
}

void asCGlobalProperty::DestroyInternal()
{
	if( initFunc )
	{
		initFunc->ReleaseInternal();
		initFunc = 0;
	}
}

void *asCGlobalProperty::GetAddressOfValue()
{ 
	return memory;
}

// The global property structure is responsible for allocating the storage
// method for script declared variables. Each allocation is independent of
// other global properties, so that variables can be added and removed at
// any time.
void asCGlobalProperty::AllocateMemory() 
{ 
	if( type.GetSizeOnStackDWords() > 2 ) 
	{ 
#ifndef WIP_16BYTE_ALIGNED
		memory = asNEWARRAY(asDWORD, type.GetSizeOnStackDWords()); 
#else
		// TODO: Avoid aligned allocation if not needed to reduce the waste of memory for the alignment
		memory = asNEWARRAYALIGNED(asDWORD, type.GetSizeOnStackDWords(), type.GetAlignment()); 
#endif
		memoryAllocated = true; 
	} 
}

void asCGlobalProperty::SetRegisteredAddress(void *p) 
{ 
	realAddress = p;
	if( type.IsObject() && !type.IsReference() && !type.IsObjectHandle() )
	{
		// The global property is a pointer to a pointer 
		memory = &realAddress;
	} 
	else
		memory = p; 
}

void *asCGlobalProperty::GetRegisteredAddress() const
{
	return realAddress;
}

void asCGlobalProperty::SetInitFunc(asCScriptFunction *initFunc)
{
	// This should only be done once
	asASSERT( this->initFunc == 0 );

	this->initFunc = initFunc;
	this->initFunc->AddRefInternal();
}

asCScriptFunction *asCGlobalProperty::GetInitFunc()
{
	return initFunc;
}

END_AS_NAMESPACE
