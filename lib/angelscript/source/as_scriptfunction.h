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
// as_scriptfunction.h
//
// A container for a compiled script function
//



#ifndef AS_SCRIPTFUNCTION_H
#define AS_SCRIPTFUNCTION_H

#include "as_config.h"
#include "as_string.h"
#include "as_array.h"
#include "as_datatype.h"
#include "as_atomic.h"

BEGIN_AS_NAMESPACE

class asCScriptEngine;
class asCModule;
class asCConfigGroup;
class asCGlobalProperty;
class asCScriptNode;
struct asSNameSpace;

struct asSScriptVariable
{
	asCString   name;
	asCDataType type;
	int         stackOffset;
	asUINT      declaredAtProgramPos;
};

enum asEListPatternNodeType
{
	asLPT_REPEAT,
	asLPT_START,
	asLPT_END,
	asLPT_TYPE
};

struct asSListPatternNode
{
	asSListPatternNode(asEListPatternNodeType t) : type(t), next(0) {}
	virtual ~asSListPatternNode() {};
	virtual asSListPatternNode *Duplicate() { return asNEW(asSListPatternNode)(type); }
	asEListPatternNodeType  type;
	asSListPatternNode     *next;
};

struct asSListPatternDataTypeNode : public asSListPatternNode
{
	asSListPatternDataTypeNode(const asCDataType &dt) : asSListPatternNode(asLPT_TYPE), dataType(dt) {}
	asSListPatternNode *Duplicate() { return asNEW(asSListPatternDataTypeNode)(dataType); }
	asCDataType dataType;
};

enum asEObjVarInfoOption
{
	asOBJ_UNINIT,
	asOBJ_INIT,
	asBLOCK_BEGIN,
	asBLOCK_END
};

struct asSObjectVariableInfo
{
	asUINT programPos;
	int    variableOffset;
	asUINT option;
};

struct asSSystemFunctionInterface;

// TODO: Might be interesting to allow enumeration of accessed global variables, and 
//       also functions/methods that are being called. This could be used to build a 
//       code database with call graphs, etc.

void RegisterScriptFunction(asCScriptEngine *engine);

class asCScriptFunction : public asIScriptFunction
{
public:
	// From asIScriptFunction
	asIScriptEngine     *GetEngine() const;

	// Memory management
	int AddRef() const;
	int Release() const;

	// Miscellaneous
	int                  GetId() const;
	asEFuncType          GetFuncType() const;
	const char          *GetModuleName() const;
	asIScriptModule     *GetModule() const;
	const char          *GetScriptSectionName() const;
	const char          *GetConfigGroup() const;
	asDWORD              GetAccessMask() const;

	// Function signature
	asIObjectType       *GetObjectType() const;
	const char          *GetObjectName() const;
	const char          *GetName() const;
	const char          *GetNamespace() const;
	const char          *GetDeclaration(bool includeObjectName = true, bool includeNamespace = false) const;
	bool                 IsReadOnly() const;
	bool                 IsPrivate() const;
	bool                 IsFinal() const;
	bool                 IsOverride() const;
	bool                 IsShared() const;
	asUINT               GetParamCount() const;
	int                  GetParamTypeId(asUINT index, asDWORD *flags = 0) const;
	int                  GetReturnTypeId(asDWORD *flags = 0) const;

	// Type id for function pointers 
	int                  GetTypeId() const;
	bool                 IsCompatibleWithTypeId(int typeId) const;

	// Delegates
	void                *GetDelegateObject() const;
	asIObjectType       *GetDelegateObjectType() const;
	asIScriptFunction   *GetDelegateFunction() const;

	// Debug information
	asUINT               GetVarCount() const;
	int                  GetVar(asUINT index, const char **name, int *typeId = 0) const;
	const char *         GetVarDecl(asUINT index, bool includeNamespace = false) const;
	int                  FindNextLineWithCode(int line) const;

	// For JIT compilation
	asDWORD             *GetByteCode(asUINT *length = 0);

	// User data
	void                *SetUserData(void *userData);
	void                *GetUserData() const;

public:
	//-----------------------------------
	// Internal methods

	asCScriptFunction(asCScriptEngine *engine, asCModule *mod, asEFuncType funcType);
	~asCScriptFunction();

	void      DestroyInternal();
	void      Orphan(asIScriptModule *mod);

	void      AddVariable(asCString &name, asCDataType &type, int stackOffset);

