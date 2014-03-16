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
// as_datatype.h
//
// This class describes the datatype for expressions during compilation
//



#ifndef AS_DATATYPE_H
#define AS_DATATYPE_H

#include "as_tokendef.h"
#include "as_string.h"

BEGIN_AS_NAMESPACE

struct asSTypeBehaviour;
class asCScriptEngine;
class asCObjectType;
class asCScriptFunction;

// TODO: refactor: Reference should not be part of the datatype. This should be stored separately, e.g. in asCTypeInfo
//                 MakeReference, MakeReadOnly, IsReference, IsReadOnly should be removed

class asCDataType
{
public:
	asCDataType();
	asCDataType(const asCDataType &);
	~asCDataType();

	bool IsValid() const;

	asCString Format(bool includeNamespace = false) const;

	static asCDataType CreatePrimitive(eTokenType tt, bool isConst);
	static asCDataType CreateObject(asCObjectType *ot, bool isConst);
	static asCDataType CreateObjectHandle(asCObjectType *ot, bool isConst);
	static asCDataType CreateFuncDef(asCScriptFunction *ot);
	static asCDataType CreateNullHandle();

	int MakeHandle(bool b, bool acceptHandleForScope = false);
	int MakeArray(asCScriptEngine *engine);
	int MakeReference(bool b);
	int MakeReadOnly(bool b);
	int MakeHandleToConst(bool b);

	bool IsTemplate()       const;
	bool IsScriptObject()   const;
	bool IsPrimitive()      const;
	bool IsObject()         const;
	bool IsReference()      const {return isReference;}
	bool IsReadOnly()       const; 
	bool IsIntegerType()    const;
	bool IsUnsignedType()   const;
	bool IsFloatType()      const;
	bool IsDoubleType()     const;
	bool IsBooleanType()    const;
	bool IsObjectHandle()   const {return isObjectHandle;}
	bool IsHandleToConst()  const;
	bool IsArrayType()      const;
	bool IsEnumType()       const;
	bool IsAnyType()        const {return tokenType == ttQuestion;}

	bool IsEqualExceptRef(const asCDataType &)             const;
	bool IsEqualExceptRefAndConst(const asCDataType &)     const;
	bool IsEqualExceptConst(const asCDataType &)           const;
	bool IsNullHandle()                                    const;

	bool SupportHandles() const;
	bool CanBeInstanciated() const;
	bool CanBeCopied() const;

	bool operator ==(const asCDataType &) const;
	bool operator !=(const asCDataType &) const;

	asCDataType        GetSubType(asUINT subtypeIndex = 0)    const;
	eTokenType         GetTokenType()  const {return tokenType;}
	asCObjectType     *GetObjectType() const {return objectType;}
	asCScriptFunction *GetFuncDef()    const {return funcDef;}

	int  GetSizeOnStackDWords()  const;
	int  GetSizeInMemoryBytes()  const;
	int  GetSizeInMemoryDWords() const;

	void SetTokenType(eTokenType tt)         {tokenType = tt;}
	void SetObjectType(asCObjectType *obj)   {objectType = obj;}
	void SetFuncDef(asCScriptFunction *func) {asASSERT(funcDef); funcDef = func; }

	asCDataType &operator =(const asCDataType &);

	asSTypeBehaviour *GetBehaviour() const;

protected:
	// Base object type
	eTokenType tokenType;

	// Behaviour type
	asCObjectType *objectType;
	asCScriptFunction *funcDef;

	// Top level
	bool isReference:1;
	bool isReadOnly:1;
	bool isObjectHandle:1;
	bool isConstHandle:1;
	char dummy:4;
};

END_AS_NAMESPACE

#endif
