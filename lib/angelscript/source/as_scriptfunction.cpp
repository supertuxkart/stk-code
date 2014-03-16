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
// as_scriptfunction.cpp
//
// A container for a compiled script function
//



#include "as_config.h"
#include "as_scriptfunction.h"
#include "as_tokendef.h"
#include "as_scriptengine.h"
#include "as_callfunc.h"
#include "as_bytecode.h"
#include "as_texts.h"
#include "as_scriptnode.h"
#include "as_builder.h"
#include "as_scriptcode.h"

#include <cstdlib> // qsort

BEGIN_AS_NAMESPACE

#ifdef AS_MAX_PORTABILITY

static void ScriptFunction_AddRef_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	self->AddRef();
}

static void ScriptFunction_Release_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	self->Release();
}

static void ScriptFunction_GetRefCount_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	*(int*)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void ScriptFunction_SetFlag_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	self->SetFlag();
}

static void ScriptFunction_GetFlag_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	*(bool*)gen->GetAddressOfReturnLocation() = self->GetFlag();
}

static void ScriptFunction_EnumReferences_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void ScriptFunction_ReleaseAllHandles_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *self = (asCScriptFunction*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

static void ScriptFunction_CreateDelegate_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *func = (asCScriptFunction*)gen->GetArgAddress(0);
	void *obj = gen->GetArgAddress(1);
	gen->SetReturnAddress(CreateDelegate(func, obj));
}

#endif


