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
// as_objecttype.cpp
//
// A class for storing object type information
//


#include <stdio.h>

#include "as_config.h"
#include "as_objecttype.h"
#include "as_configgroup.h"
#include "as_scriptengine.h"

BEGIN_AS_NAMESPACE

#ifdef AS_MAX_PORTABILITY

static void ObjectType_AddRef_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	self->AddRef();
}

static void ObjectType_Release_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	self->Release();
}

static void ObjectType_GetRefCount_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	*(int*)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void ObjectType_SetGCFlag_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	self->SetGCFlag();
}

static void ObjectType_GetGCFlag_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	*(bool*)gen->GetAddressOfReturnLocation() = self->GetGCFlag();
}

static void ObjectType_EnumReferences_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void ObjectType_ReleaseAllHandles_Generic(asIScriptGeneric *gen)
{
	asCObjectType *self = (asCObjectType*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

#endif


void RegisterObjectTypeGCBehaviours(asCScriptEngine *engine)
{
	// Register the gc behaviours for the object types
	int r = 0;
	UNUSED_VAR(r); // It is only used in debug mode
	engine->objectTypeBehaviours.engine = engine;
	engine->objectTypeBehaviours.flags = asOBJ_REF | asOBJ_GC;
	engine->objectTypeBehaviours.name = "_builtin_objecttype_";
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_ADDREF, "void f()", asMETHOD(asCObjectType,AddRef), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_RELEASE, "void f()", asMETHOD(asCObjectType,Release), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(asCObjectType,GetRefCount), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_SETGCFLAG, "void f()", asMETHOD(asCObjectType,SetGCFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(asCObjectType,GetGCFlag), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(asCObjectType,EnumReferences), asCALL_THISCALL, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(asCObjectType,ReleaseAllHandles), asCALL_THISCALL, 0); asASSERT( r >= 0 );
#else
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_ADDREF, "void f()", asFUNCTION(ObjectType_AddRef_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_RELEASE, "void f()", asFUNCTION(ObjectType_Release_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ObjectType_GetRefCount_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ObjectType_SetGCFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ObjectType_GetGCFlag_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ObjectType_EnumReferences_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
	r = engine->RegisterBehaviourToObjectType(&engine->objectTypeBehaviours, asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ObjectType_ReleaseAllHandles_Generic), asCALL_GENERIC, 0); asASSERT( r >= 0 );
#endif
}

asCObjectType::asCObjectType() 
{
	engine = 0; 
	module = 0;
	refCount.set(0); 
	derivedFrom = 0;

	acceptValueSubType = true;
	acceptRefSubType = true;

	accessMask = 0xFFFFFFFF;
	nameSpace = 0;
}

asCObjectType::asCObjectType(asCScriptEngine *engine) 
{
	this->engine = engine; 
	module = 0;
	refCount.set(0); 
	derivedFrom  = 0;

	acceptValueSubType = true;
	acceptRefSubType = true;

	accessMask = 0xFFFFFFFF;
	nameSpace = engine->nameSpaces[0];
}

int asCObjectType::AddRef() const
{
	gcFlag = false;
	return refCount.atomicInc();
}

int asCObjectType::Release() const
{
	gcFlag = false;
	return refCount.atomicDec();
}

void asCObjectType::Orphan(asCModule *mod)
{
	if( mod && mod == module )
	{
		module = 0;
		if( flags & asOBJ_SCRIPT_OBJECT )
		{
			// Tell the GC that this type exists so it can resolve potential circular references
			engine->gc.AddScriptObjectToGC(this, &engine->objectTypeBehaviours);

			// It's necessary to orphan the template instance types that refer to this object type,
			// otherwise the garbage collector cannot identify the circular references that involve 
			// the type and the template type
			engine->OrphanTemplateInstances(this);
		}
	}

	Release();
}

// interface
asIScriptModule *asCObjectType::GetModule() const
{
	return module;
}

void *asCObjectType::SetUserData(void *data, asPWORD type)
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

void *asCObjectType::GetUserData(asPWORD type) const
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

int asCObjectType::GetRefCount()
{
	return refCount.get();
}

bool asCObjectType::GetGCFlag()
{
	return gcFlag;
}

void asCObjectType::SetGCFlag()
{
	gcFlag = true;
}

asCObjectType::~asCObjectType()
{
	// Skip this for list patterns as they do not increase the references
	if( flags & asOBJ_LIST_PATTERN )
		return;

	// Release the object types held by the templateSubTypes
	for( asUINT subtypeIndex = 0; subtypeIndex < templateSubTypes.GetLength(); subtypeIndex++ )
	{
		if( templateSubTypes[subtypeIndex].GetObjectType() )
			templateSubTypes[subtypeIndex].GetObjectType()->Release();
	}

	if( derivedFrom )
		derivedFrom->Release();

	ReleaseAllProperties();

	ReleaseAllFunctions();

	asUINT n;
	for( n = 0; n < enumValues.GetLength(); n++ )
	{
		if( enumValues[n] )
			asDELETE(enumValues[n],asSEnumValue);
	}
	enumValues.SetLength(0);

	// Clean the user data
	for( n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n+1] )
		{
			for( asUINT c = 0; c < engine->cleanObjectTypeFuncs.GetLength(); c++ )
				if( engine->cleanObjectTypeFuncs[c].type == userData[n] )
					engine->cleanObjectTypeFuncs[c].cleanFunc(this);
		}
	}
}

// interface
bool asCObjectType::Implements(const asIObjectType *objType) const
{
	if( this == objType )
		return true;

	for( asUINT n = 0; n < interfaces.GetLength(); n++ )
		if( interfaces[n] == objType ) return true;

	return false;
}

// interface
bool asCObjectType::DerivesFrom(const asIObjectType *objType) const
{
	if( this == objType )
		return true;

	asCObjectType *base = derivedFrom;
	while( base )
	{
		if( base == objType )
			return true;

		base = base->derivedFrom;
	}

	return false;
}

bool asCObjectType::IsShared() const
{
	// Objects that can be declared by scripts need to have the explicit flag asOBJ_SHARED
	if( flags & (asOBJ_SCRIPT_OBJECT|asOBJ_ENUM) ) return flags & asOBJ_SHARED ? true : false;

	// Otherwise we assume the object to be shared
	return true;
}

// interface
const char *asCObjectType::GetName() const
{
	return name.AddressOf();
}

// interface
const char *asCObjectType::GetNamespace() const
{
	return nameSpace->name.AddressOf();
}

// interface
asDWORD asCObjectType::GetFlags() const
{
	return flags;
}

// interface
asUINT asCObjectType::GetSize() const
{
	return size;
}

// interface
int asCObjectType::GetTypeId() const
{
	// We need a non const pointer to create the asCDataType object.
	// We're not breaking anything here because this function is not
	// modifying the object, so this const cast is safe.
	asCObjectType *ot = const_cast<asCObjectType*>(this);

	return engine->GetTypeIdFromDataType(asCDataType::CreateObject(ot, false));
}

// interface
int asCObjectType::GetSubTypeId(asUINT subtypeIndex) const
{
	if( flags & asOBJ_TEMPLATE )
	{
		if( subtypeIndex >= templateSubTypes.GetLength() )
			return asINVALID_ARG;

		return engine->GetTypeIdFromDataType(templateSubTypes[subtypeIndex]);
	}

	// Only template types have sub types
	return asERROR;
}

// interface
asIObjectType *asCObjectType::GetSubType(asUINT subtypeIndex) const
{
	if( flags & asOBJ_TEMPLATE )
	{
		if( subtypeIndex >= templateSubTypes.GetLength() )
			return 0;

		return templateSubTypes[subtypeIndex].GetObjectType();
	}

	return 0;
}

asUINT asCObjectType::GetSubTypeCount() const
{
	return asUINT(templateSubTypes.GetLength());
}

asUINT asCObjectType::GetInterfaceCount() const
{
	return asUINT(interfaces.GetLength());
}

asIObjectType *asCObjectType::GetInterface(asUINT index) const
{
	return interfaces[index];
}

// internal
bool asCObjectType::IsInterface() const
{
	if( (flags & asOBJ_SCRIPT_OBJECT) && size == 0 )
		return true;

	return false;
}

asIScriptEngine *asCObjectType::GetEngine() const
{
	return engine;
}

// interface
asUINT asCObjectType::GetFactoryCount() const
{
	return (asUINT)beh.factories.GetLength();
}

// interface
asIScriptFunction *asCObjectType::GetFactoryByIndex(asUINT index) const
{
	if( index >= beh.factories.GetLength() )
		return 0;

	return engine->GetFunctionById(beh.factories[index]);
}

// interface
asIScriptFunction *asCObjectType::GetFactoryByDecl(const char *decl) const
{
	if( beh.factories.GetLength() == 0 )
		return 0;

	// Let the engine parse the string and find the appropriate factory function
	return engine->GetFunctionById(engine->GetFactoryIdByDecl(this, decl));
}

// interface
asUINT asCObjectType::GetMethodCount() const
{
	return (asUINT)methods.GetLength();
}

// interface
asIScriptFunction *asCObjectType::GetMethodByIndex(asUINT index, bool getVirtual) const
{
	if( index >= methods.GetLength() )
		return 0;

	asCScriptFunction *func = engine->scriptFunctions[methods[index]];
	if( !getVirtual )
	{
		if( func && func->funcType == asFUNC_VIRTUAL )
			return virtualFunctionTable[func->vfTableIdx];
	}

	return func;
}

// interface
asIScriptFunction *asCObjectType::GetMethodByName(const char *name, bool getVirtual) const
{
	int id = -1;
	for( size_t n = 0; n < methods.GetLength(); n++ )
	{
		if( engine->scriptFunctions[methods[n]]->name == name )
		{
			if( id == -1 )
				id = methods[n];
			else
				return 0;
		}
	}

	if( id == -1 ) return 0;

	asCScriptFunction *func = engine->scriptFunctions[id];
	if( !getVirtual )
	{
		if( func && func->funcType == asFUNC_VIRTUAL )
			return virtualFunctionTable[func->vfTableIdx];
	}

	return func;
}

// interface
asIScriptFunction *asCObjectType::GetMethodByDecl(const char *decl, bool getVirtual) const
{
	if( methods.GetLength() == 0 )
		return 0;

	// Get the module from one of the methods, but it will only be
	// used to allow the parsing of types not already known by the object.
	// It is possible for object types to be orphaned, e.g. by discarding 
	// the module that created it. In this case it is still possible to 
	// find the methods, but any type not known by the object will result in
	// an invalid declaration.
	asCModule *mod = engine->scriptFunctions[methods[0]]->module;
	int id = engine->GetMethodIdByDecl(this, decl, mod);
	if( id <= 0 )
		return 0;

	if( !getVirtual )
	{
		asCScriptFunction *func = engine->scriptFunctions[id];
		if( func && func->funcType == asFUNC_VIRTUAL )
			return virtualFunctionTable[func->vfTableIdx];
	}

	return engine->scriptFunctions[id];
}

// interface
asUINT asCObjectType::GetPropertyCount() const
{
	return (asUINT)properties.GetLength();
}

// interface
int asCObjectType::GetProperty(asUINT index, const char **name, int *typeId, bool *isPrivate, int *offset, bool *isReference, asDWORD *accessMask) const
{
	if( index >= properties.GetLength() )
		return asINVALID_ARG;

	if( name )
		*name = properties[index]->name.AddressOf();
	if( typeId )
		*typeId = engine->GetTypeIdFromDataType(properties[index]->type);
	if( isPrivate )
		*isPrivate = properties[index]->isPrivate;
	if( offset )
		*offset = properties[index]->byteOffset;
	if( isReference )
		*isReference = properties[index]->type.IsReference();
	if( accessMask )
		*accessMask = properties[index]->accessMask;

	return 0;
}

// interface
const char *asCObjectType::GetPropertyDeclaration(asUINT index, bool includeNamespace) const
{
	if( index >= properties.GetLength() )
		return 0;

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	if( properties[index]->isPrivate )
		*tempString = "private ";
	else
		*tempString = "";
	*tempString += properties[index]->type.Format(includeNamespace);
	*tempString += " ";
	*tempString += properties[index]->name;

	return tempString->AddressOf();
}

asIObjectType *asCObjectType::GetBaseType() const
{
	return derivedFrom; 
}

asUINT asCObjectType::GetBehaviourCount() const
{
	// Count the number of behaviours (except factory functions)
	asUINT count = 0;
	
	if( beh.destruct )               count++;
	if( beh.addref )                 count++;
	if( beh.release )                count++;
	if( beh.gcGetRefCount )          count++;
	if( beh.gcSetFlag )              count++;
	if( beh.gcGetFlag )              count++;
	if( beh.gcEnumReferences )       count++;
	if( beh.gcReleaseAllReferences ) count++; 
	if( beh.templateCallback )       count++;
	if( beh.listFactory )            count++;
	if( beh.getWeakRefFlag )         count++;

	// For reference types, the factories are also stored in the constructor
	// list, so it is sufficient to enumerate only those
	count += (asUINT)beh.constructors.GetLength();
	count += (asUINT)beh.operators.GetLength() / 2;

	return count;
}

asIScriptFunction *asCObjectType::GetBehaviourByIndex(asUINT index, asEBehaviours *outBehaviour) const
{
	// Find the correct behaviour
	asUINT count = 0;

	if( beh.destruct && count++ == index ) // only increase count if the behaviour is registered
	{ 
		if( outBehaviour ) *outBehaviour = asBEHAVE_DESTRUCT;
		return engine->scriptFunctions[beh.destruct];
	}

	if( beh.addref && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_ADDREF;
		return engine->scriptFunctions[beh.addref];
	}

	if( beh.release && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_RELEASE;
		return engine->scriptFunctions[beh.release];
	}

	if( beh.gcGetRefCount && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_GETREFCOUNT;
		return engine->scriptFunctions[beh.gcGetRefCount];
	}

	if( beh.gcSetFlag && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_SETGCFLAG;
		return engine->scriptFunctions[beh.gcSetFlag];
	}

	if( beh.gcGetFlag && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_GETGCFLAG;
		return engine->scriptFunctions[beh.gcGetFlag];
	}

	if( beh.gcEnumReferences && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_ENUMREFS;
		return engine->scriptFunctions[beh.gcEnumReferences];
	}

	if( beh.gcReleaseAllReferences && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_RELEASEREFS;
		return engine->scriptFunctions[beh.gcReleaseAllReferences];
	}

	if( beh.templateCallback && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_TEMPLATE_CALLBACK;
		return engine->scriptFunctions[beh.templateCallback];
	}

	if( beh.listFactory && count++ == index )
	{
		if( outBehaviour ) 
		{
			if( flags & asOBJ_VALUE )
				*outBehaviour = asBEHAVE_LIST_CONSTRUCT;
			else
				*outBehaviour = asBEHAVE_LIST_FACTORY;
		}

		return engine->scriptFunctions[beh.listFactory];
	}

	if( beh.getWeakRefFlag && count++ == index )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_GET_WEAKREF_FLAG;
		return engine->scriptFunctions[beh.getWeakRefFlag];
	}

	// For reference types, the factories are also stored in the constructor
	// list, so it is sufficient to enumerate only those
	if( index - count < beh.constructors.GetLength() )
	{
		if( outBehaviour ) *outBehaviour = asBEHAVE_CONSTRUCT;
		return engine->scriptFunctions[beh.constructors[index - count]];
	}
	else 
		count += (asUINT)beh.constructors.GetLength();

	if( index - count < beh.operators.GetLength() / 2 )
	{
		index = 2*(index - count);

		if( outBehaviour ) *outBehaviour = static_cast<asEBehaviours>(beh.operators[index]);
		return engine->scriptFunctions[beh.operators[index + 1]];
	}

	return 0;
}

