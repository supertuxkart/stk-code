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
// as_callfunc_arm.cpp
//
// These functions handle the actual calling of system functions on the arm platform
//
// Written by Fredrik Ehnbom in June 2009, based on as_callfunc_x86.cpp
//
// The code was complemented to support Linux with ARM by Carlos Luna in December, 2012.
//
// Added support for functor methods by Jordi Oliveras Rovira in April, 2014.


// This code has to conform to both AAPCS and the modified ABI for iOS
//
// Reference:
//
// AAPCS: http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042d/IHI0042D_aapcs.pdf
// iOS: http://developer.apple.com/library/ios/documentation/Xcode/Conceptual/iPhoneOSABIReference/iPhoneOSABIReference.pdf

#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_ARM

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

#if defined(AS_SOFTFP)

// This code supports the soft-float ABI, i.e. g++ -mfloat-abi=softfp
//
// The code for iOS, Android, Marmalade and Windows Phone goes here

BEGIN_AS_NAMESPACE

extern "C" asQWORD armFunc          (const asDWORD *, int, asFUNCTION_t);
extern "C" asQWORD armFuncR0        (const asDWORD *, int, asFUNCTION_t, asDWORD r0);
extern "C" asQWORD armFuncR0R1      (const asDWORD *, int, asFUNCTION_t, asDWORD r0, asDWORD r1);
extern "C" asQWORD armFuncObjLast   (const asDWORD *, int, asFUNCTION_t, asDWORD obj);
extern "C" asQWORD armFuncR0ObjLast (const asDWORD *, int, asFUNCTION_t, asDWORD r0, asDWORD obj);

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/, void *secondObject)
{
	asCScriptEngine *engine = context->m_engine;
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
	int callConv = sysFunc->callConv;

	asQWORD       retQW     = 0;
	asFUNCTION_t  func      = sysFunc->func;
	int           paramSize = sysFunc->paramSize;
	asFUNCTION_t *vftable;

	if( sysFunc->hostReturnInMemory )
	{
		// The return is made in memory
		callConv++;
	}
	bool isThisCallMethod = callConv >= ICC_THISCALL_OBJLAST;


	asDWORD paramBuffer[64+2];
	// Android & Linux needs to align 64bit types on even registers, but this isn't done on iOS or Windows Phone
	// TODO: optimize runtime: There should be a check for this in PrepareSystemFunction() so this 
	//                         doesn't have to be done for functions that don't have any 64bit types
#if !defined(AS_ANDROID) && !defined(AS_LINUX)
	// In cases of thiscall methods, the callstack is configured as a standard thiscall
	// adding the secondObject as first or last element in callstack
	if( sysFunc->takesObjByVal || isThisCallMethod )
#endif
	{
#if defined(AS_ANDROID) || defined(AS_LINUX)
		// mask is used as a toggler to skip uneven registers.
		int mask = 1;

		if( isThisCallMethod )
		{
			mask = 0;
		}
		else
		{
			// Check for object pointer as first argument
			switch( callConv )
			{
				case ICC_THISCALL:
				case ICC_CDECL_OBJFIRST:
				case ICC_VIRTUAL_THISCALL:
				case ICC_THISCALL_RETURNINMEM:
				case ICC_CDECL_OBJFIRST_RETURNINMEM:
				case ICC_VIRTUAL_THISCALL_RETURNINMEM:
					mask = 0;
					break;
				default:
					break;
			}
		}

		// Check for hidden address in case of return by value
		if( sysFunc->hostReturnInMemory )
			mask = !mask;
#endif
		paramSize = 0;
		int spos = 0;
		int dpos = 2;

		if( isThisCallMethod && (callConv >= ICC_THISCALL_OBJFIRST &&
			callConv <= ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM) )
		{
			// Add the object pointer as the first parameter
			paramBuffer[dpos++] = (asDWORD)secondObject;
			paramSize++;
		}

		for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
		{
			// TODO: runtime optimize: Declare a reference to descr->parameterTypes[n] so the array doesn't have to be access all the time
			if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() )
			{
#ifdef COMPLEX_OBJS_PASSED_BY_REF
				if( descr->parameterTypes[n].GetTypeInfo()->flags & COMPLEX_MASK )
				{
					paramBuffer[dpos++] = args[spos++];
					paramSize++;
				}
				else
#endif
				{
#if defined(AS_ANDROID) || defined(AS_LINUX)
					if( (descr->parameterTypes[n].GetTypeInfo()->flags & asOBJ_APP_CLASS_ALIGN8) &&
						((dpos & 1) == mask) )
					{
						// 64 bit value align
						dpos++;
						paramSize++;
					}
#endif
					// Copy the object's memory to the buffer
					memcpy(&paramBuffer[dpos], *(void**)(args+spos), descr->parameterTypes[n].GetSizeInMemoryBytes());

					// Delete the original memory
					engine->CallFree(*(char**)(args+spos));
					spos++;
					dpos += descr->parameterTypes[n].GetSizeInMemoryDWords();
					paramSize += descr->parameterTypes[n].GetSizeInMemoryDWords();
				}
			}
			else
			{
#if defined(AS_ANDROID) || defined(AS_LINUX)
				// Should an alignment be performed?
				if( !descr->parameterTypes[n].IsObjectHandle() && 
					!descr->parameterTypes[n].IsReference() && 
					descr->parameterTypes[n].GetSizeOnStackDWords() == 2 && 
					((dpos & 1) == mask) )
				{
					// 64 bit value align
					dpos++;
					paramSize++;
				}
#endif

				// Copy the value directly
				paramBuffer[dpos++] = args[spos++];
				if( descr->parameterTypes[n].GetSizeOnStackDWords() > 1 )
					paramBuffer[dpos++] = args[spos++];
				paramSize += descr->parameterTypes[n].GetSizeOnStackDWords();
			}
		}

		if( isThisCallMethod && (callConv >= ICC_THISCALL_OBJLAST &&
			callConv <= ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM) )
		{
			// Add the object pointer as the last parameter
			paramBuffer[dpos++] = (asDWORD)secondObject;
			paramSize++;
		}

		// Keep a free location at the beginning
		args = &paramBuffer[2];
	}

	switch( callConv )
	{
	case ICC_CDECL_RETURNINMEM:     // fall through
	case ICC_STDCALL_RETURNINMEM:
		retQW = armFuncR0(args, paramSize<<2, func, (asDWORD)retPointer);
		break;
	case ICC_CDECL:     // fall through
	case ICC_STDCALL:
		retQW = armFunc(args, paramSize<<2, func);
		break;
	case ICC_THISCALL:  // fall through
	case ICC_CDECL_OBJFIRST:
	case ICC_THISCALL_OBJFIRST:
	case ICC_THISCALL_OBJLAST:
		retQW = armFuncR0(args, paramSize<<2, func, (asDWORD)obj);
		break;
	case ICC_THISCALL_RETURNINMEM:
	case ICC_THISCALL_OBJFIRST_RETURNINMEM:
	case ICC_THISCALL_OBJLAST_RETURNINMEM:
#ifdef __GNUC__
		// On GNUC the address where the return value will be placed should be put in R0
		retQW = armFuncR0R1(args, paramSize<<2, func, (asDWORD)retPointer, (asDWORD)obj);
#else
		// On Windows the R0 should always hold the object pointer, and the address for the return value comes after
		retQW = armFuncR0R1(args, paramSize<<2, func, (asDWORD)obj, (asDWORD)retPointer);
#endif
		break;
	case ICC_CDECL_OBJFIRST_RETURNINMEM:
		retQW = armFuncR0R1(args, paramSize<<2, func, (asDWORD)retPointer, (asDWORD)obj);
		break;
	case ICC_VIRTUAL_THISCALL:
	case ICC_VIRTUAL_THISCALL_OBJFIRST:
	case ICC_VIRTUAL_THISCALL_OBJLAST:
		// Get virtual function table from the object pointer
		vftable = *(asFUNCTION_t**)obj;
		retQW = armFuncR0(args, paramSize<<2, vftable[FuncPtrToUInt(func)>>2], (asDWORD)obj);
		break;
	case ICC_VIRTUAL_THISCALL_RETURNINMEM:
	case ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM:
	case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
		// Get virtual function table from the object pointer
		vftable = *(asFUNCTION_t**)obj;
#ifdef __GNUC__
		// On GNUC the address where the return value will be placed should be put in R0
		retQW = armFuncR0R1(args, (paramSize+1)<<2, vftable[FuncPtrToUInt(func)>>2], (asDWORD)retPointer, (asDWORD)obj);
#else
		// On Windows the R0 should always hold the object pointer, and the address for the return value comes after
		retQW = armFuncR0R1(args, (paramSize+1)<<2, vftable[FuncPtrToUInt(func)>>2], (asDWORD)obj, (asDWORD)retPointer);
#endif
		break;
	case ICC_CDECL_OBJLAST:
		retQW = armFuncObjLast(args, paramSize<<2, func, (asDWORD)obj);
		break;
	case ICC_CDECL_OBJLAST_RETURNINMEM:
		retQW = armFuncR0ObjLast(args, paramSize<<2, func, (asDWORD)retPointer, (asDWORD)obj);
		break;
	default:
		context->SetInternalException(TXT_INVALID_CALLING_CONVENTION);
	}

	return retQW;
}