void RegisterScriptFunction(asCScriptEngine *engine)
{
	// Register the gc behaviours for the script functions
	int r = 0;
	UNUSED_VAR(r); // It is only used in debug mode
	engine->functionBehaviours.engine = engine;
	engine->functionBehaviours.flags = asOBJ_REF | asOBJ_GC | asOBJ_SCRIPT_FUNCTION;
	engine->functionBehaviours.name = "_builtin_function_";
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ADDREF, "void f()", asMETHOD(asCScriptFunction,AddRef), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASE, "void f()", asMETHOD(asCScriptFunction,Release), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(asCScriptFunction,GetRefCount), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_SETGCFLAG, "void f()", asMETHOD(asCScriptFunction,SetFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(asCScriptFunction,GetFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(asCScriptFunction,EnumReferences), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(asCScriptFunction,ReleaseAllHandles), asCALL_THISCALL, 0); asASSERT( r >= 0 );
#else
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptFunction_AddRef_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptFunction_Release_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptFunction_GetRefCount_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptFunction_SetFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptFunction_GetFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptFunction_EnumReferences_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptFunction_ReleaseAllHandles_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
#endif

	// Register the builtin function for creating delegates
	// This function returns a handle to the delegate, but since the type is not known at this time it is
	// registered to return a void then the return type is changed manually to the builtin function type
	// The name of the function is an invalid identifier so it cannot be invoked accidentally from the script
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterGlobalFunction("void f(int &in, int &in)", asFUNCTION(CreateDelegate), asCALL_CDECL); asASSERT( r >= 0 );
#else
	r = engine->RegisterGlobalFunction("void f(int &in, int &in)", asFUNCTION(ScriptFunction_CreateDelegate_Generic), asCALL_GENERIC); asASSERT( r >= 0 );
#endif

	// Rename the function so that it cannot be called manually by the script
	int idx = engine->registeredGlobalFuncs.GetIndex(engine->scriptFunctions[r]);
	engine->registeredGlobalFuncs.Erase(idx);
	engine->scriptFunctions[r]->name = DELEGATE_FACTORY;
	engine->registeredGlobalFuncs.Put(engine->scriptFunctions[r]);

	// Change the return type so the VM will know the function really returns a handle
	engine->scriptFunctions[r]->returnType = asCDataType::CreateObject(&engine->functionBehaviours, false);
	engine->scriptFunctions[r]->returnType.MakeHandle(true);
}

asCScriptFunction *CreateDelegate(asCScriptFunction *func, void *obj)
{
	if( func == 0 || obj == 0 )
	{
		// TODO: delegate: Should set script exception
		return 0;
	}

	// Create an instance of a asCScriptFunction with the type asFUNC_DELEGATE
	// The delegate shouldn't have a function id and is not added to the engine->scriptFunctions
	asCScriptFunction *delegate = asNEW(asCScriptFunction)(static_cast<asCScriptEngine*>(func->GetEngine()), 0, asFUNC_DELEGATE);
	if( delegate )
		delegate->MakeDelegate(func, obj);

	return delegate;
}

// internal
void asCScriptFunction::MakeDelegate(asCScriptFunction *func, void *obj)
{
	// Increase the reference of the function and object
	func->AddRef();
	funcForDelegate = func;

	func->GetEngine()->AddRefScriptObject(obj, func->GetObjectType());
	objForDelegate = obj;

	// The return type and parameters are copied from the delegated method to this object
	// TODO: optimize: Do we really need to copy? Whenever requested the delegate can simply return the delegated methods' info directly
	parameterTypes = func->parameterTypes;
	returnType     = func->returnType;
	inOutFlags     = func->inOutFlags;

	// The delegate doesn't own the parameters as it will only forward them to the real method
	// so the exception handler must not clean up the parameters for the delegate
	dontCleanUpOnException = true;
}

// interface
void *asCScriptFunction::GetDelegateObject() const
{
	return objForDelegate;
}

// interface
asIObjectType *asCScriptFunction::GetDelegateObjectType() const
{
	if( objForDelegate == 0 || funcForDelegate == 0 )
		return 0;

	return funcForDelegate->objectType;
}

// interface
asIScriptFunction *asCScriptFunction::GetDelegateFunction() const
{
	return funcForDelegate;
}

// internal
int asCScriptFunction::RegisterListPattern(const char *decl, asCScriptNode *listNodes)
{
	if( listNodes == 0 )
		return asINVALID_ARG;

	// Build the representation of the list pattern from the script nodes
	asSListPatternNode *node;
	listPattern = asNEW(asSListPatternNode)(asLPT_START);
	node = listPattern;

	// Recursively parse the child
	int r = ParseListPattern(node, decl, listNodes);

	node->next = asNEW(asSListPatternNode)(asLPT_END);

	return r;
}

// internal
int asCScriptFunction::ParseListPattern(asSListPatternNode *&target, const char *decl, asCScriptNode *listNodes)
{
	asSListPatternNode *node = target;

	listNodes = listNodes->firstChild;
	while( listNodes )
	{
		if( listNodes->nodeType == snIdentifier )
		{
			node->next = asNEW(asSListPatternNode)(asLPT_REPEAT);
			node = node->next;
		}
		else if( listNodes->nodeType == snDataType )
		{
			asCDataType dt;
			asCBuilder builder(engine, 0);
			asCScriptCode code;
			code.SetCode("", decl, 0, false);
			dt = builder.CreateDataTypeFromNode(listNodes, &code, engine->defaultNamespace, false, returnType.GetObjectType());

			node->next = asNEW(asSListPatternDataTypeNode)(dt);
			node = node->next;
		}
		else if( listNodes->nodeType == snListPattern )
		{
			node->next = asNEW(asSListPatternNode)(asLPT_START);
			node = node->next;

			// Recursively parse the child
			int r = ParseListPattern(node, decl, listNodes);
			if( r < 0 )
				return r;

			node->next = asNEW(asSListPatternNode)(asLPT_END);
			node = node->next;
		}
		else
		{
			// Unexpected token in the list, the parser shouldn't have allowed
			asASSERT( false );
			return -1;
		}

		listNodes = listNodes->next;
	}

	target = node;
	return 0;
}

// internal
asCScriptFunction::asCScriptFunction(asCScriptEngine *engine, asCModule *mod, asEFuncType _funcType)
{
	refCount.set(1);
	this->engine           = engine;
	this->scriptData       = 0;
	funcType               = _funcType;
	module                 = mod;
	objectType             = 0;
	name                   = "";
	isReadOnly             = false;
	isPrivate              = false;
	isFinal                = false;
	isOverride             = false;
	sysFuncIntf            = 0;
	signatureId            = 0;
	dontCleanUpOnException = false;
	vfTableIdx             = -1;
	gcFlag                 = false;
	userData               = 0;
	id                     = 0;
	accessMask             = 0xFFFFFFFF;
	isShared               = false;
	nameSpace              = engine->nameSpaces[0];
	objForDelegate         = 0;
	funcForDelegate        = 0;
	listPattern            = 0;

	if( funcType == asFUNC_SCRIPT )
		AllocateScriptFunctionData();

	// Notify the GC of script functions
	if( (funcType == asFUNC_SCRIPT && mod == 0) || (funcType == asFUNC_DELEGATE) )
		engine->gc.AddScriptObjectToGC(this, &engine->functionBehaviours);
}

void asCScriptFunction::AllocateScriptFunctionData()
{
	if( scriptData ) return;

	scriptData = asNEW(ScriptFunctionData);

	scriptData->stackNeeded      = 0;
	scriptData->variableSpace    = 0;
	scriptData->scriptSectionIdx = -1;
	scriptData->declaredAt       = 0;
	scriptData->jitFunction      = 0;
}

void asCScriptFunction::DeallocateScriptFunctionData()
{
	if( !scriptData ) return;

	for( asUINT n = 0; n < scriptData->variables.GetLength(); n++ )
		asDELETE(scriptData->variables[n],asSScriptVariable);
	scriptData->variables.SetLength(0);

	asDELETE(scriptData, ScriptFunctionData);
	scriptData = 0;
}

// internal
asCScriptFunction::~asCScriptFunction()
{
	// Imported functions are not reference counted, nor are dummy
	// functions that are allocated on the stack
	asASSERT( funcType == asFUNC_DUMMY    ||
		      funcType == asFUNC_IMPORTED ||
		      refCount.get() == 0         );

	// If the engine pointer is 0, then DestroyInternal has already been called and there is nothing more to do
	if( engine == 0 ) return;

	DestroyInternal();

	// Tell engine to free the function id. This will make it impossible to
	// refer to the function by id. Where this is done, it is quite possible
	// they will leak.
	if( funcType != -1 && funcType != asFUNC_IMPORTED && id )
		engine->FreeScriptFunctionId(id);
	id = 0;

	// Finally set the engine pointer to 0 because it must not be accessed again
	engine = 0;
}

// internal
void asCScriptFunction::DestroyInternal()
{
	// Clean up user data
	if( userData && engine->cleanFunctionFunc )
		engine->cleanFunctionFunc(this);
	userData = 0;

	// Release all references the function holds to other objects
	ReleaseReferences();
	parameterTypes.SetLength(0);
	returnType = asCDataType::CreatePrimitive(ttVoid, false);

	for( asUINT p = 0; p < defaultArgs.GetLength(); p++ )
		if( defaultArgs[p] )
			asDELETE(defaultArgs[p], asCString);
	defaultArgs.SetLength(0);

	if( sysFuncIntf )
		asDELETE(sysFuncIntf,asSSystemFunctionInterface);
	sysFuncIntf = 0;

	DeallocateScriptFunctionData();

	// Deallocate list pattern data
	while( listPattern )
	{
		asSListPatternNode *n = listPattern->next;
		asDELETE(listPattern, asSListPatternNode);
		listPattern = n;
	}
}

// interface
int asCScriptFunction::GetId() const
{
	return id;
}

// interface
int asCScriptFunction::AddRef() const
{
	gcFlag = false;
	asASSERT( funcType != asFUNC_IMPORTED );
	return refCount.atomicInc();
}

// interface
int asCScriptFunction::Release() const
{
	gcFlag = false;
	asASSERT( funcType != asFUNC_IMPORTED );
	int r = refCount.atomicDec();
	if( r == 0 &&
		funcType != asFUNC_FUNCDEF && // Funcdefs are treated as object types and will be deleted by ClearUnusedTypes()
		funcType != asFUNC_DUMMY )    // Dummy functions are allocated on the stack and cannot be deleted
		asDELETE(const_cast<asCScriptFunction*>(this),asCScriptFunction);

	return r;
}

// internal
void asCScriptFunction::Orphan(asIScriptModule *mod)
{
	if( mod && module == mod )
	{
		module = 0;
		if( funcType == asFUNC_SCRIPT && refCount.get() > 1 )
		{
			// This function is being orphaned, so notify the GC so it can check for circular references
			engine->gc.AddScriptObjectToGC(this, &engine->functionBehaviours);
		}
	}

	Release();
}

// interface
int asCScriptFunction::GetTypeId() const
{
	// This const cast is ok, the object won't be modified
	asCDataType dt = asCDataType::CreateFuncDef(const_cast<asCScriptFunction*>(this));
	return engine->GetTypeIdFromDataType(dt);
}

// interface
bool asCScriptFunction::IsCompatibleWithTypeId(int typeId) const
{
	asCDataType dt = engine->GetDataTypeFromTypeId(typeId);

	// Make sure the type is a function
	asCScriptFunction *func = dt.GetFuncDef();
	if( func == 0 )
		return false;

	if( !IsSignatureExceptNameEqual(func) )
		return false;

	// If this is a class method, then only return true if the object type is the same
	if( objectType != func->objectType )
		return false;

	return true;
}

// interface
const char *asCScriptFunction::GetModuleName() const
{
	if( module )
	{
		return module->name.AddressOf();
	}

	return 0;
}

// interface
asIScriptModule *asCScriptFunction::GetModule() const
{
	return module;
}

// interface
asIObjectType *asCScriptFunction::GetObjectType() const
{
	return objectType;
}

// interface
const char *asCScriptFunction::GetObjectName() const
{
	if( objectType )
		return objectType->GetName();

	return 0;
}

// interface
const char *asCScriptFunction::GetName() const
{
	return name.AddressOf();
}

// interface
const char *asCScriptFunction::GetNamespace() const
{
	return nameSpace->name.AddressOf();
}

// interface
bool asCScriptFunction::IsReadOnly() const
{
	return isReadOnly;
}

// interface
bool asCScriptFunction::IsPrivate() const
{
	return isPrivate;
}

// internal
int asCScriptFunction::GetSpaceNeededForArguments()
{
	// We need to check the size for each type
	int s = 0;
	for( asUINT n = 0; n < parameterTypes.GetLength(); n++ )
		s += parameterTypes[n].GetSizeOnStackDWords();

	return s;
}

// internal
int asCScriptFunction::GetSpaceNeededForReturnValue()
{
	return returnType.GetSizeOnStackDWords();
}

// internal
bool asCScriptFunction::DoesReturnOnStack() const
{
	if( returnType.GetObjectType() &&
		(returnType.GetObjectType()->flags & asOBJ_VALUE) &&
		!returnType.IsReference() )
		return true;

	return false;
}

// internal
asCString asCScriptFunction::GetDeclarationStr(bool includeObjectName, bool includeNamespace) const
{
	asCString str;

	// TODO: default arg: Make the declaration with the default args an option

	// Don't add the return type for constructors and destructors
	if( !(returnType.GetTokenType() == ttVoid &&
		  objectType &&
		  (name == objectType->name || (name.GetLength() > 0 && name[0] == '~') ||
		   name == "_beh_0_" || name == "_beh_2_")) )
	{
		str = returnType.Format();
		str += " ";
	}
	if( objectType && includeObjectName )
	{
		if( includeNamespace )
			str += objectType->nameSpace->name + "::";

		if( objectType->name != "" )
			str += objectType->name + "::";
		else
			str += "_unnamed_type_::";
	}
	else if( includeNamespace )
	{
		str += nameSpace->name + "::";
	}
	if( name == "" )
		str += "_unnamed_function_(";
	else if( name.SubString(0,5) == "_beh_" && name.GetLength() == 7 )
	{
		if( name[5] == '0' + asBEHAVE_CONSTRUCT )
			str += objectType->name + "(";
		else if( name[5] == '0' + asBEHAVE_FACTORY )
			str += returnType.GetObjectType()->name + "(";
		else if( name[5] == '0' + asBEHAVE_DESTRUCT )
			str += "~" + objectType->name + "(";
		else
			str += name + "(";
	}
	else
		str += name + "(";

	if( parameterTypes.GetLength() > 0 )
	{
		asUINT n;
		for( n = 0; n < parameterTypes.GetLength() - 1; n++ )
		{
			str += parameterTypes[n].Format();
			if( parameterTypes[n].IsReference() && inOutFlags.GetLength() > n )
			{
				if( inOutFlags[n] == asTM_INREF ) str += "in";
				else if( inOutFlags[n] == asTM_OUTREF ) str += "out";
				else if( inOutFlags[n] == asTM_INOUTREF ) str += "inout";
			}

			if( defaultArgs.GetLength() > n && defaultArgs[n] )
			{
				asCString tmp;
				tmp.Format(" = %s", defaultArgs[n]->AddressOf());
				str += tmp;
			}

			str += ", ";
		}

		// Add the last parameter
		str += parameterTypes[n].Format();
		if( parameterTypes[n].IsReference() && inOutFlags.GetLength() > n )
		{
			if( inOutFlags[n] == asTM_INREF ) str += "in";
			else if( inOutFlags[n] == asTM_OUTREF ) str += "out";
			else if( inOutFlags[n] == asTM_INOUTREF ) str += "inout";
		}

		if( defaultArgs.GetLength() > n && defaultArgs[n] )
		{
			asCString tmp;
			tmp.Format(" = %s", defaultArgs[n]->AddressOf());
			str += tmp;
		}
	}

	str += ")";

	if( isReadOnly )
		str += " const";

	// Add the declaration of the list pattern
	if( listPattern )
	{
		asSListPatternNode *n = listPattern;
		while( n )
		{
			if( n->type == asLPT_START )
				str += " {";
			else if( n->type == asLPT_END )
				str += " }";
			else if( n->type == asLPT_REPEAT )
				str += " repeat";
			else if( n->type == asLPT_TYPE )
			{
				str += " ";
				str += reinterpret_cast<asSListPatternDataTypeNode*>(n)->dataType.Format();
			}

			n = n->next;
		}
	}

	return str;
}

// interface
int asCScriptFunction::FindNextLineWithCode(int line) const
{
	if( scriptData == 0 ) return -1;
	if( scriptData->lineNumbers.GetLength() == 0 ) return -1;

	// The line numbers for constructors are not in order due to the way
	// class members can be initialized directly in the declaration
	if( objectType && objectType->name == name )
	{
		// Sort all line numbers before looking for the next
		asCArray<int> lineNbrs;
		for( asUINT n = 1; n < scriptData->lineNumbers.GetLength(); n += 2 )
			lineNbrs.PushLast(scriptData->lineNumbers[n]&0xFFFFF);

		struct C
		{
			static int cmp(const void *a, const void *b) { return *(int*)a - *(int*)b; }
		};
		qsort(&lineNbrs[0], lineNbrs.GetLength(), sizeof(int), C::cmp);

		if( line < lineNbrs[0] && line < (scriptData->declaredAt&0xFFFFF)) return -1;
		if( line > lineNbrs[lineNbrs.GetLength()-1] ) return -1;

		// Find the line with code on or right after the input line
		// TODO: optimize: Do binary search
		for( asUINT n = 0; n < lineNbrs.GetLength(); n++ )
			if( line <= lineNbrs[n] )
				return lineNbrs[n];
	}
	else
	{
		// Check if given line is outside function
		if( line < (scriptData->declaredAt&0xFFFFF) ) return -1;
		if( line > (scriptData->lineNumbers[scriptData->lineNumbers.GetLength()-1]&0xFFFFF) ) return -1;

		// Find the line with code on or right after the input line
		// TODO: optimize: Do binary search instead
		for( asUINT n = 1; n < scriptData->lineNumbers.GetLength(); n += 2 )
		{
			if( line <= (scriptData->lineNumbers[n]&0xFFFFF) )
				return (scriptData->lineNumbers[n]&0xFFFFF);
		}
	}

	return -1;
}

// internal
int asCScriptFunction::GetLineNumber(int programPosition, int *sectionIdx)
{
	asASSERT( scriptData );

	if( sectionIdx ) *sectionIdx = scriptData->scriptSectionIdx;
	if( scriptData->lineNumbers.GetLength() == 0 ) return 0;

	if( sectionIdx )
	{
		// Find the correct section index if the function is compiled from multiple sections
		// This array will be empty most of the time so we don't need a sofisticated algorithm to search it
		for( asUINT n = 0; n < scriptData->sectionIdxs.GetLength(); n += 2 )
		{
			if( scriptData->sectionIdxs[n] <= programPosition )
				*sectionIdx = scriptData->sectionIdxs[n+1];
		}
	}

	// Do a binary search in the buffer
	int max = (int)scriptData->lineNumbers.GetLength()/2 - 1;
	int min = 0;
	int i = max/2;

	for(;;)
	{
		if( scriptData->lineNumbers[i*2] < programPosition )
		{
			// Have we found the largest number < programPosition?
			if( max == i ) return scriptData->lineNumbers[i*2+1];
			if( scriptData->lineNumbers[i*2+2] > programPosition ) return scriptData->lineNumbers[i*2+1];

			min = i + 1;
			i = (max + min)/2;
		}
		else if( scriptData->lineNumbers[i*2] > programPosition )
		{
			// Have we found the smallest number > programPosition?
			if( min == i ) return scriptData->lineNumbers[i*2+1];

			max = i - 1;
			i = (max + min)/2;
		}
		else
		{
			// We found the exact position
			return scriptData->lineNumbers[i*2+1];
		}
	}
}

// interface
asEFuncType asCScriptFunction::GetFuncType() const
{
	return funcType;
}

// interface
asUINT asCScriptFunction::GetVarCount() const
{
	if( scriptData )
		return asUINT(scriptData->variables.GetLength());
	return 0;
}

// interface
int asCScriptFunction::GetVar(asUINT index, const char **name, int *typeId) const
{
	if( scriptData == 0 )
		return asNOT_SUPPORTED;
	if( index >= scriptData->variables.GetLength() )
		return asINVALID_ARG;

	if( name )
		*name = scriptData->variables[index]->name.AddressOf();
	if( typeId )
		*typeId = engine->GetTypeIdFromDataType(scriptData->variables[index]->type);

	return asSUCCESS;
}

// interface
const char *asCScriptFunction::GetVarDecl(asUINT index, bool includeNamespace) const
{
	if( scriptData == 0 || index >= scriptData->variables.GetLength() )
		return 0;

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = scriptData->variables[index]->type.Format(includeNamespace);
	*tempString += " " + scriptData->variables[index]->name;

	return tempString->AddressOf();
}

// internal
void asCScriptFunction::AddVariable(asCString &name, asCDataType &type, int stackOffset)
{
	asASSERT( scriptData );
	asSScriptVariable *var = asNEW(asSScriptVariable);
	if( var == 0 )
	{
		// Out of memory
		return;
	}
	var->name                 = name;
	var->type                 = type;
	var->stackOffset          = stackOffset;
	var->declaredAtProgramPos = 0;
	scriptData->variables.PushLast(var);
}

// internal
asCObjectType *asCScriptFunction::GetObjectTypeOfLocalVar(short varOffset)
{
	asASSERT( scriptData );

	for( asUINT n = 0; n < scriptData->objVariablePos.GetLength(); n++ )
	{
		if( scriptData->objVariablePos[n] == varOffset )
			return scriptData->objVariableTypes[n];
	}

	return 0;
}

// internal
void asCScriptFunction::ComputeSignatureId()
{
	// This function will compute the signatureId based on the
	// function name, return type, and parameter types. The object
	// type for methods is not used, so that class methods and
	// interface methods match each other.
	for( asUINT n = 0; n < engine->signatureIds.GetLength(); n++ )
	{
		if( !IsSignatureEqual(engine->signatureIds[n]) ) continue;

		// We don't need to increment the reference counter here, because
		// asCScriptEngine::FreeScriptFunctionId will maintain the signature
		// id as the function is freed.
		signatureId = engine->signatureIds[n]->signatureId;
		return;
	}

	signatureId = id;
	engine->signatureIds.PushLast(this);
}

// internal
bool asCScriptFunction::IsSignatureEqual(const asCScriptFunction *func) const
{
	if( !IsSignatureExceptNameEqual(func) || name != func->name ) return false;

	return true;
}

// internal
bool asCScriptFunction::IsSignatureExceptNameEqual(const asCScriptFunction *func) const
{
	return IsSignatureExceptNameEqual(func->returnType, func->parameterTypes, func->inOutFlags, func->objectType, func->isReadOnly);
}

// internal
bool asCScriptFunction::IsSignatureExceptNameEqual(const asCDataType &retType, const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &paramInOut, const asCObjectType *objType, bool readOnly) const
{
	if( this->returnType != retType ) return false;

	return IsSignatureExceptNameAndReturnTypeEqual(paramTypes, paramInOut, objType, readOnly);
}

// internal
bool asCScriptFunction::IsSignatureExceptNameAndObjectTypeEqual(const asCScriptFunction *func) const
{
	return IsSignatureExceptNameEqual(func->returnType, func->parameterTypes, func->inOutFlags, objectType, isReadOnly);
}

// internal
bool asCScriptFunction::IsSignatureExceptNameAndReturnTypeEqual(const asCScriptFunction *func) const
{
	return IsSignatureExceptNameAndReturnTypeEqual(func->parameterTypes, func->inOutFlags, func->objectType, func->isReadOnly);
}

// internal
bool asCScriptFunction::IsSignatureExceptNameAndReturnTypeEqual(const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &paramInOut, const asCObjectType *objType, bool readOnly) const
{
	if( this->isReadOnly        != readOnly       ) return false;
	if( this->inOutFlags        != paramInOut     ) return false;
	if( this->parameterTypes    != paramTypes     ) return false;
	if( (this->objectType != 0) != (objType != 0) ) return false;

	return true;
}

// internal
void asCScriptFunction::AddReferences()
{
	// This array will be used to make sure we only add the reference to the same resource once
	// This is especially important for global variables, as it expects the initialization function
	// to hold only one reference to the variable. However, if the variable is initialized through
	// the default constructor followed by the assignment operator we will have two references to
	// the variable in the function.
	asCArray<void*> ptrs;

	// Only count references if there is any bytecode
	if( scriptData && scriptData->byteCode.GetLength() )
	{
		if( returnType.IsObject() )
			returnType.GetObjectType()->AddRef();

		for( asUINT p = 0; p < parameterTypes.GetLength(); p++ )
			if( parameterTypes[p].IsObject() )
				parameterTypes[p].GetObjectType()->AddRef();

		for( asUINT v = 0; v < scriptData->objVariableTypes.GetLength(); v++ )
			scriptData->objVariableTypes[v]->AddRef();

		// Go through the byte code and add references to all resources used by the function
		asCArray<asDWORD> &bc = scriptData->byteCode;
		for( asUINT n = 0; n < bc.GetLength(); n += asBCTypeSize[asBCInfo[*(asBYTE*)&bc[n]].type] )
		{
			switch( *(asBYTE*)&bc[n] )
			{
			// Object types
			case asBC_OBJTYPE:
			case asBC_FREE:
			case asBC_REFCPY:
			case asBC_RefCpyV:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					objType->AddRef();
				}
				break;

			// Object type and function
			case asBC_ALLOC:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					objType->AddRef();

					int func = asBC_INTARG(&bc[n]+AS_PTR_SIZE);
					if( func )
						engine->scriptFunctions[func]->AddRef();
				}
				break;

			// Global variables
			case asBC_PGA:
			case asBC_PshGPtr:
			case asBC_LDG:
			case asBC_PshG4:
			case asBC_LdGRdR4:
			case asBC_CpyGtoV4:
			case asBC_CpyVtoG4:
			case asBC_SetG4:
				// Need to increase the reference for each global variable
				{
					void *gvarPtr = (void*)asBC_PTRARG(&bc[n]);
					if( !gvarPtr ) break;
					asCGlobalProperty *prop = GetPropertyByGlobalVarPtr(gvarPtr);
					if( !prop ) break;

					// Only addref the properties once
					if( !ptrs.Exists(gvarPtr) )
					{
						prop->AddRef();
						ptrs.PushLast(gvarPtr);
					}

					asCConfigGroup *group = engine->FindConfigGroupForGlobalVar(prop->id);
					if( group != 0 ) group->AddRef();
				}
				break;

			// System functions
			case asBC_CALLSYS:
				{
					int funcId = asBC_INTARG(&bc[n]);
					asCConfigGroup *group = engine->FindConfigGroupForFunction(funcId);
					if( group != 0 ) group->AddRef();

					engine->scriptFunctions[funcId]->AddRef();
				}
				break;

			// Functions
			case asBC_CALL:
			case asBC_CALLINTF:
				{
					int func = asBC_INTARG(&bc[n]);
					engine->scriptFunctions[func]->AddRef();
				}
				break;

			// Function pointers
			case asBC_FuncPtr:
				{
					asCScriptFunction *func = (asCScriptFunction*)asBC_PTRARG(&bc[n]);
					func->AddRef();
				}
				break;
			}
		}
	}
}

