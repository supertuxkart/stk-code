/*
   AngelCode Scripting Library
   Copyright (c) 2003-2020 Andreas Jonsson

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
// as_module.h
//
// A class that holds a script module
//

#ifndef AS_MODULE_H
#define AS_MODULE_H

#include "as_config.h"
#include "as_symboltable.h"
#include "as_atomic.h"
#include "as_string.h"
#include "as_array.h"
#include "as_datatype.h"
#include "as_scriptfunction.h"
#include "as_property.h"

BEGIN_AS_NAMESPACE

// TODO: import: Remove this when the imported functions are removed
const int FUNC_IMPORTED = 0x40000000;

class asCScriptEngine;
class asCCompiler;
class asCBuilder;
class asCContext;
class asCConfigGroup;
class asCTypedefType;
class asCFuncdefType;
struct asSNameSpace;

struct sBindInfo
{
	asCScriptFunction *importedFunctionSignature;
	asCString          importFromModule;
	int                boundFunctionId;
};

struct sObjectTypePair
{
	asCObjectType *a;
	asCObjectType *b;
};


// TODO: import: Remove function imports. When I have implemented function
//               pointers the function imports should be deprecated.

// TODO: Need a separate interface for compiling scripts. The asIScriptCompiler
//       will have a target module, and will allow the compilation of an entire
//       script or just individual functions within the scope of the module
//
//       With this separation it will be possible to compile the library without
//       the compiler, thus giving a much smaller binary executable.

// TODO: There should be a special compile option that will let the application
//       recompile an already compiled script. The compiler should check if no
//       destructive changes have been made (changing function signatures, etc)
//       then it should simply replace the bytecode within the functions without
//       changing the values of existing global properties, etc.

class asCModule : public asIScriptModule
{
//-------------------------------------------
// Public interface
//--------------------------------------------
public:
	virtual asIScriptEngine *GetEngine() const;
	virtual void             SetName(const char *name);
	virtual const char      *GetName() const;
	virtual void             Discard();

	// Compilation
	virtual int         AddScriptSection(const char *name, const char *code, size_t codeLength, int lineOffset);
	virtual int         Build();
	virtual int         CompileFunction(const char *sectionName, const char *code, int lineOffset, asDWORD reserved, asIScriptFunction **outFunc);
	virtual int         CompileGlobalVar(const char *sectionName, const char *code, int lineOffset);
	virtual asDWORD     SetAccessMask(asDWORD accessMask);
	virtual int         SetDefaultNamespace(const char *nameSpace);
	virtual const char *GetDefaultNamespace() const;

	// Script functions
	virtual asUINT             GetFunctionCount() const;
	virtual asIScriptFunction *GetFunctionByIndex(asUINT index) const;
	virtual asIScriptFunction *GetFunctionByDecl(const char *decl) const;
	virtual asIScriptFunction *GetFunctionByName(const char *name) const;
	virtual int                RemoveFunction(asIScriptFunction *func);

	// Script global variables
	// TODO: interface: Should be called InitGlobalVars, and should have a bool to reset in case already initialized
	virtual int         ResetGlobalVars(asIScriptContext *ctx);
	virtual asUINT      GetGlobalVarCount() const;
	virtual int         GetGlobalVarIndexByName(const char *name) const;
	virtual int         GetGlobalVarIndexByDecl(const char *decl) const;
	virtual const char *GetGlobalVarDeclaration(asUINT index, bool includeNamespace) const;
	virtual int         GetGlobalVar(asUINT index, const char **name, const char **nameSpace, int *typeId, bool *isConst) const;
	virtual void       *GetAddressOfGlobalVar(asUINT index);
	virtual int         RemoveGlobalVar(asUINT index);

	// Type identification
	virtual asUINT         GetObjectTypeCount() const;
	virtual asITypeInfo   *GetObjectTypeByIndex(asUINT index) const;
	virtual int            GetTypeIdByDecl(const char *decl) const;
	virtual asITypeInfo   *GetTypeInfoByName(const char *name) const;
	virtual asITypeInfo   *GetTypeInfoByDecl(const char *decl) const;

	// Enums
	virtual asUINT       GetEnumCount() const;
	virtual asITypeInfo *GetEnumByIndex(asUINT index) const;

	// Typedefs
	virtual asUINT       GetTypedefCount() const;
	virtual asITypeInfo *GetTypedefByIndex(asUINT index) const;

	// Dynamic binding between modules
	virtual asUINT      GetImportedFunctionCount() const;
	virtual int         GetImportedFunctionIndexByDecl(const char *decl) const;
	virtual const char *GetImportedFunctionDeclaration(asUINT importIndex) const;
	virtual const char *GetImportedFunctionSourceModule(asUINT importIndex) const;
	virtual int         BindImportedFunction(asUINT index, asIScriptFunction *func);
	virtual int         UnbindImportedFunction(asUINT importIndex);
	virtual int         BindAllImportedFunctions();
	virtual int         UnbindAllImportedFunctions();

	// Bytecode Saving/Loading
	virtual int SaveByteCode(asIBinaryStream *out, bool stripDebugInfo) const;
	virtual int LoadByteCode(asIBinaryStream *in, bool *wasDebugInfoStripped);

	// User data
	virtual void *SetUserData(void *data, asPWORD type);
	virtual void *GetUserData(asPWORD type) const;

//-----------------------------------------------
// Internal
//-----------------------------------------------
	asCModule(const char *name, asCScriptEngine *engine);
	~asCModule();

//protected:
	friend class asCScriptEngine;
	friend class asCBuilder;
	friend class asCCompiler;
	friend class asCContext;
	friend class asCRestore;

	void InternalReset();
	bool IsEmpty() const;
	bool HasExternalReferences(bool shuttingDown);

	int  CallInit(asIScriptContext *ctx);
	void CallExit();
	int  InitGlobalProp(asCGlobalProperty *prop, asIScriptContext *ctx);

	void JITCompile();

#ifndef AS_NO_COMPILER
	int  AddScriptFunction(int sectionIdx, int declaredAt, int id, const asCString &name, const asCDataType &returnType, const asCArray<asCDataType> &params, const asCArray<asCString> &paramNames, const asCArray<asETypeModifiers> &inOutFlags, const asCArray<asCString *> &defaultArgs, bool isInterface, asCObjectType *objType = 0, bool isGlobalFunction = false, asSFunctionTraits funcTraits = asSFunctionTraits(), asSNameSpace *ns = 0);
	int  AddScriptFunction(asCScriptFunction *func);
	int  AddImportedFunction(int id, const asCString &name, const asCDataType &returnType, const asCArray<asCDataType> &params, const asCArray<asETypeModifiers> &inOutFlags, const asCArray<asCString *> &defaultArgs, asSFunctionTraits funcTraits, asSNameSpace *ns, const asCString &moduleName);
	int  AddFuncDef(const asCString &name, asSNameSpace *ns, asCObjectType *parent);
#endif

	int                GetNextImportedFunctionId();
	asCScriptFunction *GetImportedFunction(int funcId) const;
	asCTypeInfo       *GetType(const asCString &type, asSNameSpace *ns) const;
	asCObjectType     *GetObjectType(const char *type, asSNameSpace *ns) const;
	asCGlobalProperty *AllocateGlobalProperty(const char *name, const asCDataType &dt, asSNameSpace *ns);
	void               UninitializeGlobalProp(asCGlobalProperty *prop);
	
	// Adds the class type to the module. The module assumes ownership of the reference without increasing it
	void               AddClassType(asCObjectType*);
	// Adds the enum type to the module. The module assumes ownership of the reference without increasing it
	void               AddEnumType(asCEnumType*);
	// Adds the typedef to the module. The module assumes ownership of the reference without increasing it
	void               AddTypeDef(asCTypedefType*);
	// Adds the funcdef to the module. The module assumes ownership of the reference without increasing it
	void               AddFuncDef(asCFuncdefType*);
	// Replaces an existing funcdef with another (used for shared funcdefs). Doesn't add or release refCounts
	void               ReplaceFuncDef(asCFuncdefType *oldType, asCFuncdefType *newType);

	asCString         m_name;
	asCScriptEngine  *m_engine;
	asCBuilder       *m_builder;
	asCArray<asPWORD> m_userData;
	asDWORD           m_accessMask;
	asSNameSpace     *m_defaultNamespace;

	// This array holds all functions, class members, factories, etc that were compiled with the module.
	// These references hold an internal reference to the function object.
	asCArray<asCScriptFunction *>     m_scriptFunctions; // increases ref count
	// This array holds global functions declared in the module. These references are not counted,
	// as the same pointer is always present in the scriptFunctions array too.
	asCSymbolTable<asCScriptFunction> m_globalFunctions; // doesn't increase ref count
	// This array holds imported functions in the module.
	asCArray<sBindInfo *>             m_bindInformations; // increases ref count
	// This array holds template instance types created for the module's object types
	asCArray<asCObjectType*>          m_templateInstances; // increases ref count

	// This array holds the global variables declared in the script
	asCSymbolTable<asCGlobalProperty> m_scriptGlobals; // increases ref count
	bool                              m_isGlobalVarInitialized;

	// This array holds class and interface types
	asCArray<asCObjectType*>       m_classTypes; // increases ref count
	// This array holds enum types
	asCArray<asCEnumType*>         m_enumTypes; // increases ref count
	// This array holds typedefs
	asCArray<asCTypedefType*>      m_typeDefs; // increases ref count
	// This array holds the funcdefs declared in the module
	asCArray<asCFuncdefType*>      m_funcDefs; // increases ref count

	// This map contains all the types (also contained in the arrays above) for quick lookup
	// TODO: memory: Can we eliminate the arrays above?
	asCMap<asSNameSpaceNamePair, asCTypeInfo*> m_typeLookup; // doesn't increase ref count

	// This array holds types that have been explicitly declared with 'external'
	asCArray<asCTypeInfo*>       m_externalTypes; // doesn't increase ref count
	// This array holds functions that have been explicitly declared with 'external'
	asCArray<asCScriptFunction*> m_externalFunctions; // doesn't increase ref count
};

END_AS_NAMESPACE

#endif
