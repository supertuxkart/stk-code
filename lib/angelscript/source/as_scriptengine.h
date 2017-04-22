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
// as_scriptengine.h
//
// The implementation of the script engine interface
//



#ifndef AS_SCRIPTENGINE_H
#define AS_SCRIPTENGINE_H

#include "as_config.h"
#include "as_atomic.h"
#include "as_scriptfunction.h"
#include "as_array.h"
#include "as_datatype.h"
#include "as_objecttype.h"
#include "as_module.h"
#include "as_callfunc.h"
#include "as_configgroup.h"
#include "as_memory.h"
#include "as_gc.h"
#include "as_tokenizer.h"

BEGIN_AS_NAMESPACE

class asCBuilder;
class asCContext;

// TODO: import: Remove this when import is removed
struct sBindInfo;

class asCScriptEngine : public asIScriptEngine
{
//=============================================================
// From asIScriptEngine
//=============================================================
public:
	// Memory management
	virtual int AddRef() const;
	virtual int Release() const;
	virtual int ShutDownAndRelease();

	// Engine properties
	virtual int     SetEngineProperty(asEEngineProp property, asPWORD value);
	virtual asPWORD GetEngineProperty(asEEngineProp property) const;

	// Compiler messages
	virtual int SetMessageCallback(const asSFuncPtr &callback, void *obj, asDWORD callConv);
	virtual int ClearMessageCallback();
	virtual int WriteMessage(const char *section, int row, int col, asEMsgType type, const char *message);

	// JIT Compiler
	virtual int SetJITCompiler(asIJITCompiler *compiler);
	virtual asIJITCompiler *GetJITCompiler() const;

