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
// as_callfunc_sh4.cpp
//
// These functions handle the actual calling of system functions
//
// This version is SH4 specific and was originally written
// by Fredrik Ehnbom in May, 2004
// Later updated for angelscript 2.0.0 by Fredrik Ehnbom in Jan, 2005

// References:
// * http://www.renesas.com/avs/resource/japan/eng/pdf/mpumcu/e602156_sh4.pdf
// * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcechp40/html/_callsh4_SH_4_Calling_Standard.asp


#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_SH4

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

#include <stdio.h>
#include <stdlib.h>

BEGIN_AS_NAMESPACE

#define AS_SH4_MAX_ARGS 32
// The array used to send values to the correct places.
// first 0-4 regular values to load into the r4-r7 registers
// then 0-8 float values to load into the fr4-fr11 registers
// then (AS_SH4_MAX_ARGS - 12) values to load onto the stack
// the +1 is for when CallThis (object methods) is used
// extra +1 when returning in memory
extern "C" {
static asDWORD sh4Args[AS_SH4_MAX_ARGS + 1 + 1];
}

// Loads all data into the correct places and calls the function.
// intArgSize is the size in bytes for how much data to put in int registers
// floatArgSize is the size in bytes for how much data to put in float registers
// stackArgSize is the size in bytes for how much data to put on the callstack
extern "C" asQWORD sh4Func(int intArgSize, int floatArgSize, int stackArgSize, asDWORD func);

asm(""
"	.align 4\n"
"	.global _sh4Func\n"
"_sh4Func:\n"
"	mov.l	r14,@-r15\n"
"	mov.l	r13,@-r15\n"
"	mov.l	r12,@-r15\n"
"	sts.l	pr,@-r15\n"		// must be saved since we call a subroutine
"	mov	r7, r14\n"		// func
"	mov	r6, r13\n"		// stackArgSize
"	mov.l	r5,@-r15\n"		// floatArgSize
"	mov.l	sh4Args,r0\n"
"	pref	@r0\n"
"	mov	r4, r1\n"		// intArgsize
"	mov	#33*4,r2\n"
"	extu.b	r2,r2\n"		// make unsigned (33*4 = 132 => 128)
"	mov.l	@(r0,r2), r2\n"		// r2 has adress for when returning in memory
"_sh4f_intarguments:\n"			// copy all the int arguments to the respective registers
"	mov	#4*2*2,r3\n"		// calculate how many bytes to skip
"	sub	r1,r3\n"
"	braf	r3\n"
"	add	#-4,r1\n"		// we are indexing the array backwards, so subtract one (delayed slot)
"	mov.l	@(r0,r1),r7\n"		// 4 arguments
"	add	#-4,r1\n"
"	mov.l	@(r0,r1),r6\n"		// 3 arguments
"	add	#-4,r1\n"
"	mov.l	@(r0,r1),r5\n"		// 2 arguments
"	add	#-4,r1\n"
"	mov.l	@(r0,r1),r4\n"		// 1 argument
"	nop\n"
"_sh4f_floatarguments:\n"		// copy all the float arguments to the respective registers
"	add	#4*4, r0\n"
"	mov.l	@r15+,r1\n"		// floatArgSize
"	mov	#8*2*2,r3\n"		// calculate how many bytes to skip
"	sub	r1,r3\n"
"	braf	r3\n"
"	add	#-4,r1\n"		// we are indexing the array backwards, so subtract one (delayed slot)
"	fmov.s	@(r0,r1),fr11\n"	// 8 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr10\n"	// 7 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr9\n"		// 6 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr8\n"		// 5 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr7\n"		// 4 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr6\n"		// 3 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr5\n"		// 2 arguments
"	add	#-4,r1\n"
"	fmov.s	@(r0,r1),fr4\n"		// 1 argument
"	nop\n"
"_sh4f_stackarguments:\n"		// copy all the stack argument onto the stack
"	add	#8*4, r0\n"
"	mov	r0, r1\n"
"	mov	#0, r0\n"		// init position counter (also used as a 0-check on the line after)
"	cmp/eq	r0, r13\n"
"	bt	_sh4f_functioncall\n"	// no arguments to push onto the stack
"	mov	r13, r3\n"		// stackArgSize
"	sub	r3,r15\n"		// "allocate" space on the stack
"	shlr2	r3\n"			// make into a counter
"_sh4f_stackloop:\n"
"	mov.l	@r1+, r12\n"
"	mov.l	r12, @(r0, r15)\n"
"	add	#4, r0\n"
"	dt	r3\n"
"	bf	_sh4f_stackloop\n"
"_sh4f_functioncall:\n"
"	jsr	@r14\n"			// no arguments
"	nop\n"
"	add	r13, r15\n"		// restore stack position
"	lds.l	@r15+,pr\n"
"	mov.l	@r15+, r12\n"
"	mov.l	@r15+, r13\n"
"	rts\n"
"	mov.l	@r15+, r14\n"		// delayed slot
"\n"
"	.align 4\n"
"sh4Args:\n"
"	.long	_sh4Args\n"
);

