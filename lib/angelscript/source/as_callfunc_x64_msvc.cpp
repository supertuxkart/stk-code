/*
   AngelCode Scripting Library
   Copyright (c) 2003-2015 Andreas Jonsson

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
// Added support for thiscall methods by Jordi Oliveras Rovira in April, 2014.
//

#include <stdio.h>

#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_X64_MSVC

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

// These functions are implemented in as_callfunc_x64_msvc.asm
extern "C" asQWORD CallX64(const asQWORD *args, const asQWORD *floatArgs, int paramSize, asQWORD func);
extern "C" asDWORD GetReturnedFloat();
extern "C" asQWORD GetReturnedDouble();

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/, void *secondObject)
{
	asCScriptEngine *engine = context->m_engine;
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;

	asQWORD  retQW             = 0;
	void    *func              = (void*)sysFunc->func;
	asUINT   paramSize         = 0; // QWords
	void   **vftable;

	asQWORD  allArgBuffer[64];
	asQWORD  floatArgBuffer[4];

	int callConv = sysFunc->callConv;

	// Optimization to avoid check 12 values (all ICC_ that contains THISCALL)
	if( (callConv >= ICC_THISCALL && callConv <= ICC_VIRTUAL_THISCALL_RETURNINMEM) ||
		(callConv >= ICC_THISCALL_OBJLAST && callConv <= ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM) )
	{
		// Add the object pointer as the first parameter
		allArgBuffer[paramSize++] = (asQWORD)obj;
	}

	if( sysFunc->hostReturnInMemory )
	{
		// The return is made in memory
		callConv++;

		// Set the return pointer as the first argument
		allArgBuffer[paramSize++] = (asQWORD)retPointer;
	}

	if( callConv == ICC_CDECL_OBJFIRST ||
		callConv == ICC_CDECL_OBJFIRST_RETURNINMEM )
	{
		// Add the object pointer as the first parameter
		allArgBuffer[paramSize++] = (asQWORD)obj;
	}
	else if( callConv == ICC_THISCALL_OBJFIRST ||
		callConv == ICC_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM )
	{
		// Add the object pointer as the first parameter
		allArgBuffer[paramSize++] = (asQWORD)secondObject;
	}

	if( callConv == ICC_VIRTUAL_THISCALL ||
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Get the true function pointer from the virtual function table
		vftable = *(void***)obj;
		func = vftable[asPWORD(func)>>2];
	}

	// Move the arguments to the buffer
	asUINT dpos = paramSize;
	asUINT spos = 0;
	for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
	{
		asCDataType &dt = descr->parameterTypes[n];
		if( dt.IsObject() && !dt.IsObjectHandle() && !dt.IsReference() )
		{
			if( dt.GetSizeInMemoryDWords() >= AS_LARGE_OBJ_MIN_SIZE ||
				(dt.GetTypeInfo()->flags & COMPLEX_MASK) )
			{
				allArgBuffer[dpos++] = *(asQWORD*)&args[spos];
				spos += AS_PTR_SIZE;
				paramSize++;
			}
			else
			{
				// Copy the object's memory to the buffer
				memcpy(&allArgBuffer[dpos], *(void**)(args+spos), dt.GetSizeInMemoryBytes());

				// Delete the original memory
				engine->CallFree(*(char**)(args+spos));
				spos += AS_PTR_SIZE;
				asUINT dwords = dt.GetSizeInMemoryDWords();
				asUINT qwords = (dwords >> 1) + (dwords & 1);
				dpos      += qwords;
				paramSize += qwords;
			}
		}
		else if( dt.GetTokenType() == ttQuestion )
		{
			// Copy the reference and the type id
			allArgBuffer[dpos++] = *(asQWORD*)&args[spos];
			spos += 2;
			allArgBuffer[dpos++] = args[spos++];
			paramSize += 2;
		}
		else
		{
			// Copy the value directly
			asUINT dwords = dt.GetSizeOnStackDWords();
			if( dwords > 1 )
			{
				allArgBuffer[dpos] = *(asQWORD*)&args[spos];

				// Double arguments are moved to a separate buffer in order to be placed in the XMM registers,
				// though this is only done for first 4 arguments, the rest are placed on the stack
				if( paramSize < 4 && dt.IsDoubleType() )
					floatArgBuffer[dpos] = *(asQWORD*)&args[spos];

				dpos++;
				spos += 2;
			}
			else
			{
				allArgBuffer[dpos] = args[spos];

				// Float arguments are moved to a separate buffer in order to be placed in the XMM registers,
				// though this is only done for first 4 arguments, the rest are placed on the stack
				if( paramSize < 4 && dt.IsFloatType() )
					floatArgBuffer[dpos] = args[spos];
				
				dpos++;
				spos++;
			}

			paramSize++;
		}
	}

	if( callConv == ICC_CDECL_OBJLAST ||
		callConv == ICC_CDECL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the last parameter
		allArgBuffer[paramSize++] = (asQWORD)obj;
	}
	else if( callConv == ICC_THISCALL_OBJLAST ||
		callConv == ICC_THISCALL_OBJLAST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the last parameter
		allArgBuffer[paramSize++] = (asQWORD)secondObject;
	}

	retQW = CallX64(allArgBuffer, floatArgBuffer, paramSize*8, (asPWORD)func);

	// If the return is a float value we need to get the value from the FP register
	if( sysFunc->hostReturnFloat )
	{
		if( sysFunc->hostReturnSize == 1 )
			*(asDWORD*)&retQW = GetReturnedFloat();
		else
			retQW = GetReturnedDouble();
	}

	return retQW;
}

END_AS_NAMESPACE

#endif // AS_X64_MSVC
#endif // AS_MAX_PORTABILITY