	// Global functions
	virtual int                RegisterGlobalFunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall = 0);
	virtual asUINT             GetGlobalFunctionCount() const;
	virtual asIScriptFunction *GetGlobalFunctionByIndex(asUINT index) const;
	virtual asIScriptFunction *GetGlobalFunctionByDecl(const char *declaration) const;

	// Global properties
	virtual int    RegisterGlobalProperty(const char *declaration, void *pointer);
	virtual asUINT GetGlobalPropertyCount() const;
	virtual int    GetGlobalPropertyByIndex(asUINT index, const char **name, const char **nameSpace = 0, int *typeId = 0, bool *isConst = 0, const char **configGroup = 0, void **pointer = 0, asDWORD *accessMask = 0) const;
	virtual int    GetGlobalPropertyIndexByName(const char *name) const;
	virtual int    GetGlobalPropertyIndexByDecl(const char *decl) const;

	// Type registration
	virtual int            RegisterObjectType(const char *obj, int byteSize, asDWORD flags);
	virtual int            RegisterObjectProperty(const char *obj, const char *declaration, int byteOffset);
	virtual int            RegisterObjectMethod(const char *obj, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall = 0);
	virtual int            RegisterObjectBehaviour(const char *obj, asEBehaviours behaviour, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall = 0);
	virtual int            RegisterInterface(const char *name);
	virtual int            RegisterInterfaceMethod(const char *intf, const char *declaration);
	virtual asUINT         GetObjectTypeCount() const;
	virtual asIObjectType *GetObjectTypeByIndex(asUINT index) const;
	virtual asIObjectType *GetObjectTypeByName(const char *name) const;
	virtual asIObjectType *GetObjectTypeByDecl(const char *decl) const;

	// String factory
	virtual int RegisterStringFactory(const char *datatype, const asSFuncPtr &factoryFunc, asDWORD callConv, void *objForThiscall = 0);
	virtual int GetStringFactoryReturnTypeId(asDWORD *flags) const;

	// Default array type
	virtual int RegisterDefaultArrayType(const char *type);
	virtual int GetDefaultArrayTypeId() const;

	// Enums
	virtual int         RegisterEnum(const char *type);
	virtual int         RegisterEnumValue(const char *type, const char *name, int value);
	virtual asUINT      GetEnumCount() const;
	virtual const char *GetEnumByIndex(asUINT index, int *enumTypeId, const char **nameSpace, const char **configGroup = 0, asDWORD *accessMask = 0) const;
	virtual int         GetEnumValueCount(int enumTypeId) const;
	virtual const char *GetEnumValueByIndex(int enumTypeId, asUINT index, int *outValue) const;

	// Funcdefs
	virtual int                RegisterFuncdef(const char *decl);
	virtual asUINT             GetFuncdefCount() const;
	virtual asIScriptFunction *GetFuncdefByIndex(asUINT index) const;

	// Typedefs
	// TODO: interface: Should perhaps rename this to Alias, since it doesn't really create a new type
	virtual int         RegisterTypedef(const char *type, const char *decl);
	virtual asUINT      GetTypedefCount() const;
	virtual const char *GetTypedefByIndex(asUINT index, int *typeId, const char **nameSpace, const char **configGroup = 0, asDWORD *accessMask = 0) const;

	// Configuration groups
	virtual int         BeginConfigGroup(const char *groupName);
	virtual int         EndConfigGroup();
	virtual int         RemoveConfigGroup(const char *groupName);
	virtual asDWORD     SetDefaultAccessMask(asDWORD defaultMask);
	virtual int         SetDefaultNamespace(const char *nameSpace);
	virtual const char *GetDefaultNamespace() const;

	// Script modules
	virtual asIScriptModule *GetModule(const char *module, asEGMFlags flag);
	virtual int              DiscardModule(const char *module);
	virtual asUINT           GetModuleCount() const;
	virtual asIScriptModule *GetModuleByIndex(asUINT index) const;

	// Script functions
	virtual asIScriptFunction *GetFunctionById(int funcId) const;
	virtual asIScriptFunction *GetFuncDefFromTypeId(int typeId) const;

	// Type identification
	virtual asIObjectType *GetObjectTypeById(int typeId) const;
	virtual int            GetTypeIdByDecl(const char *decl) const;
	virtual const char    *GetTypeDeclaration(int typeId, bool includeNamespace = false) const;
	virtual int            GetSizeOfPrimitiveType(int typeId) const;

	// Script execution
	virtual asIScriptContext      *CreateContext();
	virtual void                  *CreateScriptObject(const asIObjectType *type);
	virtual void                  *CreateScriptObjectCopy(void *obj, const asIObjectType *type);
	virtual void                  *CreateUninitializedScriptObject(const asIObjectType *type);
	virtual asIScriptFunction     *CreateDelegate(asIScriptFunction *func, void *obj);
	virtual int                    AssignScriptObject(void *dstObj, void *srcObj, const asIObjectType *type);
	virtual void                   ReleaseScriptObject(void *obj, const asIObjectType *type);
	virtual void                   AddRefScriptObject(void *obj, const asIObjectType *type);
	virtual int                    RefCastObject(void *obj, asIObjectType *fromType, asIObjectType *toType, void **newPtr, bool useOnlyImplicitCast = false);
#ifdef AS_DEPRECATED
	// Deprecated since 2.30.0, 2014-11-04
	virtual bool                   IsHandleCompatibleWithObject(void *obj, int objTypeId, int handleTypeId) const;