// internal
void asCScriptFunction::ReleaseReferences()
{
	asCArray<void*> ptrs;

	// Only count references if there is any bytecode
	if( scriptData && scriptData->byteCode.GetLength() )
	{
		if( returnType.IsObject() )
			returnType.GetObjectType()->Release();

		for( asUINT p = 0; p < parameterTypes.GetLength(); p++ )
			if( parameterTypes[p].IsObject() )
				parameterTypes[p].GetObjectType()->Release();

		for( asUINT v = 0; v < scriptData->objVariableTypes.GetLength(); v++ )
			if( scriptData->objVariableTypes[v] )
				scriptData->objVariableTypes[v]->Release();

		// Go through the byte code and release references to all resources used by the function
		asCArray<asDWORD> &bc = scriptData->byteCode;
		for( asUINT n = 0; n < bc.GetLength(); n += asBCTypeSize[asBCInfo[*(asBYTE*)&bc[n]].type] )
		{
			switch( *(asBYTE*)&bc[n] )
			{
			// Object types
			case asBC_OBJTYPE:
			case asBC_FREE:
			case asBC_REFCPY:
			case asBC_RefCpyV:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					if( objType )
						objType->Release();
				}
				break;

			// Object type and function
			case asBC_ALLOC:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					if( objType )
						objType->Release();

					int func = asBC_INTARG(&bc[n]+AS_PTR_SIZE);
					if( func )
					{
						asCScriptFunction *fptr = engine->scriptFunctions[func];
						if( fptr )
							fptr->Release();

						// The engine may have been forced to destroy the function internals early
						// and this may will make it impossible to find the function by id anymore.
						// This should only happen if the engine is released while the application
						// is still keeping functions alive.
						// TODO: Fix this possible memory leak
					}
				}
				break;

			// Global variables
			case asBC_PGA:
			case asBC_PshGPtr:
			case asBC_LDG:
			case asBC_PshG4:
			case asBC_LdGRdR4:
			case asBC_CpyGtoV4:
			case asBC_CpyVtoG4:
			case asBC_SetG4:
				// Need to increase the reference for each global variable
				{
					void *gvarPtr = (void*)asBC_PTRARG(&bc[n]);
					if( !gvarPtr ) break;
					asCGlobalProperty *prop = GetPropertyByGlobalVarPtr(gvarPtr);
					if( !prop ) break;

					// Only release the properties once
					if( !ptrs.Exists(gvarPtr) )
					{
						prop->Release();
						ptrs.PushLast(gvarPtr);
					}

					asCConfigGroup *group = engine->FindConfigGroupForGlobalVar(prop->id);
					if( group != 0 ) group->Release();
				}
				break;

			// System functions
			case asBC_CALLSYS:
				{
					int funcId = asBC_INTARG(&bc[n]);
					asCConfigGroup *group = engine->FindConfigGroupForFunction(funcId);
					if( group != 0 ) group->Release();

					if( funcId )
						engine->scriptFunctions[funcId]->Release();
				}
				break;

			// Functions
			case asBC_CALL:
			case asBC_CALLINTF:
				{
					int func = asBC_INTARG(&bc[n]);
					if( func )
					{
						asCScriptFunction *fptr = engine->scriptFunctions[func];
						if( fptr )
							fptr->Release();

						// The engine may have been forced to destroy the function internals early
						// and this may will make it impossible to find the function by id anymore.
						// This should only happen if the engine is released while the application
						// is still keeping functions alive.
						// TODO: Fix this possible memory leak
					}
				}
				break;

			// Function pointers
			case asBC_FuncPtr:
				{
					asCScriptFunction *func = (asCScriptFunction*)asBC_PTRARG(&bc[n]);
					if( func )
						func->Release();
				}
				break;
			}
		}

		// Release the jit compiled function
		if( scriptData->jitFunction )
			engine->jitCompiler->ReleaseJITFunction(scriptData->jitFunction);
		scriptData->jitFunction = 0;
	}

	// Delegate
	if( objForDelegate )
		engine->ReleaseScriptObject(objForDelegate, funcForDelegate->GetObjectType());
	objForDelegate = 0;
	if( funcForDelegate )
		funcForDelegate->Release();
	funcForDelegate = 0;
}