// puts the arguments in the correct place in the sh4Args-array. See comments above.
// This could be done better.
inline void splitArgs(const asDWORD *args, int argNum, int &numRegIntArgs, int &numRegFloatArgs, int &numRestArgs, int hostFlags) {
	int i;

	int argBit = 1;
	for (i = 0; i < argNum; i++) {
		if (hostFlags & argBit) {
			if (numRegFloatArgs < 12 - 4) {
				// put in float register
				sh4Args[4 + numRegFloatArgs] = args[i];
				numRegFloatArgs++;
			} else {
				// put in stack
				sh4Args[4 + 8 + numRestArgs] = args[i];
				numRestArgs++;
			}
		} else {
			if (numRegIntArgs < 8 - 4) {
				// put in int register
				sh4Args[numRegIntArgs] = args[i];
				numRegIntArgs++;
			} else {
				// put in stack
				sh4Args[4 + 8 + numRestArgs] = args[i];
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

	// put the arguments in the correct places in the sh4Args array
	if (argNum > 0)
		splitArgs(args, argNum, intArgs, floatArgs, restArgs, flags);

	return sh4Func(intArgs << 2, floatArgs << 2, restArgs << 2, func);
}

// This function is identical to CallCDeclFunction, with the only difference that
// the value in the first parameter is the object
asQWORD CallThisCallFunction(const void *obj, const asDWORD *args, int argSize, asDWORD func, int flags)
{
	int argNum = argSize >> 2;

	int intArgs = 1;
	int floatArgs = 0;
	int restArgs = 0;

	sh4Args[0] = (asDWORD) obj;
	
	// put the arguments in the correct places in the sh4Args array
	if (argNum >= 1)
		splitArgs(args, argNum, intArgs, floatArgs, restArgs, flags);

	return sh4Func(intArgs << 2, floatArgs << 2, restArgs << 2, func);
}
// This function is identical to CallCDeclFunction, with the only difference that
// the value in the last parameter is the object
asQWORD CallThisCallFunction_objLast(const void *obj, const asDWORD *args, int argSize, asDWORD func, int flags)
{
	int argNum = argSize >> 2;

	int intArgs = 0;
	int floatArgs = 0;
	int restArgs = 0;


	// put the arguments in the correct places in the sh4Args array
	if (argNum >= 1)
		splitArgs(args, argNum, intArgs, floatArgs, restArgs, flags);

	if (intArgs < 4) {
		sh4Args[intArgs] = (asDWORD) obj;
		intArgs++;
	} else {
		sh4Args[4 + 8 + restArgs] = (asDWORD) obj;
		restArgs++;
	}
	

	return sh4Func(intArgs << 2, floatArgs << 2, restArgs << 2, func);
}

asDWORD GetReturnedFloat()
{
	asDWORD f;

	asm("fmov.s	fr0, %0\n" : "=m"(f));

	return f;
}

// sizeof(double) == 4 with sh-elf-gcc (3.4.0) -m4
// so this isn't really used...
asQWORD GetReturnedDouble()
{
	asQWORD d;

	asm("fmov	dr0, %0\n" : "=m"(d));

	return d;
}

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/)
{
	asCScriptEngine *engine = context->m_engine;
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
	int callConv = sysFunc->callConv;

	asQWORD retQW = 0;

	void    *func              = (void*)sysFunc->func;
	int      paramSize         = sysFunc->paramSize;
	asDWORD *vftable;

	if( descr->returnType.IsObject() && !descr->returnType.IsReference() && !descr->returnType.IsObjectHandle() )
	{
		sh4Args[AS_SH4_MAX_ARGS+1] = (asDWORD) retPointer;
	}

	asASSERT(descr->parameterTypes.GetLength() <= 32);

	// mark all float arguments
	int argBit = 1;
	int hostFlags = 0;
	int intArgs = 0;
	for( asUINT a = 0; a < descr->parameterTypes.GetLength(); a++ ) {
		if (descr->parameterTypes[a].IsFloatType()) {
			hostFlags |= argBit;
		} else intArgs++;
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

END_AS_NAMESPACE

#endif // AS_SH4
#endif // AS_MAX_PORTABILITY


