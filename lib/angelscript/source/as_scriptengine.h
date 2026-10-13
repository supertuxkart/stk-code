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
#include "as_map.h"

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
	virtual int GetMessageCallback(asSFuncPtr* callback, void** obj, asDWORD* callConv);
	virtual int ClearMessageCallback();
	virtual int WriteMessage(const char *section, int row, int col, asEMsgType type, const char *message);

	// JIT Compiler
	virtual int                     SetJITCompiler(asIJITCompilerAbstract *compiler);
	virtual asIJITCompilerAbstract *GetJITCompiler() const;

	// Global functions
	virtual int                RegisterGlobalFunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0);
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
	virtual int            RegisterObjectType(const char *obj, int byteSize, asQWORD flags);
	virtual int            RegisterObjectProperty(const char *obj, const char *declaration, int byteOffset, int compositeOffset = 0, bool isCompositeIndirect = false);
	virtual int            RegisterObjectMethod(const char *obj, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false);
	virtual int            RegisterObjectBehaviour(const char *obj, asEBehaviours behaviour, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false);
	virtual int            RegisterInterface(const char *name);
	virtual int            RegisterInterfaceMethod(const char *intf, const char *declaration);
	virtual asUINT         GetObjectTypeCount() const;
	virtual asITypeInfo   *GetObjectTypeByIndex(asUINT index) const;

	// String factory
	virtual int RegisterStringFactory(const char *datatype, asIStringFactory *factory);
	virtual int GetStringFactory(asDWORD* typeModifiers, asIStringFactory** factory) const;
#ifdef AS_DEPRECATED
	// deprecated since 2024-07-27, 2.38.0
	virtual int GetStringFactoryReturnTypeId(asDWORD *flags) const;
