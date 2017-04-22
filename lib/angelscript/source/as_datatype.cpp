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
// as_datatype.cpp
//
// This class describes the datatype for expressions during compilation
//

#include "as_config.h"
#include "as_datatype.h"
#include "as_tokendef.h"
#include "as_objecttype.h"
#include "as_scriptengine.h"
#include "as_tokenizer.h"

BEGIN_AS_NAMESPACE

asCDataType::asCDataType()
{
	tokenType              = ttUnrecognizedToken;
	objectType             = 0;
	isReference            = false;
	isReadOnly             = false;
	isAuto                 = false;
	isObjectHandle         = false;
	isConstHandle          = false;
	funcDef                = 0;
	isHandleToAsHandleType = false;
}

asCDataType::asCDataType(const asCDataType &dt)
{
	tokenType              = dt.tokenType;
	objectType             = dt.objectType;
	isReference            = dt.isReference;
	isReadOnly             = dt.isReadOnly;
	isAuto                 = dt.isAuto;
	isObjectHandle         = dt.isObjectHandle;
	isConstHandle          = dt.isConstHandle;
	funcDef                = dt.funcDef;
	isHandleToAsHandleType = dt.isHandleToAsHandleType;
}

asCDataType::~asCDataType()
{
}

bool asCDataType::IsValid() const
{
	if( tokenType == ttUnrecognizedToken &&
		!isObjectHandle )
		return false;

	return true;
}

asCDataType asCDataType::CreateObject(asCObjectType *ot, bool isConst)
{
	asCDataType dt;

	dt.tokenType        = ttIdentifier;
	dt.objectType       = ot;
	dt.isReadOnly       = isConst;

	return dt;
}

asCDataType asCDataType::CreateAuto(bool isConst)
{
	asCDataType dt;

	dt.tokenType        = ttIdentifier;
	dt.isReadOnly       = isConst;
	dt.isAuto           = true;

	return dt;
}

asCDataType asCDataType::CreateObjectHandle(asCObjectType *ot, bool isConst)
{
	asCDataType dt;

	dt.tokenType        = ttIdentifier;
	dt.objectType       = ot;
	dt.isObjectHandle   = true;
	dt.isConstHandle    = isConst;

	return dt;
}

asCDataType asCDataType::CreateFuncDef(asCScriptFunction *func)
{
	asCDataType dt;
	dt.tokenType        = ttIdentifier;
	dt.funcDef          = func;
	dt.objectType       = &func->engine->functionBehaviours;

	return dt;
}

asCDataType asCDataType::CreatePrimitive(eTokenType tt, bool isConst)
{
	asCDataType dt;

	dt.tokenType        = tt;
	dt.isReadOnly       = isConst;

	return dt;
}

asCDataType asCDataType::CreateNullHandle()
{
	asCDataType dt;

	dt.tokenType = ttUnrecognizedToken;
	dt.isReadOnly = true;
	dt.isObjectHandle = true;
	dt.isConstHandle = true;

	return dt;
}

bool asCDataType::IsNullHandle() const
{
	if( tokenType == ttUnrecognizedToken &&
		objectType == 0 &&
		isObjectHandle  )
		return true;

	return false;
}

