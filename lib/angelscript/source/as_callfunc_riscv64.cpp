/*
   AngelCode Scripting Library
   Copyright (c) 2024 Andreas Jonsson

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
// as_callfunc_riscv64.cpp
//
// These functions handle the actual calling of system functions  
// on the 64bit RISC-V call convention used for Linux
//
// ref: https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf
//

#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_RISCV64

#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_tokendef.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

// retfloat == 0: the called function doesn't return a float value
// retfloat == 1: the called function returns a float/double value
// argValues is an array with all the values, the first 8 values will go to a0-a7 registers, the next 8 values will go to fa0-fa7 registers, and the remaining goes to the stack
// numRegularValues holds the number of regular values to put in a0-a7 registers
// numFloatValues hold the number of float values to put in fa0-fa7 registers
// numStackValues hold the number of values to push on the stack
struct asDBLQWORD { asQWORD qw1, qw2; };
extern "C" asDBLQWORD CallRiscVFunc(asFUNCTION_t func, int retfloat, asQWORD *argValues, int numRegularValues, int numFloatValues, int numStackValues);

// a0-a7 used for non-float values
// fa0-fa7 used for float values
// if more than 8 float values and there is space left in regular registers then those are used
// rest of the values are pushed on the stack
const asUINT maxRegularRegisters = 8;
const asUINT maxFloatRegisters = 8;
const asUINT maxValuesOnStack = 48 - maxRegularRegisters - maxFloatRegisters;

bool PushToFloatRegs(asQWORD val, asQWORD *argValues, asUINT &numFloatRegistersUsed, asUINT &numRegularRegistersUsed, asUINT &numStackValuesUsed)
{
	asQWORD* stackValues = argValues + maxRegularRegisters + maxFloatRegisters;

	if (numFloatRegistersUsed < maxFloatRegisters)
	{
		argValues[maxRegularRegisters + numFloatRegistersUsed] = val;
		numFloatRegistersUsed++;
	}
	else if (numRegularRegistersUsed < maxRegularRegisters)
	{
		argValues[numRegularRegistersUsed] = val;
		numRegularRegistersUsed++;
	}
	else if (numStackValuesUsed < maxValuesOnStack)
	{
		stackValues[numStackValuesUsed] = val;
		numStackValuesUsed++;
	}
	else
	{
		// Oops, we ran out of space in the argValues array!
		// TODO: This should be validated as the function is registered
		asASSERT(false);
		return false;
	}

	return true;
}

bool PushToRegularRegs(asQWORD val, asQWORD* argValues, asUINT& numRegularRegistersUsed, asUINT& numStackValuesUsed)
{
	asQWORD* stackValues = argValues + maxRegularRegisters + maxFloatRegisters;

	if (numRegularRegistersUsed < maxRegularRegisters)
	{
		argValues[numRegularRegistersUsed] = val;
		numRegularRegistersUsed++;
	}
	else if (numStackValuesUsed < maxValuesOnStack)
	{
		stackValues[numStackValuesUsed] = val;
		numStackValuesUsed++;
	}
	else
	{
		// Oops, we ran out of space in the argValues array!
		// TODO: This should be validated as the function is registered
		asASSERT(false);
		return false;
	}

	return true;
}

asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &retQW2, void *secondObj)
{
	asCScriptEngine *engine = context->m_engine;
	const asSSystemFunctionInterface *const sysFunc = descr->sysFuncIntf;
	const asCDataType &retType = descr->returnType;
	const asCTypeInfo *const retTypeInfo = retType.GetTypeInfo();
	asFUNCTION_t func = sysFunc->func;
	int callConv = sysFunc->callConv;

	// TODO: retrieve correct function pointer to call (e.g. from virtual function table, auxiliary pointer, etc)

	// Prepare the values that will be sent to the native function
	asQWORD argValues[maxRegularRegisters + maxFloatRegisters + maxValuesOnStack];
	asQWORD* stackValues = argValues + maxRegularRegisters + maxFloatRegisters;
	
	asUINT numRegularRegistersUsed = 0;
	asUINT numFloatRegistersUsed = 0;
	asUINT numStackValuesUsed = 0;

	// A function returning an object by value must give the
	// address of the memory to initialize as the first argument
	if (sysFunc->hostReturnInMemory)
	{
		// Set the return pointer as the first argument
		argValues[numRegularRegistersUsed++] = (asQWORD)retPointer;
	}

	// Determine the real function pointer in case of virtual method
	if (obj && (callConv == ICC_VIRTUAL_THISCALL || 
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM))
	{
		asFUNCTION_t* vftable = *((asFUNCTION_t**)obj);
		func = vftable[FuncPtrToUInt(func) / sizeof(void*)];
	}

	// Check if the object pointer must be added as the first argument
	if (callConv == ICC_CDECL_OBJFIRST || 
		callConv == ICC_CDECL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_THISCALL ||
		callConv == ICC_VIRTUAL_THISCALL ||
		callConv == ICC_THISCALL_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ||
		callConv == ICC_THISCALL_OBJLAST ||
		callConv == ICC_THISCALL_OBJLAST_RETURNINMEM || 
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST || 
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM)
	{
		PushToRegularRegs((asPWORD)obj, argValues, numRegularRegistersUsed, numStackValuesUsed);
	}
	else if (callConv == ICC_THISCALL_OBJFIRST ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST ||
		callConv == ICC_THISCALL_OBJFIRST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJFIRST_RETURNINMEM)
	{
		PushToRegularRegs((asPWORD)obj, argValues, numRegularRegistersUsed, numStackValuesUsed);
		PushToRegularRegs((asPWORD)secondObj, argValues, numRegularRegistersUsed, numStackValuesUsed);
	}

	asUINT argsPos = 0;
	for (asUINT n = 0; n < descr->parameterTypes.GetLength(); n++)
	{
		const asCDataType& parmType = descr->parameterTypes[n];
		const asUINT parmDWords = parmType.GetSizeOnStackDWords();

		if (parmType.IsReference() || parmType.IsObjectHandle() || parmType.IsIntegerType() || parmType.IsUnsignedType() || parmType.IsBooleanType() )
		{
			// pointers, integers, and booleans go to regular registers

			if (parmType.GetTokenType() == ttQuestion)
			{
				// Copy the reference and type id as two separate arguments
				PushToRegularRegs(*(asQWORD*)&args[argsPos], argValues, numRegularRegistersUsed, numStackValuesUsed);
				PushToRegularRegs((asQWORD)args[argsPos + AS_PTR_SIZE], argValues, numRegularRegistersUsed, numStackValuesUsed);
			}
			else 
			{
				if (parmDWords == 1)
					PushToRegularRegs((asQWORD)args[argsPos], argValues, numRegularRegistersUsed, numStackValuesUsed);
				else
					PushToRegularRegs(*(asQWORD*)&args[argsPos], argValues, numRegularRegistersUsed, numStackValuesUsed);
			}
		}
		else if (parmType.IsFloatType() || parmType.IsDoubleType())
		{
			// floats and doubles goes to the float registers
			// if there are more float/double args than registers, and there are still regular registers available then use those
			if (parmDWords == 1)
				PushToFloatRegs(0xFFFFFFFF00000000ull | (asQWORD)args[argsPos], argValues, numFloatRegistersUsed, numRegularRegistersUsed, numStackValuesUsed);
			else
				PushToFloatRegs(*(asQWORD*)&args[argsPos], argValues, numFloatRegistersUsed, numRegularRegistersUsed, numStackValuesUsed);
		}
		else if (parmType.IsObject())
		{
			if (parmType.GetTypeInfo()->flags & COMPLEX_MASK)
			{
				// complex object types are passed by address
				PushToRegularRegs(*(asQWORD*)&args[argsPos], argValues, numRegularRegistersUsed, numStackValuesUsed);
			}
			else if ((parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALLFLOATS) && !(parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_UNION) &&
				((parmType.GetSizeInMemoryDWords() <= 2 && !(parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALIGN8)) ||
				 (parmType.GetSizeInMemoryDWords() <= 4 && (parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALIGN8))) )
			{
				// simple structs with 1 or 2 floats/doubles are loaded into into float registers
				if (!(parmType.GetTypeInfo()->flags & asOBJ_APP_CLASS_ALIGN8))
				{
					// Unpack the floats
					asQWORD arg1 = 0xFFFFFFFF00000000ull | **(asDWORD**)&args[argsPos];
					asQWORD arg2 = 0xFFFFFFFF00000000ull | *((*(asDWORD**)&args[argsPos])+1);
					PushToFloatRegs(arg1, argValues, numFloatRegistersUsed, numRegularRegistersUsed, numStackValuesUsed);
					PushToFloatRegs(arg2, argValues, numFloatRegistersUsed, numRegularRegistersUsed, numStackValuesUsed);
				}
				else
				{
					// Unpack the doubles
					asQWORD arg1 = **(asQWORD**)&args[argsPos];
					asQWORD arg2 = *((*(asQWORD**)&args[argsPos]) + 1);
					PushToFloatRegs(arg1, argValues, numFloatRegistersUsed, numRegularRegistersUsed, numStackValuesUsed);
					PushToFloatRegs(arg2, argValues, numFloatRegistersUsed, numRegularRegistersUsed, numStackValuesUsed);
				}

				// Delete the original memory
				engine->CallFree(*(void**)&args[argsPos]);
			}
			else
			{
				// simple object types are passed in registers
				// TODO: what if part of the structure fits in registers but not the other part? would part of the object be pushed on the stack?
				// TODO: what of large objects? are they passed by value in registers/stack? Or by reference?
				const asUINT sizeInMemoryDWords = parmType.GetSizeInMemoryDWords();
				const asUINT parmQWords = (sizeInMemoryDWords >> 1) + (sizeInMemoryDWords & 1);

				if ((maxRegularRegisters - numRegularRegistersUsed) > parmQWords)
				{
					if (sizeInMemoryDWords == 1)
						argValues[numRegularRegistersUsed] = (asQWORD) * *(asDWORD**)&args[argsPos];
					else
						memcpy(&argValues[numRegularRegistersUsed], *(void**)&args[argsPos], sizeInMemoryDWords * 4);
					numRegularRegistersUsed += parmQWords;
				}
				else if ((maxValuesOnStack - numStackValuesUsed) > parmQWords)
				{
					if (sizeInMemoryDWords == 1)
						stackValues[numStackValuesUsed] = (asQWORD) * *(asDWORD**)&args[argsPos];
					else
						memcpy(&stackValues[numStackValuesUsed], *(void**)&args[argsPos], sizeInMemoryDWords * 4);
					numStackValuesUsed += parmQWords;
				}
				else
				{
					// Oops, we ran out of space in the argValues array!
					// TODO: This should be validated as the function is registered
					asASSERT(false);
				}

				// Delete the original memory
				engine->CallFree(*(void**)&args[argsPos]);
			}
		}

		argsPos += parmDWords;
	}

	// Check if the object pointer must be added as the last argument
	if (callConv == ICC_CDECL_OBJLAST || callConv == ICC_CDECL_OBJLAST_RETURNINMEM)
	{
		PushToRegularRegs((asPWORD)obj, argValues, numRegularRegistersUsed, numStackValuesUsed);
	}
	else if (callConv == ICC_THISCALL_OBJLAST || callConv == ICC_THISCALL_OBJLAST_RETURNINMEM ||
		callConv == ICC_VIRTUAL_THISCALL_OBJLAST || callConv == ICC_VIRTUAL_THISCALL_OBJLAST_RETURNINMEM)
	{
		PushToRegularRegs((asPWORD)secondObj, argValues, numRegularRegistersUsed, numStackValuesUsed);
	}

	int retfloat = sysFunc->hostReturnFloat ? 1 : 0;

	// Integer values are returned in a0 and a1, allowing simple structures with up to 128bits to be returned in registers
	asDBLQWORD ret = CallRiscVFunc(func, retfloat, argValues, numRegularRegistersUsed, numFloatRegistersUsed, numStackValuesUsed);
	retQW2 = ret.qw2;

	// Special case for returning a struct with two floats. C++ will return this in fa0:fa1. These needs to be compacted into a single qword
	if (retfloat && retTypeInfo && !(retTypeInfo->flags & asOBJ_APP_CLASS_ALIGN8) && retTypeInfo->flags & asOBJ_APP_CLASS_ALLFLOATS)
	{
		ret.qw1 &= 0xFFFFFFFF;
		ret.qw1 |= (retQW2 << 32);
	}

	return ret.qw1;
}

END_AS_NAMESPACE

#endif // AS_RISCV64
#endif // AS_MAX_PORTABILITY




