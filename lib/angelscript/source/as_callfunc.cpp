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
// as_callfunc.cpp
//
// These functions handle the actual calling of system functions
//



#include "as_config.h"
#include "as_callfunc.h"
#include "as_scriptengine.h"
#include "as_texts.h"
#include "as_context.h"

BEGIN_AS_NAMESPACE

// ref: Member Function Pointers and the Fastest Possible C++ Delegates
//      describes the structure of class method pointers for most compilers
//      http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible

// ref: The code comments for ItaniumCXXABI::EmitLoadOfMemberFunctionPointer in the LLVM compiler
//      describes the structure for class method pointers on Itanium and arm64 ABI
//      http://clang.llvm.org/doxygen/CodeGen_2ItaniumCXXABI_8cpp_source.html#l00937

int DetectCallingConvention(bool isMethod, const asSFuncPtr &ptr, int callConv, void *objForThiscall, asSSystemFunctionInterface *internal)
{
	memset(internal, 0, sizeof(asSSystemFunctionInterface));

	internal->func           = ptr.ptr.f.func;
	internal->objForThiscall = 0;

	// Was a compatible calling convention specified?
	if( internal->func )
	{
		if( ptr.flag == 1 && callConv != asCALL_GENERIC )
			return asWRONG_CALLING_CONV;
		else if( ptr.flag == 2 && (callConv == asCALL_GENERIC || callConv == asCALL_THISCALL || callConv == asCALL_THISCALL_ASGLOBAL || callConv == asCALL_THISCALL_OBJFIRST || callConv == asCALL_THISCALL_OBJLAST) )
			return asWRONG_CALLING_CONV;
		else if( ptr.flag == 3 && !(callConv == asCALL_THISCALL || callConv == asCALL_THISCALL_ASGLOBAL || callConv == asCALL_THISCALL_OBJFIRST || callConv == asCALL_THISCALL_OBJLAST) )
			return asWRONG_CALLING_CONV;
	}

	asDWORD base = callConv;
	if( !isMethod )
	{
		if( base == asCALL_CDECL )
			internal->callConv = ICC_CDECL;
		else if( base == asCALL_STDCALL )
			internal->callConv = ICC_STDCALL;
		else if( base == asCALL_THISCALL_ASGLOBAL )
		{
			if( objForThiscall == 0 )
				return asINVALID_ARG;
			internal->objForThiscall = objForThiscall;
			internal->callConv       = ICC_THISCALL;

			// This is really a thiscall, so it is necessary to check for virtual method pointers
			base = asCALL_THISCALL;
			isMethod = true;
		}
		else if( base == asCALL_GENERIC )
			internal->callConv = ICC_GENERIC_FUNC;
		else
			return asNOT_SUPPORTED;
	}
	
	if( isMethod )
	{
#ifndef AS_NO_CLASS_METHODS
		if( base == asCALL_THISCALL || base == asCALL_THISCALL_OBJFIRST || base == asCALL_THISCALL_OBJLAST )
		{
			internalCallConv thisCallConv;
			if( base == asCALL_THISCALL )
			{
				if( callConv != asCALL_THISCALL_ASGLOBAL && objForThiscall )
					return asINVALID_ARG;

				thisCallConv = ICC_THISCALL;
			}
			else
			{
#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
				return asNOT_SUPPORTED;
#else
				if( objForThiscall == 0 )
					return asINVALID_ARG;

				internal->objForThiscall = objForThiscall;
				if( base == asCALL_THISCALL_OBJFIRST )
					thisCallConv = ICC_THISCALL_OBJFIRST;
				else //if( base == asCALL_THISCALL_OBJLAST )
					thisCallConv = ICC_THISCALL_OBJLAST;
#endif
			}

			internal->callConv = thisCallConv;
#ifdef GNU_STYLE_VIRTUAL_METHOD
			if( (size_t(ptr.ptr.f.func) & 1) )
				internal->callConv = (internalCallConv)(thisCallConv + 2);
#endif
			internal->baseOffset = ( int )MULTI_BASE_OFFSET(ptr);
#if (defined(AS_ARM) || defined(AS_MIPS)) && (defined(__GNUC__) || defined(AS_PSVITA))
			// As the least significant bit in func is used to switch to THUMB mode
			// on ARM processors, the LSB in the __delta variable is used instead of
			// the one in __pfn on ARM processors.
			// MIPS also appear to use the base offset to indicate virtual method.
			if( (size_t(internal->baseOffset) & 1) )
				internal->callConv = (internalCallConv)(thisCallConv + 2);
#endif

#ifdef HAVE_VIRTUAL_BASE_OFFSET
			// We don't support virtual inheritance
			if( VIRTUAL_BASE_OFFSET(ptr) != 0 )
				return asNOT_SUPPORTED;
#endif
		}
		else
#endif
		if( base == asCALL_CDECL_OBJLAST )
			internal->callConv = ICC_CDECL_OBJLAST;
		else if( base == asCALL_CDECL_OBJFIRST )
			internal->callConv = ICC_CDECL_OBJFIRST;
		else if( base == asCALL_GENERIC )
			internal->callConv = ICC_GENERIC_METHOD;
		else
			return asNOT_SUPPORTED;
	}

	return 0;
}

