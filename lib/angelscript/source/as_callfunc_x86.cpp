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
// as_callfunc_x86.cpp
//
// These functions handle the actual calling of system functions
//



#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_X86

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

//
// With some compile level optimizations the functions don't clear the FPU
// stack themselves. So we have to do it as part of calling the native functions,
// as the compiler will not be able to predict when it is supposed to do it by
// itself due to the dynamic nature of scripts
//
// - fninit clears the FPU stack and the FPU control word
// - emms only clears the FPU stack, while preserving the FPU control word
//
// By default I use fninit as it seems to be what works for most people,
// but some may find it necessary to define this as emms instead.
//
// TODO: Figure out when one or the other must be used, and a way to
//       configure this automatically in as_config.h
//
#ifndef CLEAR_FPU_STACK
#define CLEAR_FPU_STACK fninit
#endif

// These macros are just to allow me to use the above macro in the GNUC style inline assembly
#define _S(x) _TOSTRING(x)
#define _TOSTRING(x) #x

// Prototypes
asQWORD CallCDeclFunction(const asDWORD *args, int paramSize, asFUNCTION_t func);
asQWORD CallCDeclFunctionObjLast(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func);
asQWORD CallCDeclFunctionObjFirst(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func);
asQWORD CallCDeclFunctionRetByRef(const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr);
asQWORD CallCDeclFunctionRetByRefObjLast(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr);
asQWORD CallCDeclFunctionRetByRefObjFirst(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr);
asQWORD CallSTDCallFunction(const asDWORD *args, int paramSize, asFUNCTION_t func);
asQWORD CallThisCallFunction(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func);
asQWORD CallThisCallFunctionRetByRef(const void *, const asDWORD *, int, asFUNCTION_t, void *retPtr);

