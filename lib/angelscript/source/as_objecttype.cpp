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

asCObjectType::asCObjectType() 
{
	externalRefCount.set(0); 
	internalRefCount.set(1); // start with one internal ref-count
	engine      = 0; 
	module      = 0;
	derivedFrom = 0;
	size        = 0;
	typeId      = -1; // start as -1 to signal that it hasn't been defined

	acceptValueSubType = true;
	acceptRefSubType   = true;

	scriptSectionIdx = -1;
	declaredAt       = 0;

	accessMask = 0xFFFFFFFF;
	nameSpace  = 0;
#ifdef WIP_16BYTE_ALIGN
	alignment  = 4;
#endif
}

asCObjectType::asCObjectType(asCScriptEngine *engine) 
{
	externalRefCount.set(0); 
	internalRefCount.set(1); // start with one internal ref count
	this->engine = engine; 
	module       = 0;
	derivedFrom  = 0;
	typeId      = -1; // start as -1 to signal that it hasn't been defined

	acceptValueSubType = true;
	acceptRefSubType   = true;

	scriptSectionIdx = -1;
	declaredAt       = 0;

	accessMask = 0xFFFFFFFF;
	nameSpace  = engine->nameSpaces[0];
#ifdef WIP_16BYTE_ALIGN
	alignment  = 4;
#endif
}

int asCObjectType::AddRef() const
{
	return externalRefCount.atomicInc();
}

int asCObjectType::Release() const
{
	int r = externalRefCount.atomicDec();

	if( r == 0 )
	{
		// There are no more external references, if there are also no
		// internal references then it is time to delete the object type
		if( internalRefCount.get() == 0 )
		{
			// If the engine is no longer set, then it has already been 
			// released and we must take care of the deletion ourselves
			asDELETE(const_cast<asCObjectType*>(this), asCObjectType);
		}
	}

	return r;
}

int asCObjectType::AddRefInternal()
{
	return internalRefCount.atomicInc();
}

int asCObjectType::ReleaseInternal()
{
	int r = internalRefCount.atomicDec();

	if( r == 0 )
	{
		// There are no more internal references, if there are also no
		// external references then it is time to delete the object type
		if( externalRefCount.get() == 0 )
		{
			// If the engine is no longer set, then it has already been 
			// released and we must take care of the deletion ourselves
			asDELETE(const_cast<asCObjectType*>(this), asCObjectType);
		}
	}

	return r;
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

void asCObjectType::DestroyInternal()
{
	if( engine == 0 ) return;

	// Skip this for list patterns as they do not increase the references
	if( flags & asOBJ_LIST_PATTERN )
	{
		// Clear the engine pointer to mark the object type as invalid
		engine = 0;
		return;
	}

	// Release the object types held by the templateSubTypes
	for( asUINT subtypeIndex = 0; subtypeIndex < templateSubTypes.GetLength(); subtypeIndex++ )
	{
		if( templateSubTypes[subtypeIndex].GetObjectType() )
			templateSubTypes[subtypeIndex].GetObjectType()->ReleaseInternal();
	}
	templateSubTypes.SetLength(0);

	if( derivedFrom )
		derivedFrom->ReleaseInternal();
	derivedFrom = 0;

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
	userData.SetLength(0);

	// Remove the type from the engine
	if( typeId != -1 )
		engine->RemoveFromTypeIdMap(this);

	// Clear the engine pointer to mark the object type as invalid
	engine = 0;
}

asCObjectType::~asCObjectType()
{
	if( engine == 0 )
		return;

	DestroyInternal();
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
	if( typeId == -1 )
	{
		// We need a non const pointer to create the asCDataType object.
		// We're not breaking anything here because this function is not
		// modifying the object, so this const cast is safe.
		asCObjectType *ot = const_cast<asCObjectType*>(this);

		// The engine will define the typeId for this object type
		engine->GetTypeIdFromDataType(asCDataType::CreateObject(ot, false));
	}

	return typeId;
}

// interface
int asCObjectType::GetSubTypeId(asUINT subtypeIndex) const
{
	// This method is only supported for templates and template specializations
	if( templateSubTypes.GetLength() == 0 )
		return asERROR;

	if( subtypeIndex >= templateSubTypes.GetLength() )
		return asINVALID_ARG;

	return engine->GetTypeIdFromDataType(templateSubTypes[subtypeIndex]);
}

// interface
asIObjectType *asCObjectType::GetSubType(asUINT subtypeIndex) const
{
	if( subtypeIndex >= templateSubTypes.GetLength() )
		return 0;

	return templateSubTypes[subtypeIndex].GetObjectType();
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
	for( asUINT n = 0; n < methods.GetLength(); n++ )
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
int asCObjectType::GetProperty(asUINT index, const char **name, int *typeId, bool *isPrivate, bool *isProtected, int *offset, bool *isReference, asDWORD *accessMask) const
{
	if( index >= properties.GetLength() )
		return asINVALID_ARG;

	asCObjectProperty *prop = properties[index];
	if( name )
		*name = prop->name.AddressOf();
	if( typeId )
		*typeId = engine->GetTypeIdFromDataType(prop->type);
	if( isPrivate )
		*isPrivate = prop->isPrivate;
	if( isProtected )
		*isProtected = prop->isProtected;
	if( offset )
		*offset = prop->byteOffset;
	if( isReference )
		*isReference = prop->type.IsReference();
	if( accessMask )
		*accessMask = prop->accessMask;

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
	else if( properties[index]->isProtected )
		*tempString = "protected ";
	else
		*tempString = "";
	*tempString += properties[index]->type.Format(nameSpace, includeNamespace);
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
asCObjectProperty *asCObjectType::AddPropertyToClass(const asCString &name, const asCDataType &dt, bool isPrivate, bool isProtected, bool isInherited)
{
	asASSERT( flags & asOBJ_SCRIPT_OBJECT );
	asASSERT( dt.CanBeInstantiated() );
	asASSERT( !IsInterface() );

	// Store the properties in the object type descriptor
	asCObjectProperty *prop = asNEW(asCObjectProperty);
	if( prop == 0 )
	{
		// Out of memory
		return 0;
	}

	prop->name        = name;
	prop->type        = dt;
	prop->isPrivate   = isPrivate;
	prop->isProtected = isProtected;
	prop->isInherited = isInherited;

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
#ifndef WIP_16BYTE_ALIGN
	if( propSize == 2 && (size & 1) ) size += 1;
	if( propSize > 2 && (size & 3) ) size += 4 - (size & 3);
#else
	asUINT alignment = dt.GetAlignment();
	const asUINT propSizeAlignmentDifference = size & (alignment-1);
	if( propSizeAlignmentDifference != 0 )
	{
		size += (alignment - propSizeAlignmentDifference);
	}

	asASSERT((size % alignment) == 0);
#endif

	prop->byteOffset = size;
	size += propSize;

	properties.PushLast(prop);

	// Make sure the struct holds a reference to the config group where the object is registered
	asCConfigGroup *group = engine->FindConfigGroupForObjectType(prop->type.GetObjectType());
	if( group != 0 ) group->AddRef();

	// Add reference to object types
	asCObjectType *type = prop->type.GetObjectType();
	if( type )
		type->AddRefInternal();

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
					type->ReleaseInternal();
			}
			else
			{
				// Release template instance types (ref increased by RegisterObjectProperty)
				asCObjectType *type = properties[n]->type.GetObjectType();
				if( type )
					type->ReleaseInternal();
			}

			asDELETE(properties[n],asCObjectProperty);
		}
	}

	properties.SetLength(0);
}

