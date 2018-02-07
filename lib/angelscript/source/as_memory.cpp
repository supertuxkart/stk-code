/*
   AngelCode Scripting Library
   Copyright (c) 2003-2016 Andreas Jonsson

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


//
// as_memory.cpp
//
// Overload the default memory management functions so that we
// can let the application decide how to do it.
//

#include <stdlib.h>

#if !defined(__APPLE__) && !defined(__SNC__) && !defined(__ghs__) && !defined(__FreeBSD__) && !defined(__OpenBSD__)
#include <malloc.h>
#endif

#include "as_config.h"
#include "as_memory.h"
#include "as_scriptnode.h"
#include "as_bytecode.h"

BEGIN_AS_NAMESPACE

#ifdef WIP_16BYTE_ALIGN

// TODO: Add support for 16byte aligned application types (e.g. __m128). The following is a list of things that needs to be implemented:
//
//  ok  - The script context must make sure to always allocate the local stack memory buffer on 16byte aligned boundaries (asCContext::ReserveStackSpace)
//  ok  - The engine must make sure to always allocate the memory for the script objects on 16byte aligned boundaries (asCScriptEngine::CallAlloc)
//  ok  - The application needs to inform a new flag when registering types that require 16byte alignment, e.g. asOBJ_APP_ALIGN16 (asCScriptEngine::RegisterObjectType)
//  ok  - The script object type must make sure to align member properties of these types correctly (asCObjectType::AddPropertyToClass)
//  ok  - Script global properties must allocate memory on 16byte boundaries if holding these types (asCGlobalProperty::AllocateMemory)
// TODO - The script compiler must make sure to allocate the local variables on 16byte boundaries (asCCompiler::AllocateVariable)
// TODO - The script compiler must add pad bytes on the stack for all function calls to guarantee that the stack position is 16byte aligned on entry in the called function (asCCompiler)
// TODO - The bytecode serializer must be capable of adjusting these pad bytes to guarantee platform independent saved bytecode. Remember that the registered type may not be 16byte aligned on all platforms (asCWriter & asCReader) 
// TODO - The bytecode serializer must also be prepared to adjust the position of the local variables according to the need fro 16byte alignment (asCWriter & asCReader)
// TODO - The code for the native calling conventions must be adjusted for all platforms that should support 16byte aligned types (as_callfunc...)
//  ok  - When the context needs to grow the local stack memory it must copy the function arguments so that the stack entry position is 16byte aligned (asCContext::CallScriptFunction)
// TODO - When the context is prepared for a new call, it must set the initial stack position so the stack entry position is 16byte aligned (asCContext::Prepare)
//
// http://www.gamedev.net/topic/650555-alignment-requirements/


// TODO: Allow user to register its own aligned memory routines
// Wrappers for aligned allocations
void *debugAlignedMalloc(size_t size, size_t align, const char *file, int line)
{
	void *mem = ((asALLOCFUNCDEBUG_t)userAlloc)(size + (align-1) + sizeof(void*), file, line);

	char *amem = ((char*)mem) + sizeof(void*);
	if( (uintptr_t)amem & (align - 1) )
		amem += align - ((uintptr_t)amem & (align - 1));

	((void**)amem)[-1] = mem;
	return amem;
}

void *alignedMalloc(size_t size, size_t align)
{
	void *mem = userAlloc(size + (align-1) + sizeof(void*));

	char *amem = ((char*)mem) + sizeof(void*);
	if( (uintptr_t)amem & (align - 1) )
		amem += align - ((uintptr_t)amem & (align - 1));

	((void**)amem)[-1] = mem;
	return amem;
}

void alignedFree(void *mem)
{
	userFree( ((void**)mem)[-1] );
}

bool isAligned(const void* const pointer, asUINT alignment)
{
	return (uintptr_t(pointer) % alignment) == 0;
}
#endif

// By default we'll use the standard memory management functions

// Make sure these globals are initialized first. Otherwise the
// library may crash in case the application initializes the engine
// as a global variable.

#ifdef _MSC_VER
// MSVC let's us choose between a couple of different initialization orders.
#pragma warning(disable: 4073)
#pragma init_seg(lib)
asALLOCFUNC_t userAlloc = malloc;
asFREEFUNC_t  userFree  = free;
#ifdef WIP_16BYTE_ALIGN
#ifdef AS_DEBUG
asALLOCALIGNEDFUNC_t userAllocAligned = (asALLOCALIGNEDFUNC_t)debugAlignedMalloc;
#else
asALLOCALIGNEDFUNC_t userAllocAligned = alignedMalloc;
#endif
asFREEALIGNEDFUNC_t  userFreeAligned  = alignedFree;
#endif
#else
// Other compilers will just have to rely on luck.
asALLOCFUNC_t userAlloc = malloc;
asFREEFUNC_t  userFree  = free;
#ifdef WIP_16BYTE_ALIGN
asALLOCALIGNEDFUNC_t userAllocAligned = alignedMalloc;
asFREEALIGNEDFUNC_t  userFreeAligned  = alignedFree;
#endif
#endif

extern "C"
{

// interface
int asSetGlobalMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc)
{
	// Clean-up thread local memory before changing the allocation routines to avoid 
	// potential problem with trying to free memory using a different allocation
	// routine than used when allocating it.
	asThreadCleanup();

	userAlloc = allocFunc;
	userFree  = freeFunc;

	return 0;
}

// interface
int asResetGlobalMemoryFunctions()
{
	// Clean-up thread local memory before changing the allocation routines to avoid 
	// potential problem with trying to free memory using a different allocation
	// routine than used when allocating it.
	asThreadCleanup();

	userAlloc = malloc;
	userFree  = free;

	return 0;
}

// interface
void *asAllocMem(size_t size)
{
	return asNEWARRAY(asBYTE, size);
}

// interface
void asFreeMem(void *mem)
{
	asDELETEARRAY(mem);
}

} // extern "C"

asCMemoryMgr::asCMemoryMgr()
{
}

asCMemoryMgr::~asCMemoryMgr()
{
	FreeUnusedMemory();
}

void asCMemoryMgr::FreeUnusedMemory()
{
	// It's necessary to protect the scriptNodePool from multiple 
	// simultaneous accesses, as the parser is used by several methods
	// that can be executed simultaneously.
	ENTERCRITICALSECTION(cs);

	int n;
	for( n = 0; n < (signed)scriptNodePool.GetLength(); n++ )
		userFree(scriptNodePool[n]);
	scriptNodePool.Allocate(0, false);

	LEAVECRITICALSECTION(cs);

	// The engine already protects against multiple threads 
	// compiling scripts simultaneously so this pool doesn't have 
	// to be protected again.
	for( n = 0; n < (signed)byteInstructionPool.GetLength(); n++ )
		userFree(byteInstructionPool[n]);
	byteInstructionPool.Allocate(0, false);
}

void *asCMemoryMgr::AllocScriptNode()
{
	ENTERCRITICALSECTION(cs);

	if( scriptNodePool.GetLength() )
	{
		void *tRet = scriptNodePool.PopLast();
		LEAVECRITICALSECTION(cs);
		return tRet;
	}

	LEAVECRITICALSECTION(cs);

#if defined(AS_DEBUG) 
	return ((asALLOCFUNCDEBUG_t)(userAlloc))(sizeof(asCScriptNode), __FILE__, __LINE__);
#else
	return userAlloc(sizeof(asCScriptNode));
#endif
}

void asCMemoryMgr::FreeScriptNode(void *ptr)
{
	ENTERCRITICALSECTION(cs);

	// Pre allocate memory for the array to avoid slow growth
	if( scriptNodePool.GetLength() == 0 )
		scriptNodePool.Allocate(100, 0);

	scriptNodePool.PushLast(ptr);

	LEAVECRITICALSECTION(cs);
}

#ifndef AS_NO_COMPILER

void *asCMemoryMgr::AllocByteInstruction()
{
	if( byteInstructionPool.GetLength() )
		return byteInstructionPool.PopLast();

#if defined(AS_DEBUG) 
	return ((asALLOCFUNCDEBUG_t)(userAlloc))(sizeof(asCByteInstruction), __FILE__, __LINE__);
#else
	return userAlloc(sizeof(asCByteInstruction));
#endif
}

void asCMemoryMgr::FreeByteInstruction(void *ptr)
{
	// Pre allocate memory for the array to avoid slow growth
	if( byteInstructionPool.GetLength() == 0 )
		byteInstructionPool.Allocate(100, 0);

	byteInstructionPool.PushLast(ptr);
}

#endif // AS_NO_COMPILER

END_AS_NAMESPACE



