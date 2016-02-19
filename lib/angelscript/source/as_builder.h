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
// as_builder.h
//
// This is the class that manages the compilation of the scripts
//


#ifndef AS_BUILDER_H
#define AS_BUILDER_H

#include "as_config.h"
#include "as_symboltable.h"
#include "as_scriptengine.h"
#include "as_module.h"
#include "as_array.h"
#include "as_scriptcode.h"
#include "as_scriptnode.h"
#include "as_datatype.h"
#include "as_property.h"

BEGIN_AS_NAMESPACE

#ifdef AS_NO_COMPILER
// Forward declare the structure, as it is part of some function signatures used even without the compiler
struct sGlobalVariableDescription;
#endif

#ifndef AS_NO_COMPILER

struct sFunctionDescription
{
	asCScriptCode       *script;
	asCScriptNode       *node;
	asCString            name;
	asCObjectType       *objType;
	asCArray<asCString>  paramNames;
	int                  funcId;
	bool                 isExistingShared;
};

struct sGlobalVariableDescription
{
	asCScriptCode     *script;
	asCScriptNode     *declaredAtNode;
	asCScriptNode     *initializationNode;
	asCString          name;
	asCGlobalProperty *property;
	asCDataType        datatype;
	asSNameSpace      *ns;
	int                index;
	bool               isCompiled;
	bool               isPureConstant;
	bool               isEnumValue;
	asQWORD            constantValue;
};

struct sPropertyInitializer
{
	sPropertyInitializer() : declNode(0), initNode(0), file(0) {}
	sPropertyInitializer(const asCString &nm, asCScriptNode *decl, asCScriptNode *init, asCScriptCode *f) : name(nm), declNode(decl), initNode(init), file(f) {}
	sPropertyInitializer &operator=(const sPropertyInitializer &o) {name = o.name; declNode = o.declNode; initNode = o.initNode; file = o.file; return *this;}

	asCString      name;
	asCScriptNode *declNode;
	asCScriptNode *initNode;
	asCScriptCode *file;
};

struct sClassDeclaration
{
	sClassDeclaration() {script = 0; node = 0; validState = 0; objType = 0; isExistingShared = false; isFinal = false;}

	asCScriptCode *script;
	asCScriptNode *node;
	asCString      name;
	int            validState;
	asCObjectType *objType;
	bool           isExistingShared;
	bool           isFinal;

	asCArray<sPropertyInitializer> propInits;
};

struct sFuncDef
{
	asCScriptCode *script;
	asCScriptNode *node;
	asCString      name;
	int            idx;
};

struct sMixinClass
{
	asCScriptCode *script;
	asCScriptNode *node;
	asCString      name;
	asSNameSpace  *ns;
};

#endif // AS_NO_COMPILER

class asCBuilder
{
public:
	asCBuilder(asCScriptEngine *engine, asCModule *module);
	~asCBuilder();

	// These methods are used by the application interface
	int VerifyProperty(asCDataType *dt, const char *decl, asCString &outName, asCDataType &outType, asSNameSpace *ns);
	int ParseDataType(const char *datatype, asCDataType *result, asSNameSpace *implicitNamespace, bool isReturnType = false);
	int ParseTemplateDecl(const char *decl, asCString *name, asCArray<asCString> &subtypeNames);
	int ParseFunctionDeclaration(asCObjectType *type, const char *decl, asCScriptFunction *func, bool isSystemFunction, asCArray<bool> *paramAutoHandles = 0, bool *returnAutoHandle = 0, asSNameSpace *ns = 0, asCScriptNode **listPattern = 0);
	int ParseVariableDeclaration(const char *decl, asSNameSpace *implicitNamespace, asCString &outName, asSNameSpace *&outNamespace, asCDataType &outDt);
	int CheckNameConflict(const char *name, asCScriptNode *node, asCScriptCode *code, asSNameSpace *ns);
	int CheckNameConflictMember(asCObjectType *type, const char *name, asCScriptNode *node, asCScriptCode *code, bool isProperty);

#ifndef AS_NO_COMPILER
	int AddCode(const char *name, const char *code, int codeLength, int lineOffset, int sectionIdx, bool makeCopy);
	int Build();

