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
// as_callfunc_mips.cpp
//
// These functions handle the actual calling of system functions
//
// This version is MIPS specific and was originally written
// by Manu Evans in April, 2006 for Playstation Portable (PSP)
//
// Support for Linux with MIPS was added by Andreas Jonsson in April, 2015
//


#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_MIPS

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

#include <stdio.h>
#include <stdlib.h>
#if !defined(AS_ANDROID)
#include <regdef.h>
#endif

BEGIN_AS_NAMESPACE

#if defined(__linux__) && defined(_ABIO32)

// The MIPS ABI used by Linux is implemented here
// (Tested on CI20 MIPS Creator with Debian Linux)
//
// ref: SYSTEM V 
//      APPLICATION BINARY INTERFACE
//      MIPS RISC Processor
//      http://math-atlas.sourceforge.net/devel/assembly/mipsabi32.pdf
//
// ref: MIPS Instruction Reference
//      http://www.mrc.uidaho.edu/mrc/people/jff/digital/MIPSir.html

union SFloatRegs
{
	union { double d0; struct { float f0; asDWORD dummy0; };};
	union { double d1; struct { float f1; asDWORD dummy1; };};
} ;

extern "C" asQWORD mipsFunc(asUINT argSize, asDWORD *argBuffer, void *func, SFloatRegs &floatRegs);
asDWORD GetReturnedFloat();
asQWORD GetReturnedDouble();

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/, void *secondObject)
{
	asCScriptEngine *engine = context->m_engine;
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
	int callConv = sysFunc->callConv;

	asQWORD retQW = 0;

	void *func = (void*)sysFunc->func;
	void **vftable;

	asDWORD argBuffer[128]; // Ought to be big enough
	asASSERT( sysFunc->paramSize < 128 );
	
	asDWORD argOffset = 0;
	
	SFloatRegs floatRegs;
	asDWORD floatOffset = 0;
	
	// If the application function returns the value in memory then
	// the first argument must be the pointer to that memory
	if( sysFunc->hostReturnInMemory )
	{
		asASSERT( retPointer );
		argBuffer[argOffset++] = (asPWORD)retPointer;
	}
	
	if( callConv == ICC_CDECL_OBJFIRST || callConv == ICC_CDECL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_THISCALL || callConv == ICC_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL || callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ||
		callConv == ICC_THISCALL_OBJFIRST || callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_THISCALL_OBJFIRST_RETURNINMEM || callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM || 
		callConv == ICC_THISCALL_OBJLAST || callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_THISCALL_OBJLAST_RETURNINMEM || callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the first argument
		argBuffer[argOffset++] = (asPWORD)obj;
	}
	
	if( callConv == ICC_THISCALL_OBJFIRST || callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_THISCALL_OBJFIRST_RETURNINMEM || callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM )
	{
		// Add the second object pointer 
		argBuffer[argOffset++] = (asPWORD)secondObject;
	}
	
	int spos = 0;
	for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
	{
		asCDataType &paramType = descr->parameterTypes[n];
		if( paramType.IsObject() && !paramType.IsObjectHandle() && !paramType.IsReference() )
		{
			if( paramType.GetObjectType()->flags & COMPLEX_MASK )
			{
				// The object is passed by reference
				argBuffer[argOffset++] = args[spos++];
			}
			else
			{
				// Ensure 8byte alignment for classes that need it
				if( (paramType.GetObjectType()->flags & asOBJ_APP_CLASS_ALIGN8) && (argOffset & 1) )
					argOffset++;
			
				// Copy the object's memory to the buffer
				memcpy(&argBuffer[argOffset], *(void**)(args+spos), paramType.GetSizeInMemoryBytes());
				// Delete the original memory
				engine->CallFree(*(char**)(args+spos));
				spos++;
				argOffset += paramType.GetSizeInMemoryDWords();
			}
		}
		else if( paramType.GetTokenType() == ttQuestion )
		{
			// Copy both pointer and type id
			argBuffer[argOffset++] = args[spos++];
			argBuffer[argOffset++] = args[spos++];
		}
		else
		{
			// The first 2 floats or doubles are loaded into the float registers.
			// Actually this is only done if they are the first arguments to the function, 
			// but it doesn't cause any harm to load them into the registers even if they 
			// won't be used so we don't need to check if they really are the first args.
			if( floatOffset == 0 )
			{
				if( paramType.GetTokenType() == ttFloat )
					floatRegs.f0 = *reinterpret_cast<float*>(&args[spos]);
				else if( paramType.GetTokenType() == ttDouble )
					floatRegs.d0 = *reinterpret_cast<double*>(&args[spos]);
				floatOffset++;
			}
			else if( floatOffset == 1 )
			{
				if( paramType.GetTokenType() == ttFloat )
					floatRegs.f1 = *reinterpret_cast<float*>(&args[spos]);
				else if( paramType.GetTokenType() == ttDouble )
					floatRegs.d1 = *reinterpret_cast<double*>(&args[spos]);
				floatOffset++;
			}
		
			// Copy the value directly
			if( paramType.GetSizeOnStackDWords() > 1 )
			{
				// Make sure the argument is 8byte aligned
				if( argOffset & 1 )
					argOffset++;
				*reinterpret_cast<asQWORD*>(&argBuffer[argOffset]) = *reinterpret_cast<asQWORD*>(&args[spos]);
				argOffset += 2;
				spos += 2;
			}
			else
				argBuffer[argOffset++] = args[spos++];
		}
	}
	
	if( callConv == ICC_CDECL_OBJLAST || callConv == ICC_CDECL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the last argument
		argBuffer[argOffset++] = (asPWORD)obj;
	}

	if( callConv == ICC_THISCALL_OBJLAST || callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_THISCALL_OBJLAST_RETURNINMEM || callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Add the second object pointer 
		argBuffer[argOffset++] = (asPWORD)secondObject;
	}

	switch( callConv )
	{
	case ICC_CDECL:
	case ICC_CDECL_RETURNINMEM:
	case ICC_STDCALL:
	case ICC_STDCALL_RETURNINMEM:
	case ICC_CDECL_OBJLAST:
	case ICC_CDECL_OBJLAST_RETURNINMEM:
	case ICC_CDECL_OBJFIRST:
	case ICC_CDECL_OBJFIRST_RETURNINMEM:
	case ICC_THISCALL:
	case ICC_THISCALL_RETURNINMEM:
	case ICC_THISCALL_OBJFIRST:
	case ICC_THISCALL_OBJFIRST_RETURNINMEM:
	case ICC_THISCALL_OBJLAST:
	case ICC_THISCALL_OBJLAST_RETURNINMEM:
		retQW = mipsFunc(argOffset*4, argBuffer, func, floatRegs);
		break;

	case ICC_VIRTUAL_THISCALL:
	case ICC_VIRTUAL_THISCALL_RETURNINMEM:
	case ICC_VIRTUAL_THISCALL_OBJFIRST:
	case ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM:
	case ICC_VIRTUAL_THISCALL_OBJLAST:
	case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
		// Get virtual function table from the object pointer
		vftable = *(void***)obj;
		retQW = mipsFunc(argOffset*4, argBuffer, vftable[asPWORD(func)>>2], floatRegs);
		break;
	default:
		context->SetInternalException(TXT_INVALID_CALLING_CONVENTION);
	}

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

asDWORD GetReturnedFloat()
{
	asDWORD f;

	asm("swc1 $f0, %0\n" : "=m"(f));

	return f;
}

asQWORD GetReturnedDouble()
{
	asQWORD d = 0;

	asm("sdc1 $f0, %0\n" : "=m"(d));

	return d;
}

// asQWORD mipsFunc(asUINT argSize, asDWORD *argBuffer, void *func, SFloatRegs &floatRegs);
// $2,$3                   $4                $5               $6                $7
asm(
"	.text\n"
//"	.align 2\n"
"	.cfi_startproc\n"
"	.global mipsFunc\n"
"	.ent	mipsFunc\n"
"mipsFunc:\n"
//"	.frame	$fp,64,$31		# vars= 0, regs= 0/0, args= 0, gp= 0\n"
//"	.mask	0x00000000,0\n"
//"	.fmask	0x00000000,0\n"
"	.set	noreorder\n"
"	.set	nomacro\n"

// align the stack frame to 8 bytes
"	addiu	$12, $4, 7\n"		// t4 ($12) = argSize ($4) + 7
"	li		$13, -8\n"			// t5 ($13) = 0xfffffffffffffff8
"	and		$12, $12, $13\n"	// t4 ($12) &= t5 ($13). t4 holds the size of the argument block
// It is required that the caller reserves space for at least 16 bytes even if there are less than 4 arguments
// and add 8 bytes for the return pointer and s0 ($16) backup
"	addiu	$13, $12, 24\n"		// t5 = t4 + 24. t5 ($13) holds the total size of the stack frame (including return pointer)
// save the s0 register (so we can use it to remember where our return pointer is lives)
"	sw		$16, -4($sp)\n"		// store the s0 register (so we can use it to remember how big our stack frame is)
"	.cfi_offset 16, -4\n"
// store the return pointer
"	sw		$31, -8($sp)\n"
"	.cfi_offset 31, -8\n"
// keep original stack pointer
"	move	$16, $sp\n"
"	.cfi_def_cfa_register 16\n"
// push the stack
"	subu	$sp, $sp, $13\n"

// store the argument in temporary registers
"	addiu	$25, $6, 0\n"		// t9 ($25) holds the function pointer (must be t9 for position independent code)
"	addiu	$3, $4, 0\n"		// v1 ($3) holds the size of the argument buffer
"	move	$15, $5\n"			// t7 ($15) holds the pointer to the argBuffer
"	move	$14, $7\n"			// t6 ($14) holds the values for the float registers

// load integer registers
"	lw		$4, 0($15)\n"		// a0 ($4)
"	lw		$5, 4($15)\n"		// a1 ($5)
"	lw		$6, 8($15)\n"		// a2 ($6)
"	lw		$7, 12($15)\n"		// a3 ($7)

// load float registers
"	ldc1	$f12, 8($14)\n"
"	ldc1	$f14, 0($14)\n"

// skip stack parameters if there are 4 or less as they are moved into the registers
"	addi	$14, $3, -16\n"		// The first 4 args were already loaded into registers
"	blez	$14, andCall\n"
"	nop\n"

// push stack parameters
"pushArgs:\n"
"	addi	$3, -4\n"
// load from $15 + stack bytes ($3)
"	addu	$14, $15, $3\n"
"	lw		$14, 0($14)\n"
// store to $sp + stack bytes ($3)
"	addu	$13, $sp, $3\n"
"	sw		$14, 0($13)\n"
// if there are more, loop...
"	bne		$3, $0, pushArgs\n"
"	nop\n"

// and call the function
"andCall:\n"
"	jalr	$25\n"
"	nop\n"

// restore original stack pointer
"	move	$sp, $16\n"
// restore the return pointer
"	lw		$31, -8($sp)\n"
// restore the original value of $16
"	lw		$16, -4($sp)\n"
// and return from the function
"	jr		$31\n"
"	nop\n"

"	.set	macro\n"
"	.set	reorder\n"
"	.end	mipsFunc\n"
"	.cfi_endproc\n"
"	.size	mipsFunc, .-mipsFunc\n"
);

#else // !(defined(__linux__) && defined(_ABIO32))

// The MIPS ABI used by PSP and PS2 is implemented here

#define AS_MIPS_MAX_ARGS 32
#define AS_NUM_REG_FLOATS 8
#define AS_NUM_REG_INTS 8

// The array used to send values to the correct places.
// first 0-8 regular values to load into the a0-a3, t0-t3 registers
// then 0-8 float values to load into the f12-f19 registers
// then (AS_MIPS_MAX_ARGS - 16) values to load onto the stack
// the +1 is for when CallThis (object methods) is used
// extra +1 when returning in memory
extern "C" {
// TODO: This array shouldn't be global. It should be a local array in CallSystemFunctionNative
asDWORD mipsArgs[AS_MIPS_MAX_ARGS + 1 + 1];
}

// Loads all data into the correct places and calls the function.
// intArgSize is the size in bytes for how much data to put in int registers
// floatArgSize is the size in bytes for how much data to put in float registers
// stackArgSize is the size in bytes for how much data to put on the callstack
extern "C" asQWORD mipsFunc(int intArgSize, int floatArgSize, int stackArgSize, asDWORD func);

// puts the arguments in the correct place in the mipsArgs-array. See comments above.
// This could be done better.
inline void splitArgs(const asDWORD *args, int argNum, int &numRegIntArgs, int &numRegFloatArgs, int &numRestArgs, int hostFlags)
{
	int i;

	int argBit = 1;
	for (i = 0; i < argNum; i++)
	{
		if (hostFlags & argBit)
		{
			if (numRegFloatArgs < AS_NUM_REG_FLOATS)
			{
				// put in float register
				mipsArgs[AS_NUM_REG_INTS + numRegFloatArgs] = args[i];
				numRegFloatArgs++;
			}
			else
			{
				// put in stack
				mipsArgs[AS_NUM_REG_INTS + AS_NUM_REG_FLOATS + numRestArgs] = args[i];
				numRestArgs++;
			}
		}
		else
		{
			if (numRegIntArgs < AS_NUM_REG_INTS)
			{
				// put in int register
				mipsArgs[numRegIntArgs] = args[i];
				numRegIntArgs++;
			}
			else
			{
				// put in stack
				mipsArgs[AS_NUM_REG_INTS + AS_NUM_REG_FLOATS + numRestArgs] = args[i];
				numRestArgs++;
			}
		}
		argBit <<= 1;
	}
}

asQWORD CallCDeclFunction(const asDWORD *args, int argSize, asDWORD func, int flags)
{
	int argNum = argSize >> 2;

	int intArgs = 0;
	int floatArgs = 0;
	int restArgs = 0;

	// put the arguments in the correct places in the mipsArgs array
	if(argNum > 0)
		splitArgs(args, argNum, intArgs, floatArgs, restArgs, flags);

	return mipsFunc(intArgs << 2, floatArgs << 2, restArgs << 2, func);
}

// This function is identical to CallCDeclFunction, with the only difference that
// the value in the first parameter is the object
asQWORD CallThisCallFunction(const void *obj, const asDWORD *args, int argSize, asDWORD func, int flags)
{
	int argNum = argSize >> 2;

	int intArgs = 1;
	int floatArgs = 0;
	int restArgs = 0;

	mipsArgs[0] = (asDWORD) obj;

	// put the arguments in the correct places in the mipsArgs array
	if (argNum > 0)
		splitArgs(args, argNum, intArgs, floatArgs, restArgs, flags);

	return mipsFunc(intArgs << 2, floatArgs << 2, restArgs << 2, func);
}

// This function is identical to CallCDeclFunction, with the only difference that
// the value in the last parameter is the object
asQWORD CallThisCallFunction_objLast(const void *obj, const asDWORD *args, int argSize, asDWORD func, int flags)
{
	int argNum = argSize >> 2;

	int intArgs = 0;
	int floatArgs = 0;
	int restArgs = 0;

	// put the arguments in the correct places in the mipsArgs array
	if(argNum > 0)
		splitArgs(args, argNum, intArgs, floatArgs, restArgs, flags);

	if(intArgs < AS_NUM_REG_INTS)
	{
		mipsArgs[intArgs] = (asDWORD) obj;
		intArgs++;
	}
	else
	{
		mipsArgs[AS_NUM_REG_INTS + AS_NUM_REG_FLOATS + restArgs] = (asDWORD) obj;
		restArgs++;
	}

	return mipsFunc(intArgs << 2, floatArgs << 2, restArgs << 2, func);
}

asDWORD GetReturnedFloat()
{
	asDWORD f;

	asm("swc1 $f0, %0\n" : "=m"(f));

	return f;
}

asQWORD GetReturnedDouble()
{
	asQWORD d = 0;

	asm("sdc1 $f0, %0\n" : "=m"(d));

	return d;
}

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/, void */*secondObject*/)
{
	asCScriptEngine *engine = context->m_engine;
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
	int callConv = sysFunc->callConv;

	// TODO: Mips does not yet support THISCALL_OBJFIRST/LAST

	asQWORD retQW = 0;

	void    *func              = (void*)sysFunc->func;
	int      paramSize         = sysFunc->paramSize;
	asDWORD *vftable;

	if( descr->returnType.IsObject() && !descr->returnType.IsReference() && !descr->returnType.IsObjectHandle() )
	{
		mipsArgs[AS_MIPS_MAX_ARGS+1] = (asDWORD) retPointer;
	}

	asASSERT(descr->parameterTypes.GetLength() <= AS_MIPS_MAX_ARGS);

	// mark all float arguments
	int argBit = 1;
	int hostFlags = 0;
	int intArgs = 0;
	for( size_t a = 0; a < descr->parameterTypes.GetLength(); a++ )
	{
		if (descr->parameterTypes[a].IsFloatType()) 
			hostFlags |= argBit;
		else 
			intArgs++;
		argBit <<= 1;
	}

	asDWORD paramBuffer[64];
	if( sysFunc->takesObjByVal )
	{
		paramSize = 0;
		int spos = 0;
		int dpos = 1;
		for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
		{
			if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() )
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
				// Copy the value directly
				paramBuffer[dpos++] = args[spos++];
				if( descr->parameterTypes[n].GetSizeOnStackDWords() > 1 )
					paramBuffer[dpos++] = args[spos++];
				paramSize += descr->parameterTypes[n].GetSizeOnStackDWords();
			}
		}
		// Keep a free location at the beginning
		args = &paramBuffer[1];
	}

	switch( callConv )
	{
	case ICC_CDECL:
	case ICC_CDECL_RETURNINMEM:
	case ICC_STDCALL:
	case ICC_STDCALL_RETURNINMEM:
		retQW = CallCDeclFunction(args, paramSize<<2, (asDWORD)func, hostFlags);
		break;
	case ICC_THISCALL:
	case ICC_THISCALL_RETURNINMEM:
		retQW = CallThisCallFunction(obj, args, paramSize<<2, (asDWORD)func, hostFlags);
		break;
	case ICC_VIRTUAL_THISCALL:
	case ICC_VIRTUAL_THISCALL_RETURNINMEM:
		// Get virtual function table from the object pointer
		vftable = *(asDWORD**)obj;
		retQW = CallThisCallFunction(obj, args, paramSize<<2, vftable[asDWORD(func)>>2], hostFlags);
		break;
	case ICC_CDECL_OBJLAST:
	case ICC_CDECL_OBJLAST_RETURNINMEM:
		retQW = CallThisCallFunction_objLast(obj, args, paramSize<<2, (asDWORD)func, hostFlags);
		break;
	case ICC_CDECL_OBJFIRST:
	case ICC_CDECL_OBJFIRST_RETURNINMEM:
		retQW = CallThisCallFunction(obj, args, paramSize<<2, (asDWORD)func, hostFlags);
		break;
	default:
		context->SetInternalException(TXT_INVALID_CALLING_CONVENTION);
	}

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