asDWORD GetReturnedFloat();
asQWORD GetReturnedDouble();

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &/*retQW2*/)
{
	asCScriptEngine            *engine    = context->m_engine;
	asSSystemFunctionInterface *sysFunc   = descr->sysFuncIntf;

	asQWORD retQW = 0;

	// Prepare the parameters
	int paramSize = sysFunc->paramSize;
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
					// TODO: bug: Must call the object's copy constructor instead of doing a memcpy, 
					//            as the object may hold a pointer to itself. It's not enough to 
					//            change only this memcpy as the assembler routine also makes a copy
					//            of paramBuffer to the final stack location. To avoid the second 
					//            copy the C++ routine should point paramBuffer to the final stack
					//            position and copy the values directly to that location. The assembler
					//            routines then don't need to copy anything, and will just be 
					//            responsible for setting up the registers and the stack frame appropriately.
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

	// Make the actual call
	asFUNCTION_t func = sysFunc->func;
	int callConv = sysFunc->callConv;
	if( sysFunc->hostReturnInMemory )
		callConv++;

	switch( callConv )
	{
	case ICC_CDECL:
		retQW = CallCDeclFunction(args, paramSize<<2, func);
		break;

	case ICC_CDECL_RETURNINMEM:
		retQW = CallCDeclFunctionRetByRef(args, paramSize<<2, func, retPointer);
		break;

	case ICC_STDCALL:
		retQW = CallSTDCallFunction(args, paramSize<<2, func);
		break;

	case ICC_STDCALL_RETURNINMEM:
		// Push the return pointer on the stack
		paramSize++;
		args--;
		*(asPWORD*)args = (size_t)retPointer;

		retQW = CallSTDCallFunction(args, paramSize<<2, func);
		break;

	case ICC_THISCALL:
		retQW = CallThisCallFunction(obj, args, paramSize<<2, func);
		break;

	case ICC_THISCALL_RETURNINMEM:
		retQW = CallThisCallFunctionRetByRef(obj, args, paramSize<<2, func, retPointer);
		break;

	case ICC_VIRTUAL_THISCALL:
		{
			// Get virtual function table from the object pointer
			asFUNCTION_t *vftable = *(asFUNCTION_t**)obj;
			retQW = CallThisCallFunction(obj, args, paramSize<<2, vftable[FuncPtrToUInt(func)>>2]);
		}
		break;

	case ICC_VIRTUAL_THISCALL_RETURNINMEM:
		{
			// Get virtual function table from the object pointer
			asFUNCTION_t *vftable = *(asFUNCTION_t**)obj;
			retQW = CallThisCallFunctionRetByRef(obj, args, paramSize<<2, vftable[FuncPtrToUInt(func)>>2], retPointer);
		}
		break;

	case ICC_CDECL_OBJLAST:
		retQW = CallCDeclFunctionObjLast(obj, args, paramSize<<2, func);
		break;

	case ICC_CDECL_OBJLAST_RETURNINMEM:
		// Call the system object method as a cdecl with the obj ref as the last parameter
		retQW = CallCDeclFunctionRetByRefObjLast(obj, args, paramSize<<2, func, retPointer);
		break;

	case ICC_CDECL_OBJFIRST:
		// Call the system object method as a cdecl with the obj ref as the first parameter
		retQW = CallCDeclFunctionObjFirst(obj, args, paramSize<<2, func);
		break;

	case ICC_CDECL_OBJFIRST_RETURNINMEM:
		// Call the system object method as a cdecl with the obj ref as the first parameter
		retQW = CallCDeclFunctionRetByRefObjFirst(obj, args, paramSize<<2, func, retPointer);
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

// On GCC we need to prevent the compiler from inlining these assembler routines when
// optimizing for speed (-O3), as the loop labels get duplicated which cause compile errors.

#ifdef __GNUC__
    #define NOINLINE __attribute ((__noinline__))
#else
    #define NOINLINE
#endif


asQWORD NOINLINE CallCDeclFunction(const asDWORD *args, int paramSize, asFUNCTION_t func)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	// It is not possible to rely on ESP or BSP to refer to variables or arguments on the stack
	// depending on compiler settings BSP may not even be used, and the ESP is not always on the
	// same offset from the local variables. Because the code adjusts the ESP register it is not
	// possible to inform the arguments through symbolic names below.

	// It's not also not possible to rely on the memory layout of the function arguments, because
	// on some compiler versions and settings the arguments may be copied to local variables with a
	// different ordering before they are accessed by the rest of the code.

	// I'm copying the arguments into this array where I know the exact memory layout. The address
	// of this array will then be passed to the inline asm in the EDX register.
	volatile asPWORD a[] = {asPWORD(args), asPWORD(paramSize), asPWORD(func)};

	asm __volatile__(
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  4(%%ebx), %%eax  \n" // paramSize
		"addl  $4, %%eax        \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		// Copy all arguments to the stack and call the function
		"movl  4(%%ebx), %%ecx  \n" // paramSize
		"movl  0(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy          \n"
		"copyloop:              \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop         \n"
		"endcopy:               \n"
		"call  *8(%%ebx)        \n"
		"addl  4(%%ebx), %%esp  \n" // pop arguments

		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asQWORD NOINLINE CallCDeclFunctionObjLast(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Push the object pointer as the last argument to the function
		push obj

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize
		add  esp, 4

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(obj), asPWORD(args), asPWORD(paramSize), asPWORD(func)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  8(%%ebx), %%eax  \n" // paramSize
		"addl  $8, %%eax        \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"pushl 0(%%ebx)         \n" // obj
		"movl  8(%%ebx), %%ecx  \n" // paramSize
		"movl  4(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy8         \n"
		"copyloop8:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop8        \n"
		"endcopy8:              \n"
		"call  *12(%%ebx)       \n"
		"addl  8(%%ebx), %%esp  \n" // pop arguments
		"addl  $4, %%esp        \n" // pop obj

		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asQWORD NOINLINE CallCDeclFunctionObjFirst(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// push object as first parameter
		push obj

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize
		add  esp, 4

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(obj), asPWORD(args), asPWORD(paramSize), asPWORD(func)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  8(%%ebx), %%eax  \n" // paramSize
		"addl  $8, %%eax        \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"movl  8(%%ebx), %%ecx  \n" // paramSize
		"movl  4(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy6         \n"
		"copyloop6:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop6        \n"
		"endcopy6:              \n"
		"pushl 0(%%ebx)         \n" // push obj
		"call  *12(%%ebx)       \n"
		"addl  8(%%ebx), %%esp  \n" // pop arguments
		"addl  $4, %%esp        \n" // pop obj

		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asQWORD NOINLINE CallCDeclFunctionRetByRefObjFirst(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Push the object pointer
		push obj

		// Push the return pointer
		push retPtr;

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize

#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 8
#else
		add  esp, 4
#endif

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(obj), asPWORD(args), asPWORD(paramSize), asPWORD(func), asPWORD(retPtr)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  8(%%ebx), %%eax  \n" // paramSize
		"addl  $12, %%eax       \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"movl  8(%%ebx), %%ecx  \n" // paramSize
		"movl  4(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy5         \n"
		"copyloop5:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop5        \n"
		"endcopy5:              \n"
		"pushl 0(%%ebx)         \n" // push object first
		"pushl 16(%%ebx)        \n" // retPtr
		"call  *12(%%ebx)       \n" // func
		"addl  8(%%ebx), %%esp  \n" // pop arguments
#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $8, %%esp        \n" // Pop the return pointer and object pointer
#else
		"addl  $4, %%esp        \n" // Pop the object pointer
#endif
		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);
#endif

	return retQW;
}

asQWORD NOINLINE CallCDeclFunctionRetByRef(const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Push the return pointer
		push retPtr;

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize

#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 4
#endif

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(args), asPWORD(paramSize), asPWORD(func), asPWORD(retPtr)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  4(%%ebx), %%eax  \n" // paramSize
		"addl  $8, %%eax        \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"movl  4(%%ebx), %%ecx  \n" // paramSize
		"movl  0(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy7         \n"
		"copyloop7:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop7        \n"
		"endcopy7:              \n"
		"pushl 12(%%ebx)        \n" // retPtr
		"call  *8(%%ebx)        \n" // func
		"addl  4(%%ebx), %%esp  \n" // pop arguments
#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $4, %%esp        \n" // Pop the return pointer
#endif
		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asQWORD NOINLINE CallCDeclFunctionRetByRefObjLast(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		push obj

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Push the return pointer
		push retPtr;

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize
		add  esp, 4

#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 4
#endif

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(obj), asPWORD(args), asPWORD(paramSize), asPWORD(func), asPWORD(retPtr)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  8(%%ebx), %%eax  \n" // paramSize
		"addl  $12, %%eax       \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"pushl 0(%%ebx)         \n" // obj
		"movl  8(%%ebx), %%ecx  \n" // paramSize
		"movl  4(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy4         \n"
		"copyloop4:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop4        \n"
		"endcopy4:              \n"
		"pushl 16(%%ebx)        \n" // retPtr
		"call  *12(%%ebx)       \n" // func
		"addl  8(%%ebx), %%esp  \n" // pop arguments
#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $8, %%esp        \n" // Pop the return pointer and object pointer
#else
		"addl  $4, %%esp        \n" // Pop the object pointer
#endif
		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asQWORD NOINLINE CallSTDCallFunction(const asDWORD *args, int paramSize, asFUNCTION_t func)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Call function
		call [func]

		// The callee already removed parameters from the stack

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(args), asPWORD(paramSize), asPWORD(func)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  4(%%ebx), %%eax  \n" // paramSize
		"addl  $4, %%eax        \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"movl  4(%%ebx), %%ecx  \n" // paramSize
		"movl  0(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy2         \n"
		"copyloop2:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop2        \n"
		"endcopy2:              \n"
		"call  *8(%%ebx)        \n" // callee pops the arguments

		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}


asQWORD NOINLINE CallThisCallFunction(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Push the object pointer on the stack
		push obj
#else
		// Move object pointer to ECX
		mov  ecx, obj
#endif

		// Call function
		call [func]

#ifndef THISCALL_CALLEE_POPS_ARGUMENTS
		// Pop arguments
		add  esp, paramSize
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Pop object pointer
		add esp, 4
#endif
#endif

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(obj), asPWORD(args), asPWORD(paramSize), asPWORD(func)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)    "\n"
		"pushl %%ebx            \n"
		"movl  %%edx, %%ebx     \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  8(%%ebx), %%eax  \n" // paramSize
		"addl  $8, %%eax        \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx     \n"
		"subl  %%eax, %%ecx     \n"
		"andl  $15, %%ecx       \n"
		"movl  %%esp, %%eax     \n"
		"subl  %%ecx, %%esp     \n"
		"pushl %%eax            \n" // Store the original stack pointer

		"movl  8(%%ebx), %%ecx  \n" // paramSize
		"movl  4(%%ebx), %%eax  \n" // args
		"addl  %%ecx, %%eax     \n" // push all arguments on the stack
		"cmp   $0, %%ecx        \n"
		"je    endcopy1         \n"
		"copyloop1:             \n"
		"subl  $4, %%eax        \n"
		"pushl (%%eax)          \n"
		"subl  $4, %%ecx        \n"
		"jne   copyloop1        \n"
		"endcopy1:              \n"
		"movl  0(%%ebx), %%ecx  \n" // move obj into ECX
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		"pushl %%ecx            \n" // push obj on the stack
#endif
		"call  *12(%%ebx)       \n"
#ifndef THISCALL_CALLEE_POPS_ARGUMENTS
		"addl  8(%%ebx), %%esp  \n" // pop arguments
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		"addl  $4, %%esp        \n" // pop obj
#endif
#endif
		// Pop the alignment bytes
		"popl  %%esp            \n"
		"popl  %%ebx            \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asQWORD NOINLINE CallThisCallFunctionRetByRef(const void *obj, const asDWORD *args, int paramSize, asFUNCTION_t func, void *retPtr)
{
	volatile asQWORD retQW = 0;

#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
		push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		CLEAR_FPU_STACK

		// Copy arguments from script
		// stack to application stack
		mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Push the object pointer on the stack
		push obj
#else
		// Move object pointer to ECX
		mov  ecx, obj
#endif

		// Push the return pointer
		push retPtr

		// Call function
		call [func]

#ifndef THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 4
#endif

#ifndef THISCALL_CALLEE_POPS_ARGUMENTS
		// Pop arguments
		add  esp, paramSize
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Pop object pointer
		add esp, 4
#endif
#endif

		// Copy return value from EAX:EDX
		lea  ecx, retQW
		mov  [ecx], eax
		mov  4[ecx], edx

		// Restore registers
		pop  ecx
	}

#elif defined ASM_AT_N_T

	volatile asPWORD a[] = {asPWORD(obj), asPWORD(args), asPWORD(paramSize), asPWORD(func), asPWORD(retPtr)};

	asm __volatile__ (
		_S(CLEAR_FPU_STACK)   "\n"
		"pushl %%ebx           \n"
		"movl  %%edx, %%ebx    \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  8(%%ebx), %%eax \n" // paramSize
		"addl  $12, %%eax      \n" // counting esp that we will push on the stack
		"movl  %%esp, %%ecx    \n"
		"subl  %%eax, %%ecx    \n"
		"andl  $15, %%ecx      \n"
		"movl  %%esp, %%eax    \n"
		"subl  %%ecx, %%esp    \n"
		"pushl %%eax           \n" // Store the original stack pointer

		"movl  8(%%ebx), %%ecx \n" // paramSize
		"movl  4(%%ebx), %%eax \n" // args
		"addl  %%ecx, %%eax    \n" // push all arguments to the stack
		"cmp   $0, %%ecx       \n"
		"je    endcopy3        \n"
		"copyloop3:            \n"
		"subl  $4, %%eax       \n"
		"pushl (%%eax)         \n"
		"subl  $4, %%ecx       \n"
		"jne   copyloop3       \n"
		"endcopy3:             \n"
#if defined(__MINGW32__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || __GNUC__ > 4)
        // MinGW made some strange choices with 4.7, and the thiscall calling convention
        // when returning an object in memory is completely different from when not returning
        // in memory
        "pushl 0(%%ebx)        \n" // push obj on the stack
        "movl 16(%%ebx), %%ecx \n" // move the return pointer into ECX
        "call  *12(%%ebx)      \n" // call the function
#else
		"movl  0(%%ebx), %%ecx \n" // move obj into ECX
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		"pushl %%ecx           \n" // push obj on the stack
#endif
		"pushl 16(%%ebx)       \n" // push retPtr on the stack
		"call  *12(%%ebx)      \n"
#ifndef THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $4, %%esp       \n" // pop return pointer
#endif
#ifndef THISCALL_CALLEE_POPS_ARGUMENTS
		"addl  8(%%ebx), %%esp \n" // pop arguments
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		"addl  $4, %%esp       \n" // pop the object pointer
#endif
#endif
#endif // MINGW
		// Pop the alignment bytes
		"popl  %%esp           \n"
		"popl  %%ebx           \n"

		// Copy EAX:EDX to retQW. As the stack pointer has been
		// restored it is now safe to access the local variable
		"leal  %1, %%ecx        \n"
		"movl  %%eax, 0(%%ecx)  \n"
		"movl  %%edx, 4(%%ecx)  \n"
		:                           // output
		: "d"(a), "m"(retQW)        // input - pass pointer of args in edx, pass pointer of retQW in memory argument
		: "%eax", "%ecx"            // clobber
		);

#endif

	return retQW;
}

asDWORD GetReturnedFloat()
{
	asDWORD f;

#if defined ASM_INTEL

	// Get the float value from ST0
	__asm fstp dword ptr [f]

#elif defined ASM_AT_N_T

	asm("fstps %0 \n" : "=m" (f));

#endif

	return f;
}

asQWORD GetReturnedDouble()
{
	asQWORD d;

#if defined ASM_INTEL

	// Get the double value from ST0
	__asm fstp qword ptr [d]

#elif defined ASM_AT_N_T

	asm("fstpl %0 \n" : "=m" (d));

#endif

	return d;
}

END_AS_NAMESPACE

#endif // AS_X86
#endif // AS_MAX_PORTABILITY




