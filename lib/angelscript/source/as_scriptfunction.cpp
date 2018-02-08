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

// TODO: operator==
/*static void ScriptFunction_opEquals_Generic(asIScriptGeneric *gen)
{
	asCScriptFunction *funcSelf = (asCScriptFunction*)gen->GetObject();
	asCScriptFunction *funcOther = (asCScriptFunction*)gen->GetArgAddress(0);
	*(bool*)gen->GetAddressOfReturnLocation() = *funcSelf == *funcOther;
}
*/

#endif


void RegisterScriptFunction(asCScriptEngine *engine)
{
	// Register the gc behaviours for the script functions
	int r = 0;
	UNUSED_VAR(r); // It is only used in debug mode
	engine->functionBehaviours.engine = engine;
	engine->functionBehaviours.flags = asOBJ_REF | asOBJ_GC;
	engine->functionBehaviours.name = "$func";
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ADDREF, "void f()", asMETHOD(asCScriptFunction,AddRef), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASE, "void f()", asMETHOD(asCScriptFunction,Release), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(asCScriptFunction,GetRefCount), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_SETGCFLAG, "void f()", asMETHOD(asCScriptFunction,SetFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(asCScriptFunction,GetFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(asCScriptFunction,EnumReferences), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(asCScriptFunction,ReleaseAllHandles), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	// TODO: Need some way to allow the arg type to adapt when the funcdefs are instantiated
//	r = engine->RegisterMethodToObjectType(&engine->functionBehaviours, "bool opEquals(const int &in)", asMETHOD(asCScriptFunction,operator==), asCALL_THISCALL); asASSERT( r >= 0 );
#else
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptFunction_AddRef_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptFunction_Release_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptFunction_GetRefCount_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptFunction_SetFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptFunction_GetFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptFunction_EnumReferences_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->functionBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptFunction_ReleaseAllHandles_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
//	r = engine->RegisterMethodToObjectType(&engine->functionBehaviours, "bool opEquals(const int &in)", asFUNCTION(ScriptFunction_opEquals_Generic), asCALL_GENERIC); asASSERT( r >= 0 );
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
	engine->scriptFunctions[r]->returnType = asCDataType::CreateType(&engine->functionBehaviours, false);
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
void *asCScriptFunction::GetAuxiliary() const
{
	if (sysFuncIntf)
		return sysFuncIntf->auxiliary;

	return 0;
}

// interface
void *asCScriptFunction::GetDelegateObject() const
{
	return objForDelegate;
}

// interface
asITypeInfo *asCScriptFunction::GetDelegateObjectType() const
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

// TODO: operator==
/*
// internal
bool asCScriptFunction::operator==(const asCScriptFunction &other) const
{
	if( this == &other ) return true;

	if( this->funcType == asFUNC_DELEGATE && other.funcType == asFUNC_DELEGATE )
	{
		if( this->objForDelegate == other.objForDelegate &&
			this->funcForDelegate == other.funcForDelegate )
			return true;
	}

	return false;
}
*/

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
			asCString token(&decl[listNodes->tokenPos], listNodes->tokenLength);
			if( token == "repeat" )
			{
				node->next = asNEW(asSListPatternNode)(asLPT_REPEAT);
				node = node->next;
			}
			else if( token == "repeat_same" )
			{
				// TODO: list: Should make sure this is a sub-list
				node->next = asNEW(asSListPatternNode)(asLPT_REPEAT_SAME);
				node = node->next;
			}
			else
			{
				// Shouldn't happen as the parser already reported the error
				asASSERT(false);
			}
		}
		else if( listNodes->nodeType == snDataType )
		{
			asCDataType dt;
			asCBuilder builder(engine, 0);
			asCScriptCode code;
			code.SetCode("", decl, 0, false);
			dt = builder.CreateDataTypeFromNode(listNodes, &code, engine->defaultNamespace, false, CastToObjectType(returnType.GetTypeInfo()));

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
	funcType               = _funcType;
	if( funcType == asFUNC_DELEGATE )
	{
		// Delegates behave like object instances, rather than script code
		externalRefCount.set(1);
		internalRefCount.set(0);
	}
	else
	{
		internalRefCount.set(1);
		externalRefCount.set(0);
	}

	this->engine           = engine;
	this->scriptData       = 0;
	module                 = mod;
	objectType             = 0;
	name                   = "";
	sysFuncIntf            = 0;
	signatureId            = 0;
	dontCleanUpOnException = false;
	vfTableIdx             = -1;
	gcFlag                 = false;
	userData               = 0;
	id                     = 0;
	accessMask             = 0xFFFFFFFF;
	nameSpace              = engine->nameSpaces[0];
	objForDelegate         = 0;
	funcForDelegate        = 0;
	listPattern            = 0;
	funcdefType            = 0;

	if( funcType == asFUNC_SCRIPT )
		AllocateScriptFunctionData();

	// Notify the GC of delegates
	if( funcType == asFUNC_DELEGATE )
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
	// Dummy functions that are allocated on the stack are not reference counted
	asASSERT( funcType == asFUNC_DUMMY    ||
		      (externalRefCount.get() == 0 && internalRefCount.get() == 0) );

	// Remove the script function from the engine's scriptFunctions array here
	// Don't remove it before, because there may still be functions referring to it
	// by index until now. If it was removed in DestroyInternal, those would not
	// be able to release the refcount, thus causing memory leak.
	if( engine && id != 0 && funcType != asFUNC_DUMMY )
		engine->RemoveScriptFunction(this);

	// If the engine pointer is 0, then DestroyInternal has already been called and there is nothing more to do
	if( engine == 0 ) return;

	DestroyInternal();

	// Finally set the engine pointer to 0 because it must not be accessed again
	engine = 0;
}

// internal
void asCScriptFunction::DestroyHalfCreated()
{
	asASSERT( externalRefCount.get() == 0 && internalRefCount.get() == 1 );

	// Set the funcType to dummy so the destructor won't complain
	funcType = asFUNC_DUMMY;

	// If the bytecode exist remove it before destroying, otherwise it
	// will fail when the destructor releases the references as the bytecode
	// is not fully constructed.
	if( scriptData )
		scriptData->byteCode.SetLength(0);

	asDELETE(this, asCScriptFunction);
}

// internal
void asCScriptFunction::DestroyInternal()
{
	// Clean up user data
	for( asUINT n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n+1] )
		{
			for( asUINT c = 0; c < engine->cleanFunctionFuncs.GetLength(); c++ )
				if( engine->cleanFunctionFuncs[c].type == userData[n] )
					engine->cleanFunctionFuncs[c].cleanFunc(this);
		}
	}
	userData.SetLength(0);

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

	if( objectType )
	{
		objectType->ReleaseInternal();
		objectType = 0;
	}

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
	return externalRefCount.atomicInc();
}

