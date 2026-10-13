/*
   AngelCode Scripting Library
   Copyright (c) 2020-2024 Andreas Jonsson

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
// as_callfunc_arm64.cpp
//
// These functions handle the actual calling of system functions on the arm64 platform
//
// Written by Max Waine in July 2020, based on as_callfunc_arm.cpp
//
// Adapted for Apple M1/M2 by Sam Tupy in Jan 2024
//
// ref for Apple ABI: https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms 
//


#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_ARM64

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

// ARM64 targets use has no software floating-point ABI, it's all hardware (or totally disabled)

#define HFA_RET_REGISTERS   4 // s0-s3/d0-d3
#define GP_ARG_REGISTERS    8 // x0-x7
#define FLOAT_ARG_REGISTERS 8 // v0-v7

BEGIN_AS_NAMESPACE

// x0-7:   Argument registers (pass params or return results. OK as volatile local variables)
// x8:     Indirect result register (e.g. address of large returned struct)
// x9-15:  Volatile local variable registers
// x16-17: Intra-procedure-call temporary registers
// x18:    Platform register (reserved for use of platform ABIs)
// x19-29: Non-volatile variable registers (must be saved and restored if modified)
// x29:    Frame pointer register
// x30:    Link register (where to return to)

extern "C" void GetHFAReturnDouble(asQWORD *out1, asQWORD *out2, asQWORD returnSize);
extern "C" void GetHFAReturnFloat(asQWORD *out1, asQWORD *out2, asQWORD returnSize);

extern "C" asQWORD CallARM64RetInMemory(
	const asQWORD *gpRegArgs,    asQWORD numGPRegArgs,
	const asQWORD *floatRegArgs, asQWORD numFloatRegArgs,
	const asQWORD *stackArgs,    asQWORD numStackArgs,
	void *retPointer,            asFUNCTION_t func
);
extern "C" double CallARM64Double(
	const asQWORD *gpRegArgs,    asQWORD numGPRegArgs,
	const asQWORD *floatRegArgs, asQWORD numFloatRegArgs,
	const asQWORD *stackArgs,    asQWORD numStackArgs,
	asFUNCTION_t func
);
extern "C" float CallARM64Float(
	const asQWORD *gpRegArgs,    asQWORD numGPRegArgs,
	const asQWORD *floatRegArgs, asQWORD numFloatRegArgs,
	const asQWORD *stackArgs,    asQWORD numStackArgs,
	asFUNCTION_t func
);
extern "C" asQWORD CallARM64(
	const asQWORD *gpRegArgs,    asQWORD numGPRegArgs,
	const asQWORD *floatRegArgs, asQWORD numFloatRegArgs,
	const asQWORD *stackArgs,    asQWORD numStackArgs,
	asFUNCTION_t func
);
extern "C" asQWORD CallARM64Ret128(
	const asQWORD *gpRegArgs,    asQWORD numGPRegArgs,
	const asQWORD *floatRegArgs, asQWORD numFloatRegArgs,
	const asQWORD *stackArgs,    asQWORD numStackArgs,
	asQWORD *higherQWORD,        asFUNCTION_t func
);

//
// If it's possible to fit in registers,
// there may not be enough float register space even if true is returned
//
static inline bool IsRegisterHFA(const asCDataType &type)
{
	const asCTypeInfo *const typeInfo = type.GetTypeInfo();

	if( typeInfo == 0 ||
		(typeInfo->flags & asOBJ_APP_CLASS_ALLFLOATS) == 0 ||
		type.IsObjectHandle() || type.IsReference() )
		return false;

	const bool doubles = (typeInfo->flags & asOBJ_APP_CLASS_ALIGN8) != 0;
	const int maxAllowedSize = doubles ? sizeof(double) * HFA_RET_REGISTERS : sizeof(float) * HFA_RET_REGISTERS;

	return type.GetSizeInMemoryBytes() <= maxAllowedSize;
}

//
// If it's possible to fit it in registers,
// if true is returned there is enough space to fit
//
static inline bool IsRegisterHFAParameter(const asCDataType &type, const asQWORD numFloatRegArgs)
{
	if( !IsRegisterHFA(type) )
		return false;

	const bool doubles = (type.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALIGN8) != 0;
	const int registersUsed = type.GetSizeInMemoryDWords() / (doubles ? sizeof(double) : sizeof(float));

	return numFloatRegArgs + registersUsed <= FLOAT_ARG_REGISTERS;
}

#if defined(AS_IPHONE) || defined(AS_MAC)
static inline void PadAppleStack(asQWORD* const argsArray, asQWORD& numArgs, asQWORD& stackDispositionBytes, asUINT argSize)
{
	if(stackDispositionBytes == 0) return;
	asUINT padBytes = argSize - (stackDispositionBytes % argSize);
	if(padBytes > 0 && padBytes != argSize)
	{
		memset( ((asBYTE*)&argsArray[numArgs]) + stackDispositionBytes, 0, padBytes);
		stackDispositionBytes += padBytes;
	}
	numArgs += stackDispositionBytes / sizeof(asQWORD);
	if(stackDispositionBytes >= sizeof(asQWORD))
		stackDispositionBytes -= sizeof(asQWORD) * (stackDispositionBytes / sizeof(asQWORD) );
}
#endif

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &retQW2, void *secondObject)
{
	asCScriptEngine *engine = context->m_engine;
	const asSSystemFunctionInterface *const sysFunc = descr->sysFuncIntf;
	const asCDataType &retType = descr->returnType;
	const asCTypeInfo *const retTypeInfo = retType.GetTypeInfo();
	asFUNCTION_t func = sysFunc->func;
	int callConv = sysFunc->callConv;
	asQWORD       retQW     = 0;

	asQWORD       gpRegArgs[GP_ARG_REGISTERS];
	asQWORD       floatRegArgs[FLOAT_ARG_REGISTERS];
	asQWORD       stackArgs[64]; // It's how many x64 users can have
	asQWORD       numGPRegArgs    = 0;
	asQWORD       numFloatRegArgs = 0;
	asQWORD       numStackArgs    = 0;
#if defined(AS_IPHONE) || defined(AS_MAC)
	asQWORD stackDispositionBytes = 0; // Apple silikin aligns arguments on the stack by the size of their type instead of by 8 bytes like standard arm.
#endif
	asFUNCTION_t *vftable;

	// Optimization to avoid check 12 values (all ICC_ that contains THISCALL)
	if( (callConv >= ICC_THISCALL && callConv <= ICC_VIRTUAL_THISCALL_RETURNINMEM) ||
		(callConv >= ICC_THISCALL_OBJLAST && callConv <= ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM) )
	{
		// Add the object pointer as the first parameter
		gpRegArgs[numGPRegArgs++] = (asQWORD)obj;
	}

	if( callConv == ICC_CDECL_OBJFIRST || callConv == ICC_CDECL_OBJFIRST_RETURNINMEM )
	{
		// Add the object pointer as the first parameter
		gpRegArgs[numGPRegArgs++] = (asQWORD)obj;
	}
	else if( callConv == ICC_THISCALL_OBJFIRST || callConv == ICC_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST || callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM )
	{
		// Add the object pointer as the first parameter
		gpRegArgs[numGPRegArgs++] = (asQWORD)secondObject;
	}

	if( callConv == ICC_VIRTUAL_THISCALL || callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM || callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
	callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM || callConv == ICC_VIRTUAL_THISCALL_OBJLAST || callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Get virtual function table from the object pointer
		vftable = *(asFUNCTION_t**)obj;
		func = vftable[FuncPtrToUInt(func)/sizeof(void*)];
	}

	asUINT argsPos = 0;
	for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
	{
		const asCDataType &parmType = descr->parameterTypes[n];
		const asCTypeInfo *const parmTypeInfo = parmType.GetTypeInfo();

		if( parmType.IsObject() && !parmType.IsObjectHandle() && !parmType.IsReference() )
		{
			const asUINT parmDWords = parmType.GetSizeInMemoryDWords();
			const asUINT parmQWords = (parmDWords >> 1) + (parmDWords & 1);

			const bool passedAsPointer = parmQWords <= 2;
			const bool fitsInRegisters = passedAsPointer ? (numGPRegArgs < GP_ARG_REGISTERS) : (numGPRegArgs + parmQWords <= GP_ARG_REGISTERS);
			asQWORD *const argsArray = fitsInRegisters ? gpRegArgs : stackArgs;
			asQWORD &numArgs = fitsInRegisters ? numGPRegArgs : numStackArgs;

			if( (parmTypeInfo->flags & COMPLEX_MASK) )
			{
#if defined(AS_IPHONE) || defined(AS_MAC)
				if(!fitsInRegisters) PadAppleStack(argsArray, numArgs, stackDispositionBytes, sizeof(asQWORD));
#endif
				argsArray[numArgs++] = *(asQWORD*)&args[argsPos];
				argsPos += AS_PTR_SIZE;
			}
			else if( IsRegisterHFAParameter(parmType, numFloatRegArgs) )
			{
				if( (parmTypeInfo->flags & asOBJ_APP_CLASS_ALIGN8) != 0 )
				{
					const asQWORD *const contents = *(asQWORD**)&args[argsPos];
					for( asUINT i = 0; i < parmQWords; i++ )
						floatRegArgs[numFloatRegArgs++] = *(asQWORD*)&contents[i];
				}
				else
				{
					const asDWORD *const contents = *(asDWORD**)&args[argsPos];
					for( asUINT i = 0; i < parmDWords; i++ )
						floatRegArgs[numFloatRegArgs++] = *(asDWORD*)&contents[i];
				}
				engine->CallFree(*(char**)(args+argsPos));
				argsPos += AS_PTR_SIZE;
			}
			else
			{
				// Copy the object's memory to the buffer
				asUINT parmBytes = parmType.GetSizeInMemoryBytes();
#if defined(AS_IPHONE) || defined(AS_MAC)
				if(!fitsInRegisters)
				{
					PadAppleStack(argsArray, numArgs, stackDispositionBytes, parmBytes);
					memcpy( ((asBYTE*)&argsArray[numArgs]) + stackDispositionBytes, *(void**)(args+argsPos), parmBytes);
					stackDispositionBytes += parmBytes;
					// numArgs will be fixed on next call to PadAppleStack.
				}
				else
#endif
				{
					memcpy(&argsArray[numArgs], *(void**)(args+argsPos), parmBytes);
					numArgs += parmQWords;
				}

				// Delete the original memory
				engine->CallFree(*(char**)(args+argsPos));
				argsPos += AS_PTR_SIZE;
			}
		}
		else if( parmType.IsFloatType() && !parmType.IsReference() )
		{
			if( numFloatRegArgs >= FLOAT_ARG_REGISTERS )
#if defined(AS_IPHONE) || defined(AS_MAC)
			{
				PadAppleStack(stackArgs, numStackArgs, stackDispositionBytes, sizeof(float));
				memcpy( ((asBYTE*)&stackArgs[numStackArgs]) + stackDispositionBytes, &args[argsPos], sizeof(float));
				stackDispositionBytes += sizeof(float);
			}
#else
				stackArgs[numStackArgs++] = args[argsPos];
#endif
			else
				floatRegArgs[numFloatRegArgs++] = args[argsPos];
			argsPos++;
		}
		else if( parmType.IsDoubleType() && !parmType.IsReference() )
		{
			if( numFloatRegArgs >= FLOAT_ARG_REGISTERS )
#if defined(AS_IPHONE) || defined(AS_MAC)
			{
				PadAppleStack(stackArgs, numStackArgs, stackDispositionBytes, sizeof(asQWORD));
				memcpy( ((asBYTE*)&stackArgs[numStackArgs]) + stackDispositionBytes, (asQWORD*)&args[argsPos], sizeof(asQWORD));
				stackDispositionBytes += sizeof(asQWORD);
			}
#else
				stackArgs[numStackArgs++] = *(asQWORD*)&args[argsPos];
#endif
			else
				floatRegArgs[numFloatRegArgs++] = *(asQWORD*)&args[argsPos];
			argsPos += 2;
		}
		else if (parmType.GetTokenType() == ttQuestion)
		{
			// Copy the reference and the type id as separate arguments
			// 
			// First the reference
			bool fitsInRegisters = numGPRegArgs + 1 <= GP_ARG_REGISTERS;
			asQWORD* argsArray = fitsInRegisters ? gpRegArgs : stackArgs;
			asQWORD* numArgs = fitsInRegisters ? &numGPRegArgs : &numStackArgs;
#if defined(AS_IPHONE) || defined(AS_MAC)
			if (!fitsInRegisters)
			{
				asUINT parmBytes = AS_PTR_SIZE * 4;
				PadAppleStack(argsArray, *numArgs, stackDispositionBytes, parmBytes);
				memcpy(((asBYTE*)&argsArray[*numArgs]) + stackDispositionBytes, (void*)(args + argsPos), parmBytes);
				stackDispositionBytes += parmBytes;
			}
			else
#endif
			{
				memcpy(&argsArray[*numArgs], (void*)(args + argsPos), AS_PTR_SIZE * 4);
				(*numArgs) += 1;
			}
			argsPos += AS_PTR_SIZE;

			// Then the type id
			fitsInRegisters = numGPRegArgs + 1 <= GP_ARG_REGISTERS;
			argsArray = fitsInRegisters ? gpRegArgs : stackArgs;
			numArgs = fitsInRegisters ? &numGPRegArgs : &numStackArgs;
#if defined(AS_IPHONE) || defined(AS_MAC)
			if (!fitsInRegisters)
			{
				asUINT parmBytes = 4;
				PadAppleStack(argsArray, *numArgs, stackDispositionBytes, parmBytes);
				memcpy(((asBYTE*)&argsArray[*numArgs]) + stackDispositionBytes, (void*)(args + argsPos), parmBytes);
				stackDispositionBytes += parmBytes;
			}
			else
#endif
			{
				memcpy(&argsArray[*numArgs], (void*)(args + argsPos), 4);
				(*numArgs) += 1;
			}
			argsPos += 1;
		}
		else
		{
			// Copy the value directly
			const asUINT parmDWords = parmType.GetSizeOnStackDWords();
			const asUINT parmQWords = (parmDWords >> 1) + (parmDWords & 1);

			const bool fitsInRegisters = numGPRegArgs + parmQWords <= GP_ARG_REGISTERS;
			asQWORD *const argsArray = fitsInRegisters ? gpRegArgs : stackArgs;
			asQWORD &numArgs = fitsInRegisters ? numGPRegArgs : numStackArgs;
#if defined(AS_IPHONE) || defined(AS_MAC)
			if(!fitsInRegisters)
			{
				asUINT parmBytes = parmType.GetSizeInMemoryBytes();
				PadAppleStack(argsArray, numArgs, stackDispositionBytes, parmBytes);
				memcpy( ((asBYTE*)&argsArray[numArgs]) + stackDispositionBytes, (void*)(args+argsPos), parmBytes);
				stackDispositionBytes += parmBytes;
			}
			else
#endif
			{
				memcpy(&argsArray[numArgs], (void*)(args+argsPos), parmDWords * 4);
				numArgs += parmQWords;
			}

			argsPos += parmDWords;
		}
	}
#if defined(AS_IPHONE) || defined(AS_MAC)
	PadAppleStack(stackArgs, numStackArgs, stackDispositionBytes, sizeof(asQWORD));
#endif
	if( callConv == ICC_CDECL_OBJLAST || callConv == ICC_CDECL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the last parameter
		if( numGPRegArgs < GP_ARG_REGISTERS )
			gpRegArgs[numGPRegArgs++] = (asQWORD)obj;
		else
			stackArgs[numStackArgs++] = (asQWORD)obj;
	}
	else if( callConv == ICC_THISCALL_OBJLAST || callConv == ICC_THISCALL_OBJLAST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST || callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM )
	{
		// Add the object pointer as the last parameter
		if( numGPRegArgs < GP_ARG_REGISTERS )
			gpRegArgs[numGPRegArgs++] = (asQWORD)secondObject;
		else
			stackArgs[numStackArgs++] = (asQWORD)secondObject;
	}

	if( IsRegisterHFA(retType) && !(retTypeInfo->flags & COMPLEX_MASK) )
	{
		// This is to deal with HFAs (Homogeneous Floating-point Aggregates):
		// ARM64 will place all-float composite types (of equal precision)
		// with <= 4 members in the float return registers

		const int structSize = retType.GetSizeInMemoryBytes();

		CallARM64(gpRegArgs, numGPRegArgs, floatRegArgs, numFloatRegArgs, stackArgs, numStackArgs, func);
		if( (retTypeInfo->flags & asOBJ_APP_CLASS_ALIGN8) != 0 )
		{
			if( structSize <= sizeof(double) * 2 )
				GetHFAReturnDouble(&retQW, &retQW2, structSize);
			else
				GetHFAReturnDouble((asQWORD*)retPointer, ((asQWORD*)retPointer) + 1, structSize);
		}
		else
			GetHFAReturnFloat(&retQW, &retQW2, structSize);
	}
	else if( sysFunc->hostReturnFloat )
	{
		if( sysFunc->hostReturnSize == 1 )
			*(float*)&retQW = CallARM64Float(gpRegArgs, numGPRegArgs, floatRegArgs, numFloatRegArgs, stackArgs, numStackArgs, func);
		else
			*(double*)&retQW = CallARM64Double(gpRegArgs, numGPRegArgs, floatRegArgs, numFloatRegArgs, stackArgs, numStackArgs, func);
	}
	else if( sysFunc->hostReturnInMemory )
		retQW = CallARM64RetInMemory(gpRegArgs, numGPRegArgs, floatRegArgs, numFloatRegArgs, stackArgs, numStackArgs, retPointer, func);
	else
	{
		if( retType.GetSizeInMemoryBytes() > sizeof(asQWORD) )
			retQW = CallARM64Ret128(gpRegArgs, numGPRegArgs, floatRegArgs, numFloatRegArgs, stackArgs, numStackArgs, &retQW2, func);
		else
			retQW = CallARM64(gpRegArgs, numGPRegArgs, floatRegArgs, numFloatRegArgs, stackArgs, numStackArgs, func);
	}

	return retQW;
}

END_AS_NAMESPACE

#endif // AS_ARM64
#endif // AS_MAX_PORTABILITY