// interface
const char *asCObjectType::GetConfigGroup() const
{
	asCConfigGroup *group = engine->FindConfigGroupForObjectType(this);
	if( group == 0 )
		return 0;

	return group->groupName.AddressOf();
}

// interface
asDWORD asCObjectType::GetAccessMask() const
{
	return accessMask;
}

// internal
asCObjectProperty *asCObjectType::AddPropertyToClass(const asCString &name, const asCDataType &dt, bool isPrivate)
{
	asASSERT( flags & asOBJ_SCRIPT_OBJECT );
	asASSERT( dt.CanBeInstanciated() );
	asASSERT( !IsInterface() );

	// Store the properties in the object type descriptor
	asCObjectProperty *prop = asNEW(asCObjectProperty);
	if( prop == 0 )
	{
		// Out of memory
		return 0;
	}

	prop->name      = name;
	prop->type      = dt;
	prop->isPrivate = isPrivate;

	int propSize;
	if( dt.IsObject() )
	{
		// Non-POD value types can't be allocated inline,
		// because there is a risk that the script might
		// try to access the content without knowing that
		// it hasn't been initialized yet.
		if( dt.GetObjectType()->flags & asOBJ_POD )
			propSize = dt.GetSizeInMemoryBytes();
		else
		{
			propSize = dt.GetSizeOnStackDWords()*4;
			if( !dt.IsObjectHandle() )
				prop->type.MakeReference(true);
		}
	}
	else
		propSize = dt.GetSizeInMemoryBytes();

	// Add extra bytes so that the property will be properly aligned
	if( propSize == 2 && (size & 1) ) size += 1;
	if( propSize > 2 && (size & 3) ) size += 4 - (size & 3);

	prop->byteOffset = size;
	size += propSize;

	properties.PushLast(prop);

	// Make sure the struct holds a reference to the config group where the object is registered
	asCConfigGroup *group = engine->FindConfigGroupForObjectType(prop->type.GetObjectType());
	if( group != 0 ) group->AddRef();

	// Add reference to object types
	asCObjectType *type = prop->type.GetObjectType();
	if( type )
		type->AddRef();

	return prop;
}