// This function should prepare system functions so that it will be faster to call them
int PrepareSystemFunctionGeneric(asCScriptFunction *func, asSSystemFunctionInterface *internal, asCScriptEngine * /*engine*/)
{
	asASSERT(internal->callConv == ICC_GENERIC_METHOD || internal->callConv == ICC_GENERIC_FUNC);

	// Calculate the size needed for the parameters
	internal->paramSize = func->GetSpaceNeededForArguments();

	// Prepare the clean up instructions for the function arguments
	internal->cleanArgs.SetLength(0);
	int offset = 0;
	for( asUINT n = 0; n < func->parameterTypes.GetLength(); n++ )
	{
		asCDataType &dt = func->parameterTypes[n];

		if( dt.IsObject() && !dt.IsReference() )
		{
			asSTypeBehaviour *beh = &dt.GetObjectType()->beh;
			if( dt.GetObjectType()->flags & asOBJ_REF )
			{
				asASSERT( (dt.GetObjectType()->flags & asOBJ_NOCOUNT) || beh->release );
				if( beh->release )
				{
					asSSystemFunctionInterface::SClean clean;
					clean.op  = 0; // call release
					clean.ot  = dt.GetObjectType();
					clean.off = short(offset);
					internal->cleanArgs.PushLast(clean);
				}
			}
			else
			{
				asSSystemFunctionInterface::SClean clean;
				clean.op  = 1; // call free
				clean.ot  = dt.GetObjectType();
				clean.off = short(offset);

				// Call the destructor then free the memory
				if( beh->destruct )
					clean.op = 2; // call destruct, then free

				internal->cleanArgs.PushLast(clean);
			}
		}

		if( dt.IsObject() && !dt.IsObjectHandle() && !dt.IsReference() )
			offset += AS_PTR_SIZE;
		else
			offset += dt.GetSizeOnStackDWords();
	}

	return 0;
}