#endif

	// Default array type
	virtual int RegisterDefaultArrayType(const char *type);
	virtual int GetDefaultArrayTypeId() const;

	// Enums
	virtual int          RegisterEnum(const char *type);
	virtual int          RegisterEnumValue(const char *type, const char *name, int value);
	virtual asUINT       GetEnumCount() const;
	virtual asITypeInfo *GetEnumByIndex(asUINT index) const;

	// Funcdefs
	virtual int          RegisterFuncdef(const char *decl);
	virtual asUINT       GetFuncdefCount() const;
	virtual asITypeInfo *GetFuncdefByIndex(asUINT index) const;

	// Typedefs
	// TODO: interface: Should perhaps rename this to Alias, since it doesn't really create a new type
	virtual int          RegisterTypedef(const char *type, const char *decl);
	virtual asUINT       GetTypedefCount() const;
	virtual asITypeInfo *GetTypedefByIndex(asUINT index) const;

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
	virtual int                GetLastFunctionId() const;
	virtual asIScriptFunction *GetFunctionById(int funcId) const;

	// Type identification
	virtual int            GetTypeIdByDecl(const char *decl) const;
	virtual const char    *GetTypeDeclaration(int typeId, bool includeNamespace = false) const;
	virtual int            GetSizeOfPrimitiveType(int typeId) const;
	virtual asITypeInfo   *GetTypeInfoById(int typeId) const;
	virtual asITypeInfo   *GetTypeInfoByName(const char *name) const;
	virtual asITypeInfo   *GetTypeInfoByDecl(const char *decl) const;

	// Script execution
	virtual asIScriptContext      *CreateContext();
	virtual void                  *CreateScriptObject(const asITypeInfo *type);
	virtual void                  *CreateScriptObjectCopy(void *obj, const asITypeInfo *type);
	virtual void                  *CreateUninitializedScriptObject(const asITypeInfo *type);
	virtual asIScriptFunction     *CreateDelegate(asIScriptFunction *func, void *obj);
	virtual int                    AssignScriptObject(void *dstObj, void *srcObj, const asITypeInfo *type);
	virtual void                   ReleaseScriptObject(void *obj, const asITypeInfo *type);
	virtual void                   AddRefScriptObject(void *obj, const asITypeInfo *type);
	virtual int                    RefCastObject(void *obj, asITypeInfo *fromType, asITypeInfo *toType, void **newPtr, bool useOnlyImplicitCast = false);
	virtual asILockableSharedBool *GetWeakRefFlagOfScriptObject(void *obj, const asITypeInfo *type) const;

	// Context pooling
	virtual asIScriptContext *RequestContext();
	virtual void              ReturnContext(asIScriptContext *ctx);
	virtual int               SetContextCallbacks(asREQUESTCONTEXTFUNC_t requestCtx, asRETURNCONTEXTFUNC_t returnCtx, void *param = 0);

	// String interpretation
	virtual asETokenClass ParseToken(const char *string, size_t stringLength = 0, asUINT *tokenLength = 0) const;

	// Garbage collection
	virtual int  GarbageCollect(asDWORD flags = asGC_FULL_CYCLE, asUINT numIterations = 1);
	virtual void GetGCStatistics(asUINT *currentSize, asUINT *totalDestroyed, asUINT *totalDetected, asUINT *newObjects, asUINT *totalNewDestroyed) const;
	virtual int  NotifyGarbageCollectorOfNewObject(void *obj, asITypeInfo *type);
	virtual int  GetObjectInGC(asUINT idx, asUINT *seqNbr, void **obj = 0, asITypeInfo **type = 0);
	virtual void GCEnumCallback(void *reference);
	virtual void ForwardGCEnumReferences(void *ref, asITypeInfo *type);
	virtual void ForwardGCReleaseReferences(void *ref, asITypeInfo *type);
	virtual void SetCircularRefDetectedCallback(asCIRCULARREFFUNC_t callback, void *param = 0);

	// User data
	virtual void *SetUserData(void *data, asPWORD type);
	virtual void *GetUserData(asPWORD type) const;
	virtual void  SetEngineUserDataCleanupCallback(asCLEANENGINEFUNC_t callback, asPWORD type);
	virtual void  SetModuleUserDataCleanupCallback(asCLEANMODULEFUNC_t callback, asPWORD type);
	virtual void  SetContextUserDataCleanupCallback(asCLEANCONTEXTFUNC_t callback, asPWORD type);
	virtual void  SetFunctionUserDataCleanupCallback(asCLEANFUNCTIONFUNC_t callback, asPWORD type);
	virtual void  SetTypeInfoUserDataCleanupCallback(asCLEANTYPEINFOFUNC_t callback, asPWORD type);
	virtual void  SetScriptObjectUserDataCleanupCallback(asCLEANSCRIPTOBJECTFUNC_t callback, asPWORD type);

	// Exception handling
	virtual int SetTranslateAppExceptionCallback(const asSFuncPtr &callback, void *param, int callConv);

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

	int RegisterMethodToObjectType(asCObjectType *objectType, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false);
	int RegisterBehaviourToObjectType(asCObjectType *objectType, asEBehaviours behaviour, const char *decl, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary = 0, int compositeOffset = 0, bool isCompositeIndirect = false);

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
	int   CallScriptObjectMethod(void *obj, int func);

	void ConstructScriptObjectCopy(void *mem, void *obj, asCObjectType *type);

	void DeleteDiscardedModules();

	void RemoveTemplateInstanceType(asCObjectType *t);

	asCConfigGroup *FindConfigGroupForFunction(int funcId) const;
	asCConfigGroup *FindConfigGroupForGlobalVar(int gvarId) const;
	asCConfigGroup *FindConfigGroupForTypeInfo(const asCTypeInfo *type) const;
	asCConfigGroup *FindConfigGroupForFuncDef(const asCFuncdefType *funcDef) const;

	int  RequestBuild();
	void BuildCompleted();

	void PrepareEngine();
	bool isPrepared;

	int CreateContext(asIScriptContext **context, bool isInternal);

	asCTypeInfo *GetRegisteredType(const asCString &name, asSNameSpace *ns) const;

	asCObjectType *GetListPatternType(int listPatternFuncId);
	void DestroyList(asBYTE *buffer, const asCObjectType *listPatternType);
	void DestroySubList(asBYTE *&buffer, asSListPatternNode *&patternNode);

	int AddBehaviourFunction(asCScriptFunction &func, asSSystemFunctionInterface &internal);

	int ParseNamespace(const char* ns, asCArray<asCString>& nsStrings) const;
	asCString GetFunctionDeclaration(int funcId);

	asCScriptFunction *GetScriptFunction(int funcId) const;

	asCModule *GetModule(const char *name, bool create);
	asCModule *GetModuleFromFuncId(int funcId);

	int  GetMethodIdByDecl(const asCObjectType *ot, const char *decl, asCModule *mod);
	int  GetFactoryIdByDecl(const asCObjectType *ot, const char *decl);

	int  GetNextScriptFunctionId();
	void AddScriptFunction(asCScriptFunction *func);
	void RemoveScriptFunction(asCScriptFunction *func);
	void RemoveFuncdef(asCFuncdefType *func);

	int ConfigError(int err, const char *funcName, const char *arg1, const char *arg2);

	int                GetTypeIdFromDataType(const asCDataType &dt) const;
	asCDataType        GetDataTypeFromTypeId(int typeId) const;
	asCObjectType     *GetObjectTypeFromTypeId(int typeId) const;
	void               RemoveFromTypeIdMap(asCTypeInfo *type);

	bool               IsTemplateType(const char *name) const;
	bool               IsTemplateFn(const char *name) const;
	int                GetTemplateFunctionInstance(asCScriptFunction* templateFunction, const asCArray<asCDataType>& subTypes);
	int                SetTemplateRestrictions(asCObjectType *templateType, asCScriptFunction *func, const char *caller, const char *decl);
	asCObjectType     *GetTemplateInstanceType(asCObjectType *templateType, asCArray<asCDataType> &subTypes, asCModule *requestingModule);
	asCScriptFunction *GenerateFactoryStubForTemplateObjectInstance(asCObjectType *templateType, asCObjectType *templateInstanceType, int origFactoryId);
	bool               GenerateFunctionForTemplateObjectInstance(asCObjectType *templateType, asCObjectType *templateInstanceType, asCScriptFunction *templateFunc, asCScriptFunction **newFunc);
	asCFuncdefType    *GenerateFuncdefForTemplateObjectInstance(asCObjectType *templateType, asCObjectType *templateInstanceType, asCFuncdefType *templateFuncdef);
	asCDataType        DetermineTypeForTemplate(const asCDataType &orig, const asCArray<asCDataType>& templateSubTypes, const asCArray<asCDataType>& targetSubTypes, asCModule *mod, asCObjectType* tmpl, asCObjectType* ot);

	asCModule         *FindNewOwnerForSharedType(asCTypeInfo *type, asCModule *mod);
	asCModule         *FindNewOwnerForSharedFunc(asCScriptFunction *func, asCModule *mod);

	asCFuncdefType    *FindMatchingFuncdef(asCScriptFunction *func, asCModule *mod);

	int                DetermineNameAndNamespace(const char *in_name, asSNameSpace *implicitNs, asCString &out_name, asSNameSpace *&out_ns) const;
	asCTypeInfo       *GetTemplateSubTypeByName(const asCString &name);
	
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

	asCObjectType   *defaultArrayObjectType;
	asCObjectType    scriptTypeBehaviours;
	asCObjectType    functionBehaviours;

	// Registered interface
	asCArray<asCObjectType *>         registeredObjTypes;      // doesn't increase ref count
	asCArray<asCTypedefType *>        registeredTypeDefs;      // doesn't increase ref count
	asCArray<asCEnumType *>           registeredEnums;         // doesn't increase ref count
	// TODO: memory savings: Since there can be only one property with the same name a simpler symbol table should be used for global props
	asCSymbolTable<asCGlobalProperty> registeredGlobalProps;   // increases ref count
	asCSymbolTable<asCScriptFunction> registeredGlobalFuncs;
	// The template global function instances will be stored in this array
	asCArray<asCScriptFunction*>      generatedTemplateFunctionInstances;  // increases ref count
	// This array contains a list of registered template global functions
	asCArray<asCScriptFunction*>      registeredTemplateGlobalFuncs;
	asCArray<asCFuncdefType *>        registeredFuncDefs;      // doesn't increase ref count
	asCArray<asCObjectType *>         registeredTemplateTypes; // doesn't increase ref count
	asIStringFactory                 *stringFactory;
	asCDataType                       stringType;
	bool configFailed;

	// Stores all registered types
	asCMap<asSNameSpaceNamePair, asCTypeInfo*> allRegisteredTypes; // increases ref count

	// Dummy types used to name the subtypes in the template objects
	asCArray<asCTypeInfo *>        registeredTemplateSubTypes;

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
	asCArray<asCTypeInfo *> sharedScriptTypes; // increases ref count
	// This array stores the template instances types that have been automatically generated from template types
	asCArray<asCObjectType *> generatedTemplateTypes;
	// Stores the funcdefs
	// TODO: redesign: Only shared funcdefs should be stored here
	//                 a funcdef becomes shared if all arguments and the return type are shared (or application registered)
	asCArray<asCFuncdefType *> funcDefs; // doesn't increases ref count

	// Stores the names of the script sections for debugging purposes
	asCArray<asCString *> scriptSectionNames;

	// Type identifiers
	mutable int                             typeIdSeqNbr;
	mutable asCMap<int, asCTypeInfo*>       mapTypeIdToTypeInfo;

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
	asSFuncPtr                  msgCallbackOriginalFuncPtr;
	asDWORD                     msgCallbackOriginalCallConv;
	void                       *msgCallbackObj;
	struct preMessage_t
	{
		preMessage_t() { isSet = false; r = c = 0; }
		bool      isSet;
		asCString message;
		asCString scriptname;
		int       r;
		int       c;
	} preMessage;

	// JIt compilation
	asIJITCompilerAbstract *jitCompiler;

	// Namespaces
	// These are shared between all entities and are
	// only deleted once the engine is destroyed
	asCArray<asSNameSpace*> nameSpaces;

	// Callbacks for context pooling
	asREQUESTCONTEXTFUNC_t  requestCtxFunc;
	asRETURNCONTEXTFUNC_t   returnCtxFunc;
	void                   *ctxCallbackParam;

	// User data
	asCArray<asPWORD>       userData;

	struct SEngineClean    { asPWORD type; asCLEANENGINEFUNC_t       cleanFunc; };
	asCArray<SEngineClean>    cleanEngineFuncs;
	struct SModuleClean    { asPWORD type; asCLEANMODULEFUNC_t       cleanFunc; };
	asCArray<SModuleClean>    cleanModuleFuncs;
	struct SContextClean   { asPWORD type; asCLEANCONTEXTFUNC_t      cleanFunc; };
	asCArray<SContextClean>   cleanContextFuncs;
	struct SFunctionClean  { asPWORD type; asCLEANFUNCTIONFUNC_t     cleanFunc; };
	asCArray<SFunctionClean>  cleanFunctionFuncs;
	struct STypeInfoClean  { asPWORD type; asCLEANTYPEINFOFUNC_t     cleanFunc; };
	asCArray<STypeInfoClean>  cleanTypeInfoFuncs;
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
		asUINT initContextStackSize;
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
		asUINT alwaysImplDefaultConstruct;
		int    compilerWarnings;
		bool   disallowValueAssignForRefType;
		// TODO: 3.0.0: Remove the alterSyntaxNamedArgs
		int    alterSyntaxNamedArgs;
		bool   disableIntegerDivision;
		bool   disallowEmptyListElements;
		// TODO: 3.0.0: Remove the privatePropAsProtected
		bool   privatePropAsProtected;
		bool   allowUnicodeIdentifiers;
		int    heredocTrimMode;
		asUINT maxNestedCalls;
		asUINT genericCallMode;
		asUINT initCallStackSize;
		asUINT maxCallStackSize;
		bool   ignoreDuplicateSharedIntf;
		bool   noDebugOutput;
		bool   disableScriptClassGC;
		asUINT jitInterfaceVersion;
		asUINT alwaysImplDefaultCopy;
		asUINT alwaysImplDefaultCopyConstruct;
		asUINT memberInitMode;
		asUINT boolConversionMode;
		bool   foreachSupport;
	} ep;

	// Callbacks
#ifndef AS_NO_EXCEPTIONS
	bool                       translateExceptionCallback;
	asSSystemFunctionInterface translateExceptionCallbackFunc;
	void *                     translateExceptionCallbackObj;
#endif

	// This flag is to allow a quicker shutdown when releasing the engine
	bool shuttingDown;

	// This flag is set when the engine's destructor is called, this is to
	// avoid recursive calls if an object happens to increment/decrement
	// the ref counter during shutdown
	bool inDestructor;
};

END_AS_NAMESPACE

#endif