// internal
void asCObjectType::ReleaseAllProperties()
{
	for( asUINT n = 0; n < properties.GetLength(); n++ )
	{
		if( properties[n] ) 
		{
			if( flags & asOBJ_SCRIPT_OBJECT )
			{
				// Release the config group for script classes that are being destroyed
				asCConfigGroup *group = engine->FindConfigGroupForObjectType(properties[n]->type.GetObjectType());
				if( group != 0 ) group->Release();

				// Release references to objects types
				asCObjectType *type = properties[n]->type.GetObjectType();
				if( type )
					type->Release();
			}

			asDELETE(properties[n],asCObjectProperty);
		}
	}

	properties.SetLength(0);
}

// internal
void asCObjectType::ReleaseAllHandles(asIScriptEngine *)
{
	ReleaseAllFunctions();
	ReleaseAllProperties();
}

// internal
void asCObjectType::ReleaseAllFunctions()
{
	beh.factory     = 0;
	beh.copyfactory = 0;
	for( asUINT a = 0; a < beh.factories.GetLength(); a++ )
	{
		if( engine->scriptFunctions[beh.factories[a]] ) 
			engine->scriptFunctions[beh.factories[a]]->Release();
	}
	beh.factories.SetLength(0);

	beh.construct     = 0;
	beh.copyconstruct = 0;
	for( asUINT b = 0; b < beh.constructors.GetLength(); b++ )
	{
		if( engine->scriptFunctions[beh.constructors[b]] ) 
			engine->scriptFunctions[beh.constructors[b]]->Release();
	}
	beh.constructors.SetLength(0);

	if( beh.templateCallback )
		engine->scriptFunctions[beh.templateCallback]->Release();
	beh.templateCallback = 0;

	if( beh.listFactory )
		engine->scriptFunctions[beh.listFactory]->Release();
	beh.listFactory = 0;

	if( beh.destruct )
		engine->scriptFunctions[beh.destruct]->Release();
	beh.destruct  = 0;

	if( beh.copy )
		engine->scriptFunctions[beh.copy]->Release();
	beh.copy = 0;

	for( asUINT e = 1; e < beh.operators.GetLength(); e += 2 )
	{
		if( engine->scriptFunctions[beh.operators[e]] )
			engine->scriptFunctions[beh.operators[e]]->Release();
	}
	beh.operators.SetLength(0);

	for( asUINT c = 0; c < methods.GetLength(); c++ )
	{
		if( engine->scriptFunctions[methods[c]] ) 
			engine->scriptFunctions[methods[c]]->Release();
	}
	methods.SetLength(0);

	for( asUINT d = 0; d < virtualFunctionTable.GetLength(); d++ )
	{
		if( virtualFunctionTable[d] )
			virtualFunctionTable[d]->Release();
	}
	virtualFunctionTable.SetLength(0);

	// GC behaviours
	if( beh.addref )
		engine->scriptFunctions[beh.addref]->Release();
	beh.addref = 0;

	if( beh.release )
		engine->scriptFunctions[beh.release]->Release();
	beh.release = 0;

	if( beh.gcEnumReferences )
		engine->scriptFunctions[beh.gcEnumReferences]->Release();
	beh.gcEnumReferences = 0;

	if( beh.gcGetFlag )
		engine->scriptFunctions[beh.gcGetFlag]->Release();
	beh.gcGetFlag = 0;

	if( beh.gcGetRefCount )
		engine->scriptFunctions[beh.gcGetRefCount]->Release();
	beh.gcGetRefCount = 0;

	if( beh.gcReleaseAllReferences )
		engine->scriptFunctions[beh.gcReleaseAllReferences]->Release();
	beh.gcReleaseAllReferences = 0;

	if( beh.gcSetFlag )
		engine->scriptFunctions[beh.gcSetFlag]->Release();
	beh.gcSetFlag = 0;

	if ( beh.getWeakRefFlag )
		engine->scriptFunctions[beh.getWeakRefFlag]->Release();
	beh.getWeakRefFlag = 0;
}

