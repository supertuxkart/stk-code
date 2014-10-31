/*
   AngelCode Scripting Library
   Copyright (c) 2003-2013 Andreas Jonsson

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
// as_callfunc_xenon.cpp
//
// These functions handle the actual calling of system functions
//
// This version is Xenon specific
// Modified from as_callfunc_ppc.cpp by Laszlo Perneky February 2007
//
// Modified by Cyril Tissier March 2010:
// various fixes in 'float' args passing / function return
// properly handling 'double' type
// various fixes in asm ppcFunc
// fix for variable arguments
//



// XBox 360 calling convention
// ===========================
// I've yet to find an official document with the ABI for XBox 360, 
// but I'll describe what I've gathered from the code and tests
// performed by the AngelScript community.
//
// Arguments are passed in the following registers:
// r3  - r10   : integer/pointer arguments (each register is 64bit)
// fr1 - fr13  : float/double arguments    (each register is 64bit)
// 
// Arguments that don't fit in the registers will be pushed on the stack.
// 
// When a float or double is passed as argument, its value will be placed
// in the next available float register, but it will also reserve general
// purpose register. 
// 
// Example: void foo(float a, int b). a will be passed in fr1 and b in r4.
//
// For each argument passed to a function an 8byte slot is reserved on the 
// stack, so that the function can offload the value there if needed. The
// first slot is at r1+20, the next at r1+28, etc.
//
// If the function is a class method, the this pointer is passed as hidden 
// first argument. If the function returns an object in memory, the address
// for that memory is passed as hidden first argument.
//
// Return value are placed in the following registers:
// r3  : integer/pointer values
// fr1 : float/double values
//
// Rules for registers
// r1          : stack pointer
// r14-r31     : nonvolatile, i.e. their values must be preserved
// fr14-fr31   : nonvolatile, i.e. their values must be preserved
// r0, r2, r13 : dedicated. I'm not sure what it means, but it is probably best not to use them
//
// The stack pointer must always be aligned at 8 bytes.
//
// References:
// https://www-01.ibm.com/chips/techlib/techlib.nsf/techdocs/852569B20050FF77852569970071B0D6/$file/eabi_app.pdf
//
// TODO: The code doesn't handle int64 and uint64 parameters
// TODO: The code doesn't handle objects passed by value (unless they are max 4 bytes in size)



#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#if defined(AS_XENON)

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <xtl.h>

BEGIN_AS_NAMESPACE

#define AS_PPC_MAX_ARGS         32
#define AS_PPC_THISCALL_REG     1
#define AS_PPC_RETURNINMEM_REG  1
#define AS_PPC_ENDOFARGS        1

// The array used to send values to the correct places.
// Contains a byte of argTypes to indicate the register type to load, or zero if end of arguments
enum argTypes
{
	ppcENDARG    = 0,
	ppcINTARG    = 1,
	ppcFLOATARG  = 2,
	ppcDOUBLEARG = 3
};

// Loads all data into the correct places and calls the function.
// pArgs     is the array of the argument values
// pArgTypes is an array containing a byte indicating the type (enum argTypes) for each argument.
// dwFunc    is the address of the function that will be called
asQWORD __declspec( naked ) ppcFunc(const asDWORD* pArgs, asDWORD dwFunc, const asBYTE* pArgTypes)
{
	__asm
	{
_ppcFunc:
		// Prologue
		// Read and stack the link register (return address)
		mflr    r12
		stw     r12,-8(r1)

		// Backup all non-volatile registers we use in this function
		std     r31,-10h(r1) // stack pointer for pushing arguments
		std     r27,-18h(r1) // dwFunc
		std     r26,-20h(r1) // pArgs
		std     r25,-28h(r1) // pArgTypes
		std     r24,-30h(r1) // current arg type
		std     r23,-38h(r1) // counter for used GPRs
		std     r22,-40h(r1) // counter for used float registers

		// Setup the stack frame to make room for the backup of registers
		// and the arguments that will be passed to the application function.
		// 512 bytes is enough for about 50 arguments plus backup of 8
		// TODO: Should perhaps make this dynamic based on number of arguments
		stwu	r1,-200h(r1)

//////////////////////////////////////////////////////////////////////////
// Initialize local variables
//////////////////////////////////////////////////////////////////////////

		// r31 is our pointer into the stack where the arguments will be place
		// The MSVC optimizer seems to rely on nobody copying the r1 register directly
		// so we can't just do a simple 'addi r31, r1, 14h' as the optimizer may
		// end up moving this instruction to before the update of r1 above. 
		// Instead we'll read the previous stack pointer from the stack, and then 
		// subtract to get the correct offset.
		lwz		r31, 0(r1)
		subi	r31, r31, 1ECh	// prev r1 - 512 + 20 = curr r1 + 20

		mr r26, r3			// pArgs
		mr r27, r4			// dwFunc
		mr r25, r5			// pArgTypes

		// Counting of used/assigned GPR's
		sub  r23, r23, r23
		// Counting of used/assigned Float Registers
		sub  r22, r22, r22

		// Begin loading and stacking registers
		subi r25, r25, 1

//////////////////////////////////////////////////////////////////////////
// Fetch the next argument
//////////////////////////////////////////////////////////////////////////
ppcNextArg:
		// Increment rArgTypePtr
		addi r25, r25, 1
		// Get data type
		lbz r24, 0(r25)

		// r24 holds the data type
		cmplwi cr6, r24, 0
		beq    cr6, ppcArgsEnd
		cmplwi cr6, r24, 1
		beq    cr6, ppcArgIsInteger
		cmplwi cr6, r24, 2
		beq    cr6, ppcArgIsFloat
		cmplwi cr6, r24, 3
		beq    cr6, ppcArgIsDouble

//////////////////////////////////////////////////////////////////////////
// Load and stack integer arguments
//////////////////////////////////////////////////////////////////////////
ppcArgIsInteger:
		// Get the arg from the stack
		lwz r12, 0(r26)

		// r23 holds the integer arg count so far
		cmplwi cr6, r23, 0
		beq    cr6, ppcLoadIntReg0
		cmplwi cr6, r23, 1
		beq    cr6, ppcLoadIntReg1
		cmplwi cr6, r23, 2
		beq    cr6, ppcLoadIntReg2
		cmplwi cr6, r23, 3
		beq    cr6, ppcLoadIntReg3
		cmplwi cr6, r23, 4
		beq    cr6, ppcLoadIntReg4
		cmplwi cr6, r23, 5
		beq    cr6, ppcLoadIntReg5
		cmplwi cr6, r23, 6
		beq    cr6, ppcLoadIntReg6
		cmplwi cr6, r23, 7
		beq    cr6, ppcLoadIntReg7

		// no more than 8 parameters
		b		ppcLoadIntRegUpd

		ppcLoadIntReg0:
		mr r3, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg1:
		mr r4, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg2:
		mr r5, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg3:
		mr r6, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg4:
		mr r7, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg5:
		mr r8, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg6:
		mr r9, r12
		b ppcLoadIntRegUpd
		ppcLoadIntReg7:
		mr r10, r12
		b ppcLoadIntRegUpd

		ppcLoadIntRegUpd:
		stw	    r12, 0(r31)			// push on the stack
		addi	r31, r31, 8			// inc stack by 1 reg

		addi r23, r23, 1			// Increment used int register count
		addi r26, r26, 4			// Increment pArgs
		b ppcNextArg				// Call next arg

//////////////////////////////////////////////////////////////////////////
// Load and stack float arguments
//////////////////////////////////////////////////////////////////////////
ppcArgIsFloat:
		// Get the arg from the stack
		lfs fr0, 0(r26)

		// r22 holds the float arg count so far
		cmplwi cr6, r22, 0
		beq    cr6, ppcLoadFloatReg0
		cmplwi cr6, r22, 1
		beq    cr6, ppcLoadFloatReg1
		cmplwi cr6, r22, 2
		beq    cr6, ppcLoadFloatReg2
		cmplwi cr6, r22, 3
		beq    cr6, ppcLoadFloatReg3
		cmplwi cr6, r22, 4
		beq    cr6, ppcLoadFloatReg4
		cmplwi cr6, r22, 5
		beq    cr6, ppcLoadFloatReg5
		cmplwi cr6, r22, 6
		beq    cr6, ppcLoadFloatReg6
		cmplwi cr6, r22, 7
		beq    cr6, ppcLoadFloatReg7
		cmplwi cr6, r22, 8
		beq    cr6, ppcLoadFloatReg8
		cmplwi cr6, r22, 9
		beq    cr6, ppcLoadFloatReg9
		cmplwi cr6, r22, 10
		beq    cr6, ppcLoadFloatReg10
		cmplwi cr6, r22, 11
		beq    cr6, ppcLoadFloatReg11
		cmplwi cr6, r22, 12
		beq    cr6, ppcLoadFloatReg12

		// no more than 12 parameters
		b		ppcLoadFloatRegUpd

		ppcLoadFloatReg0:
		fmr fr1, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg1:
		fmr fr2, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg2:
		fmr fr3, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg3:
		fmr fr4, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg4:
		fmr fr5, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg5:
		fmr fr6, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg6:
		fmr fr7, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg7:
		fmr fr8, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg8:
		fmr fr9, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg9:
		fmr fr10, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg10:
		fmr fr11, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg11:
		fmr fr12, fr0
		b ppcLoadFloatRegUpd
		ppcLoadFloatReg12:
		fmr fr13, fr0
		b ppcLoadFloatRegUpd

		ppcLoadFloatRegUpd:
		stfs	fr0, 0(r31)			// push on the stack
		addi	r31, r31, 8			// inc stack by 1 reg
		
		addi r22, r22, 1			// Increment used float register count
		addi r23, r23, 1			// Increment used int register count - a float reg eats up a GPR		
		addi r26, r26, 4			// Increment pArgs
		b ppcNextArg				// Call next arg

//////////////////////////////////////////////////////////////////////////
// Load and stack double float arguments
//////////////////////////////////////////////////////////////////////////
ppcArgIsDouble:
		// Get the arg from the stack
		lfd fr0, 0(r26)

		// r22 holds the float arg count so far
		cmplwi cr6, r22, 0
		beq    cr6, ppcLoadDoubleReg0
		cmplwi cr6, r22, 1
		beq    cr6, ppcLoadDoubleReg1
		cmplwi cr6, r22, 2
		beq    cr6, ppcLoadDoubleReg2
		cmplwi cr6, r22, 3
		beq    cr6, ppcLoadDoubleReg3
		cmplwi cr6, r22, 4
		beq    cr6, ppcLoadDoubleReg4
		cmplwi cr6, r22, 5
		beq    cr6, ppcLoadDoubleReg5
		cmplwi cr6, r22, 6
		beq    cr6, ppcLoadDoubleReg6
		cmplwi cr6, r22, 7
		beq    cr6, ppcLoadDoubleReg7
		cmplwi cr6, r22, 8
		beq    cr6, ppcLoadDoubleReg8
		cmplwi cr6, r22, 9
		beq    cr6, ppcLoadDoubleReg9
		cmplwi cr6, r22, 10
		beq    cr6, ppcLoadDoubleReg10
		cmplwi cr6, r22, 11
		beq    cr6, ppcLoadDoubleReg11
		cmplwi cr6, r22, 12
		beq    cr6, ppcLoadDoubleReg12

		// no more than 12 parameters
		b      ppcLoadDoubleRegUpd

		ppcLoadDoubleReg0:
		fmr fr1, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg1:
		fmr fr2, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg2:
		fmr fr3, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg3:
		fmr fr4, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg4:
		fmr fr5, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg5:
		fmr fr6, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg6:
		fmr fr7, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg7:
		fmr fr8, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg8:
		fmr fr9, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg9:
		fmr fr10, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg10:
		fmr fr11, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg11:
		fmr fr12, fr0
		b ppcLoadDoubleRegUpd
		ppcLoadDoubleReg12:
		fmr fr13, fr0
		b ppcLoadDoubleRegUpd

		ppcLoadDoubleRegUpd:
		stfd	fr0, 0(r31)			// push on the stack
		addi	r31, r31, 8			// inc stack by 1 reg
		
		addi r22, r22, 1			// Increment used float register count		
		addi r23, r23, 1			// Increment used int register count
		addi r26, r26, 8			// Increment pArgs
		b ppcNextArg

//////////////////////////////////////////////////////////////////////////
// Finished
//////////////////////////////////////////////////////////////////////////
ppcArgsEnd:
		// Call the function
		mtctr r27
		bctrl

		// Epilogue
		// Restore callers stack
		addi	r1, r1, 200h

		// restore all registers we used in this fct
		ld     r22,-40h(r1)
		ld     r23,-38h(r1)
		ld     r24,-30h(r1)
		ld     r25,-28h(r1)
		ld     r26,-20h(r1)
		ld     r27,-18h(r1)
		ld     r31,-10h(r1)

		// Fetch return link to caller
		lwz     r12,-8(r1)
		mtlr	r12
		blr
	}
}

asDWORD GetReturnedFloat()
{
	// This variable must be declared volatile so that the 
	// compiler optimizations do not remove its initialization
	// with the fr1 register due to believing the fr1 register
	// isn't initialized.
	volatile asDWORD f;

	__asm
	{
		stfs fr1, f
	}

	return f;
}

asQWORD GetReturnedDouble()
{
	// This variable must be declared volatile so that the 
	// compiler optimizations do not remove its initialization
	// with the fr1 register due to believing the fr1 register
	// isn't initialized.
	volatile asQWORD f;

	__asm
	{
		stfd fr1, f
	}

	return f;
}

// returns true if the given parameter is a 'variable argument'
inline bool IsVariableArgument( asCDataType type )
{
	return (type.GetTokenType() == ttQuestion) ? true : false;
}

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/)
{
	asCScriptEngine            *engine    = context->m_engine;
	asSSystemFunctionInterface *sysFunc   = descr->sysFuncIntf;	
	int                         callConv  = sysFunc->callConv;
	asQWORD                     retQW     = 0;
	void                       *func      = (void*)sysFunc->func;
	asDWORD                    *vftable;

	// Pack the arguments into an array that ppcFunc() can use to load each CPU register properly
	asBYTE  ppcArgsType[AS_PPC_MAX_ARGS + AS_PPC_RETURNINMEM_REG + AS_PPC_THISCALL_REG + AS_PPC_ENDOFARGS];
	asDWORD ppcArgs[AS_PPC_MAX_ARGS + AS_PPC_RETURNINMEM_REG + AS_PPC_THISCALL_REG];
	int     argsCnt = 0;

	// If the function returns an object in memory, we allocate the memory and put the ptr to the front (will go to r3)
	if( sysFunc->hostReturnInMemory )
	{
		ppcArgs[argsCnt] = (asDWORD)retPointer;
		ppcArgsType[argsCnt] = ppcINTARG;
		argsCnt++;
	}

	// If we have an object and it's not objectlast, then we put it as the first arg
	if ( obj &&
		 callConv != ICC_CDECL_OBJLAST &&
		 callConv != ICC_CDECL_OBJLAST_RETURNINMEM )
	{
		ppcArgs[argsCnt] = (asDWORD)obj;
		ppcArgsType[argsCnt] = ppcINTARG;
		argsCnt++;
	}

	// If the function takes any objects by value, they must be copied 
	// to the stack, shifting the other arguments as necessary. paramBuffer
	// will then replace the args pointer that was received from the VM.
	// TODO: Is this really how XBox 360 passes objects by value?
	asDWORD paramBuffer[AS_PPC_MAX_ARGS];
	if( sysFunc->takesObjByVal )
	{
		int paramSize = 0;
		int spos = 0;
		int dpos = 1;

		for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
		{
			// Parameter object by value
			if( descr->parameterTypes[n].IsObject() && 
				!descr->parameterTypes[n].IsObjectHandle() &&
				!descr->parameterTypes[n].IsReference() )
			{
#ifdef COMPLEX_OBJS_PASSED_BY_REF
				if( descr->parameterTypes[n].GetObjectType()->flags & COMPLEX_MASK )
				{
					paramBuffer[dpos++] = args[spos++];
					paramSize++;
				}
				else
#endif
				{
					// Copy the object's memory to the buffer
					memcpy( &paramBuffer[dpos], *(void**)(args + spos), descr->parameterTypes[n].GetSizeInMemoryBytes() );

					// Delete the original memory
					engine->CallFree(*(char**)(args + spos));

					spos++;
					dpos += descr->parameterTypes[n].GetSizeInMemoryDWords();
					paramSize += descr->parameterTypes[n].GetSizeInMemoryDWords();
				}
			}
			else
			{
				// Copy the value directly
				paramBuffer[dpos++] = args[spos++];
				if( descr->parameterTypes[n].GetSizeOnStackDWords() > 1 )
					paramBuffer[dpos++] = args[spos++];
				paramSize += descr->parameterTypes[n].GetSizeOnStackDWords();
			}

			// If this was a variable argument parameter, then account for the implicit typeId
			if( IsVariableArgument( descr->parameterTypes[n] ) )
			{
				// the TypeId is just a DWORD
				paramBuffer[dpos++] = args[spos++];
				++paramSize;
			}
		}

		// Keep a free location at the beginning
		args = &paramBuffer[1];

		asASSERT( paramSize <= AS_PPC_MAX_ARGS );
	}


	const asUINT paramCount = (asUINT)descr->parameterTypes.GetLength();

	asBYTE * pCurArgType		= (asBYTE*)&ppcArgsType[argsCnt];
	asBYTE * pCurFixedArgValue	= (asBYTE*)&ppcArgs[argsCnt];
	asBYTE * pCurStackArgValue	= (asBYTE*)args;

	for( asUINT n = 0; n < paramCount; n++ )
	{		
		argsCnt++;

		if (descr->parameterTypes[n].IsFloatType() && !descr->parameterTypes[n].IsReference())
		{
			*pCurArgType++ = ppcFLOATARG;

			*((float*) pCurFixedArgValue) = *((float*) pCurStackArgValue);

			pCurFixedArgValue += 4;
			pCurStackArgValue += 4;
		}
		else if (descr->parameterTypes[n].IsDoubleType() && !descr->parameterTypes[n].IsReference())
		{
			*pCurArgType++ = ppcDOUBLEARG;

			*((double*) pCurFixedArgValue) = *((double*) pCurStackArgValue);

			pCurFixedArgValue += 8;
			pCurStackArgValue += 8;
		}
		else
		{
			// TODO: How should int64 and uint64 be passed natively? 
			//       Currently the code doesn't handle these types

			// TODO: The code also ignore the fact that large objects
			//       passed by value has been copied to the stack
			//       in the above loop.

			*pCurArgType++ = ppcINTARG;

			*((int*) pCurFixedArgValue) = *((int*) pCurStackArgValue);

			if( !descr->parameterTypes[n].IsReference() )
			{
				// If the arg is less that 4 bytes, then move the  
				// bytes to the higher bytes within the dword
				asUINT numBytes = descr->parameterTypes[n].GetSizeInMemoryBytes();
				if( numBytes == 1 )
				{
					pCurFixedArgValue[3] = pCurFixedArgValue[0];
					pCurFixedArgValue[0] = 0;
				}
				else if( numBytes == 2 )
				{
					*(asWORD*)&pCurFixedArgValue[2] = *(asWORD*)&pCurFixedArgValue[0];
					*(asWORD*)&pCurFixedArgValue[0] = 0;
				}
			}

			pCurFixedArgValue += 4;
			pCurStackArgValue += 4;

			// if it is a variable argument, account for the typeId
			// implicitly add another parameter (AFTER the parameter above) for the typeId
			if( IsVariableArgument(descr->parameterTypes[n]) )
			{			
				argsCnt++;

				*pCurArgType++ = ppcINTARG;

				*((int*) pCurFixedArgValue) = *((int*) pCurStackArgValue);

				pCurFixedArgValue += 4;
				pCurStackArgValue += 4;
			}
		}
	}

	// Add the arg list end indicator
	ppcArgsType[argsCnt] = ppcENDARG;

	switch( callConv )
	{
	case ICC_CDECL:
	case ICC_CDECL_RETURNINMEM:
	case ICC_STDCALL:
	case ICC_STDCALL_RETURNINMEM:
	case ICC_THISCALL:
	case ICC_THISCALL_RETURNINMEM:
	case ICC_CDECL_OBJFIRST:
	case ICC_CDECL_OBJFIRST_RETURNINMEM:
	{
		retQW = ppcFunc( ppcArgs, (asDWORD)func, ppcArgsType );
		break;
	}
	case ICC_VIRTUAL_THISCALL:
	case ICC_VIRTUAL_THISCALL_RETURNINMEM:
	{
		// Get virtual function table from the object pointer
		vftable = *(asDWORD**)obj;
		retQW = ppcFunc( ppcArgs, vftable[asDWORD(func)>>2], ppcArgsType );
		break;
	}
	case ICC_CDECL_OBJLAST:
	case ICC_CDECL_OBJLAST_RETURNINMEM:
	{
		// Add the object pointer as the last argument
		ppcArgsType[argsCnt++] = ppcINTARG;
		ppcArgsType[argsCnt] = ppcENDARG;
		*((asPWORD*)pCurFixedArgValue) = (asPWORD)obj;
		retQW = ppcFunc( ppcArgs, (asDWORD)func, ppcArgsType );
		break;
	}
	default:
		context->SetInternalException( TXT_INVALID_CALLING_CONVENTION );
	}

	// If the return is a float value we need to get the value from the FP register
	if( sysFunc->hostReturnFloat )
	{
		if( sysFunc->hostReturnSize == 1 )
			*(asDWORD*)&retQW = GetReturnedFloat();
		else
			retQW = GetReturnedDouble();
	}
	else if( sysFunc->hostReturnSize == 1 )
	{
		// Move the bits to the higher value to compensate for the adjustment that the caller does
		retQW <<= 32;
	}

	return retQW;
}

END_AS_NAMESPACE

#endif // AS_XENON
#endif // AS_MAX_PORTABILITY