asCString asCDataType::Format(asSNameSpace *currNs, bool includeNamespace) const
{
	if( IsNullHandle() )
		return "<null handle>";

	asCString str;

	if( isReadOnly )
		str = "const ";

	// If the type is not declared in the current namespace, then the namespace 
	// must always be informed to guarantee that the correct type is informed
	if( includeNamespace || (objectType && objectType->nameSpace != currNs) || (funcDef && funcDef->nameSpace != currNs) )
	{
		if( objectType && objectType->nameSpace->name != "" )
			str += objectType->nameSpace->name + "::";
		else if( funcDef && funcDef->nameSpace->name != "" )
			str += funcDef->nameSpace->name + "::";
	}

	if( tokenType != ttIdentifier )
	{
		str += asCTokenizer::GetDefinition(tokenType);
	}
	else if( IsArrayType() && objectType && !objectType->engine->ep.expandDefaultArrayToTemplate )
	{
		asASSERT( objectType->templateSubTypes.GetLength() == 1 );
		str += objectType->templateSubTypes[0].Format(currNs, includeNamespace);
		str += "[]";
	}
	else if( funcDef )
	{
		str += funcDef->name;
	}
	else if( objectType )
	{
		str += objectType->name;
		if( objectType->templateSubTypes.GetLength() > 0 )
		{
			str += "<";
			for( asUINT subtypeIndex = 0; subtypeIndex < objectType->templateSubTypes.GetLength(); subtypeIndex++ )
			{
				str += objectType->templateSubTypes[subtypeIndex].Format(currNs, includeNamespace);
				if( subtypeIndex != objectType->templateSubTypes.GetLength()-1 )
					str += ",";
			}
			str += ">";
		}
	}
	else if( isAuto )
	{
		if( isObjectHandle )
			str += "<auto@>";
		else
			str += "<auto>";
	}
	else
	{
		str = "<unknown>";
	}

	if( isObjectHandle )
	{
		str += "@";
		if( isConstHandle )
			str += "const";
	}

	if( isReference )
		str += "&";

	return str;
}

asCDataType &asCDataType::operator =(const asCDataType &dt)
{
	tokenType              = dt.tokenType;
	isReference            = dt.isReference;
	objectType             = dt.objectType;
	isReadOnly             = dt.isReadOnly;
	isObjectHandle         = dt.isObjectHandle;
	isConstHandle          = dt.isConstHandle;
	isAuto                 = dt.isAuto;
	funcDef                = dt.funcDef;
	isHandleToAsHandleType = dt.isHandleToAsHandleType;

	return (asCDataType &)*this;
}

int asCDataType::MakeHandle(bool b, bool acceptHandleForScope)
{
	if( !b )
	{
		isObjectHandle = false;
		isConstHandle = false;
		isHandleToAsHandleType = false;
	}
	else
	{
		if( isAuto )
		{
			isObjectHandle = true;
		}
		else if( !isObjectHandle )
		{
			// Only reference types are allowed to be handles, 
			// but not nohandle reference types, and not scoped references 
			// (except when returned from registered function)
			// funcdefs are special reference types and support handles
			// value types with asOBJ_ASHANDLE are treated as a handle
			if( !funcDef && 
				(!objectType || 
				!((objectType->flags & asOBJ_REF) || (objectType->flags & asOBJ_TEMPLATE_SUBTYPE) || (objectType->flags & asOBJ_ASHANDLE)) || 
				(objectType->flags & asOBJ_NOHANDLE) || 
				((objectType->flags & asOBJ_SCOPED) && !acceptHandleForScope)) )
				return -1;

			isObjectHandle = b;
			isConstHandle = false;

			// ASHANDLE supports being handle, but as it really is a value type it will not be marked as a handle
			if( (objectType->flags & asOBJ_ASHANDLE) )
			{
				isObjectHandle = false;
				isHandleToAsHandleType = true;
			}
		}
	}

	return 0;
}

int asCDataType::MakeArray(asCScriptEngine *engine, asCModule *module)
{
	if( engine->defaultArrayObjectType == 0 )
		return asINVALID_TYPE;

	bool tmpIsReadOnly = isReadOnly;
	isReadOnly = false;
	asCArray<asCDataType> subTypes;
	subTypes.PushLast(*this);
	asCObjectType *at = engine->GetTemplateInstanceType(engine->defaultArrayObjectType, subTypes, module);
	isReadOnly = tmpIsReadOnly;

	isObjectHandle = false;
	isConstHandle = false;
	
	objectType = at;
	tokenType = ttIdentifier;

	return 0;
}

int asCDataType::MakeReference(bool b)
{
	isReference = b;

	return 0;
}

int asCDataType::MakeReadOnly(bool b)
{
	if( isObjectHandle )
	{
		isConstHandle = b;
		return 0;
	}

	isReadOnly = b;
	return 0;
}

int asCDataType::MakeHandleToConst(bool b)
{
	if( !isObjectHandle ) return -1;

	isReadOnly = b;
	return 0;
}