// internal
void asCObjectType::EnumReferences(asIScriptEngine *)
{
	for( asUINT a = 0; a < beh.factories.GetLength(); a++ )
		if( engine->scriptFunctions[beh.factories[a]] ) 
			engine->GCEnumCallback(engine->scriptFunctions[beh.factories[a]]);

	for( asUINT b = 0; b < beh.constructors.GetLength(); b++ )
		if( engine->scriptFunctions[beh.constructors[b]] ) 
			engine->GCEnumCallback(engine->scriptFunctions[beh.constructors[b]]);

	if( beh.templateCallback )
		engine->GCEnumCallback(engine->scriptFunctions[beh.templateCallback]);

	if( beh.listFactory )
		engine->GCEnumCallback(engine->scriptFunctions[beh.listFactory]);
		
	if( beh.destruct )
		engine->GCEnumCallback(engine->scriptFunctions[beh.destruct]);

	if( beh.addref )
		engine->GCEnumCallback(engine->scriptFunctions[beh.addref]);

	if( beh.release )
		engine->GCEnumCallback(engine->scriptFunctions[beh.release]);

	if( beh.copy )
		engine->GCEnumCallback(engine->scriptFunctions[beh.copy]);

	if( beh.gcEnumReferences )
		engine->GCEnumCallback(engine->scriptFunctions[beh.gcEnumReferences]);

	if( beh.gcGetFlag )
		engine->GCEnumCallback(engine->scriptFunctions[beh.gcGetFlag]);

	if( beh.gcGetRefCount )
		engine->GCEnumCallback(engine->scriptFunctions[beh.gcGetRefCount]);

	if( beh.gcReleaseAllReferences )
		engine->GCEnumCallback(engine->scriptFunctions[beh.gcReleaseAllReferences]);

	if( beh.gcSetFlag )
		engine->GCEnumCallback(engine->scriptFunctions[beh.gcSetFlag]);

	for( asUINT e = 1; e < beh.operators.GetLength(); e += 2 )
		if( engine->scriptFunctions[beh.operators[e]] )
			engine->GCEnumCallback(engine->scriptFunctions[beh.operators[e]]);

	for( asUINT c = 0; c < methods.GetLength(); c++ )
		if( engine->scriptFunctions[methods[c]] ) 
			engine->GCEnumCallback(engine->scriptFunctions[methods[c]]);

	for( asUINT d = 0; d < virtualFunctionTable.GetLength(); d++ )
		if( virtualFunctionTable[d] )
			engine->GCEnumCallback(virtualFunctionTable[d]);

	for( asUINT p = 0; p < properties.GetLength(); p++ )
	{
		asCObjectType *type = properties[p]->type.GetObjectType();
		if( type )
			engine->GCEnumCallback(type);
	}

	for( asUINT t = 0; t < templateSubTypes.GetLength(); t++ )
		if( templateSubTypes[t].GetObjectType() )
			engine->GCEnumCallback(templateSubTypes[t].GetObjectType());

	if( beh.getWeakRefFlag )
		engine->GCEnumCallback(engine->scriptFunctions[beh.getWeakRefFlag]);

	if( derivedFrom )
		engine->GCEnumCallback(derivedFrom);
}

END_AS_NAMESPACE