// interface
int asCScriptFunction::Release() const
{
	gcFlag = false;
	int r = externalRefCount.atomicDec();
	if( r == 0 &&
		funcType != asFUNC_DUMMY )    // Dummy functions are allocated on the stack and cannot be deleted
	{
		// There are no more external references, if there are also no
		// internal references then it is time to delete the function
		if( internalRefCount.get() == 0 )
		{
			// If there are no internal references, then no module is owning the function
			// For example if the function was dynamically compiled without adding it to the scope of the module
			asASSERT( module == 0 );

			asDELETE(const_cast<asCScriptFunction*>(this),asCScriptFunction);
		}
	}

	return r;
}

// internal
int asCScriptFunction::AddRefInternal()
{
	return internalRefCount.atomicInc();
}

// internal
int asCScriptFunction::ReleaseInternal()
{
	int r = internalRefCount.atomicDec();
	if( r == 0 &&
		funcType != asFUNC_DUMMY )
	{
		// There are no more internal references, if there are also no
		// external references then it is time to delete the function
		if( externalRefCount.get() == 0 )
		{
			asDELETE(const_cast<asCScriptFunction*>(this),asCScriptFunction);
		}
	}

	return r;
}

// interface
int asCScriptFunction::GetTypeId() const
{
	// This const cast is ok, the object won't be modified
	asCDataType dt = asCDataType::CreateType(engine->FindMatchingFuncdef(const_cast<asCScriptFunction*>(this), 0), false);
	return engine->GetTypeIdFromDataType(dt);
}