	int CompileFunction(const char *sectionName, const char *code, int lineOffset, asDWORD compileFlags, asCScriptFunction **outFunc);
	int CompileGlobalVar(const char *sectionName, const char *code, int lineOffset);
#endif

protected:
	friend class asCModule;
	friend class asCParser;
	friend class asCScriptFunction;
	friend class asCScriptEngine;

	void               Reset();

	void               WriteInfo(const asCString &scriptname, const asCString &msg, int r, int c, bool preMessage);
	void               WriteInfo(const asCString &msg, asCScriptCode *file, asCScriptNode *node);
	void               WriteError(const asCString &scriptname, const asCString &msg, int r, int c);
	void               WriteError(const asCString &msg, asCScriptCode *file, asCScriptNode *node);
	void               WriteWarning(const asCString &scriptname, const asCString &msg, int r, int c);
	void               WriteWarning(const asCString &msg, asCScriptCode *file, asCScriptNode *node);

	bool               DoesGlobalPropertyExist(const char *prop, asSNameSpace *ns, asCGlobalProperty **outProp = 0, sGlobalVariableDescription **outDesc = 0, bool *isAppProp = 0);
	asCGlobalProperty *GetGlobalProperty(const char *prop, asSNameSpace *ns, bool *isCompiled, bool *isPureConstant, asQWORD *constantValue, bool *isAppProp);
	int                ValidateDefaultArgs(asCScriptCode *script, asCScriptNode *node, asCScriptFunction *func);
	asCString          GetCleanExpressionString(asCScriptNode *n, asCScriptCode *file);

	asSNameSpace      *GetNameSpaceFromNode(asCScriptNode *node, asCScriptCode *script, asSNameSpace *implicitNs, asCScriptNode **next);
	asSNameSpace      *GetNameSpaceByString(const asCString &nsName, asSNameSpace *implicitNs, asCScriptNode *errNode, asCScriptCode *script, bool isRequired = true);
	asCString          GetScopeFromNode(asCScriptNode *n, asCScriptCode *script, asCScriptNode **next = 0);

	asCObjectType     *GetObjectType(const char *type, asSNameSpace *ns);
	asCScriptFunction *GetFuncDef(const char *type);
	asCObjectType     *GetObjectTypeFromTypesKnownByObject(const char *type, asCObjectType *currentType);
	asCDataType        CreateDataTypeFromNode(asCScriptNode *node, asCScriptCode *file, asSNameSpace *implicitNamespace, bool acceptHandleForScope = false, asCObjectType *currentType = 0);
	asCDataType        ModifyDataTypeFromNode(const asCDataType &type, asCScriptNode *node, asCScriptCode *file, asETypeModifiers *inOutFlag, bool *autoHandle);

	int numErrors;
	int numWarnings;
	bool silent;

	asCScriptEngine *engine;
	asCModule       *module;

#ifndef AS_NO_COMPILER
protected:
	friend class asCCompiler;