asm(
"	.text\n"
//"	.align 2\n"
"	.global mipsFunc\n"
"	.ent	mipsFunc\n"
"mipsFunc:\n"
//"	.frame	$fp,64,$31		# vars= 0, regs= 0/0, args= 0, gp= 0\n"
//"	.mask	0x00000000,0\n"
//"	.fmask	0x00000000,0\n"
"	.set	noreorder\n"
"	.set	nomacro\n"
// align the stack frame to 8 bytes
"	addiu	$12, $6, 7\n"
"	li		$13, -8\n"			// 0xfffffffffffffff8
"	and		$12, $12, $13\n"	// t4 holds the size of the argument block
// and add 8 bytes for the return pointer and s0 backup
"	addiu	$13, $12, 8\n"		// t5 holds the total size of the stack frame (including return pointer)
// save the s0 register (so we can use it to remember where our return pointer is lives)
"	sw		$16, -4($sp)\n"		// store the s0 register (so we can use it to remember how big our stack frame is)
// push the stack
"	subu	$sp, $sp, $13\n"
// find the return address, place in s0
"	addu	$16, $sp, $12\n"
// store the return pointer
"	sw		$31, 0($16)\n"

// backup our function params
"	addiu	$2, $7, 0\n"
"	addiu	$3, $6, 0\n"

// get global mipsArgs[] array pointer
//"	lui		$15, %hi(mipsArgs)\n"
//"	addiu	$15, $15, %lo(mipsArgs)\n"
// we'll use the macro instead because SN Systems doesnt like %hi/%lo
".set macro\n"
" la  $15, mipsArgs\n"
".set nomacro\n"
// load register params
"	lw		$4, 0($15)\n"
"	lw		$5, 4($15)\n"
"	lw		$6, 8($15)\n"
"	lw		$7, 12($15)\n"
"	lw		$8, 16($15)\n"
"	lw		$9, 20($15)\n"
"	lw		$10, 24($15)\n"
"	lw		$11, 28($15)\n"

// load float params
"	lwc1	$f12, 32($15)\n"
"	lwc1	$f13, 36($15)\n"
"	lwc1	$f14, 40($15)\n"
"	lwc1	$f15, 44($15)\n"
"	lwc1	$f16, 48($15)\n"
"	lwc1	$f17, 52($15)\n"
"	lwc1	$f18, 56($15)\n"
"	lwc1	$f19, 60($15)\n"

// skip stack paramaters if there are none
"	beq		$3, $0, andCall\n"

// push stack paramaters
"	addiu	$15, $15, 64\n"
"pushArgs:\n"
"	addiu	$3, -4\n"
// load from $15 + stack bytes ($3)
"	addu	$14, $15, $3\n"
"	lw		$14, 0($14)\n"
// store to $sp + stack bytes ($3)
"	addu	$13, $sp, $3\n"
"	sw		$14, 0($13)\n"
// if there are more, loop...
"	bne		$3, $0, pushArgs\n"
"	nop\n"

// and call the function
"andCall:\n"
"	jal		$2\n"
"	nop\n"

// restore the return pointer
"	lw		$31, 0($16)\n"
// pop the stack pointer (remembering the return pointer was 8 bytes below the top)
"	addiu	$sp, $16, 8\n"
// and return from the function
"	jr		$31\n"
// restore the s0 register (in the branch delay slot)
"	lw		$16, -4($sp)\n"
"	.set	macro\n"
"	.set	reorder\n"
"	.end	mipsFunc\n"
"	.size	mipsFunc, .-mipsFunc\n"
);

#endif // PSP and PS2 MIPS ABI
	
END_AS_NAMESPACE

#endif // AS_MIPS
#endif // AS_MAX_PORTABILITY