// internal
void asCObjectType::ReleaseAllFunctions()
{
	beh.factory     = 0;
	beh.copyfactory = 0;
	for( asUINT a = 0; a < beh.factories.GetLength(); a++ )
	{
		if( engine->scriptFunctions[beh.factories[a]] ) 
			engine->scriptFunctions[beh.factories[a]]->ReleaseInternal();
	}
	beh.factories.SetLength(0);

	beh.construct     = 0;
	beh.copyconstruct = 0;
	for( asUINT b = 0; b < beh.constructors.GetLength(); b++ )
	{
		if( engine->scriptFunctions[beh.constructors[b]] ) 
			engine->scriptFunctions[beh.constructors[b]]->ReleaseInternal();
	}
	beh.constructors.SetLength(0);

	if( beh.templateCallback )
		engine->scriptFunctions[beh.templateCallback]->ReleaseInternal();
	beh.templateCallback = 0;

	if( beh.listFactory )
		engine->scriptFunctions[beh.listFactory]->ReleaseInternal();
	beh.listFactory = 0;

	if( beh.destruct )
		engine->scriptFunctions[beh.destruct]->ReleaseInternal();
	beh.destruct  = 0;

	if( beh.copy )
		engine->scriptFunctions[beh.copy]->ReleaseInternal();
	beh.copy = 0;

	for( asUINT c = 0; c < methods.GetLength(); c++ )
	{
		if( engine->scriptFunctions[methods[c]] ) 
			engine->scriptFunctions[methods[c]]->ReleaseInternal();
	}
	methods.SetLength(0);

	for( asUINT d = 0; d < virtualFunctionTable.GetLength(); d++ )
	{
		if( virtualFunctionTable[d] )
			virtualFunctionTable[d]->ReleaseInternal();
	}
	virtualFunctionTable.SetLength(0);

	// GC behaviours
	if( beh.addref )
		engine->scriptFunctions[beh.addref]->ReleaseInternal();
	beh.addref = 0;

	if( beh.release )
		engine->scriptFunctions[beh.release]->ReleaseInternal();
	beh.release = 0;

	if( beh.gcEnumReferences )
		engine->scriptFunctions[beh.gcEnumReferences]->ReleaseInternal();
	beh.gcEnumReferences = 0;

	if( beh.gcGetFlag )
		engine->scriptFunctions[beh.gcGetFlag]->ReleaseInternal();
	beh.gcGetFlag = 0;

	if( beh.gcGetRefCount )
		engine->scriptFunctions[beh.gcGetRefCount]->ReleaseInternal();
	beh.gcGetRefCount = 0;

	if( beh.gcReleaseAllReferences )
		engine->scriptFunctions[beh.gcReleaseAllReferences]->ReleaseInternal();
	beh.gcReleaseAllReferences = 0;

	if( beh.gcSetFlag )
		engine->scriptFunctions[beh.gcSetFlag]->ReleaseInternal();
	beh.gcSetFlag = 0;

	if ( beh.getWeakRefFlag )
		engine->scriptFunctions[beh.getWeakRefFlag]->ReleaseInternal();
	beh.getWeakRefFlag = 0;
}

END_AS_NAMESPACE