bool asCDataType::SupportHandles() const
{
	if( objectType &&
		(objectType->flags & (asOBJ_REF | asOBJ_ASHANDLE)) && 
		!(objectType->flags & asOBJ_NOHANDLE) &&
		!isObjectHandle )
		return true;

	return false;
}

bool asCDataType::CanBeInstantiated() const
{
	if( GetSizeOnStackDWords() == 0 ) // Void
		return false;

	if( !IsObject() ) // Primitives
		return true; 

	if( IsObjectHandle() && !(objectType->flags & asOBJ_NOHANDLE) ) // Handles
		return true;

	if( funcDef ) // Funcdefs can be instantiated as delegates
		 return true;

	if( (objectType->flags & asOBJ_REF) && objectType->beh.factories.GetLength() == 0 ) // ref types without factories
		return false;

	if( (objectType->flags & asOBJ_ABSTRACT) && !IsObjectHandle() ) // Can't instantiate abstract classes
		return false;

	return true;
}

bool asCDataType::IsAbstractClass() const
{
	return objectType && (objectType->flags & asOBJ_ABSTRACT) ? true : false;
}

bool asCDataType::IsInterface() const
{
	return objectType && objectType->IsInterface();
}

bool asCDataType::CanBeCopied() const
{
	// All primitives can be copied
	if( IsPrimitive() ) return true;

	// Plain-old-data structures can always be copied
	if( objectType->flags & asOBJ_POD ) return true;

	// It must be possible to instantiate the type
	if( !CanBeInstantiated() ) return false;

	// It must have a default constructor or factory
	if( objectType->beh.construct == 0 &&
		objectType->beh.factory   == 0 ) return false;

	// It must be possible to copy the type
	if( objectType->beh.copy == 0 ) return false;

	return true;
}

bool asCDataType::IsReadOnly() const
{
	if( isObjectHandle )
		return isConstHandle;

	return isReadOnly;
}

bool asCDataType::IsHandleToConst() const
{
	if( !isObjectHandle ) return false;
	return isReadOnly;
}

bool asCDataType::IsObjectConst() const
{
	if( IsObjectHandle() )
		return IsHandleToConst();

	return IsReadOnly();
}

// TODO: 3.0.0: This should be removed
bool asCDataType::IsArrayType() const
{
	// This is only true if the type used is the default array type, i.e. the one used for the [] syntax form
	if( objectType && objectType->engine->defaultArrayObjectType )
		return objectType->name == objectType->engine->defaultArrayObjectType->name;
	
	return false;
}

bool asCDataType::IsTemplate() const
{
	if( objectType && (objectType->flags & asOBJ_TEMPLATE) )
		return true;

	return false;
}

bool asCDataType::IsScriptObject() const
{
	if( objectType && (objectType->flags & asOBJ_SCRIPT_OBJECT) )
		return true;

	return false;
}

asCDataType asCDataType::GetSubType(asUINT subtypeIndex) const
{
	asASSERT(objectType);
	return objectType->templateSubTypes[subtypeIndex];
}


bool asCDataType::operator !=(const asCDataType &dt) const
{
	return !(*this == dt);
}

bool asCDataType::operator ==(const asCDataType &dt) const
{
	if( !IsEqualExceptRefAndConst(dt) ) return false;
	if( isReference != dt.isReference ) return false;
	if( isReadOnly != dt.isReadOnly ) return false;
	if( isConstHandle != dt.isConstHandle ) return false;

	return true;
}

bool asCDataType::IsEqualExceptRef(const asCDataType &dt) const
{
	if( !IsEqualExceptRefAndConst(dt) ) return false;
	if( isReadOnly != dt.isReadOnly ) return false;
	if( isConstHandle != dt.isConstHandle ) return false;

	return true;
}

bool asCDataType::IsEqualExceptRefAndConst(const asCDataType &dt) const
{
	// Check base type
	if( tokenType != dt.tokenType ) return false;
	if( objectType != dt.objectType ) return false;
	if( isObjectHandle != dt.isObjectHandle ) return false;
	if( isObjectHandle )
		if( isReadOnly != dt.isReadOnly ) return false;
	if( funcDef != dt.funcDef ) return false;

	return true;
}

