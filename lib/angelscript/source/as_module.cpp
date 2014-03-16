/*
   AngelCode Scripting Library
   Copyright (c) 2003-2014 Andreas Jonsson

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
// as_module.cpp
//
// A class that holds a script module
//

#include "as_config.h"
#include "as_module.h"
#include "as_builder.h"
#include "as_context.h"
#include "as_texts.h"
#include "as_debug.h"
#include "as_restore.h"

BEGIN_AS_NAMESPACE

// internal
asCModule::asCModule(const char *name, asCScriptEngine *engine)
{
	this->name     = name;
	this->engine   = engine;

	userData = 0;
	builder = 0;
	isGlobalVarInitialized = false;

	accessMask = 1;

	defaultNamespace = engine->nameSpaces[0];
}

// internal
asCModule::~asCModule()
{
	InternalReset();

	if( builder )
	{
		asDELETE(builder,asCBuilder);
		builder = 0;
	}

	if( engine )
	{
		// Clean the user data
		if( userData && engine->cleanModuleFunc )
			engine->cleanModuleFunc(this);

		// Remove the module from the engine
		if( engine->lastModule == this )
			engine->lastModule = 0;
		engine->scriptModules.RemoveValue(this);
	}
}

// interface
void asCModule::Discard()
{
	asDELETE(this,asCModule);
}

// interface
void *asCModule::SetUserData(void *data)
{
	void *oldData = userData;
	userData = data;
	return oldData;
}

// interface
void *asCModule::GetUserData() const
{
	return userData;
}

// interface
asIScriptEngine *asCModule::GetEngine() const
{
	return engine;
}

// interface
void asCModule::SetName(const char *name)
{
	this->name = name;
}

// interface
const char *asCModule::GetName() const
{
	return name.AddressOf();
}

// interface
const char *asCModule::GetDefaultNamespace() const
{
	return defaultNamespace->name.AddressOf();
}

// interface
int asCModule::SetDefaultNamespace(const char *nameSpace)
{
	// TODO: cleanup: This function is similar to asCScriptEngine::SetDefaultNamespace. Can we reuse the code?
	if( nameSpace == 0 )
		return asINVALID_ARG;

	asCString ns = nameSpace;
	if( ns != "" )
	{
		// Make sure the namespace is composed of alternating identifier and ::
		size_t pos = 0;
		bool expectIdentifier = true;
		size_t len;
		eTokenType t = ttIdentifier;

		for( ; pos < ns.GetLength(); pos += len )
		{
			t = engine->tok.GetToken(ns.AddressOf() + pos, ns.GetLength() - pos, &len);
			if( (expectIdentifier && t != ttIdentifier) || (!expectIdentifier && t != ttScope) )
				return asINVALID_DECLARATION;

			expectIdentifier = !expectIdentifier;
		}

		// If the namespace ends with :: then strip it off
		if( t == ttScope )
			ns.SetLength(ns.GetLength()-2);
	}

	defaultNamespace = engine->AddNameSpace(ns.AddressOf());

	return 0;
}

// interface
int asCModule::AddScriptSection(const char *name, const char *code, size_t codeLength, int lineOffset)
{
#ifdef AS_NO_COMPILER
	UNUSED_VAR(name);
	UNUSED_VAR(code);
	UNUSED_VAR(codeLength);
	UNUSED_VAR(lineOffset);
	return asNOT_SUPPORTED;
#else
	if( !builder )
	{
		builder = asNEW(asCBuilder)(engine, this);
		if( builder == 0 )
			return asOUT_OF_MEMORY;
	}

	return builder->AddCode(name, code, (int)codeLength, lineOffset, (int)engine->GetScriptSectionNameIndex(name ? name : ""), engine->ep.copyScriptSections);
#endif
}

// internal
void asCModule::JITCompile()
{
	asIJITCompiler *jit = engine->GetJITCompiler();
	if( !jit )
		return;

	for (unsigned int i = 0; i < scriptFunctions.GetLength(); i++)
		scriptFunctions[i]->JITCompile();
}

// interface
int asCModule::Build()
{
#ifdef AS_NO_COMPILER
	return asNOT_SUPPORTED;
#else
	TimeIt("asCModule::Build");

	// Only one thread may build at one time
	// TODO: It should be possible to have multiple threads perform compilations
	int r = engine->RequestBuild();
	if( r < 0 )
		return r;

	engine->PrepareEngine();
	if( engine->configFailed )
	{
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_INVALID_CONFIGURATION);
		engine->BuildCompleted();
		return asINVALID_CONFIGURATION;
	}

 	InternalReset();

	if( !builder )
	{
		engine->BuildCompleted();
		return asSUCCESS;
	}

	// Compile the script
	r = builder->Build();
	asDELETE(builder,asCBuilder);
	builder = 0;
	
	if( r < 0 )
	{
		// Reset module again
		InternalReset();

		engine->BuildCompleted();
		return r;
	}

	JITCompile();

	engine->PrepareEngine();

#ifdef AS_DEBUG
	// Verify that there are no unwanted gaps in the scriptFunctions array.
	for( asUINT n = 1; n < engine->scriptFunctions.GetLength(); n++ )
	{
		int id = n;
		if( engine->scriptFunctions[n] == 0 && !engine->freeScriptFunctionIds.Exists(id) )
			asASSERT( false );
	}
#endif

	engine->BuildCompleted();

	// Initialize global variables
	if( r >= 0 && engine->ep.initGlobalVarsAfterBuild )
		r = ResetGlobalVars(0);

	return r;
#endif
}

// interface
int asCModule::ResetGlobalVars(asIScriptContext *ctx)
{
	if( isGlobalVarInitialized ) 
		CallExit();

	return CallInit(ctx);
}

// interface
asIScriptFunction *asCModule::GetFunctionByIndex(asUINT index) const
{
	return const_cast<asCScriptFunction*>(globalFunctions.Get(index));
}

// internal
int asCModule::CallInit(asIScriptContext *myCtx)
{
	if( isGlobalVarInitialized ) 
		return asERROR;

	// Each global variable needs to be cleared individually
	asCSymbolTableIterator<asCGlobalProperty> it = scriptGlobals.List();
	while( it )
	{
		asCGlobalProperty *desc = *it;
		memset(desc->GetAddressOfValue(), 0, sizeof(asDWORD)*desc->type.GetSizeOnStackDWords());
		it++;
	}

	// Call the init function for each of the global variables
	asIScriptContext *ctx = myCtx;
	int r = asEXECUTION_FINISHED;
	it = scriptGlobals.List();
	while( it && r == asEXECUTION_FINISHED )
	{
		asCGlobalProperty *desc = *it;
		it++;
		if( desc->GetInitFunc() )
		{
			if( ctx == 0 )
			{
				r = engine->CreateContext(&ctx, true);
				if( r < 0 )
					break;
			}

			r = ctx->Prepare(desc->GetInitFunc());
			if( r >= 0 )
			{
				r = ctx->Execute();
				if( r != asEXECUTION_FINISHED )
				{
					asCString msg;
					msg.Format(TXT_FAILED_TO_INITIALIZE_s, desc->name.AddressOf());
					asCScriptFunction *func = desc->GetInitFunc();

					engine->WriteMessage(func->scriptData->scriptSectionIdx >= 0 ? engine->scriptSectionNames[func->scriptData->scriptSectionIdx]->AddressOf() : "",
										 func->GetLineNumber(0, 0) & 0xFFFFF, 
										 func->GetLineNumber(0, 0) >> 20,
										 asMSGTYPE_ERROR,
										 msg.AddressOf());
										 
					if( r == asEXECUTION_EXCEPTION )
					{
						const asIScriptFunction *function = ctx->GetExceptionFunction();

						msg.Format(TXT_EXCEPTION_s_IN_s, ctx->GetExceptionString(), function->GetDeclaration());

						engine->WriteMessage(function->GetScriptSectionName(), 
											 ctx->GetExceptionLineNumber(), 
											 0,
											 asMSGTYPE_INFORMATION,
											 msg.AddressOf());
					}
				}
			}
		}
	}

	if( ctx && !myCtx )
	{
		ctx->Release();
		ctx = 0;
	}

	// Even if the initialization failed we need to set the 
	// flag that the variables have been initialized, otherwise
	// the module won't free those variables that really were 
	// initialized.
	isGlobalVarInitialized = true;

	if( r != asEXECUTION_FINISHED )
		return asINIT_GLOBAL_VARS_FAILED;

	return asSUCCESS;
}

// internal
void asCModule::CallExit()
{
	if( !isGlobalVarInitialized ) return;

	asCSymbolTableIterator<asCGlobalProperty> it = scriptGlobals.List();
	while( it )
	{
		if( (*it)->type.IsObject() )
		{
			void **obj = (void**)(*it)->GetAddressOfValue();
			if( *obj )
			{
				asCObjectType *ot = (*it)->type.GetObjectType();

				if( ot->flags & asOBJ_REF )
				{
					asASSERT( (ot->flags & asOBJ_NOCOUNT) || ot->beh.release );
					if( ot->beh.release )
						engine->CallObjectMethod(*obj, ot->beh.release);
				}
				else
				{
					if( ot->beh.destruct )
						engine->CallObjectMethod(*obj, ot->beh.destruct);

					engine->CallFree(*obj);
				}

				// Set the address to 0 as someone might try to access the variable afterwards
				*obj = 0;
			}
		}
		it++;
	}

	isGlobalVarInitialized = false;
}

// internal
void asCModule::InternalReset()
{
	CallExit();

	size_t n;

	// Release all global functions
	asCSymbolTable<asCScriptFunction>::iterator funcIt = globalFunctions.List();
	for( ; funcIt; funcIt++ )
		(*funcIt)->Release();
	globalFunctions.Clear();

	// First release all compiled functions
	for( n = 0; n < scriptFunctions.GetLength(); n++ )
		if( scriptFunctions[n] )
			scriptFunctions[n]->Orphan(this);
	scriptFunctions.SetLength(0);

	// Release the global properties declared in the module
	asCSymbolTableIterator<asCGlobalProperty> globIt = scriptGlobals.List();
	while( globIt )
	{
		(*globIt)->Orphan(this);
		globIt++;
	}
	scriptGlobals.Clear();

	UnbindAllImportedFunctions();

	// Free bind information
	for( n = 0; n < bindInformations.GetLength(); n++ )
	{
		if( bindInformations[n] )
		{
			asUINT id = bindInformations[n]->importedFunctionSignature->id & ~FUNC_IMPORTED;
			engine->importedFunctions[id] = 0;
			engine->freeImportedFunctionIdxs.PushLast(id);

			asDELETE(bindInformations[n]->importedFunctionSignature, asCScriptFunction);
			asDELETE(bindInformations[n], sBindInfo);
		}
	}
	bindInformations.SetLength(0);

	// Free declared types, including classes, typedefs, and enums
	// TODO: optimize: Check if it is possible to destroy the object directly without notifying the GC
	for( n = 0; n < classTypes.GetLength(); n++ )
		classTypes[n]->Orphan(this);
	classTypes.SetLength(0);
	for( n = 0; n < enumTypes.GetLength(); n++ )
		enumTypes[n]->Release();
	enumTypes.SetLength(0);
	for( n = 0; n < typeDefs.GetLength(); n++ )
		typeDefs[n]->Release();
	typeDefs.SetLength(0);

	// Free funcdefs
	for( n = 0; n < funcDefs.GetLength(); n++ )
	{
		// The funcdefs are not removed from the engine at this moment as they may still be referred
		// to by other types. The engine's ClearUnusedTypes will take care of the clean up.
		funcDefs[n]->Release();
	}
	funcDefs.SetLength(0);

	// Allow the engine to clean up what is not used
	engine->CleanupAfterDiscardModule();

	asASSERT( IsEmpty() );
}

// interface
asIScriptFunction *asCModule::GetFunctionByName(const char *name) const
{
	const asCArray<unsigned int> &idxs = globalFunctions.GetIndexes(defaultNamespace, name);
	if( idxs.GetLength() != 1 )
		return 0;

	const asIScriptFunction *func = globalFunctions.Get(idxs[0]);
	return const_cast<asIScriptFunction*>(func);
}

// interface
asUINT asCModule::GetImportedFunctionCount() const
{
	return (asUINT)bindInformations.GetLength();
}

// interface
int asCModule::GetImportedFunctionIndexByDecl(const char *decl) const
{
	asCBuilder bld(engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(engine, const_cast<asCModule*>(this), asFUNC_DUMMY);
	bld.ParseFunctionDeclaration(0, decl, &func, false, 0, 0, defaultNamespace);

	// TODO: optimize: Improve linear search
	// Search script functions for matching interface
	int id = -1;
	for( asUINT n = 0; n < bindInformations.GetLength(); ++n )
	{
		if( func.name == bindInformations[n]->importedFunctionSignature->name && 
			func.returnType == bindInformations[n]->importedFunctionSignature->returnType &&
			func.parameterTypes.GetLength() == bindInformations[n]->importedFunctionSignature->parameterTypes.GetLength() )
		{
			bool match = true;
			for( asUINT p = 0; p < func.parameterTypes.GetLength(); ++p )
			{
				if( func.parameterTypes[p] != bindInformations[n]->importedFunctionSignature->parameterTypes[p] )
				{
					match = false;
					break;
				}
			}

			if( match )
			{
				if( id == -1 )
					id = n;
				else
					return asMULTIPLE_FUNCTIONS;
			}
		}
	}

	if( id == -1 ) return asNO_FUNCTION;

	return id;
}

// interface
asUINT asCModule::GetFunctionCount() const
{
	return (asUINT)globalFunctions.GetSize();
}

// interface
asIScriptFunction *asCModule::GetFunctionByDecl(const char *decl) const
{
	asCBuilder bld(engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(engine, const_cast<asCModule*>(this), asFUNC_DUMMY);
	int r = bld.ParseFunctionDeclaration(0, decl, &func, false, 0, 0, defaultNamespace);
	if( r < 0 )
	{
		// Invalid declaration
		// TODO: Write error to message stream
		return 0;
	}

	// Use the defaultNamespace implicitly unless an explicit namespace has been provided
	asSNameSpace *ns = func.nameSpace == engine->nameSpaces[0] ? defaultNamespace : func.nameSpace;

	// Search script functions for matching interface
	asIScriptFunction *f = 0;
	const asCArray<unsigned int> &idxs = globalFunctions.GetIndexes(ns, func.name);
	for( unsigned int n = 0; n < idxs.GetLength(); n++ )
	{
		const asCScriptFunction *funcPtr = globalFunctions.Get(idxs[n]);
		if( funcPtr->objectType == 0 &&
			func.returnType                 == funcPtr->returnType &&
			func.parameterTypes.GetLength() == funcPtr->parameterTypes.GetLength()
			)
		{
			bool match = true;
			for( size_t p = 0; p < func.parameterTypes.GetLength(); ++p )
			{
				if( func.parameterTypes[p] != funcPtr->parameterTypes[p] )
				{
					match = false;
					break;
				}
			}

			if( match )
			{
				if( f == 0 )
					f = const_cast<asCScriptFunction*>(funcPtr);
				else
					// Multiple functions
					return 0;
			}
		}
	}

	return f;
}

// interface
asUINT asCModule::GetGlobalVarCount() const
{
	return (asUINT)scriptGlobals.GetSize();
}

// interface
int asCModule::GetGlobalVarIndexByName(const char *name) const
{
	// Find the global var id
	int id = scriptGlobals.GetFirstIndex(defaultNamespace, name);

	if( id == -1 ) return asNO_GLOBAL_VAR;

	return id;
}

// interface
int asCModule::RemoveGlobalVar(asUINT index)
{
	asCGlobalProperty *prop = scriptGlobals.Get(index);
	if( !prop )
		return asINVALID_ARG;
	prop->Orphan(this);
	scriptGlobals.Erase(index);

	return 0;
}

// interface
int asCModule::GetGlobalVarIndexByDecl(const char *decl) const
{
	asCBuilder bld(engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCString name;
	asSNameSpace *nameSpace;
	asCDataType dt;
	int r = bld.ParseVariableDeclaration(decl, defaultNamespace, name, nameSpace, dt);
	if( r < 0 )
		return r;

	// Search global variables for a match
	int id = scriptGlobals.GetFirstIndex(nameSpace, name, asCCompGlobPropType(dt));
	if( id != -1 )
		return id;

	return asNO_GLOBAL_VAR;
}

// interface
void *asCModule::GetAddressOfGlobalVar(asUINT index)
{
	asCGlobalProperty *prop = scriptGlobals.Get(index);
	if( !prop ) 
		return 0;

	// For object variables it's necessary to dereference the pointer to get the address of the value
	if( prop->type.IsObject() &&
		!prop->type.IsObjectHandle() )
		return *(void**)(prop->GetAddressOfValue());

	return (void*)(prop->GetAddressOfValue());
}

// interface
const char *asCModule::GetGlobalVarDeclaration(asUINT index, bool includeNamespace) const
{
	const asCGlobalProperty *prop = scriptGlobals.Get(index);
	if (!prop) return 0;

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = prop->type.Format();
	*tempString += " ";
	if( includeNamespace )
		*tempString += prop->nameSpace->name + "::";
	*tempString += prop->name;

	return tempString->AddressOf();
}

// interface
int asCModule::GetGlobalVar(asUINT index, const char **name, const char **nameSpace, int *typeId, bool *isConst) const
{
	const asCGlobalProperty *prop = scriptGlobals.Get(index);
	if (!prop) return 0;

	if( name )
		*name = prop->name.AddressOf();
	if( nameSpace )
		*nameSpace = prop->nameSpace->name.AddressOf();
	if( typeId )
		*typeId = engine->GetTypeIdFromDataType(prop->type);
	if( isConst )
		*isConst = prop->type.IsReadOnly();

	return asSUCCESS;
}

// interface
asUINT asCModule::GetObjectTypeCount() const
{
	return (asUINT)classTypes.GetLength();
}

// interface 
asIObjectType *asCModule::GetObjectTypeByIndex(asUINT index) const
{
	if( index >= classTypes.GetLength() ) 
		return 0;

	return classTypes[index];
}

// interface
asIObjectType *asCModule::GetObjectTypeByName(const char *name) const
{
	for( asUINT n = 0; n < classTypes.GetLength(); n++ )
	{
		if( classTypes[n] &&
			classTypes[n]->name == name &&
			classTypes[n]->nameSpace == defaultNamespace )
			return classTypes[n];
	}

	return 0;
}

// interface
int asCModule::GetTypeIdByDecl(const char *decl) const
{
	asCDataType dt;

	// This const cast is safe since we know the engine won't be modified
	asCBuilder bld(engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	int r = bld.ParseDataType(decl, &dt, defaultNamespace);
	if( r < 0 )
		return asINVALID_TYPE;

	return engine->GetTypeIdFromDataType(dt);
}

// interface
asUINT asCModule::GetEnumCount() const
{
	return (asUINT)enumTypes.GetLength();
}

// interface
const char *asCModule::GetEnumByIndex(asUINT index, int *enumTypeId, const char **nameSpace) const
{
	if( index >= enumTypes.GetLength() )
		return 0;

	if( enumTypeId )
		*enumTypeId = engine->GetTypeIdFromDataType(asCDataType::CreateObject(enumTypes[index], false));

	if( nameSpace )
		*nameSpace = enumTypes[index]->nameSpace->name.AddressOf();

	return enumTypes[index]->name.AddressOf();
}

// interface
int asCModule::GetEnumValueCount(int enumTypeId) const
{
	asCDataType dt = engine->GetDataTypeFromTypeId(enumTypeId);
	asCObjectType *t = dt.GetObjectType();
	if( t == 0 || !(t->GetFlags() & asOBJ_ENUM) ) 
		return asINVALID_TYPE;

	return (int)t->enumValues.GetLength();
}

// interface
const char *asCModule::GetEnumValueByIndex(int enumTypeId, asUINT index, int *outValue) const
{
	asCDataType dt = engine->GetDataTypeFromTypeId(enumTypeId);
	asCObjectType *t = dt.GetObjectType();
	if( t == 0 || !(t->GetFlags() & asOBJ_ENUM) ) 
		return 0;

	if( index >= t->enumValues.GetLength() )
		return 0;

	if( outValue )
		*outValue = t->enumValues[index]->value;

	return t->enumValues[index]->name.AddressOf();
}

// interface
asUINT asCModule::GetTypedefCount() const
{
	return (asUINT)typeDefs.GetLength();
}

// interface
const char *asCModule::GetTypedefByIndex(asUINT index, int *typeId, const char **nameSpace) const
{
	if( index >= typeDefs.GetLength() )
		return 0;

	if( typeId )
		*typeId = engine->GetTypeIdFromDataType(typeDefs[index]->templateSubTypes[0]); 

	if( nameSpace )
		*nameSpace = typeDefs[index]->nameSpace->name.AddressOf();

	return typeDefs[index]->name.AddressOf();
}


// internal
int asCModule::GetNextImportedFunctionId()
{
	// TODO: multithread: This will break if one thread if freeing a module, while another is being compiled
	if( engine->freeImportedFunctionIdxs.GetLength() )
		return FUNC_IMPORTED | (asUINT)engine->freeImportedFunctionIdxs[engine->freeImportedFunctionIdxs.GetLength()-1];

	return FUNC_IMPORTED | (asUINT)engine->importedFunctions.GetLength();
}

#ifndef AS_NO_COMPILER
// internal
int asCModule::AddScriptFunction(int sectionIdx, int declaredAt, int id, const asCString &name, const asCDataType &returnType, const asCArray<asCDataType> &params, const asCArray<asETypeModifiers> &inOutFlags, const asCArray<asCString *> &defaultArgs, bool isInterface, asCObjectType *objType, bool isConstMethod, bool isGlobalFunction, bool isPrivate, bool isFinal, bool isOverride, bool isShared, asSNameSpace *ns)
{
	asASSERT(id >= 0);

	// Store the function information
	asCScriptFunction *func = asNEW(asCScriptFunction)(engine, this, isInterface ? asFUNC_INTERFACE : asFUNC_SCRIPT);
	if( func == 0 )
	{
		// Free the default args
		for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
			if( defaultArgs[n] )
				asDELETE(defaultArgs[n], asCString);

		return asOUT_OF_MEMORY;
	}

	if( ns == 0 )
		ns = engine->nameSpaces[0];

	// All methods of shared objects are also shared
	if( objType && objType->IsShared() )
		isShared = true;

	func->name             = name;
	func->nameSpace        = ns;
	func->id               = id;
	func->returnType       = returnType;
	if( func->funcType == asFUNC_SCRIPT )
	{
		func->scriptData->scriptSectionIdx = sectionIdx;
		func->scriptData->declaredAt = declaredAt;
	}
	func->parameterTypes   = params;
	func->inOutFlags       = inOutFlags;
	func->defaultArgs      = defaultArgs;
	func->objectType       = objType;
	func->isReadOnly       = isConstMethod;
	func->isPrivate        = isPrivate;
	func->isFinal          = isFinal;
	func->isOverride       = isOverride;
	func->isShared         = isShared;

	asASSERT( params.GetLength() == inOutFlags.GetLength() && params.GetLength() == defaultArgs.GetLength() );

	// Verify that we are not assigning either the final or override specifier(s) if we are registering a non-member function
	asASSERT( !(!objType && isFinal) );
	asASSERT( !(!objType && isOverride) );

	// The script function's refCount was initialized to 1
	scriptFunctions.PushLast(func);
	engine->SetScriptFunction(func);

	// Compute the signature id
	if( objType )
		func->ComputeSignatureId();

	// Add reference
	if( isGlobalFunction )
	{
		globalFunctions.Put(func);
		func->AddRef();
	}

	return 0;
}

// internal 
int asCModule::AddScriptFunction(asCScriptFunction *func)
{
	scriptFunctions.PushLast(func);
	func->AddRef();
	engine->SetScriptFunction(func);

	return 0;
}

// internal
int asCModule::AddImportedFunction(int id, const asCString &name, const asCDataType &returnType, const asCArray<asCDataType> &params, const asCArray<asETypeModifiers> &inOutFlags, const asCArray<asCString *> &defaultArgs, asSNameSpace *ns, const asCString &moduleName)
{
	asASSERT(id >= 0);

	// Store the function information
	asCScriptFunction *func = asNEW(asCScriptFunction)(engine, this, asFUNC_IMPORTED);
	if( func == 0 )
	{
		// Free the default args
		for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
			if( defaultArgs[n] )
				asDELETE(defaultArgs[n], asCString);

		return asOUT_OF_MEMORY;
	}

	func->name           = name;
	func->id             = id;
	func->returnType     = returnType;
	func->nameSpace      = ns;
	func->parameterTypes = params;
	func->inOutFlags     = inOutFlags;
	func->defaultArgs    = defaultArgs;
	func->objectType     = 0;

	sBindInfo *info = asNEW(sBindInfo);
	if( info == 0 )
	{
		asDELETE(func, asCScriptFunction);
		return asOUT_OF_MEMORY;
	}

	info->importedFunctionSignature = func;
	info->boundFunctionId           = -1;
	info->importFromModule          = moduleName;
	bindInformations.PushLast(info);

	// Add the info to the array in the engine
	if( engine->freeImportedFunctionIdxs.GetLength() )
		engine->importedFunctions[engine->freeImportedFunctionIdxs.PopLast()] = info;
	else
		engine->importedFunctions.PushLast(info);

	return 0;
}
#endif

// internal
asCScriptFunction *asCModule::GetImportedFunction(int index) const
{
	return bindInformations[index]->importedFunctionSignature;
}

// interface
int asCModule::BindImportedFunction(asUINT index, asIScriptFunction *func)
{
	// First unbind the old function
	int r = UnbindImportedFunction(index);
	if( r < 0 ) return r;

	// Must verify that the interfaces are equal
	asCScriptFunction *dst = GetImportedFunction(index);
	if( dst == 0 ) return asNO_FUNCTION;

	if( func == 0 )
		return asINVALID_ARG;

	asCScriptFunction *src = engine->GetScriptFunction(func->GetId());
	if( src == 0 ) 
		return asNO_FUNCTION;

	// Verify return type
	if( dst->returnType != src->returnType )
		return asINVALID_INTERFACE;

	if( dst->parameterTypes.GetLength() != src->parameterTypes.GetLength() )
		return asINVALID_INTERFACE;

	for( size_t n = 0; n < dst->parameterTypes.GetLength(); ++n )
	{
		if( dst->parameterTypes[n] != src->parameterTypes[n] )
			return asINVALID_INTERFACE;
	}

	bindInformations[index]->boundFunctionId = src->GetId();
	src->AddRef();

	return asSUCCESS;
}

// interface
int asCModule::UnbindImportedFunction(asUINT index)
{
	if( index >= bindInformations.GetLength() )
		return asINVALID_ARG;

	// Remove reference to old module
	if( bindInformations[index] )
	{
		int oldFuncID = bindInformations[index]->boundFunctionId;
		if( oldFuncID != -1 )
		{
			bindInformations[index]->boundFunctionId = -1;
			engine->scriptFunctions[oldFuncID]->Release();
		}
	}

	return asSUCCESS;
}

// interface
const char *asCModule::GetImportedFunctionDeclaration(asUINT index) const
{
	asCScriptFunction *func = GetImportedFunction(index);
	if( func == 0 ) return 0;

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = func->GetDeclarationStr();

	return tempString->AddressOf();
}

// interface
const char *asCModule::GetImportedFunctionSourceModule(asUINT index) const
{
	if( index >= bindInformations.GetLength() )
		return 0;

	return bindInformations[index]->importFromModule.AddressOf();
}

// inteface
int asCModule::BindAllImportedFunctions()
{
	bool notAllFunctionsWereBound = false;

	// Bind imported functions
	int c = GetImportedFunctionCount();
	for( int n = 0; n < c; ++n )
	{
		asCScriptFunction *importFunc = GetImportedFunction(n);
		if( importFunc == 0 ) return asERROR;

		asCString str = importFunc->GetDeclarationStr();

		// Get module name from where the function should be imported
		const char *moduleName = GetImportedFunctionSourceModule(n);
		if( moduleName == 0 ) return asERROR;

		asCModule *srcMod = engine->GetModule(moduleName, false);
		asIScriptFunction *func = 0;
		if( srcMod )
			func = srcMod->GetFunctionByDecl(str.AddressOf());

		if( func == 0 )
			notAllFunctionsWereBound = true;
		else
		{
			if( BindImportedFunction(n, func) < 0 )
				notAllFunctionsWereBound = true;
		}
	}

	if( notAllFunctionsWereBound )
		return asCANT_BIND_ALL_FUNCTIONS;

	return asSUCCESS;
}

// interface
int asCModule::UnbindAllImportedFunctions()
{
	asUINT c = GetImportedFunctionCount();
	for( asUINT n = 0; n < c; ++n )
		UnbindImportedFunction(n);

	return asSUCCESS;
}

// internal
asCObjectType *asCModule::GetObjectType(const char *type, asSNameSpace *ns)
{
	size_t n;

	// TODO: optimize: Improve linear search
	for( n = 0; n < classTypes.GetLength(); n++ )
		if( classTypes[n]->name == type &&
			classTypes[n]->nameSpace == ns )
			return classTypes[n];

	for( n = 0; n < enumTypes.GetLength(); n++ )
		if( enumTypes[n]->name == type && 
			enumTypes[n]->nameSpace == ns )
			return enumTypes[n];

	for( n = 0; n < typeDefs.GetLength(); n++ )
		if( typeDefs[n]->name == type && 
			typeDefs[n]->nameSpace == ns )
			return typeDefs[n];

	return 0;
}

// internal
asCGlobalProperty *asCModule::AllocateGlobalProperty(const char *name, const asCDataType &dt, asSNameSpace *ns)
{
	asCGlobalProperty *prop = engine->AllocateGlobalProperty();
	prop->name = name;
	prop->nameSpace = ns;

	// Allocate the memory for this property based on its type
	prop->type = dt;
	prop->AllocateMemory();

	// Make an entry in the address to variable map
	engine->varAddressMap.Insert(prop->GetAddressOfValue(), prop);

	// Store the variable in the module scope (the reference count is already set to 1)
	scriptGlobals.Put(prop);

	return prop;
}

// internal
bool asCModule::IsEmpty() const
{
	if( scriptFunctions.GetLength()  ) return false;
	if( globalFunctions.GetSize()    ) return false;
	if( bindInformations.GetLength() ) return false;
	if( scriptGlobals.GetSize()      ) return false;
	if( classTypes.GetLength()       ) return false;
	if( enumTypes.GetLength()        ) return false;
	if( typeDefs.GetLength()         ) return false;
	if( funcDefs.GetLength()         ) return false;

	return true;
}

// interface
int asCModule::SaveByteCode(asIBinaryStream *out, bool stripDebugInfo) const
{
#ifdef AS_NO_COMPILER
	UNUSED_VAR(out);
	UNUSED_VAR(stripDebugInfo);
	return asNOT_SUPPORTED;
#else
	if( out == 0 ) return asINVALID_ARG;

	// Make sure there is actually something to save
	if( IsEmpty() )
		return asERROR;

	asCWriter write(const_cast<asCModule*>(this), out, engine, stripDebugInfo);
	return write.Write();
#endif
}

// interface
int asCModule::LoadByteCode(asIBinaryStream *in, bool *wasDebugInfoStripped)
{
	if( in == 0 ) return asINVALID_ARG;

	// Only permit loading bytecode if no other thread is currently compiling
	// TODO: It should be possible to have multiple threads perform compilations
	int r = engine->RequestBuild();
	if( r < 0 )
		return r;

	asCReader read(this, in, engine);
	r = read.Read(wasDebugInfoStripped);

	JITCompile();

#ifdef AS_DEBUG
	// Verify that there are no unwanted gaps in the scriptFunctions array.
	for( asUINT n = 1; n < engine->scriptFunctions.GetLength(); n++ )
	{
		int id = n;
		if( engine->scriptFunctions[n] == 0 && !engine->freeScriptFunctionIds.Exists(id) )
			asASSERT( false );
	}
#endif

	engine->BuildCompleted();

	return r;
}

// interface
int asCModule::CompileGlobalVar(const char *sectionName, const char *code, int lineOffset)
{
#ifdef AS_NO_COMPILER
	UNUSED_VAR(sectionName);
	UNUSED_VAR(code);
	UNUSED_VAR(lineOffset);
	return asNOT_SUPPORTED;
#else
	// Validate arguments
	if( code == 0 )
		return asINVALID_ARG;

	// Only one thread may build at one time
	// TODO: It should be possible to have multiple threads perform compilations
	int r = engine->RequestBuild();
	if( r < 0 )
		return r;

	// Prepare the engine
	engine->PrepareEngine();
	if( engine->configFailed )
	{
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_INVALID_CONFIGURATION);
		engine->BuildCompleted();
		return asINVALID_CONFIGURATION;
	}

	// Compile the global variable and add it to the module scope
	asCBuilder builder(engine, this);
	asCString str = code;
	r = builder.CompileGlobalVar(sectionName, str.AddressOf(), lineOffset);

	engine->BuildCompleted();

	// Initialize the variable
	if( r >= 0 && engine->ep.initGlobalVarsAfterBuild )
	{
		// Clear the memory 
		asCGlobalProperty *prop = scriptGlobals.GetLast();
		if( prop )
		{
			memset(prop->GetAddressOfValue(), 0, sizeof(asDWORD)*prop->type.GetSizeOnStackDWords());
	
			if( prop->GetInitFunc() )
			{
				// Call the init function for the global variable
				asIScriptContext *ctx = 0;
				int r = engine->CreateContext(&ctx, true);
				if( r < 0 )
					return r;
	
				r = ctx->Prepare(prop->GetInitFunc());
				if( r >= 0 )
					r = ctx->Execute();
	
				ctx->Release();
			}
		}
	}

	return r;
#endif
}

// interface
int asCModule::CompileFunction(const char *sectionName, const char *code, int lineOffset, asDWORD compileFlags, asIScriptFunction **outFunc)
{
	// Make sure the outFunc is null if the function fails, so the 
	// application doesn't attempt to release a non-existent function
	if( outFunc )
		*outFunc = 0;

#ifdef AS_NO_COMPILER
	UNUSED_VAR(sectionName);
	UNUSED_VAR(code);
	UNUSED_VAR(lineOffset);
	UNUSED_VAR(compileFlags);
	return asNOT_SUPPORTED;
#else
	// Validate arguments
	if( code == 0 || 
		(compileFlags != 0 && compileFlags != asCOMP_ADD_TO_MODULE) )
		return asINVALID_ARG;

	// Only one thread may build at one time
	// TODO: It should be possible to have multiple threads perform compilations
	int r = engine->RequestBuild();
	if( r < 0 )
		return r;

	// Prepare the engine
	engine->PrepareEngine();
	if( engine->configFailed )
	{
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_INVALID_CONFIGURATION);
		engine->BuildCompleted();
		return asINVALID_CONFIGURATION;
	}

	// Compile the single function
	asCBuilder builder(engine, this);
	asCString str = code;
	asCScriptFunction *func = 0;
	r = builder.CompileFunction(sectionName, str.AddressOf(), lineOffset, compileFlags, &func);

	engine->BuildCompleted();

	if( r >= 0 && outFunc && func )
	{
		// Return the function to the caller
		*outFunc = func;
		func->AddRef();
	}

	// Release our reference to the function
	if( func )
		func->Release();

	return r;
#endif
}

// interface
int asCModule::RemoveFunction(asIScriptFunction *func)
{
	// Find the global function
	asCScriptFunction *f = static_cast<asCScriptFunction*>(func);
	int idx = globalFunctions.GetIndex(f);
	if( idx >= 0 )
	{
		globalFunctions.Erase(idx);
		f->Release();
		scriptFunctions.RemoveValue(f);
		f->Orphan(this);
		return 0;
	}

	return asNO_FUNCTION;
}

#ifndef AS_NO_COMPILER
// internal
int asCModule::AddFuncDef(const asCString &name, asSNameSpace *ns)
{
	asCScriptFunction *func = asNEW(asCScriptFunction)(engine, 0, asFUNC_FUNCDEF);
	if( func == 0 )
		return asOUT_OF_MEMORY;

	func->name      = name;
	func->nameSpace = ns;

	funcDefs.PushLast(func);

	engine->funcDefs.PushLast(func);
	func->id = engine->GetNextScriptFunctionId();
	engine->SetScriptFunction(func);

	return (int)funcDefs.GetLength()-1;
}
#endif

// interface
asDWORD asCModule::SetAccessMask(asDWORD mask)
{
	asDWORD old = accessMask;
	accessMask = mask;
	return old;
}

END_AS_NAMESPACE