// interface
bool asCScriptFunction::IsCompatibleWithTypeId(int typeId) const
{
	asCDataType dt = engine->GetDataTypeFromTypeId(typeId);

	// Make sure the type is a function
	if (!dt.IsFuncdef())
		return false;

	asCScriptFunction *func = CastToFuncdefType(dt.GetTypeInfo())->funcdef;
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
asITypeInfo *asCScriptFunction::GetObjectType() const
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
	if (nameSpace)
		return nameSpace->name.AddressOf();

	return 0;
}

// interface
bool asCScriptFunction::IsReadOnly() const
{
	return traits.GetTrait(asTRAIT_CONST);
}

// interface
bool asCScriptFunction::IsPrivate() const
{
	return traits.GetTrait(asTRAIT_PRIVATE);
}

// interface
bool asCScriptFunction::IsProtected() const
{
	return traits.GetTrait(asTRAIT_PROTECTED);
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
	if( returnType.GetTypeInfo() &&
		(returnType.GetTypeInfo()->flags & asOBJ_VALUE) &&
		!returnType.IsReference() )
		return true;

	return false;
}

// internal
asCString asCScriptFunction::GetDeclarationStr(bool includeObjectName, bool includeNamespace, bool includeParamNames) const
{
	asCString str;

	// TODO: default arg: Make the declaration with the default args an option

	// Don't add the return type for constructors and destructors
	if( !(returnType.GetTokenType() == ttVoid &&
		  objectType &&
		  (name == objectType->name || (name.GetLength() > 0 && name[0] == '~') ||
		   name == "$beh0" || name == "$beh2")) )
	{
		str = returnType.Format(nameSpace, includeNamespace);
		str += " ";
	}
	if( objectType && includeObjectName )
	{
		if( includeNamespace && objectType->nameSpace->name != "" )
			str += objectType->nameSpace->name + "::";

		if( objectType->name != "" )
			str += objectType->name + "::";
		else
			str += "_unnamed_type_::";
	}
	else if (funcdefType && funcdefType->parentClass && includeObjectName)
	{
		if (includeNamespace && funcdefType->parentClass->nameSpace->name != "")
			str += funcdefType->parentClass->nameSpace->name + "::";

		if (funcdefType->parentClass->name != "")
			str += funcdefType->parentClass->name + "::";
		else
			str += "_unnamed_type_::";
	}
	else if( includeNamespace && nameSpace->name != "" )
	{
		str += nameSpace->name + "::";
	}
	if( name == "" )
		str += "_unnamed_function_(";
	else if( name.SubString(0,4) == "$beh" && name.GetLength() == 5 )
	{
		if( name[4] == '0' + asBEHAVE_CONSTRUCT )
			str += objectType->name + "(";
		else if( name[4] == '0' + asBEHAVE_FACTORY )
			str += returnType.GetTypeInfo()->name + "(";
		else if( name[4] == '0' + asBEHAVE_DESTRUCT )
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
			str += parameterTypes[n].Format(nameSpace, includeNamespace);
			if( parameterTypes[n].IsReference() && inOutFlags.GetLength() > n )
			{
				if( inOutFlags[n] == asTM_INREF ) str += "in";
				else if( inOutFlags[n] == asTM_OUTREF ) str += "out";
				else if( inOutFlags[n] == asTM_INOUTREF ) str += "inout";
			}

			if( includeParamNames && n < parameterNames.GetLength() && parameterNames[n].GetLength() != 0 )
			{
				str += " ";
				str += parameterNames[n];
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
		str += parameterTypes[n].Format(nameSpace, includeNamespace);
		if( parameterTypes[n].IsReference() && inOutFlags.GetLength() > n )
		{
			if( inOutFlags[n] == asTM_INREF ) str += "in";
			else if( inOutFlags[n] == asTM_OUTREF ) str += "out";
			else if( inOutFlags[n] == asTM_INOUTREF ) str += "inout";
		}

		if( includeParamNames && n < parameterNames.GetLength() && parameterNames[n].GetLength() != 0 )
		{
			str += " ";
			str += parameterNames[n];
		}

		if( defaultArgs.GetLength() > n && defaultArgs[n] )
		{
			asCString tmp;
			tmp.Format(" = %s", defaultArgs[n]->AddressOf());
			str += tmp;
		}
	}

	str += ")";

	if( IsReadOnly() )
		str += " const";

	// Add the declaration of the list pattern
	if( listPattern )
	{
		asSListPatternNode *n = listPattern;
		bool first = true;
		while( n )
		{
			if( n->type == asLPT_START )
			{
				str += " {";
				first = true;
			}
			else if( n->type == asLPT_END )
			{
				str += " }";
				first = false;
			}
			else if( n->type == asLPT_REPEAT )
				str += " repeat";
			else if( n->type == asLPT_REPEAT_SAME )
				str += " repeat_same";
			else if( n->type == asLPT_TYPE )
			{
				if( first )
				{
					str += " ";
					first = false;
				}
				else
					str += ", ";
				str += reinterpret_cast<asSListPatternDataTypeNode*>(n)->dataType.Format(nameSpace, includeNamespace);
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
		std::qsort(&lineNbrs[0], lineNbrs.GetLength(), sizeof(int), C::cmp);

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
int asCScriptFunction::GetVar(asUINT index, const char **out_name, int *out_typeId) const
{
	if( scriptData == 0 )
		return asNOT_SUPPORTED;
	if( index >= scriptData->variables.GetLength() )
		return asINVALID_ARG;

	if( out_name )
		*out_name = scriptData->variables[index]->name.AddressOf();
	if( out_typeId )
		*out_typeId = engine->GetTypeIdFromDataType(scriptData->variables[index]->type);

	return asSUCCESS;
}

// interface
const char *asCScriptFunction::GetVarDecl(asUINT index, bool includeNamespace) const
{
	if( scriptData == 0 || index >= scriptData->variables.GetLength() )
		return 0;

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = scriptData->variables[index]->type.Format(nameSpace, includeNamespace);
	*tempString += " " + scriptData->variables[index]->name;

	return tempString->AddressOf();
}

// internal
void asCScriptFunction::AddVariable(asCString &in_name, asCDataType &in_type, int in_stackOffset)
{
	asASSERT( scriptData );
	asSScriptVariable *var = asNEW(asSScriptVariable);
	if( var == 0 )
	{
		// Out of memory
		return;
	}
	var->name                 = in_name;
	var->type                 = in_type;
	var->stackOffset          = in_stackOffset;
	var->declaredAtProgramPos = 0;
	scriptData->variables.PushLast(var);
}

// internal
asCTypeInfo *asCScriptFunction::GetTypeInfoOfLocalVar(short varOffset)
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
	if( name != func->name || !IsSignatureExceptNameEqual(func) ) return false;

	return true;
}

// internal
bool asCScriptFunction::IsSignatureExceptNameEqual(const asCScriptFunction *func) const
{
	return IsSignatureExceptNameEqual(func->returnType, func->parameterTypes, func->inOutFlags, func->objectType, func->IsReadOnly());
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
	return IsSignatureExceptNameEqual(func->returnType, func->parameterTypes, func->inOutFlags, objectType, IsReadOnly());
}

// internal
bool asCScriptFunction::IsSignatureExceptNameAndReturnTypeEqual(const asCScriptFunction *func) const
{
	return IsSignatureExceptNameAndReturnTypeEqual(func->parameterTypes, func->inOutFlags, func->objectType, func->IsReadOnly());
}

// internal
bool asCScriptFunction::IsSignatureExceptNameAndReturnTypeEqual(const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &paramInOut, const asCObjectType *objType, bool readOnly) const
{
	if( this->IsReadOnly()      != readOnly       ) return false;
	if( (this->objectType != 0) != (objType != 0) ) return false;
	if( this->inOutFlags        != paramInOut     ) return false;
	if( this->parameterTypes    != paramTypes     ) return false;

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
		if( returnType.GetTypeInfo() )
		{
			returnType.GetTypeInfo()->AddRefInternal();

			asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(returnType.GetTypeInfo());
			if( group != 0 ) group->AddRef();
		}

		for( asUINT p = 0; p < parameterTypes.GetLength(); p++ )
			if( parameterTypes[p].GetTypeInfo() )
			{
				parameterTypes[p].GetTypeInfo()->AddRefInternal();

				asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(parameterTypes[p].GetTypeInfo());
				if( group != 0 ) group->AddRef();
			}

		for( asUINT v = 0; v < scriptData->objVariableTypes.GetLength(); v++ )
			if( scriptData->objVariableTypes[v] ) // The null handle is also stored, but it doesn't have an object type
			{
				scriptData->objVariableTypes[v]->AddRefInternal();

				asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(scriptData->objVariableTypes[v]);
				if( group != 0 ) group->AddRef();
			}

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
					asASSERT( objType );
					if( objType )
						objType->AddRefInternal();
				}
				break;

			// Object type and function
			case asBC_ALLOC:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					asASSERT( objType );
					if( objType )
						objType->AddRefInternal();

					int funcId = asBC_INTARG(&bc[n]+AS_PTR_SIZE);
					if( funcId )
						engine->scriptFunctions[funcId]->AddRefInternal();
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

					if (!prop)
					{
						// The pointer is a string constant. In order to make sure the correct resource
						// management is maintained we request a new string constant here, so the compiler
						// or bytecode loader can release its copy afterwards.
						asCString str;
						asUINT length;
						int r = engine->stringFactory->GetRawStringData(gvarPtr, 0, &length);
						if (r >= 0)
						{
							str.SetLength(length);
							engine->stringFactory->GetRawStringData(gvarPtr, str.AddressOf(), &length);

							// Get a new pointer (depending on the string factory implementation it may actually be the same)
							gvarPtr = const_cast<void*>(engine->stringFactory->GetStringConstant(str.AddressOf(), length));
							asBC_PTRARG(&bc[n]) = (asPWORD)gvarPtr;
						}
						
						// If we get an error from the string factory there is not
						// anything we can do about it, except report a message.
						// TODO: NEWSTRING: Write a message and then exit gracefully
						asASSERT(r >= 0);
						break;
					}

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

					asASSERT( funcId > 0 );
					if( funcId > 0 )
						engine->scriptFunctions[funcId]->AddRefInternal();
				}
				break;

			// Functions
			case asBC_CALL:
			case asBC_CALLINTF:
				{
					int funcId = asBC_INTARG(&bc[n]);
					asASSERT( funcId > 0 );
					if( funcId > 0 )
						engine->scriptFunctions[funcId]->AddRefInternal();
				}
				break;

			// Function pointers
			case asBC_FuncPtr:
				{
					asCScriptFunction *func = (asCScriptFunction*)asBC_PTRARG(&bc[n]);
					asASSERT( func );
					if( func )
						func->AddRefInternal();
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
		if( returnType.GetTypeInfo() )
		{
			returnType.GetTypeInfo()->ReleaseInternal();

			asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(returnType.GetTypeInfo());
			if( group != 0 ) group->Release();
		}

		for( asUINT p = 0; p < parameterTypes.GetLength(); p++ )
			if( parameterTypes[p].GetTypeInfo() )
			{
				parameterTypes[p].GetTypeInfo()->ReleaseInternal();

				asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(parameterTypes[p].GetTypeInfo());
				if( group != 0 ) group->Release();
			}

		for( asUINT v = 0; v < scriptData->objVariableTypes.GetLength(); v++ )
			if( scriptData->objVariableTypes[v] ) // The null handle is also stored, but it doesn't have an object type
			{
				scriptData->objVariableTypes[v]->ReleaseInternal();

				asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(scriptData->objVariableTypes[v]);
				if( group != 0 ) group->Release();
			}

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
						objType->ReleaseInternal();
				}
				break;

			// Object type and function
			case asBC_ALLOC:
				{
					asCObjectType *objType = (asCObjectType*)asBC_PTRARG(&bc[n]);
					if( objType )
						objType->ReleaseInternal();

					int funcId = asBC_INTARG(&bc[n]+AS_PTR_SIZE);
					if( funcId > 0 )
					{
						asCScriptFunction *fptr = engine->scriptFunctions[funcId];
						if( fptr )
							fptr->ReleaseInternal();

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

					if (!prop)
					{
						// The pointer is a string constant, so it needs to be released by the string factory
						int r = engine->stringFactory->ReleaseStringConstant(gvarPtr);
						UNUSED_VAR(r);

						// If we get an error from the string factory there is not
						// anything we can do about it, except report a message.
						// TODO: Write a message showing that the string couldn't be 
						//       released. Include the first 10 characters and the length
						//       to make it easier to identify which string it was
						asASSERT(r >= 0);
						break;
					}

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
					{
						asCScriptFunction *fptr = engine->scriptFunctions[funcId];
						if( fptr )
							fptr->ReleaseInternal();
					}
				}
				break;

			// Functions
			case asBC_CALL:
			case asBC_CALLINTF:
				{
					int funcId = asBC_INTARG(&bc[n]);
					if( funcId )
					{
						asCScriptFunction *fptr = engine->scriptFunctions[funcId];
						if( fptr )
							fptr->ReleaseInternal();

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
						func->ReleaseInternal();
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
int asCScriptFunction::GetParam(asUINT index, int *out_typeId, asDWORD *out_flags, const char **out_name, const char **out_defaultArg) const
{
	if( index >= parameterTypes.GetLength() )
		return asINVALID_ARG;

	if( out_typeId )
		*out_typeId = engine->GetTypeIdFromDataType(parameterTypes[index]);

	if( out_flags )
	{
		*out_flags = inOutFlags[index];
		*out_flags |= parameterTypes[index].IsReadOnly() ? asTM_CONST : 0;
	}

	if( out_name )
	{
		// The parameter names are not stored if loading from bytecode without debug information
		if( index < parameterNames.GetLength() )
			*out_name = parameterNames[index].AddressOf();
		else
			*out_name = 0;
	}

	if( out_defaultArg )
	{
		if( index < defaultArgs.GetLength() && defaultArgs[index] )
			*out_defaultArg = defaultArgs[index]->AddressOf();
		else
			*out_defaultArg = 0;
	}

	return asSUCCESS;
}

// interface
asIScriptEngine *asCScriptFunction::GetEngine() const
{
	return engine;
}

// interface
const char *asCScriptFunction::GetDeclaration(bool includeObjectName, bool includeNamespace, bool includeParamNames) const
{
	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = GetDeclarationStr(includeObjectName, includeNamespace, includeParamNames);
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
		group = engine->FindConfigGroupForFuncDef(this->funcdefType);

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

	// Make sure the function has been compiled with JitEntry instructions
	// For functions that has JitEntry this will be a quick test
	asUINT length;
	asDWORD *byteCode = GetByteCode(&length);
	asDWORD *end = byteCode + length;
	bool foundJitEntry = false;
	while( byteCode < end )
	{
		// Determine the instruction
		asEBCInstr op = asEBCInstr(*(asBYTE*)byteCode);
		if( op == asBC_JitEntry )
		{
			foundJitEntry = true;
			break;
		}

		// Move to next instruction
		byteCode += asBCTypeSize[asBCInfo[op].type];
	}

	if( !foundJitEntry )
	{
		asCString msg;
		msg.Format(TXT_NO_JIT_IN_FUNC_s, GetDeclaration());
		engine->WriteMessage("", 0, 0, asMSGTYPE_WARNING, msg.AddressOf());
	}

	// Release the previous function, if any
	if( scriptData->jitFunction )
	{
		engine->jitCompiler->ReleaseJITFunction(scriptData->jitFunction);
		scriptData->jitFunction = 0;
	}

	// Compile for native system
	int r = jit->CompileFunction(this, &scriptData->jitFunction);
	if( r < 0 )
		asASSERT( scriptData->jitFunction == 0 );
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
void *asCScriptFunction::SetUserData(void *data, asPWORD type)
{
	// As a thread might add a new new user data at the same time as another
	// it is necessary to protect both read and write access to the userData member
	ACQUIREEXCLUSIVE(engine->engineRWLock);

	// It is not intended to store a lot of different types of userdata,
	// so a more complex structure like a associative map would just have
	// more overhead than a simple array.
	for( asUINT n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n] == type )
		{
			void *oldData = reinterpret_cast<void*>(userData[n+1]);
			userData[n+1] = reinterpret_cast<asPWORD>(data);

			RELEASEEXCLUSIVE(engine->engineRWLock);

			return oldData;
		}
	}

	userData.PushLast(type);
	userData.PushLast(reinterpret_cast<asPWORD>(data));

	RELEASEEXCLUSIVE(engine->engineRWLock);

	return 0;
}

// interface
void *asCScriptFunction::GetUserData(asPWORD type) const
{
	// There may be multiple threads reading, but when
	// setting the user data nobody must be reading.
	ACQUIRESHARED(engine->engineRWLock);

	for( asUINT n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n] == type )
		{
			RELEASESHARED(engine->engineRWLock);
			return reinterpret_cast<void*>(userData[n+1]);
		}
	}

	RELEASESHARED(engine->engineRWLock);

	return 0;
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
	asASSERT( funcType == asFUNC_DELEGATE );

	return externalRefCount.get();
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
	asASSERT( funcType == asFUNC_DELEGATE );

	// Delegate
	if( objForDelegate )
		engine->GCEnumCallback(objForDelegate);
}

// internal
void asCScriptFunction::ReleaseAllHandles(asIScriptEngine *)
{
	asASSERT( funcType == asFUNC_DELEGATE );

	// Release paramaters

	// Delegate
	if( objForDelegate )
		engine->ReleaseScriptObject(objForDelegate, funcForDelegate->GetObjectType());
	objForDelegate = 0;
}

// internal
bool asCScriptFunction::IsShared() const
{
	// All system functions are shared
	if( funcType == asFUNC_SYSTEM ) return true;

	// All class methods for shared classes are also shared
	asASSERT( objectType == 0 || objectType->engine == engine || objectType->engine == 0 );
	if( objectType && (objectType->flags & asOBJ_SHARED) ) return true;

	// funcdefs that are registered by the application are shared
	if (funcType == asFUNC_FUNCDEF && module == 0) return true;

	// Functions that have been specifically marked as shared are shared
	return traits.GetTrait(asTRAIT_SHARED);
}

// internal
bool asCScriptFunction::IsFinal() const
{
	return traits.GetTrait(asTRAIT_FINAL);
}

// internal
bool asCScriptFunction::IsOverride() const
{
	return traits.GetTrait(asTRAIT_OVERRIDE);
}

END_AS_NAMESPACE