// This function should prepare system functions so that it will be faster to call them
int PrepareSystemFunction(asCScriptFunction *func, asSSystemFunctionInterface *internal, asCScriptEngine *engine)
{
#ifdef AS_MAX_PORTABILITY
	UNUSED_VAR(func);
	UNUSED_VAR(internal);
	UNUSED_VAR(engine);

	// This should never happen, as when AS_MAX_PORTABILITY is on, all functions 
	// are asCALL_GENERIC, which are prepared by PrepareSystemFunctionGeneric
	asASSERT(false);
#else
	// References are always returned as primitive data
	if( func->returnType.IsReference() || func->returnType.IsObjectHandle() )
	{
		internal->hostReturnInMemory = false;
		internal->hostReturnSize     = sizeof(void*)/4;
		internal->hostReturnFloat    = false;
	}
	// Registered types have special flags that determine how they are returned
	else if( func->returnType.IsObject() )
	{
		asDWORD objType = func->returnType.GetObjectType()->flags;
	
		// Only value types can be returned by value
		asASSERT( objType & asOBJ_VALUE );

		if( !(objType & (asOBJ_APP_CLASS | asOBJ_APP_PRIMITIVE | asOBJ_APP_FLOAT | asOBJ_APP_ARRAY)) )
		{
			// If the return is by value then we need to know the true type
			engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, func->GetDeclarationStr().AddressOf());

			asCString str;
			str.Format(TXT_CANNOT_RET_TYPE_s_BY_VAL, func->returnType.GetObjectType()->name.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			engine->ConfigError(asINVALID_CONFIGURATION, 0, 0, 0);
		}
		else if( objType & asOBJ_APP_ARRAY )
		{
			// Array types are always returned in memory
			internal->hostReturnInMemory = true;
			internal->hostReturnSize     = sizeof(void*)/4;
			internal->hostReturnFloat = false;
		}
		else if( objType & asOBJ_APP_CLASS )
		{
			internal->hostReturnFloat = false;
			if( objType & COMPLEX_RETURN_MASK )
			{
				internal->hostReturnInMemory = true;
				internal->hostReturnSize     = sizeof(void*)/4;
			}
			else
			{
#ifdef HAS_128_BIT_PRIMITIVES
				if( func->returnType.GetSizeInMemoryDWords() > 4 )
#else
				if( func->returnType.GetSizeInMemoryDWords() > 2 )
#endif
				{
					internal->hostReturnInMemory = true;
					internal->hostReturnSize = sizeof(void*)/4;
				}
				else
				{
					internal->hostReturnInMemory = false;
					internal->hostReturnSize     = func->returnType.GetSizeInMemoryDWords();
#ifdef SPLIT_OBJS_BY_MEMBER_TYPES
					if( func->returnType.GetObjectType()->flags & asOBJ_APP_CLASS_ALLFLOATS )
						internal->hostReturnFloat = true;
#endif
				}

#ifdef THISCALL_RETURN_SIMPLE_IN_MEMORY
				if((internal->callConv == ICC_THISCALL ||
#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
					internal->callConv == ICC_VIRTUAL_THISCALL) &&
#else
					internal->callConv == ICC_VIRTUAL_THISCALL ||
					internal->callConv == ICC_THISCALL_OBJFIRST ||
					internal->callConv == ICC_THISCALL_OBJLAST) &&
#endif
					func->returnType.GetSizeInMemoryDWords() >= THISCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE)
				{
					internal->hostReturnInMemory = true;
					internal->hostReturnSize     = sizeof(void*)/4;
				}
#endif
#ifdef CDECL_RETURN_SIMPLE_IN_MEMORY
				if((internal->callConv == ICC_CDECL         ||
					internal->callConv == ICC_CDECL_OBJLAST ||
					internal->callConv == ICC_CDECL_OBJFIRST) &&
					func->returnType.GetSizeInMemoryDWords() >= CDECL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE)
				{
					internal->hostReturnInMemory = true;
					internal->hostReturnSize     = sizeof(void*)/4;
				}
#endif
#ifdef STDCALL_RETURN_SIMPLE_IN_MEMORY
				if( internal->callConv == ICC_STDCALL &&
					func->returnType.GetSizeInMemoryDWords() >= STDCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE)
				{
					internal->hostReturnInMemory = true;
					internal->hostReturnSize     = sizeof(void*)/4;
				}
#endif
			}

#ifdef SPLIT_OBJS_BY_MEMBER_TYPES
			// It's not safe to return objects by value because different registers
			// will be used depending on the memory layout of the object.
			// Ref: http://www.x86-64.org/documentation/abi.pdf
			// Ref: http://www.agner.org/optimize/calling_conventions.pdf
			// If the application informs that the class should be treated as all integers, then we allow it
			if( !internal->hostReturnInMemory &&
			    !(func->returnType.GetObjectType()->flags & (asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_ALLFLOATS)) )	
			{
				engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, func->GetDeclarationStr().AddressOf());

				asCString str;
				str.Format(TXT_DONT_SUPPORT_RET_TYPE_s_BY_VAL, func->returnType.Format(func->nameSpace).AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				engine->ConfigError(asINVALID_CONFIGURATION, 0, 0, 0);
			}
#endif
		}
		else if( objType & asOBJ_APP_PRIMITIVE )
		{
			internal->hostReturnInMemory = false;
			internal->hostReturnSize     = func->returnType.GetSizeInMemoryDWords();
			internal->hostReturnFloat    = false;
		}
		else if( objType & asOBJ_APP_FLOAT )
		{
			internal->hostReturnInMemory = false;
			internal->hostReturnSize     = func->returnType.GetSizeInMemoryDWords();
			internal->hostReturnFloat    = true;
		}
	}
	// Primitive types can easily be determined