// interface
int asCScriptFunction::GetReturnTypeId(asDWORD *flags) const
{
	if( flags )
	{
		if( returnType.IsReference() )
		{
			*flags = asTM_INOUTREF;
			*flags |= returnType.IsReadOnly() ? asTM_CONST : 0;
		}
		else
			*flags = asTM_NONE;
	}

	return engine->GetTypeIdFromDataType(returnType);
}

// interface
asUINT asCScriptFunction::GetParamCount() const
{
	return (asUINT)parameterTypes.GetLength();
}

// interface
int asCScriptFunction::GetParamTypeId(asUINT index, asDWORD *flags) const
{
	if( index >= parameterTypes.GetLength() )
		return asINVALID_ARG;

	if( flags )
	{
		*flags = inOutFlags[index];
		*flags |= parameterTypes[index].IsReadOnly() ? asTM_CONST : 0;
	}

	return engine->GetTypeIdFromDataType(parameterTypes[index]);
}

// interface
asIScriptEngine *asCScriptFunction::GetEngine() const
{
	return engine;
}

// interface
const char *asCScriptFunction::GetDeclaration(bool includeObjectName, bool includeNamespace) const
{
	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = GetDeclarationStr(includeObjectName, includeNamespace);
	return tempString->AddressOf();
}