	int                CheckForConflictsDueToDefaultArgs(asCScriptCode *script, asCScriptNode *node, asCScriptFunction *func, asCObjectType *objType);
	int                GetNamespaceAndNameFromNode(asCScriptNode *n, asCScriptCode *script, asSNameSpace *implicitNs, asSNameSpace *&outNs, asCString &outName);
	int                RegisterMixinClass(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	sMixinClass       *GetMixinClass(const char *name, asSNameSpace *ns);
	void               IncludePropertiesFromMixins(sClassDeclaration *decl);
	void               IncludeMethodsFromMixins(sClassDeclaration *decl);
	void               AddInterfaceToClass(sClassDeclaration *decl, asCScriptNode *errNode, asCObjectType *intf);
	void               AddInterfaceFromMixinToClass(sClassDeclaration *decl, asCScriptNode *errNode, sMixinClass *mixin);

	int                RegisterScriptFunctionFromNode(asCScriptNode *node, asCScriptCode *file, asCObjectType *object = 0, bool isInterface = false, bool isGlobalFunction = false, asSNameSpace *ns = 0, bool isExistingShared = false, bool isMixin = false);
	int                RegisterScriptFunction(asCScriptNode *node, asCScriptCode *file, asCObjectType *objType, bool isInterface, bool isGlobalFunction, asSNameSpace *ns, bool isExistingShared, bool isMixin, asCString &name, asCDataType &returnType, asCArray<asCString> &parameterNames, asCArray<asCDataType> &parameterTypes, asCArray<asETypeModifiers> &inOutFlags, asCArray<asCString *> &defaultArgs, bool isConstMethod, bool isConstructor, bool isDestructor, bool isPrivate, bool isProtected, bool isOverride, bool isFinal, bool isShared);
	int                RegisterVirtualProperty(asCScriptNode *node, asCScriptCode *file, asCObjectType *object = 0, bool isInterface = false, bool isGlobalFunction = false, asSNameSpace *ns = 0, bool isExistingShared = false);
	int                RegisterImportedFunction(int funcID, asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	int                RegisterGlobalVar(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	int                RegisterClass(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	int                RegisterInterface(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	int                RegisterEnum(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	int                RegisterTypedef(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	int                RegisterFuncDef(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns);
	asCScriptFunction *RegisterLambda(asCScriptNode *node, asCScriptCode *file, asCScriptFunction *funcDef, const asCString &name, asSNameSpace *ns);
	void               CompleteFuncDef(sFuncDef *funcDef);
	void               CompileInterfaces();
	void               CompileClasses(asUINT originalNumTempl);
	void               GetParsedFunctionDetails(asCScriptNode *node, asCScriptCode *file, asCObjectType *objType, asCString &name, asCDataType &returnType, asCArray<asCString> &parameterNames, asCArray<asCDataType> &parameterTypes, asCArray<asETypeModifiers> &inOutFlags, asCArray<asCString *> &defaultArgs, bool &isConstMethod, bool &isConstructor, bool &isDestructor, bool &isPrivate, bool &isProtected, bool &isOverride, bool &isFinal, bool &isShared, asSNameSpace *implicitNamespace);
	bool               DoesMethodExist(asCObjectType *objType, int methodId, asUINT *methodIndex = 0);
	void               AddDefaultConstructor(asCObjectType *objType, asCScriptCode *file);
	asCObjectProperty *AddPropertyToClass(sClassDeclaration *c, const asCString &name, const asCDataType &type, bool isPrivate, bool isProtected, bool isInherited, asCScriptCode *file = 0, asCScriptNode *node = 0);
	int                CreateVirtualFunction(asCScriptFunction *func, int idx);
	void               ParseScripts();
	void               RegisterTypesFromScript(asCScriptNode *node, asCScriptCode *script, asSNameSpace *ns);
	void               RegisterNonTypesFromScript(asCScriptNode *node, asCScriptCode *script, asSNameSpace *ns);
	void               CompileFunctions();
	void               CompileGlobalVariables();
	int                GetEnumValueFromObjectType(asCObjectType *objType, const char *name, asCDataType &outDt, asDWORD &outValue);
	int                GetEnumValue(const char *name, asCDataType &outDt, asDWORD &outValue, asSNameSpace *ns);
	bool               DoesTypeExist(const asCString &type);
	asCObjectProperty *GetObjectProperty(asCDataType &obj, const char *prop);
	asCScriptFunction *GetFunctionDescription(int funcId);
	void               GetFunctionDescriptions(const char *name, asCArray<int> &funcs, asSNameSpace *ns);
	void               GetObjectMethodDescriptions(const char *name, asCObjectType *objectType, asCArray<int> &methods, bool objIsConst, const asCString &scope = "", asCScriptNode *errNode = 0, asCScriptCode *script = 0);
	void               EvaluateTemplateInstances(asUINT startIdx, bool keepSilent);

	asCArray<asCScriptCode *>                  scripts;
	asCArray<sFunctionDescription *>           functions;
	asCSymbolTable<sGlobalVariableDescription> globVariables;
	asCArray<sClassDeclaration *>              classDeclarations;
	asCArray<sClassDeclaration *>              interfaceDeclarations;
	asCArray<sClassDeclaration *>              namedTypeDeclarations;
	asCArray<sFuncDef *>                       funcDefs;
	asCArray<sMixinClass *>                    mixinClasses;

	// For use with the DoesTypeExists() method
	bool                    hasCachedKnownTypes;
	asCMap<asCString, bool> knownTypes;
#endif
};

END_AS_NAMESPACE

#endif