END_AS_NAMESPACE

#elif !defined(AS_SOFTFP)

// This code supports the hard-float ABI, i.e. g++ -mfloat-abi=hard
// The main difference is that the floating point values are passed in the fpu registers

#define VFP_OFFSET 70
#define STACK_OFFSET 6
#define PARAM_BUFFER_SIZE 104

BEGIN_AS_NAMESPACE

extern "C" asQWORD armFunc          (const asDWORD *, int, asFUNCTION_t);
extern "C" asQWORD armFuncR0        (const asDWORD *, int, asFUNCTION_t, asDWORD r0);
extern "C" asQWORD armFuncR0R1      (const asDWORD *, int, asFUNCTION_t, asDWORD r0, asDWORD r1);
extern "C" asQWORD armFuncObjLast   (const asDWORD *, int, asFUNCTION_t, asDWORD obj);
extern "C" asQWORD armFuncR0ObjLast (const asDWORD *, int, asFUNCTION_t, asDWORD r0, asDWORD obj);

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/, void *secondObject)
{
	asCScriptEngine *engine = context->m_engine;
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
	int callConv = sysFunc->callConv;

	asQWORD       retQW     = 0;
	asFUNCTION_t  func      = sysFunc->func;
	int           paramSize = sysFunc->paramSize;
	asFUNCTION_t *vftable;

	//---------------------------------------------------------------------------- RPi
	int freeFloatSlot	= VFP_OFFSET;
	int freeDoubleSlot	= VFP_OFFSET;
	int stackPos		= STACK_OFFSET;
	int stackSize		= 0;
	//----------------------------------------------------------------------------

	//---------------------------------------------------------------------------- RPi
	// We´ll divide paramBuffer into several segments:
	//
	// 0-1							Unused
	// 2-5		(+8   / +0   asm)	values that should be placed in R0 - R3
	// 6-67		(+24  / +16  asm)	values that should be placed on the stack
	// 68		(+272 / +264 asm)	number of values stored in r registers (R0 - R3)
	// 69		(+276 / +268 asm)	number of args stored on the stack
	// 70-85	(+280 / +272 asm)	values that should be placed in VFP registers (16)
	// 86-87	(+344 / +336 asm)	sp original value - sp final value - for debugging
	// 88-103	(+352 / +344 asm)	Check area for free-used VFP registers
	//
	// Total number of elements: 104
	//
	// When passing the paramBuffer to the asm routines via the args pointer we are
	// offsetting the start of the array to being at element # 2. That´s why in asm
	// all addresses must have an offset of -2 words (-8 bytes).
	//---------------------------------------------------------------------------- RPi

	asDWORD paramBuffer[PARAM_BUFFER_SIZE];
	memset(paramBuffer, 0, sizeof(asDWORD) * PARAM_BUFFER_SIZE);

	if( sysFunc->hostReturnInMemory )
	{
		// TODO: runtime optimize: This check should be done in PrepareSystemFunction
		if ( !( descr->returnType.GetTypeInfo()->flags & COMPLEX_RETURN_MASK )		&&
			  ( descr->returnType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALLFLOATS )	&&
			    descr->returnType.GetSizeInMemoryBytes() <= 8 )
			callConv--;
		
		// The return is made in memory
		callConv++;
	}

	bool isThisCallMethod = callConv >= ICC_THISCALL_OBJLAST;

	// Linux needs to align 64bit types on even registers, but this isn't done on iOS or Windows Phone
	// TODO: optimize runtime: There should be a check for this in PrepareSystemFunction() so this 
	//                         doesn't have to be done for functions that don't have any 64bit types
	{
		// mask is used as a toggler to skip uneven registers.
		int mask = 1;

		if( isThisCallMethod )
		{
			mask = 0;
		}
		else
		{
			// Check for object pointer as first argument
			switch( callConv )
			{
				case ICC_THISCALL:
				case ICC_CDECL_OBJFIRST:
				case ICC_VIRTUAL_THISCALL:
				case ICC_THISCALL_RETURNINMEM:
				case ICC_CDECL_OBJFIRST_RETURNINMEM:
				case ICC_VIRTUAL_THISCALL_RETURNINMEM:
					mask = 0;
					break;
				default:
					break;
			}
		}
		// Check for hidden address in case of return by value
		if( sysFunc->hostReturnInMemory )
			mask = !mask;

		paramSize = 0;
		int spos = 0;
		int dpos = 2;

		if( isThisCallMethod && (callConv >= ICC_THISCALL_OBJFIRST &&
			callConv <= ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM) )
		{
			// Add the object pointer as the first parameter
			paramBuffer[dpos++] = (asDWORD)secondObject;
			paramSize++;
		}

		for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
		{
			// TODO: runtime optimize: Declare a reference to descr->parameterTypes[n] so the array doesn't have to be access all the time
			if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() &&
				!(descr->parameterTypes[n].GetTypeInfo()->flags & asOBJ_APP_ARRAY) )
			{
#ifdef COMPLEX_OBJS_PASSED_BY_REF
				if( descr->parameterTypes[n].GetTypeInfo()->flags & COMPLEX_MASK )
				{
					paramBuffer[dpos++] = args[spos++];
					paramSize++;
				}
				else
#endif
				{
					if( (descr->parameterTypes[n].GetTypeInfo()->flags & asOBJ_APP_CLASS_ALIGN8) )
					{
						if ( (dpos & 1) == mask )
						{
							// 64 bit value align
							dpos++;
							paramSize++;
						}

						if ( (stackPos & 1) == mask )
						{
							// 64 bit value align
							stackPos++;
							stackSize++;
						}
					}

					// Copy the object's memory to the buffer
					if (descr->parameterTypes[n].GetTypeInfo()->flags & asOBJ_APP_CLASS_ALLFLOATS)
					{
						int target = (freeFloatSlot > freeDoubleSlot) ? freeFloatSlot : freeDoubleSlot;

						if ( descr->parameterTypes[n].GetSizeInMemoryDWords() <= ( (VFP_OFFSET + 16) - target) )
						{
							memcpy(&paramBuffer[target], *(void**)(args+spos), descr->parameterTypes[n].GetSizeInMemoryBytes());
							memset(&paramBuffer[target + 18], (asDWORD)1, descr->parameterTypes[n].GetSizeInMemoryDWords());
							target += descr->parameterTypes[n].GetSizeInMemoryDWords();
							freeFloatSlot = freeDoubleSlot = target;
						}
						else
						{
							memcpy(&paramBuffer[stackPos], *(void**)(args+spos), descr->parameterTypes[n].GetSizeInMemoryBytes());
							stackPos += descr->parameterTypes[n].GetSizeInMemoryDWords();
							stackSize += descr->parameterTypes[n].GetSizeOnStackDWords();
						}
					}
					else
					{
						memcpy(&paramBuffer[dpos], *(void**)(args+spos), descr->parameterTypes[n].GetSizeInMemoryBytes());
						dpos += descr->parameterTypes[n].GetSizeInMemoryDWords();
						paramSize += descr->parameterTypes[n].GetSizeInMemoryDWords();
					}

					// Delete the original memory
					engine->CallFree(*(char**)(args+spos));
					spos++;					
				}

				continue;
			}
			else if( descr->parameterTypes[n].IsFloatType() && !descr->parameterTypes[n].IsReference() )
			{
				// Are there any "s" registers available?
				if ( freeFloatSlot < (VFP_OFFSET + 16) )
				{
					if (freeFloatSlot == freeDoubleSlot)
						freeDoubleSlot += 2;

					paramBuffer[freeFloatSlot + 18] = (asDWORD)1;
					paramBuffer[freeFloatSlot++] = args[spos++];

					while(freeFloatSlot < (VFP_OFFSET + 16) && paramBuffer[freeFloatSlot + 18] != 0)
						freeFloatSlot++;
				}
				// If not, then store the float arg in the stack area
				else
				{
					paramBuffer[stackPos++] = args[spos++];
					stackSize++;
				}

				continue;
			}
			else if( descr->parameterTypes[n].IsDoubleType() && !descr->parameterTypes[n].IsReference() )
			{
				// Are there any "d" registers available?
				if ( freeDoubleSlot < (VFP_OFFSET + 15) )
				{
					if (freeFloatSlot == freeDoubleSlot)
						freeFloatSlot += 2;

					// Copy two dwords for the double
					paramBuffer[freeDoubleSlot + 18] = (asDWORD)1;
					paramBuffer[freeDoubleSlot + 19] = (asDWORD)1;
					paramBuffer[freeDoubleSlot++] = args[spos++];
					paramBuffer[freeDoubleSlot++] = args[spos++];

					while(freeDoubleSlot < (VFP_OFFSET + 15) && paramBuffer[freeDoubleSlot + 18] != 0)
						freeDoubleSlot += 2;
				}
				// If not, then store the double arg in the stack area
				else
				{
					if ( (stackPos & 1) == mask )
					{
						// 64 bit value align
						stackPos++;
						stackSize++;
					}

					paramBuffer[stackPos++] = args[spos++];
					paramBuffer[stackPos++] = args[spos++];
					stackSize += 2;
				}
				
				continue;
			}
			else
			{
				// Copy the value directly to "r" registers or the stack, checking for alignment
				if (paramSize < 4)
				{
					// Should an alignment be performed?
					if( (dpos & 1) == mask && descr->parameterTypes[n].GetSizeOnStackDWords() == 2 && 
						!descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() &&
						!descr->parameterTypes[n].IsAnyType() )
					{
						// 64 bit value align
						dpos++;
						paramSize++;
					}

					paramBuffer[dpos++] = args[spos++];
					paramSize += descr->parameterTypes[n].GetSizeOnStackDWords();
				}
				else
				{
					// Should an alignment be performed?
					if( (stackPos & 1) == mask && descr->parameterTypes[n].GetSizeOnStackDWords() == 2 &&
						!descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() &&
						!descr->parameterTypes[n].IsAnyType() )
					{
						// 64 bit value align
						stackPos++;
						stackSize++;
					}

					paramBuffer[stackPos++] = args[spos++];
					stackSize += descr->parameterTypes[n].GetSizeOnStackDWords();
				}

				if( descr->parameterTypes[n].GetSizeOnStackDWords() > 1 )
				{
					if (paramSize < 5)
						paramBuffer[dpos++] = args[spos++];
					else
						paramBuffer[stackPos++] = args[spos++];
				}
			}// else...
		}// Loop

		if( isThisCallMethod && (callConv >= ICC_THISCALL_OBJLAST &&
			callConv <= ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM) )
		{
			if (paramSize < 4)
			{
				paramBuffer[dpos++] = (asDWORD)secondObject;
				paramSize++;
			}
			else
			{
				paramBuffer[stackPos++] = (asDWORD)secondObject;
				stackSize++;
			}
		}

		// Keep a free location at the beginning
		args = &paramBuffer[2];
	}

	paramBuffer[69] = static_cast<asDWORD>(stackSize<<2);

	switch( callConv )
	{
	case ICC_CDECL_RETURNINMEM:     // fall through
	case ICC_STDCALL_RETURNINMEM:
		retQW = armFuncR0(args, paramSize<<2, func, (asDWORD)retPointer);
		break;
	case ICC_CDECL:     // fall through
	case ICC_STDCALL:
		retQW = armFunc(args, paramSize<<2, func);
		break;
	case ICC_THISCALL:  // fall through
	case ICC_CDECL_OBJFIRST:
	case ICC_THISCALL_OBJFIRST:
	case ICC_THISCALL_OBJLAST:
		retQW = armFuncR0(args, paramSize<<2, func, (asDWORD)obj);
		break;
	case ICC_THISCALL_RETURNINMEM:
	case ICC_THISCALL_OBJFIRST_RETURNINMEM:
	case ICC_THISCALL_OBJLAST_RETURNINMEM:
		// On GNUC the address where the return value will be placed should be put in R0
		retQW = armFuncR0R1(args, paramSize<<2, func, (asDWORD)retPointer, (asDWORD)obj);
		break;
	case ICC_CDECL_OBJFIRST_RETURNINMEM:
		retQW = armFuncR0R1(args, paramSize<<2, func, (asDWORD)retPointer, (asDWORD)obj);
		break;
	case ICC_VIRTUAL_THISCALL:
	case ICC_VIRTUAL_THISCALL_OBJFIRST:
	case ICC_VIRTUAL_THISCALL_OBJLAST:
		// Get virtual function table from the object pointer
		vftable = *(asFUNCTION_t**)obj;
		retQW = armFuncR0(args, paramSize<<2, vftable[FuncPtrToUInt(func)>>2], (asDWORD)obj);
		break;
	case ICC_VIRTUAL_THISCALL_RETURNINMEM:
	case ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM:
	case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
		// Get virtual function table from the object pointer
		vftable = *(asFUNCTION_t**)obj;
		// On GNUC the address where the return value will be placed should be put in R0
		retQW = armFuncR0R1(args, (paramSize+1)<<2, vftable[FuncPtrToUInt(func)>>2], (asDWORD)retPointer, (asDWORD)obj);
		break;
	case ICC_CDECL_OBJLAST:
		retQW = armFuncObjLast(args, paramSize<<2, func, (asDWORD)obj);
		break;
	case ICC_CDECL_OBJLAST_RETURNINMEM:
		retQW = armFuncR0ObjLast(args, paramSize<<2, func, (asDWORD)retPointer, (asDWORD)obj);
		break;
	default:
		context->SetInternalException(TXT_INVALID_CALLING_CONVENTION);
	}

	// On Linux with arm the float and double values are returns in the 
	// floating point registers, s0 and s1. Objects that contain only 
	// float types and are not considered complex are also returned in the
	// floating point registers.
	if( sysFunc->hostReturnFloat )
	{
		retQW = paramBuffer[VFP_OFFSET];

		if ( sysFunc->hostReturnSize > 1 )
			retQW = *( (asQWORD*)&paramBuffer[VFP_OFFSET] );
	}
	else if ( descr->returnType.IsObject() )
	{
		// TODO: runtime optimize: This should be identified with a flag determined in PrepareSystemFunction
		if ( !descr->returnType.IsObjectHandle()									&&
			 !descr->returnType.IsReference()										&&
			 !(descr->returnType.GetTypeInfo()->flags & COMPLEX_RETURN_MASK)		&&
			 (descr->returnType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALLFLOATS) )
			memcpy( retPointer, &paramBuffer[VFP_OFFSET], descr->returnType.GetSizeInMemoryBytes() );
	}

	return retQW;
}

END_AS_NAMESPACE

#endif // AS_LINUX

#endif // AS_ARM
#endif // AS_MAX_PORTABILITY




