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

asCObjectType::asCObjectType() : asCTypeInfo()
{
	derivedFrom = 0;

	acceptValueSubType = true;
	acceptRefSubType   = true;

#ifdef WIP_16BYTE_ALIGN
	alignment  = 4;
#endif
}

asCObjectType::asCObjectType(asCScriptEngine *in_engine) : asCTypeInfo(in_engine)
{
	derivedFrom  = 0;

	acceptValueSubType = true;
	acceptRefSubType = true;

#ifdef WIP_16BYTE_ALIGN
	alignment  = 4;
#endif
}

// interface
asUINT asCObjectType::GetChildFuncdefCount() const
{
	return childFuncDefs.GetLength();
}

// interface
asITypeInfo *asCObjectType::GetChildFuncdef(asUINT index) const
{
	if (index >= childFuncDefs.GetLength())
		return 0;

	return childFuncDefs[index];
}

// internal
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
	bool isTemplateInstance = templateSubTypes.GetLength() > 0;
	for( asUINT subtypeIndex = 0; subtypeIndex < templateSubTypes.GetLength(); subtypeIndex++ )
	{
		if( templateSubTypes[subtypeIndex].GetTypeInfo() )
			templateSubTypes[subtypeIndex].GetTypeInfo()->ReleaseInternal();
	}
	templateSubTypes.SetLength(0);

	// Clear the child types
	for (asUINT n = 0; n < childFuncDefs.GetLength(); n++)
	{
		asCFuncdefType *func = childFuncDefs[n];
		if (func)
		{
			func->parentClass = 0;
			if (isTemplateInstance)
			{
				// Any child funcdefs that have been created as part of the template 
				// instantiation must be destroyed too
				// TODO: Before destroying the funcdef, make sure no external references to it is held
				if (func->externalRefCount.get() == 0)
				{
					func->DestroyInternal();
					engine->RemoveFuncdef(func);
					func->module = 0;
					func->ReleaseInternal();
				}
			}
		}
	}
	childFuncDefs.SetLength(0);

	if( derivedFrom )
		derivedFrom->ReleaseInternal();
	derivedFrom = 0;

	ReleaseAllProperties();

	ReleaseAllFunctions();

	CleanUserData();

	// Remove the type from the engine
	if( typeId != -1 )
		engine->RemoveFromTypeIdMap(this);

	// Clear the engine pointer to mark the object type as invalid
	engine = 0;
}

asCObjectType::~asCObjectType()
{
	DestroyInternal();
}

// interface
bool asCObjectType::Implements(const asITypeInfo *objType) const
{
	if( this == objType )
		return true;

	for( asUINT n = 0; n < interfaces.GetLength(); n++ )
		if( interfaces[n] == objType ) return true;

	return false;
}

// interface
bool asCObjectType::DerivesFrom(const asITypeInfo *objType) const
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
asITypeInfo *asCObjectType::GetSubType(asUINT subtypeIndex) const
{
	if( subtypeIndex >= templateSubTypes.GetLength() )
		return 0;

	return templateSubTypes[subtypeIndex].GetTypeInfo();
}

asUINT asCObjectType::GetSubTypeCount() const
{
	return asUINT(templateSubTypes.GetLength());
}

asUINT asCObjectType::GetInterfaceCount() const
{
	return asUINT(interfaces.GetLength());
}

asITypeInfo *asCObjectType::GetInterface(asUINT index) const
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
asIScriptFunction *asCObjectType::GetMethodByName(const char *in_name, bool in_getVirtual) const
{
	int id = -1;
	for( asUINT n = 0; n < methods.GetLength(); n++ )
	{
		if( engine->scriptFunctions[methods[n]]->name == in_name )
		{
			if( id == -1 )
				id = methods[n];
			else
				return 0;
		}
	}

	if( id == -1 ) return 0;

	asCScriptFunction *func = engine->scriptFunctions[id];
	if( !in_getVirtual )
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
int asCObjectType::GetProperty(asUINT index, const char **out_name, int *out_typeId, bool *out_isPrivate, bool *out_isProtected, int *out_offset, bool *out_isReference, asDWORD *out_accessMask, int *out_compositeOffset, bool *out_isCompositeIndirect) const
{
	if( index >= properties.GetLength() )
		return asINVALID_ARG;

	asCObjectProperty *prop = properties[index];
	if( out_name )
		*out_name = prop->name.AddressOf();
	if( out_typeId )
		*out_typeId = engine->GetTypeIdFromDataType(prop->type);
	if( out_isPrivate )
		*out_isPrivate = prop->isPrivate;
	if( out_isProtected )
		*out_isProtected = prop->isProtected;
	if( out_offset )
		*out_offset = prop->byteOffset;
	if( out_isReference )
		*out_isReference = prop->type.IsReference();
	if( out_accessMask )
		*out_accessMask = prop->accessMask;
	if (out_compositeOffset)
		*out_compositeOffset = prop->compositeOffset;
	if (out_isCompositeIndirect)
		*out_isCompositeIndirect = prop->isCompositeIndirect;

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

asITypeInfo *asCObjectType::GetBaseType() const
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

// internal
asCObjectProperty *asCObjectType::AddPropertyToClass(const asCString &propName, const asCDataType &dt, bool isPrivate, bool isProtected, bool isInherited)
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

	prop->name        = propName;
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
		if( dt.GetTypeInfo()->flags & asOBJ_POD )
			propSize = dt.GetSizeInMemoryBytes();
		else
		{
			propSize = dt.GetSizeOnStackDWords()*4;
			if( !dt.IsObjectHandle() )
				prop->type.MakeReference(true);
		}
	}
	else if (dt.IsFuncdef())
	{
		// Funcdefs don't have a size, as they must always be stored as handles
		asASSERT(dt.IsObjectHandle());
		propSize = AS_PTR_SIZE * 4;
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
	asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(prop->type.GetTypeInfo());
	if( group != 0 ) group->AddRef();

	// Add reference to object types
	asCTypeInfo *type = prop->type.GetTypeInfo();
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
				asCConfigGroup *group = engine->FindConfigGroupForTypeInfo(properties[n]->type.GetTypeInfo());
				if( group != 0 ) group->Release();

				// Release references to objects types
				asCTypeInfo *type = properties[n]->type.GetTypeInfo();
				if( type )
					type->ReleaseInternal();
			}
			else
			{
				// Release template instance types (ref increased by RegisterObjectProperty)
				asCTypeInfo *type = properties[n]->type.GetTypeInfo();
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



