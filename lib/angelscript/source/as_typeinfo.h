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
// as_typeinfo.h
//



#ifndef AS_TYPEINFO_H
#define AS_TYPEINFO_H

#include "as_config.h"
#include "as_string.h"
#include "as_atomic.h"
#include "as_datatype.h"

BEGIN_AS_NAMESPACE

class asCScriptEngine;
class asCModule;
class asCObjectType;
class asCEnumType;
class asCTypedefType;
class asCFuncdefType;
struct asSNameSpace;

// TODO: type: asCPrimitiveType shall be implemented to represent primitives (void, int, double, etc)

// TODO: type: asCTypeInfo should have an internal virtual method GetBehaviours. For asCObjectType it 
//             should return the beh member. For asCFuncdefType it should return the beh member of 
//             engine->functionBehaviours. This will allow the code that needs the behaviour to handle 
//             both object types and funcdefs the same way

class asCTypeInfo : public asITypeInfo
{
public:
	//=====================================
	// From asITypeInfo
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
	asITypeInfo     *GetBaseType() const { return 0; }
	bool             DerivesFrom(const asITypeInfo *objType) const { UNUSED_VAR(objType); return 0; }
	asQWORD          GetFlags() const;
	asUINT           GetSize() const;
	int              GetTypeId() const;
	int              GetSubTypeId(asUINT subtypeIndex = 0) const { UNUSED_VAR(subtypeIndex); return -1; }
	asITypeInfo     *GetSubType(asUINT subtypeIndex = 0) const { UNUSED_VAR(subtypeIndex); return 0; }
	asUINT           GetSubTypeCount() const { return 0; }

	// Interfaces
	asUINT           GetInterfaceCount() const { return 0; }
	asITypeInfo     *GetInterface(asUINT index) const { UNUSED_VAR(index); return 0; }
	bool             Implements(const asITypeInfo *objType) const { UNUSED_VAR(objType); return false; }

	// Factories
	asUINT             GetFactoryCount() const { return 0; }
	asIScriptFunction *GetFactoryByIndex(asUINT index) const { UNUSED_VAR(index); return 0; }
	asIScriptFunction *GetFactoryByDecl(const char *decl) const { UNUSED_VAR(decl); return 0; }

	// Methods
	asUINT             GetMethodCount() const { return 0; }
	asIScriptFunction *GetMethodByIndex(asUINT index, bool getVirtual) const { UNUSED_VAR(index); UNUSED_VAR(getVirtual); return 0; }
	asIScriptFunction *GetMethodByName(const char *in_name, bool getVirtual) const { UNUSED_VAR(in_name); UNUSED_VAR(getVirtual); return 0; }
	asIScriptFunction *GetMethodByDecl(const char *decl, bool getVirtual) const { UNUSED_VAR(decl); UNUSED_VAR(getVirtual); return 0; }

	// Properties
	asUINT      GetPropertyCount() const { return 0; }
	int         GetProperty(asUINT index, const char **name, int *typeId, bool *isPrivate, bool *isProtected, int *offset, bool *isReference, asDWORD *accessMask, int *compositeOffset, bool *isCompositeIndirect, bool *isConst) const;
	const char *GetPropertyDeclaration(asUINT index, bool includeNamespace = false) const { UNUSED_VAR(index); UNUSED_VAR(includeNamespace); return 0; }

	// Behaviours
	asUINT             GetBehaviourCount() const { return 0; }
	asIScriptFunction *GetBehaviourByIndex(asUINT index, asEBehaviours *outBehaviour) const { UNUSED_VAR(index); UNUSED_VAR(outBehaviour); return 0; }

	// Child types
	asUINT       GetChildFuncdefCount() const { return 0; }
	asITypeInfo *GetChildFuncdef(asUINT index) const { UNUSED_VAR(index); return 0; }
	asITypeInfo *GetParentType() const { return 0; }

	// Enums
	virtual asUINT      GetEnumValueCount() const { return 0; }
	virtual const char *GetEnumValueByIndex(asUINT index, int *outValue) const { UNUSED_VAR(index); if (outValue) *outValue = 0; return 0; }

	// Typedef
	virtual int GetTypedefTypeId() const { return asERROR; }

	// Funcdef
	virtual asIScriptFunction *GetFuncdefSignature() const { return 0; }

	// User data
	void *SetUserData(void *data, asPWORD type);
	void *GetUserData(asPWORD type) const;

	//===========================================
	// Internal
	//===========================================
public:
	asCTypeInfo(asCScriptEngine *engine);
	virtual ~asCTypeInfo();

	// Keep an internal reference counter to separate references coming from 
	// application or script objects and references coming from the script code
	virtual int AddRefInternal();
	virtual int ReleaseInternal();

	virtual void DestroyInternal() {}

	void CleanUserData();

	bool IsShared() const;

	// These can be safely used on null pointers (which will return null)
	friend asCObjectType  *CastToObjectType(asCTypeInfo *);
	friend asCEnumType    *CastToEnumType(asCTypeInfo *);
	friend asCTypedefType *CastToTypedefType(asCTypeInfo *);
	friend asCFuncdefType *CastToFuncdefType(asCTypeInfo *);


	asCString                    name;
	asSNameSpace                *nameSpace;
	int                          size;
	mutable int                  typeId;
	asQWORD                      flags;
	asDWORD                      accessMask;

	// Store the script section where the code was declared
	int                             scriptSectionIdx;
	// Store the location where the function was declared (row in the lower 20 bits, and column in the upper 12)
	int                             declaredAt;

	asCScriptEngine  *engine;
	asCModule        *module;
	asCArray<asPWORD> userData;

protected:
	friend class asCScriptEngine;
	friend class asCConfigGroup;
	friend class asCModule;
	friend class asCObjectType;
	asCTypeInfo();

	mutable asCAtomic externalRefCount;
	asCAtomic         internalRefCount;
};

struct asSEnumValue
{
	asCString name;
	int       value;
};

class asCEnumType : public asCTypeInfo
{
public:
	asCEnumType(asCScriptEngine *engine) : asCTypeInfo(engine) {}
	~asCEnumType();

	asCArray<asSEnumValue*> enumValues;

	asUINT      GetEnumValueCount() const;
	const char *GetEnumValueByIndex(asUINT index, int *outValue) const;

protected:
	asCEnumType() : asCTypeInfo() {}
};

class asCTypedefType : public asCTypeInfo
{
public:
	asCTypedefType(asCScriptEngine *engine) : asCTypeInfo(engine) {}
	~asCTypedefType();

	void DestroyInternal();

	asCDataType aliasForType; // increase refCount for typeinfo inside datatype

	int GetTypedefTypeId() const;

protected:
	asCTypedefType() : asCTypeInfo() {}
};

class asCFuncdefType : public asCTypeInfo
{
public:
	asCFuncdefType(asCScriptEngine *engine, asCScriptFunction *func);
	~asCFuncdefType();

	asIScriptFunction *GetFuncdefSignature() const;
	asITypeInfo       *GetParentType() const;

	void DestroyInternal();
	asCScriptFunction *funcdef;     // increases refCount
	asCObjectType     *parentClass; // doesn't increase refCount

protected:
	asCFuncdefType() : asCTypeInfo(), funcdef(0), parentClass(0) {}
};

END_AS_NAMESPACE

#endif