bool asCDataType::IsEqualExceptConst(const asCDataType &dt) const
{
	if( !IsEqualExceptRefAndConst(dt) ) return false;
	if( isReference != dt.isReference ) return false;

	return true;
}

bool asCDataType::IsPrimitive() const
{
	// Enumerations are primitives
	if( IsEnumType() )
		return true;

	// A registered object is never a primitive neither is a pointer, nor an array
	if( objectType || funcDef )
		return false;

	// Null handle doesn't have an objectType, but it is not a primitive
	if( tokenType == ttUnrecognizedToken )
		return false;

	return true;
}

bool asCDataType::IsMathType() const
{
	if( tokenType == ttInt || tokenType == ttInt8 || tokenType == ttInt16 || tokenType == ttInt64 ||
		tokenType == ttUInt || tokenType == ttUInt8 || tokenType == ttUInt16 || tokenType == ttUInt64 ||
		tokenType == ttFloat || tokenType == ttDouble )
		return true;

	return false;
}

bool asCDataType::IsIntegerType() const
{
	if( tokenType == ttInt ||
		tokenType == ttInt8 ||
		tokenType == ttInt16 ||
		tokenType == ttInt64 )
		return true;

	// Enums are also integer types
	return IsEnumType();
}

bool asCDataType::IsUnsignedType() const
{
	if( tokenType == ttUInt ||
		tokenType == ttUInt8 ||
		tokenType == ttUInt16 ||
		tokenType == ttUInt64 )
		return true;

	return false;
}

bool asCDataType::IsFloatType() const
{
	if( tokenType == ttFloat )
		return true;

	return false;
}

bool asCDataType::IsDoubleType() const
{
	if( tokenType == ttDouble )
		return true;

	return false;
}

bool asCDataType::IsBooleanType() const
{
	if( tokenType == ttBool )
		return true;

	return false;
}

bool asCDataType::IsObject() const
{
	if( IsPrimitive() )
		return false;

	// Null handle doesn't have an object type but should still be considered an object
	if( objectType == 0 ) 
		return IsNullHandle();

	return true;
}

int asCDataType::GetSizeInMemoryBytes() const
{
	if( objectType != 0 )
		return objectType->size;

	if( tokenType == ttVoid )
		return 0;

	if( tokenType == ttInt8 ||
		tokenType == ttUInt8 )
		return 1;

	if( tokenType == ttInt16 ||
		tokenType == ttUInt16 )
		return 2;

	if( tokenType == ttDouble ||
		tokenType == ttInt64 ||
		tokenType == ttUInt64 )
		return 8;

	if( tokenType == ttBool )
		return AS_SIZEOF_BOOL;

	// null handle
	if( tokenType == ttUnrecognizedToken )
		return 4*AS_PTR_SIZE;

	return 4;
}

int asCDataType::GetSizeInMemoryDWords() const
{
	int s = GetSizeInMemoryBytes();
	if( s == 0 ) return 0;
	if( s <= 4 ) return 1;
	
	// Pad the size to 4 bytes
	if( s & 0x3 )
		s += 4 - (s & 0x3);

	return s/4;
}

int asCDataType::GetSizeOnStackDWords() const
{
	// If the type is the variable type then the typeid is stored on the stack too
	int size = tokenType == ttQuestion ? 1 : 0;

	if( isReference ) return AS_PTR_SIZE + size;
	if( objectType && !IsEnumType() ) return AS_PTR_SIZE + size;

	return GetSizeInMemoryDWords() + size;
}

#ifdef WIP_16BYTE_ALIGN
int  asCDataType::GetAlignment() const
{
	if( objectType == NULL )
	{
		// TODO: Small primitives should not be aligned to 4 byte boundaries
		return 4; //Default alignment
	}

	return objectType->alignment;
}
#endif

asSTypeBehaviour *asCDataType::GetBehaviour() const
{ 
	return objectType ? &objectType->beh : 0; 
}

bool asCDataType::IsEnumType() const
{
	// Do a sanity check on the objectType, to verify that we aren't trying to access memory after it has been released
	asASSERT( objectType == 0 || objectType->name.GetLength() < 100 );

	if( objectType && (objectType->flags & asOBJ_ENUM) )
		return true;

	return false;
}

END_AS_NAMESPACE