#ifdef HAS_128_BIT_PRIMITIVES
	else if( func->returnType.GetSizeInMemoryDWords() > 4 )
	{
		// Shouldn't be possible to get here
		asASSERT(false);
	}
	else if( func->returnType.GetSizeInMemoryDWords() == 4 )
	{
		internal->hostReturnInMemory = false;
		internal->hostReturnSize     = 4;
		internal->hostReturnFloat    = false;
	}
#else
	else if( func->returnType.GetSizeInMemoryDWords() > 2 )
	{
		// Shouldn't be possible to get here
		asASSERT(false);
	}
#endif
	else if( func->returnType.GetSizeInMemoryDWords() == 2 )
	{
		internal->hostReturnInMemory = false;
		internal->hostReturnSize     = 2;
		internal->hostReturnFloat    = func->returnType.IsEqualExceptConst(asCDataType::CreatePrimitive(ttDouble, true));
	}
	else if( func->returnType.GetSizeInMemoryDWords() == 1 )
	{
		internal->hostReturnInMemory = false;
		internal->hostReturnSize     = 1;
		internal->hostReturnFloat    = func->returnType.IsEqualExceptConst(asCDataType::CreatePrimitive(ttFloat, true));
	}
	else
	{
		internal->hostReturnInMemory = false;
		internal->hostReturnSize     = 0;
		internal->hostReturnFloat    = false;
	}

	// Calculate the size needed for the parameters
	internal->paramSize = func->GetSpaceNeededForArguments();

	// Verify if the function takes any objects by value
	asUINT n;
	internal->takesObjByVal = false;
	for( n = 0; n < func->parameterTypes.GetLength(); n++ )
	{
		if( func->parameterTypes[n].IsObject() && !func->parameterTypes[n].IsObjectHandle() && !func->parameterTypes[n].IsReference() )
		{
			internal->takesObjByVal = true;

			// Can't pass objects by value unless the application type is informed
			if( !(func->parameterTypes[n].GetObjectType()->flags & (asOBJ_APP_CLASS | asOBJ_APP_PRIMITIVE | asOBJ_APP_FLOAT | asOBJ_APP_ARRAY)) )
			{
				engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, func->GetDeclarationStr().AddressOf());
	
				asCString str;
				str.Format(TXT_CANNOT_PASS_TYPE_s_BY_VAL, func->parameterTypes[n].GetObjectType()->name.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				engine->ConfigError(asINVALID_CONFIGURATION, 0, 0, 0);
			}


#ifdef SPLIT_OBJS_BY_MEMBER_TYPES
			// It's not safe to pass objects by value because different registers
			// will be used depending on the memory layout of the object
			// Ref: http://www.x86-64.org/documentation/abi.pdf
			// Ref: http://www.agner.org/optimize/calling_conventions.pdf
			if( 
#ifdef COMPLEX_OBJS_PASSED_BY_REF
			    !(func->parameterTypes[n].GetObjectType()->flags & COMPLEX_MASK) &&	
#endif
#ifdef LARGE_OBJS_PASS_BY_REF
			    func->parameterTypes[n].GetSizeInMemoryDWords() < AS_LARGE_OBJ_MIN_SIZE &&
#endif
			    !(func->parameterTypes[n].GetObjectType()->flags & (asOBJ_APP_PRIMITIVE | asOBJ_APP_FLOAT | asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_ALLFLOATS)) )
			{
				engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, func->GetDeclarationStr().AddressOf());

				asCString str;
				str.Format(TXT_DONT_SUPPORT_TYPE_s_BY_VAL, func->parameterTypes[n].GetObjectType()->name.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				engine->ConfigError(asINVALID_CONFIGURATION, 0, 0, 0);
			}
#endif
			break;
		}
	}

	// Prepare the clean up instructions for the function arguments
	internal->cleanArgs.SetLength(0);
	int offset = 0;
	for( n = 0; n < func->parameterTypes.GetLength(); n++ )
	{
		asCDataType &dt = func->parameterTypes[n];

#if defined(COMPLEX_OBJS_PASSED_BY_REF) || defined(AS_LARGE_OBJS_PASSED_BY_REF)
		bool needFree = false;
#ifdef COMPLEX_OBJS_PASSED_BY_REF
		if( dt.GetObjectType() && dt.GetObjectType()->flags & COMPLEX_MASK ) needFree = true;
#endif
#ifdef AS_LARGE_OBJS_PASSED_BY_REF
		if( dt.GetSizeInMemoryDWords() >= AS_LARGE_OBJ_MIN_SIZE ) needFree = true;
#endif
		if( needFree &&
			dt.IsObject() &&
			!dt.IsObjectHandle() && 
			!dt.IsReference() )
		{
			asSSystemFunctionInterface::SClean clean;
			clean.op  = 1; // call free
			clean.ot  = dt.GetObjectType();
			clean.off = short(offset);

#ifndef AS_CALLEE_DESTROY_OBJ_BY_VAL
			// If the called function doesn't destroy objects passed by value we must do so here
			asSTypeBehaviour *beh = &dt.GetObjectType()->beh;
			if( beh->destruct )
				clean.op = 2; // call destruct, then free
#endif

			internal->cleanArgs.PushLast(clean);
		}
#endif

		if( n < internal->paramAutoHandles.GetLength() && internal->paramAutoHandles[n] )
		{
			asSSystemFunctionInterface::SClean clean;
			clean.op  = 0; // call release
			clean.ot  = dt.GetObjectType();
			clean.off = short(offset);
			internal->cleanArgs.PushLast(clean);
		}

		if( dt.IsObject() && !dt.IsObjectHandle() && !dt.IsReference() )
			offset += AS_PTR_SIZE;
		else
			offset += dt.GetSizeOnStackDWords();
	}
