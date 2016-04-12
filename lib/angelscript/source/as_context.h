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
// as_context.h
//
// This class handles the execution of the byte code
//


#ifndef AS_CONTEXT_H
#define AS_CONTEXT_H

#include "as_config.h"
#include "as_atomic.h"
#include "as_array.h"
#include "as_string.h"
#include "as_objecttype.h"
#include "as_callfunc.h"

BEGIN_AS_NAMESPACE

class asCScriptFunction;
class asCScriptEngine;

class asCContext : public asIScriptContext
{
public:
	// Memory management
	int  AddRef() const;
	int  Release() const;

	// Miscellaneous
	asIScriptEngine *GetEngine() const;

	// Execution
	int             Prepare(asIScriptFunction *func);
	int             Unprepare();
	int             Execute();
	int             Abort();
	int             Suspend();
	asEContextState GetState() const;
	int             PushState();
	int             PopState();
	bool            IsNested(asUINT *nestCount = 0) const;

	// Object pointer for calling class methods
	int SetObject(void *obj);

	// Arguments
	int   SetArgByte(asUINT arg, asBYTE value);
	int   SetArgWord(asUINT arg, asWORD value);
	int   SetArgDWord(asUINT arg, asDWORD value);
	int   SetArgQWord(asUINT arg, asQWORD value);
	int   SetArgFloat(asUINT arg, float value);
	int   SetArgDouble(asUINT arg, double value);
	int   SetArgAddress(asUINT arg, void *addr);
	int   SetArgObject(asUINT arg, void *obj);
	int   SetArgVarType(asUINT arg, void *ptr, int typeId);
	void *GetAddressOfArg(asUINT arg);

	// Return value
	asBYTE  GetReturnByte();
	asWORD  GetReturnWord();
	asDWORD GetReturnDWord();
	asQWORD GetReturnQWord();
	float   GetReturnFloat();
	double  GetReturnDouble();
	void   *GetReturnAddress();
	void   *GetReturnObject();
	void   *GetAddressOfReturnValue();

	// Exception handling
	int                SetException(const char *descr);
	int                GetExceptionLineNumber(int *column, const char **sectionName);
	asIScriptFunction *GetExceptionFunction();
	const char *       GetExceptionString();
	int                SetExceptionCallback(asSFuncPtr callback, void *obj, int callConv);
	void               ClearExceptionCallback();

	// Debugging
	int                SetLineCallback(asSFuncPtr callback, void *obj, int callConv);
	void               ClearLineCallback();
	asUINT             GetCallstackSize() const;
	asIScriptFunction *GetFunction(asUINT stackLevel);
	int                GetLineNumber(asUINT stackLevel, int *column, const char **sectionName);
	int                GetVarCount(asUINT stackLevel);
	const char        *GetVarName(asUINT varIndex, asUINT stackLevel);
	const char        *GetVarDeclaration(asUINT varIndex, asUINT stackLevel, bool includeNamespace);
	int                GetVarTypeId(asUINT varIndex, asUINT stackLevel);
	void              *GetAddressOfVar(asUINT varIndex, asUINT stackLevel);
	bool               IsVarInScope(asUINT varIndex, asUINT stackLevel);
	int                GetThisTypeId(asUINT stackLevel);
    void              *GetThisPointer(asUINT stackLevel);
	asIScriptFunction *GetSystemFunction();

	// User data
	void *SetUserData(void *data, asPWORD type);
	void *GetUserData(asPWORD type) const;

public:
	// Internal public functions
	asCContext(asCScriptEngine *engine, bool holdRef);
	virtual ~asCContext();

//protected:
	friend class asCScriptEngine;

	void CallLineCallback();
	void CallExceptionCallback();

	int  CallGeneric(asCScriptFunction *func);

	void DetachEngine();

	void ExecuteNext();
	void CleanStack();
	void CleanStackFrame();
	void CleanArgsOnStack();
	void CleanReturnObject();
	void DetermineLiveObjects(asCArray<int> &liveObjects, asUINT stackLevel);

	void PushCallState();
	void PopCallState();
	void CallScriptFunction(asCScriptFunction *func);
	void CallInterfaceMethod(asCScriptFunction *func);
	void PrepareScriptFunction();

	bool ReserveStackSpace(asUINT size);

	void SetInternalException(const char *descr);

	// Must be protected for multiple accesses
	mutable asCAtomic m_refCount;

	bool             m_holdEngineRef;
	asCScriptEngine *m_engine;

	asEContextState m_status;
	bool            m_doSuspend;
	bool            m_doAbort;
	bool            m_externalSuspendRequest;

	asCScriptFunction *m_currentFunction;
	asCScriptFunction *m_callingSystemFunction;

	// The call stack holds program pointer, stack pointer, etc for caller functions
	asCArray<size_t>    m_callStack;

	// Dynamically growing local stack
	asCArray<asDWORD *> m_stackBlocks;
	asUINT              m_stackBlockSize;
	asUINT              m_stackIndex;
	asDWORD            *m_originalStackPointer;

	// Exception handling
	bool      m_isStackMemoryNotAllocated;
	bool      m_needToCleanupArgs;
	bool      m_inExceptionHandler;
	asCString m_exceptionString;
	int       m_exceptionFunction;
	int       m_exceptionSectionIdx;
	int       m_exceptionLine;
	int       m_exceptionColumn;

	// The last prepared function, and some cached values related to it
	asCScriptFunction *m_initialFunction;
	int                m_returnValueSize;
	int                m_argumentsSize;

	// callbacks
	bool                       m_lineCallback;
	asSSystemFunctionInterface m_lineCallbackFunc;
	void *                     m_lineCallbackObj;

	bool                       m_exceptionCallback;
	asSSystemFunctionInterface m_exceptionCallbackFunc;
	void *                     m_exceptionCallbackObj;

	asCArray<asPWORD> m_userData;

	// Registers available to JIT compiler functions
	asSVMRegisters m_regs;
};

// TODO: Move these to as_utils.h
int     as_powi(int base, int exponent, bool& isOverflow);
asDWORD as_powu(asDWORD base, asDWORD exponent, bool& isOverflow);
asINT64 as_powi64(asINT64 base, asINT64 exponent, bool& isOverflow);
asQWORD as_powu64(asQWORD base, asQWORD exponent, bool& isOverflow);

// Optional template version of powi if overflow detection is not used.
#if 0
template <class T>
T as_powi(T base, T exponent)
{
	// Test for sign bit (huge number is OK)
	if( exponent & (T(1)<<(sizeof(T)*8-1)) )
		return 0;
	else
	{
		int result = 1;
		while( exponent )
		{
			if( exponent & 1 )
				result *= base;
			exponent >>= 1;
			base *= base;
		}
		return result;
	}
}
#endif

END_AS_NAMESPACE

#endif