// interface
const char *asCScriptFunction::GetScriptSectionName() const
{
	if( scriptData && scriptData->scriptSectionIdx >= 0 )
		return engine->scriptSectionNames[scriptData->scriptSectionIdx]->AddressOf();

	return 0;
}

// interface
const char *asCScriptFunction::GetConfigGroup() const
{
	asCConfigGroup *group = 0;
	if( funcType != asFUNC_FUNCDEF )
		group = engine->FindConfigGroupForFunction(id);
	else
		group = engine->FindConfigGroupForFuncDef(this);

	if( group == 0 )
		return 0;

	return group->groupName.AddressOf();
}

// interface
asDWORD asCScriptFunction::GetAccessMask() const
{
	return accessMask;
}

// internal
void asCScriptFunction::JITCompile()
{
	if( funcType != asFUNC_SCRIPT )
		return;

	asASSERT( scriptData );

	asIJITCompiler *jit = engine->GetJITCompiler();
	if( !jit )
		return;

	// Release the previous function, if any
	if( scriptData->jitFunction )
	{
		engine->jitCompiler->ReleaseJITFunction(scriptData->jitFunction);
		scriptData->jitFunction = 0;
	}

	// Compile for native system
	int r = jit->CompileFunction(this, &scriptData->jitFunction);
	if( r < 0 )
	{
		asASSERT( scriptData->jitFunction == 0 );
	}
}

