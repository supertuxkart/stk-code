/*
   AngelCode Scripting Library
   Copyright (c) 2003-2012 Andreas Jonsson

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




#include "as_config.h"
#include "as_property.h"
#include "as_scriptengine.h"

BEGIN_AS_NAMESPACE

asCGlobalProperty::asCGlobalProperty() 
{ 
	memory          = &storage; 
	memoryAllocated = false; 
	realAddress     = 0; 
	initFunc        = 0;
	accessMask      = 0xFFFFFFFF;

	refCount.set(1);
}

asCGlobalProperty::~asCGlobalProperty()
{ 
	if( memoryAllocated ) { asDELETEARRAY(memory); } 
	if( initFunc )
		initFunc->Release();
}

void asCGlobalProperty::AddRef()
{
	gcFlag = false;
	refCount.atomicInc();
}

void asCGlobalProperty::Release()
{
	gcFlag = false;

	// The property doesn't delete itself. The  
	// engine will do that at a later time
	if( refCount.atomicDec() == 2 && initFunc )
	{
		// Since the initFunc holds references to the property,
		// we'll release it when we reach refCount 2. This will
		// break the circle and allow the engine to free the property
		// without the need for the GC to attempt finding circular 
		// references.
		initFunc->Release();
		initFunc = 0;
	}
}

void *asCGlobalProperty::GetAddressOfValue()
{ 
	return memory;
}

// The global property structure is responsible for allocating the storage
// method for script declared variables. Each allocation is independent of
// other global properties, so that variables can be added and removed at
// any time.
void asCGlobalProperty::AllocateMemory() 
{ 
	if( type.GetSizeOnStackDWords() > 2 ) 
	{ 
		memory = asNEWARRAY(asDWORD, type.GetSizeOnStackDWords()); 
		memoryAllocated = true; 
	} 
}

void asCGlobalProperty::SetRegisteredAddress(void *p) 
{ 
	realAddress = p; 	
	if( type.IsObject() && !type.IsReference() && !type.IsObjectHandle() )
	{
		// The global property is a pointer to a pointer 
		memory = &realAddress;
	} 
	else
		memory = p; 
}

void *asCGlobalProperty::GetRegisteredAddress() const
{
	return realAddress;
}

int asCGlobalProperty::GetRefCount()
{
	return refCount.get();
}

void asCGlobalProperty::SetGCFlag()
{
	gcFlag = true;
}

bool asCGlobalProperty::GetGCFlag()
{
	return gcFlag;
}

void asCGlobalProperty::EnumReferences(asIScriptEngine *engine)
{
	engine->GCEnumCallback(initFunc);
}

void asCGlobalProperty::ReleaseAllHandles(asIScriptEngine *)
{
	if( initFunc )
	{
		initFunc->Release();
		initFunc = 0;
	}
}

void asCGlobalProperty::Orphan(asCModule *module)
{
	if( initFunc && initFunc->module == module )
	{
		// The owning module is releasing the property, so we need to notify 
		// the GC in order to resolve any circular references that may exists

		// This will add the property
		initFunc->engine->gc.AddScriptObjectToGC(this, &initFunc->engine->globalPropertyBehaviours);

		// This will add the function
		initFunc->AddRef();
		initFunc->Orphan(module);
	}

	Release();
}

void asCGlobalProperty::SetInitFunc(asCScriptFunction *initFunc)
{
	// This should only be done once
	asASSERT( this->initFunc == 0 );

	this->initFunc = initFunc;
	this->initFunc->AddRef();
}

asCScriptFunction *asCGlobalProperty::GetInitFunc()
{
	return initFunc;
}


#ifdef AS_MAX_PORTABILITY

static void GlobalProperty_AddRef_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	self->AddRef();
}

static void GlobalProperty_Release_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	self->Release();
}

static void GlobalProperty_GetRefCount_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	*(int*)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void GlobalProperty_SetGCFlag_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	self->SetGCFlag();
}

static void GlobalProperty_GetGCFlag_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	*(bool*)gen->GetAddressOfReturnLocation() = self->GetGCFlag();
}

static void GlobalProperty_EnumReferences_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void GlobalProperty_ReleaseAllHandles_Generic(asIScriptGeneric *gen)
{
	asCGlobalProperty *self = (asCGlobalProperty*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

#endif


void asCGlobalProperty::RegisterGCBehaviours(asCScriptEngine *engine)
{
	// Register the gc behaviours for the global properties
	int r = 0;
	UNUSED_VAR(r); // It is only used in debug mode
	engine->globalPropertyBehaviours.engine = engine;
	engine->globalPropertyBehaviours.flags = asOBJ_REF | asOBJ_GC;
	engine->globalPropertyBehaviours.name = "_builtin_globalprop_";
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_ADDREF, "void f()", asMETHOD(asCGlobalProperty,AddRef), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_RELEASE, "void f()", asMETHOD(asCGlobalProperty,Release), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(asCGlobalProperty,GetRefCount), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_SETGCFLAG, "void f()", asMETHOD(asCGlobalProperty,SetGCFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(asCGlobalProperty,GetGCFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(asCGlobalProperty,EnumReferences), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(asCGlobalProperty,ReleaseAllHandles), asCALL_THISCALL, 0); asASSERT( r >= 0 );
#else
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_ADDREF, "void f()", asFUNCTION(GlobalProperty_AddRef_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_RELEASE, "void f()", asFUNCTION(GlobalProperty_Release_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(GlobalProperty_GetRefCount_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(GlobalProperty_SetGCFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(GlobalProperty_GetGCFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(GlobalProperty_EnumReferences_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->globalPropertyBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(GlobalProperty_ReleaseAllHandles_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
#endif
}

END_AS_NAMESPACE