#endif // !defined(AS_MAX_PORTABILITY)
	return 0;
}

#ifdef AS_MAX_PORTABILITY

int CallSystemFunction(int id, asCContext *context)
{
	asCScriptEngine *engine = context->m_engine;
	asCScriptFunction *func = engine->scriptFunctions[id];
	asSSystemFunctionInterface *sysFunc = func->sysFuncIntf;
	int callConv = sysFunc->callConv;
	if( callConv == ICC_GENERIC_FUNC || callConv == ICC_GENERIC_METHOD )
		return context->CallGeneric(func);

	context->SetInternalException(TXT_INVALID_CALLING_CONVENTION);

	return 0;
}

#else

//
// CallSystemFunctionNative
//
// This function is implemented for each platform where the native calling conventions is supported.
// See the various as_callfunc_xxx.cpp files for their implementation. It is responsible for preparing
// the arguments for the function call, calling the function, and then retrieving the return value.
//
// Parameters:
//
// context    - This is the context that can be used to retrieve specific information from the engine
// descr      - This is the script function object that holds the information on how to call the function
// obj        - This is the object pointer, if the call is for a class method, otherwise it is null
// args       - This is the function arguments, which are packed as in AngelScript
// retPointer - This points to a the memory buffer where the return object is to be placed, if the function returns the value in memory rather than in registers
// retQW2     - This output parameter should be used if the function returns a value larger than 64bits in registers
// secondObj  - This is the object pointer that the proxy method should invoke its method on when the call convention is THISCALL_OBJFIRST/LAST
//
// Return value:
//
// The function should return the value that is returned in registers. 
asQWORD CallSystemFunctionNative(asCContext *context, asCScriptFunction *descr, void *obj, asDWORD *args, void *retPointer, asQWORD &retQW2, void *secondObj);