// interface
asDWORD *asCScriptFunction::GetByteCode(asUINT *length)
{
	if( scriptData == 0 ) return 0;

	if( length )
		*length = (asUINT)scriptData->byteCode.GetLength();

	if( scriptData->byteCode.GetLength() )
		return scriptData->byteCode.AddressOf();

	return 0;
}

// interface
void *asCScriptFunction::SetUserData(void *data)
{
	void *oldData = userData;
	userData = data;
	return oldData;
}

// interface
void *asCScriptFunction::GetUserData() const
{
	return userData;
}

// internal
// TODO: cleanup: This method should probably be a member of the engine
asCGlobalProperty *asCScriptFunction::GetPropertyByGlobalVarPtr(void *gvarPtr)
{
	asSMapNode<void*, asCGlobalProperty*> *node;
	if( engine->varAddressMap.MoveTo(&node, gvarPtr) )
	{
		asASSERT(gvarPtr == node->value->GetAddressOfValue());
		return node->value;
	}
	return 0;
}

// internal
int asCScriptFunction::GetRefCount()
{
	return refCount.get();
}

// internal
void asCScriptFunction::SetFlag()
{
	gcFlag = true;
}

// internal
bool asCScriptFunction::GetFlag()
{
	return gcFlag;
}

// internal
void asCScriptFunction::EnumReferences(asIScriptEngine *)
{
	// Notify the GC of all object types used
	if( returnType.IsObject() )
		engine->GCEnumCallback(returnType.GetObjectType());

	for( asUINT p = 0; p < parameterTypes.GetLength(); p++ )
		if( parameterTypes[p].IsObject() )
			engine->GCEnumCallback(parameterTypes[p].GetObjectType());

	if( scriptData )
	{
		for( asUINT t = 0; t < scriptData->objVariableTypes.GetLength(); t++ )
			engine->GCEnumCallback(scriptData->objVariableTypes[t]);

		// Notify the GC of all script functions that is accessed
		asCArray<asDWORD> &bc = scriptData->byteCode;
		for( asUINT n = 0; n < bc.GetLength(); n += asBCTypeSize[asBCInfo[*(asBYTE*)&bc[n]].type] )
		{
			switch( *(asBYTE*)&bc[n] )
			{
			case asBC_OBJTYPE:
			case asBC_FREE:
			case asBC_REFCPY:
			case asBC_RefCpyV:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					engine->GCEnumCallback(objType);
				}
				break;

			case asBC_ALLOC:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					engine->GCEnumCallback(objType);

					int func = asBC_INTARG(&bc[n]+AS_PTR_SIZE);
					if( func )
						engine->GCEnumCallback(engine->scriptFunctions[func]);
				}
				break;

			case asBC_CALL:
			case asBC_CALLINTF:
				{
					int func = asBC_INTARG(&bc[n]);
					if( func )
						engine->GCEnumCallback(engine->scriptFunctions[func]);
				}
				break;

			// Function pointers
			case asBC_FuncPtr:
				{
					asCScriptFunction *func = (asCScriptFunction*)asBC_PTRARG(&bc[n]);
					if( func )
						engine->GCEnumCallback(func);
				}
				break;

			// Global variables
			case asBC_PGA:
			case asBC_PshGPtr:
			case asBC_LDG:
			case asBC_PshG4:
			case asBC_LdGRdR4:
			case asBC_CpyGtoV4:
			case asBC_CpyVtoG4:
			case asBC_SetG4:
				// Need to enumerate the reference for each global variable
				{
					// TODO: optimize: Keep an array of accessed global properties
					void *gvarPtr = (void*)asBC_PTRARG(&bc[n]);
					asCGlobalProperty *prop = GetPropertyByGlobalVarPtr(gvarPtr);

					engine->GCEnumCallback(prop);
				}
				break;
			}
		}
	}

	// Delegate
	if( objForDelegate )
		engine->GCEnumCallback(objForDelegate);
	if( funcForDelegate )
		engine->GCEnumCallback(funcForDelegate);
}