	int       GetSpaceNeededForArguments();
	int       GetSpaceNeededForReturnValue();
	asCString GetDeclarationStr(bool includeObjectName = true, bool includeNamespace = false) const;
	int       GetLineNumber(int programPosition, int *sectionIdx);
	void      ComputeSignatureId();
	bool      IsSignatureEqual(const asCScriptFunction *func) const;
	bool      IsSignatureExceptNameEqual(const asCScriptFunction *func) const;
	bool      IsSignatureExceptNameEqual(const asCDataType &retType, const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &inOutFlags, const asCObjectType *type, bool isReadOnly) const;
	bool      IsSignatureExceptNameAndReturnTypeEqual(const asCScriptFunction *fun) const;
	bool      IsSignatureExceptNameAndReturnTypeEqual(const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &inOutFlags, const asCObjectType *type, bool isReadOnly) const;
	bool      IsSignatureExceptNameAndObjectTypeEqual(const asCScriptFunction *func) const;

	asCObjectType *GetObjectTypeOfLocalVar(short varOffset);

	void      MakeDelegate(asCScriptFunction *func, void *obj);

	int       RegisterListPattern(const char *decl, asCScriptNode *listPattern);
	int       ParseListPattern(asSListPatternNode *&target, const char *decl, asCScriptNode *listPattern);

	bool      DoesReturnOnStack() const;

	void      JITCompile();

	void      AddReferences();
	void      ReleaseReferences();

	void      AllocateScriptFunctionData();
	void      DeallocateScriptFunctionData();

	asCGlobalProperty *GetPropertyByGlobalVarPtr(void *gvarPtr);

	// GC methods
	int  GetRefCount();
	void SetFlag();
	bool GetFlag();
	void EnumReferences(asIScriptEngine *engine);
	void ReleaseAllHandles(asIScriptEngine *engine);

public:
	//-----------------------------------
	// Properties

	mutable asCAtomic            refCount;
	mutable bool                 gcFlag;
	asCScriptEngine             *engine;
	asCModule                   *module;

	void                        *userData;

	// Function signature
	asCString                    name;
	asCDataType                  returnType;
	asCArray<asCDataType>        parameterTypes;
	asCArray<asETypeModifiers>   inOutFlags;
	asCArray<asCString *>        defaultArgs;
	bool                         isReadOnly;
	bool                         isPrivate;
	bool                         isFinal;
	bool                         isOverride;
	asCObjectType               *objectType;
	int                          signatureId;

	int                          id;

	asEFuncType                  funcType;
	asDWORD                      accessMask;
	bool                         isShared;

	asSNameSpace                *nameSpace;

	// Used by asFUNC_DELEGATE
	void              *objForDelegate;
	asCScriptFunction *funcForDelegate;

	// Used by list factory behaviour
	asSListPatternNode *listPattern;

	// Used by asFUNC_SCRIPT
	struct ScriptFunctionData
	{
		// Bytecode for the script function
		asCArray<asDWORD>               byteCode;

		// The stack space needed for the local variables
		asDWORD                         variableSpace;

		// These hold information objects and function pointers, including temporary
		// variables used by exception handler and when saving bytecode
		asCArray<asCObjectType*>        objVariableTypes;
		asCArray<asCScriptFunction*>    funcVariableTypes;
		asCArray<int>                   objVariablePos;

		// The first variables in above array are allocated on the heap, the rest on the stack.
		// This variable shows how many are on the heap.
		asUINT                          objVariablesOnHeap;

		// Holds information on scope for object variables on the stack
		asCArray<asSObjectVariableInfo> objVariableInfo;

		// The stack needed to execute the function
		int                             stackNeeded;

		// JIT compiled code of this function
		asJITFunction                   jitFunction;

		// Holds debug information on explicitly declared variables
		asCArray<asSScriptVariable*>    variables;
		// Store position, line number pairs for debug information
		asCArray<int>                   lineNumbers;
		// Store the script section where the code was declared
		int                             scriptSectionIdx;
		// Store the location where the function was declared
		int                             declaredAt;
		// Store position/index pairs if the bytecode is compiled from multiple script sections
		asCArray<int>                   sectionIdxs;
	};
	ScriptFunctionData          *scriptData;

	// Stub functions and delegates don't own the object and parameters
	bool                         dontCleanUpOnException;

	// Used by asFUNC_VIRTUAL
	int                          vfTableIdx;

	// Used by asFUNC_SYSTEM
	asSSystemFunctionInterface  *sysFuncIntf;
};

const char * const DELEGATE_FACTORY = "%delegate_factory";
asCScriptFunction *CreateDelegate(asCScriptFunction *func, void *obj);

END_AS_NAMESPACE

#endif
