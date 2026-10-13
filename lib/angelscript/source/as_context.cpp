/*
   AngelCode Scripting Library
   Copyright (c) 2003-2025 Andreas Jonsson

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
// as_context.cpp
//
// This class handles the execution of the byte code
//

#include <math.h> // fmodf() pow()

#include "as_config.h"
#include "as_context.h"
#include "as_scriptengine.h"
#include "as_tokendef.h"
#include "as_texts.h"
#include "as_callfunc.h"
#include "as_generic.h"
#include "as_debug.h" // mkdir()
#include "as_bytecode.h"
#include "as_scriptobject.h"

#ifdef _MSC_VER
#pragma warning(disable:4702) // unreachable code

// Apparently a bug in MSVC (or perhaps Windows SDK) caused use HUGE_VALF to issue a warning
// ref: https://developercommunity.visualstudio.com/t/C4756-related-issues-in-VS-2022/10697767
#pragma warning(disable:4756)
#endif

//make compiler shut up
#if __cplusplus >= 201703L
#define FALLTHROUGH [[fallthrough]];
#else
#define FALLTHROUGH
#endif

BEGIN_AS_NAMESPACE

#if defined(AS_DEBUG)

class asCDebugStats
{
public:
	asCDebugStats()
	{
		memset(instrCount, 0, sizeof(instrCount));
		memset(instrCount2, 0, sizeof(instrCount2));
		lastBC = 255;
	}

	~asCDebugStats()
	{
		// This code writes out some statistics for the VM.
		// It's useful for determining what needs to be optimized.
		if (!outputDebug) return;

#ifndef __MINGW32__
		// _mkdir is broken on mingw
		_mkdir("AS_DEBUG");
#endif
		#if _MSC_VER >= 1500 && !defined(AS_MARMALADE)
			FILE *f;
			fopen_s(&f, "AS_DEBUG/stats.txt", "wt");
		#else
			FILE *f = fopen("AS_DEBUG/stats.txt", "wt");
		#endif
		if( f )
		{
			// Output instruction statistics
			fprintf(f, "\nTotal count\n");
			int n;
			for( n = 0; n < asBC_MAXBYTECODE; n++ )
			{
				if( asBCInfo[n].name && instrCount[n] > 0 )
					fprintf(f, "%-10.10s : %.0f\n", asBCInfo[n].name, instrCount[n]);
			}

			fprintf(f, "\nNever executed\n");
			for( n = 0; n < asBC_MAXBYTECODE; n++ )
			{
				if( asBCInfo[n].name && instrCount[n] == 0 )
					fprintf(f, "%-10.10s\n", asBCInfo[n].name);
			}

			fprintf(f, "\nSequences\n");
			for( n = 0; n < 256; n++ )
			{
				if( asBCInfo[n].name )
				{
					for( int m = 0; m < 256; m++ )
					{
						if( instrCount2[n][m] )
							fprintf(f, "%-10.10s, %-10.10s : %.0f\n", asBCInfo[n].name, asBCInfo[m].name, instrCount2[n][m]);
					}
				}
			}
			fclose(f);
		}
	}

	void Instr(asBYTE bc, bool writeDebug)
	{
		++instrCount[bc];
		++instrCount2[lastBC][bc];
		lastBC = bc;
		outputDebug = writeDebug;
	}

	// Instruction statistics
	double instrCount[256];
	double instrCount2[256][256];
	int lastBC;
	bool outputDebug;
} stats;

#endif

// interface
AS_API asIScriptContext *asGetActiveContext()
{
	asCThreadLocalData *tld = asCThreadManager::GetLocalData();

	// tld can be 0 if asGetActiveContext is called before any engine has been created.

	// Observe! I've seen a case where an application linked with the library twice
	// and thus ended up with two separate instances of the code and global variables.
	// The application somehow mixed the two instances so that a function called from
	// a script ended up calling asGetActiveContext from the other instance that had
	// never been initialized.

	if( tld == 0 || tld->activeContexts.GetLength() == 0 )
		return 0;
	return tld->activeContexts[tld->activeContexts.GetLength()-1];
}

// internal
asCThreadLocalData *asPushActiveContext(asIScriptContext *ctx)
{
	asCThreadLocalData *tld = asCThreadManager::GetLocalData();
	asASSERT( tld );
	if( tld == 0 )
		return 0;
	tld->activeContexts.PushLast(ctx);
	return tld;
}

// internal
void asPopActiveContext(asCThreadLocalData *tld, asIScriptContext *ctx)
{
	UNUSED_VAR(ctx);
	asASSERT(tld && tld->activeContexts[tld->activeContexts.GetLength() - 1] == ctx);
	if (tld)
		tld->activeContexts.PopLast();
}

asCContext::asCContext(asCScriptEngine *engine, bool holdRef)
{
	m_refCount.set(1);

	m_holdEngineRef = holdRef;
	if( holdRef )
		engine->AddRef();

	m_engine                    = engine;
	m_status                    = asEXECUTION_UNINITIALIZED;
	m_stackBlockSize            = 0;
	m_originalStackPointer      = 0;
	m_originalStackIndex        = 0;
	m_inExceptionHandler        = false;
	m_isStackMemoryNotAllocated = false;
	m_needToCleanupArgs         = false;
	m_currentFunction           = 0;
	m_callingSystemFunction     = 0;
	m_initialFunction           = 0;
	m_lineCallback              = false;
	m_exceptionCallback         = false;
	m_regs.doProcessSuspend     = false;
	m_doSuspend                 = false;
	m_exceptionWillBeCaught     = false;
	m_regs.ctx                  = this;
	m_regs.objectRegister       = 0;
	m_regs.objectType           = 0;
}

asCContext::~asCContext()
{
	DetachEngine();
}

// interface
bool asCContext::IsNested(asUINT *nestCount) const
{
	if( nestCount )
		*nestCount = 0;

	asUINT c = GetCallstackSize();
	if( c == 0 )
		return false;

	// Search for a marker on the call stack
	// This loop starts at 2 because the 0th entry is not stored in m_callStack,
	// and then we need to subtract one more to get the base of each frame
	for( asUINT n = 2; n <= c; n++ )
	{
		const asPWORD *s = m_callStack.AddressOf() + (c - n)*CALLSTACK_FRAME_SIZE;
		if( s && s[0] == 0 )
		{
			if( nestCount )
				(*nestCount)++;
			else
				return true;
		}
	}

	if( nestCount && *nestCount > 0 )
		return true;

	return false;
}

// interface
int asCContext::AddRef() const
{
	return m_refCount.atomicInc();
}

// interface
int asCContext::Release() const
{
	int r = m_refCount.atomicDec();

	if( r == 0 )
	{
		asDELETE(const_cast<asCContext*>(this),asCContext);
		return 0;
	}

	return r;
}

// internal
void asCContext::DetachEngine()
{
	if( m_engine == 0 ) return;

	// Clean up all calls, included nested ones
	do
	{
		// Abort any execution
		Abort();

		// Free all resources
		Unprepare();
	}
	while( IsNested() );

	// Free the stack blocks
	for( asUINT n = 0; n < m_stackBlocks.GetLength(); n++ )
	{
		if( m_stackBlocks[n] )
		{
#ifndef WIP_16BYTE_ALIGN
			asDELETEARRAY(m_stackBlocks[n]);
#else
			asDELETEARRAYALIGNED(m_stackBlocks[n]);
#endif
		}
	}
	m_stackBlocks.SetLength(0);
	m_stackBlockSize = 0;

	// Clean the user data
	for( asUINT n = 0; n < m_userData.GetLength(); n += 2 )
	{
		if( m_userData[n+1] )
		{
			for( asUINT c = 0; c < m_engine->cleanContextFuncs.GetLength(); c++ )
				if( m_engine->cleanContextFuncs[c].type == m_userData[n] )
					m_engine->cleanContextFuncs[c].cleanFunc(this);
		}
	}
	m_userData.SetLength(0);

	// Clear engine pointer
	if( m_holdEngineRef )
		m_engine->Release();
	m_engine = 0;
}

// interface
asIScriptEngine *asCContext::GetEngine() const
{
	return m_engine;
}

// interface
void *asCContext::SetUserData(void *data, asPWORD type)
{
	// As a thread might add a new new user data at the same time as another
	// it is necessary to protect both read and write access to the userData member
	ACQUIREEXCLUSIVE(m_engine->engineRWLock);

	// It is not intended to store a lot of different types of userdata,
	// so a more complex structure like a associative map would just have
	// more overhead than a simple array.
	for( asUINT n = 0; n < m_userData.GetLength(); n += 2 )
	{
		if( m_userData[n] == type )
		{
			void *oldData = reinterpret_cast<void*>(m_userData[n+1]);
			m_userData[n+1] = reinterpret_cast<asPWORD>(data);

			RELEASEEXCLUSIVE(m_engine->engineRWLock);

			return oldData;
		}
	}

	m_userData.PushLast(type);
	m_userData.PushLast(reinterpret_cast<asPWORD>(data));

	RELEASEEXCLUSIVE(m_engine->engineRWLock);

	return 0;
}

// interface
void *asCContext::GetUserData(asPWORD type) const
{
	// There may be multiple threads reading, but when
	// setting the user data nobody must be reading.
	ACQUIRESHARED(m_engine->engineRWLock);

	for( asUINT n = 0; n < m_userData.GetLength(); n += 2 )
	{
		if( m_userData[n] == type )
		{
			RELEASESHARED(m_engine->engineRWLock);
			return reinterpret_cast<void*>(m_userData[n+1]);
		}
	}

	RELEASESHARED(m_engine->engineRWLock);

	return 0;
}

// interface
asIScriptFunction *asCContext::GetSystemFunction()
{
	return m_callingSystemFunction;
}

// interface
int asCContext::PushFunction(asIScriptFunction *func, void *object)
{
	asCScriptFunction *realFunc = static_cast<asCScriptFunction*>(func);

	if( realFunc == 0 )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "PushFunction", errorNames[-asINVALID_ARG], asINVALID_ARG);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asINVALID_ARG;
	}

	if( m_status != asEXECUTION_DESERIALIZATION )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "PushFunction", errorNames[-asCONTEXT_NOT_PREPARED], asCONTEXT_NOT_PREPARED);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_NOT_PREPARED;
	}

	if( realFunc->funcType == asFUNC_DELEGATE )
	{
		asASSERT(object == 0);

		object   = realFunc->objForDelegate;
		realFunc = realFunc->funcForDelegate;
	}

	realFunc = GetRealFunc(realFunc, &object);

	if( GetCallstackSize() == 0 )
	{
		m_status = asEXECUTION_UNINITIALIZED;
		Prepare(realFunc);
		if(object) 
			*(asPWORD*)&m_regs.stackFramePointer[0] = (asPWORD)object;
		m_status = asEXECUTION_DESERIALIZATION;
	}
	else
	{
		if(realFunc->funcType == asFUNC_INTERFACE || realFunc->funcType == asFUNC_VIRTUAL)
			CallInterfaceMethod(realFunc);
		else
			CallScriptFunction(realFunc);

		if(object) 
			*(asPWORD*)&m_regs.stackFramePointer[0] = (asPWORD)object;
	}

	asASSERT(m_currentFunction->funcType != asFUNC_DELEGATE);

	return asSUCCESS;
}

// interface
int asCContext::GetStateRegisters(asUINT stackLevel, asIScriptFunction **_callingSystemFunction, asIScriptFunction **_initialFunction, asDWORD *_originalStackPointer, asDWORD *_argumentSize, asQWORD *_valueRegister, void **_objectRegister, asITypeInfo **_objectRegisterType)
{
	asIScriptFunction * callingSystemFunction;
	asIScriptFunction * initialFunction;
	asDWORD *           originalStackPointer;
	int                 argumentsSize;
	asQWORD             valueRegister;
	void *              objectRegister;
	asITypeInfo *       objectType;

	if (stackLevel >= GetCallstackSize()) 
		return asINVALID_ARG;

	if( stackLevel == 0 )
	{
		callingSystemFunction = m_callingSystemFunction;
		initialFunction       = m_initialFunction;
		originalStackPointer  = m_originalStackPointer;
		argumentsSize         = m_argumentsSize;

		// Need to push the value of registers so they can be restored
		valueRegister         = m_regs.valueRegister;
		objectRegister        = m_regs.objectRegister;
		objectType            = m_regs.objectType;
	}
	else
	{
		asPWORD const *tmp = &m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE*stackLevel];

		// Only return state registers for a nested call, see PushState()
		if( tmp[0] != 0 )
			return asNO_FUNCTION;

		// Restore the previous initial function and the associated values
		callingSystemFunction = reinterpret_cast<asCScriptFunction*>(tmp[1]);
		initialFunction       = reinterpret_cast<asCScriptFunction*>(tmp[2]);
		originalStackPointer  = (asDWORD*)tmp[3];
		argumentsSize         = (int)tmp[4];

		valueRegister   = asQWORD(asDWORD(tmp[5]));
		valueRegister  |= asQWORD(tmp[6])<<32;
		objectRegister  = (void*)tmp[7];
		objectType      = (asITypeInfo*)tmp[8];
	}

	if(_callingSystemFunction) *_callingSystemFunction = callingSystemFunction;
	if(_initialFunction)       *_initialFunction       = initialFunction;
	asDWORD sp = SerializeStackPointer(originalStackPointer);
	if (_originalStackPointer)  *_originalStackPointer = sp;
	if(_argumentSize)          *_argumentSize          = argumentsSize;
	if(_valueRegister)         *_valueRegister         = valueRegister;
	if(_objectRegister)        *_objectRegister        = objectRegister;
	if(_objectRegisterType)    *_objectRegisterType    = objectType;

	if (int(sp) < 0)
		return asERROR;

	return asSUCCESS;
}

// interface
int asCContext::GetCallStateRegisters(asUINT stackLevel, asDWORD *_stackFramePointer, asIScriptFunction **_currentFunction, asDWORD *_programPointer, asDWORD *_stackPointer, asDWORD *_stackIndex)
{
	asDWORD           *stackFramePointer;
	asCScriptFunction *currentFunction;
	asDWORD           *programPointer;
	asDWORD           *stackPointer;
	int                stackIndex;

	if (stackLevel >= GetCallstackSize())
		return asINVALID_ARG;

	if( stackLevel == 0 )
	{
		stackFramePointer = m_regs.stackFramePointer;
		currentFunction   = m_currentFunction;
		programPointer    = m_regs.programPointer;
		stackPointer      = m_regs.stackPointer;
		stackIndex        = m_stackIndex;
	}
	else
	{
		asPWORD const*s = &m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE*stackLevel];

		stackFramePointer = (asDWORD*)s[0];
		currentFunction   = (asCScriptFunction*)s[1];
		programPointer    = (asDWORD*)s[2];
		stackPointer      = (asDWORD*)s[3];
		stackIndex        = (int)s[4];
	}

	if( stackFramePointer == 0 )
		return asNO_FUNCTION; // It just means that the stackLevel represent a pushed state

	asDWORD sfp = SerializeStackPointer(stackFramePointer);
	if(_stackFramePointer) *_stackFramePointer = sfp; // TODO: Calculate stack frame pointer as delta from previous stack frame pointer (Or perhaps it will always be the same as the stack pointer in previous function?)
	if(_currentFunction)   *_currentFunction   = currentFunction;
	if(_programPointer)    *_programPointer    = programPointer != 0? asUINT(programPointer - currentFunction->scriptData->byteCode.AddressOf()) : -1;
	asDWORD sp = SerializeStackPointer(stackPointer);
	if(_stackPointer)      *_stackPointer      = sp; // TODO: Calculate the stack pointer as offset from the stack frame pointer
	if(_stackIndex)        *_stackIndex        = stackIndex; // TODO: This shouldn't be returned, as it should be calculated during deserialization

	if (int(sfp) < 0 || int(sp) < 0)
		return asERROR;

	return asSUCCESS;
}

// interface
int asCContext::SetStateRegisters(asUINT stackLevel, asIScriptFunction *callingSystemFunction, asIScriptFunction *initialFunction, asDWORD originalStackPointer, asDWORD argumentsSize, asQWORD valueRegister, void *objectRegister, asITypeInfo *objectType)
{
	if( m_status != asEXECUTION_DESERIALIZATION)
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "SetStateRegisters", errorNames[-asCONTEXT_ACTIVE], asCONTEXT_ACTIVE);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_ACTIVE;
	}

	if (stackLevel >= GetCallstackSize())
		return asINVALID_ARG;

	if( stackLevel == 0 )
	{
		m_callingSystemFunction = reinterpret_cast<asCScriptFunction*>(callingSystemFunction);
		m_initialFunction       = reinterpret_cast<asCScriptFunction*>(initialFunction);
		m_originalStackPointer  = DeserializeStackPointer(originalStackPointer);
		m_originalStackIndex    = DetermineStackIndex(m_originalStackPointer);
		if (m_originalStackIndex >= m_stackBlocks.GetLength())
		{
			asCString str;
			str.Format(TXT_FAILED_IN_FUNC_s_s_d, "SetStateRegisters", errorNames[-asCONTEXT_ACTIVE], asCONTEXT_ACTIVE);
			m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			return asINVALID_ARG;
		}
		m_argumentsSize         = argumentsSize; // TODO: Calculate this from the initialFunction so it doesn't need to be serialized

		// Need to push the value of registers so they can be restored
		m_regs.valueRegister  = valueRegister;
		m_regs.objectRegister = objectRegister;
		m_regs.objectType     = objectType;
	}
	else
	{
		asPWORD *tmp = &m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE*stackLevel];

		if(tmp[0] != 0)
			return asERROR; // TODO: This is not really an error. It just means that the stackLevel doesn't represent a pushed state

		tmp[0] = 0;
		tmp[1] = (asPWORD)callingSystemFunction;
		tmp[2] = (asPWORD)initialFunction;
		tmp[3] = (asPWORD)DeserializeStackPointer(originalStackPointer);
		tmp[4] = (asPWORD)argumentsSize; // TODO: Calculate this from the initialFunction so it doesn't need to be serialized

		// Need to push the value of registers so they can be restored
		tmp[5] = (asPWORD)asDWORD(valueRegister);
		tmp[6] = (asPWORD)asDWORD(valueRegister>>32);
		tmp[7] = (asPWORD)objectRegister;
		tmp[8] = (asPWORD)objectType;
	}

	return asSUCCESS;
}

// interface
int asCContext::SetCallStateRegisters(asUINT stackLevel, asDWORD stackFramePointer, asIScriptFunction *_currentFunction, asDWORD _programPointer, asDWORD stackPointer, asDWORD stackIndex)
{
	if( m_status != asEXECUTION_DESERIALIZATION)
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "SetCallStateRegisters", errorNames[-asCONTEXT_ACTIVE], asCONTEXT_ACTIVE);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_ACTIVE;
	}

	if (stackLevel >= GetCallstackSize())
		return asINVALID_ARG;

	// TODO: The arg _currentFunction is just used in debug mode to validate that it is the same that is already given in m_currentFunction or on the call stack. Do we really need to take this argument?
	asCScriptFunction *currentFunction =  static_cast<asCScriptFunction*>(_currentFunction);

	if( currentFunction->funcType == asFUNC_DELEGATE )
	{
		currentFunction = currentFunction->funcForDelegate;
	}

	if( stackLevel == 0 )
	{
		asASSERT(currentFunction->signatureId == m_currentFunction->signatureId);
		currentFunction = m_currentFunction;

		asDWORD *programPointer = currentFunction->scriptData->byteCode.AddressOf();
		if(currentFunction->scriptData->byteCode.GetLength() > _programPointer)
		{
			programPointer += _programPointer;
		}

		m_regs.stackFramePointer = DeserializeStackPointer(stackFramePointer);
		m_regs.programPointer    = programPointer;
		m_regs.stackPointer      = DeserializeStackPointer(stackPointer);
		m_stackIndex             = stackIndex;
	}
	else
	{
		asPWORD *tmp = &m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE*stackLevel];

		asASSERT(currentFunction->signatureId == ((asCScriptFunction*)tmp[1])->signatureId);
		currentFunction = ((asCScriptFunction*)tmp[1]);

		asDWORD *programPointer = currentFunction->scriptData->byteCode.AddressOf();
		if(currentFunction->scriptData->byteCode.GetLength() > _programPointer)
		{
			programPointer += _programPointer;
		}

		tmp[0] = (asPWORD)DeserializeStackPointer(stackFramePointer);
	//	tmp[1] = (asPWORD)(currentFunction);
		tmp[2] = (asPWORD)programPointer;
		tmp[3] = (asPWORD)DeserializeStackPointer(stackPointer);
		tmp[4] = (asPWORD)stackIndex;
	}

	return asSUCCESS;
}

// internal
int asCContext::DetermineStackIndex(asDWORD* ptr) const
{
	for (asUINT n = 0; n < m_stackBlocks.GetLength(); n++)
	{
		asUINT blockSize = m_engine->ep.initContextStackSize << n;
		asINT64 delta = ptr - m_stackBlocks[n];
		if (delta <= blockSize && delta > 0)
			return n;
	}

	return asERROR;
}

// interface
int asCContext::Prepare(asIScriptFunction *func)
{
	if( func == 0 )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_WITH_s_s_d, "Prepare", "null", errorNames[-asNO_FUNCTION], asNO_FUNCTION);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asNO_FUNCTION;
	}

	if( m_status == asEXECUTION_ACTIVE || m_status == asEXECUTION_SUSPENDED )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_WITH_s_s_d, "Prepare", func->GetDeclaration(true, true), errorNames[-asCONTEXT_ACTIVE], asCONTEXT_ACTIVE);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_ACTIVE;
	}

	// Clean the stack if not done before
	if( m_status != asEXECUTION_FINISHED && m_status != asEXECUTION_UNINITIALIZED )
		CleanStack();

	// Release the returned object (if any)
	CleanReturnObject();

	// Check if there has been a previous function prepared
	if (m_initialFunction)
	{
		// Release the previous object, if it is a script object
		if (m_initialFunction && m_initialFunction->objectType && (m_initialFunction->objectType->flags & asOBJ_SCRIPT_OBJECT))
		{
			asCScriptObject* obj = *(asCScriptObject**)&m_regs.stackFramePointer[0];
			if (obj)
				obj->Release();

			*(asPWORD*)&m_regs.stackFramePointer[0] = 0;
		}

		// Reset stack pointer
		m_regs.stackPointer = m_originalStackPointer;
		m_stackIndex = m_originalStackIndex;

		asASSERT(int(m_stackIndex) == DetermineStackIndex(m_regs.stackPointer));
	}

	if( m_initialFunction && m_initialFunction == func )
	{
		// If the same function is executed again, we can skip a lot of the setup
		m_currentFunction = m_initialFunction;
	}
	else
	{
		// Make sure the function is from the same engine as the context to avoid mixups
		if( m_engine != func->GetEngine() )
		{
			asCString str;
			str.Format(TXT_FAILED_IN_FUNC_s_WITH_s_s_d, "Prepare", func->GetDeclaration(true, true), errorNames[-asINVALID_ARG], asINVALID_ARG);
			m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			return asINVALID_ARG;
		}

		if( m_initialFunction )
			m_initialFunction->Release();

		// We trust the application not to pass anything else but a asCScriptFunction
		m_initialFunction = reinterpret_cast<asCScriptFunction *>(func);
		m_initialFunction->AddRef();
		m_currentFunction = m_initialFunction;

		// TODO: runtime optimize: GetSpaceNeededForArguments() should be precomputed
		m_argumentsSize = m_currentFunction->GetSpaceNeededForArguments() + (m_currentFunction->objectType ? AS_PTR_SIZE : 0);

		// Reserve space for the arguments and return value
		if( m_currentFunction->DoesReturnOnStack() )
		{
			m_returnValueSize = m_currentFunction->returnType.GetSizeInMemoryDWords();
			m_argumentsSize += AS_PTR_SIZE;
		}
		else
			m_returnValueSize = 0;

		// Determine the minimum stack size needed
		int stackSize = m_argumentsSize + m_returnValueSize;
		if( m_currentFunction->scriptData )
			stackSize += m_currentFunction->scriptData->stackNeeded;

		// Make sure there is enough space on the stack for the arguments and return value
		if( !ReserveStackSpace(stackSize) )
			return asOUT_OF_MEMORY;

		// Set up the call stack too
		if (m_callStack.GetCapacity() < m_engine->ep.initCallStackSize)
			m_callStack.AllocateNoConstruct(m_engine->ep.initCallStackSize * CALLSTACK_FRAME_SIZE, true);
	}

	// Reset state
	// Most of the time the previous state will be asEXECUTION_FINISHED, in which case the values are already initialized
	if( m_status != asEXECUTION_FINISHED )
	{
		m_exceptionLine           = -1;
		m_exceptionFunction       = 0;
		m_doAbort                 = false;
		m_doSuspend               = false;
		m_regs.doProcessSuspend   = m_lineCallback;
		m_externalSuspendRequest  = false;
	}
	m_status = asEXECUTION_PREPARED;
	m_regs.programPointer = 0;

	// Reserve space for the arguments and return value
	m_regs.stackFramePointer = m_regs.stackPointer - m_argumentsSize - m_returnValueSize;
	m_originalStackPointer   = m_regs.stackPointer;
	m_originalStackIndex     = m_stackIndex;
	m_regs.stackPointer      = m_regs.stackFramePointer;

	// Set arguments to 0
	memset(m_regs.stackPointer, 0, 4*m_argumentsSize);

	if( m_returnValueSize )
	{
		// Set the address of the location where the return value should be put
		asDWORD *ptr = m_regs.stackFramePointer;
		if( m_currentFunction->objectType )
			ptr += AS_PTR_SIZE;

		*(void**)ptr = (void*)(m_regs.stackFramePointer + m_argumentsSize);
	}

	return asSUCCESS;
}

// Free all resources
int asCContext::Unprepare()
{
	if( m_status == asEXECUTION_ACTIVE || m_status == asEXECUTION_SUSPENDED )
		return asCONTEXT_ACTIVE;

	// Set the context as active so that any clean up code can use access it if desired
	asCThreadLocalData *tld = asPushActiveContext((asIScriptContext *)this);
	asDWORD count = m_refCount.get();
	UNUSED_VAR(count);

	// Only clean the stack if the context was prepared but not executed until the end
	if( m_status != asEXECUTION_UNINITIALIZED &&
		m_status != asEXECUTION_FINISHED )
		CleanStack();

	asASSERT( m_needToCleanupArgs == false );

	// Release the returned object (if any)
	CleanReturnObject();

	// TODO: Unprepare is called during destruction, so nobody
	//       must be allowed to keep an extra reference
	asASSERT(m_refCount.get() == count);
	asPopActiveContext(tld, this);

	// Release the object if it is a script object
	if( m_initialFunction && m_initialFunction->objectType && (m_initialFunction->objectType->flags & asOBJ_SCRIPT_OBJECT) )
	{
		asCScriptObject *obj = *(asCScriptObject**)&m_regs.stackFramePointer[0];
		if( obj )
			obj->Release();
	}

	// Release the initial function
	if( m_initialFunction )
	{
		m_initialFunction->Release();

		// Reset stack pointer
		m_regs.stackPointer = m_originalStackPointer;
		m_stackIndex = m_originalStackIndex;
	}

	// Clear function pointers
	m_initialFunction = 0;
	m_currentFunction = 0;
	m_exceptionFunction = 0;
	m_regs.programPointer = 0;

	// Reset status
	m_status = asEXECUTION_UNINITIALIZED;

	m_regs.stackFramePointer = 0;

	return 0;
}

asBYTE asCContext::GetReturnByte()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() ) return 0;

	return *(asBYTE*)&m_regs.valueRegister;
}

asWORD asCContext::GetReturnWord()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() ) return 0;

	return *(asWORD*)&m_regs.valueRegister;
}

asDWORD asCContext::GetReturnDWord()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() ) return 0;

	return *(asDWORD*)&m_regs.valueRegister;
}

asQWORD asCContext::GetReturnQWord()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() ) return 0;

	return m_regs.valueRegister;
}

float asCContext::GetReturnFloat()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() ) return 0;

	return *(float*)&m_regs.valueRegister;
}

double asCContext::GetReturnDouble()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() ) return 0;

	return *(double*)&m_regs.valueRegister;
}

void *asCContext::GetReturnAddress()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( dt->IsReference() )
		return *(void**)&m_regs.valueRegister;
	else if( dt->IsObject() || dt->IsFuncdef() )
	{
		if( m_initialFunction->DoesReturnOnStack() )
		{
			// The address of the return value was passed as the first argument, after the object pointer
			int offset = 0;
			if( m_initialFunction->objectType )
				offset += AS_PTR_SIZE;

			return *(void**)(&m_regs.stackFramePointer[offset]);
		}

		return m_regs.objectRegister;
	}

	return 0;
}

void *asCContext::GetReturnObject()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	if( !dt->IsObject() && !dt->IsFuncdef() ) return 0;

	if( dt->IsReference() )
		return *(void**)(asPWORD)m_regs.valueRegister;
	else
	{
		if( m_initialFunction->DoesReturnOnStack() )
		{
			// The address of the return value was passed as the first argument, after the object pointer
			int offset = 0;
			if( m_initialFunction->objectType )
				offset += AS_PTR_SIZE;

			return *(void**)(&m_regs.stackFramePointer[offset]);
		}

		return m_regs.objectRegister;
	}
}

void *asCContext::GetAddressOfReturnValue()
{
	if( m_status != asEXECUTION_FINISHED ) return 0;

	asCDataType *dt = &m_initialFunction->returnType;

	// An object is stored in the objectRegister
	if( !dt->IsReference() && (dt->IsObject() || dt->IsFuncdef()) )
	{
		// Need to dereference objects
		if( !dt->IsObjectHandle() )
		{
			if( m_initialFunction->DoesReturnOnStack() )
			{
				// The address of the return value was passed as the first argument, after the object pointer
				int offset = 0;
				if( m_initialFunction->objectType )
					offset += AS_PTR_SIZE;

				return *(void**)(&m_regs.stackFramePointer[offset]);
			}

			return *(void**)&m_regs.objectRegister;
		}
		return &m_regs.objectRegister;
	}

	// Primitives and references are stored in valueRegister
	return &m_regs.valueRegister;
}

int asCContext::SetObject(void *obj)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( !m_initialFunction->objectType )
	{
		m_status = asEXECUTION_ERROR;
		return asERROR;
	}

	asASSERT( *(asPWORD*)&m_regs.stackFramePointer[0] == 0 );

	*(asPWORD*)&m_regs.stackFramePointer[0] = (asPWORD)obj;

	// TODO: This should be optional by having a flag where the application can chose whether it should be done or not
	//       The flag could be named something like takeOwnership and have default value of true
	if( obj && (m_initialFunction->objectType->flags & asOBJ_SCRIPT_OBJECT) )
		reinterpret_cast<asCScriptObject*>(obj)->AddRef();

	return 0;
}

int asCContext::SetArgByte(asUINT arg, asBYTE value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	if( dt->GetSizeInMemoryBytes() != 1 )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(asBYTE*)&m_regs.stackFramePointer[offset] = value;

	return 0;
}

int asCContext::SetArgWord(asUINT arg, asWORD value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	if( dt->GetSizeInMemoryBytes() != 2 )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(asWORD*)&m_regs.stackFramePointer[offset] = value;

	return 0;
}

int asCContext::SetArgDWord(asUINT arg, asDWORD value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	if( dt->GetSizeInMemoryBytes() != 4 )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(asDWORD*)&m_regs.stackFramePointer[offset] = value;

	return 0;
}

int asCContext::SetArgQWord(asUINT arg, asQWORD value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	if( dt->GetSizeOnStackDWords() != 2 )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(asQWORD*)(&m_regs.stackFramePointer[offset]) = value;

	return 0;
}

int asCContext::SetArgFloat(asUINT arg, float value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	if( dt->GetSizeOnStackDWords() != 1 )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(float*)(&m_regs.stackFramePointer[offset]) = value;

	return 0;
}

int asCContext::SetArgDouble(asUINT arg, double value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->IsObject() || dt->IsFuncdef() || dt->IsReference() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	if( dt->GetSizeOnStackDWords() != 2 )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(double*)(&m_regs.stackFramePointer[offset]) = value;

	return 0;
}

int asCContext::SetArgAddress(asUINT arg, void *value)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( !dt->IsReference() && !dt->IsObjectHandle() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(asPWORD*)(&m_regs.stackFramePointer[offset]) = (asPWORD)value;

	return 0;
}

int asCContext::SetArgObject(asUINT arg, void *obj)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( !dt->IsObject() && !dt->IsFuncdef() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// If the object should be sent by value we must make a copy of it
	if( !dt->IsReference() )
	{
		if( dt->IsObjectHandle() )
		{
			// Increase the reference counter
			if (obj && dt->IsFuncdef())
				((asIScriptFunction*)obj)->AddRef();
			else
			{
				asSTypeBehaviour *beh = &CastToObjectType(dt->GetTypeInfo())->beh;
				if (obj && beh->addref)
					m_engine->CallObjectMethod(obj, beh->addref);
			}
		}
		else
		{
			obj = m_engine->CreateScriptObjectCopy(obj, dt->GetTypeInfo());
		}
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the value
	*(asPWORD*)(&m_regs.stackFramePointer[offset]) = (asPWORD)obj;

	return 0;
}

int asCContext::SetArgVarType(asUINT arg, void *ptr, int typeId)
{
	if( m_status != asEXECUTION_PREPARED )
		return asCONTEXT_NOT_PREPARED;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_ARG;
	}

	// Verify the type of the argument
	asCDataType *dt = &m_initialFunction->parameterTypes[arg];
	if( dt->GetTokenType() != ttQuestion )
	{
		m_status = asEXECUTION_ERROR;
		return asINVALID_TYPE;
	}

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// Set the typeId and pointer
	*(asPWORD*)(&m_regs.stackFramePointer[offset]) = (asPWORD)ptr;
	offset += AS_PTR_SIZE;
	*(int*)(&m_regs.stackFramePointer[offset]) = typeId;

	return 0;
}

// TODO: Instead of GetAddressOfArg, maybe we need a SetArgValue(int arg, void *value, bool takeOwnership) instead.

// interface
void *asCContext::GetAddressOfArg(asUINT arg)
{
	if( m_status != asEXECUTION_PREPARED )
		return 0;

	if( arg >= (unsigned)m_initialFunction->parameterTypes.GetLength() )
		return 0;

	// Determine the position of the argument
	int offset = 0;
	if( m_initialFunction->objectType )
		offset += AS_PTR_SIZE;

	// If function returns object by value an extra pointer is pushed on the stack
	if( m_returnValueSize )
		offset += AS_PTR_SIZE;

	for( asUINT n = 0; n < arg; n++ )
		offset += m_initialFunction->parameterTypes[n].GetSizeOnStackDWords();

	// We should return the address of the location where the argument value will be placed

	// All registered types are always sent by reference, even if
	// the function is declared to receive the argument by value.
	return &m_regs.stackFramePointer[offset];
}


int asCContext::Abort()
{
	if( m_engine == 0 ) return asERROR;

	// TODO: multithread: Make thread safe. There is a chance that the status
	//                    changes to something else after being set to ABORTED here.
	if( m_status == asEXECUTION_SUSPENDED )
		m_status = asEXECUTION_ABORTED;

	m_doSuspend = true;
	m_regs.doProcessSuspend = true;
	m_externalSuspendRequest = true;
	m_doAbort = true;

	return 0;
}

// interface
int asCContext::Suspend()
{
	// This function just sets some internal flags and is safe
	// to call from a secondary thread, even if the library has
	// been built without multi-thread support.

	if( m_engine == 0 ) return asERROR;

	m_doSuspend = true;
	m_externalSuspendRequest = true;
	m_regs.doProcessSuspend = true;

	return 0;
}

// interface
int asCContext::Execute()
{
	asASSERT( m_engine != 0 );

	if( m_status != asEXECUTION_SUSPENDED && m_status != asEXECUTION_PREPARED )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "Execute", errorNames[-asCONTEXT_NOT_PREPARED], asCONTEXT_NOT_PREPARED);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_NOT_PREPARED;
	}

	m_status = asEXECUTION_ACTIVE;

	asCThreadLocalData *tld = asPushActiveContext((asIScriptContext *)this);

	// Make sure there are not too many nested calls, as it could crash the application
	// by filling up the thread call stack
	if (tld->activeContexts.GetLength() > m_engine->ep.maxNestedCalls)
		SetInternalException(TXT_TOO_MANY_NESTED_CALLS);
	else if( m_regs.programPointer == 0 )
		SetProgramPointer();

	asUINT gcPreObjects = 0;
	if( m_engine->ep.autoGarbageCollect )
		m_engine->gc.GetStatistics(&gcPreObjects, 0, 0, 0, 0);

	while (m_status == asEXECUTION_ACTIVE)
	{
		ExecuteNext();

		// If an exception was raised that will be caught, then unwind the stack
		// and move the program pointer to the catch block before proceeding
		if (m_status == asEXECUTION_EXCEPTION && m_exceptionWillBeCaught)
			CleanStack(true);
	}

	if( m_lineCallback )
	{
		// Call the line callback one last time before leaving
		// so anyone listening can catch the state change
		CallLineCallback();
		m_regs.doProcessSuspend = true;
	}
	else
		m_regs.doProcessSuspend = false;

	m_doSuspend = false;

	if( m_engine->ep.autoGarbageCollect )
	{
		asUINT gcPosObjects = 0;
		m_engine->gc.GetStatistics(&gcPosObjects, 0, 0, 0, 0);
		if( gcPosObjects > gcPreObjects )
		{
			// Execute as many steps as there were new objects created
			m_engine->GarbageCollect(asGC_ONE_STEP | asGC_DESTROY_GARBAGE | asGC_DETECT_GARBAGE, gcPosObjects - gcPreObjects);
		}
		else if( gcPosObjects > 0 )
		{
			// Execute at least one step, even if no new objects were created
			m_engine->GarbageCollect(asGC_ONE_STEP | asGC_DESTROY_GARBAGE | asGC_DETECT_GARBAGE, 1);
		}
	}

	// Pop the active context
	asPopActiveContext(tld, this);

	if( m_status == asEXECUTION_FINISHED )
	{
		m_regs.objectType = m_initialFunction->returnType.GetTypeInfo();
		return asEXECUTION_FINISHED;
	}

	if( m_doAbort )
	{
		m_doAbort = false;

		m_status = asEXECUTION_ABORTED;
		return asEXECUTION_ABORTED;
	}

	if( m_status == asEXECUTION_SUSPENDED )
		return asEXECUTION_SUSPENDED;

	if( m_status == asEXECUTION_EXCEPTION )
		return asEXECUTION_EXCEPTION;

	return asERROR;
}

// internal
asCScriptFunction *asCContext::GetRealFunc(asCScriptFunction * currentFunction, void ** _This)
{
	if( currentFunction->funcType == asFUNC_VIRTUAL ||
		currentFunction->funcType == asFUNC_INTERFACE )
	{
		// The currentFunction is a virtual method

		// Determine the true function from the object
		asCScriptObject *obj = *(asCScriptObject**)_This;

		if( obj == 0 )
		{
			SetInternalException(TXT_NULL_POINTER_ACCESS);
		}
		else
		{
			asCObjectType *objType = obj->objType;
			asCScriptFunction * realFunc = 0;

			if( currentFunction->funcType == asFUNC_VIRTUAL )
			{
				if( objType->virtualFunctionTable.GetLength() > (asUINT)currentFunction->vfTableIdx )
				{
					realFunc = objType->virtualFunctionTable[currentFunction->vfTableIdx];
				}
			}
			else
			{
				// Search the object type for a function that matches the interface function
				for( asUINT n = 0; n < objType->methods.GetLength(); n++ )
				{
					asCScriptFunction *f2 = m_engine->scriptFunctions[objType->methods[n]];
					if( f2->signatureId == currentFunction->signatureId )
					{
						if( f2->funcType == asFUNC_VIRTUAL )
							realFunc = objType->virtualFunctionTable[f2->vfTableIdx];
						else
							realFunc = f2;

						break;
					}
				}
			}

			if( realFunc && realFunc->signatureId == currentFunction->signatureId )
				return realFunc;
			else
				SetInternalException(TXT_NULL_POINTER_ACCESS);
		}
	}
	else if( currentFunction->funcType == asFUNC_IMPORTED )
	{
		int funcId = m_engine->importedFunctions[currentFunction->id & ~FUNC_IMPORTED]->boundFunctionId;
		if( funcId > 0 )
			return m_engine->scriptFunctions[funcId];
		else
			SetInternalException(TXT_UNBOUND_FUNCTION);
	}

	return currentFunction;
}

// internal
void asCContext::SetProgramPointer()
{
	// This shouldn't be called if the program pointer is already set
	asASSERT(m_regs.programPointer == 0);

	// Can't set up the program pointer if no function has been set yet
	asASSERT(m_currentFunction != 0);

	// If the function is a delegate then get then set the function and object from the delegate
	if( m_currentFunction->funcType == asFUNC_DELEGATE )
	{
		// Push the object pointer onto the stack
		asASSERT( m_regs.stackPointer - AS_PTR_SIZE >= m_stackBlocks[m_stackIndex] );
		m_regs.stackPointer -= AS_PTR_SIZE;
		m_regs.stackFramePointer -= AS_PTR_SIZE;
		*(asPWORD*)m_regs.stackPointer = asPWORD(m_currentFunction->objForDelegate);

		// Make the call to the delegated object method
		m_currentFunction = m_currentFunction->funcForDelegate;
	}

	m_currentFunction = GetRealFunc(m_currentFunction, (void**)m_regs.stackFramePointer);

	if( m_currentFunction->funcType == asFUNC_SCRIPT )
	{
		m_regs.programPointer = m_currentFunction->scriptData->byteCode.AddressOf();

		// Set up the internal registers for executing the script function
		PrepareScriptFunction();
	}
	else if( m_currentFunction->funcType == asFUNC_SYSTEM )
	{
		asASSERT(m_status != asEXECUTION_DESERIALIZATION);

		// The current function is an application registered function

		// Call the function directly
		CallSystemFunction(m_currentFunction->id, this);

		// Was the call successful?
		if( m_status == asEXECUTION_ACTIVE )
			m_status = asEXECUTION_FINISHED;
	}
	else
	{
		// This can happen, e.g. if attempting to call a template function
		if( m_status != asEXECUTION_EXCEPTION )
			SetInternalException(TXT_NULL_POINTER_ACCESS, false);
	}
}

// interface
int asCContext::PushState()
{
	// Only allow the state to be pushed when active
	// TODO: Can we support a suspended state too? So the reuse of
	//       the context can be done outside the Execute() call?
	if( m_status != asEXECUTION_ACTIVE )
	{
		// TODO: Write message. Wrong usage
		return asERROR;
	}

	// Allocate space on the callstack for at least two states
	if (m_callStack.GetLength() >= m_callStack.GetCapacity() - 2*CALLSTACK_FRAME_SIZE)
	{
		if (m_engine->ep.maxCallStackSize > 0 && m_callStack.GetLength() >= m_engine->ep.maxCallStackSize*CALLSTACK_FRAME_SIZE)
		{
			// The call stack is too big to grow further
			// If an error occurs, no change to the context should be done
			return asOUT_OF_MEMORY;
		}

		// Allocate space for 10 call states at a time to save time
		m_callStack.AllocateNoConstruct(m_callStack.GetLength() + 10 * CALLSTACK_FRAME_SIZE, true);
	}

	// Push the current script function that is calling the system function
	// This cannot fail, since the memory was already allocated above
	PushCallState();

	// Push the system function too, which will serve both as a marker and
	// informing which system function that created the nested call
	m_callStack.SetLengthNoConstruct(m_callStack.GetLength() + CALLSTACK_FRAME_SIZE);

	// Need to push m_initialFunction as it must be restored later
	asPWORD *tmp = m_callStack.AddressOf() + m_callStack.GetLength() - CALLSTACK_FRAME_SIZE;
	tmp[0] = 0;
	tmp[1] = (asPWORD)m_callingSystemFunction;
	tmp[2] = (asPWORD)m_initialFunction;
	tmp[3] = (asPWORD)m_originalStackPointer;
	tmp[4] = (asPWORD)m_argumentsSize;

	// Need to push the value of registers so they can be restored
	tmp[5] = (asPWORD)asDWORD(m_regs.valueRegister);
	tmp[6] = (asPWORD)asDWORD(m_regs.valueRegister>>32);
	tmp[7] = (asPWORD)m_regs.objectRegister;
	tmp[8] = (asPWORD)m_regs.objectType;

	// Decrease stackpointer to prevent the top value from being overwritten
	m_regs.stackPointer -= 2;

	// Clear the initial function so that Prepare() knows it must do all validations
	m_initialFunction = 0;

	// After this the state should appear as if uninitialized
	m_callingSystemFunction = 0;

	m_regs.objectRegister = 0;
	m_regs.objectType = 0;

	// Set the status to uninitialized as application
	// should call Prepare() after this to reuse the context
	if( m_status != asEXECUTION_DESERIALIZATION )
		m_status = asEXECUTION_UNINITIALIZED;

	return asSUCCESS;
}

// interface
int asCContext::PopState()
{
	if( !IsNested() )
		return asERROR;

	// Clean up the current execution
	Unprepare();

	// The topmost state must be a marker for nested call
	asASSERT( m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE] == 0 );

	// Restore the previous state
	asPWORD *tmp = &m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE];
	m_callingSystemFunction = reinterpret_cast<asCScriptFunction*>(tmp[1]);
	m_callStack.SetLength(m_callStack.GetLength() - CALLSTACK_FRAME_SIZE);

	// Restore the previous initial function and the associated values
	m_initialFunction      = reinterpret_cast<asCScriptFunction*>(tmp[2]);
	m_originalStackPointer = (asDWORD*)tmp[3];
	m_originalStackIndex   = DetermineStackIndex(m_originalStackPointer);
	m_argumentsSize        = (int)tmp[4];

	m_regs.valueRegister   = asQWORD(asDWORD(tmp[5]));
	m_regs.valueRegister  |= asQWORD(tmp[6])<<32;
	m_regs.objectRegister  = (void*)tmp[7];
	m_regs.objectType      = (asITypeInfo*)tmp[8];

	// Calculate the returnValueSize
	if( m_initialFunction->DoesReturnOnStack() )
		m_returnValueSize = m_initialFunction->returnType.GetSizeInMemoryDWords();
	else
		m_returnValueSize = 0;

	// Pop the current script function. This will also restore the previous stack pointer
	PopCallState();

	m_status = asEXECUTION_ACTIVE;

	return asSUCCESS;
}

// internal
int asCContext::PushCallState()
{
	// PushCallState is called whenever we already have m_callStack n*CALLSTACK_FRAME_SIZE memory
	// We only need to increment it if it is full (old_length >= m_callStack.maxLength)
	// Here we assume that AllocateNoConstruct will always execute and allocate us extra memory,
	// so we can use the faster m_callStack.SetLengthNoAllocate since it is already known the capacity 
	// is enough.

	asUINT oldLength = m_callStack.GetLength();
    if (oldLength >= m_callStack.GetCapacity())
    {
        if (m_engine->ep.maxCallStackSize > 0 && oldLength >= m_engine->ep.maxCallStackSize * CALLSTACK_FRAME_SIZE)
        {
			// The call stack is too big to grow further
            SetInternalException(TXT_STACK_OVERFLOW);
            return asERROR;
        }
        m_callStack.AllocateNoConstruct(oldLength + 10 * CALLSTACK_FRAME_SIZE, true);
    }
	m_callStack.SetLengthNoAllocate(oldLength + CALLSTACK_FRAME_SIZE);

    // Separating the loads and stores limits data cache trash, and with a smart compiler
    // could turn into SIMD style loading/storing if available.
    // The compiler can't do this itself due to potential pointer aliasing between the pointers,
    // ie writing to tmp could overwrite the data contained in registers.stackFramePointer for example
    // for all the compiler knows. So introducing the local variable s, which is never referred to by
    // its address we avoid this issue.

	asPWORD s[5];
	s[0] = (asPWORD)m_regs.stackFramePointer;
	s[1] = (asPWORD)m_currentFunction;
	s[2] = (asPWORD)m_regs.programPointer;
	s[3] = (asPWORD)m_regs.stackPointer;
	s[4] = m_stackIndex;

	asPWORD *tmp = m_callStack.AddressOf() + oldLength;
	tmp[0] = s[0];
	tmp[1] = s[1];
	tmp[2] = s[2];
	tmp[3] = s[3];
	tmp[4] = s[4];

	return asSUCCESS;
}

// internal
void asCContext::PopCallState()
{
	// See comments in PushCallState about pointer aliasing and data cache trashing
	asUINT newLength = m_callStack.GetLength() - CALLSTACK_FRAME_SIZE;

	asPWORD *tmp = m_callStack.array + newLength;
	asPWORD s[5];
	s[0] = tmp[0];
	s[1] = tmp[1];
	s[2] = tmp[2];
	s[3] = tmp[3];
	s[4] = tmp[4];

	m_regs.stackFramePointer = (asDWORD*)s[0];
	m_currentFunction        = (asCScriptFunction*)s[1];
	m_regs.programPointer    = (asDWORD*)s[2];
	m_regs.stackPointer      = (asDWORD*)s[3];
	m_stackIndex             = (int)s[4];

	// Here we reduce the length, so we can use the faster SetLengtNoAllocate.
	m_callStack.SetLengthNoAllocate(newLength); 
}

// interface
asUINT asCContext::GetCallstackSize() const
{
	if( m_currentFunction == 0 ) return 0;

	// The current function is accessed at stackLevel 0
	return asUINT(1 + m_callStack.GetLength() / CALLSTACK_FRAME_SIZE);
}

// interface
asIScriptFunction *asCContext::GetFunction(asUINT stackLevel)
{
	if( stackLevel >= GetCallstackSize() ) return 0;

	if( stackLevel == 0 ) return m_currentFunction;

	asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize() - stackLevel - 1)*CALLSTACK_FRAME_SIZE;
	asCScriptFunction *func = (asCScriptFunction*)s[1];

	return func;
}

// interface
int asCContext::GetLineNumber(asUINT stackLevel, int *column, const char **sectionName)
{
	if( stackLevel >= GetCallstackSize() ) return asINVALID_ARG;

	asCScriptFunction *func;
	asDWORD *bytePos;
	if( stackLevel == 0 )
	{
		func = m_currentFunction;
		if( func->scriptData == 0 ) return 0;
		bytePos = m_regs.programPointer;
	}
	else
	{
		asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize()-stackLevel-1)*CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		if( func->scriptData == 0 ) return 0;
		bytePos = (asDWORD*)s[2];

		// Subract 1 from the bytePos, because we want the line where
		// the call was made, and not the instruction after the call
		bytePos -= 1;
	}

	// For nested calls it is possible that func is null
	if( func == 0 )
	{
		if( column ) *column = 0;
		if( sectionName ) *sectionName = 0;
		return 0;
	}

	if (bytePos == 0)
	{
		// If the context has been Prepared but Execute hasn't been called yet the 
		// programPointer will be zero. In this case simply use the address of the 
		// bytecode as starting point
		bytePos = func->scriptData->byteCode.AddressOf();
	}

	int sectionIdx;
	asDWORD line = func->GetLineNumber(int(bytePos - func->scriptData->byteCode.AddressOf()), &sectionIdx);
	if( column ) *column = (line >> 20);
	if( sectionName )
	{
		asASSERT( sectionIdx < int(m_engine->scriptSectionNames.GetLength()) );
		if( sectionIdx >= 0 && asUINT(sectionIdx) < m_engine->scriptSectionNames.GetLength() )
			*sectionName = m_engine->scriptSectionNames[sectionIdx]->AddressOf();
		else
			*sectionName = 0;
	}
	return (line & 0xFFFFF);
}

// internal
bool asCContext::ReserveStackSpace(asUINT size)
{
#ifdef WIP_16BYTE_ALIGN
	// Pad size to a multiple of MAX_TYPE_ALIGNMENT.
	const asUINT remainder = size % MAX_TYPE_ALIGNMENT;
	if(remainder != 0)
	{
		size = size + (MAX_TYPE_ALIGNMENT - (size % MAX_TYPE_ALIGNMENT));
	}
#endif

	// Make sure the first stack block is allocated
	if( m_stackBlocks.GetLength() == 0 )
	{
		m_stackBlockSize = m_engine->ep.initContextStackSize;
		asASSERT( m_stackBlockSize > 0 );

#ifndef WIP_16BYTE_ALIGN
		asDWORD *stack = asNEWARRAY(asDWORD,m_stackBlockSize);
#else
		asDWORD *stack = asNEWARRAYALIGNED(asDWORD, m_stackBlockSize, MAX_TYPE_ALIGNMENT);
#endif
		if( stack == 0 )
		{
			// Out of memory
			return false;
		}

#ifdef WIP_16BYTE_ALIGN
		asASSERT( isAligned(stack, MAX_TYPE_ALIGNMENT) );
#endif

		m_stackBlocks.PushLast(stack);
		m_stackIndex = 0;
		m_regs.stackPointer = m_stackBlocks[0] + m_stackBlockSize;

#ifdef WIP_16BYTE_ALIGN
		// Align the stack pointer. This is necessary as the m_stackBlockSize is not necessarily evenly divisable with the max alignment
		((asPWORD&)m_regs.stackPointer) &= ~(MAX_TYPE_ALIGNMENT-1);

		asASSERT( isAligned(m_regs.stackPointer, MAX_TYPE_ALIGNMENT) );
#endif
	}

	// Check if there is enough space on the current stack block, otherwise move
	// to the next one. New and larger blocks will be allocated as necessary
	while( m_regs.stackPointer - (size + RESERVE_STACK) < m_stackBlocks[m_stackIndex] )
	{
		// Make sure we don't allocate more space than allowed
		if( m_engine->ep.maximumContextStackSize )
		{
			// This test will only stop growth once it is on or already crossed the limit
			if( m_stackBlockSize * ((1 << (m_stackIndex+1)) - 1) >= m_engine->ep.maximumContextStackSize )
			{
				m_isStackMemoryNotAllocated = true;

				// Set the stackFramePointer, even though the stackPointer wasn't updated
				m_regs.stackFramePointer = m_regs.stackPointer;

				SetInternalException(TXT_STACK_OVERFLOW);
				return false;
			}
		}

		m_stackIndex++;
		if( m_stackBlocks.GetLength() == m_stackIndex )
		{
			// Allocate the new stack block, with twice the size of the previous
#ifndef WIP_16BYTE_ALIGN
			asDWORD *stack = asNEWARRAY(asDWORD, (m_stackBlockSize << m_stackIndex));
#else
			asDWORD *stack = asNEWARRAYALIGNED(asDWORD, (m_stackBlockSize << m_stackIndex), MAX_TYPE_ALIGNMENT);
#endif
			if( stack == 0 )
			{
				// Out of memory
				m_isStackMemoryNotAllocated = true;

				// Set the stackFramePointer, even though the stackPointer wasn't updated
				m_regs.stackFramePointer = m_regs.stackPointer;

				SetInternalException(TXT_STACK_OVERFLOW);
				return false;
			}

#ifdef WIP_16BYTE_ALIGN
			asASSERT( isAligned(stack, MAX_TYPE_ALIGNMENT) );
#endif

			m_stackBlocks.PushLast(stack);
		}

		// Update the stack pointer to point to the new block.
		// Leave enough room above the stackpointer to copy the arguments from the previous stackblock
		m_regs.stackPointer = m_stackBlocks[m_stackIndex] +
			                  (m_stackBlockSize<<m_stackIndex) -
			                  m_currentFunction->GetSpaceNeededForArguments() -
			                  (m_currentFunction->objectType ? AS_PTR_SIZE : 0) -
			                  (m_currentFunction->DoesReturnOnStack() ? AS_PTR_SIZE : 0);

#ifdef WIP_16BYTE_ALIGN
		// Align the stack pointer
		(asPWORD&)m_regs.stackPointer &= ~(MAX_TYPE_ALIGNMENT-1);

		asASSERT( isAligned(m_regs.stackPointer, MAX_TYPE_ALIGNMENT) );
#endif
	}

	return true;
}

// internal
void asCContext::CallScriptFunction(asCScriptFunction *func)
{
	asASSERT( func->scriptData );

	// Push the framepointer, function id and programCounter on the stack
	if (PushCallState() < 0)
		return;

	// Update the current function and program position before increasing the stack
	// so the exception handler will know what to do if there is a stack overflow
	m_currentFunction = func;
	m_regs.programPointer = m_currentFunction->scriptData->byteCode.AddressOf();

	PrepareScriptFunction();
}

void asCContext::PrepareScriptFunction()
{
	asASSERT( m_currentFunction->scriptData );

	// Make sure there is space on the stack to execute the function
	asDWORD *oldStackPointer = m_regs.stackPointer;
	asUINT needSize = m_currentFunction->scriptData->stackNeeded;

	// With a quick check we know right away that we don't need to call ReserveStackSpace and do other checks inside it
	if (m_stackBlocks.GetLength() == 0 ||
		oldStackPointer - (needSize + RESERVE_STACK) < m_stackBlocks[m_stackIndex])
	{
		if( !ReserveStackSpace(needSize) )
			return;

		if( m_regs.stackPointer != oldStackPointer )
		{
			int numDwords = m_currentFunction->GetSpaceNeededForArguments() +
			                (m_currentFunction->objectType ? AS_PTR_SIZE : 0) +
			                (m_currentFunction->DoesReturnOnStack() ? AS_PTR_SIZE : 0);
			memcpy(m_regs.stackPointer, oldStackPointer, sizeof(asDWORD)*numDwords);
		}
	}

	// Update framepointer
	m_regs.stackFramePointer = m_regs.stackPointer;

	// Set all object variables to 0 to guarantee that they are null before they are used
	// Only variables on the heap should be cleared. The rest will be cleared by calling the constructor
	// TODO: Need a fast way to iterate over this list (perhaps a pointer in variables to give index of next object var, or perhaps just order the array with object types first)
	for (asUINT n = m_currentFunction->scriptData->variables.GetLength(); n-- > 0; )
	{
		asSScriptVariable *var = m_currentFunction->scriptData->variables[n];

		// Don't clear the function arguments
		if (var->stackOffset <= 0)
			continue;

		if( var->onHeap && (var->type.IsObject() || var->type.IsFuncdef()) )
			*(asPWORD*)&m_regs.stackFramePointer[-var->stackOffset] = 0;
	}

	// Initialize the stack pointer with the space needed for local variables
	m_regs.stackPointer -= m_currentFunction->scriptData->variableSpace;

	// Call the line callback for each script function, to guarantee that infinitely recursive scripts can
	// be interrupted, even if the scripts have been compiled with asEP_BUILD_WITHOUT_LINE_CUES
	if( m_regs.doProcessSuspend )
	{
		if( m_lineCallback )
			CallLineCallback();
		if( m_doSuspend )
			m_status = asEXECUTION_SUSPENDED;
	}
}

void asCContext::CallInterfaceMethod(asCScriptFunction *func)
{
	// Resolve the interface method using the current script type
	asCScriptObject *obj = *(asCScriptObject**)(asPWORD*)m_regs.stackPointer;
	if( obj == 0 )
	{
		// Tell the exception handler to clean up the arguments to this method
		m_needToCleanupArgs = true;
		SetInternalException(TXT_NULL_POINTER_ACCESS);
		return;
	}

	asCObjectType *objType = obj->objType;

	// Search the object type for a function that matches the interface function
	asCScriptFunction *realFunc = 0;
	if( func->funcType == asFUNC_INTERFACE )
	{
		// Find the offset for the interface's virtual function table chunk
		asUINT offset = 0;
		bool found = false;
		asCObjectType *findInterface = func->objectType;

		// TODO: runtime optimize: The list of interfaces should be ordered by the address
		//                         Then a binary search pattern can be used.
		asUINT intfCount = asUINT(objType->interfaces.GetLength());
		for( asUINT n = 0; n < intfCount; n++ )
		{
			if( objType->interfaces[n] == findInterface )
			{
				offset = objType->interfaceVFTOffsets[n];
				found = true;
				break;
			}
		}

		if( !found )
		{
			// Tell the exception handler to clean up the arguments to this method
			m_needToCleanupArgs = true;
			SetInternalException(TXT_NULL_POINTER_ACCESS);
			return;
		}

		// Find the real function in the virtual table chunk with the found offset
		realFunc = objType->virtualFunctionTable[func->vfTableIdx + offset];

		// Since the interface was implemented by the class, it shouldn't
		// be possible that the real function isn't found
		asASSERT( realFunc );

		asASSERT( realFunc->signatureId == func->signatureId );
	}
	else // if( func->funcType == asFUNC_VIRTUAL )
	{
		realFunc = objType->virtualFunctionTable[func->vfTableIdx];
	}

	// Then call the true script function
	CallScriptFunction(realFunc);
}

#if AS_USE_COMPUTED_GOTOS
#define INSTRUCTION(x) case_##x
#define NEXT_INSTRUCTION() goto *(void*) dispatch_table[*(asBYTE*)l_bc]
#define BEGIN() NEXT_INSTRUCTION();
#else
#define INSTRUCTION(x) case x
#define NEXT_INSTRUCTION() break
#define BEGIN() switch( *(asBYTE*)l_bc )
#endif

void asCContext::ExecuteNext()
{
#if AS_USE_COMPUTED_GOTOS
static const void *const dispatch_table[256] = {
&&INSTRUCTION(asBC_PopPtr),		&&INSTRUCTION(asBC_PshGPtr),	&&INSTRUCTION(asBC_PshC4),		&&INSTRUCTION(asBC_PshV4),
&&INSTRUCTION(asBC_PSF),		&&INSTRUCTION(asBC_SwapPtr),	&&INSTRUCTION(asBC_NOT),		&&INSTRUCTION(asBC_PshG4),
&&INSTRUCTION(asBC_LdGRdR4),	&&INSTRUCTION(asBC_CALL),		&&INSTRUCTION(asBC_RET),		&&INSTRUCTION(asBC_JMP),
&&INSTRUCTION(asBC_JZ),			&&INSTRUCTION(asBC_JNZ),		&&INSTRUCTION(asBC_JS),			&&INSTRUCTION(asBC_JNS),
&&INSTRUCTION(asBC_JP),			&&INSTRUCTION(asBC_JNP),		&&INSTRUCTION(asBC_TZ),			&&INSTRUCTION(asBC_TNZ),
&&INSTRUCTION(asBC_TS),			&&INSTRUCTION(asBC_TNS),		&&INSTRUCTION(asBC_TP),			&&INSTRUCTION(asBC_TNP),
&&INSTRUCTION(asBC_NEGi),		&&INSTRUCTION(asBC_NEGf),		&&INSTRUCTION(asBC_NEGd),		&&INSTRUCTION(asBC_INCi16),
&&INSTRUCTION(asBC_INCi8),		&&INSTRUCTION(asBC_DECi16),		&&INSTRUCTION(asBC_DECi8),		&&INSTRUCTION(asBC_INCi),
&&INSTRUCTION(asBC_DECi),		&&INSTRUCTION(asBC_INCf),		&&INSTRUCTION(asBC_DECf),		&&INSTRUCTION(asBC_INCd),
&&INSTRUCTION(asBC_DECd),		&&INSTRUCTION(asBC_IncVi),		&&INSTRUCTION(asBC_DecVi),		&&INSTRUCTION(asBC_BNOT),
&&INSTRUCTION(asBC_BAND),		&&INSTRUCTION(asBC_BOR),		&&INSTRUCTION(asBC_BXOR),		&&INSTRUCTION(asBC_BSLL),
&&INSTRUCTION(asBC_BSRL),		&&INSTRUCTION(asBC_BSRA),		&&INSTRUCTION(asBC_COPY),		&&INSTRUCTION(asBC_PshC8),
&&INSTRUCTION(asBC_PshVPtr),	&&INSTRUCTION(asBC_RDSPtr),		&&INSTRUCTION(asBC_CMPd),		&&INSTRUCTION(asBC_CMPu),
&&INSTRUCTION(asBC_CMPf),		&&INSTRUCTION(asBC_CMPi),		&&INSTRUCTION(asBC_CMPIi),		&&INSTRUCTION(asBC_CMPIf),
&&INSTRUCTION(asBC_CMPIu),		&&INSTRUCTION(asBC_JMPP),		&&INSTRUCTION(asBC_PopRPtr),	&&INSTRUCTION(asBC_PshRPtr),
&&INSTRUCTION(asBC_STR),		&&INSTRUCTION(asBC_CALLSYS),	&&INSTRUCTION(asBC_CALLBND),	&&INSTRUCTION(asBC_SUSPEND),
&&INSTRUCTION(asBC_ALLOC),		&&INSTRUCTION(asBC_FREE),		&&INSTRUCTION(asBC_LOADOBJ),	&&INSTRUCTION(asBC_STOREOBJ),
&&INSTRUCTION(asBC_GETOBJ),		&&INSTRUCTION(asBC_REFCPY),		&&INSTRUCTION(asBC_CHKREF),		&&INSTRUCTION(asBC_GETOBJREF),
&&INSTRUCTION(asBC_GETREF),		&&INSTRUCTION(asBC_PshNull),	&&INSTRUCTION(asBC_ClrVPtr),	&&INSTRUCTION(asBC_OBJTYPE),
&&INSTRUCTION(asBC_TYPEID),		&&INSTRUCTION(asBC_SetV4),		&&INSTRUCTION(asBC_SetV8),		&&INSTRUCTION(asBC_ADDSi),
&&INSTRUCTION(asBC_CpyVtoV4),	&&INSTRUCTION(asBC_CpyVtoV8),	&&INSTRUCTION(asBC_CpyVtoR4),	&&INSTRUCTION(asBC_CpyVtoR8),
&&INSTRUCTION(asBC_CpyVtoG4),	&&INSTRUCTION(asBC_CpyRtoV4),	&&INSTRUCTION(asBC_CpyRtoV8),	&&INSTRUCTION(asBC_CpyGtoV4),
&&INSTRUCTION(asBC_WRTV1),		&&INSTRUCTION(asBC_WRTV2),		&&INSTRUCTION(asBC_WRTV4),		&&INSTRUCTION(asBC_WRTV8),
&&INSTRUCTION(asBC_RDR1),		&&INSTRUCTION(asBC_RDR2),		&&INSTRUCTION(asBC_RDR4),		&&INSTRUCTION(asBC_RDR8),
&&INSTRUCTION(asBC_LDG),		&&INSTRUCTION(asBC_LDV),		&&INSTRUCTION(asBC_PGA),		&&INSTRUCTION(asBC_CmpPtr),
&&INSTRUCTION(asBC_VAR),		&&INSTRUCTION(asBC_iTOf),		&&INSTRUCTION(asBC_fTOi),		&&INSTRUCTION(asBC_uTOf),
&&INSTRUCTION(asBC_fTOu),		&&INSTRUCTION(asBC_sbTOi),		&&INSTRUCTION(asBC_swTOi),		&&INSTRUCTION(asBC_ubTOi),
&&INSTRUCTION(asBC_uwTOi),		&&INSTRUCTION(asBC_dTOi),		&&INSTRUCTION(asBC_dTOu),		&&INSTRUCTION(asBC_dTOf),
&&INSTRUCTION(asBC_iTOd),		&&INSTRUCTION(asBC_uTOd),		&&INSTRUCTION(asBC_fTOd),		&&INSTRUCTION(asBC_ADDi),
&&INSTRUCTION(asBC_SUBi),		&&INSTRUCTION(asBC_MULi),		&&INSTRUCTION(asBC_DIVi),		&&INSTRUCTION(asBC_MODi),
&&INSTRUCTION(asBC_ADDf),		&&INSTRUCTION(asBC_SUBf),		&&INSTRUCTION(asBC_MULf),		&&INSTRUCTION(asBC_DIVf),
&&INSTRUCTION(asBC_MODf),		&&INSTRUCTION(asBC_ADDd),		&&INSTRUCTION(asBC_SUBd),		&&INSTRUCTION(asBC_MULd),
&&INSTRUCTION(asBC_DIVd),		&&INSTRUCTION(asBC_MODd),		&&INSTRUCTION(asBC_ADDIi),		&&INSTRUCTION(asBC_SUBIi),
&&INSTRUCTION(asBC_MULIi),		&&INSTRUCTION(asBC_ADDIf),		&&INSTRUCTION(asBC_SUBIf),		&&INSTRUCTION(asBC_MULIf),
&&INSTRUCTION(asBC_SetG4),		&&INSTRUCTION(asBC_ChkRefS),	&&INSTRUCTION(asBC_ChkNullV),	&&INSTRUCTION(asBC_CALLINTF),
&&INSTRUCTION(asBC_iTOb),		&&INSTRUCTION(asBC_iTOw),		&&INSTRUCTION(asBC_SetV1),		&&INSTRUCTION(asBC_SetV2),
&&INSTRUCTION(asBC_Cast),		&&INSTRUCTION(asBC_i64TOi),		&&INSTRUCTION(asBC_uTOi64),		&&INSTRUCTION(asBC_iTOi64),
&&INSTRUCTION(asBC_fTOi64),		&&INSTRUCTION(asBC_dTOi64),		&&INSTRUCTION(asBC_fTOu64),		&&INSTRUCTION(asBC_dTOu64),
&&INSTRUCTION(asBC_i64TOf),		&&INSTRUCTION(asBC_u64TOf),		&&INSTRUCTION(asBC_i64TOd),		&&INSTRUCTION(asBC_u64TOd),
&&INSTRUCTION(asBC_NEGi64),		&&INSTRUCTION(asBC_INCi64),		&&INSTRUCTION(asBC_DECi64),		&&INSTRUCTION(asBC_BNOT64),
&&INSTRUCTION(asBC_ADDi64),		&&INSTRUCTION(asBC_SUBi64),		&&INSTRUCTION(asBC_MULi64),		&&INSTRUCTION(asBC_DIVi64),
&&INSTRUCTION(asBC_MODi64),		&&INSTRUCTION(asBC_BAND64),		&&INSTRUCTION(asBC_BOR64),		&&INSTRUCTION(asBC_BXOR64),
&&INSTRUCTION(asBC_BSLL64),		&&INSTRUCTION(asBC_BSRL64),		&&INSTRUCTION(asBC_BSRA64),		&&INSTRUCTION(asBC_CMPi64),
&&INSTRUCTION(asBC_CMPu64),		&&INSTRUCTION(asBC_ChkNullS),	&&INSTRUCTION(asBC_ClrHi),		&&INSTRUCTION(asBC_JitEntry),
&&INSTRUCTION(asBC_CallPtr),	&&INSTRUCTION(asBC_FuncPtr),	&&INSTRUCTION(asBC_LoadThisR),	&&INSTRUCTION(asBC_PshV8),
&&INSTRUCTION(asBC_DIVu),		&&INSTRUCTION(asBC_MODu),		&&INSTRUCTION(asBC_DIVu64),		&&INSTRUCTION(asBC_MODu64),
&&INSTRUCTION(asBC_LoadRObjR),	&&INSTRUCTION(asBC_LoadVObjR),	&&INSTRUCTION(asBC_RefCpyV),	&&INSTRUCTION(asBC_JLowZ),
&&INSTRUCTION(asBC_JLowNZ),		&&INSTRUCTION(asBC_AllocMem),	&&INSTRUCTION(asBC_SetListSize),&&INSTRUCTION(asBC_PshListElmnt),
&&INSTRUCTION(asBC_SetListType),&&INSTRUCTION(asBC_POWi),		&&INSTRUCTION(asBC_POWu),		&&INSTRUCTION(asBC_POWf),
&&INSTRUCTION(asBC_POWd),		&&INSTRUCTION(asBC_POWdi),		&&INSTRUCTION(asBC_POWi64),		&&INSTRUCTION(asBC_POWu64),
&&INSTRUCTION(asBC_Thiscall1),

								&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),
&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT),			&&INSTRUCTION(FAULT)
};
#endif

	asDWORD *l_bc = m_regs.programPointer;
	asDWORD *l_sp = m_regs.stackPointer;
	asDWORD *l_fp = m_regs.stackFramePointer;

	for(;;)
	{

#ifdef AS_DEBUG
	// Gather statistics on executed bytecode
	stats.Instr(*(asBYTE*)l_bc, !m_engine->ep.noDebugOutput);

	// Used to verify that the size of the instructions are correct
	asDWORD *old = l_bc;
#endif


	// Remember to keep the cases in order and without
	// gaps, because that will make the switch faster.
	// It will be faster since only one lookup will be
	// made to find the correct jump destination. If not
	// in order, the switch will make two lookups.
	BEGIN()
	{
//--------------
// memory access functions

	INSTRUCTION(asBC_PopPtr):
		// Pop a pointer from the stack
		l_sp += AS_PTR_SIZE;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PshGPtr):
		// Replaces PGA + RDSPtr
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = *(asPWORD*)asBC_PTRARG(l_bc);
		l_bc += 1 + AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	// Push a dword value on the stack
	INSTRUCTION(asBC_PshC4):
		--l_sp;
		*l_sp = asBC_DWORDARG(l_bc);
		l_bc += 2;
		NEXT_INSTRUCTION();

	// Push the dword value of a variable on the stack
	INSTRUCTION(asBC_PshV4):
		--l_sp;
		*l_sp = *(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	// Push the address of a variable on the stack
	INSTRUCTION(asBC_PSF):
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = asPWORD(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	// Swap the top 2 pointers on the stack
	INSTRUCTION(asBC_SwapPtr):
		{
			asPWORD p = *(asPWORD*)l_sp;
			*(asPWORD*)l_sp = *(asPWORD*)(l_sp+AS_PTR_SIZE);
			*(asPWORD*)(l_sp+AS_PTR_SIZE) = p;
			l_bc++;
		}
		NEXT_INSTRUCTION();

	// Do a boolean not operation, modifying the value of the variable
	INSTRUCTION(asBC_NOT):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is equal to 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on the pointer.

			volatile asBYTE *ptr = (asBYTE*)(l_fp - asBC_SWORDARG0(l_bc));
			asBYTE val = (ptr[0] == 0) ? VALUE_OF_BOOLEAN_TRUE : 0;
			ptr[0] = val; // The result is stored in the lower byte
			ptr[1] = 0;   // Make sure the rest of the DWORD is 0
			ptr[2] = 0;
			ptr[3] = 0;
		}
#else
		*(l_fp - asBC_SWORDARG0(l_bc)) = (*(l_fp - asBC_SWORDARG0(l_bc)) == 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	// Push the dword value of a global variable on the stack
	INSTRUCTION(asBC_PshG4):
		--l_sp;
		*l_sp = *(asDWORD*)asBC_PTRARG(l_bc);
		l_bc += 1 + AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	// Load the address of a global variable in the register, then
	// copy the value of the global variable into a local variable
	INSTRUCTION(asBC_LdGRdR4):
		*(void**)&m_regs.valueRegister = (void*)asBC_PTRARG(l_bc);
		*(l_fp - asBC_SWORDARG0(l_bc)) = **(asDWORD**)&m_regs.valueRegister;
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

//----------------
// path control instructions

	// Begin execution of a script function
	INSTRUCTION(asBC_CALL):
		{
			int i = asBC_INTARG(l_bc);
			l_bc += 2;

			asASSERT( i >= 0 );
			asASSERT( (i & FUNC_IMPORTED) == 0 );

			// Need to move the values back to the context
			m_regs.programPointer = l_bc;
			m_regs.stackPointer = l_sp;
			m_regs.stackFramePointer = l_fp;

			CallScriptFunction(m_engine->scriptFunctions[i]);

			// Extract the values from the context again
			l_bc = m_regs.programPointer;
			l_sp = m_regs.stackPointer;
			l_fp = m_regs.stackFramePointer;

			// If status isn't active anymore then we must stop
			if( m_status != asEXECUTION_ACTIVE )
				return;
		}
		NEXT_INSTRUCTION();

	// Return to the caller, and remove the arguments from the stack
	INSTRUCTION(asBC_RET):
		{
			// Return if this was the first function, or a nested execution
			if( m_callStack.GetLength() == 0 ||
				m_callStack[m_callStack.GetLength() - CALLSTACK_FRAME_SIZE] == 0 )
			{
				m_status = asEXECUTION_FINISHED;
				return;
			}

			asWORD w = asBC_WORDARG0(l_bc);

			// Read the old framepointer, functionid, and programCounter from the call stack
			PopCallState();

			// Extract the values from the context again
			l_bc = m_regs.programPointer;
			l_sp = m_regs.stackPointer;
			l_fp = m_regs.stackFramePointer;

			// Pop arguments from stack
			l_sp += w;
		}
		NEXT_INSTRUCTION();

	// Jump to a relative position
	INSTRUCTION(asBC_JMP):
		l_bc += 2 + asBC_INTARG(l_bc);
		NEXT_INSTRUCTION();

//----------------
// Conditional jumps

	// Jump to a relative position if the value in the register is 0
	INSTRUCTION(asBC_JZ):
		if( *(int*)&m_regs.valueRegister == 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	// Jump to a relative position if the value in the register is not 0
	INSTRUCTION(asBC_JNZ):
		if( *(int*)&m_regs.valueRegister != 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	// Jump to a relative position if the value in the register is negative
	INSTRUCTION(asBC_JS):
		if( *(int*)&m_regs.valueRegister < 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	// Jump to a relative position if the value in the register it not negative
	INSTRUCTION(asBC_JNS):
		if( *(int*)&m_regs.valueRegister >= 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	// Jump to a relative position if the value in the register is greater than 0
	INSTRUCTION(asBC_JP):
		if( *(int*)&m_regs.valueRegister > 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	// Jump to a relative position if the value in the register is not greater than 0
	INSTRUCTION(asBC_JNP):
		if( *(int*)&m_regs.valueRegister <= 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();
//--------------------
// test instructions

	// If the value in the register is 0, then set the register to 1, else to 0
	INSTRUCTION(asBC_TZ):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is equal to 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on valueRegister.

			volatile int    *regPtr  = (int*)&m_regs.valueRegister;
			volatile asBYTE *regBptr = (asBYTE*)&m_regs.valueRegister;
			asBYTE val = (regPtr[0] == 0) ? VALUE_OF_BOOLEAN_TRUE : 0;
			regBptr[0] = val; // The result is stored in the lower byte
			regBptr[1] = 0;   // Make sure the rest of the register is 0
			regBptr[2] = 0;
			regBptr[3] = 0;
			regBptr[4] = 0;
			regBptr[5] = 0;
			regBptr[6] = 0;
			regBptr[7] = 0;
		}
#else
		*(int*)&m_regs.valueRegister = (*(int*)&m_regs.valueRegister == 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	// If the value in the register is not 0, then set the register to 1, else to 0
	INSTRUCTION(asBC_TNZ):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is not equal to 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on valueRegister.

			volatile int    *regPtr  = (int*)&m_regs.valueRegister;
			volatile asBYTE *regBptr = (asBYTE*)&m_regs.valueRegister;
			asBYTE val = (regPtr[0] == 0) ? 0 : VALUE_OF_BOOLEAN_TRUE;
			regBptr[0] = val; // The result is stored in the lower byte
			regBptr[1] = 0;   // Make sure the rest of the register is 0
			regBptr[2] = 0;
			regBptr[3] = 0;
			regBptr[4] = 0;
			regBptr[5] = 0;
			regBptr[6] = 0;
			regBptr[7] = 0;
		}
#else
		*(int*)&m_regs.valueRegister = (*(int*)&m_regs.valueRegister == 0 ? 0 : VALUE_OF_BOOLEAN_TRUE);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	// If the value in the register is negative, then set the register to 1, else to 0
	INSTRUCTION(asBC_TS):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is less than 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on valueRegister.

			volatile int    *regPtr  = (int*)&m_regs.valueRegister;
			volatile asBYTE *regBptr = (asBYTE*)&m_regs.valueRegister;
			asBYTE val = (regPtr[0] < 0) ? VALUE_OF_BOOLEAN_TRUE : 0;
			regBptr[0] = val; // The result is stored in the lower byte
			regBptr[1] = 0;   // Make sure the rest of the register is 0
			regBptr[2] = 0;
			regBptr[3] = 0;
			regBptr[4] = 0;
			regBptr[5] = 0;
			regBptr[6] = 0;
			regBptr[7] = 0;
		}
#else
		*(int*)&m_regs.valueRegister = (*(int*)&m_regs.valueRegister < 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	// If the value in the register is not negative, then set the register to 1, else to 0
	INSTRUCTION(asBC_TNS):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is not less than 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on valueRegister.

			volatile int    *regPtr  = (int*)&m_regs.valueRegister;
			volatile asBYTE *regBptr = (asBYTE*)&m_regs.valueRegister;
			asBYTE val = (regPtr[0] >= 0) ? VALUE_OF_BOOLEAN_TRUE : 0;
			regBptr[0] = val; // The result is stored in the lower byte
			regBptr[1] = 0;   // Make sure the rest of the register is 0
			regBptr[2] = 0;
			regBptr[3] = 0;
			regBptr[4] = 0;
			regBptr[5] = 0;
			regBptr[6] = 0;
			regBptr[7] = 0;
		}
#else
		*(int*)&m_regs.valueRegister = (*(int*)&m_regs.valueRegister < 0 ? 0 : VALUE_OF_BOOLEAN_TRUE);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	// If the value in the register is greater than 0, then set the register to 1, else to 0
	INSTRUCTION(asBC_TP):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is greater than 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on valueRegister.

			volatile int    *regPtr  = (int*)&m_regs.valueRegister;
			volatile asBYTE *regBptr = (asBYTE*)&m_regs.valueRegister;
			asBYTE val = (regPtr[0] > 0) ? VALUE_OF_BOOLEAN_TRUE : 0;
			regBptr[0] = val; // The result is stored in the lower byte
			regBptr[1] = 0;   // Make sure the rest of the register is 0
			regBptr[2] = 0;
			regBptr[3] = 0;
			regBptr[4] = 0;
			regBptr[5] = 0;
			regBptr[6] = 0;
			regBptr[7] = 0;
		}
#else
		*(int*)&m_regs.valueRegister = (*(int*)&m_regs.valueRegister > 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	// If the value in the register is not greater than 0, then set the register to 1, else to 0
	INSTRUCTION(asBC_TNP):
#if AS_SIZEOF_BOOL == 1
		{
			// Set the value to true if it is not greater than 0

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on valueRegister.

			volatile int    *regPtr  = (int*)&m_regs.valueRegister;
			volatile asBYTE *regBptr = (asBYTE*)&m_regs.valueRegister;
			asBYTE val = (regPtr[0] <= 0) ? VALUE_OF_BOOLEAN_TRUE : 0;
			regBptr[0] = val; // The result is stored in the lower byte
			regBptr[1] = 0;   // Make sure the rest of the register is 0
			regBptr[2] = 0;
			regBptr[3] = 0;
			regBptr[4] = 0;
			regBptr[5] = 0;
			regBptr[6] = 0;
			regBptr[7] = 0;
		}
#else
		*(int*)&m_regs.valueRegister = (*(int*)&m_regs.valueRegister > 0 ? 0 : VALUE_OF_BOOLEAN_TRUE);
#endif
		l_bc++;
		NEXT_INSTRUCTION();

//--------------------
// negate value

	// Negate the integer value in the variable
	INSTRUCTION(asBC_NEGi):
		*(l_fp - asBC_SWORDARG0(l_bc)) = asDWORD(-int(*(l_fp - asBC_SWORDARG0(l_bc))));
		l_bc++;
		NEXT_INSTRUCTION();

	// Negate the float value in the variable
	INSTRUCTION(asBC_NEGf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = -*(float*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	// Negate the double value in the variable
	INSTRUCTION(asBC_NEGd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = -*(double*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

//-------------------------
// Increment value pointed to by address in register

	// Increment the short value pointed to by the register
	INSTRUCTION(asBC_INCi16):
		(**(short**)&m_regs.valueRegister)++;
		l_bc++;
		NEXT_INSTRUCTION();

	// Increment the byte value pointed to by the register
	INSTRUCTION(asBC_INCi8):
		(**(char**)&m_regs.valueRegister)++;
		l_bc++;
		NEXT_INSTRUCTION();

	// Decrement the short value pointed to by the register
	INSTRUCTION(asBC_DECi16):
		(**(short**)&m_regs.valueRegister)--;
		l_bc++;
		NEXT_INSTRUCTION();

	// Decrement the byte value pointed to by the register
	INSTRUCTION(asBC_DECi8):
		(**(char**)&m_regs.valueRegister)--;
		l_bc++;
		NEXT_INSTRUCTION();

	// Increment the integer value pointed to by the register
	INSTRUCTION(asBC_INCi):
		++(**(int**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	// Decrement the integer value pointed to by the register
	INSTRUCTION(asBC_DECi):
		--(**(int**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	// Increment the float value pointed to by the register
	INSTRUCTION(asBC_INCf):
		++(**(float**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	// Decrement the float value pointed to by the register
	INSTRUCTION(asBC_DECf):
		--(**(float**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	// Increment the double value pointed to by the register
	INSTRUCTION(asBC_INCd):
		++(**(double**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	// Decrement the double value pointed to by the register
	INSTRUCTION(asBC_DECd):
		--(**(double**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	// Increment the local integer variable
	INSTRUCTION(asBC_IncVi):
		(*(int*)(l_fp - asBC_SWORDARG0(l_bc)))++;
		l_bc++;
		NEXT_INSTRUCTION();

	// Decrement the local integer variable
	INSTRUCTION(asBC_DecVi):
		(*(int*)(l_fp - asBC_SWORDARG0(l_bc)))--;
		l_bc++;
		NEXT_INSTRUCTION();

//--------------------
// bits instructions

	// Do a bitwise not on the value in the variable
	INSTRUCTION(asBC_BNOT):
		*(l_fp - asBC_SWORDARG0(l_bc)) = ~*(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	// Do a bitwise and of two variables and store the result in a third variable
	INSTRUCTION(asBC_BAND):
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(l_fp - asBC_SWORDARG1(l_bc)) & *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	// Do a bitwise or of two variables and store the result in a third variable
	INSTRUCTION(asBC_BOR):
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(l_fp - asBC_SWORDARG1(l_bc)) | *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	// Do a bitwise xor of two variables and store the result in a third variable
	INSTRUCTION(asBC_BXOR):
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(l_fp - asBC_SWORDARG1(l_bc)) ^ *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	// Do a logical shift left of two variables and store the result in a third variable
	INSTRUCTION(asBC_BSLL):
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(l_fp - asBC_SWORDARG1(l_bc)) << *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	// Do a logical shift right of two variables and store the result in a third variable
	INSTRUCTION(asBC_BSRL):
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(l_fp - asBC_SWORDARG1(l_bc)) >> *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	// Do an arithmetic shift right of two variables and store the result in a third variable
	INSTRUCTION(asBC_BSRA):
		*(l_fp - asBC_SWORDARG0(l_bc)) = int(*(l_fp - asBC_SWORDARG1(l_bc))) >> *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_COPY):
		{
			void *d = (void*)*(asPWORD*)l_sp; l_sp += AS_PTR_SIZE;
			void *s = (void*)*(asPWORD*)l_sp;
			if( s == 0 || d == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
			memcpy(d, s, asBC_WORDARG0(l_bc)*4);

			// replace the pointer on the stack with the lvalue
			*(asPWORD**)l_sp = (asPWORD*)d;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PshC8):
		l_sp -= 2;
		*(asQWORD*)l_sp = asBC_QWORDARG(l_bc);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PshVPtr):
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = *(asPWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_RDSPtr):
		{
			// The pointer must not be null
			asPWORD a = *(asPWORD*)l_sp;
			if( a == 0 )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
			// Pop an address from the stack, read a pointer from that address and push it on the stack
			*(asPWORD*)l_sp = *(asPWORD*)a;
		}
		l_bc++;
		NEXT_INSTRUCTION();

	//----------------------------
	// Comparisons
	INSTRUCTION(asBC_CMPd):
		{
			// Do a comparison of the values, rather than a subtraction
			// in order to get proper behaviour for infinity values.
			double dbl1 = *(double*)(l_fp - asBC_SWORDARG0(l_bc));
			double dbl2 = *(double*)(l_fp - asBC_SWORDARG1(l_bc));
			if( dbl1 == dbl2 )     *(int*)&m_regs.valueRegister =  0;
			else if( dbl1 < dbl2 ) *(int*)&m_regs.valueRegister = -1;
			else                   *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPu):
		{
			asDWORD d1 = *(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc));
			asDWORD d2 = *(asDWORD*)(l_fp - asBC_SWORDARG1(l_bc));
			if( d1 == d2 )     *(int*)&m_regs.valueRegister =  0;
			else if( d1 < d2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPf):
		{
			// Do a comparison of the values, rather than a subtraction
			// in order to get proper behaviour for infinity values.
			float f1 = *(float*)(l_fp - asBC_SWORDARG0(l_bc));
			float f2 = *(float*)(l_fp - asBC_SWORDARG1(l_bc));
			if( f1 == f2 )     *(int*)&m_regs.valueRegister =  0;
			else if( f1 < f2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPi):
		{
			int i1 = *(int*)(l_fp - asBC_SWORDARG0(l_bc));
			int i2 = *(int*)(l_fp - asBC_SWORDARG1(l_bc));
			if( i1 == i2 )     *(int*)&m_regs.valueRegister =  0;
			else if( i1 < i2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	//----------------------------
	// Comparisons with constant value
	INSTRUCTION(asBC_CMPIi):
		{
			int i1 = *(int*)(l_fp - asBC_SWORDARG0(l_bc));
			int i2 = asBC_INTARG(l_bc);
			if( i1 == i2 )     *(int*)&m_regs.valueRegister =  0;
			else if( i1 < i2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPIf):
		{
			// Do a comparison of the values, rather than a subtraction
			// in order to get proper behaviour for infinity values.
			float f1 = *(float*)(l_fp - asBC_SWORDARG0(l_bc));
			float f2 = asBC_FLOATARG(l_bc);
			if( f1 == f2 )     *(int*)&m_regs.valueRegister =  0;
			else if( f1 < f2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPIu):
		{
			asDWORD d1 = *(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc));
			asDWORD d2 = asBC_DWORDARG(l_bc);
			if( d1 == d2 )     *(int*)&m_regs.valueRegister =  0;
			else if( d1 < d2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_JMPP):
		l_bc += 1 + (*(int*)(l_fp - asBC_SWORDARG0(l_bc)))*2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PopRPtr):
		*(asPWORD*)&m_regs.valueRegister = *(asPWORD*)l_sp;
		l_sp += AS_PTR_SIZE;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PshRPtr):
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = *(asPWORD*)&m_regs.valueRegister;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_STR):
		// TODO: NEWSTRING: Deprecate this instruction
		asASSERT(false);
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CALLSYS):
		{
			// Get function ID from the argument
			int i = asBC_INTARG(l_bc);

			// Need to move the values back to the context as the called functions
			// may use the debug interface to inspect the registers
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			l_sp += CallSystemFunction(i, this);

			// Update the program position after the call so that line number is correct
			l_bc += 2;

			if( m_regs.doProcessSuspend )
			{
				// Should the execution be suspended?
				if( m_doSuspend )
				{
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					m_status = asEXECUTION_SUSPENDED;
					return;
				}
				// An exception might have been raised
				if( m_status != asEXECUTION_ACTIVE )
				{
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					return;
				}
			}
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CALLBND):
		{
			// TODO: Clean-up: This code is very similar to asBC_CallPtr. Create a shared method for them
			// Get the function ID from the stack
			int i = asBC_INTARG(l_bc);

			asASSERT( i >= 0 );
			asASSERT( i & FUNC_IMPORTED );

			// Need to move the values back to the context
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			int funcId = m_engine->importedFunctions[i & ~FUNC_IMPORTED]->boundFunctionId;
			if( funcId == -1 )
			{
				// Need to update the program pointer for the exception handler
				m_regs.programPointer += 2;

				// Tell the exception handler to clean up the arguments to this function
				m_needToCleanupArgs = true;
				SetInternalException(TXT_UNBOUND_FUNCTION);
				return;
			}
			else
			{
				asCScriptFunction *func = m_engine->GetScriptFunction(funcId);
				if( func->funcType == asFUNC_SCRIPT )
				{
					m_regs.programPointer += 2;
					CallScriptFunction(func);
				}
				else if( func->funcType == asFUNC_SYSTEM )
				{
					m_regs.stackPointer += CallSystemFunction(func->id, this);

					// Update program position after the call so the line number
					// is correct in case the system function queries it
					m_regs.programPointer += 2;
				}
				else
				{
					asASSERT(func->funcType == asFUNC_DELEGATE);

					// Delegates cannot be bound to imported functions as the delegates do not have a function id
					asASSERT(false);
				}
			}

			// Extract the values from the context again
			l_bc = m_regs.programPointer;
			l_sp = m_regs.stackPointer;
			l_fp = m_regs.stackFramePointer;

			// If status isn't active anymore then we must stop
			if( m_status != asEXECUTION_ACTIVE )
				return;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUSPEND):
		if( m_regs.doProcessSuspend )
		{
			if( m_lineCallback )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				CallLineCallback();
			}
			if( m_doSuspend )
			{
				l_bc++;

				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				m_status = asEXECUTION_SUSPENDED;
				return;
			}
		}

		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ALLOC):
		{
			asCObjectType *objType = (asCObjectType*)asBC_PTRARG(l_bc);
			int func = asBC_INTARG(l_bc+AS_PTR_SIZE);

			if( objType->flags & asOBJ_SCRIPT_OBJECT )
			{
				// Need to move the values back to the context as the construction
				// of the script object may reuse the context for nested calls.
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Pre-allocate the memory
				asDWORD *mem = (asDWORD*)m_engine->CallAlloc(objType);

				// Pre-initialize the memory by calling the constructor for asCScriptObject
				ScriptObject_Construct(objType, (asCScriptObject*)mem);

				// Call the constructor to initalize the memory
				asCScriptFunction *f = m_engine->scriptFunctions[func];

				asDWORD **a = (asDWORD**)*(asPWORD*)(m_regs.stackPointer + f->GetSpaceNeededForArguments());
				if( a ) *a = mem;

				// Push the object pointer on the stack
				m_regs.stackPointer -= AS_PTR_SIZE;
				*(asPWORD*)m_regs.stackPointer = (asPWORD)mem;

				m_regs.programPointer += 2+AS_PTR_SIZE;

				CallScriptFunction(f);

				// Extract the values from the context again
				l_bc = m_regs.programPointer;
				l_sp = m_regs.stackPointer;
				l_fp = m_regs.stackFramePointer;

				// If status isn't active anymore then we must stop
				if( m_status != asEXECUTION_ACTIVE )
					return;
			}
			else
			{
				// Pre-allocate the memory
				asDWORD *mem = (asDWORD*)m_engine->CallAlloc(objType);

				if( func )
				{
					// Push the object pointer on the stack (it will be popped by the function)
					l_sp -= AS_PTR_SIZE;
					*(asPWORD*)l_sp = (asPWORD)mem;

					// Need to move the values back to the context as the called functions
					// may use the debug interface to inspect the registers
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					l_sp += CallSystemFunction(func, this);
				}

				// Pop the variable address from the stack
				asDWORD **a = (asDWORD**)*(asPWORD*)l_sp;
				l_sp += AS_PTR_SIZE;
				if( a ) *a = mem;

				l_bc += 2+AS_PTR_SIZE;

				if( m_regs.doProcessSuspend )
				{
					// Should the execution be suspended?
					if( m_doSuspend )
					{
						m_regs.programPointer    = l_bc;
						m_regs.stackPointer      = l_sp;
						m_regs.stackFramePointer = l_fp;

						m_status = asEXECUTION_SUSPENDED;
						return;
					}
					// An exception might have been raised
					if( m_status != asEXECUTION_ACTIVE )
					{
						m_regs.programPointer    = l_bc;
						m_regs.stackPointer      = l_sp;
						m_regs.stackFramePointer = l_fp;

						m_engine->CallFree(mem);
						*a = 0;

						return;
					}
				}
			}
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_FREE):
		{
			// Get the variable that holds the object handle/reference
			asPWORD *a = (asPWORD*)asPWORD(l_fp - asBC_SWORDARG0(l_bc));
			if( *a )
			{
				asCObjectType *objType = (asCObjectType*)asBC_PTRARG(l_bc);
				asSTypeBehaviour *beh = &objType->beh;

				// Need to move the values back to the context as the called functions
				// may use the debug interface to inspect the registers
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				if( objType->flags & asOBJ_REF )
				{
					asASSERT( (objType->flags & asOBJ_NOCOUNT) || beh->release );
					if( beh->release )
						m_engine->CallObjectMethod((void*)(asPWORD)*a, beh->release);
				}
				else
				{
					if( beh->destruct )
						m_engine->CallObjectMethod((void*)(asPWORD)*a, beh->destruct);
					else if( objType->flags & asOBJ_LIST_PATTERN )
						m_engine->DestroyList((asBYTE*)(asPWORD)*a, objType);

					m_engine->CallFree((void*)(asPWORD)*a);
				}

				// Clear the variable
				*a = 0;
			}
		}
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_LOADOBJ):
		{
			// Move the object pointer from the object variable into the object register
			void **a = (void**)(l_fp - asBC_SWORDARG0(l_bc));
			m_regs.objectType = 0;
			m_regs.objectRegister = *a;
			*a = 0;
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_STOREOBJ):
		// Move the object pointer from the object register to the object variable
		*(asPWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = asPWORD(m_regs.objectRegister);
		m_regs.objectRegister = 0;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_GETOBJ):
		{
			// Read variable index from location on stack
			asPWORD *a = (asPWORD*)(l_sp + asBC_WORDARG0(l_bc));
			asPWORD offset = *a;
			// Move pointer from variable to the same location on the stack
			asPWORD *v = (asPWORD*)(l_fp - offset);
			*a = *v;
			// Clear variable
			*v = 0;
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_REFCPY):
		{
			asCObjectType *objType = (asCObjectType*)asBC_PTRARG(l_bc);
			asSTypeBehaviour *beh = &objType->beh;

			// Pop address of destination pointer from the stack
			void **d = (void**)*(asPWORD*)l_sp;
			l_sp += AS_PTR_SIZE;

			// Read wanted pointer from the stack
			void *s = (void*)*(asPWORD*)l_sp;

			// Need to move the values back to the context as the called functions
			// may use the debug interface to inspect the registers
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			// Update ref counter for object types that require it
			if( !(objType->flags & (asOBJ_NOCOUNT | asOBJ_VALUE)) )
			{
				// Release previous object held by destination pointer
				if( *d != 0 && beh->release )
					m_engine->CallObjectMethod(*d, beh->release);
				// Increase ref counter of wanted object
				if( s != 0 && beh->addref )
					m_engine->CallObjectMethod(s, beh->addref);
			}

			// Set the new object in the destination
			*d = s;
		}
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CHKREF):
		{
			// Verify if the pointer on the stack is null
			// This is used when validating a pointer that an operator will work on
			asPWORD a = *(asPWORD*)l_sp;
			if( a == 0 )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_GETOBJREF):
		{
			// Get the location on the stack where the reference will be placed
			asPWORD *a = (asPWORD*)(l_sp + asBC_WORDARG0(l_bc));

			// Replace the variable index with the object handle held in the variable
			*(asPWORD**)a = *(asPWORD**)(l_fp - *a);
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_GETREF):
		{
			// Get the location on the stack where the reference will be placed
			asPWORD *a = (asPWORD*)(l_sp + asBC_WORDARG0(l_bc));

			// Replace the variable index with the address of the variable
			*(asPWORD**)a = (asPWORD*)(l_fp - (int)*a);
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PshNull):
		// Push a null pointer on the stack
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = 0;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ClrVPtr):
		// TODO: runtime optimize: Is this instruction really necessary?
		//                         CallScriptFunction() can clear the null handles upon entry, just as is done for
		//                         all other object variables
		// Clear pointer variable
		*(asPWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = 0;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_OBJTYPE):
		// Push the object type on the stack
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = asBC_PTRARG(l_bc);
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_TYPEID):
		// Equivalent to PshC4, but kept as separate instruction for bytecode serialization
		--l_sp;
		*l_sp = asBC_DWORDARG(l_bc);
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SetV4):
		*(l_fp - asBC_SWORDARG0(l_bc)) = asBC_DWORDARG(l_bc);
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SetV8):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = asBC_QWORDARG(l_bc);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ADDSi):
		{
			// The pointer must not be null
			asPWORD a = *(asPWORD*)l_sp;
			if( a == 0 )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
			// Add an offset to the pointer
			*(asPWORD*)l_sp = a + asBC_SWORDARG0(l_bc);
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyVtoV4):
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(l_fp - asBC_SWORDARG1(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyVtoV8):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyVtoR4):
		*(asDWORD*)&m_regs.valueRegister = *(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyVtoR8):
		*(asQWORD*)&m_regs.valueRegister = *(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyVtoG4):
		*(asDWORD*)asBC_PTRARG(l_bc) = *(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc += 1 + AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyRtoV4):
		*(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asDWORD*)&m_regs.valueRegister;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyRtoV8):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = m_regs.valueRegister;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CpyGtoV4):
		*(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asDWORD*)asBC_PTRARG(l_bc);
		l_bc += 1 + AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_WRTV1):
		// The pointer in the register points to a byte, and *(l_fp - offset) too
		**(asBYTE**)&m_regs.valueRegister = *(asBYTE*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_WRTV2):
		// The pointer in the register points to a word, and *(l_fp - offset) too
		**(asWORD**)&m_regs.valueRegister = *(asWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_WRTV4):
		**(asDWORD**)&m_regs.valueRegister = *(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_WRTV8):
		**(asQWORD**)&m_regs.valueRegister = *(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_RDR1):
		{
			// The pointer in the register points to a byte, and *(l_fp - offset) will also point to a byte
			asBYTE *bPtr = (asBYTE*)(l_fp - asBC_SWORDARG0(l_bc));
			bPtr[0] = **(asBYTE**)&m_regs.valueRegister; // read the byte
			bPtr[1] = 0;                      // 0 the rest of the DWORD
			bPtr[2] = 0;
			bPtr[3] = 0;
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_RDR2):
		{
			// The pointer in the register points to a word, and *(l_fp - offset) will also point to a word
			asWORD *wPtr = (asWORD*)(l_fp - asBC_SWORDARG0(l_bc));
			wPtr[0] = **(asWORD**)&m_regs.valueRegister; // read the word
			wPtr[1] = 0;                      // 0 the rest of the DWORD
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_RDR4):
		*(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = **(asDWORD**)&m_regs.valueRegister;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_RDR8):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = **(asQWORD**)&m_regs.valueRegister;
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_LDG):
		*(asPWORD*)&m_regs.valueRegister = asBC_PTRARG(l_bc);
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_LDV):
		*(asDWORD**)&m_regs.valueRegister = (l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PGA):
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = asBC_PTRARG(l_bc);
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CmpPtr):
		{
			// TODO: runtime optimize: This instruction should really just be an equals, and return true or false.
			//                         The instruction is only used for is and !is tests anyway.
			asPWORD p1 = *(asPWORD*)(l_fp - asBC_SWORDARG0(l_bc));
			asPWORD p2 = *(asPWORD*)(l_fp - asBC_SWORDARG1(l_bc));
			if( p1 == p2 )     *(int*)&m_regs.valueRegister =  0;
			else if( p1 < p2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_VAR):
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = (asPWORD)asBC_SWORDARG0(l_bc);
		l_bc++;
		NEXT_INSTRUCTION();

	//----------------------------
	// Type conversions
	INSTRUCTION(asBC_iTOf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = float(*(int*)(l_fp - asBC_SWORDARG0(l_bc)));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_fTOi):
		*(l_fp - asBC_SWORDARG0(l_bc)) = int(*(float*)(l_fp - asBC_SWORDARG0(l_bc)));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_uTOf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = float(*(l_fp - asBC_SWORDARG0(l_bc)));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_fTOu):
		// We must cast to int first, because on some compilers the cast of a negative float value to uint result in 0
		*(l_fp - asBC_SWORDARG0(l_bc)) = asUINT(int(*(float*)(l_fp - asBC_SWORDARG0(l_bc))));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_sbTOi):
		// *(l_fp - offset) points to a char, and will point to an int afterwards
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(signed char*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_swTOi):
		// *(l_fp - offset) points to a short, and will point to an int afterwards
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(short*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ubTOi):
		// (l_fp - offset) points to a byte, and will point to an int afterwards
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(asBYTE*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_uwTOi):
		// *(l_fp - offset) points to a word, and will point to an int afterwards
		*(l_fp - asBC_SWORDARG0(l_bc)) = *(asWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_dTOi):
		*(l_fp - asBC_SWORDARG0(l_bc)) = int(*(double*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_dTOu):
		// We must cast to int first, because on some compilers the cast of a negative float value to uint result in 0
		*(l_fp - asBC_SWORDARG0(l_bc)) = asUINT(int(*(double*)(l_fp - asBC_SWORDARG1(l_bc))));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_dTOf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = float(*(double*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_iTOd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = double(*(int*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_uTOd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = double(*(asUINT*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_fTOd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = double(*(float*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	//------------------------------
	// Math operations
	INSTRUCTION(asBC_ADDi):
		*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) + *(int*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUBi):
		*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) - *(int*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MULi):
		*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) * *(int*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DIVi):
		{
			int divider = *(int*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			else if( divider == -1 )
			{
				// Need to check if the value that is divided is 0x80000000
				// as dividing it with -1 will cause an overflow exception
				if( *(int*)(l_fp - asBC_SWORDARG1(l_bc)) == int(0x80000000) )
				{
					// Need to move the values back to the context
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					// Raise exception
					SetInternalException(TXT_DIVIDE_OVERFLOW);
					return;
				}
			}
			*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) / divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MODi):
		{
			int divider = *(int*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			else if( divider == -1 )
			{
				// Need to check if the value that is divided is 0x80000000
				// as dividing it with -1 will cause an overflow exception
				if( *(int*)(l_fp - asBC_SWORDARG1(l_bc)) == int(0x80000000) )
				{
					// Need to move the values back to the context
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					// Raise exception
					SetInternalException(TXT_DIVIDE_OVERFLOW);
					return;
				}
			}
			*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) % divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ADDf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) + *(float*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUBf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) - *(float*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MULf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) * *(float*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DIVf):
		{
			float divider = *(float*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) / divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MODf):
		{
			float divider = *(float*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = fmodf(*(float*)(l_fp - asBC_SWORDARG1(l_bc)), divider);
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ADDd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = *(double*)(l_fp - asBC_SWORDARG1(l_bc)) + *(double*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUBd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = *(double*)(l_fp - asBC_SWORDARG1(l_bc)) - *(double*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MULd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = *(double*)(l_fp - asBC_SWORDARG1(l_bc)) * *(double*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DIVd):
		{
			double divider = *(double*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}

			*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = *(double*)(l_fp - asBC_SWORDARG1(l_bc)) / divider;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MODd):
		{
			double divider = *(double*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}

			*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = fmod(*(double*)(l_fp - asBC_SWORDARG1(l_bc)), divider);
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	//------------------------------
	// Math operations with constant value
	INSTRUCTION(asBC_ADDIi):
		*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) + asBC_INTARG(l_bc+1);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUBIi):
		*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) - asBC_INTARG(l_bc+1);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MULIi):
		*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = *(int*)(l_fp - asBC_SWORDARG1(l_bc)) * asBC_INTARG(l_bc+1);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ADDIf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) + asBC_FLOATARG(l_bc+1);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUBIf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) - asBC_FLOATARG(l_bc+1);
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MULIf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = *(float*)(l_fp - asBC_SWORDARG1(l_bc)) * asBC_FLOATARG(l_bc+1);
		l_bc += 3;
		NEXT_INSTRUCTION();

	//-----------------------------------
	INSTRUCTION(asBC_SetG4):
		*(asDWORD*)asBC_PTRARG(l_bc) = asBC_DWORDARG(l_bc+AS_PTR_SIZE);
		l_bc += 2 + AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ChkRefS):
		{
			// Verify if the pointer on the stack refers to a non-null value
			// This is used to validate a reference to a handle
			asPWORD *a = (asPWORD*)*(asPWORD*)l_sp;
			if( *a == 0 )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ChkNullV):
		{
			// Verify if variable (on the stack) is not null
			asDWORD *a = *(asDWORD**)(l_fp - asBC_SWORDARG0(l_bc));
			if( a == 0 )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CALLINTF):
		{
			int i = asBC_INTARG(l_bc);
			l_bc += 2;

			asASSERT( i >= 0 );
			asASSERT( (i & FUNC_IMPORTED) == 0 );

			// Need to move the values back to the context
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			CallInterfaceMethod(m_engine->GetScriptFunction(i));

			// Extract the values from the context again
			l_bc = m_regs.programPointer;
			l_sp = m_regs.stackPointer;
			l_fp = m_regs.stackFramePointer;

			// If status isn't active anymore then we must stop
			if( m_status != asEXECUTION_ACTIVE )
				return;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_iTOb):
		{
			// *(l_fp - offset) points to an int, and will point to a byte afterwards

			// We need to use volatile here to tell the compiler not to rearrange
			// read and write operations during optimizations.
			volatile asDWORD val  = *(l_fp - asBC_SWORDARG0(l_bc));
			volatile asBYTE *bPtr = (asBYTE*)(l_fp - asBC_SWORDARG0(l_bc));
			bPtr[0] = (asBYTE)val; // write the byte
			bPtr[1] = 0;           // 0 the rest of the DWORD
			bPtr[2] = 0;
			bPtr[3] = 0;
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_iTOw):
		{
			// *(l_fp - offset) points to an int, and will point to word afterwards

			// We need to use volatile here to tell the compiler not to rearrange
			// read and write operations during optimizations.
			volatile asDWORD val  = *(l_fp - asBC_SWORDARG0(l_bc));
			volatile asWORD *wPtr = (asWORD*)(l_fp - asBC_SWORDARG0(l_bc));
			wPtr[0] = (asWORD)val; // write the word
			wPtr[1] = 0;           // 0 the rest of the DWORD
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SetV1):
		// TODO: This is exactly the same as SetV4. This is a left over from the time
		//       when the bytecode instructions were more tightly packed. It can now
		//       be removed. When removing it, make sure the value is correctly converted
		//       on big-endian CPUs.

		// The byte is already stored correctly in the argument
		*(l_fp - asBC_SWORDARG0(l_bc)) = asBC_DWORDARG(l_bc);
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SetV2):
		// TODO: This is exactly the same as SetV4. This is a left over from the time
		//       when the bytecode instructions were more tightly packed. It can now
		//       be removed. When removing it, make sure the value is correctly converted
		//       on big-endian CPUs.

		// The word is already stored correctly in the argument
		*(l_fp - asBC_SWORDARG0(l_bc)) = asBC_DWORDARG(l_bc);
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_Cast):
		// Cast the handle at the top of the stack to the type in the argument
		{
			asDWORD **a = (asDWORD**)*(asPWORD*)l_sp;
			if( a && *a )
			{
				asDWORD typeId = asBC_DWORDARG(l_bc);

				asCScriptObject *obj = (asCScriptObject *)* a;
				asCObjectType *objType = obj->objType;
				asCObjectType *to = m_engine->GetObjectTypeFromTypeId(typeId);

				// This instruction can only be used with script classes and interfaces
				asASSERT( objType->flags & asOBJ_SCRIPT_OBJECT );
				asASSERT( to->flags & asOBJ_SCRIPT_OBJECT );

				if( objType->Implements(to) || objType->DerivesFrom(to) )
				{
					m_regs.objectType = 0;
					m_regs.objectRegister = obj;
					obj->AddRef();
				}
				else
				{
					// The object register should already be null, so there
					// is no need to clear it if the cast is unsuccessful
					asASSERT( m_regs.objectRegister == 0 );
				}
			}
			l_sp += AS_PTR_SIZE;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_i64TOi):
		*(l_fp - asBC_SWORDARG0(l_bc)) = int(*(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_uTOi64):
		*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = asINT64(*(asUINT*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_iTOi64):
		*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = asINT64(*(int*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_fTOi64):
		*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = asINT64(*(float*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_dTOi64):
		*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = asINT64(*(double*)(l_fp - asBC_SWORDARG0(l_bc)));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_fTOu64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = asQWORD(asINT64(*(float*)(l_fp - asBC_SWORDARG1(l_bc))));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_dTOu64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = asQWORD(asINT64(*(double*)(l_fp - asBC_SWORDARG0(l_bc))));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_i64TOf):
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = float(*(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_u64TOf):
#if defined(_MSC_VER) && _MSC_VER <= 1200 // MSVC6
		{
			// MSVC6 doesn't permit UINT64 to double
			asINT64 v = *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc));
			if( v < 0 )
				*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = 18446744073709551615.0f+float(v);
			else
				*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = float(v);
		}
#else
		*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = float(*(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)));
#endif
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_i64TOd):
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = double(*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_u64TOd):
#if defined(_MSC_VER) && _MSC_VER <= 1200 // MSVC6
		{
			// MSVC6 doesn't permit UINT64 to double
			asINT64 v = *(asINT64*)(l_fp - asBC_SWORDARG0(l_bc));
			if( v < 0 )
				*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = 18446744073709551615.0+double(v);
			else
				*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = double(v);
		}
#else
		*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = double(*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)));
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_NEGi64):
		*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = -*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_INCi64):
		++(**(asQWORD**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DECi64):
		--(**(asQWORD**)&m_regs.valueRegister);
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BNOT64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = ~*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ADDi64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) + *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SUBi64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) - *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MULi64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) * *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DIVi64):
		{
			asINT64 divider = *(asINT64*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			else if( divider == -1 )
			{
				// Need to check if the value that is divided is 1<<63
				// as dividing it with -1 will cause an overflow exception
				if( *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)) == (asINT64(1)<<63) )
				{
					// Need to move the values back to the context
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					// Raise exception
					SetInternalException(TXT_DIVIDE_OVERFLOW);
					return;
				}
			}

			*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)) / divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MODi64):
		{
			asINT64 divider = *(asINT64*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			else if( divider == -1 )
			{
				// Need to check if the value that is divided is 1<<63
				// as dividing it with -1 will cause an overflow exception
				if( *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)) == (asINT64(1)<<63) )
				{
					// Need to move the values back to the context
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					// Raise exception
					SetInternalException(TXT_DIVIDE_OVERFLOW);
					return;
				}
			}
			*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)) % divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BAND64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) & *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BOR64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) | *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BXOR64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) ^ *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BSLL64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) << *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BSRL64):
		*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) >> *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_BSRA64):
		*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)) >> *(l_fp - asBC_SWORDARG2(l_bc));
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPi64):
		{
			asINT64 i1 = *(asINT64*)(l_fp - asBC_SWORDARG0(l_bc));
			asINT64 i2 = *(asINT64*)(l_fp - asBC_SWORDARG1(l_bc));
			if( i1 == i2 )     *(int*)&m_regs.valueRegister =  0;
			else if( i1 < i2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CMPu64):
		{
			asQWORD d1 = *(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc));
			asQWORD d2 = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc));
			if( d1 == d2 )     *(int*)&m_regs.valueRegister =  0;
			else if( d1 < d2 ) *(int*)&m_regs.valueRegister = -1;
			else               *(int*)&m_regs.valueRegister =  1;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ChkNullS):
		{
			// Verify if the pointer on the stack is null
			// This is used for example when validating handles passed as function arguments
			asPWORD a = *(asPWORD*)(l_sp + asBC_WORDARG0(l_bc));
			if( a == 0 )
			{
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}
		}
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_ClrHi):
#if AS_SIZEOF_BOOL == 1
		{
			// Clear the upper bytes, so that trash data don't interfere with boolean operations

			// We need to use volatile here to tell the compiler it cannot
			// change the order of read and write operations on the pointer.

			volatile asBYTE *ptr = (asBYTE*)&m_regs.valueRegister;
			ptr[1] = 0;   // The boolean value is stored in the lower byte, so we clear the rest
			ptr[2] = 0;
			ptr[3] = 0;
		}
#else
		// We don't have anything to do here
#endif
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_JitEntry):
		{
			if( m_currentFunction->scriptData->jitFunction )
			{
				asPWORD jitArg = asBC_PTRARG(l_bc);

				if( jitArg )
				{
					// Resume JIT operation
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					(m_currentFunction->scriptData->jitFunction)(&m_regs, jitArg);

					l_bc = m_regs.programPointer;
					l_sp = m_regs.stackPointer;
					l_fp = m_regs.stackFramePointer;

					// If status isn't active anymore then we must stop
					if( m_status != asEXECUTION_ACTIVE )
						return;

					NEXT_INSTRUCTION();
				}
			}

			// Not a JIT resume point, treat as nop
			l_bc += 1+AS_PTR_SIZE;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_CallPtr):
		{
			// Get the function pointer from the local variable
			asCScriptFunction *func = *(asCScriptFunction**)(l_fp - asBC_SWORDARG0(l_bc));

			// Need to move the values back to the context
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			if( func == 0 )
			{
				// Need to update the program pointer anyway for the exception handler
				m_regs.programPointer++;

				// Tell the exception handler to clean up the arguments to this method
				m_needToCleanupArgs = true;

				// TODO: funcdef: Should we have a different exception string?
				SetInternalException(TXT_UNBOUND_FUNCTION);
				return;
			}
			else
			{
				if (func->funcType == asFUNC_SCRIPT)
				{
					m_regs.programPointer++;
					CallScriptFunction(func);
				}
				else if (func->funcType == asFUNC_DELEGATE)
				{
					// Push the object pointer on the stack. There is always a reserved space for this so
					// we don't don't need to worry about overflowing the allocated memory buffer
					asASSERT(m_regs.stackPointer - AS_PTR_SIZE >= m_stackBlocks[m_stackIndex]);
					m_regs.stackPointer -= AS_PTR_SIZE;
					*(asPWORD*)m_regs.stackPointer = asPWORD(func->objForDelegate);

					// Call the delegated method
					if (func->funcForDelegate->funcType == asFUNC_SYSTEM)
					{
						m_regs.stackPointer += CallSystemFunction(func->funcForDelegate->id, this);

						// Update program position after the call so the line number
						// is correct in case the system function queries it
						m_regs.programPointer++;
					}
					else
					{
						m_regs.programPointer++;

						// TODO: run-time optimize: The true method could be figured out when creating the delegate
						CallInterfaceMethod(func->funcForDelegate);
					}
				}
				else if (func->funcType == asFUNC_SYSTEM)
				{
					m_regs.stackPointer += CallSystemFunction(func->id, this);

					// Update program position after the call so the line number
					// is correct in case the system function queries it
					m_regs.programPointer++;
				}
				else if (func->funcType == asFUNC_IMPORTED)
				{
					m_regs.programPointer++;
					int funcId = m_engine->importedFunctions[func->id & ~FUNC_IMPORTED]->boundFunctionId;
					if (funcId > 0)
						CallScriptFunction(m_engine->scriptFunctions[funcId]);
					else
					{
						// Tell the exception handler to clean up the arguments to this method
						m_needToCleanupArgs = true;

						SetInternalException(TXT_UNBOUND_FUNCTION);
					}
				}
				else
				{
					// Should not get here
					asASSERT(false);
				}
			}

			// Extract the values from the context again
			l_bc = m_regs.programPointer;
			l_sp = m_regs.stackPointer;
			l_fp = m_regs.stackFramePointer;

			// If status isn't active anymore then we must stop
			if( m_status != asEXECUTION_ACTIVE )
				return;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_FuncPtr):
		// Push the function pointer on the stack. The pointer is in the argument
		l_sp -= AS_PTR_SIZE;
		*(asPWORD*)l_sp = asBC_PTRARG(l_bc);
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_LoadThisR):
		{
			// PshVPtr 0
			asPWORD tmp = *(asPWORD*)l_fp;

			// Make sure the pointer is not null
			if( tmp == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}

			// ADDSi
			tmp = tmp + asBC_SWORDARG0(l_bc);

			// PopRPtr
			*(asPWORD*)&m_regs.valueRegister = tmp;
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	// Push the qword value of a variable on the stack
	INSTRUCTION(asBC_PshV8):
		l_sp -= 2;
		*(asQWORD*)l_sp = *(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc));
		l_bc++;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DIVu):
		{
			asUINT divider = *(asUINT*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			*(asUINT*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asUINT*)(l_fp - asBC_SWORDARG1(l_bc)) / divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MODu):
		{
			asUINT divider = *(asUINT*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			*(asUINT*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asUINT*)(l_fp - asBC_SWORDARG1(l_bc)) % divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_DIVu64):
		{
			asQWORD divider = *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) / divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_MODu64):
		{
			asQWORD divider = *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc));
			if( divider == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_DIVIDE_BY_ZERO);
				return;
			}
			*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = *(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)) % divider;
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_LoadRObjR):
		{
			// PshVPtr x
			asPWORD tmp = *(asPWORD*)(l_fp - asBC_SWORDARG0(l_bc));

			// Make sure the pointer is not null
			if( tmp == 0 )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_NULL_POINTER_ACCESS);
				return;
			}

			// ADDSi y
			tmp = tmp + asBC_SWORDARG1(l_bc);

			// PopRPtr
			*(asPWORD*)&m_regs.valueRegister = tmp;
			l_bc += 3;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_LoadVObjR):
		{
			// PSF x
			asPWORD tmp = (asPWORD)(l_fp - asBC_SWORDARG0(l_bc));

			// ADDSi y
			tmp = tmp + asBC_SWORDARG1(l_bc);

			// PopRPtr
			*(asPWORD*)&m_regs.valueRegister = tmp;
			l_bc += 3;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_RefCpyV):
		// Same as PSF v, REFCPY
		{
			asCObjectType *objType = (asCObjectType*)asBC_PTRARG(l_bc);
			asSTypeBehaviour *beh = &objType->beh;

			// Determine destination from argument
			void **d = (void**)asPWORD(l_fp - asBC_SWORDARG0(l_bc));

			// Read wanted pointer from the stack
			void *s = (void*)*(asPWORD*)l_sp;

			// Need to move the values back to the context as the called functions
			// may use the debug interface to inspect the registers
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			// Update ref counter for object types that require it
			if( !(objType->flags & (asOBJ_NOCOUNT | asOBJ_VALUE)) )
			{
				// Release previous object held by destination pointer
				if( *d != 0 && beh->release )
					m_engine->CallObjectMethod(*d, beh->release);
				// Increase ref counter of wanted object
				if( s != 0 && beh->addref )
					m_engine->CallObjectMethod(s, beh->addref);
			}

			// Set the new object in the destination
			*d = s;
		}
		l_bc += 1+AS_PTR_SIZE;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_JLowZ):
		if( *(asBYTE*)&m_regs.valueRegister == 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_JLowNZ):
		if( *(asBYTE*)&m_regs.valueRegister != 0 )
			l_bc += asBC_INTARG(l_bc) + 2;
		else
			l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_AllocMem):
		// Allocate a buffer and store the pointer in the local variable
		{
			// TODO: runtime optimize: As the list buffers are going to be short lived, it may be interesting
			//                         to use a memory pool to avoid reallocating the memory all the time

			asUINT size = asBC_DWORDARG(l_bc);
			asBYTE **var = (asBYTE**)(l_fp - asBC_SWORDARG0(l_bc));
#ifndef WIP_16BYTE_ALIGN
			*var = asNEWARRAY(asBYTE, size);
#else
			*var = asNEWARRAYALIGNED(asBYTE, size, MAX_TYPE_ALIGNMENT);
#endif

			// Clear the buffer for the pointers that will be placed in it
			memset(*var, 0, size);
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SetListSize):
		{
			// Set the size element in the buffer
			asBYTE *var = *(asBYTE**)(l_fp - asBC_SWORDARG0(l_bc));
			asUINT off  = asBC_DWORDARG(l_bc);
			asUINT size = asBC_DWORDARG(l_bc+1);

			asASSERT( var );

			*(asUINT*)(var+off) = size;
		}
		l_bc += 3;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_PshListElmnt):
		{
			// Push the pointer to the list element on the stack
			// In essence it does the same as PSF, RDSPtr, ADDSi
			asBYTE *var = *(asBYTE**)(l_fp - asBC_SWORDARG0(l_bc));
			asUINT off = asBC_DWORDARG(l_bc);

			asASSERT( var );

			l_sp -= AS_PTR_SIZE;
			*(asPWORD*)l_sp = asPWORD(var+off);
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_SetListType):
		{
			// Set the type id in the buffer
			asBYTE *var = *(asBYTE**)(l_fp - asBC_SWORDARG0(l_bc));
			asUINT off  = asBC_DWORDARG(l_bc);
			asUINT type = asBC_DWORDARG(l_bc+1);

			asASSERT( var );

			*(asUINT*)(var+off) = type;
		}
		l_bc += 3;
		NEXT_INSTRUCTION();

	//------------------------------
	// Exponent operations
	INSTRUCTION(asBC_POWi):
		{
			bool isOverflow;
			*(int*)(l_fp - asBC_SWORDARG0(l_bc)) = as_powi(*(int*)(l_fp - asBC_SWORDARG1(l_bc)), *(int*)(l_fp - asBC_SWORDARG2(l_bc)), isOverflow);
			if( isOverflow )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_POWu):
		{
			bool isOverflow;
			*(asDWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = as_powu(*(asDWORD*)(l_fp - asBC_SWORDARG1(l_bc)), *(asDWORD*)(l_fp - asBC_SWORDARG2(l_bc)), isOverflow);
			if( isOverflow )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_POWf):
		{
			float r = powf(*(float*)(l_fp - asBC_SWORDARG1(l_bc)), *(float*)(l_fp - asBC_SWORDARG2(l_bc)));
			*(float*)(l_fp - asBC_SWORDARG0(l_bc)) = r;
			if( r == HUGE_VALF || isinf(r) )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_POWd):
		{
			double r = pow(*(double*)(l_fp - asBC_SWORDARG1(l_bc)), *(double*)(l_fp - asBC_SWORDARG2(l_bc)));
			*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = r;
			if( r == HUGE_VAL || isinf(r) )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_POWdi):
		{
			double r = pow(*(double*)(l_fp - asBC_SWORDARG1(l_bc)), *(int*)(l_fp - asBC_SWORDARG2(l_bc)));
			*(double*)(l_fp - asBC_SWORDARG0(l_bc)) = r;
			if( r == HUGE_VAL || isinf(r) )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
			l_bc += 2;
		}
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_POWi64):
		{
			bool isOverflow;
			*(asINT64*)(l_fp - asBC_SWORDARG0(l_bc)) = as_powi64(*(asINT64*)(l_fp - asBC_SWORDARG1(l_bc)), *(asINT64*)(l_fp - asBC_SWORDARG2(l_bc)), isOverflow);
			if( isOverflow )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_POWu64):
		{
			bool isOverflow;
			*(asQWORD*)(l_fp - asBC_SWORDARG0(l_bc)) = as_powu64(*(asQWORD*)(l_fp - asBC_SWORDARG1(l_bc)), *(asQWORD*)(l_fp - asBC_SWORDARG2(l_bc)), isOverflow);
			if( isOverflow )
			{
				// Need to move the values back to the context
				m_regs.programPointer    = l_bc;
				m_regs.stackPointer      = l_sp;
				m_regs.stackFramePointer = l_fp;

				// Raise exception
				SetInternalException(TXT_POW_OVERFLOW);
				return;
			}
		}
		l_bc += 2;
		NEXT_INSTRUCTION();

	INSTRUCTION(asBC_Thiscall1):
		// This instruction is a faster version of asBC_CALLSYS. It is faster because
		// it has much less runtime overhead with determining the calling convention
		// and no dynamic code for loading the parameters. The instruction can only
		// be used to call functions with the following signatures:
		//
		//  type &obj::func(int)
		//  type &obj::func(uint)
		//  void  obj::func(int)
		//  void  obj::func(uint)
		{
			// Get function ID from the argument
			int i = asBC_INTARG(l_bc);

			// Need to move the values back to the context as the called functions
			// may use the debug interface to inspect the registers
			m_regs.programPointer    = l_bc;
			m_regs.stackPointer      = l_sp;
			m_regs.stackFramePointer = l_fp;

			// Pop the thispointer from the stack
			void *obj = *(void**)l_sp;
			if (obj == 0)
				SetInternalException(TXT_NULL_POINTER_ACCESS);
			else
			{
				// Only update the stack pointer if all is OK so the
				// exception handler can properly clean up the stack
				l_sp += AS_PTR_SIZE;

				// Pop the int arg from the stack
				int arg = *(int*)l_sp;
				l_sp++;

				// Call the method
				m_callingSystemFunction = m_engine->scriptFunctions[i];
				void *ptr = 0;
#ifdef AS_NO_EXCEPTIONS
				ptr = m_engine->CallObjectMethodRetPtr(obj, arg, m_callingSystemFunction);
#else
				// This try/catch block is to catch potential exception that may
				// be thrown by the registered function.
				try
				{
					ptr = m_engine->CallObjectMethodRetPtr(obj, arg, m_callingSystemFunction);
				}
				catch (...)
				{
					// Convert the exception to a script exception so the VM can
					// properly report the error to the application and then clean up
					HandleAppException();
				}
#endif
				m_callingSystemFunction = 0;
				*(asPWORD*)&m_regs.valueRegister = (asPWORD)ptr;
			}

			// Update the program position after the call so that line number is correct
			l_bc += 2;

			if( m_regs.doProcessSuspend )
			{
				// Should the execution be suspended?
				if( m_doSuspend )
				{
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					m_status = asEXECUTION_SUSPENDED;
					return;
				}
				// An exception might have been raised
				if( m_status != asEXECUTION_ACTIVE )
				{
					m_regs.programPointer    = l_bc;
					m_regs.stackPointer      = l_sp;
					m_regs.stackFramePointer = l_fp;

					return;
				}
			}
		}
		NEXT_INSTRUCTION();

	// Don't let the optimizer optimize for size,
	// since it requires extra conditions and jumps
#if AS_USE_COMPUTED_GOTOS == 0
	INSTRUCTION(201): l_bc = (asDWORD*)201; goto case_FAULT;
	INSTRUCTION(202): l_bc = (asDWORD*)202; goto case_FAULT;
	INSTRUCTION(203): l_bc = (asDWORD*)203; goto case_FAULT;
	INSTRUCTION(204): l_bc = (asDWORD*)204; goto case_FAULT;
	INSTRUCTION(205): l_bc = (asDWORD*)205; goto case_FAULT;
	INSTRUCTION(206): l_bc = (asDWORD*)206; goto case_FAULT;
	INSTRUCTION(207): l_bc = (asDWORD*)207; goto case_FAULT;
	INSTRUCTION(208): l_bc = (asDWORD*)208; goto case_FAULT;
	INSTRUCTION(209): l_bc = (asDWORD*)209; goto case_FAULT;
	INSTRUCTION(210): l_bc = (asDWORD*)210; goto case_FAULT;
	INSTRUCTION(211): l_bc = (asDWORD*)211; goto case_FAULT;
	INSTRUCTION(212): l_bc = (asDWORD*)212; goto case_FAULT;
	INSTRUCTION(213): l_bc = (asDWORD*)213; goto case_FAULT;
	INSTRUCTION(214): l_bc = (asDWORD*)214; goto case_FAULT;
	INSTRUCTION(215): l_bc = (asDWORD*)215; goto case_FAULT;
	INSTRUCTION(216): l_bc = (asDWORD*)216; goto case_FAULT;
	INSTRUCTION(217): l_bc = (asDWORD*)217; goto case_FAULT;
	INSTRUCTION(218): l_bc = (asDWORD*)218; goto case_FAULT;
	INSTRUCTION(219): l_bc = (asDWORD*)219; goto case_FAULT;
	INSTRUCTION(220): l_bc = (asDWORD*)220; goto case_FAULT;
	INSTRUCTION(221): l_bc = (asDWORD*)221; goto case_FAULT;
	INSTRUCTION(222): l_bc = (asDWORD*)222; goto case_FAULT;
	INSTRUCTION(223): l_bc = (asDWORD*)223; goto case_FAULT;
	INSTRUCTION(224): l_bc = (asDWORD*)224; goto case_FAULT;
	INSTRUCTION(225): l_bc = (asDWORD*)225; goto case_FAULT;
	INSTRUCTION(226): l_bc = (asDWORD*)226; goto case_FAULT;
	INSTRUCTION(227): l_bc = (asDWORD*)227; goto case_FAULT;
	INSTRUCTION(228): l_bc = (asDWORD*)228; goto case_FAULT;
	INSTRUCTION(229): l_bc = (asDWORD*)229; goto case_FAULT;
	INSTRUCTION(230): l_bc = (asDWORD*)230; goto case_FAULT;
	INSTRUCTION(231): l_bc = (asDWORD*)231; goto case_FAULT;
	INSTRUCTION(232): l_bc = (asDWORD*)232; goto case_FAULT;
	INSTRUCTION(233): l_bc = (asDWORD*)233; goto case_FAULT;
	INSTRUCTION(234): l_bc = (asDWORD*)234; goto case_FAULT;
	INSTRUCTION(235): l_bc = (asDWORD*)235; goto case_FAULT;
	INSTRUCTION(236): l_bc = (asDWORD*)236; goto case_FAULT;
	INSTRUCTION(237): l_bc = (asDWORD*)237; goto case_FAULT;
	INSTRUCTION(238): l_bc = (asDWORD*)238; goto case_FAULT;
	INSTRUCTION(239): l_bc = (asDWORD*)239; goto case_FAULT;
	INSTRUCTION(240): l_bc = (asDWORD*)240; goto case_FAULT;
	INSTRUCTION(241): l_bc = (asDWORD*)241; goto case_FAULT;
	INSTRUCTION(242): l_bc = (asDWORD*)242; goto case_FAULT;
	INSTRUCTION(243): l_bc = (asDWORD*)243; goto case_FAULT;
	INSTRUCTION(244): l_bc = (asDWORD*)244; goto case_FAULT;
	INSTRUCTION(245): l_bc = (asDWORD*)245; goto case_FAULT;
	INSTRUCTION(246): l_bc = (asDWORD*)246; goto case_FAULT;
	INSTRUCTION(247): l_bc = (asDWORD*)247; goto case_FAULT;
	INSTRUCTION(248): l_bc = (asDWORD*)248; goto case_FAULT;
	INSTRUCTION(249): l_bc = (asDWORD*)249; goto case_FAULT;
	INSTRUCTION(250): l_bc = (asDWORD*)250; goto case_FAULT;
	INSTRUCTION(251): l_bc = (asDWORD*)251; goto case_FAULT;
	INSTRUCTION(252): l_bc = (asDWORD*)252; goto case_FAULT;
	INSTRUCTION(253): l_bc = (asDWORD*)253; goto case_FAULT;
	INSTRUCTION(254): l_bc = (asDWORD*)254; goto case_FAULT;
	INSTRUCTION(255): l_bc = (asDWORD*)255; goto case_FAULT;
#endif

#ifdef AS_DEBUG
	default:
		asASSERT(false);
		SetInternalException(TXT_UNRECOGNIZED_BYTE_CODE);
#endif
#if defined(_MSC_VER) && !defined(AS_DEBUG)
	default:
		// This Microsoft specific code allows the
		// compiler to optimize the switch case as
		// it will know that the code will never
		// reach this point
		__assume(0);
#endif
	} // end of switch

#ifdef AS_DEBUG
		asDWORD instr = *(asBYTE*)old;
		if( instr != asBC_JMP && instr != asBC_JMPP && (instr < asBC_JZ || instr > asBC_JNP) && instr != asBC_JLowZ && instr != asBC_JLowNZ &&
			instr != asBC_CALL && instr != asBC_CALLBND && instr != asBC_CALLINTF && instr != asBC_RET && instr != asBC_ALLOC && instr != asBC_CallPtr &&
			instr != asBC_JitEntry )
		{
			asASSERT( (l_bc - old) == asBCTypeSize[asBCInfo[instr].type] );
		}
#endif
	} // end of for(;;)

case_FAULT:
	// Store for debugging info
	m_regs.programPointer    = l_bc;
	m_regs.stackPointer      = l_sp;
	m_regs.stackFramePointer = l_fp;

	SetInternalException(TXT_UNRECOGNIZED_BYTE_CODE);
	asASSERT(false);
}

// interface
int asCContext::SetException(const char *descr, bool allowCatch)
{
	// Only allow this if we're executing a CALL byte code
	if( m_callingSystemFunction == 0 ) return asERROR;

	SetInternalException(descr, allowCatch);

	return 0;
}

void asCContext::SetInternalException(const char *descr, bool allowCatch)
{
	if( m_inExceptionHandler )
	{
		asASSERT(false); // Shouldn't happen
		return; // but if it does, at least this will not crash the application
	}

	m_status                = asEXECUTION_EXCEPTION;
	m_regs.doProcessSuspend = true;

	m_exceptionString       = descr;
	m_exceptionFunction     = m_currentFunction->id;

	if( m_currentFunction->scriptData )
	{
		m_exceptionLine    = m_currentFunction->GetLineNumber(int(m_regs.programPointer - m_currentFunction->scriptData->byteCode.AddressOf()), &m_exceptionSectionIdx);
		m_exceptionColumn  = m_exceptionLine >> 20;
		m_exceptionLine   &= 0xFFFFF;
	}
	else
	{
		m_exceptionSectionIdx = 0;
		m_exceptionLine       = 0;
		m_exceptionColumn     = 0;
	}

	// Recursively search the callstack for try/catch blocks
	m_exceptionWillBeCaught = allowCatch && FindExceptionTryCatch();

	if( m_exceptionCallback )
		CallExceptionCallback();
}

// interface
bool asCContext::WillExceptionBeCaught()
{
	return m_exceptionWillBeCaught;
}

void asCContext::CleanReturnObject()
{
	if( m_initialFunction && m_initialFunction->DoesReturnOnStack() && m_status == asEXECUTION_FINISHED )
	{
		// If function returns on stack we need to call the destructor on the returned object
		if(CastToObjectType(m_initialFunction->returnType.GetTypeInfo())->beh.destruct )
			m_engine->CallObjectMethod(GetReturnObject(), CastToObjectType(m_initialFunction->returnType.GetTypeInfo())->beh.destruct);

		return;
	}

	if( m_regs.objectRegister == 0 ) return;

	asASSERT( m_regs.objectType != 0 );

	if( m_regs.objectType )
	{
		if (m_regs.objectType->GetFlags() & asOBJ_FUNCDEF)
		{
			// Release the function pointer
			reinterpret_cast<asIScriptFunction*>(m_regs.objectRegister)->Release();
			m_regs.objectRegister = 0;
		}
		else
		{
			// Call the destructor on the object
			asSTypeBehaviour *beh = &(CastToObjectType(reinterpret_cast<asCTypeInfo*>(m_regs.objectType))->beh);
			if (m_regs.objectType->GetFlags() & asOBJ_REF)
			{
				asASSERT(beh->release || (m_regs.objectType->GetFlags() & asOBJ_NOCOUNT));

				if (beh->release)
					m_engine->CallObjectMethod(m_regs.objectRegister, beh->release);

				m_regs.objectRegister = 0;
			}
			else
			{
				if (beh->destruct)
					m_engine->CallObjectMethod(m_regs.objectRegister, beh->destruct);

				// Free the memory
				m_engine->CallFree(m_regs.objectRegister);
				m_regs.objectRegister = 0;
			}
		}
	}
}

void asCContext::CleanStack(bool catchException)
{
	m_inExceptionHandler = true;

	// Run the clean up code and move to catch block
	bool caught = CleanStackFrame(catchException);
	if( !caught )
	{
		// Set the status to exception so that the stack unwind is done correctly.
		// This shouldn't be done for the current function, which is why we only
		// do this after the first CleanStackFrame() is done.
		m_status = asEXECUTION_EXCEPTION;

		while (!caught && m_callStack.GetLength() > 0)
		{
			// Only clean up until the top most marker for a nested call
			asPWORD *s = m_callStack.AddressOf() + m_callStack.GetLength() - CALLSTACK_FRAME_SIZE;
			if (s[0] == 0)
				break;

			PopCallState();

			caught = CleanStackFrame(catchException);
		}
	}

	// If the exception was caught, then move the status to
	// active as is now possible to resume the execution
	if (caught)
		m_status = asEXECUTION_ACTIVE;

	m_inExceptionHandler = false;
}

// Interface
bool asCContext::IsVarInScope(asUINT varIndex, asUINT stackLevel)
{
	// Don't return anything if there is no bytecode, e.g. before calling Execute()
	if( m_regs.programPointer == 0 ) return false;

	if( stackLevel >= GetCallstackSize() ) return false;

	asCScriptFunction *func;
	asUINT pos;

	if( stackLevel == 0 )
	{
		func = m_currentFunction;
		if( func->scriptData == 0 ) return false;
		pos = asUINT(m_regs.programPointer - func->scriptData->byteCode.AddressOf());
	}
	else
	{
		asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize()-stackLevel-1)*CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		if( func->scriptData == 0 ) return false;
		pos = asUINT((asDWORD*)s[2] - func->scriptData->byteCode.AddressOf());
	}

	// First determine if the program position is after the variable declaration
	if( func->scriptData->variables.GetLength() <= varIndex ) return false;
	if( func->scriptData->variables[varIndex]->declaredAtProgramPos > pos ) return false;

	asUINT declaredAt = func->scriptData->variables[varIndex]->declaredAtProgramPos;

	// If the program position is after the variable declaration it is necessary
	// determine if the program position is still inside the statement block where
	// the variable was declared.
	bool foundVarDecl = false;

	// Temporary variables aren't explicitly declared, they are just reserved slots available throughout the function call.
	// So we'll consider that the variable declaration is found at the very beginning
	if (func->scriptData->variables[varIndex]->name.GetLength() == 0)
		foundVarDecl = true;

	for( int n = 0; n < (int)func->scriptData->objVariableInfo.GetLength(); n++ )
	{
		// Find the varDecl
		if( func->scriptData->objVariableInfo[n].programPos >= declaredAt )
		{
			// skip instructions at the same program position, but before the varDecl. 
			// Note, varDecl will only be in the objVariableInfo for object types
			if (func->scriptData->objVariableInfo[n].programPos == declaredAt && 
				!foundVarDecl && 
				func->scriptData->objVariableInfo[n].option != asOBJ_VARDECL)
				continue;

			foundVarDecl = true;

			// If the current block ends between the declaredAt and current
			// program position, then we know the variable is no longer visible
			int level = 0;
			for( ; n < (int)func->scriptData->objVariableInfo.GetLength(); n++ )
			{
				if( func->scriptData->objVariableInfo[n].programPos > pos )
					break;

				if( func->scriptData->objVariableInfo[n].option == asBLOCK_BEGIN ) level++;
				if( func->scriptData->objVariableInfo[n].option == asBLOCK_END && --level < 0 )
					return false;
			}

			break;
		}
	}

	// Variable is visible
	return true;
}

// Internal
void asCContext::DetermineLiveObjects(asCArray<int> &liveObjects, asUINT stackLevel)
{
	asASSERT( stackLevel < GetCallstackSize() );

	asCScriptFunction *func;
	asUINT pos;

	if( stackLevel == 0 )
	{
		func = m_currentFunction;
		if( func->scriptData == 0 )
			return;

		pos = asUINT(m_regs.programPointer - func->scriptData->byteCode.AddressOf());

		if( m_status == asEXECUTION_EXCEPTION )
		{
			// Don't consider the last instruction as executed, as it failed with an exception
			// It's not actually necessary to decrease the exact size of the instruction. Just
			// before the current position is enough to disconsider it.
			pos--;
		}
	}
	else
	{
		asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize()-stackLevel-1)*CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		if( func->scriptData == 0 )
			return;

		pos = asUINT((asDWORD*)s[2] - func->scriptData->byteCode.AddressOf());

		// Don't consider the last instruction as executed, as the function that was called by it
		// is still being executed. If we consider it as executed already, then a value object
		// returned by value would be considered alive, which it is not.
		pos--;
	}

	// Determine which object variables that are really live ones
	liveObjects.SetLength(func->scriptData->variables.GetLength());
	memset(liveObjects.AddressOf(), 0, sizeof(int)*liveObjects.GetLength());
	for( int n = 0; n < (int)func->scriptData->objVariableInfo.GetLength(); n++ )
	{
		// Find the first variable info with a larger position than the current
		// As the variable info are always placed on the instruction right after the
		// one that initialized or freed the object, the current position needs to be
		// considered as valid.
		if( func->scriptData->objVariableInfo[n].programPos > pos )
		{
			// We've determined how far the execution ran, now determine which variables are alive
			for( --n; n >= 0; n-- )
			{
				switch( func->scriptData->objVariableInfo[n].option )
				{
				case asOBJ_UNINIT: // Object was destroyed
					{
						// TODO: optimize: This should have been done by the compiler already
						// Which variable is this? Use IsVarInScope to get the correct variable in case there are multiple variables sharing the same offset
						asUINT var = asUINT(-1);
						for (asUINT v = 0; v < func->scriptData->variables.GetLength(); v++)
							if (func->scriptData->variables[v]->stackOffset == func->scriptData->objVariableInfo[n].variableOffset && 
								IsVarInScope(v, stackLevel) )
							{
								var = v;
								break;
							}
						asASSERT(var != asUINT(-1));
						liveObjects[var] -= 1;
					}
					break;
				case asOBJ_INIT: // Object was created
					{
						// Which variable is this? Use IsVarInScope to get the correct variable in case there are multiple variables sharing the same offset
						asUINT var = asUINT(-1);
						for (asUINT v = 0; v < func->scriptData->variables.GetLength(); v++)
							if (func->scriptData->variables[v]->stackOffset == func->scriptData->objVariableInfo[n].variableOffset &&
								IsVarInScope(v, stackLevel) )
							{
								var = v;
								break;
							}
						if( var != asUINT(-1) )
							liveObjects[var] += 1;
					}
					break;
				case asBLOCK_BEGIN: // Start block
					// We should ignore start blocks, since it just means the
					// program was within the block when the exception occurred
					break;
				case asBLOCK_END: // End block
					// We need to skip the entire block, as the objects created
					// and destroyed inside this block are already out of scope
					{
						int nested = 1;
						while( nested > 0 )
						{
							int option = func->scriptData->objVariableInfo[--n].option;
							if( option == 3 )
								nested++;
							if( option == 2 )
								nested--;
						}
					}
					break;
				case asOBJ_VARDECL: // A variable was declared
					// We don't really care about the variable declarations at this moment
					break;
				}
			}

			// We're done with the investigation
			break;
		}
	}
}

void asCContext::CleanArgsOnStack()
{
	if( !m_needToCleanupArgs )
		return;

	asASSERT( m_currentFunction->scriptData );

	// Find the instruction just before the current program pointer
	asDWORD *instr = m_currentFunction->scriptData->byteCode.AddressOf();
	asDWORD *prevInstr = 0;
	while( instr < m_regs.programPointer )
	{
		prevInstr = instr;
		instr += asBCTypeSize[asBCInfo[*(asBYTE*)(instr)].type];
	}

	// Determine what function was being called
	asCScriptFunction *func = 0;
	asBYTE bc = *(asBYTE*)prevInstr;
	if( bc == asBC_CALL || bc == asBC_CALLSYS || bc == asBC_CALLINTF )
	{
		int funcId = asBC_INTARG(prevInstr);
		func = m_engine->scriptFunctions[funcId];
	}
	else if( bc == asBC_CALLBND )
	{
		int funcId = asBC_INTARG(prevInstr);
		func = m_engine->importedFunctions[funcId & ~FUNC_IMPORTED]->importedFunctionSignature;
	}
	else if( bc == asBC_CallPtr )
	{
		asUINT v;
		int var = asBC_SWORDARG0(prevInstr);

		// Find the funcdef from the local variable
		for( v = 0; v < m_currentFunction->scriptData->variables.GetLength(); v++ )
			if( m_currentFunction->scriptData->variables[v]->stackOffset == var )
			{
				asASSERT(m_currentFunction->scriptData->variables[v]->type.GetTypeInfo());
				func = CastToFuncdefType(m_currentFunction->scriptData->variables[v]->type.GetTypeInfo())->funcdef;
				break;
			}

		if( func == 0 )
		{
			// Look in parameters
			int paramPos = 0;
			if( m_currentFunction->objectType )
				paramPos -= AS_PTR_SIZE;
			if( m_currentFunction->DoesReturnOnStack() )
				paramPos -= AS_PTR_SIZE;
			for( v = 0; v < m_currentFunction->parameterTypes.GetLength(); v++ )
			{
				if( var == paramPos )
				{
					if (m_currentFunction->parameterTypes[v].IsFuncdef())
						func = CastToFuncdefType(m_currentFunction->parameterTypes[v].GetTypeInfo())->funcdef;
					break;
				}
				paramPos -= m_currentFunction->parameterTypes[v].GetSizeOnStackDWords();
			}
		}
	}
	else
		asASSERT( false );

	asASSERT( func );

	// Clean parameters
	int offset = 0;
	if( func->objectType )
		offset += AS_PTR_SIZE;
	if( func->DoesReturnOnStack() )
		offset += AS_PTR_SIZE;
	for( asUINT n = 0; n < func->parameterTypes.GetLength(); n++ )
	{
		if( (func->parameterTypes[n].IsObject() || func->parameterTypes[n].IsFuncdef()) && !func->parameterTypes[n].IsReference() )
		{
			// TODO: cleanup: This logic is repeated twice in CleanStackFrame too. Should create a common function to share the code
			if( *(asPWORD*)&m_regs.stackPointer[offset] )
			{
				// Call the object's destructor
				asSTypeBehaviour *beh = func->parameterTypes[n].GetBehaviour();
				if (func->parameterTypes[n].GetTypeInfo()->flags & asOBJ_FUNCDEF)
				{
					(*(asCScriptFunction**)&m_regs.stackPointer[offset])->Release();
				}
				else if( func->parameterTypes[n].GetTypeInfo()->flags & asOBJ_REF )
				{
					asASSERT( (func->parameterTypes[n].GetTypeInfo()->flags & asOBJ_NOCOUNT) || beh->release );

					if( beh->release )
						m_engine->CallObjectMethod((void*)*(asPWORD*)&m_regs.stackPointer[offset], beh->release);
				}
				else
				{
					if( beh->destruct )
						m_engine->CallObjectMethod((void*)*(asPWORD*)&m_regs.stackPointer[offset], beh->destruct);

					// Free the memory
					m_engine->CallFree((void*)*(asPWORD*)&m_regs.stackPointer[offset]);
				}
				*(asPWORD*)&m_regs.stackPointer[offset] = 0;
			}
		}

		offset += func->parameterTypes[n].GetSizeOnStackDWords();
	}

	// Restore the stack pointer
	m_regs.stackPointer += offset;

	m_needToCleanupArgs = false;
}

bool asCContext::FindExceptionTryCatch()
{
	// Check each of the script functions on the callstack to see if
	// the current program position is within a try/catch block
	if (m_currentFunction && m_currentFunction->scriptData)
	{
		asUINT currPos = asUINT(m_regs.programPointer - m_currentFunction->scriptData->byteCode.AddressOf());
		for (asUINT n = 0; n < m_currentFunction->scriptData->tryCatchInfo.GetLength(); n++)
		{
			if (currPos >= m_currentFunction->scriptData->tryCatchInfo[n].tryPos &&
				currPos < m_currentFunction->scriptData->tryCatchInfo[n].catchPos)
				return true;
		}
	}

	int stackSize = GetCallstackSize();
	for (int level = 1; level < stackSize; level++)
	{
		asPWORD *s = m_callStack.AddressOf() + (stackSize - level - 1)*CALLSTACK_FRAME_SIZE;
		asCScriptFunction *func = (asCScriptFunction*)s[1];
		if (func && func->scriptData)
		{
			asUINT currPos = asUINT((asDWORD*)s[2] - func->scriptData->byteCode.AddressOf());
			for (asUINT n = 0; n < func->scriptData->tryCatchInfo.GetLength(); n++)
			{
				if (currPos >= func->scriptData->tryCatchInfo[n].tryPos &&
					currPos < func->scriptData->tryCatchInfo[n].catchPos)
					return true;
			}
		}
	}

	return false;
}

bool asCContext::CleanStackFrame(bool catchException)
{
	bool exceptionCaught = false;
	asSTryCatchInfo *tryCatchInfo = 0;

	if (m_currentFunction == 0)
		return false;

	if (m_currentFunction->funcType == asFUNC_SCRIPT && m_currentFunction->scriptData == 0)
	{
		asCString msg;
		msg.Format(TXT_FUNC_s_RELEASED_BEFORE_CLEANUP, m_currentFunction->name.AddressOf());
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
		return false;
	}

	// Clean object variables on the stack
	// If the stack memory is not allocated or the program pointer
	// is not set, then there is nothing to clean up on the stack frame
	if( !m_isStackMemoryNotAllocated && m_regs.programPointer )
	{
		// If the exception occurred while calling a function it is necessary
		// to clean up the arguments that were put on the stack.
		CleanArgsOnStack();

		// Check if this function will catch the exception
		// Try blocks can be nested, so use the innermost block
		if (catchException && m_currentFunction->scriptData)
		{
			asUINT currPos = asUINT(m_regs.programPointer - m_currentFunction->scriptData->byteCode.AddressOf());

			for (asUINT n = 0; n < m_currentFunction->scriptData->tryCatchInfo.GetLength(); n++)
			{
				if (currPos >= m_currentFunction->scriptData->tryCatchInfo[n].tryPos &&
					currPos < m_currentFunction->scriptData->tryCatchInfo[n].catchPos)
				{
					tryCatchInfo = &m_currentFunction->scriptData->tryCatchInfo[n];
					exceptionCaught = true;
				}
				if (currPos < m_currentFunction->scriptData->tryCatchInfo[n].tryPos)
					break;
			}
		}

		// Restore the stack pointer
		if( !exceptionCaught )
			m_regs.stackPointer += m_currentFunction->scriptData->variableSpace;

		// Determine which object variables that are really live ones
		asCArray<int> liveObjects;
		DetermineLiveObjects(liveObjects, 0);

		for (asUINT n = 0; n < m_currentFunction->scriptData->variables.GetLength(); n++)
		{
			int pos = m_currentFunction->scriptData->variables[n]->stackOffset;

			// If the exception was caught, then only clean up objects within the try block
			if (exceptionCaught)
			{
				// Find out where the variable was declared, and skip cleaning of those that were declared before the try catch
				// Multiple variables in different scopes may occupy the same slot on the stack so it is necessary to search
				// the entire list to determine which variable occupies the slot now.
				int skipClean = 0;
				for( asUINT p = 0; p < m_currentFunction->scriptData->objVariableInfo.GetLength(); p++ )
				{
					asSObjectVariableInfo &info = m_currentFunction->scriptData->objVariableInfo[p];
					if (info.variableOffset == pos &&
						info.option == asOBJ_VARDECL )
					{
						asUINT progPos = info.programPos;
						if (progPos < tryCatchInfo->tryPos )
						{
							if( skipClean >= 0 )
								skipClean = 1;
							break;
						}
						else if( progPos < tryCatchInfo->catchPos )
						{
							skipClean = -1;
							break;
						}
					}
				}

				// Skip only variables that have been declared before the try block. Variables declared
				// within the try block and variables whose declaration was not identified (temporary objects)
				// will not be skipped.
				// TODO: What if a temporary variable reuses a slot from a declared variable that is no longer in scope?
				if (skipClean > 0)
					continue;
			}

			if( m_currentFunction->scriptData->variables[n]->onHeap )
			{
				// Check if the pointer is initialized
				if( *(asPWORD*)&m_regs.stackFramePointer[-pos] )
				{
					// Skip pointers with unknown types, as this is either a null pointer or just a reference that is not owned by function
					if( m_currentFunction->scriptData->variables[n]->type.GetTypeInfo() && !m_currentFunction->scriptData->variables[n]->type.IsReference() )
					{
						// Call the object's destructor
						if( m_currentFunction->scriptData->variables[n]->type.GetTypeInfo()->flags & asOBJ_FUNCDEF )
						{
							(*(asCScriptFunction**)&m_regs.stackFramePointer[-pos])->Release();
						}
						else if (m_currentFunction->scriptData->variables[n]->type.GetTypeInfo()->flags & asOBJ_REF)
						{
							asSTypeBehaviour* beh = &CastToObjectType(m_currentFunction->scriptData->variables[n]->type.GetTypeInfo())->beh;
							asASSERT((m_currentFunction->scriptData->variables[n]->type.GetTypeInfo()->flags & asOBJ_NOCOUNT) || beh->release);
							if( beh->release )
								m_engine->CallObjectMethod((void*)*(asPWORD*)&m_regs.stackFramePointer[-pos], beh->release);
						}
						else
						{
							asSTypeBehaviour* beh = &CastToObjectType(m_currentFunction->scriptData->variables[n]->type.GetTypeInfo())->beh;
							if (beh->destruct)
								m_engine->CallObjectMethod((void*)*(asPWORD*)&m_regs.stackFramePointer[-pos], beh->destruct);
							else if (m_currentFunction->scriptData->variables[n]->type.GetTypeInfo()->flags & asOBJ_LIST_PATTERN)
								m_engine->DestroyList((asBYTE*)*(asPWORD*)&m_regs.stackFramePointer[-pos], CastToObjectType(m_currentFunction->scriptData->variables[n]->type.GetTypeInfo()));

							// Free the memory
							m_engine->CallFree((void*)*(asPWORD*)&m_regs.stackFramePointer[-pos]);
						}
					}
					*(asPWORD*)&m_regs.stackFramePointer[-pos] = 0;
				}
			}
			else
			{
				// Only destroy the object if it is truly alive
				if( liveObjects[n] > 0 )
				{
					asASSERT(m_currentFunction->scriptData->variables[n]->type.GetTypeInfo() && m_currentFunction->scriptData->variables[n]->type.GetTypeInfo()->GetFlags() & asOBJ_VALUE);

					asSTypeBehaviour* beh = &CastToObjectType(m_currentFunction->scriptData->variables[n]->type.GetTypeInfo())->beh;
					if( beh->destruct )
						m_engine->CallObjectMethod((void*)(asPWORD*)&m_regs.stackFramePointer[-pos], beh->destruct);
				}
			}
		}
	}
	else
		m_isStackMemoryNotAllocated = false;

	// If the exception was caught then move the program position and stack pointer to the catch block then stop the unwinding
	if (exceptionCaught)
	{
		m_regs.stackPointer = m_regs.stackFramePointer - tryCatchInfo->stackSize - m_currentFunction->scriptData->variableSpace;
		m_regs.programPointer = m_currentFunction->scriptData->byteCode.AddressOf() + tryCatchInfo->catchPos;
		return exceptionCaught;
	}

	// Functions that do not own the object and parameters shouldn't do any clean up
	if( m_currentFunction == 0 || m_currentFunction->dontCleanUpOnException )
		return exceptionCaught;

	// Clean object and parameters
	int offset = 0;
	if( m_currentFunction->objectType )
		offset += AS_PTR_SIZE;
	if( m_currentFunction->DoesReturnOnStack() )
		offset += AS_PTR_SIZE;
	for( asUINT n = 0; n < m_currentFunction->parameterTypes.GetLength(); n++ )
	{
		if( (m_currentFunction->parameterTypes[n].IsObject() ||m_currentFunction->parameterTypes[n].IsFuncdef()) && !m_currentFunction->parameterTypes[n].IsReference() )
		{
			if( *(asPWORD*)&m_regs.stackFramePointer[offset] )
			{
				// Call the object's destructor
				asSTypeBehaviour *beh = m_currentFunction->parameterTypes[n].GetBehaviour();
				if (m_currentFunction->parameterTypes[n].GetTypeInfo()->flags & asOBJ_FUNCDEF)
				{
					(*(asCScriptFunction**)&m_regs.stackFramePointer[offset])->Release();
				}
				else if( m_currentFunction->parameterTypes[n].GetTypeInfo()->flags & asOBJ_REF )
				{
					asASSERT( (m_currentFunction->parameterTypes[n].GetTypeInfo()->flags & asOBJ_NOCOUNT) || beh->release );

					if( beh->release )
						m_engine->CallObjectMethod((void*)*(asPWORD*)&m_regs.stackFramePointer[offset], beh->release);
				}
				else
				{
					if( beh->destruct )
						m_engine->CallObjectMethod((void*)*(asPWORD*)&m_regs.stackFramePointer[offset], beh->destruct);

					// Free the memory
					m_engine->CallFree((void*)*(asPWORD*)&m_regs.stackFramePointer[offset]);
				}
				*(asPWORD*)&m_regs.stackFramePointer[offset] = 0;
			}
		}

		offset += m_currentFunction->parameterTypes[n].GetSizeOnStackDWords();
	}

	return exceptionCaught;
}

// interface
int asCContext::GetExceptionLineNumber(int *column, const char **sectionName)
{
	// Return the last exception even if the context is no longer in the exception state
	// if( GetState() != asEXECUTION_EXCEPTION ) return asERROR;

	if( column ) *column = m_exceptionColumn;

	if( sectionName )
	{
		// The section index can be -1 if the exception was raised in a generated function, e.g. $fact for templates
		if( m_exceptionSectionIdx >= 0 )
			*sectionName = m_engine->scriptSectionNames[m_exceptionSectionIdx]->AddressOf();
		else
			*sectionName = 0;
	}

	return m_exceptionLine;
}

// interface
asIScriptFunction *asCContext::GetExceptionFunction()
{
	// Return the last exception even if the context is no longer in the exception state
	// if( GetState() != asEXECUTION_EXCEPTION ) return 0;

	return m_engine->scriptFunctions[m_exceptionFunction];
}

// interface
const char *asCContext::GetExceptionString()
{
	// Return the last exception even if the context is no longer in the exception state
	// if( GetState() != asEXECUTION_EXCEPTION ) return 0;

	return m_exceptionString.AddressOf();
}

// interface
asEContextState asCContext::GetState() const
{
	return m_status;
}

// interface
int asCContext::SetLineCallback(const asSFuncPtr &callback, void *obj, int callConv)
{
	// First turn off the line callback to avoid a second thread
	// attempting to call it while the new one is still being set
	m_lineCallback = false;

	m_lineCallbackObj = obj;
	bool isObj = false;
	if( (unsigned)callConv == asCALL_GENERIC || (unsigned)callConv == asCALL_THISCALL_OBJFIRST || (unsigned)callConv == asCALL_THISCALL_OBJLAST )
	{
		m_regs.doProcessSuspend = m_doSuspend;
		return asNOT_SUPPORTED;
	}
	if( (unsigned)callConv >= asCALL_THISCALL )
	{
		isObj = true;
		if( obj == 0 )
		{
			m_regs.doProcessSuspend = m_doSuspend;
			return asINVALID_ARG;
		}
	}

	int r = DetectCallingConvention(isObj, callback, callConv, 0, &m_lineCallbackFunc);

	// Turn on the line callback after setting both the function pointer and object pointer
	if( r >= 0 ) m_lineCallback = true;

	// The BC_SUSPEND instruction should be processed if either line
	// callback is set or if the application has requested a suspension
	m_regs.doProcessSuspend = m_doSuspend || m_lineCallback;

	return r;
}

void asCContext::CallLineCallback()
{
	if( m_lineCallbackFunc.callConv < ICC_THISCALL )
		m_engine->CallGlobalFunction(this, m_lineCallbackObj, &m_lineCallbackFunc, 0);
	else
		m_engine->CallObjectMethod(m_lineCallbackObj, this, &m_lineCallbackFunc, 0);
}

// interface
int asCContext::SetExceptionCallback(const asSFuncPtr &callback, void *obj, int callConv)
{
	m_exceptionCallback = true;
	m_exceptionCallbackObj = obj;
	bool isObj = false;
	if( (unsigned)callConv == asCALL_GENERIC || (unsigned)callConv == asCALL_THISCALL_OBJFIRST || (unsigned)callConv == asCALL_THISCALL_OBJLAST )
		return asNOT_SUPPORTED;
	if( (unsigned)callConv >= asCALL_THISCALL )
	{
		isObj = true;
		if( obj == 0 )
		{
			m_exceptionCallback = false;
			return asINVALID_ARG;
		}
	}
	int r = DetectCallingConvention(isObj, callback, callConv, 0, &m_exceptionCallbackFunc);
	if( r < 0 ) m_exceptionCallback = false;
	return r;
}

void asCContext::CallExceptionCallback()
{
	if( m_exceptionCallbackFunc.callConv < ICC_THISCALL )
		m_engine->CallGlobalFunction(this, m_exceptionCallbackObj, &m_exceptionCallbackFunc, 0);
	else
		m_engine->CallObjectMethod(m_exceptionCallbackObj, this, &m_exceptionCallbackFunc, 0);
}

#ifndef AS_NO_EXCEPTIONS
// internal
void asCContext::HandleAppException()
{
	// This method is called from within a catch(...) block
	if (m_engine->translateExceptionCallback)
	{
		// Allow the application to translate the application exception to a proper exception string
		if (m_engine->translateExceptionCallbackFunc.callConv < ICC_THISCALL)
			m_engine->CallGlobalFunction(this, m_engine->translateExceptionCallbackObj, &m_engine->translateExceptionCallbackFunc, 0);
		else
			m_engine->CallObjectMethod(m_engine->translateExceptionCallbackObj, this, &m_engine->translateExceptionCallbackFunc, 0);
	}

	// Make sure an exception is set even if the application decides not to do any specific translation
	if( m_status != asEXECUTION_EXCEPTION )
		SetException(TXT_EXCEPTION_CAUGHT);
}
#endif

// interface
void asCContext::ClearLineCallback()
{
	m_lineCallback = false;
	m_regs.doProcessSuspend = m_doSuspend;
}

// interface
void asCContext::ClearExceptionCallback()
{
	m_exceptionCallback = false;
}

int asCContext::CallGeneric(asCScriptFunction *descr)
{
	asSSystemFunctionInterface *sysFunc = descr->sysFuncIntf;
	void (*func)(asIScriptGeneric*) = (void (*)(asIScriptGeneric*))sysFunc->func;
	int popSize = sysFunc->paramSize;
	asDWORD *args = m_regs.stackPointer;

	// Verify the object pointer if it is a class method
	void *currentObject = 0;
	asASSERT( sysFunc->callConv == ICC_GENERIC_FUNC || sysFunc->callConv == ICC_GENERIC_METHOD );
	if( sysFunc->callConv == ICC_GENERIC_METHOD )
	{
		// The object pointer should be popped from the context stack
		popSize += AS_PTR_SIZE;

		// Check for null pointer
		currentObject = (void*)*(asPWORD*)(args);
		if( currentObject == 0 )
		{
			SetInternalException(TXT_NULL_POINTER_ACCESS);
			return 0;
		}

		asASSERT( sysFunc->baseOffset == 0 );

		// Skip object pointer
		args += AS_PTR_SIZE;
	}

	if( descr->DoesReturnOnStack() )
	{
		// Skip the address where the return value will be stored
		args += AS_PTR_SIZE;
		popSize += AS_PTR_SIZE;
	}

	asDWORD varArgCount = 0;
	if (descr->IsVariadic())
	{
		varArgCount = *args;

		args += 1;
		popSize += 1;

		// Calculate the arguments that need to be popped
		asCDataType variadicType = descr->parameterTypes[descr->parameterTypes.GetLength() - 1];
		int sizeOfVariadicArg = variadicType.GetSizeOnStackDWords();

		// sysFunc->paramSize already added one variadic arg for the ..., but there might not actually be any
		popSize -= sizeOfVariadicArg;

		// Add the actual space used for the variadic args
		popSize += sizeOfVariadicArg * (varArgCount - descr->parameterTypes.GetLength() + 1);
	}

	// TODO: variadic: Put them in different branch. Do we really need a separate object for variadics?
	asCGeneric genOrdinary(m_engine, descr, currentObject, args);
	asCGenericVariadic genVar(m_engine, descr, currentObject, args, varArgCount);

	asCGeneric& gen = descr->IsVariadic() ? genVar : genOrdinary;

	m_callingSystemFunction = descr;
#ifdef AS_NO_EXCEPTIONS
	func(&gen);
#else
	// This try/catch block is to catch potential exception that may
	// be thrown by the registered function.
	try
	{
		func(&gen);
	}
	catch (...)
	{
		// Convert the exception to a script exception so the VM can
		// properly report the error to the application and then clean up
		HandleAppException();
	}
#endif
	m_callingSystemFunction = 0;

	m_regs.valueRegister = gen.returnVal;
	m_regs.objectRegister = gen.objectRegister;
	m_regs.objectType = descr->returnType.GetTypeInfo();

	// Increase the returned handle if the function has been declared with autohandles
	// and the engine is not set to use the old mode for the generic calling convention
	if (sysFunc->returnAutoHandle && m_engine->ep.genericCallMode == 1 && m_regs.objectRegister)
	{
		asASSERT(!(descr->returnType.GetTypeInfo()->flags & asOBJ_NOCOUNT));
		m_engine->CallObjectMethod(m_regs.objectRegister, CastToObjectType(descr->returnType.GetTypeInfo())->beh.addref);
	}

	// Clean up arguments
	const asUINT cleanCount = sysFunc->cleanArgs.GetLength();
	if( cleanCount )
	{
		asSSystemFunctionInterface::SClean *clean = sysFunc->cleanArgs.AddressOf();
		for( asUINT n = 0; n < cleanCount; n++, clean++ )
		{
			void **addr = (void**)&args[clean->off];
			if( clean->op == 0 )
			{
				if( *addr != 0 )
				{
					m_engine->CallObjectMethod(*addr, clean->ot->beh.release);
					*addr = 0;
				}
			}
			else
			{
				asASSERT( clean->op == 1 || clean->op == 2 );
				asASSERT( *addr );

				if( clean->op == 2 )
					m_engine->CallObjectMethod(*addr, clean->ot->beh.destruct);

				m_engine->CallFree(*addr);
			}
		}
	}

	// Return how much should be popped from the stack
	return popSize;
}

// interface
int asCContext::GetVarCount(asUINT stackLevel)
{
	asIScriptFunction *func = GetFunction(stackLevel);
	if( func == 0 ) return asINVALID_ARG;

	return func->GetVarCount();
}

// interface
int asCContext::GetVar(asUINT varIndex, asUINT stackLevel, const char** name, int* typeId, asETypeModifiers* typeModifiers, bool* isVarOnHeap, int* stackOffset)
{
	asCScriptFunction* func = reinterpret_cast<asCScriptFunction*>(GetFunction(stackLevel));
	if (func == 0) return asINVALID_ARG;

	int r = func->GetVar(varIndex, name, typeId);
	if (r < 0) 
		return r;

	if (isVarOnHeap)
		*isVarOnHeap = func->scriptData->variables[varIndex]->onHeap;

	if( stackOffset )
		*stackOffset = func->scriptData->variables[varIndex]->stackOffset;

	if (typeModifiers)
	{
		*typeModifiers = asTM_NONE;

		if (func->scriptData && func->scriptData->variables[varIndex]->type.IsReference())
		{
			// Find the function argument if it is not a local variable
			int pos = func->scriptData->variables[varIndex]->stackOffset;
			if (pos <= 0)
			{
				int stackPos = 0;
				if (func->objectType)
					stackPos -= AS_PTR_SIZE;

				if (func->DoesReturnOnStack())
				{
					if (stackPos == pos)
						*typeModifiers = asTM_INOUTREF;
					stackPos -= AS_PTR_SIZE;
				}

				for (asUINT n = 0; n < func->parameterTypes.GetLength(); n++)
				{
					if (stackPos == pos)
					{
						// The right argument was found. Is this a reference parameter?
						*typeModifiers = func->inOutFlags[n];
						break;
					}
					stackPos -= func->parameterTypes[n].GetSizeOnStackDWords();
				}
			}
			else
				*typeModifiers = asTM_INOUTREF;
		}

		if (func->scriptData &&
			func->scriptData->variables[varIndex]->type.IsReadOnly())
		{
			*typeModifiers = (asETypeModifiers)(*typeModifiers | asTM_CONST);
		}
	}

	return asSUCCESS;
}

#ifdef AS_DEPRECATED
// interface
const char *asCContext::GetVarName(asUINT varIndex, asUINT stackLevel)
{
	asIScriptFunction *func = GetFunction(stackLevel);
	if( func == 0 ) return 0;

	const char *name = 0;
	int r = func->GetVar(varIndex, &name);
	return r >= 0 ? name : 0;
}
#endif

// interface
const char *asCContext::GetVarDeclaration(asUINT varIndex, asUINT stackLevel, bool includeNamespace)
{
	asIScriptFunction *func = GetFunction(stackLevel);
	if( func == 0 ) return 0;

	return func->GetVarDecl(varIndex, includeNamespace);
}

#ifdef AS_DEPRECATED
// interface
int asCContext::GetVarTypeId(asUINT varIndex, asUINT stackLevel)
{
	asCScriptFunction *func = reinterpret_cast<asCScriptFunction*>(GetFunction(stackLevel));
	if( func == 0 ) return asINVALID_ARG;

	int typeId;
	int r = func->GetVar(varIndex, 0, &typeId);
	if (r < 0)
		return r;

	return typeId;
}
#endif

// interface
void *asCContext::GetAddressOfVar(asUINT varIndex, asUINT stackLevel, bool dontDereference, bool returnAddressOfUnitializedObjects)
{
	// Don't return anything if there is no bytecode, e.g. before calling Execute()
	if( m_regs.programPointer == 0 ) return 0;

	if( stackLevel >= GetCallstackSize() ) return 0;

	asCScriptFunction *func;
	asDWORD *sf;
	if( stackLevel == 0 )
	{
		func = m_currentFunction;
		sf = m_regs.stackFramePointer;
	}
	else
	{
		asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize()-stackLevel-1)*CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		sf = (asDWORD*)s[0];
	}

	if( func == 0 )
		return 0;

	if( func->scriptData == 0 )
		return 0;

	if( varIndex >= func->scriptData->variables.GetLength() )
		return 0;

	// For object variables it's necessary to dereference the pointer to get the address of the value
	// Reference parameters must also be dereferenced to give the address of the value
	int pos = func->scriptData->variables[varIndex]->stackOffset;
	if( (func->scriptData->variables[varIndex]->type.IsObject() && !func->scriptData->variables[varIndex]->type.IsObjectHandle()) || (pos <= 0) )
	{
		// Determine if the object is really on the heap
		bool onHeap = func->scriptData->variables[varIndex]->onHeap;
		if( func->scriptData->variables[varIndex]->type.IsObject() &&
			!func->scriptData->variables[varIndex]->type.IsObjectHandle() &&
			!func->scriptData->variables[varIndex]->type.IsReference() )
		{
			if( func->scriptData->variables[varIndex]->type.GetTypeInfo()->GetFlags() & asOBJ_VALUE )
			{
				if (!onHeap && !returnAddressOfUnitializedObjects)
				{
					// If the object on the stack is not initialized return a null pointer instead
					asCArray<int> liveObjects;
					DetermineLiveObjects(liveObjects, stackLevel);

					if (liveObjects[varIndex] <= 0)
						return 0;
				}
			}
		}

		// If it wasn't an object on the heap, then check if it is a reference parameter
		if( !onHeap && pos <= 0 )
		{
			if (func->scriptData->variables[varIndex]->type.IsReference())
				onHeap = true;
		}

		// If dontDereference is true then the application wants the address of the reference, rather than the value it refers to
		if( onHeap && !dontDereference )
			return *(void**)(sf - func->scriptData->variables[varIndex]->stackOffset);
	}

	return sf - func->scriptData->variables[varIndex]->stackOffset;
}

// interface
// returns the typeId of the 'this' object at the given call stack level (0 for current)
// returns 0 if the function call at the given stack level is not a method
int asCContext::GetThisTypeId(asUINT stackLevel)
{
	asIScriptFunction *func = GetFunction(stackLevel);
	if( func == 0 ) return asINVALID_ARG;

	if( func->GetObjectType() == 0 )
		return 0; // not in a method

	// create a datatype
	asCDataType dt = asCDataType::CreateType((asCObjectType*)func->GetObjectType(), false);

	// return a typeId from the data type
	return m_engine->GetTypeIdFromDataType(dt);
}

// interface
// returns the 'this' object pointer at the given call stack level (0 for current)
// returns 0 if the function call at the given stack level is not a method
void *asCContext::GetThisPointer(asUINT stackLevel)
{
	if( stackLevel >= GetCallstackSize() )
		return 0;

	asCScriptFunction *func;
	asDWORD *sf;
	if( stackLevel == 0 )
	{
		func = m_currentFunction;
		sf = m_regs.stackFramePointer;
	}
	else
	{
		asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize()-stackLevel-1)*CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		sf = (asDWORD*)s[0];
	}

	// sf is null if this is for a nested state
	if( sf == 0 || func == 0 || func->objectType == 0 )
		return 0;

	void *thisPointer = (void*)*(asPWORD*)(sf);
	if( thisPointer == 0 )
	{
		return 0;
	}

	// NOTE: this returns the pointer to the 'this' while the GetVarPointer functions return
	// a pointer to a pointer. I can't imagine someone would want to change the 'this'
	return thisPointer;
}

// interface
int asCContext::StartDeserialization()
{
	if( m_status == asEXECUTION_ACTIVE || m_status == asEXECUTION_SUSPENDED )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "StartDeserialization", errorNames[-asCONTEXT_ACTIVE], asCONTEXT_ACTIVE);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_ACTIVE;
	}

	Unprepare();
	m_status = asEXECUTION_DESERIALIZATION;

	return asSUCCESS;
}

// internal
int asCContext::DeserializeProgramPointer(int programPointer, asCScriptFunction *currentFunction, void *object, asDWORD *&p, asCScriptFunction *&realFunc)
{
	realFunc = currentFunction;

	if( currentFunction->funcType == asFUNC_VIRTUAL ||
		currentFunction->funcType == asFUNC_INTERFACE )
	{
		// The currentFunction is a virtual method

		// Determine the true function from the object
		asCScriptObject *obj = *(asCScriptObject**)(asPWORD*)object;

		if( obj == 0 )
		{
			return asINVALID_ARG;
		}
		else
		{
			realFunc = GetRealFunc(m_currentFunction, (void**)&obj);

			if( realFunc && realFunc->signatureId == m_currentFunction->signatureId )
				m_currentFunction = realFunc;
			else
				return asINVALID_ARG;
		}
	}

	if( currentFunction->funcType == asFUNC_SCRIPT )
	{
		// TODO: Instead of returning pointer, this should count number of instructions so that the deserialized program pointer is 32/64bit agnostic
		p = currentFunction->scriptData->byteCode.AddressOf() + programPointer;
	}

	return asSUCCESS;
}

// interface
int asCContext::FinishDeserialization()
{
	if( m_status != asEXECUTION_DESERIALIZATION )
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "FinishDeserialization", errorNames[-asCONTEXT_NOT_PREPARED], asCONTEXT_NOT_PREPARED);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return asCONTEXT_NOT_PREPARED;
	}

	// Sanity test
	if (m_currentFunction == 0)
	{
		asCString str;
		str.Format(TXT_FAILED_IN_FUNC_s_WITH_s_s_d, "FinishDeserialization", "No function set", errorNames[-asCONTEXT_NOT_PREPARED], asCONTEXT_NOT_PREPARED);
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());

		// Clean up before returning to leave the context in a valid state
		Unprepare();

		return asCONTEXT_NOT_PREPARED;
	}
	
	m_status = asEXECUTION_SUSPENDED;

	return asSUCCESS;
}

// internal
asDWORD *asCContext::DeserializeStackPointer(asDWORD v)
{
	// TODO: This function should find the correct stack block and then get the address within that stack block. It must not be expected that the same initContextStackSize was used when the stack pointer was serialized
	int block = (v >> (32-6)) & 0x3F;
	asDWORD offset = v & 0x03FFFFFF;

	asASSERT((asUINT)block < m_stackBlocks.GetLength());
	asASSERT(offset <= m_engine->ep.initContextStackSize*(1 << block));

	return m_stackBlocks[block] + offset;
}

// internal
asDWORD asCContext::SerializeStackPointer(asDWORD *v) const
{
	// TODO: This function should determine actual offset from the lowest stack by unwinding the stack. This way when deserializing it doesn't matter if the same block sizes are used or not
	asASSERT(v != 0);
	asASSERT(m_stackBlocks.GetLength());

	// Find the stack block that is used, and the offset into that block
	asUINT stackIndex = DetermineStackIndex(v);
	asASSERT(int(stackIndex) >= 0);
	if (stackIndex >= m_stackBlocks.GetLength()) 
		return asUINT(asERROR);
	asQWORD offset    = asQWORD(v - m_stackBlocks[stackIndex]);

	asASSERT(offset < 0x03FFFFFF && (asUINT)stackIndex < 0x3F);

	// Return the seriaized pointer as the offset in the lower 26 bits + the index of the stack block in the upper 6 bits
	return (offset & 0x03FFFFFF) | ((stackIndex & 0x3F) << (32-6));
}

// interface
int asCContext::GetArgsOnStackCount(asUINT stackLevel)
{
	// Clear cache
	m_argsOnStackCache.SetLength(0);
	m_argsOnStackCacheProgPos = 0;
	m_argsOnStackCacheFunc = 0;

	// Don't return anything if there is no bytecode, e.g. before calling Execute()
	if (m_regs.programPointer == 0) return asERROR;

	if (stackLevel >= GetCallstackSize()) return asINVALID_ARG;

	asCScriptFunction* func;
	asDWORD* sf;
	asDWORD* sp;
	asDWORD* progPointer;
	if (stackLevel == 0)
	{
		func = m_currentFunction;
		sf = m_regs.stackFramePointer;
		sp = m_regs.stackPointer;
		progPointer = m_regs.programPointer;
	}
	else
	{
		asPWORD* s = m_callStack.AddressOf() + (GetCallstackSize() - stackLevel - 1) * CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		sf = (asDWORD*)s[0];
		sp = (asDWORD*)s[3];
		progPointer = (asDWORD*)s[2];
	}

	// Determine the highest stack position for local variables
	// asCScriptFunction::variableSpace give this value
	// If the stack pointer is higher than that, then there are data pushed on the stack
	asUINT stackPos = asDWORD(sf - sp) - func->scriptData->variableSpace;
	if (stackPos == 0)
		return 0;

	// If a function is already being called at a higher call stack position, subtract the args for that function
	asCScriptFunction* calledFunc = 0;
	if (stackLevel == 1)
		calledFunc = m_currentFunction;
	else if( stackLevel > 1 )
	{
		asPWORD *s = m_callStack.AddressOf() + (GetCallstackSize() - stackLevel - 2) * CALLSTACK_FRAME_SIZE;
		calledFunc = (asCScriptFunction*)s[1];
	}
	if( calledFunc )
		stackPos -= calledFunc->GetSpaceNeededForArguments() + (calledFunc->DoesReturnOnStack() ? AS_PTR_SIZE : 0) + (calledFunc->GetObjectType() ? AS_PTR_SIZE : 0);

	// Cache the list of arg types by func pointer and program position
	m_argsOnStackCacheFunc = func;
	m_argsOnStackCacheProgPos = asUINT(progPointer - &func->scriptData->byteCode[0]);

	// Iteratively search for functions that will be called until all values on the arg has been determined
	asUINT progPos = asUINT(progPointer - &func->scriptData->byteCode[0]);
	while( stackPos > 0 )
	{
		// Find the next function that will be called to determine the arg types and sizes
		int stackDelta = 0;
		calledFunc = func->FindNextFunctionCalled(progPos, &stackDelta, &progPos);

		// Determine which args have not yet been pushed on the stack based on the stackDelta
		if (stackDelta > 0 && calledFunc->DoesReturnOnStack())
			stackDelta -= AS_PTR_SIZE;
		if (stackDelta > 0 && calledFunc->GetObjectType())
			stackDelta -= AS_PTR_SIZE;
		int param = -1;
		while (stackDelta > 0 && ++param < int(calledFunc->GetParamCount()))
		{
			int typeId;
			asDWORD flags;
			calledFunc->GetParam(param, &typeId, &flags);

			// TODO: When enums can be of different sizes this needs to be adjusted
			if ((flags & asTM_INOUTREF) || (typeId & asTYPEID_MASK_OBJECT))
				stackDelta -= AS_PTR_SIZE;
			else if (typeId == asTYPEID_UINT64 || typeId == asTYPEID_INT64 || typeId == asTYPEID_DOUBLE)
				stackDelta -= 2;
			else
				stackDelta -= 1;
		}

		// Determine the args already pushed on the stack
		while (stackPos > 0)
		{
			if (++param < int(calledFunc->GetParamCount()))
			{
				int typeId;
				asDWORD flags;
				calledFunc->GetParam(param, &typeId, &flags);

				if ((flags & asTM_INOUTREF) || (typeId & asTYPEID_MASK_OBJECT))
				{
					// TODO: The value pushed on the stack here can be just the offset of the variable, not the actual pointer
					stackPos -= AS_PTR_SIZE;
				}
				else if (typeId == asTYPEID_UINT64 || typeId == asTYPEID_INT64 || typeId == asTYPEID_DOUBLE)
					stackPos -= 2;
				else
					stackPos -= 1;
				m_argsOnStackCache.PushLast(typeId);
				m_argsOnStackCache.PushLast(flags);
				continue;
			}

			// There is no need to check for the this pointer or the pointer to the return value since the
			// context cannot be suspended between the moment these are pushed on the stack and the call itself.

			// There are no more args for this function, there is a nested call
			break;
		}
	}

	return m_argsOnStackCache.GetLength() / 2;
}

// interface
int asCContext::GetArgOnStack(asUINT stackLevel, asUINT arg, int* outTypeId, asUINT* outFlags, void** outAddress)
{
	// Don't return anything if there is no bytecode, e.g. before calling Execute()
	if (m_regs.programPointer == 0) return asERROR;

	if (stackLevel >= GetCallstackSize()) return asINVALID_ARG;

	asCScriptFunction* func;
	asDWORD* sp;
	asDWORD* progPointer;
	if (stackLevel == 0)
	{
		func = m_currentFunction;
		sp = m_regs.stackPointer;
		progPointer = m_regs.programPointer;
	}
	else
	{
		asPWORD* s = m_callStack.AddressOf() + (GetCallstackSize() - stackLevel - 1) * CALLSTACK_FRAME_SIZE;
		func = (asCScriptFunction*)s[1];
		sp = (asDWORD*)s[3];
		progPointer = (asDWORD*)s[2];
	}

	// If a function is already being called at a higher call stack position, subtract the args for that function
	asCScriptFunction* calledFunc = 0;
	if (stackLevel == 1)
		calledFunc = m_currentFunction;
	else if (stackLevel > 1)
	{
		asPWORD* s = m_callStack.AddressOf() + (GetCallstackSize() - stackLevel - 2) * CALLSTACK_FRAME_SIZE;
		calledFunc = (asCScriptFunction*)s[1];
	}
	if (calledFunc)
		sp += calledFunc->GetSpaceNeededForArguments() + (calledFunc->DoesReturnOnStack() ? AS_PTR_SIZE : 0) + (calledFunc->GetObjectType() ? AS_PTR_SIZE : 0);

	// Check that the cache for GetArgsOnStack is up-to-date
	if (m_argsOnStackCacheFunc != func || m_argsOnStackCacheProgPos != asUINT(progPointer - &func->scriptData->byteCode[0]))
		GetArgsOnStackCount(stackLevel);

	// The arg types in the array are stored from top to bottom, so we'll go through them in the inverse order
	// TODO: Check that arg is not too high
	arg = m_argsOnStackCache.GetLength() / 2 - arg - 1;
	asUINT stackDelta = 0;
	for (asUINT n = 0; n < arg; n++)
	{
		int typeId = m_argsOnStackCache[n * 2];
		asUINT flags = m_argsOnStackCache[n * 2 + 1];

		if ((flags & asTM_INOUTREF) || (typeId & asTYPEID_MASK_OBJECT))
			stackDelta += AS_PTR_SIZE;
		else if (typeId == asTYPEID_UINT64 || typeId == asTYPEID_INT64 || typeId == asTYPEID_DOUBLE)
			stackDelta += 2;
		else
			stackDelta += 1;
	}

	if (outAddress) *outAddress = sp + stackDelta;
	if (outTypeId) *outTypeId = m_argsOnStackCache[arg * 2];
	if (outFlags) *outFlags = m_argsOnStackCache[arg * 2 + 1];

	return asSUCCESS;
}

// TODO: Move these to as_utils.cpp

struct POW_INFO
{
	asQWORD MaxBaseu64;
	asDWORD MaxBasei64;
	asWORD  MaxBaseu32;
	asWORD  MaxBasei32;
	char    HighBit;
};

const POW_INFO pow_info[] =
{
	{          0ULL,          0UL,     0,     0, 0 },  // 0 is a special case
	{          0ULL,          0UL,     0,     0, 1 },  // 1 is a special case
	{ 3037000499ULL, 2147483647UL, 65535, 46340, 2 },  // 2
	{    2097152ULL,    1664510UL,  1625,  1290, 2 },  // 3
	{      55108ULL,      46340UL,   255,   215, 3 },  // 4
	{       6208ULL,       5404UL,    84,    73, 3 },  // 5
	{       1448ULL,       1290UL,    40,    35, 3 },  // 6
	{        511ULL,        463UL,    23,    21, 3 },  // 7
	{        234ULL,        215UL,    15,    14, 4 },  // 8
	{        128ULL,        118UL,    11,    10, 4 },  // 9
	{         78ULL,         73UL,     9,     8, 4 },  // 10
	{         52ULL,         49UL,     7,     7, 4 },  // 11
	{         38ULL,         35UL,     6,     5, 4 },  // 12
	{         28ULL,         27UL,     5,     5, 4 },  // 13
	{         22ULL,         21UL,     4,     4, 4 },  // 14
	{         18ULL,         17UL,     4,     4, 4 },  // 15
	{         15ULL,         14UL,     3,     3, 5 },  // 16
	{         13ULL,         12UL,     3,     3, 5 },  // 17
	{         11ULL,         10UL,     3,     3, 5 },  // 18
	{          9ULL,          9UL,     3,     3, 5 },  // 19
	{          8ULL,          8UL,     3,     2, 5 },  // 20
	{          8ULL,          7UL,     2,     2, 5 },  // 21
	{          7ULL,          7UL,     2,     2, 5 },  // 22
	{          6ULL,          6UL,     2,     2, 5 },  // 23
	{          6ULL,          5UL,     2,     2, 5 },  // 24
	{          5ULL,          5UL,     2,     2, 5 },  // 25
	{          5ULL,          5UL,     2,     2, 5 },  // 26
	{          5ULL,          4UL,     2,     2, 5 },  // 27
	{          4ULL,          4UL,     2,     2, 5 },  // 28
	{          4ULL,          4UL,     2,     2, 5 },  // 29
	{          4ULL,          4UL,     2,     2, 5 },  // 30
	{          4ULL,          4UL,     2,     1, 5 },  // 31
	{          3ULL,          3UL,     1,     1, 6 },  // 32
	{          3ULL,          3UL,     1,     1, 6 },  // 33
	{          3ULL,          3UL,     1,     1, 6 },  // 34
	{          3ULL,          3UL,     1,     1, 6 },  // 35
	{          3ULL,          3UL,     1,     1, 6 },  // 36
	{          3ULL,          3UL,     1,     1, 6 },  // 37
	{          3ULL,          3UL,     1,     1, 6 },  // 38
	{          3ULL,          3UL,     1,     1, 6 },  // 39
	{          2ULL,          2UL,     1,     1, 6 },  // 40
	{          2ULL,          2UL,     1,     1, 6 },  // 41
	{          2ULL,          2UL,     1,     1, 6 },  // 42
	{          2ULL,          2UL,     1,     1, 6 },  // 43
	{          2ULL,          2UL,     1,     1, 6 },  // 44
	{          2ULL,          2UL,     1,     1, 6 },  // 45
	{          2ULL,          2UL,     1,     1, 6 },  // 46
	{          2ULL,          2UL,     1,     1, 6 },  // 47
	{          2ULL,          2UL,     1,     1, 6 },  // 48
	{          2ULL,          2UL,     1,     1, 6 },  // 49
	{          2ULL,          2UL,     1,     1, 6 },  // 50
	{          2ULL,          2UL,     1,     1, 6 },  // 51
	{          2ULL,          2UL,     1,     1, 6 },  // 52
	{          2ULL,          2UL,     1,     1, 6 },  // 53
	{          2ULL,          2UL,     1,     1, 6 },  // 54
	{          2ULL,          2UL,     1,     1, 6 },  // 55
	{          2ULL,          2UL,     1,     1, 6 },  // 56
	{          2ULL,          2UL,     1,     1, 6 },  // 57
	{          2ULL,          2UL,     1,     1, 6 },  // 58
	{          2ULL,          2UL,     1,     1, 6 },  // 59
	{          2ULL,          2UL,     1,     1, 6 },  // 60
	{          2ULL,          2UL,     1,     1, 6 },  // 61
	{          2ULL,          2UL,     1,     1, 6 },  // 62
	{          2ULL,          1UL,     1,     1, 6 },  // 63
};

int as_powi(int base, int exponent, bool& isOverflow)
{
	if( exponent < 0 )
	{
		if( base == 0 )
			// Divide by zero
			isOverflow = true;
		else
			// Result is less than 1, so it truncates to 0
			isOverflow = false;

		return 0;
	}
	else if( exponent == 0 && base == 0 )
	{
		// Domain error
		isOverflow = true;
		return 0;
	}
	else if( exponent >= 31 )
	{
		switch( base )
		{
		case -1:
			isOverflow = false;
			return exponent & 1 ? -1 : 1;
		case 0:
			isOverflow = false;
			break;
		case 1:
			isOverflow = false;
			return 1;
		default:
			isOverflow = true;
			break;
		}
		return 0;
	}
	else
	{
		const asWORD max_base = pow_info[exponent].MaxBasei32;
		const char high_bit = pow_info[exponent].HighBit;
		if( max_base != 0 && max_base < (base < 0 ? -base : base) )
		{
			isOverflow = true;
			return 0;  // overflow
		}

		int result = 1;
		switch( high_bit )
		{
		case 5:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 4:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 3:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 2:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 1:
			if( exponent ) result *= base;
			FALLTHROUGH
		default:
			isOverflow = false;
			return result;
		}
	}
}

asDWORD as_powu(asDWORD base, asDWORD exponent, bool& isOverflow)
{
	if( exponent == 0 && base == 0 )
	{
		// Domain error
		isOverflow = true;
		return 0;
	}
	else if( exponent >= 32 )
	{
		switch( base )
		{
		case 0:
			isOverflow = false;
			break;
		case 1:
			isOverflow = false;
			return 1;
		default:
			isOverflow = true;
			break;
		}
		return 0;
	}
	else
	{
		const asWORD max_base = pow_info[exponent].MaxBaseu32;
		const char high_bit = pow_info[exponent].HighBit;
		if( max_base != 0 && max_base < base )
		{
			isOverflow = true;
			return 0;  // overflow
		}

		asDWORD result = 1;
		switch( high_bit )
		{
		case 5:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 4:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 3:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 2:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 1:
			if( exponent ) result *= base;
			FALLTHROUGH
		default:
			isOverflow = false;
			return result;
		}
	}
}

asINT64 as_powi64(asINT64 base, asINT64 exponent, bool& isOverflow)
{
	if( exponent < 0 )
	{
		if( base == 0 )
			// Divide by zero
			isOverflow = true;
		else
			// Result is less than 1, so it truncates to 0
			isOverflow = false;

		return 0;
	}
	else if( exponent == 0 && base == 0 )
	{
		// Domain error
		isOverflow = true;
		return 0;
	}
	else if( exponent >= 63 )
	{
		switch( base )
		{
		case -1:
			isOverflow = false;
			return exponent & 1 ? -1 : 1;
		case 0:
			isOverflow = false;
			break;
		case 1:
			isOverflow = false;
			return 1;
		default:
			isOverflow = true;
			break;
		}
		return 0;
	}
	else
	{
		const asDWORD max_base = pow_info[exponent].MaxBasei64;
		const char high_bit = pow_info[exponent].HighBit;
		if( max_base != 0 && max_base < (base < 0 ? -base : base) )
		{
			isOverflow = true;
			return 0;  // overflow
		}

		asINT64 result = 1;
		switch( high_bit )
		{
		case 6:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 5:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 4:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 3:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 2:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 1:
			if( exponent ) result *= base;
			FALLTHROUGH
		default:
			isOverflow = false;
			return result;
		}
	}
}

asQWORD as_powu64(asQWORD base, asQWORD exponent, bool& isOverflow)
{
	if( exponent == 0 && base == 0 )
	{
		// Domain error
		isOverflow = true;
		return 0;
	}
	else if( exponent >= 64 )
	{
		switch( base )
		{
		case 0:
			isOverflow = false;
			break;
		case 1:
			isOverflow = false;
			return 1;
		default:
			isOverflow = true;
			break;
		}
		return 0;
	}
	else
	{
		const asQWORD max_base = pow_info[exponent].MaxBaseu64;
		const char high_bit = pow_info[exponent].HighBit;
		if( max_base != 0 && max_base < base )
		{
			isOverflow = true;
			return 0;  // overflow
		}

		asQWORD result = 1;
		switch( high_bit )
		{
		case 6:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 5:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 4:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 3:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 2:
			if( exponent & 1 ) result *= base;
			exponent >>= 1;
			base *= base;
			FALLTHROUGH
		case 1:
			if( exponent ) result *= base;
			FALLTHROUGH
		default:
			isOverflow = false;
			return result;
		}
	}
}

END_AS_NAMESPACE



