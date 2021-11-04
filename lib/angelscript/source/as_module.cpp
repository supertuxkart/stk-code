/*
   AngelCode Scripting Library
   Copyright (c) 2003-2021 Andreas Jonsson

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
	m_name     = name;
	m_engine   = engine;

	m_userData = 0;
	m_builder = 0;
	m_isGlobalVarInitialized = false;

	m_accessMask = 1;

	m_defaultNamespace = engine->nameSpaces[0];
}

// internal
asCModule::~asCModule()
{
	InternalReset();

	// The builder is not removed by InternalReset because it holds the script
	// sections that will be built, so we need to explictly remove it now if it exists
	if( m_builder )
	{
		asDELETE(m_builder,asCBuilder);
		m_builder = 0;
	}

	if( m_engine )
	{
		// Clean the user data
		for( asUINT n = 0; n < m_userData.GetLength(); n += 2 )
		{
			if( m_userData[n+1] )
			{
				for( asUINT c = 0; c < m_engine->cleanModuleFuncs.GetLength(); c++ )
					if( m_engine->cleanModuleFuncs[c].type == m_userData[n] )
						m_engine->cleanModuleFuncs[c].cleanFunc(this);
			}
		}

		// Remove the module from the engine
		ACQUIREEXCLUSIVE(m_engine->engineRWLock);
		// The module must have been discarded before it is deleted
		asASSERT( !m_engine->scriptModules.Exists(this) );
		m_engine->discardedModules.RemoveValue(this);
		RELEASEEXCLUSIVE(m_engine->engineRWLock);
	}
}

// interface
void asCModule::Discard()
{
	// Reset the global variables already so that no object in the global variables keep the module alive forever.
	// If any live object tries to access the global variables during clean up they will fail with a script exception,
	// so the application must keep that in mind before discarding a module.
	CallExit();

	// Keep a local copy of the engine pointer, because once the module is moved do the discarded
	// pile, it is possible that another thread might discard it while we are still in here. So no
	// further access to members may be done after that
	asCScriptEngine *engine = m_engine;

	// Instead of deleting the module immediately, move it to the discarded pile
	// This will turn it invisible to the application, yet keep it alive until all
	// external references to its entities have been released.
	ACQUIREEXCLUSIVE(engine->engineRWLock);
	if( engine->lastModule == this )
		engine->lastModule = 0;
	engine->scriptModules.RemoveValue(this);
	engine->discardedModules.PushLast(this);
	RELEASEEXCLUSIVE(engine->engineRWLock);

	// Allow the engine to go over the list of discarded modules to see what can be cleaned up at this moment.
	// Don't do this if the engine is already shutting down, as it will be done explicitly by the engine itself with error reporting
	if( !engine->shuttingDown )
	{
		if( engine->ep.autoGarbageCollect )
			engine->GarbageCollect();
		else
		{
			// GarbageCollect calls DeleteDiscardedModules, so no need
			// to call it again if we already called GarbageCollect
			engine->DeleteDiscardedModules();
		}
	}
}

// interface
void *asCModule::SetUserData(void *data, asPWORD type)
{
	// As a thread might add a new new user data at the same time as another
	// it is necessary to protect both read and write access to the userData member
	ACQUIREEXCLUSIVE(m_engine->engineRWLock);

	// It is not intended to store a lot of different types of userdata,
	// so a more complex structure like a associative map would just have
	// more overhead than a simple array.
	for( asUINT n = 0; n < m_userData.GetLength(); n += 2 )
	{
		if( m_userData[n] == type )
		{
			void *oldData = reinterpret_cast<void*>(m_userData[n+1]);
			m_userData[n+1] = reinterpret_cast<asPWORD>(data);

			RELEASEEXCLUSIVE(m_engine->engineRWLock);

			return oldData;
		}
	}

	m_userData.PushLast(type);
	m_userData.PushLast(reinterpret_cast<asPWORD>(data));

	RELEASEEXCLUSIVE(m_engine->engineRWLock);

	return 0;
}

// interface
void *asCModule::GetUserData(asPWORD type) const
{
	// There may be multiple threads reading, but when
	// setting the user data nobody must be reading.
	ACQUIRESHARED(m_engine->engineRWLock);

	for( asUINT n = 0; n < m_userData.GetLength(); n += 2 )
	{
		if( m_userData[n] == type )
		{
			void *ud = reinterpret_cast<void*>(m_userData[n+1]);
			RELEASESHARED(m_engine->engineRWLock);
			return ud;
		}
	}

	RELEASESHARED(m_engine->engineRWLock);

	return 0;
}

// interface
asIScriptEngine *asCModule::GetEngine() const
{
	return m_engine;
}

// interface
void asCModule::SetName(const char *in_name)
{
	m_name = in_name;
}

// interface
const char *asCModule::GetName() const
{
	return m_name.AddressOf();
}

// interface
const char *asCModule::GetDefaultNamespace() const
{
	return m_defaultNamespace->name.AddressOf();
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
			t = m_engine->tok.GetToken(ns.AddressOf() + pos, ns.GetLength() - pos, &len);
			if( (expectIdentifier && t != ttIdentifier) || (!expectIdentifier && t != ttScope) )
				return asINVALID_DECLARATION;

			expectIdentifier = !expectIdentifier;
		}

		// If the namespace ends with :: then strip it off
		if( t == ttScope )
			ns.SetLength(ns.GetLength()-2);
	}

	m_defaultNamespace = m_engine->AddNameSpace(ns.AddressOf());

	return 0;
}

// interface
int asCModule::AddScriptSection(const char *in_name, const char *in_code, size_t in_codeLength, int in_lineOffset)
{
#ifdef AS_NO_COMPILER
	UNUSED_VAR(in_name);
	UNUSED_VAR(in_code);
	UNUSED_VAR(in_codeLength);
	UNUSED_VAR(in_lineOffset);
	return asNOT_SUPPORTED;
#else
	if( !m_builder )
	{
		m_builder = asNEW(asCBuilder)(m_engine, this);
		if( m_builder == 0 )
			return asOUT_OF_MEMORY;
	}

	return m_builder->AddCode(in_name, in_code, (int)in_codeLength, in_lineOffset, (int)m_engine->GetScriptSectionNameIndex(in_name ? in_name : ""), m_engine->ep.copyScriptSections);
#endif
}

// internal
void asCModule::JITCompile()
{
	asIJITCompiler *jit = m_engine->GetJITCompiler();
	if( !jit )
		return;

	for (unsigned int i = 0; i < m_scriptFunctions.GetLength(); i++)
		m_scriptFunctions[i]->JITCompile();
}

// interface
int asCModule::Build()
{
#ifdef AS_NO_COMPILER
	return asNOT_SUPPORTED;
#else
	TimeIt("asCModule::Build");

	// Don't allow the module to be rebuilt if there are still
	// external references that will need the previous code
	// TODO: interface: The asIScriptModule must have a method for querying if the module is used
	if( HasExternalReferences(false) )
	{
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_MODULE_IS_IN_USE);
		return asMODULE_IS_IN_USE;
	}

	// Only one thread may build at one time
	// TODO: It should be possible to have multiple threads perform compilations
	int r = m_engine->RequestBuild();
	if( r < 0 )
		return r;

	m_engine->PrepareEngine();
	if( m_engine->configFailed )
	{
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_INVALID_CONFIGURATION);
		m_engine->BuildCompleted();
		return asINVALID_CONFIGURATION;
	}

	InternalReset();

	if( !m_builder )
	{
		m_engine->BuildCompleted();
		return asSUCCESS;
	}

	// Compile the script
	r = m_builder->Build();
	asDELETE(m_builder,asCBuilder);
	m_builder = 0;

	if( r < 0 )
	{
		// Reset module again
		InternalReset();

		m_engine->BuildCompleted();
		return r;
	}

	JITCompile();

	m_engine->PrepareEngine();

#ifdef AS_DEBUG
	// Verify that there are no unwanted gaps in the scriptFunctions array.
	for( asUINT n = 1; n < m_engine->scriptFunctions.GetLength(); n++ )
	{
		int id = n;
		if( m_engine->scriptFunctions[n] == 0 && !m_engine->freeScriptFunctionIds.Exists(id) )
			asASSERT( false );
	}
#endif

	m_engine->BuildCompleted();

	// Initialize global variables
	if( r >= 0 && m_engine->ep.initGlobalVarsAfterBuild )
		r = ResetGlobalVars(0);

	return r;
#endif
}

// interface
int asCModule::ResetGlobalVars(asIScriptContext *ctx)
{
	if( m_isGlobalVarInitialized )
		CallExit();

	return CallInit(ctx);
}

// interface
asIScriptFunction *asCModule::GetFunctionByIndex(asUINT index) const
{
	return const_cast<asCScriptFunction*>(m_globalFunctions.Get(index));
}

// internal
int asCModule::CallInit(asIScriptContext *myCtx)
{
	if( m_isGlobalVarInitialized )
		return asERROR;

	// Each global variable needs to be cleared individually
	asCSymbolTableIterator<asCGlobalProperty> it = m_scriptGlobals.List();
	while( it )
	{
		asCGlobalProperty *desc = *it;
		memset(desc->GetAddressOfValue(), 0, sizeof(asDWORD)*desc->type.GetSizeOnStackDWords());
		it++;
	}

	// Call the init function for each of the global variables
	asIScriptContext *ctx = myCtx;
	int r = asEXECUTION_FINISHED;
	it = m_scriptGlobals.List();
	while( it && r == asEXECUTION_FINISHED )
	{
		asCGlobalProperty *desc = *it;
		it++;
		if( desc->GetInitFunc() )
		{
			if( ctx == 0 )
			{
				ctx = m_engine->RequestContext();
				if( ctx == 0 )
					break;
			}

			r = InitGlobalProp(desc, ctx);
		}
	}

	if( ctx && !myCtx )
	{
		m_engine->ReturnContext(ctx);
		ctx = 0;
	}

	// Even if the initialization failed we need to set the
	// flag that the variables have been initialized, otherwise
	// the module won't free those variables that really were
	// initialized.
	m_isGlobalVarInitialized = true;

	if( r != asEXECUTION_FINISHED )
		return asINIT_GLOBAL_VARS_FAILED;

	return asSUCCESS;
}

// internal
// This function assumes the memory for the global property is already cleared
int asCModule::InitGlobalProp(asCGlobalProperty *prop, asIScriptContext *myCtx)
{
	// Call the init function for each of the global variables
	asIScriptContext *ctx = myCtx;
	int r = asEXECUTION_FINISHED;
	if( prop->GetInitFunc() )
	{
		if( ctx == 0 )
		{
			ctx = m_engine->RequestContext();
			if( ctx == 0 )
				return asERROR;
		}

		r = ctx->Prepare(prop->GetInitFunc());
		if( r >= 0 )
		{
			r = ctx->Execute();
			if( r != asEXECUTION_FINISHED )
			{
				asCString msg;
				msg.Format(TXT_FAILED_TO_INITIALIZE_s, prop->name.AddressOf());
				asCScriptFunction *func = prop->GetInitFunc();

				m_engine->WriteMessage(func->scriptData->scriptSectionIdx >= 0 ? m_engine->scriptSectionNames[func->scriptData->scriptSectionIdx]->AddressOf() : "",
									 func->GetLineNumber(0, 0) & 0xFFFFF,
									 func->GetLineNumber(0, 0) >> 20,
									 asMSGTYPE_ERROR,
									 msg.AddressOf());

				if( r == asEXECUTION_EXCEPTION )
				{
					const asIScriptFunction *function = ctx->GetExceptionFunction();

					msg.Format(TXT_EXCEPTION_s_IN_s, ctx->GetExceptionString(), function->GetDeclaration());

					m_engine->WriteMessage(function->GetScriptSectionName(),
										   ctx->GetExceptionLineNumber(),
										   0,
										   asMSGTYPE_INFORMATION,
										   msg.AddressOf());
				}
			}
		}
	}

	if( ctx && !myCtx )
	{
		m_engine->ReturnContext(ctx);
		ctx = 0;
	}

	// Even if the initialization failed we need to set the
	// flag that the variables have been initialized, otherwise
	// the module won't free those variables that really were
	// initialized.
	m_isGlobalVarInitialized = true;

	if( r != asEXECUTION_FINISHED )
		return asINIT_GLOBAL_VARS_FAILED;

	return asSUCCESS;
}

// internal
void asCModule::UninitializeGlobalProp(asCGlobalProperty *prop)
{
	if (prop == 0) 
		return;

	if (prop->type.IsObject())
	{
		void **obj = (void**)prop->GetAddressOfValue();
		if (*obj)
		{
			asCObjectType *ot = CastToObjectType(prop->type.GetTypeInfo());

			if (ot->flags & asOBJ_REF)
			{
				asASSERT((ot->flags & asOBJ_NOCOUNT) || ot->beh.release);
				if (ot->beh.release)
					m_engine->CallObjectMethod(*obj, ot->beh.release);
			}
			else
			{
				if (ot->beh.destruct)
					m_engine->CallObjectMethod(*obj, ot->beh.destruct);

				m_engine->CallFree(*obj);
			}

			// Set the address to 0 as someone might try to access the variable afterwards
			*obj = 0;
		}
	}
	else if (prop->type.IsFuncdef())
	{
		asCScriptFunction **func = (asCScriptFunction**)prop->GetAddressOfValue();
		if (*func)
		{
			(*func)->Release();
			*func = 0;
		}
	}
}

// internal
void asCModule::CallExit()
{
	if( !m_isGlobalVarInitialized ) return;

	asCSymbolTableIterator<asCGlobalProperty> it = m_scriptGlobals.List();
	while( it )
	{
		UninitializeGlobalProp(*it);
		it++;
	}

	m_isGlobalVarInitialized = false;
}

// internal
bool asCModule::HasExternalReferences(bool shuttingDown)
{
	// Check all entities in the module for any external references.
	// If there are any external references the module cannot be deleted yet.

	asCSymbolTableIterator<asCGlobalProperty> it = m_scriptGlobals.List();
	while (it)
	{
		asCGlobalProperty *desc = *it;
		if (desc->GetInitFunc() && desc->GetInitFunc()->externalRefCount.get())
		{
			if( !shuttingDown )
				return true;
			else
			{
				asCString msg;
				msg.Format(TXT_EXTRNL_REF_TO_MODULE_s, m_name.AddressOf());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_WARNING, msg.AddressOf());

				// TODO: Use a better error message
				asCString tmpName = "init " + desc->name;
				msg.Format(TXT_PREV_FUNC_IS_NAMED_s_TYPE_IS_d, tmpName.AddressOf(), desc->GetInitFunc()->GetFuncType());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
			}
		}
		it++;
	}

	for (asUINT n = 0; n < m_scriptFunctions.GetLength(); n++)
	{
		asCScriptFunction *func = m_scriptFunctions[n];
		if (func && func->externalRefCount.get())
		{
			// If the func is shared and can be moved to another module then this is not a reason to keep the module alive
			if (func->IsShared() && m_engine->FindNewOwnerForSharedFunc(func, this) != this)
				continue;

			if (!shuttingDown)
				return true;
			else
			{
				asCString msg;
				msg.Format(TXT_EXTRNL_REF_TO_MODULE_s, m_name.AddressOf());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_WARNING, msg.AddressOf());

				msg.Format(TXT_PREV_FUNC_IS_NAMED_s_TYPE_IS_d, m_scriptFunctions[n]->GetName(), m_scriptFunctions[n]->GetFuncType());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
			}
		}
	}

	for (asUINT n = 0; n < m_classTypes.GetLength(); n++)
	{
		asCObjectType *obj = m_classTypes[n];
		if (obj && obj->externalRefCount.get())
		{
			// If the obj is shared and can be moved to another module then this is not a reason to keep the module alive
			if (obj->IsShared() && m_engine->FindNewOwnerForSharedType(obj, this) != this)
				continue;

			if (!shuttingDown)
				return true;
			else
			{
				asCString msg;
				msg.Format(TXT_EXTRNL_REF_TO_MODULE_s, m_name.AddressOf());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_WARNING, msg.AddressOf());

				msg.Format(TXT_PREV_TYPE_IS_NAMED_s, m_classTypes[n]->GetName());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
			}
		}
	}

	for (asUINT n = 0; n < m_funcDefs.GetLength(); n++)
	{
		asCFuncdefType *func = m_funcDefs[n];
		if (func && func->externalRefCount.get())
		{
			// If the funcdef is shared and can be moved to another module then this is not a reason to keep the module alive
			if (func->IsShared() && m_engine->FindNewOwnerForSharedType(func, this) != this)
				continue;

			if (!shuttingDown)
				return true;
			else
			{
				asCString msg;
				msg.Format(TXT_EXTRNL_REF_TO_MODULE_s, m_name.AddressOf());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_WARNING, msg.AddressOf());

				msg.Format(TXT_PREV_FUNC_IS_NAMED_s_TYPE_IS_d, m_funcDefs[n]->GetName(), m_funcDefs[n]->funcdef->GetFuncType());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
			}
		}
	}

	for (asUINT n = 0; n < m_templateInstances.GetLength(); n++)
	{
		asCObjectType *obj = m_templateInstances[n];
		if (obj && obj->externalRefCount.get())
		{
			// If the template can be moved to another module then this is not a reason to keep the module alive
			if (obj->IsShared() && m_engine->FindNewOwnerForSharedType(obj, this) != this)
				continue;

			if (!shuttingDown)
				return true;
			else
			{
				asCString msg;
				msg.Format(TXT_EXTRNL_REF_TO_MODULE_s, m_name.AddressOf());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_WARNING, msg.AddressOf());

				msg.Format(TXT_PREV_TYPE_IS_NAMED_s, m_templateInstances[n]->GetName());
				m_engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
			}
		}
	}

	return false;
}

// internal
void asCModule::InternalReset()
{
	CallExit();

	asUINT n;

	// Remove all global functions
	m_globalFunctions.Clear();

	// Destroy the internals of the global properties here, but do not yet remove them from the
	// engine, because functions need the engine's varAddressMap to get to the property. If the
	// property is removed already, it may leak as the refCount doesn't reach 0.
	asCSymbolTableIterator<asCGlobalProperty> globIt = m_scriptGlobals.List();
	while( globIt )
	{
		(*globIt)->DestroyInternal();
		globIt++;
	}

	UnbindAllImportedFunctions();

	// Free bind information
	for( n = 0; n < m_bindInformations.GetLength(); n++ )
	{
		if( m_bindInformations[n] )
		{
			m_bindInformations[n]->importedFunctionSignature->ReleaseInternal();

			asDELETE(m_bindInformations[n], sBindInfo);
		}
	}
	m_bindInformations.SetLength(0);

	// Free declared types, including classes, typedefs, and enums
	for( n = 0; n < m_templateInstances.GetLength(); n++ )
	{
		asCObjectType *type = m_templateInstances[n];
		if( m_engine->FindNewOwnerForSharedType(type, this) != this )
		{
			// The type is owned by another module, just release our reference
			type->ReleaseInternal();
			continue;
		}

		// Orphan the template instance
		type->module = 0;

		// No other module is holding the template type
		m_engine->RemoveTemplateInstanceType(type);
		type->ReleaseInternal();
	}
	m_templateInstances.SetLength(0);
	for( n = 0; n < m_classTypes.GetLength(); n++ )
	{
		asCObjectType *type = m_classTypes[n];
		if( type->IsShared() )
		{
			// The type is shared, so transfer ownership to another module that also uses it
			if( m_engine->FindNewOwnerForSharedType(type, this) != this )
			{
				// The type is owned by another module, just release our reference
				type->ReleaseInternal();
				continue;
			}
		}

		// The type should be destroyed now
		type->DestroyInternal();

		// Remove the type from the engine
		if( type->IsShared() )
		{
			m_engine->sharedScriptTypes.RemoveValue(type);
			type->ReleaseInternal();
		}

		// Release it from the module
		type->module = 0;
		type->ReleaseInternal();
	}
	m_classTypes.SetLength(0);
	for( n = 0; n < m_enumTypes.GetLength(); n++ )
	{
		asCEnumType *type = m_enumTypes[n];
		if( type->IsShared() )
		{
			// The type is shared, so transfer ownership to another module that also uses it
			if( m_engine->FindNewOwnerForSharedType(type, this) != this )
			{
				// The type is owned by another module, just release our reference
				type->ReleaseInternal();
				continue;
			}
		}

		// Remove the type from the engine
		if( type->IsShared() )
		{
			m_engine->sharedScriptTypes.RemoveValue(type);
			type->ReleaseInternal();
		}

		// Release it from the module
		type->module = 0;
		type->ReleaseInternal();
	}
	m_enumTypes.SetLength(0);
	for( n = 0; n < m_typeDefs.GetLength(); n++ )
	{
		asCTypedefType *type = m_typeDefs[n];

		// The type should be destroyed now
		type->DestroyInternal();

		// Release it from the module
		type->module = 0;
		type->ReleaseInternal();
	}
	m_typeDefs.SetLength(0);

	// Free funcdefs
	for( n = 0; n < m_funcDefs.GetLength(); n++ )
	{
		asCFuncdefType *func = m_funcDefs[n];
		asASSERT(func);
		if( func->funcdef && func->funcdef->IsShared() )
		{
			// The funcdef is shared, so transfer ownership to another module that also uses it
			if( m_engine->FindNewOwnerForSharedType(func, this) != this )
			{
				// The funcdef is owned by another module, just release our reference
				func->ReleaseInternal();
				continue;
			}
		}

		func->DestroyInternal();
		m_engine->RemoveFuncdef(func);
		func->module = 0;
		func->ReleaseInternal();
	}
	m_funcDefs.SetLength(0);

	// Then release the functions
	for( n = 0; n < m_scriptFunctions.GetLength(); n++ )
	{
		asCScriptFunction *func = m_scriptFunctions[n];
		if( func->IsShared() )
		{
			// The func is shared, so transfer ownership to another module that also uses it
			if( m_engine->FindNewOwnerForSharedFunc(func, this) != this )
			{
				// The func is owned by another module, just release our reference
				func->ReleaseInternal();
				continue;
			}
		}

		func->DestroyInternal();
		func->module = 0;
		func->ReleaseInternal();
	}
	m_scriptFunctions.SetLength(0);

	// Now remove and release the global properties as there are no more references to them
	globIt = m_scriptGlobals.List();
	while( globIt )
	{
		m_engine->RemoveGlobalProperty(*globIt);
		asASSERT( (*globIt)->refCount.get() == 1 );
		(*globIt)->Release();
		globIt++;
	}
	m_scriptGlobals.Clear();

	// Clear the type lookup
	// The references were already released as the types were removed from the respective arrays
	m_typeLookup.EraseAll();

	asASSERT( IsEmpty() );
}

// interface
asIScriptFunction *asCModule::GetFunctionByName(const char *in_name) const
{
	asCString name;
	asSNameSpace *ns = 0;
	if( m_engine->DetermineNameAndNamespace(in_name, m_defaultNamespace, name, ns) < 0 )
		return 0;
	
	// Search recursively in the given namespace, moving up to parent namespace until the function is found
	while( ns )
	{
		const asCArray<unsigned int> &idxs = m_globalFunctions.GetIndexes(ns, name);
		if( idxs.GetLength() != 1 )
			return 0;

		const asIScriptFunction *func = m_globalFunctions.Get(idxs[0]);
		if( func )
			return const_cast<asIScriptFunction*>(func);

		// Recursively search parent namespaces
		ns = m_engine->GetParentNameSpace(ns);
	}

	return 0;
}

// interface
asUINT asCModule::GetImportedFunctionCount() const
{
	return (asUINT)m_bindInformations.GetLength();
}

// interface
int asCModule::GetImportedFunctionIndexByDecl(const char *decl) const
{
	asCBuilder bld(m_engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(m_engine, const_cast<asCModule*>(this), asFUNC_DUMMY);
	bld.ParseFunctionDeclaration(0, decl, &func, false, 0, 0, m_defaultNamespace);

	// TODO: optimize: Improve linear search
	// Search script functions for matching interface
	int id = -1;
	for( asUINT n = 0; n < m_bindInformations.GetLength(); ++n )
	{
		if( func.name == m_bindInformations[n]->importedFunctionSignature->name &&
			func.returnType == m_bindInformations[n]->importedFunctionSignature->returnType &&
			func.parameterTypes.GetLength() == m_bindInformations[n]->importedFunctionSignature->parameterTypes.GetLength() )
		{
			bool match = true;
			for( asUINT p = 0; p < func.parameterTypes.GetLength(); ++p )
			{
				if( func.parameterTypes[p] != m_bindInformations[n]->importedFunctionSignature->parameterTypes[p] )
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
	return (asUINT)m_globalFunctions.GetSize();
}

// interface
asIScriptFunction *asCModule::GetFunctionByDecl(const char *decl) const
{
	asCBuilder bld(m_engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(m_engine, const_cast<asCModule*>(this), asFUNC_DUMMY);
	int r = bld.ParseFunctionDeclaration(0, decl, &func, false, 0, 0, m_defaultNamespace);
	if( r < 0 )
	{
		// Invalid declaration
		// TODO: Write error to message stream
		return 0;
	}

	// Use the defaultNamespace implicitly unless an explicit namespace has been provided
	asSNameSpace *ns = func.nameSpace == m_engine->nameSpaces[0] ? m_defaultNamespace : func.nameSpace;

	// Search script functions for matching interface
	while( ns )
	{
		asIScriptFunction *f = 0;
		const asCArray<unsigned int> &idxs = m_globalFunctions.GetIndexes(ns, func.name);
		for( unsigned int n = 0; n < idxs.GetLength(); n++ )
		{
			const asCScriptFunction *funcPtr = m_globalFunctions.Get(idxs[n]);
			if( funcPtr->objectType == 0 &&
				func.returnType                 == funcPtr->returnType &&
				func.parameterTypes.GetLength() == funcPtr->parameterTypes.GetLength()
				)
			{
				bool match = true;
				for( asUINT p = 0; p < func.parameterTypes.GetLength(); ++p )
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

		if( f )
			return f;
		else
		{
			// Search for matching functions in the parent namespace
			ns = m_engine->GetParentNameSpace(ns);
		}
	}

	return 0;
}

// interface
asUINT asCModule::GetGlobalVarCount() const
{
	return (asUINT)m_scriptGlobals.GetSize();
}

// interface
int asCModule::GetGlobalVarIndexByName(const char *in_name) const
{
	asCString name;
	asSNameSpace *ns = 0;
	if( m_engine->DetermineNameAndNamespace(in_name, m_defaultNamespace, name, ns) < 0 )
		return asINVALID_ARG;
	
	// Find the global var id
	while( ns )
	{
		int id = m_scriptGlobals.GetFirstIndex(ns, name);
		if( id >= 0 ) return id;

		// Recursively search parent namespaces
		ns = m_engine->GetParentNameSpace(ns);
	}

	return asNO_GLOBAL_VAR;
}

// interface
int asCModule::RemoveGlobalVar(asUINT index)
{
	asCGlobalProperty *prop = m_scriptGlobals.Get(index);
	if( !prop )
		return asINVALID_ARG;

	// If the global variables have already been initialized 
	// then uninitialize the variable before it is removed
	if (m_isGlobalVarInitialized)
		UninitializeGlobalProp(prop);

	// Destroy the internal of the global variable (removes the initialization function)
	prop->DestroyInternal();

	// Check if the module is the only one referring to the property, if so remove it from the engine too
	// If the property is not removed now, it will be removed later when the module is discarded
	if( prop->refCount.get() == 2 )
		m_engine->RemoveGlobalProperty(prop);

	// Remove the global variable from the module
	m_scriptGlobals.Erase(index);
	prop->Release();

	return 0;
}

// interface
int asCModule::GetGlobalVarIndexByDecl(const char *decl) const
{
	asCBuilder bld(m_engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCString declName;
	asSNameSpace *nameSpace;
	asCDataType dt;
	int r = bld.ParseVariableDeclaration(decl, m_defaultNamespace, declName, nameSpace, dt);
	if( r < 0 )
		return r;

	// Search global variables for a match
	while( nameSpace )
	{
		int id = m_scriptGlobals.GetFirstIndex(nameSpace, declName, asCCompGlobPropType(dt));
		if( id != -1 )
			return id;

		// Recursively search parent namespace
		nameSpace = m_engine->GetParentNameSpace(nameSpace);
	}

	return asNO_GLOBAL_VAR;
}

// interface
void *asCModule::GetAddressOfGlobalVar(asUINT index)
{
	asCGlobalProperty *prop = m_scriptGlobals.Get(index);
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
	const asCGlobalProperty *prop = m_scriptGlobals.Get(index);
	if (!prop) return 0;

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = prop->type.Format(m_defaultNamespace);
	*tempString += " ";
	if( includeNamespace && prop->nameSpace->name != "" )
		*tempString += prop->nameSpace->name + "::";
	*tempString += prop->name;

	return tempString->AddressOf();
}

// interface
int asCModule::GetGlobalVar(asUINT index, const char **out_name, const char **out_nameSpace, int *out_typeId, bool *out_isConst) const
{
	const asCGlobalProperty *prop = m_scriptGlobals.Get(index);
	if (!prop) return asINVALID_ARG;

	if( out_name )
		*out_name = prop->name.AddressOf();
	if( out_nameSpace )
		*out_nameSpace = prop->nameSpace->name.AddressOf();
	if( out_typeId )
		*out_typeId = m_engine->GetTypeIdFromDataType(prop->type);
	if( out_isConst )
		*out_isConst = prop->type.IsReadOnly();

	return asSUCCESS;
}

// interface
asUINT asCModule::GetObjectTypeCount() const
{
	return (asUINT)m_classTypes.GetLength();
}

// interface
asITypeInfo *asCModule::GetObjectTypeByIndex(asUINT index) const
{
	if( index >= m_classTypes.GetLength() )
		return 0;

	return m_classTypes[index];
}

// interface
asITypeInfo *asCModule::GetTypeInfoByName(const char *in_name) const
{
	asCString name;
	asSNameSpace *ns = 0;
	if( m_engine->DetermineNameAndNamespace(in_name, m_defaultNamespace, name, ns) < 0 )
		return 0;
		
	while (ns)
	{
		asITypeInfo* info = GetType(name, ns);
		if(info)
		{
			return info;
		}

		// Recursively search parent namespace
		ns = m_engine->GetParentNameSpace(ns);
	}

	return 0;
}

// interface
int asCModule::GetTypeIdByDecl(const char *decl) const
{
	asCDataType dt;

	// This const cast is safe since we know the engine won't be modified
	asCBuilder bld(m_engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	int r = bld.ParseDataType(decl, &dt, m_defaultNamespace);
	if( r < 0 )
		return asINVALID_TYPE;

	return m_engine->GetTypeIdFromDataType(dt);
}

// interface
asITypeInfo *asCModule::GetTypeInfoByDecl(const char *decl) const
{
	asCDataType dt;

	// This const cast is safe since we know the engine won't be modified
	asCBuilder bld(m_engine, const_cast<asCModule*>(this));

	// Don't write parser errors to the message callback
	bld.silent = true;

	int r = bld.ParseDataType(decl, &dt, m_defaultNamespace);
	if (r < 0)
		return 0;

	return dt.GetTypeInfo();
}

// interface
asUINT asCModule::GetEnumCount() const
{
	return m_enumTypes.GetLength();
}

// interface
asITypeInfo *asCModule::GetEnumByIndex(asUINT index) const
{
	if( index >= m_enumTypes.GetLength() )
		return 0;

	return m_enumTypes[index];
}

// interface
asUINT asCModule::GetTypedefCount() const
{
	return (asUINT)m_typeDefs.GetLength();
}

// interface
asITypeInfo *asCModule::GetTypedefByIndex(asUINT index) const
{
	if( index >= m_typeDefs.GetLength() )
		return 0;

	return m_typeDefs[index];
}

// internal
int asCModule::GetNextImportedFunctionId()
{
	// TODO: multithread: This will break if one thread if freeing a module, while another is being compiled
	if( m_engine->freeImportedFunctionIdxs.GetLength() )
		return FUNC_IMPORTED | (asUINT)m_engine->freeImportedFunctionIdxs[m_engine->freeImportedFunctionIdxs.GetLength()-1];

	return FUNC_IMPORTED | (asUINT)m_engine->importedFunctions.GetLength();
}

#ifndef AS_NO_COMPILER
// internal
int asCModule::AddScriptFunction(int sectionIdx, int declaredAt, int id, const asCString &funcName, const asCDataType &returnType, const asCArray<asCDataType> &params, const asCArray<asCString> &paramNames, const asCArray<asETypeModifiers> &inOutFlags, const asCArray<asCString *> &defaultArgs, bool isInterface, asCObjectType *objType, bool isGlobalFunction, asSFunctionTraits funcTraits, asSNameSpace *ns)
{
	asASSERT(id >= 0);

	// Store the function information
	asCScriptFunction *func = asNEW(asCScriptFunction)(m_engine, this, isInterface ? asFUNC_INTERFACE : asFUNC_SCRIPT);
	if( func == 0 )
	{
		// Free the default args
		for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
			if( defaultArgs[n] )
				asDELETE(defaultArgs[n], asCString);

		return asOUT_OF_MEMORY;
	}

	if( ns == 0 )
		ns = m_engine->nameSpaces[0];

	// All methods of shared objects are also shared
	if( objType && objType->IsShared() )
		funcTraits.SetTrait(asTRAIT_SHARED, true);

	func->name             = funcName;
	func->nameSpace        = ns;
	func->id               = id;
	func->returnType       = returnType;
	if( func->funcType == asFUNC_SCRIPT )
	{
		func->scriptData->scriptSectionIdx = sectionIdx;
		func->scriptData->declaredAt = declaredAt;
	}
	func->parameterTypes   = params;
	func->parameterNames   = paramNames;
	func->inOutFlags       = inOutFlags;
	func->defaultArgs      = defaultArgs;
	func->objectType       = objType;
	if( objType )
		objType->AddRefInternal();
	func->traits           = funcTraits;

	asASSERT( params.GetLength() == inOutFlags.GetLength() && params.GetLength() == defaultArgs.GetLength() );

	// Verify that we are not assigning either the final or override specifier(s) if we are registering a non-member function
	asASSERT( !(!objType && funcTraits.GetTrait(asTRAIT_FINAL)) );
	asASSERT( !(!objType && funcTraits.GetTrait(asTRAIT_OVERRIDE)) );

	// The internal ref count was already set by the constructor
	m_scriptFunctions.PushLast(func);
	m_engine->AddScriptFunction(func);

	// Compute the signature id
	if( objType )
		func->ComputeSignatureId();

	// Add reference
	if( isGlobalFunction )
		m_globalFunctions.Put(func);

	return 0;
}

// internal
int asCModule::AddScriptFunction(asCScriptFunction *func)
{
	m_scriptFunctions.PushLast(func);
	func->AddRefInternal();
	m_engine->AddScriptFunction(func);

	// If the function that is being added is an already compiled shared function
	// then it is necessary to look for anonymous functions that may be declared
	// within it and add those as well
	if( func->IsShared() && func->funcType == asFUNC_SCRIPT )
	{
		// Loop through the byte code and check all the
		// asBC_FuncPtr instructions for anonymous functions
		asDWORD *bc = func->scriptData->byteCode.AddressOf();
		asUINT bcLength = (asUINT)func->scriptData->byteCode.GetLength();
		for( asUINT n = 0; n < bcLength; )
		{
			int c = *(asBYTE*)&bc[n];
			if( c == asBC_FuncPtr )
			{
				asCScriptFunction *f = reinterpret_cast<asCScriptFunction*>(asBC_PTRARG(&bc[n]));
				// Anonymous functions start with $
				// There are never two equal anonymous functions so it is not necessary to look for duplicates
				if( f && f->name[0] == '$' )
				{
					AddScriptFunction(f);
					m_globalFunctions.Put(f);
				}
			}
			n += asBCTypeSize[asBCInfo[c].type];
		}
	}

	return 0;
}

// internal
int asCModule::AddImportedFunction(int id, const asCString &funcName, const asCDataType &returnType, const asCArray<asCDataType> &params, const asCArray<asETypeModifiers> &inOutFlags, const asCArray<asCString *> &defaultArgs, asSFunctionTraits funcTraits, asSNameSpace *ns, const asCString &moduleName)
{
	asASSERT(id >= 0);

	// Store the function information
	asCScriptFunction *func = asNEW(asCScriptFunction)(m_engine, this, asFUNC_IMPORTED);
	if( func == 0 )
	{
		// Free the default args
		for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
			if( defaultArgs[n] )
				asDELETE(defaultArgs[n], asCString);

		return asOUT_OF_MEMORY;
	}

	func->name           = funcName;
	func->id             = id;
	func->returnType     = returnType;
	func->nameSpace      = ns;
	func->parameterTypes = params;
	func->inOutFlags     = inOutFlags;
	func->defaultArgs    = defaultArgs;
	func->objectType     = 0;
	func->traits         = funcTraits;

	sBindInfo *info = asNEW(sBindInfo);
	if( info == 0 )
	{
		asDELETE(func, asCScriptFunction);
		return asOUT_OF_MEMORY;
	}

	info->importedFunctionSignature = func;
	info->boundFunctionId           = -1;
	info->importFromModule          = moduleName;
	m_bindInformations.PushLast(info);

	// Add the info to the array in the engine
	if( m_engine->freeImportedFunctionIdxs.GetLength() )
		m_engine->importedFunctions[m_engine->freeImportedFunctionIdxs.PopLast()] = info;
	else
		m_engine->importedFunctions.PushLast(info);

	return 0;
}
#endif

// internal
asCScriptFunction *asCModule::GetImportedFunction(int index) const
{
	return m_bindInformations[index]->importedFunctionSignature;
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

	asCScriptFunction *src = m_engine->GetScriptFunction(func->GetId());
	if( src == 0 )
		return asNO_FUNCTION;

	// Verify return type
	if( dst->returnType != src->returnType )
		return asINVALID_INTERFACE;

	if( dst->parameterTypes.GetLength() != src->parameterTypes.GetLength() )
		return asINVALID_INTERFACE;

	for( asUINT n = 0; n < dst->parameterTypes.GetLength(); ++n )
	{
		if( dst->parameterTypes[n] != src->parameterTypes[n] )
			return asINVALID_INTERFACE;
	}

	m_bindInformations[index]->boundFunctionId = src->GetId();
	src->AddRefInternal();

	return asSUCCESS;
}

// interface
int asCModule::UnbindImportedFunction(asUINT index)
{
	if( index >= m_bindInformations.GetLength() )
		return asINVALID_ARG;

	// Remove reference to old module
	if( m_bindInformations[index] )
	{
		int oldFuncID = m_bindInformations[index]->boundFunctionId;
		if( oldFuncID != -1 )
		{
			m_bindInformations[index]->boundFunctionId = -1;
			m_engine->scriptFunctions[oldFuncID]->ReleaseInternal();
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
	// TODO: Allow the application to decide if the parameter name should be included or not (requires change in the interface)
	*tempString = func->GetDeclarationStr(true, true, false);

	return tempString->AddressOf();
}

// interface
const char *asCModule::GetImportedFunctionSourceModule(asUINT index) const
{
	if( index >= m_bindInformations.GetLength() )
		return 0;

	return m_bindInformations[index]->importFromModule.AddressOf();
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

		asCString str = importFunc->GetDeclarationStr(false, true);

		// Get module name from where the function should be imported
		const char *moduleName = GetImportedFunctionSourceModule(n);
		if( moduleName == 0 ) return asERROR;

		asCModule *srcMod = m_engine->GetModule(moduleName, false);
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
void asCModule::AddClassType(asCObjectType* type)
{
	m_classTypes.PushLast(type);
	m_typeLookup.Insert(asSNameSpaceNamePair(type->nameSpace, type->name), type);
}

// internal
void asCModule::AddEnumType(asCEnumType* type)
{
	m_enumTypes.PushLast(type);
	m_typeLookup.Insert(asSNameSpaceNamePair(type->nameSpace, type->name), type);
}

// internal
void asCModule::AddTypeDef(asCTypedefType* type)
{
	m_typeDefs.PushLast(type);
	m_typeLookup.Insert(asSNameSpaceNamePair(type->nameSpace, type->name), type);
}

// internal
void asCModule::AddFuncDef(asCFuncdefType* type)
{
	m_funcDefs.PushLast(type);
	m_typeLookup.Insert(asSNameSpaceNamePair(type->nameSpace, type->name), type);
}

// internal
void asCModule::ReplaceFuncDef(asCFuncdefType* type, asCFuncdefType* newType)
{
	int i = m_funcDefs.IndexOf(type);
	if( i >= 0 )
	{
		m_funcDefs[i] = newType;
		
		// Replace it in the lookup map too
		asSMapNode<asSNameSpaceNamePair, asCTypeInfo*>* result = 0;
		if(m_typeLookup.MoveTo(&result, asSNameSpaceNamePair(type->nameSpace, type->name)))
		{
			asASSERT( result->value == type );
			result->value = newType;
		}
	}
}

// internal
asCTypeInfo *asCModule::GetType(const asCString &type, asSNameSpace *ns) const
{
	asSMapNode<asSNameSpaceNamePair, asCTypeInfo*>* result = 0;
	if(m_typeLookup.MoveTo(&result, asSNameSpaceNamePair(ns, type)))
	{
		return result->value;
	}
	return 0;
}

// internal
asCObjectType *asCModule::GetObjectType(const char *type, asSNameSpace *ns) const
{
	asSMapNode<asSNameSpaceNamePair, asCTypeInfo*>* result = 0;
	if(m_typeLookup.MoveTo(&result, asSNameSpaceNamePair(ns, type)))
	{
		return CastToObjectType(result->value);
	}
 
	return 0;
}

// internal
asCGlobalProperty *asCModule::AllocateGlobalProperty(const char *propName, const asCDataType &dt, asSNameSpace *ns)
{
	asCGlobalProperty *prop = m_engine->AllocateGlobalProperty();
	prop->name = propName;
	prop->nameSpace = ns;

	// Allocate the memory for this property based on its type
	prop->type = dt;
	prop->AllocateMemory();

	// Make an entry in the address to variable map
	m_engine->varAddressMap.Insert(prop->GetAddressOfValue(), prop);

	// Store the variable in the module scope
	m_scriptGlobals.Put(prop);
	prop->AddRef();

	return prop;
}

// internal
bool asCModule::IsEmpty() const
{
	if( m_scriptFunctions.GetLength()  ) return false;
	if( m_globalFunctions.GetSize()    ) return false;
	if( m_bindInformations.GetLength() ) return false;
	if( m_scriptGlobals.GetSize()      ) return false;
	if( m_classTypes.GetLength()       ) return false;
	if( m_enumTypes.GetLength()        ) return false;
	if( m_typeDefs.GetLength()         ) return false;
	if( m_funcDefs.GetLength()         ) return false;

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

	asCWriter write(const_cast<asCModule*>(this), out, m_engine, stripDebugInfo);
	return write.Write();
#endif
}

// interface
int asCModule::LoadByteCode(asIBinaryStream *in, bool *wasDebugInfoStripped)
{
	if( in == 0 ) return asINVALID_ARG;

	// Don't allow the module to be rebuilt if there are still
	// external references that will need the previous code
	if( HasExternalReferences(false) )
	{
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_MODULE_IS_IN_USE);
		return asMODULE_IS_IN_USE;
	}

	// Only permit loading bytecode if no other thread is currently compiling
	// TODO: It should be possible to have multiple threads perform compilations
	int r = m_engine->RequestBuild();
	if( r < 0 )
		return r;

	asCReader read(this, in, m_engine);
	r = read.Read(wasDebugInfoStripped);
	if (r < 0)
	{
		m_engine->BuildCompleted();
		return r;
	}

	JITCompile();

#ifdef AS_DEBUG
	// Verify that there are no unwanted gaps in the scriptFunctions array.
	for( asUINT n = 1; n < m_engine->scriptFunctions.GetLength(); n++ )
	{
		int id = n;
		if( m_engine->scriptFunctions[n] == 0 && !m_engine->freeScriptFunctionIds.Exists(id) )
			asASSERT( false );
	}
#endif

	m_engine->BuildCompleted();

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
	int r = m_engine->RequestBuild();
	if( r < 0 )
		return r;

	// Prepare the engine
	m_engine->PrepareEngine();
	if( m_engine->configFailed )
	{
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_INVALID_CONFIGURATION);
		m_engine->BuildCompleted();
		return asINVALID_CONFIGURATION;
	}

	// Compile the global variable and add it to the module scope
	asCBuilder varBuilder(m_engine, this);
	asCString str = code;
	r = varBuilder.CompileGlobalVar(sectionName, str.AddressOf(), lineOffset);

	m_engine->BuildCompleted();

	// Initialize the variable
	if( r >= 0 )
	{
		// Clear the memory
		asCGlobalProperty *prop = m_scriptGlobals.GetLast();
		if( prop )
		{
			memset(prop->GetAddressOfValue(), 0, sizeof(asDWORD)*prop->type.GetSizeOnStackDWords());
		}

		if( prop && m_engine->ep.initGlobalVarsAfterBuild )
		{
			// Flag that there are initialized global variables
			m_isGlobalVarInitialized = true;

			r = InitGlobalProp(prop, 0);
		}
	}

	return r;
#endif
}

// interface
int asCModule::CompileFunction(const char* sectionName, const char* code, int lineOffset, asDWORD compileFlags, asIScriptFunction** outFunc)
{
	// Make sure the outFunc is null if the function fails, so the
	// application doesn't attempt to release a non-existent function
	if (outFunc)
		*outFunc = 0;

#ifdef AS_NO_COMPILER
	UNUSED_VAR(sectionName);
	UNUSED_VAR(code);
	UNUSED_VAR(lineOffset);
	UNUSED_VAR(compileFlags);
	return asNOT_SUPPORTED;
#else
	// Validate arguments
	if (code == 0 ||
		(compileFlags != 0 && compileFlags != asCOMP_ADD_TO_MODULE))
		return asINVALID_ARG;

	// Only one thread may build at one time
	// TODO: It should be possible to have multiple threads perform compilations
	int r = m_engine->RequestBuild();
	if (r < 0)
		return r;

	// Prepare the engine
	m_engine->PrepareEngine();
	if (m_engine->configFailed)
	{
		m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_INVALID_CONFIGURATION);
		m_engine->BuildCompleted();
		return asINVALID_CONFIGURATION;
	}

	// Compile the single function
	asCBuilder funcBuilder(m_engine, this);
	asCString str = code;
	asCScriptFunction* func = 0;
	r = funcBuilder.CompileFunction(sectionName, str.AddressOf(), lineOffset, compileFlags, &func);

	if (r >= 0)
	{
		// Invoke the JIT compiler if it has been set
		asIJITCompiler* jit = m_engine->GetJITCompiler();
		if (jit)
		{
			func->JITCompile();
		}
	}

	m_engine->BuildCompleted();

	if( r >= 0 && outFunc && func )
	{
		// Return the function to the caller and add an external reference
		*outFunc = func;
		func->AddRef();
	}

	// Release our reference to the function
	if( func )
		func->ReleaseInternal();

	return r;
#endif
}

// interface
int asCModule::RemoveFunction(asIScriptFunction *func)
{
	// Find the global function
	asCScriptFunction *f = static_cast<asCScriptFunction*>(func);
	int idx = m_globalFunctions.GetIndex(f);
	if( idx >= 0 )
	{
		m_globalFunctions.Erase(idx);
		m_scriptFunctions.RemoveValue(f);
		f->ReleaseInternal();
		return 0;
	}

	return asNO_FUNCTION;
}

#ifndef AS_NO_COMPILER
// internal
int asCModule::AddFuncDef(const asCString &funcName, asSNameSpace *ns, asCObjectType *parent)
{
	// namespace and parent are mutually exclusive
	asASSERT((ns == 0 && parent) || (ns && parent == 0));

	asCScriptFunction *func = asNEW(asCScriptFunction)(m_engine, 0, asFUNC_FUNCDEF);
	if (func == 0)
		return asOUT_OF_MEMORY;

	func->name      = funcName;
	func->nameSpace = ns;
	func->module    = this;

	asCFuncdefType *fdt = asNEW(asCFuncdefType)(m_engine, func);
	AddFuncDef(fdt); // The constructor set the refcount to 1

	m_engine->funcDefs.PushLast(fdt); // doesn't increase refcount
	func->id = m_engine->GetNextScriptFunctionId();
	m_engine->AddScriptFunction(func);

	if (parent)
	{
		parent->childFuncDefs.PushLast(fdt);
		fdt->parentClass = parent;
	}

	return (int)m_funcDefs.GetLength()-1;
}
#endif

// interface
asDWORD asCModule::SetAccessMask(asDWORD mask)
{
	asDWORD old = m_accessMask;
	m_accessMask = mask;
	return old;
}

END_AS_NAMESPACE