int CallSystemFunction(int id, asCContext *context)
{
	asCScriptEngine            *engine  = context->m_engine;
	asCScriptFunction          *descr   = engine->scriptFunctions[id];
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;

	int callConv = sysFunc->callConv;
	if( callConv == ICC_GENERIC_FUNC || callConv == ICC_GENERIC_METHOD )
		return context->CallGeneric(descr);

	asQWORD  retQW             = 0;
	asQWORD  retQW2            = 0;
	asDWORD *args              = context->m_regs.stackPointer;
	void    *retPointer        = 0;
	int      popSize           = sysFunc->paramSize;

#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
	void    *obj               = 0;
	void    *secondObj         = 0;

	if( callConv >= ICC_THISCALL )
	{
		if( sysFunc->objForThiscall )
		{
			// This class method is being called as if it is a global function
			obj = sysFunc->objForThiscall;
		}
		else
		{
			// The object pointer should be popped from the context stack
			popSize += AS_PTR_SIZE;

			// Check for null pointer
			obj = (void*)*(asPWORD*)(args);
			if( obj == 0 )
			{
				context->SetInternalException(TXT_NULL_POINTER_ACCESS);
				return 0;
			}

			// Add the base offset for multiple inheritance
#if (defined(__GNUC__) && (defined(AS_ARM) || defined(AS_MIPS))) || defined(AS_PSVITA)
			// On GNUC + ARM the lsb of the offset is used to indicate a virtual function
			// and the whole offset is thus shifted one bit left to keep the original
			// offset resolution
			// MIPS also work like ARM in this regard
			obj = (void*)(asPWORD(obj) + (sysFunc->baseOffset>>1));
#else
			obj = (void*)(asPWORD(obj) + sysFunc->baseOffset);
#endif

			// Skip the object pointer
			args += AS_PTR_SIZE;
		}
	}
#else
	// TODO: clean-up: CallSystemFunctionNative should have two arguments for object pointers
	//                 objForThiscall is the object pointer that should be used for the thiscall
	//                 objForArg is the object pointer that should be passed as argument when using OBJFIRST or OBJLAST

	// Used to save two object pointers with THISCALL_OBJLAST or THISCALL_OBJFIRST
	void *obj               = 0;
	void *secondObj         = 0;

	if( callConv >= ICC_THISCALL )
	{
		bool continueCheck = true;  // True if need check objectPointer or context stack for object
		int continueCheckIndex = 0; // Index into objectsPtrs to save the object if continueCheck

		if( callConv >= ICC_THISCALL_OBJLAST )
		{
			asASSERT( sysFunc->objForThiscall != 0 );
			// This class method is being called as object method (sysFunc->objForThiscall must be set).
			obj = sysFunc->objForThiscall;
			continueCheckIndex = 1;
		}
		else if( sysFunc->objForThiscall )
		{
			// This class method is being called as if it is a global function
			obj = sysFunc->objForThiscall;
			continueCheck = false;
		}

		if( continueCheck )
		{
			void *tempPtr = 0;

			// The object pointer should be popped from the context stack
			popSize += AS_PTR_SIZE;

			// Check for null pointer
			tempPtr = (void*)*(asPWORD*)(args);
			if( tempPtr == 0 )
			{
				context->SetInternalException(TXT_NULL_POINTER_ACCESS);
				return 0;
			}

			// Add the base offset for multiple inheritance
#if (defined(__GNUC__) && (defined(AS_ARM) || defined(AS_MIPS))) || defined(AS_PSVITA)
			// On GNUC + ARM the lsb of the offset is used to indicate a virtual function
			// and the whole offset is thus shifted one bit left to keep the original
			// offset resolution
			// MIPS also work like ARM in this regard
			tempPtr = (void*)(asPWORD(tempPtr) + (sysFunc->baseOffset>>1));
#else
			tempPtr = (void*)(asPWORD(tempPtr) + sysFunc->baseOffset);
#endif

			// Skip the object pointer
			args += AS_PTR_SIZE;

			if( continueCheckIndex )
				secondObj = tempPtr;
			else
			{
				asASSERT( obj == 0 );
				obj = tempPtr;
			}
		}
	}
#endif // AS_NO_THISCALL_FUNCTOR_METHOD

	if( descr->DoesReturnOnStack() )
	{
		// Get the address of the location for the return value from the stack
		retPointer = (void*)*(asPWORD*)(args);
		popSize += AS_PTR_SIZE;
		args += AS_PTR_SIZE;

		// When returning the value on the location allocated by the called 
		// we shouldn't set the object type in the register
		context->m_regs.objectType = 0;
	}
	else
	{
		// Set the object type of the reference held in the register
		context->m_regs.objectType = descr->returnType.GetObjectType();
	}

	context->m_callingSystemFunction = descr;
	bool cppException = false;
#ifdef AS_NO_EXCEPTIONS
	retQW = CallSystemFunctionNative(context, descr, obj, args, sysFunc->hostReturnInMemory ? retPointer : 0, retQW2, secondObj);
#else
	// This try/catch block is to catch potential exception that may 
	// be thrown by the registered function. The implementation of the
	// CallSystemFunctionNative() must make sure not to have any manual
	// clean-up after the call to the real function, or that won't be 
	// executed in case of an exception.
	try
	{
		retQW = CallSystemFunctionNative(context, descr, obj, args, sysFunc->hostReturnInMemory ? retPointer : 0, retQW2, secondObj);
	}
	catch(...)
	{
		cppException = true;

		// Convert the exception to a script exception so the VM can 
		// properly report the error to the application and then clean up
		context->SetException(TXT_EXCEPTION_CAUGHT);
	}
#endif
	context->m_callingSystemFunction = 0;

	// Store the returned value in our stack
	if( descr->returnType.IsObject() && !descr->returnType.IsReference() )
	{
		if( descr->returnType.IsObjectHandle() )
		{
#if defined(AS_BIG_ENDIAN) && AS_PTR_SIZE == 1
			// Since we're treating the system function as if it is returning a QWORD we are
			// actually receiving the value in the high DWORD of retQW.
			retQW >>= 32;
#endif

			context->m_regs.objectRegister = (void*)(asPWORD)retQW;

			if( sysFunc->returnAutoHandle && context->m_regs.objectRegister )
			{
				asASSERT( !(descr->returnType.GetObjectType()->flags & asOBJ_NOCOUNT) );
				engine->CallObjectMethod(context->m_regs.objectRegister, descr->returnType.GetObjectType()->beh.addref);
			}
		}
		else
		{
			asASSERT( retPointer );

			if( !sysFunc->hostReturnInMemory )
			{
				// Copy the returned value to the pointer sent by the script engine
				if( sysFunc->hostReturnSize == 1 )
				{
#if defined(AS_BIG_ENDIAN) && AS_PTR_SIZE == 1
					// Since we're treating the system function as if it is returning a QWORD we are
					// actually receiving the value in the high DWORD of retQW.
					retQW >>= 32;
#endif

					*(asDWORD*)retPointer = (asDWORD)retQW;
				}
				else if( sysFunc->hostReturnSize == 2 )
					*(asQWORD*)retPointer = retQW;
				else if( sysFunc->hostReturnSize == 3 )
				{
					*(asQWORD*)retPointer         = retQW;
					*(((asDWORD*)retPointer) + 2) = (asDWORD)retQW2;
				}
				else // if( sysFunc->hostReturnSize == 4 )
				{
					*(asQWORD*)retPointer         = retQW;
					*(((asQWORD*)retPointer) + 1) = retQW2;
				}
			}

			if( context->m_status == asEXECUTION_EXCEPTION && !cppException )
			{
				// If the function raised a script exception it really shouldn't have 
				// initialized the object. However, as it is a soft exception there is 
				// no way for the application to not return a value, so instead we simply
				// destroy it here, to pretend it was never created.
				if( descr->returnType.GetObjectType()->beh.destruct )
					engine->CallObjectMethod(retPointer, descr->returnType.GetObjectType()->beh.destruct);
			}
		}
	}
	else
	{
		// Store value in value register
		if( sysFunc->hostReturnSize == 1 )
		{
#if defined(AS_BIG_ENDIAN)
			// Since we're treating the system function as if it is returning a QWORD we are
			// actually receiving the value in the high DWORD of retQW.
			retQW >>= 32;

			// Due to endian issues we need to handle return values that are
			// less than a DWORD (32 bits) in size specially
			int numBytes = descr->returnType.GetSizeInMemoryBytes();
			if( descr->returnType.IsReference() ) numBytes = 4;
			switch( numBytes )
			{
			case 1:
				{
					// 8 bits
					asBYTE *val = (asBYTE*)&context->m_regs.valueRegister;
					val[0] = (asBYTE)retQW;
					val[1] = 0;
					val[2] = 0;
					val[3] = 0;
					val[4] = 0;
					val[5] = 0;
					val[6] = 0;
					val[7] = 0;
				}
				break;
			case 2:
				{
					// 16 bits
					asWORD *val = (asWORD*)&context->m_regs.valueRegister;
					val[0] = (asWORD)retQW;
					val[1] = 0;
					val[2] = 0;
					val[3] = 0;
				}
				break;
			default:
				{
					// 32 bits
					asDWORD *val = (asDWORD*)&context->m_regs.valueRegister;
					val[0] = (asDWORD)retQW;
					val[1] = 0;
				}
				break;
			}
#else
			*(asDWORD*)&context->m_regs.valueRegister = (asDWORD)retQW;
#endif
		}
		else
			context->m_regs.valueRegister = retQW;
	}

	// Clean up arguments
	const asUINT cleanCount = sysFunc->cleanArgs.GetLength();
	if( cleanCount )
	{
		args = context->m_regs.stackPointer;

		// Skip the hidden argument for the return pointer
		// TODO: runtime optimize: This check and increment should have been done in PrepareSystemFunction
		if( descr->DoesReturnOnStack() )
			args += AS_PTR_SIZE;

		// Skip the object pointer on the stack
		// TODO: runtime optimize: This check and increment should have been done in PrepareSystemFunction
		if( callConv >= ICC_THISCALL && sysFunc->objForThiscall == 0 )
			args += AS_PTR_SIZE;

		asSSystemFunctionInterface::SClean *clean = sysFunc->cleanArgs.AddressOf();
		for( asUINT n = 0; n < cleanCount; n++, clean++ )
		{
			void **addr = (void**)&args[clean->off];
			if( clean->op == 0 )
			{
				if( *addr != 0 )
				{
					engine->CallObjectMethod(*addr, clean->ot->beh.release);
					*addr = 0;
				}
			}
			else 
			{
				asASSERT( clean->op == 1 || clean->op == 2 );
				asASSERT( *addr );

				if( clean->op == 2 )
					engine->CallObjectMethod(*addr, clean->ot->beh.destruct);
				
				engine->CallFree(*addr);
			}
		}
	}

	return popSize;
}

#endif // AS_MAX_PORTABILITY

END_AS_NAMESPACE

