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
class asCFuncdefType;
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
	asLPT_REPEAT_SAME,
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

enum asEFuncTrait
{
	asTRAIT_CONSTRUCTOR = 1,
	asTRAIT_DESTRUCTOR  = 2,
	asTRAIT_CONST       = 4,
	asTRAIT_PRIVATE     = 8,
	asTRAIT_PROTECTED   = 16,
	asTRAIT_FINAL       = 32,
	asTRAIT_OVERRIDE    = 64,
	asTRAIT_SHARED      = 128,
	asTRAIT_EXTERNAL    = 256
};

struct asSFunctionTraits
{
	asSFunctionTraits() : traits(0) {}
	void SetTrait(asEFuncTrait trait, bool set) { if (set) traits |= trait; else traits &= ~trait; }
	bool GetTrait(asEFuncTrait trait) const { return (traits & trait) ? true : false; }
protected:
	asDWORD traits;
};

struct asSObjectVariableInfo
{
	asUINT              programPos;
	int                 variableOffset;
	asEObjVarInfoOption option;
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
	void                *GetAuxiliary() const;

	// Function signature
	asITypeInfo         *GetObjectType() const;
	const char          *GetObjectName() const;
	const char          *GetName() const;
	const char          *GetNamespace() const;
	const char          *GetDeclaration(bool includeObjectName = true, bool includeNamespace = false, bool includeParamNames = false) const;
	bool                 IsReadOnly() const;
	bool                 IsPrivate() const;
	bool                 IsProtected() const;
	bool                 IsFinal() const;
	bool                 IsOverride() const;
	bool                 IsShared() const;
	asUINT               GetParamCount() const;
	int                  GetParam(asUINT index, int *typeId, asDWORD *flags = 0, const char **name = 0, const char **defaultArg = 0) const;
	int                  GetReturnTypeId(asDWORD *flags = 0) const;

	// Type id for function pointers
	int                  GetTypeId() const;
	bool                 IsCompatibleWithTypeId(int typeId) const;

	// Delegates
	void                *GetDelegateObject() const;
	asITypeInfo         *GetDelegateObjectType() const;
	asIScriptFunction   *GetDelegateFunction() const;

	// Debug information
	asUINT               GetVarCount() const;
	int                  GetVar(asUINT index, const char **name, int *typeId = 0) const;
	const char *         GetVarDecl(asUINT index, bool includeNamespace = false) const;
	int                  FindNextLineWithCode(int line) const;

	// For JIT compilation
	asDWORD             *GetByteCode(asUINT *length = 0);

	// User data
	void                *SetUserData(void *userData, asPWORD type);
	void                *GetUserData(asPWORD type) const;

public:
	//-----------------------------------
	// Internal methods

	void SetShared(bool set) {traits.SetTrait(asTRAIT_SHARED, set);}
	void SetReadOnly(bool set) { traits.SetTrait(asTRAIT_CONST, set); }
	void SetFinal(bool set) { traits.SetTrait(asTRAIT_FINAL, set); }
	void SetOverride(bool set) { traits.SetTrait(asTRAIT_OVERRIDE, set); }
	void SetProtected(bool set) { traits.SetTrait(asTRAIT_PROTECTED, set); }
	void SetPrivate(bool set) { traits.SetTrait(asTRAIT_PRIVATE, set); }

	asCScriptFunction(asCScriptEngine *engine, asCModule *mod, asEFuncType funcType);
	~asCScriptFunction();

	// Keep an internal reference counter to separate references coming from
	// application or script objects and references coming from the script code
	int AddRefInternal();
	int ReleaseInternal();

	void     DestroyHalfCreated();

	// TODO: operator==
	// TODO: The asIScriptFunction should provide operator== and operator!= that should do a
	//       a value comparison. Two delegate objects that point to the same object and class method should compare as equal
	// TODO: The operator== should also be provided in script as opEquals to allow the same comparison in script
	//       To do this we'll need some way to adapt the argtype for opEquals for each funcdef, preferrably without instantiating lots of different methods
	//       Perhaps reusing 'auto' to mean the same type as the object
	//bool      operator==(const asCScriptFunction &other) const;

	void      DestroyInternal();

	void      AddVariable(asCString &name, asCDataType &type, int stackOffset);

	int       GetSpaceNeededForArguments();
	int       GetSpaceNeededForReturnValue();
	asCString GetDeclarationStr(bool includeObjectName = true, bool includeNamespace = false, bool includeParamNames = false) const;
	int       GetLineNumber(int programPosition, int *sectionIdx);
	void      ComputeSignatureId();
	bool      IsSignatureEqual(const asCScriptFunction *func) const;
	bool      IsSignatureExceptNameEqual(const asCScriptFunction *func) const;
	bool      IsSignatureExceptNameEqual(const asCDataType &retType, const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &inOutFlags, const asCObjectType *type, bool isReadOnly) const;
	bool      IsSignatureExceptNameAndReturnTypeEqual(const asCScriptFunction *fun) const;
	bool      IsSignatureExceptNameAndReturnTypeEqual(const asCArray<asCDataType> &paramTypes, const asCArray<asETypeModifiers> &inOutFlags, const asCObjectType *type, bool isReadOnly) const;
	bool      IsSignatureExceptNameAndObjectTypeEqual(const asCScriptFunction *func) const;

	asCTypeInfo *GetTypeInfoOfLocalVar(short varOffset);

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

	// GC methods (for delegates)
	int  GetRefCount();
	void SetFlag();
	bool GetFlag();
	void EnumReferences(asIScriptEngine *engine);
	void ReleaseAllHandles(asIScriptEngine *engine);

public:
	//-----------------------------------
	// Properties

	mutable asCAtomic            externalRefCount; // Used for external referneces
	        asCAtomic            internalRefCount; // Used for internal references
	mutable bool                 gcFlag;
	asCScriptEngine             *engine;
	asCModule                   *module;

	asCArray<asPWORD>            userData;

	// Function signature
	asCString                    name;
	asCDataType                  returnType;
	asCArray<asCDataType>        parameterTypes;
	asCArray<asCString>          parameterNames;
	asCArray<asETypeModifiers>   inOutFlags;
	asCArray<asCString *>        defaultArgs;
	asSFunctionTraits            traits;
	asCObjectType               *objectType;
	int                          signatureId;

	int                          id;

	asEFuncType                  funcType;
	asDWORD                      accessMask;

	// Namespace will be null for funcdefs that are declared as child funcdefs
	// of a class. In this case the namespace shall be taken from the parentClass
	// in the funcdefType
	asSNameSpace                *nameSpace;

	asCFuncdefType              *funcdefType; // Doesn't increase refCount

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

		// These hold information on objects and function pointers, including temporary
		// variables used by exception handler and when saving bytecode
		asCArray<asCTypeInfo*>          objVariableTypes;
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
		// Store the location where the function was declared  (row in the lower 20 bits, and column in the upper 12)
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

const char * const DELEGATE_FACTORY = "$dlgte";
asCScriptFunction *CreateDelegate(asCScriptFunction *func, void *obj);

END_AS_NAMESPACE

#endif
