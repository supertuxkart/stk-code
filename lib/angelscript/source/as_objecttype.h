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
// as_objecttype.h
//
// A class for storing object type information
//



#ifndef AS_OBJECTTYPE_H
#define AS_OBJECTTYPE_H

#include "as_atomic.h"
#include "as_string.h"
#include "as_property.h"
#include "as_array.h"
#include "as_scriptfunction.h"

BEGIN_AS_NAMESPACE

// TODO: memory: Need to minimize used memory here, because not all types use all properties of the class

// TODO: The type id should have flags for differenciating between value types and reference types. It should also have a flag for differenciating interface types.

// Additional flag to the class object type
const asDWORD asOBJ_IMPLICIT_HANDLE  = 0x00400000;
const asDWORD asOBJ_LIST_PATTERN     = 0x08000000;
const asDWORD asOBJ_ENUM             = 0x10000000;
const asDWORD asOBJ_TEMPLATE_SUBTYPE = 0x20000000;
const asDWORD asOBJ_TYPEDEF          = 0x40000000;



// asOBJ_GC is used to indicate that the type can potentially 
// form circular references, thus is garbage collected.

// The fact that an object is garbage collected doesn't imply that an other object  
// that can reference it also must be garbage collected, only if the garbage collected 
// object can reference the other object as well.

// For registered types however, we set the flag asOBJ_GC if the GC 
// behaviours are registered. For script types that contain any such type we 
// automatically make garbage collected as well, because we cannot know what type
// of references that object can contain, and must assume the worst.

struct asSTypeBehaviour
{
	asSTypeBehaviour() 
	{
		factory = 0;
		listFactory = 0;
		copyfactory = 0;
		construct = 0; 
		copyconstruct = 0;
		destruct = 0; 
		copy = 0; 
		addref = 0; 
		release = 0; 
		gcGetRefCount = 0; 
		gcSetFlag = 0; 
		gcGetFlag = 0; 
		gcEnumReferences = 0; 
		gcReleaseAllReferences = 0;
		templateCallback = 0;
		getWeakRefFlag = 0;
	}

	int factory;
	int listFactory; // Used for initialization lists only
	int copyfactory;
	int construct;
	int copyconstruct;
	int destruct;
	int copy;
	int addref;
	int release;
	int templateCallback;

	// GC behaviours
	int gcGetRefCount;
	int gcSetFlag;
	int gcGetFlag;
	int gcEnumReferences;
	int gcReleaseAllReferences;

	// Weakref behaviours
	int getWeakRefFlag;

	asCArray<int> factories;
	asCArray<int> constructors;
	asCArray<int> operators;
};

struct asSEnumValue
{
	asCString name;
	int       value;
};

class asCScriptEngine;
struct asSNameSpace;

void RegisterObjectTypeGCBehaviours(asCScriptEngine *engine);

class asCObjectType : public asIObjectType
{
public:
//=====================================
// From asIObjectType
//=====================================
	asIScriptEngine *GetEngine() const;
	const char      *GetConfigGroup() const;
	asDWORD          GetAccessMask() const;
	asIScriptModule *GetModule() const;

	// Memory management
	int AddRef() const;
	int Release() const;

	// Type info
	const char      *GetName() const;
	const char      *GetNamespace() const;
	asIObjectType   *GetBaseType() const;
	bool             DerivesFrom(const asIObjectType *objType) const;
	asDWORD          GetFlags() const;
	asUINT           GetSize() const;
	int              GetTypeId() const;
	int              GetSubTypeId(asUINT subtypeIndex = 0) const;
	asIObjectType   *GetSubType(asUINT subtypeIndex = 0) const;
	asUINT           GetSubTypeCount() const;

	// Interfaces
	asUINT           GetInterfaceCount() const;
	asIObjectType   *GetInterface(asUINT index) const;
	bool             Implements(const asIObjectType *objType) const;

	// Factories
	asUINT             GetFactoryCount() const;
	asIScriptFunction *GetFactoryByIndex(asUINT index) const;
	asIScriptFunction *GetFactoryByDecl(const char *decl) const;

	// Methods
	asUINT             GetMethodCount() const;
	asIScriptFunction *GetMethodByIndex(asUINT index, bool getVirtual) const;
	asIScriptFunction *GetMethodByName(const char *name, bool getVirtual) const;
	asIScriptFunction *GetMethodByDecl(const char *decl, bool getVirtual) const;

	// Properties
	asUINT      GetPropertyCount() const;
	int         GetProperty(asUINT index, const char **name, int *typeId, bool *isPrivate, int *offset, bool *isReference, asDWORD *accessMask) const;
	const char *GetPropertyDeclaration(asUINT index, bool includeNamespace = false) const;

	// Behaviours
	asUINT             GetBehaviourCount() const;
	asIScriptFunction *GetBehaviourByIndex(asUINT index, asEBehaviours *outBehaviour) const;

	// User data
	void *SetUserData(void *data, asPWORD type);
	void *GetUserData(asPWORD type) const;

//===========================================
// Internal
//===========================================
public:
	asCObjectType(asCScriptEngine *engine);
	~asCObjectType();

	void Orphan(asCModule *module);
	int  GetRefCount();
	void SetGCFlag();
	bool GetGCFlag();
	void EnumReferences(asIScriptEngine *);
	void ReleaseAllHandles(asIScriptEngine *);

	void ReleaseAllFunctions();

	bool IsInterface() const;
	bool IsShared() const;

	asCObjectProperty *AddPropertyToClass(const asCString &name, const asCDataType &dt, bool isPrivate);
	void ReleaseAllProperties();

	asCString                    name;
	asSNameSpace                *nameSpace;
	int                          size;
	asCArray<asCObjectProperty*> properties;
	asCArray<int>                methods;
	asCArray<asCObjectType*>     interfaces;
	asCArray<asUINT>             interfaceVFTOffsets;
	asCArray<asSEnumValue*>      enumValues;
	asCObjectType *              derivedFrom;
	asCArray<asCScriptFunction*> virtualFunctionTable;

	asDWORD flags;
	asDWORD accessMask;

	asSTypeBehaviour beh;

	// Used for template types
	asCArray<asCDataType> templateSubTypes;
	bool                  acceptValueSubType;
	bool                  acceptRefSubType;

	asCScriptEngine  *engine;
	asCModule        *module;
	asCArray<asPWORD> userData;

protected:
	friend class asCScriptEngine;
	asCObjectType();

	mutable asCAtomic refCount;
	mutable bool      gcFlag;
};

END_AS_NAMESPACE

#endif