#endif
	virtual asILockableSharedBool *GetWeakRefFlagOfScriptObject(void *obj, const asIObjectType *type) const;

	// Context pooling
	virtual asIScriptContext *RequestContext();
	virtual void              ReturnContext(asIScriptContext *ctx);
	virtual int               SetContextCallbacks(asREQUESTCONTEXTFUNC_t requestCtx, asRETURNCONTEXTFUNC_t returnCtx, void *param = 0);

	// String interpretation
	virtual asETokenClass ParseToken(const char *string, size_t stringLength = 0, asUINT *tokenLength = 0) const;

	// Garbage collection
	virtual int  GarbageCollect(asDWORD flags = asGC_FULL_CYCLE, asUINT numIterations = 1);
	virtual void GetGCStatistics(asUINT *currentSize, asUINT *totalDestroyed, asUINT *totalDetected, asUINT *newObjects, asUINT *totalNewDestroyed) const;
	virtual int  NotifyGarbageCollectorOfNewObject(void *obj, asIObjectType *type);
	virtual int  GetObjectInGC(asUINT idx, asUINT *seqNbr, void **obj = 0, asIObjectType **type = 0);
	virtual void GCEnumCallback(void *reference);

	// User data
	virtual void *SetUserData(void *data, asPWORD type);
	virtual void *GetUserData(asPWORD type) const;
	virtual void  SetEngineUserDataCleanupCallback(asCLEANENGINEFUNC_t callback, asPWORD type);
	virtual void  SetModuleUserDataCleanupCallback(asCLEANMODULEFUNC_t callback, asPWORD type);
	virtual void  SetContextUserDataCleanupCallback(asCLEANCONTEXTFUNC_t callback, asPWORD type);
	virtual void  SetFunctionUserDataCleanupCallback(asCLEANFUNCTIONFUNC_t callback, asPWORD type);
	virtual void  SetObjectTypeUserDataCleanupCallback(asCLEANOBJECTTYPEFUNC_t callback, asPWORD type);
	virtual void  SetScriptObjectUserDataCleanupCallback(asCLEANSCRIPTOBJECTFUNC_t callback, asPWORD type);

//===========================================================
// internal methods
//===========================================================
public:
	asCScriptEngine();
	virtual ~asCScriptEngine();

