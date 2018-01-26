/*
   AngelCode Scripting Library
   Copyright (c) 2003-2017 Andreas Jonsson

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

/*
 * Implements the AMD64 calling convention for gcc-based 64bit Unices
 *
 * Author: Ionut "gargltk" Leonte <ileonte@bitdefender.com>
 *
 * Initial author: niteice
 *
 * Added support for functor methods by Jordi Oliveras Rovira in April, 2014.
 */

// Useful references for the System V AMD64 ABI:
// http://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/
// http://math-atlas.sourceforge.net/devel/assembly/abi_sysV_amd64.pdf
 
#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_X64_GCC

#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

enum argTypes { x64INTARG = 0, x64FLOATARG = 1 };
typedef asQWORD ( *funcptr_t )( void );

#define X64_MAX_ARGS             32
#define MAX_CALL_INT_REGISTERS    6
#define MAX_CALL_SSE_REGISTERS    8
#define X64_CALLSTACK_SIZE        ( X64_MAX_ARGS + MAX_CALL_SSE_REGISTERS + 3 )

// Note to self: Always remember to inform the used registers on the clobber line, 
// so that the gcc optimizer doesn't try to use them for other things

static asQWORD __attribute__((noinline)) X64_CallFunction(const asQWORD *args, int cnt, funcptr_t func, asQWORD &retQW2, bool returnFloat) 
{
	// Need to flag the variable as volatile so the compiler doesn't optimize out the variable
	volatile asQWORD retQW1 = 0;

	// Reference: http://www.x86-64.org/documentation/abi.pdf

	__asm__ __volatile__ (

		"  movq %0, %%rcx \n"	// rcx = cnt
		"  movq %1, %%r10 \n"	// r10 = args
		"  movq %2, %%r11 \n"	// r11 = func

	// Backup stack pointer in R15 that is guaranteed to maintain its value over function calls
		"  movq %%rsp, %%r15 \n"
#ifdef __OPTIMIZE__
	// Make sure the stack unwind logic knows we've backed up the stack pointer in register r15
	// This should only be done if any optimization is done. If no optimization (-O0) is used,
	// then the compiler already backups the rsp before entering the inline assembler code
		" .cfi_def_cfa_register r15 \n"
#endif

	// Skip the first 128 bytes on the stack frame, called "red zone",  
	// that might be used by the compiler to store temporary values
		"  sub $128, %%rsp \n"

	// Make sure the stack pointer will be aligned to 16 bytes when the function is called
		"  movq %%rcx, %%rdx \n"
		"  salq $3, %%rdx \n"
		"  movq %%rsp, %%rax \n"
		"  sub %%rdx, %%rax \n"
		"  and $15, %%rax \n"
		"  sub %%rax, %%rsp \n"

	// Push the stack parameters, i.e. the arguments that won't be loaded into registers
		"  movq %%rcx, %%rsi \n"
		"  testl %%esi, %%esi \n"
		"  jle endstack \n"
		"  subl $1, %%esi \n"
		"  xorl %%edx, %%edx \n"
		"  leaq 8(, %%rsi, 8), %%rcx \n"
		"loopstack: \n"
		"  movq 112(%%r10, %%rdx), %%rax \n"
		"  pushq %%rax \n"
		"  addq $8, %%rdx \n"
		"  cmpq %%rcx, %%rdx \n"
		"  jne loopstack \n"
		"endstack: \n"

	// Populate integer and floating point parameters
		"  movq %%r10, %%rax \n"
		"  mov     (%%rax), %%rdi \n"
		"  mov    8(%%rax), %%rsi \n"
		"  mov   16(%%rax), %%rdx \n"
		"  mov   24(%%rax), %%rcx \n"
		"  mov   32(%%rax), %%r8 \n"
		"  mov   40(%%rax), %%r9 \n"
		"  add   $48, %%rax \n"
		"  movsd   (%%rax), %%xmm0 \n"
		"  movsd  8(%%rax), %%xmm1 \n"
		"  movsd 16(%%rax), %%xmm2 \n"
		"  movsd 24(%%rax), %%xmm3 \n"
		"  movsd 32(%%rax), %%xmm4 \n"
		"  movsd 40(%%rax), %%xmm5 \n"
		"  movsd 48(%%rax), %%xmm6 \n"
		"  movsd 56(%%rax), %%xmm7 \n"

	// Call the function
		"  call *%%r11 \n"

	// Restore stack pointer
		"  mov %%r15, %%rsp \n"
#ifdef __OPTIMIZE__
	// Inform the stack unwind logic that the stack pointer has been restored
	// This should only be done if any optimization is done. If no optimization (-O0) is used,
	// then the compiler already backups the rsp before entering the inline assembler code
		" .cfi_def_cfa_register rsp \n"
#endif

	// Put return value in retQW1 and retQW2, using either RAX:RDX or XMM0:XMM1 depending on type of return value
		"  movl %5, %%ecx \n"
		"  testb %%cl, %%cl \n"
		"  je intret \n"
		"  lea %3, %%rax \n"
		"  movq %%xmm0, (%%rax) \n"
		"  lea %4, %%rdx \n"
		"  movq %%xmm1, (%%rdx) \n"
		"  jmp endcall \n"
		"intret: \n"
		"  movq %%rax, %3 \n"
		"  movq %%rdx, %4 \n"
		"endcall: \n"

		: : "g" ((asQWORD)cnt), "g" (args), "g" (func), "m" (retQW1), "m" (retQW2), "m" (returnFloat)
		: "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", 
		  "%rdi", "%rsi", "%rax", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r15");
		
	return retQW1;
}

// returns true if the given parameter is a 'variable argument'
static inline bool IsVariableArgument( asCDataType type )
{
	return ( type.GetTokenType() == ttQuestion ) ? true : false;
}

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &retQW2, void *secondObject)
{
	asCScriptEngine            *engine             = context->m_engine;
	asSSystemFunctionInterface *sysFunc            = descr->sysFuncIntf;
	int                         callConv           = sysFunc->callConv;
	asQWORD                     retQW              = 0;
	asDWORD                    *stack_pointer      = args;
	funcptr_t                  *vftable            = NULL;
	int                         totalArgumentCount = 0;
	int                         n                  = 0;
	int                         param_post         = 0;
	int                         argIndex           = 0;
	funcptr_t                   func               = (funcptr_t)sysFunc->func;

	if( sysFunc->hostReturnInMemory ) 
	{
		// The return is made in memory
		callConv++;
	}

#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
	// Determine the real function pointer in case of virtual method
	if ( obj && ( callConv == ICC_VIRTUAL_THISCALL || callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ) ) 
#else
	if ( obj && ( callConv == ICC_VIRTUAL_THISCALL ||
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM) )
#endif
	{
		vftable = *((funcptr_t**)obj);
		func = vftable[FuncPtrToUInt(asFUNCTION_t(func)) >> 3];
	}

	// Determine the type of the arguments, and prepare the input array for the X64_CallFunction 
	asQWORD  paramBuffer[X64_CALLSTACK_SIZE] = { 0 };
	asBYTE	 argsType[X64_CALLSTACK_SIZE] = { 0 };

	switch ( callConv ) 
	{
		case ICC_CDECL_RETURNINMEM:
		case ICC_STDCALL_RETURNINMEM: 
		{
			paramBuffer[0] = (asPWORD)retPointer;
			argsType[0] = x64INTARG;

			argIndex = 1;

			break;
		}
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
		case ICC_THISCALL_OBJLAST:
		case ICC_VIRTUAL_THISCALL_OBJLAST:
			param_post = 2;
#endif
		case ICC_THISCALL:
		case ICC_VIRTUAL_THISCALL:
		case ICC_CDECL_OBJFIRST: 
		{
			paramBuffer[0] = (asPWORD)obj;
			argsType[0] = x64INTARG;

			argIndex = 1;

			break;
		}
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
		case ICC_THISCALL_OBJLAST_RETURNINMEM:
		case ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM:
			param_post = 2;
#endif
		case ICC_THISCALL_RETURNINMEM:
		case ICC_VIRTUAL_THISCALL_RETURNINMEM:
		case ICC_CDECL_OBJFIRST_RETURNINMEM: 
		{
			paramBuffer[0] = (asPWORD)retPointer;
			paramBuffer[1] = (asPWORD)obj;
			argsType[0] = x64INTARG;
			argsType[1] = x64INTARG;

			argIndex = 2;

			break;
		}
#ifndef AS_NO_THISCALL_FUNCTOR_METHOD
		case ICC_THISCALL_OBJFIRST:
		case ICC_VIRTUAL_THISCALL_OBJFIRST:
		{
			paramBuffer[0] = (asPWORD)obj;
			paramBuffer[1] = (asPWORD)secondObject;
			argsType[0] = x64INTARG;
			argsType[1] = x64INTARG;

			argIndex = 2;
			break;
		}
		case ICC_THISCALL_OBJFIRST_RETURNINMEM:
		case ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM:
		{
			paramBuffer[0] = (asPWORD)retPointer;
			paramBuffer[1] = (asPWORD)obj;
			paramBuffer[2] = (asPWORD)secondObject;
			argsType[0] = x64INTARG;
			argsType[1] = x64INTARG;
			argsType[2] = x64INTARG;

			argIndex = 3;
			break;
		}
#endif
		case ICC_CDECL_OBJLAST:
			param_post = 1;
			break;
		case ICC_CDECL_OBJLAST_RETURNINMEM: 
		{
			paramBuffer[0] = (asPWORD)retPointer;
			argsType[0] = x64INTARG;

			argIndex = 1;
			param_post = 1;

			break;
		}
	}

	int argumentCount = ( int )descr->parameterTypes.GetLength();
	for( int a = 0; a < argumentCount; ++a ) 
	{
		const asCDataType &parmType = descr->parameterTypes[a];
		if( parmType.IsFloatType() && !parmType.IsReference() ) 
		{
			argsType[argIndex] = x64FLOATARG;
			memcpy(paramBuffer + argIndex, stack_pointer, sizeof(float));
			argIndex++;
			stack_pointer++;
		}
		else if( parmType.IsDoubleType() && !parmType.IsReference() ) 
		{
			argsType[argIndex] = x64FLOATARG;
			memcpy(paramBuffer + argIndex, stack_pointer, sizeof(double));
			argIndex++;
			stack_pointer += 2;
		}
		else if( IsVariableArgument( parmType ) ) 
		{
			// The variable args are really two, one pointer and one type id
			argsType[argIndex] = x64INTARG;
			argsType[argIndex+1] = x64INTARG;
			memcpy(paramBuffer + argIndex, stack_pointer, sizeof(void*));
			memcpy(paramBuffer + argIndex + 1, stack_pointer + 2, sizeof(asDWORD));
			argIndex += 2;
			stack_pointer += 3;
		}
		else if( parmType.IsPrimitive() ||
		         parmType.IsReference() || 
		         parmType.IsObjectHandle() )
		{
			argsType[argIndex] = x64INTARG;
			if( parmType.GetSizeOnStackDWords() == 1 )
			{
				memcpy(paramBuffer + argIndex, stack_pointer, sizeof(asDWORD));
				stack_pointer++;
			}
			else
			{
				memcpy(paramBuffer + argIndex, stack_pointer, sizeof(asQWORD));
				stack_pointer += 2;
			}
			argIndex++;
		}
		else
		{
			// An object is being passed by value
			if( (parmType.GetTypeInfo()->flags & COMPLEX_MASK) ||
			    parmType.GetSizeInMemoryDWords() > 4 )
			{
				// Copy the address of the object
				argsType[argIndex] = x64INTARG;
				memcpy(paramBuffer + argIndex, stack_pointer, sizeof(asQWORD));
				argIndex++;
			}
			else if( (parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALLINTS) ||
			         (parmType.GetTypeInfo()->flags & asOBJ_APP_PRIMITIVE) )
			{
				// Copy the value of the object
				if( parmType.GetSizeInMemoryDWords() > 2 )
				{
					argsType[argIndex] = x64INTARG;
					argsType[argIndex+1] = x64INTARG;
					memcpy(paramBuffer + argIndex, *(asDWORD**)stack_pointer, parmType.GetSizeInMemoryBytes());
					argIndex += 2;
				}
				else
				{
					argsType[argIndex] = x64INTARG;
					memcpy(paramBuffer + argIndex, *(asDWORD**)stack_pointer, parmType.GetSizeInMemoryBytes());
					argIndex++;
				}
				// Delete the original memory
				engine->CallFree(*(void**)stack_pointer);
			}
			else if( (parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALLFLOATS) ||
			         (parmType.GetTypeInfo()->flags & asOBJ_APP_FLOAT) )
			{
				// Copy the value of the object
				if( parmType.GetSizeInMemoryDWords() > 2 )
				{
					argsType[argIndex] = x64FLOATARG;
					argsType[argIndex+1] = x64FLOATARG;
					memcpy(paramBuffer + argIndex, *(asDWORD**)stack_pointer, parmType.GetSizeInMemoryBytes());
					argIndex += 2;
				}
				else
				{
					argsType[argIndex] = x64FLOATARG;
					memcpy(paramBuffer + argIndex, *(asDWORD**)stack_pointer, parmType.GetSizeInMemoryBytes());
					argIndex++;
				}
				// Delete the original memory
				engine->CallFree(*(void**)stack_pointer);
			}
			stack_pointer += 2;
		}
	}

	// For the CDECL_OBJ_LAST calling convention we need to add the object pointer as the last argument
	if( param_post )
	{
#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
		paramBuffer[argIndex] = (asPWORD)obj;
#else
		paramBuffer[argIndex] = (asPWORD)(param_post > 1 ? secondObject : obj);
#endif
		argsType[argIndex] = x64INTARG;
		argIndex++;
	}

	totalArgumentCount = argIndex;

	/*
	 * Q: WTF is going on here !?
	 *
	 * A: The idea is to pre-arange the parameters so that X64_CallFunction() can do
	 * it's little magic which must work regardless of how the compiler decides to
	 * allocate registers. Basically:
	 * - the first MAX_CALL_INT_REGISTERS entries in tempBuff will
	 *   contain the values/types of the x64INTARG parameters - that is the ones who
	 *   go into the registers. If the function has less then MAX_CALL_INT_REGISTERS
	 *   integer parameters then the last entries will be set to 0
	 * - the next MAX_CALL_SSE_REGISTERS entries will contain the float/double arguments
	 *   that go into the floating point registers. If the function has less than
	 *   MAX_CALL_SSE_REGISTERS floating point parameters then the last entries will
	 *   be set to 0
	 * - index MAX_CALL_INT_REGISTERS + MAX_CALL_SSE_REGISTERS marks the start of the
	 *   parameters which will get passed on the stack. These are added to the array
	 *   in reverse order so that X64_CallFunction() can simply push them to the stack
	 *   without the need to perform further tests
	 */
	asQWORD tempBuff[X64_CALLSTACK_SIZE] = { 0 };
	asBYTE  argsSet[X64_CALLSTACK_SIZE]  = { 0 };
	int     used_int_regs   = 0;
	int     used_sse_regs   = 0;
	int     used_stack_args = 0;
	int     idx             = 0;
	for ( n = 0; ( n < totalArgumentCount ) && ( used_int_regs < MAX_CALL_INT_REGISTERS ); n++ ) 
	{
		if ( argsType[n] == x64INTARG ) 
		{
			argsSet[n] = 1;
			tempBuff[idx++] = paramBuffer[n];
			used_int_regs++;
		}
	}
	idx = MAX_CALL_INT_REGISTERS;
	for ( n = 0; ( n < totalArgumentCount ) && ( used_sse_regs < MAX_CALL_SSE_REGISTERS ); n++ ) 
	{
		if ( argsType[n] == x64FLOATARG ) 
		{
			argsSet[n] = 1;
			tempBuff[idx++] = paramBuffer[n];
			used_sse_regs++;
		}
	}
	idx = MAX_CALL_INT_REGISTERS + MAX_CALL_SSE_REGISTERS;
	for ( n = totalArgumentCount - 1; n >= 0; n-- ) 
	{
		if ( !argsSet[n] ) 
		{
			tempBuff[idx++] = paramBuffer[n];
			used_stack_args++;
		}
	}

	retQW = X64_CallFunction( tempBuff, used_stack_args, func, retQW2, sysFunc->hostReturnFloat );

	return retQW;
}

END_AS_NAMESPACE

#endif // AS_X64_GCC
#endif // AS_MAX_PORTABILITY

