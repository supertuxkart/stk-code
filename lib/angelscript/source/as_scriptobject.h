/*
   AngelCode Scripting Library
   Copyright (c) 2003-2019 Andreas Jonsson

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
// as_scriptobject.h
//
// A generic class for handling script declared structures
//



#ifndef AS_SCRIPTOBJECT_H
#define AS_SCRIPTOBJECT_H

#include "as_config.h"
#include "as_atomic.h"

BEGIN_AS_NAMESPACE

class asCObjectType;

// TODO: Add const overload for GetAddressOfProperty

// TODO: weak: Should move to its own file
class asCLockableSharedBool : public asILockableSharedBool
{
public:
	asCLockableSharedBool();
	int AddRef() const;
	int Release() const;

	bool Get() const;
	void Set(bool);

	void Lock() const;
	void Unlock() const;

protected:
	mutable asCAtomic refCount;
	bool      value;
	DECLARECRITICALSECTION(mutable lock)
};

class asCScriptObject : public asIScriptObject
{
public:
//===================================
// From asIScriptObject
//===================================

	// Memory management
	int                    AddRef() const;
	int                    Release() const;
	asILockableSharedBool *GetWeakRefFlag() const;

	// Type info
	int            GetTypeId() const;
	asITypeInfo   *GetObjectType() const;

	// Class properties
	asUINT      GetPropertyCount() const;
	int         GetPropertyTypeId(asUINT prop) const;
	const char *GetPropertyName(asUINT prop) const;
	void       *GetAddressOfProperty(asUINT prop);

	// Miscellaneous
	asIScriptEngine *GetEngine() const;
	int              CopyFrom(const asIScriptObject *other);

	// User data
	void *SetUserData(void *data, asPWORD type = 0);
	void *GetUserData(asPWORD type = 0) const;

//====================================
// Internal
//====================================
	asCScriptObject(asCObjectType *objType, bool doInitialize = true);
	virtual ~asCScriptObject();

	asCScriptObject &operator=(const asCScriptObject &other);

	// GC methods
	void Destruct();
	int  GetRefCount();
	void SetFlag();
	bool GetFlag();
	void EnumReferences(asIScriptEngine *engine);
	void ReleaseAllHandles(asIScriptEngine *engine);

	// Used for properties
	void *AllocateUninitializedObject(asCObjectType *objType, asCScriptEngine *engine);
	void FreeObject(void *ptr, asCObjectType *objType, asCScriptEngine *engine);
	void CopyObject(const void *src, void *dst, asCObjectType *objType, asCScriptEngine *engine);
	void CopyHandle(asPWORD *src, asPWORD *dst, asCObjectType *objType, asCScriptEngine *engine);
	int  CopyFromAs(const asCScriptObject *other, asCObjectType *objType);

	void CallDestructor();

//=============================================
// Properties
//=============================================
protected:
	friend class asCContext;
	asCObjectType    *objType;

	mutable asCAtomic refCount;
	mutable asBYTE    gcFlag:1;
	mutable asBYTE    hasRefCountReachedZero:1;
	bool              isDestructCalled;

	// Most script classes instances won't have neither the weakRefFlags nor
	// userData so we only allocate this if requested. Even when used it is
	// not something that will be accessed all the time so having the extra
	// indirection will not affect the performance significantly.
	struct SExtra
	{
		SExtra() : weakRefFlag(0) {};
		asCLockableSharedBool *weakRefFlag;
		asCArray<asPWORD>      userData;
	};
	mutable SExtra *extra;
};

void ScriptObject_Construct(asCObjectType *objType, asCScriptObject *self);
asCScriptObject &ScriptObject_Assignment(asCScriptObject *other, asCScriptObject *self);

void ScriptObject_ConstructUnitialized(asCObjectType *objType, asCScriptObject *self);

void RegisterScriptObject(asCScriptEngine *engine);

asIScriptObject *ScriptObjectFactory(const asCObjectType *objType, asCScriptEngine *engine);
asIScriptObject *ScriptObjectCopyFactory(const asCObjectType *objType, void *origObj, asCScriptEngine *engine);

END_AS_NAMESPACE

#endif
