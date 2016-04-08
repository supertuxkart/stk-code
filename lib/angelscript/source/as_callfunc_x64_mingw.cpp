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

//
// This code was adapted from as_callfunc_x64_msvc by _Vicious_ on August 20th, 2011.
//
// Added support for functor methods by Jordi Oliveras Rovira in April, 2014.
//

#include <stdio.h>

#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_X64_MINGW

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

static asQWORD __attribute__((noinline)) CallX64(const asQWORD *args, const asQWORD *floatArgs, const int paramSize, asQWORD func)
{
	volatile asQWORD ret = 0;

	__asm__ __volatile__ (
		"# Move the parameters into registers before the rsp is modified\n"
		"mov %1, %%r10\n" // r10 = args
		"mov %2, %%r11\n" // r11 = floatArgs
		"xor %%r12, %%r12\n"
		"mov %3, %%r12d\n"
		"mov %4, %%r14\n" // r14 = func

        "# Store the stack pointer in r15 since it is guaranteed not to change over a function call\n"
        "mov %%rsp, %%r15\n"

		"# Allocate space on the stack for the arguments\n"
		"# Make room for at least 4 arguments even if there are less. When\n"
		"# the compiler does optimizations for speed it may use these for \n"
		"# temporary storage.\n"
		"mov %%r12, %%rdi\n"
		"add $32,%%edi\n"

		"# Make sure the stack pointer is 16byte aligned so the\n"
		"# whole program optimizations will work properly\n"
		"# TODO: runtime optimize: Can this be optimized with fewer instructions?\n"
		"mov %%rsp,%%rsi\n"
		"sub %%rdi,%%rsi\n"
		"and $0x8,%%rsi\n"
		"add %%rsi,%%rdi\n"
		"sub %%rdi,%%rsp\n"

		"# Jump straight to calling the function if no parameters\n"
		"cmp $0,%%r12 # Compare paramSize with 0\n"
		"je callfunc # Jump to call funtion if (paramSize == 0)\n"

		"# Copy arguments from script stack to application stack\n"
		"# Order is (first to last):\n"
		"# rcx, rdx, r8, r9 & everything else goes on stack\n"
		"movq   (%%r10),%%rcx\n"
		"movq  8(%%r10),%%rdx\n"
		"movq 16(%%r10),%%r8\n"
		"movq 24(%%r10),%%r9\n"

		"# Negate the 4 params from the size to be copied\n"
		"sub $32,%%r12d\n"
		"js copyfloat # Jump if negative result\n"
		"jz copyfloat # Jump if zero result\n"

		"# Now copy all remaining params onto stack allowing space for first four\n"
		"# params to be flushed back to the stack if required by the callee.\n"
		"add $32,%%r10 # Position input pointer 4 args ahead\n"
		"mov %%rsp,%%r13 # Put the stack pointer into r13\n"
		"add $32,%%r13 # Leave space for first 4 args on stack\n"

	"copyoverflow:\n"
		"movq (%%r10),%%rdi # Read param from source stack into rdi\n"
		"movq %%rdi,(%%r13) # Copy param to real stack\n"
		"add $8,%%r13 # Move virtual stack pointer\n"
		"add $8,%%r10 # Move source stack pointer\n"
		"sub $8,%%r12d # Decrement remaining count\n"
		"jnz copyoverflow # Continue if more params\n"

	"copyfloat:\n"
		"# Any floating point params?\n"
		"cmp $0,%%r11\n"
		"je callfunc\n"

		"movlpd   (%%r11),%%xmm0\n"
		"movlpd  8(%%r11),%%xmm1\n"
		"movlpd 16(%%r11),%%xmm2\n"
		"movlpd 24(%%r11),%%xmm3\n"

	"callfunc:\n"
		"call *%%r14\n"

        "# restore stack pointer\n"
        "mov %%r15, %%rsp\n"

		"lea  %0, %%rbx\n"     // Load the address of the ret variable into rbx
		"movq %%rax,(%%rbx)\n" // Copy the returned value into the ret variable

 		: // no output
		: "m" (ret), "r" (args), "r" (floatArgs), "r" (paramSize), "r" (func)
		: "rdi", "rsi", "rsp", "rbx", "r10", "r11", "%r12", "r13", "r14", "r15"
	);

	return ret;
}

