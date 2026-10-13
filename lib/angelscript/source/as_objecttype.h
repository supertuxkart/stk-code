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
// as_objecttype.h
//
// A class for storing object type information
//



#ifndef AS_OBJECTTYPE_H
#define AS_OBJECTTYPE_H

#include "as_property.h"
#include "as_array.h"
#include "as_scriptfunction.h"
#include "as_typeinfo.h"

BEGIN_AS_NAMESPACE

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
};

class asCScriptEngine;
struct asSNameSpace;

class asCObjectType : public asCTypeInfo
{
public:
	asITypeInfo       *GetBaseType() const;
	bool               DerivesFrom(const asITypeInfo *objType) const;
	int                GetSubTypeId(asUINT subtypeIndex = 0) const;
	asITypeInfo       *GetSubType(asUINT subtypeIndex = 0) const;
	asUINT             GetSubTypeCount() const;
	asUINT             GetInterfaceCount() const;
	asITypeInfo       *GetInterface(asUINT index) const;
	bool               Implements(const asITypeInfo *objType) const;
	asUINT             GetFactoryCount() const;
	asIScriptFunction *GetFactoryByIndex(asUINT index) const;
	asIScriptFunction *GetFactoryByDecl(const char *decl) const;
	asUINT             GetMethodCount() const;
	asIScriptFunction *GetMethodByIndex(asUINT index, bool getVirtual) const;
	asIScriptFunction *GetMethodByName(const char *name, bool getVirtual) const;
	asIScriptFunction *GetMethodByDecl(const char *decl, bool getVirtual) const;
	asUINT             GetPropertyCount() const;
	int                GetProperty(asUINT index, const char **name, int *typeId, bool *isPrivate, bool *isProtected, int *offset, bool *isReference, asDWORD *accessMask, int *compositeOffset, bool *isCompositeIndirect, bool *isConst) const;
	const char        *GetPropertyDeclaration(asUINT index, bool includeNamespace = false) const;
	asUINT             GetBehaviourCount() const;
	asIScriptFunction *GetBehaviourByIndex(asUINT index, asEBehaviours *outBehaviour) const;
	asUINT             GetChildFuncdefCount() const;
	asITypeInfo       *GetChildFuncdef(asUINT index) const;

public:
	asCObjectType(asCScriptEngine *engine);
	~asCObjectType();
	void DestroyInternal();

	void ReleaseAllFunctions();

	bool IsInterface() const;

	asCObjectProperty *AddPropertyToClass(const asCString &name, const asCDataType &dt, bool isPrivate, bool isProtected, bool isInherited);
	void ReleaseAllProperties();

#ifdef WIP_16BYTE_ALIGN
	int                          alignment;
#endif
	asCArray<asCObjectProperty*> properties;
	asCArray<int>                methods;

	// TODO: These are not used by template types. Should perhaps create a derived class to save memory on ordinary object types
	asCArray<asCObjectType*>     interfaces;
	asCArray<asUINT>             interfaceVFTOffsets;
	asCObjectType *              derivedFrom;
	asCArray<asCScriptFunction*> virtualFunctionTable;

	// Used for funcdefs declared as members of class.
	// TODO: child funcdef: Should be possible to enumerate these from application
	asCArray<asCFuncdefType*> childFuncDefs;

	asSTypeBehaviour beh;

	// Used for template types
	asCArray<asCDataType> templateSubTypes;   // increases refCount for typeinfo held in datatype
	bool                  acceptValueSubType;
	bool                  acceptRefSubType;

protected:
	friend class asCScriptEngine;
	friend class asCConfigGroup;
	friend class asCModule;
	asCObjectType();
};

END_AS_NAMESPACE

#endif