//protected:
	friend class asCBuilder;
	friend class asCCompiler;
	friend class asCContext;
	friend class asCDataType;
	friend class asCModule;
	friend class asCRestore;
	friend class asCByteCode;
	friend int PrepareSystemFunction(asCScriptFunction *func, asSSystemFunctionInterface *internal, asCScriptEngine *engine);

	int RegisterMethodToObjectType(asCObjectType *objectType, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall = 0);
	int RegisterBehaviourToObjectType(asCObjectType *objectType, asEBehaviours behaviour, const char *decl, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall);

	int VerifyVarTypeNotInFunction(asCScriptFunction *func);

	void *CallAlloc(const asCObjectType *objType) const;
	void  CallFree(void *obj) const;

	void *CallGlobalFunctionRetPtr(int func) const;
	void *CallGlobalFunctionRetPtr(int func, void *param1) const;
	void *CallGlobalFunctionRetPtr(asSSystemFunctionInterface *func, asCScriptFunction *desc) const;
	void *CallGlobalFunctionRetPtr(asSSystemFunctionInterface *i, asCScriptFunction *s, void *param1) const;
	void  CallObjectMethod(void *obj, int func) const;
	void  CallObjectMethod(void *obj, void *param, int func) const;
	void  CallObjectMethod(void *obj, asSSystemFunctionInterface *func, asCScriptFunction *desc) const;
	void  CallObjectMethod(void *obj, void *param, asSSystemFunctionInterface *func, asCScriptFunction *desc) const;
	bool  CallObjectMethodRetBool(void *obj, int func) const;
	int   CallObjectMethodRetInt(void *obj, int func) const;
	void *CallObjectMethodRetPtr(void *obj, int func) const;
	void *CallObjectMethodRetPtr(void *obj, int param1, asCScriptFunction *func) const;
	void  CallGlobalFunction(void *param1, void *param2, asSSystemFunctionInterface *func, asCScriptFunction *desc) const;
	bool  CallGlobalFunctionRetBool(void *param1, void *param2, asSSystemFunctionInterface *func, asCScriptFunction *desc) const;

	void ConstructScriptObjectCopy(void *mem, void *obj, asCObjectType *type);

	void DeleteDiscardedModules();

	void RemoveTemplateInstanceType(asCObjectType *t);
	void RemoveTypeAndRelatedFromList(asCMap<asCObjectType*,char> &types, asCObjectType *ot);

	asCConfigGroup *FindConfigGroupForFunction(int funcId) const;
	asCConfigGroup *FindConfigGroupForGlobalVar(int gvarId) const;
	asCConfigGroup *FindConfigGroupForObjectType(const asCObjectType *type) const;
	asCConfigGroup *FindConfigGroupForFuncDef(const asCScriptFunction *funcDef) const;

	int  RequestBuild();
	void BuildCompleted();

	void PrepareEngine();
	bool isPrepared;

	int CreateContext(asIScriptContext **context, bool isInternal);

	asCObjectType *GetRegisteredObjectType(const asCString &name, asSNameSpace *ns) const;

	asCObjectType *GetListPatternType(int listPatternFuncId);
	void DestroyList(asBYTE *buffer, const asCObjectType *listPatternType);
	void DestroySubList(asBYTE *&buffer, asSListPatternNode *&patternNode);

	int AddBehaviourFunction(asCScriptFunction &func, asSSystemFunctionInterface &internal);

	asCString GetFunctionDeclaration(int funcId);

	asCScriptFunction *GetScriptFunction(int funcId) const;

	asCModule *GetModule(const char *name, bool create);
	asCModule *GetModuleFromFuncId(int funcId);

	int  GetMethodIdByDecl(const asCObjectType *ot, const char *decl, asCModule *mod);
	int  GetFactoryIdByDecl(const asCObjectType *ot, const char *decl);

	int  GetNextScriptFunctionId();
	void AddScriptFunction(asCScriptFunction *func);
	void RemoveScriptFunction(asCScriptFunction *func);
	void RemoveFuncdef(asCScriptFunction *func);

	int ConfigError(int err, const char *funcName, const char *arg1, const char *arg2);

	int                GetTypeIdFromDataType(const asCDataType &dt) const;
	asCDataType        GetDataTypeFromTypeId(int typeId) const;
	asCObjectType     *GetObjectTypeFromTypeId(int typeId) const;
	void               RemoveFromTypeIdMap(asCObjectType *type);

	bool               IsTemplateType(const char *name) const;
	asCObjectType     *GetTemplateInstanceType(asCObjectType *templateType, asCArray<asCDataType> &subTypes, asCModule *requestingModule);
	asCScriptFunction *GenerateTemplateFactoryStub(asCObjectType *templateType, asCObjectType *templateInstanceType, int origFactoryId);
	bool               GenerateNewTemplateFunction(asCObjectType *templateType, asCObjectType *templateInstanceType, asCScriptFunction *templateFunc, asCScriptFunction **newFunc);
	asCDataType        DetermineTypeForTemplate(const asCDataType &orig, asCObjectType *tmpl, asCObjectType *ot);
	bool               RequireTypeReplacement(asCDataType &type, asCObjectType *templateType);

	asCModule         *FindNewOwnerForSharedType(asCObjectType *type, asCModule *mod);
	asCModule         *FindNewOwnerForSharedFunc(asCScriptFunction *func, asCModule *mod);

	// String constants
	// TODO: Must free unused string constants, thus the ref count for each must be tracked
	int              AddConstantString(const char *str, size_t length);
	const asCString &GetConstantString(int id);

	// Global property management
	asCGlobalProperty *AllocateGlobalProperty();
	void RemoveGlobalProperty(asCGlobalProperty *prop);

	int GetScriptSectionNameIndex(const char *name);

	// Namespace management
	asSNameSpace *AddNameSpace(const char *name);
	asSNameSpace *FindNameSpace(const char *name) const;
	asSNameSpace *GetParentNameSpace(asSNameSpace *ns) const;