static asDWORD GetReturnedFloat()
{
	volatile asDWORD ret = 0;

	__asm__ __volatile__ (
		"lea      %0, %%rax\n"
		"movss    %%xmm0, (%%rax)"
		: /* no output */
		: "m" (ret)
		: "%rax"
	);

	return ret;
}

static asQWORD GetReturnedDouble()
{
	volatile asQWORD ret = 0;

	__asm__ __volatile__ (
		"lea     %0, %%rax\n"
		"movlpd  %%xmm0, (%%rax)"
		: /* no optput */
		: "m" (ret)
		: "%rax"
	);

	return ret;
}

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

	if( sysFunc->hostReturnInMemory )
	{
		// The return is made in memory
		callConv++;

		// Set the return pointer as the first argument
		allArgBuffer[paramSize++] = (asQWORD)retPointer;
	}

#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
	if( callConv == ICC_THISCALL ||
		callConv == ICC_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL ||
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM )
#else
	// Optimization to avoid check 12 values (all ICC_ that contains THISCALL)
	if( (callConv >= ICC_THISCALL && callConv <= ICC_VIRTUAL_THISCALL_RETURNINMEM) ||
		(callConv >= ICC_THISCALL_OBJLAST && callConv <= ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM) )
#endif
	{
		// Add the object pointer as the first parameter
		allArgBuffer[paramSize++] = (asQWORD)obj;
	}

	if( callConv == ICC_CDECL_OBJFIRST ||
		callConv == ICC_CDECL_OBJFIRST_RETURNINMEM )
	{
		// Add the object pointer as the first parameter
		allArgBuffer[paramSize++] = (asQWORD)obj;
	}
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
	else if( callConv == ICC_THISCALL_OBJFIRST ||
		callConv == ICC_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM )
	{
		// Add the object pointer as the first parameter
		allArgBuffer[paramSize++] = (asQWORD)secondObject;
	}
#endif

#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
	if( callConv == ICC_VIRTUAL_THISCALL ||
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM )
#else
	if( callConv == ICC_VIRTUAL_THISCALL ||
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
#endif
	{
		// Get the true function pointer from the virtual function table
		vftable = *(void***)obj;
		func = vftable[asPWORD(func)>>3];
	}

	// Move the arguments to the buffer
	asUINT dpos = paramSize;
	asUINT spos = 0;
	for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
	{
		if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() )
		{
			if( descr->parameterTypes[n].GetSizeInMemoryDWords() >= AS_LARGE_OBJ_MIN_SIZE ||
				(descr->parameterTypes[n].GetObjectType()->flags & COMPLEX_MASK) )
			{
				allArgBuffer[dpos++] = *(asQWORD*)&args[spos];
				spos += AS_PTR_SIZE;
				paramSize++;
			}
			else
			{
				// Copy the object's memory to the buffer
				memcpy(&allArgBuffer[dpos], *(void**)(args+spos), descr->parameterTypes[n].GetSizeInMemoryBytes());

				// Delete the original memory
				engine->CallFree(*(char**)(args+spos));
				spos += AS_PTR_SIZE;
				asUINT dwords = descr->parameterTypes[n].GetSizeInMemoryDWords();
				asUINT qwords = (dwords >> 1) + (dwords & 1);
				dpos      += qwords;
				paramSize += qwords;
			}
		}
		else if( descr->parameterTypes[n].GetTokenType() == ttQuestion )
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
			asUINT dwords = descr->parameterTypes[n].GetSizeOnStackDWords();
			if( dwords > 1 )
			{
				allArgBuffer[dpos] = *(asQWORD*)&args[spos];

				// Double arguments are moved to a separate buffer in order to be placed in the XMM registers,
				// though this is only done for first 4 arguments, the rest are placed on the stack
				if( paramSize < 4 && descr->parameterTypes[n].IsDoubleType() )
					floatArgBuffer[dpos] = *(asQWORD*)&args[spos];

				dpos++;
				spos += 2;
			}
			else
			{
				allArgBuffer[dpos] = args[spos];

				// Float arguments are moved to a separate buffer in order to be placed in the XMM registers,
				// though this is only done for first 4 arguments, the rest are placed on the stack
				if( paramSize < 4 && descr->parameterTypes[n].IsFloatType() )
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
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
	else if( callConv == ICC_THISCALL_OBJLAST ||
		callConv == ICC_THISCALL_OBJLAST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the last parameter
		allArgBuffer[paramSize++] = (asQWORD)secondObject;
	}
#endif

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