// internal
void asCScriptFunction::ReleaseAllHandles(asIScriptEngine *)
{
	// Release paramaters
	if( scriptData && scriptData->byteCode.GetLength() )
	{
		if( returnType.IsObject() )
		{
			returnType.GetObjectType()->Release();
			returnType = asCDataType::CreatePrimitive(ttVoid, false);
		}

		for( asUINT p = 0; p < parameterTypes.GetLength(); p++ )
			if( parameterTypes[p].IsObject() )
			{
				parameterTypes[p].GetObjectType()->Release();
				parameterTypes[p] = asCDataType::CreatePrimitive(ttInt, false);
			}

		for( asUINT n = 0; n < scriptData->objVariableTypes.GetLength(); n++ )
			scriptData->objVariableTypes[n]->Release();
		scriptData->objVariableTypes.SetLength(0);

		// Release all script functions
		asCArray<asDWORD> &bc = scriptData->byteCode;
		for( asUINT n = 0; n < bc.GetLength(); n += asBCTypeSize[asBCInfo[*(asBYTE*)&bc[n]].type] )
		{
			switch( *(asBYTE*)&bc[n] )
			{
			// Object types
			case asBC_OBJTYPE:
			case asBC_FREE:
			case asBC_REFCPY:
			case asBC_RefCpyV:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					if( objType )
					{
						objType->Release();
						*(asPWORD*)&bc[n+1] = 0;
					}
				}
				break;

			case asBC_ALLOC:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					if( objType )
					{
						objType->Release();
						*(asPWORD*)&bc[n+1] = 0;
					}

					int func = asBC_INTARG(&bc[n]+AS_PTR_SIZE);
					if( func )
					{
						engine->scriptFunctions[func]->Release();
						bc[n+AS_PTR_SIZE+1] = 0;
					}
				}
				break;

			case asBC_CALL:
			case asBC_CALLINTF:
				{
					int func = asBC_INTARG(&bc[n]);
					if( func )
					{
						engine->scriptFunctions[func]->Release();
						bc[n+1] = 0;
					}
				}
				break;

			// Function pointers
			case asBC_FuncPtr:
				{
					asCScriptFunction *func = (asCScriptFunction*)asBC_PTRARG(&bc[n]);
					if( func )
					{
						func->Release();
						*(asPWORD*)&bc[n+1] = 0;
					}
				}
				break;

			// The global variables are not released here. It is enough that the global
			// variable itself release the function to break the circle
			}
		}
	}

	// Delegate
	if( objForDelegate )
		engine->ReleaseScriptObject(objForDelegate, funcForDelegate->GetObjectType());
	objForDelegate = 0;
	if( funcForDelegate )
		funcForDelegate->Release();
	funcForDelegate = 0;
}

// internal
bool asCScriptFunction::IsShared() const
{
	// All system functions are shared
	if( funcType == asFUNC_SYSTEM ) return true;

	// All class methods for shared classes are also shared
	if( objectType && (objectType->flags & asOBJ_SHARED) ) return true;

	// Functions that have been specifically marked as shared are shared
	return isShared;
}

// internal
bool asCScriptFunction::IsFinal() const
{
	return isFinal;
}

// internal
bool asCScriptFunction::IsOverride() const
{
	return isOverride;
}

END_AS_NAMESPACE