//===========================================================
// internal properties
//===========================================================
	asCMemoryMgr memoryMgr;

	asUINT initialContextStackSize;

	asCObjectType   *defaultArrayObjectType;
	asCObjectType    scriptTypeBehaviours;
	asCObjectType    functionBehaviours;

	// Registered interface
	asCArray<asCObjectType *>         registeredObjTypes;
	asCArray<asCObjectType *>         registeredTypeDefs;
	asCArray<asCObjectType *>         registeredEnums;
	asCSymbolTable<asCGlobalProperty> registeredGlobalProps; // increases ref count // TODO: memory savings: Since there can be only one property with the same name a simpler symbol table should be used
	asCSymbolTable<asCScriptFunction> registeredGlobalFuncs;
	asCArray<asCScriptFunction *>     registeredFuncDefs;
	asCArray<asCObjectType *>         registeredTemplateTypes;
	asCScriptFunction                *stringFactory;
	bool configFailed;

	// Stores all registered types except funcdefs
	asCMap<asSNameSpaceNamePair, asCObjectType*> allRegisteredTypes; // increases ref count

	// Dummy types used to name the subtypes in the template objects 
	asCArray<asCObjectType *>      templateSubTypes;

	// Store information about template types
	// This list will contain all instances of templates, both registered specialized 
	// types and those automacially instantiated from scripts
	asCArray<asCObjectType *>      templateInstanceTypes; // increases ref count

	// Store information about list patterns
	asCArray<asCObjectType *>      listPatternTypes; // increases ref count

	// Stores all global properties, both those registered by application, and those declared by scripts.
	// The id of a global property is the index in this array.
	asCArray<asCGlobalProperty *> globalProperties; // increases ref count
	asCArray<int>                 freeGlobalPropertyIds;

	// This map is used to quickly find a property by its memory address
	// It is used principally during building, cleanup, and garbage detection for script functions
	asCMap<void*, asCGlobalProperty*> varAddressMap; // doesn't increase ref count

	// Stores all functions, i.e. registered functions, script functions, class methods, behaviours, etc.
	asCArray<asCScriptFunction *> scriptFunctions;       // doesn't increase ref count
	asCArray<int>                 freeScriptFunctionIds;
	asCArray<asCScriptFunction *> signatureIds;

	// An array with all module imported functions
	asCArray<sBindInfo *>  importedFunctions; // doesn't increase ref count
	asCArray<int>          freeImportedFunctionIdxs;

	// Synchronized
	mutable asCAtomic      refCount;
	// Synchronized with engineRWLock
	// This array holds all live script modules
	asCArray<asCModule *>  scriptModules;
	// Synchronized with engineRWLock
	// This is a pointer to the last module that was requested. It is used for performance 
	// improvement, since it is common that the same module is accessed many times in a row
	asCModule             *lastModule;
	// Synchronized with engineRWLock
	// This flag is true if a script is currently being compiled. It is used to prevent multiple
	// threads from requesting builds at the same time (without blocking)
	bool                   isBuilding;
	// Synchronized with engineRWLock
	// This array holds modules that have been discard (thus are no longer visible to the application)
	// but cannot yet be deleted due to having external references to some of the entities in them
	asCArray<asCModule *>  discardedModules;
	// This flag is set to true during compilations of scripts (or loading pre-compiled scripts) 
	// to delay the validation of template types until the subtypes have been fully declared 
	bool                   deferValidationOfTemplateTypes;

	// Tokenizer is instantiated once to share resources
	asCTokenizer tok;

	// Stores shared script declared types (classes, interfaces, enums)
	asCArray<asCObjectType *> sharedScriptTypes; // increases ref count
	// This array stores the template instances types that have been automatically generated from template types
	asCArray<asCObjectType *> generatedTemplateTypes;
	// Stores the funcdefs
	// TODO: redesign: Only shared funcdefs should be stored here
	//                 a funcdef becomes shared if all arguments and the return type are shared (or application registered)
	asCArray<asCScriptFunction *> funcDefs; // doesn't increase ref count

	// Stores the names of the script sections for debugging purposes
	asCArray<asCString *> scriptSectionNames;

	// Type identifiers
	mutable int                             typeIdSeqNbr;
	mutable asCMap<int, asCObjectType*>     mapTypeIdToObjectType;
	mutable asCMap<int, asCScriptFunction*> mapTypeIdToFunction;

	// Garbage collector
	asCGarbageCollector gc;

	// Dynamic groups
	asCConfigGroup             defaultGroup;
	asCArray<asCConfigGroup*>  configGroups;
	asCConfigGroup            *currentGroup;
	asDWORD                    defaultAccessMask;
	asSNameSpace              *defaultNamespace;

	// Message callback
	bool                        msgCallback;
	asSSystemFunctionInterface  msgCallbackFunc;
	void                       *msgCallbackObj;
	struct preMessage_t
	{
		preMessage_t() { isSet = false; }
		bool      isSet;
		asCString message;
		asCString scriptname;
		int       r;
		int       c;
	} preMessage;

	// JIt compilation
	asIJITCompiler             *jitCompiler;

	// Namespaces
	// These are shared between all entities and are 
	// only deleted once the engine is destroyed
	asCArray<asSNameSpace*> nameSpaces;

	// String constants
	// These are shared between all scripts and are
	// only deleted once the engine is destroyed
	asCArray<asCString*>          stringConstants;
	asCMap<asCStringPointer, int> stringToIdMap;

	// Callbacks for context pooling
	asREQUESTCONTEXTFUNC_t  requestCtxFunc;
	asRETURNCONTEXTFUNC_t   returnCtxFunc;
	void                   *ctxCallbackParam;

	// User data
	asCArray<asPWORD>       userData;

	struct SEngineClean    { asPWORD type; asCLEANENGINEFUNC_t      cleanFunc; };
	asCArray<SEngineClean>    cleanEngineFuncs;
	struct SModuleClean    { asPWORD type; asCLEANMODULEFUNC_t      cleanFunc; };
	asCArray<SModuleClean>    cleanModuleFuncs;
	struct SContextClean   { asPWORD type; asCLEANCONTEXTFUNC_t     cleanFunc; };
	asCArray<SContextClean>   cleanContextFuncs;
	struct SFunctionClean  { asPWORD type; asCLEANFUNCTIONFUNC_t    cleanFunc; };
	asCArray<SFunctionClean>  cleanFunctionFuncs;
	struct SObjTypeClean   { asPWORD type; asCLEANOBJECTTYPEFUNC_t  cleanFunc; };
	asCArray<SObjTypeClean>   cleanObjectTypeFuncs;
	struct SScriptObjClean { asPWORD type; asCLEANSCRIPTOBJECTFUNC_t cleanFunc; };
	asCArray<SScriptObjClean> cleanScriptObjectFuncs;

	// Synchronization for threads
	DECLAREREADWRITELOCK(mutable engineRWLock)

	// Engine properties
	struct
	{
		bool   allowUnsafeReferences;
		bool   optimizeByteCode;
		bool   copyScriptSections;
		asUINT maximumContextStackSize;
		bool   useCharacterLiterals;
		bool   allowMultilineStrings;
		bool   allowImplicitHandleTypes;
		bool   buildWithoutLineCues;
		bool   initGlobalVarsAfterBuild;
		bool   requireEnumScope;
		int    scanner;
		bool   includeJitInstructions;
		int    stringEncoding;
		int    propertyAccessorMode;
		bool   expandDefaultArrayToTemplate;
		bool   autoGarbageCollect;
		bool   disallowGlobalVars;
		bool   alwaysImplDefaultConstruct;
		int    compilerWarnings;
		bool   disallowValueAssignForRefType;
		// TODO: 3.0.0: Remove the alterSyntaxNamedArgs
		int    alterSyntaxNamedArgs;
		bool   disableIntegerDivision;
		bool   disallowEmptyListElements;
		// TODO: 3.0.0: Remove the privatePropAsProtected
		bool   privatePropAsProtected;
	} ep;

	// This flag is to allow a quicker shutdown when releasing the engine
	bool shuttingDown;

	// This flag is set when the engine's destructor is called, this is to 
	// avoid recursive calls if an object happens to increment/decrement
	// the ref counter during shutdown
	bool inDestructor;
};

END_AS_NAMESPACE

#endif
