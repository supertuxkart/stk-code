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
// as_restore.cpp
//
// Functions for saving and restoring module bytecode
// asCRestore was originally written by Dennis Bollyn, dennis@gyrbo.be

#include "as_config.h"
#include "as_restore.h"
#include "as_bytecode.h"
#include "as_scriptobject.h"
#include "as_texts.h"
#include "as_debug.h"

BEGIN_AS_NAMESPACE

// Macros for doing endianess agnostic bitmask serialization
#define SAVE_TO_BIT(dst, val, bit) ((dst) |= ((val) << (bit)))
#define LOAD_FROM_BIT(dst, val, bit) ((dst) = ((val) >> (bit)) & 1)

asCReader::asCReader(asCModule* _module, asIBinaryStream* _stream, asCScriptEngine* _engine)
 : module(_module), stream(_stream), engine(_engine)
{
	error = false;
	bytesRead = 0;
}

int asCReader::ReadData(void *data, asUINT size)
{
	asASSERT(size == 1 || size == 2 || size == 4 || size == 8);
	int ret = 0;
#if defined(AS_BIG_ENDIAN)
	for( asUINT n = 0; ret >= 0 && n < size; n++ )
		ret = stream->Read(((asBYTE*)data)+n, 1);
#else
	for( int n = size-1; ret >= 0 && n >= 0; n-- )
		ret = stream->Read(((asBYTE*)data)+n, 1);
#endif
	if (ret < 0)
		Error(TXT_UNEXPECTED_END_OF_FILE);
	bytesRead += size;
	return ret;
}

int asCReader::Read(bool *wasDebugInfoStripped)
{
	TimeIt("asCReader::Read");

	// Before starting the load, make sure that
	// any existing resources have been freed
	module->InternalReset();

	// Call the inner method to do the actual loading
	int r = ReadInner();
	if( r < 0 )
	{
		// Something went wrong while loading the bytecode, so we need
		// to clean-up whatever has been created during the process.

		// Make sure none of the loaded functions attempt to release
		// references that have not yet been increased
		asUINT i;
		for( i = 0; i < module->m_scriptFunctions.GetLength(); i++ )
			if( !dontTranslate.MoveTo(0, module->m_scriptFunctions[i]) )
				if( module->m_scriptFunctions[i]->scriptData )
					module->m_scriptFunctions[i]->scriptData->byteCode.SetLength(0);

		asCSymbolTable<asCGlobalProperty>::iterator it = module->m_scriptGlobals.List();
		for( ; it; it++ )
			if( (*it)->GetInitFunc() )
				if( (*it)->GetInitFunc()->scriptData )
					(*it)->GetInitFunc()->scriptData->byteCode.SetLength(0);

		module->InternalReset();
	}
	else
	{
		// Init system functions properly
		engine->PrepareEngine();

		// Initialize the global variables (unless requested not to)
		if( engine->ep.initGlobalVarsAfterBuild )
			r = module->ResetGlobalVars(0);

		if( wasDebugInfoStripped )
			*wasDebugInfoStripped = noDebugInfo;
	}

	// Clean up the loaded string constants
	for (asUINT n = 0; n < usedStringConstants.GetLength(); n++)
		engine->stringFactory->ReleaseStringConstant(usedStringConstants[n]);
	usedStringConstants.SetLength(0);

	return r;
}

int asCReader::Error(const char *msg)
{
	// Don't write if it has already been reported an error earlier
	if( !error )
	{
		asCString str;
		str.Format(msg, bytesRead);
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		error = true;
	}

	return asERROR;
}

int asCReader::ReadInner()
{
	TimeIt("asCReader::ReadInner");

	// This function will load each entity one by one from the stream.
	// If any error occurs, it will return to the caller who is
	// responsible for cleaning up the partially loaded entities.

	engine->deferValidationOfTemplateTypes = true;

	unsigned long i, count;
	asCScriptFunction* func;

	// Read the flag as 1 byte even on platforms with 4byte booleans
	noDebugInfo = ReadEncodedUInt() ? VALUE_OF_BOOLEAN_TRUE : 0;

	// Read enums
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	module->m_enumTypes.Allocate(count, false);
	for( i = 0; i < count && !error; i++ )
	{
		asCEnumType *et = asNEW(asCEnumType)(engine);
		if( et == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		bool isExternal = false;
		ReadTypeDeclaration(et, 1, &isExternal);

		// If the type is shared then we should use the original if it exists
		bool sharedExists = false;
		if( et->IsShared() )
		{
			for( asUINT n = 0; n < engine->sharedScriptTypes.GetLength(); n++ )
			{
				asCTypeInfo *t = engine->sharedScriptTypes[n];
				if( t &&
					t->IsShared() &&
					t->name == et->name &&
					t->nameSpace == et->nameSpace &&
					(t->flags & asOBJ_ENUM) )
				{
					asDELETE(et, asCEnumType);
					et = CastToEnumType(t);
					sharedExists = true;
					break;
				}
			}
		}

		if (isExternal && !sharedExists)
		{
			asCString msg;
			msg.Format(TXT_EXTERNAL_SHARED_s_NOT_FOUND, et->name.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
			asDELETE(et, asCEnumType);
			error = true;
			return asERROR;
		}

		if( sharedExists )
		{
			existingShared.Insert(et, true);
			et->AddRefInternal();
		}
		else
		{
			if( et->IsShared() )
			{
				engine->sharedScriptTypes.PushLast(et);
				et->AddRefInternal();
			}

			// Set this module as the owner
			et->module = module;
		}
		module->AddEnumType(et);

		if (isExternal)
			module->m_externalTypes.PushLast(et);

		ReadTypeDeclaration(et, 2);
	}

	if( error ) return asERROR;

	// classTypes[]
	// First restore the structure names, then the properties
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	module->m_classTypes.Allocate(count, false);
	for( i = 0; i < count && !error; ++i )
	{
		asCObjectType *ot = asNEW(asCObjectType)(engine);
		if( ot == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		bool isExternal = false;
		ReadTypeDeclaration(ot, 1, &isExternal);

		// If the type is shared, then we should use the original if it exists
		bool sharedExists = false;
		if( ot->IsShared() )
		{
			for( asUINT n = 0; n < engine->sharedScriptTypes.GetLength(); n++ )
			{
				asCTypeInfo *ti = engine->sharedScriptTypes[n];
				asCObjectType *t = CastToObjectType(ti);
				if( t &&
					t->IsShared() &&
					t->name == ot->name &&
					t->nameSpace == ot->nameSpace &&
					t->IsInterface() == ot->IsInterface() )
				{
					asDELETE(ot, asCObjectType);
					ot = CastToObjectType(t);
					sharedExists = true;
					break;
				}
			}
		}

		if (isExternal && !sharedExists)
		{
			asCString msg;
			msg.Format(TXT_EXTERNAL_SHARED_s_NOT_FOUND, ot->name.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
			asDELETE(ot, asCObjectType);
			error = true;
			return asERROR;
		}

		if( sharedExists )
		{
			existingShared.Insert(ot, true);
			ot->AddRefInternal();
		}
		else
		{
			if( ot->IsShared() )
			{
				engine->sharedScriptTypes.PushLast(ot);
				ot->AddRefInternal();
			}

			// Set this module as the owner
			ot->module = module;
		}
		module->AddClassType(ot);

		if (isExternal)
			module->m_externalTypes.PushLast(ot);
	}

	if( error ) return asERROR;

	// Read func defs
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	module->m_funcDefs.Allocate(count, false);
	for( i = 0; i < count && !error; i++ )
	{
		bool isNew, isExternal;
		asCScriptFunction *funcDef = ReadFunction(isNew, false, true, true, &isExternal);
		if(funcDef)
		{
			funcDef->module = module;

			asCFuncdefType *fdt = funcDef->funcdefType;
			fdt->module = module;

			module->AddFuncDef(fdt);
			engine->funcDefs.PushLast(fdt);

			// TODO: clean up: This is also done by the builder. It should probably be moved to a method in the module
			// Check if there is another identical funcdef from another module and if so reuse that instead
			if(funcDef->IsShared())
			{
				for( asUINT n = 0; n < engine->funcDefs.GetLength(); n++ )
				{
					asCFuncdefType *f2 = engine->funcDefs[n];
					if( f2 == 0 || fdt == f2 )
						continue;

					if( !f2->funcdef->IsShared() )
						continue;

					if( f2->name == fdt->name &&
						f2->nameSpace == fdt->nameSpace &&
						f2->parentClass == fdt->parentClass &&
						f2->funcdef->IsSignatureExceptNameEqual(funcDef) )
					{
						// Replace our funcdef for the existing one
						module->ReplaceFuncDef(fdt, f2);
						f2->AddRefInternal();

						if (isExternal)
							module->m_externalTypes.PushLast(f2);

						engine->funcDefs.RemoveValue(fdt);

						savedFunctions[savedFunctions.IndexOf(funcDef)] = f2->funcdef;

						if (fdt->parentClass)
						{
							// The real funcdef should already be in the object
							asASSERT(fdt->parentClass->childFuncDefs.IndexOf(f2) >= 0);

							fdt->parentClass = 0;
						}

						fdt->ReleaseInternal();
						funcDef = 0;
						break;
					}
				}
			}

			// Add the funcdef to the parentClass if this is a child funcdef
			if (funcDef && fdt->parentClass)
				fdt->parentClass->childFuncDefs.PushLast(fdt);

			// Check if an external shared funcdef was really found
			if (isExternal && funcDef)
			{
				asCString msg;
				msg.Format(TXT_EXTERNAL_SHARED_s_NOT_FOUND, funcDef->name.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
				error = true;
				return asERROR;
			}
		}
		else
			Error(TXT_INVALID_BYTECODE_d);
	}

	// Read interface methods
	for( i = 0; i < module->m_classTypes.GetLength() && !error; i++ )
	{
		if( module->m_classTypes[i]->IsInterface() )
			ReadTypeDeclaration(module->m_classTypes[i], 2);
	}

	// Read class methods and behaviours
	for( i = 0; i < module->m_classTypes.GetLength() && !error; ++i )
	{
		if( !module->m_classTypes[i]->IsInterface() )
			ReadTypeDeclaration(module->m_classTypes[i], 2);
	}

	// Read class properties
	for( i = 0; i < module->m_classTypes.GetLength() && !error; ++i )
	{
		if( !module->m_classTypes[i]->IsInterface() )
			ReadTypeDeclaration(module->m_classTypes[i], 3);
	}

	if( error ) return asERROR;

	// Read typedefs
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	module->m_typeDefs.Allocate(count, false);
	for( i = 0; i < count && !error; i++ )
	{
		asCTypedefType *td = asNEW(asCTypedefType)(engine);
		if( td == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		bool isExternal = false;
		ReadTypeDeclaration(td, 1, &isExternal);
		td->module = module;
		module->AddTypeDef(td);
		ReadTypeDeclaration(td, 2);
	}

	if( error ) return asERROR;

	// scriptGlobals[]
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	if( count && engine->ep.disallowGlobalVars )
	{
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_GLOBAL_VARS_NOT_ALLOWED);
		Error(TXT_INVALID_BYTECODE_d);
	}
	module->m_scriptGlobals.Allocate(count, false);
	for( i = 0; i < count && !error; ++i )
	{
		ReadGlobalProperty();
	}

	// scriptFunctions[]
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	for( i = 0; i < count && !error; ++i )
	{
		size_t len = module->m_scriptFunctions.GetLength();
		bool isNew, isExternal;
		func = ReadFunction(isNew, true, true, true, &isExternal);
		if( func == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			break;
		}

		// Is the function shared and was it created now?
		if( func->IsShared() && len != module->m_scriptFunctions.GetLength() )
		{
			// If the function already existed in another module, then
			// we need to replace it with previously existing one
			for( asUINT n = 0; n < engine->scriptFunctions.GetLength() && !error; n++ )
			{
				asCScriptFunction *realFunc = engine->scriptFunctions[n];
				if( realFunc &&
					realFunc != func &&
					realFunc->IsShared() &&
					realFunc->nameSpace == func->nameSpace &&
					realFunc->IsSignatureEqual(func) )
				{
					// Replace the recently created function with the pre-existing function
					module->m_scriptFunctions[module->m_scriptFunctions.GetLength()-1] = realFunc;
					realFunc->AddRefInternal();
					savedFunctions[savedFunctions.GetLength()-1] = realFunc;
					engine->RemoveScriptFunction(func);

					// Insert the function in the dontTranslate array
					dontTranslate.Insert(realFunc, true);

					if (isExternal)
						module->m_externalFunctions.PushLast(realFunc);

					// Release the function, but make sure nothing else is released
					func->id = 0;
					if( func->scriptData )
						func->scriptData->byteCode.SetLength(0);
					func->ReleaseInternal();
					func = 0;
					break;
				}
			}
		}

		// Check if an external shared func was really found
		if (isExternal && func)
		{
			asCString msg;
			msg.Format(TXT_EXTERNAL_SHARED_s_NOT_FOUND, func->name.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
			error = true;
			return asERROR;
		}
	}

	// globalFunctions[]
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	for( i = 0; i < count && !error; ++i )
	{
		bool isNew;
		func = ReadFunction(isNew, false, false);
		if( func )
		{
			// All the global functions were already loaded while loading the scriptFunctions, here
			// we're just re-reading the references to know which goes into the globalFunctions array
			asASSERT( !isNew );

			module->m_globalFunctions.Put(func);
		}
		else
			Error(TXT_INVALID_BYTECODE_d);
	}

	if( error ) return asERROR;

	// bindInformations[]
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	module->m_bindInformations.Allocate(count, false);
	for( i = 0; i < count && !error; ++i )
	{
		sBindInfo *info = asNEW(sBindInfo);
		if( info == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		bool isNew;
		info->importedFunctionSignature = ReadFunction(isNew, false, false);
		if( info->importedFunctionSignature == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			break;
		}

		if( engine->freeImportedFunctionIdxs.GetLength() )
		{
			int id = engine->freeImportedFunctionIdxs.PopLast();
			info->importedFunctionSignature->id = int(FUNC_IMPORTED + id);
			engine->importedFunctions[id] = info;
		}
		else
		{
			info->importedFunctionSignature->id = int(FUNC_IMPORTED + engine->importedFunctions.GetLength());
			engine->importedFunctions.PushLast(info);
		}
		ReadString(&info->importFromModule);
		info->boundFunctionId = -1;
		module->m_bindInformations.PushLast(info);
	}

	if( error ) return asERROR;

	// usedTypes[]
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	usedTypes.Allocate(count, false);
	for( i = 0; i < count && !error; ++i )
	{
		asCTypeInfo *ti = ReadTypeInfo();
		usedTypes.PushLast(ti);
	}

	// usedTypeIds[]
	if( !error )
		ReadUsedTypeIds();

	// usedFunctions[]
	if( !error )
		ReadUsedFunctions();

	// usedGlobalProperties[]
	if( !error )
		ReadUsedGlobalProps();

	// usedStringConstants[]
	if( !error )
		ReadUsedStringConstants();

	// usedObjectProperties
	if( !error )
		ReadUsedObjectProps();

	// Validate the template types
	if( !error )
	{
		for( i = 0; i < usedTypes.GetLength() && !error; i++ )
		{
			asCObjectType *ot = CastToObjectType(usedTypes[i]);
			if( !ot ||
				!(ot->flags & asOBJ_TEMPLATE) ||
				!ot->beh.templateCallback )
				continue;

			bool dontGarbageCollect = false;
			asCScriptFunction *callback = engine->scriptFunctions[ot->beh.templateCallback];
			if( !engine->CallGlobalFunctionRetBool(ot, &dontGarbageCollect, callback->sysFuncIntf, callback) )
			{
				asCString sub = ot->templateSubTypes[0].Format(ot->nameSpace);
				for( asUINT n = 1; n < ot->templateSubTypes.GetLength(); n++ )
				{
					sub += ",";
					sub += ot->templateSubTypes[n].Format(ot->nameSpace);
				}
				asCString str;
				str.Format(TXT_INSTANCING_INVLD_TMPL_TYPE_s_s, ot->name.AddressOf(), sub.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				Error(TXT_INVALID_BYTECODE_d);
			}
			else
			{
				// If the callback said this template instance won't be garbage collected then remove the flag
				if( dontGarbageCollect )
					ot->flags &= ~asOBJ_GC;
			}
		}
	}
	engine->deferValidationOfTemplateTypes = false;

	if( error ) return asERROR;

	// Update the loaded bytecode to point to the correct types, property offsets,
	// function ids, etc. This is basically a linking stage.
	for( i = 0; i < module->m_scriptFunctions.GetLength() && !error; i++ )
		if( module->m_scriptFunctions[i]->funcType == asFUNC_SCRIPT )
			TranslateFunction(module->m_scriptFunctions[i]);

	asCSymbolTable<asCGlobalProperty>::iterator globIt = module->m_scriptGlobals.List();
	while( globIt && !error )
	{
		asCScriptFunction *initFunc = (*globIt)->GetInitFunc();
		if( initFunc )
			TranslateFunction(initFunc);
		globIt++;
	}

	if( error ) return asERROR;

	// Add references for all functions (except for the pre-existing shared code)
	for( i = 0; i < module->m_scriptFunctions.GetLength(); i++ )
		if( !dontTranslate.MoveTo(0, module->m_scriptFunctions[i]) )
			module->m_scriptFunctions[i]->AddReferences();

	globIt = module->m_scriptGlobals.List();
	while( globIt )
	{
		asCScriptFunction *initFunc = (*globIt)->GetInitFunc();
		if( initFunc )
			initFunc->AddReferences();
		globIt++;
	}
	return error ? asERROR : asSUCCESS;
}

void asCReader::ReadUsedStringConstants()
{
	TimeIt("asCReader::ReadUsedStringConstants");

	asCString str;

	asUINT count;
	count = SanityCheck(ReadEncodedUInt(), 1000000);

	if (count > 0 && engine->stringFactory == 0)
	{
		Error(TXT_STRINGS_NOT_RECOGNIZED);
		return;
	}

	usedStringConstants.Allocate(count, false);
	for( asUINT i = 0; i < count; ++i )
	{
		ReadString(&str);
		usedStringConstants.PushLast(const_cast<void*>(engine->stringFactory->GetStringConstant(str.AddressOf(), (asUINT)str.GetLength())));
	}
}

void asCReader::ReadUsedFunctions()
{
	TimeIt("asCReader::ReadUsedFunctions");

	asUINT count;
	count = SanityCheck(ReadEncodedUInt(), 1000000);
	usedFunctions.SetLength(count);
	if( usedFunctions.GetLength() != count )
	{
		// Out of memory
		error = true;
		return;
	}
	memset(usedFunctions.AddressOf(), 0, sizeof(asCScriptFunction *)*count);

	for( asUINT n = 0; n < usedFunctions.GetLength(); n++ )
	{
		char c;

		// Read the data to be able to uniquely identify the function

		// Is the function from the module or the application?
		ReadData(&c, 1);

		if( c == 'n' )
		{
			// Null function pointer
			usedFunctions[n] = 0;
		}
		else
		{
			asCScriptFunction func(engine, c == 'm' ? module : 0, asFUNC_DUMMY);
			asCObjectType *parentClass = 0;
			ReadFunctionSignature(&func, &parentClass);
			if( error )
			{
				func.funcType = asFUNC_DUMMY;
				return;
			}

			// Find the correct function
			if( c == 'm' )
			{
				if( func.funcType == asFUNC_IMPORTED )
				{
					for( asUINT i = 0; i < module->m_bindInformations.GetLength(); i++ )
					{
						asCScriptFunction *f = module->m_bindInformations[i]->importedFunctionSignature;
						if( func.objectType != f->objectType ||
							func.funcType != f->funcType ||
							func.nameSpace != f->nameSpace ||
							!func.IsSignatureEqual(f) )
							continue;

						usedFunctions[n] = f;
						break;
					}
				}
				else if( func.funcType == asFUNC_FUNCDEF )
				{
					const asCArray<asCFuncdefType *> &funcs = module->m_funcDefs;
					for( asUINT i = 0; i < funcs.GetLength(); i++ )
					{
						asCScriptFunction *f = funcs[i]->funcdef;
						if( f == 0 ||
							func.name != f->name ||
							!func.IsSignatureExceptNameAndObjectTypeEqual(f) ||
							funcs[i]->parentClass != parentClass )
							continue;

						asASSERT( f->objectType == 0 );

						usedFunctions[n] = f;
						break;
					}
				}
				else
				{
					// TODO: optimize: Global functions should be searched for in module->globalFunctions
					// TODO: optimize: funcdefs should be searched for in module->funcDefs
					// TODO: optimize: object methods should be searched for directly in the object type
					for( asUINT i = 0; i < module->m_scriptFunctions.GetLength(); i++ )
					{
						asCScriptFunction *f = module->m_scriptFunctions[i];
						if( func.objectType != f->objectType ||
							func.funcType != f->funcType ||
							func.nameSpace != f->nameSpace ||
							!func.IsSignatureEqual(f) )
							continue;

						usedFunctions[n] = f;
						break;
					}
				}
			}
			else if (c == 's')
			{
				// Look for shared entities in the engine, as they may not necessarily be part
				// of the scope of the module if they have been inhereted from other modules.
				if (func.funcType == asFUNC_FUNCDEF)
				{
					const asCArray<asCFuncdefType *> &funcs = engine->funcDefs;
					for (asUINT i = 0; i < funcs.GetLength(); i++)
					{
						asCScriptFunction *f = funcs[i]->funcdef;
						if (f == 0 ||
							func.name != f->name ||
							!func.IsSignatureExceptNameAndObjectTypeEqual(f) ||
							funcs[i]->parentClass != parentClass)
							continue;

						asASSERT(f->objectType == 0);

						usedFunctions[n] = f;
						break;
					}
				}
				else
				{
					for (asUINT i = 0; i < engine->scriptFunctions.GetLength(); i++)
					{
						asCScriptFunction *f = engine->scriptFunctions[i];
						if (f == 0 || !f->IsShared() ||
							func.objectType != f->objectType ||
							func.funcType != f->funcType ||
							func.nameSpace != f->nameSpace ||
							!func.IsSignatureEqual(f))
							continue;

						usedFunctions[n] = f;
						break;
					}
				}
			}
			else
			{
				asASSERT(c == 'a');

				if( func.funcType == asFUNC_FUNCDEF )
				{
					// This is a funcdef (registered or shared)
					const asCArray<asCFuncdefType *> &funcs = engine->funcDefs;
					for( asUINT i = 0; i < funcs.GetLength(); i++ )
					{
						asCScriptFunction *f = funcs[i]->funcdef;
						if( f == 0 || func.name != f->name || !func.IsSignatureExceptNameAndObjectTypeEqual(f) || funcs[i]->parentClass != parentClass )
							continue;

						asASSERT( f->objectType == 0 );

						usedFunctions[n] = f;
						break;
					}
				}
				else if( func.name[0] == '$' )
				{
					// This is a special function

					if( func.name == "$beh0" && func.objectType )
					{
						if (func.objectType->flags & asOBJ_TEMPLATE)
						{
							// Look for the matching constructor inside the factory stubs generated for the template instance
							// See asCCompiler::PerformFunctionCall
							for (asUINT i = 0; i < func.objectType->beh.constructors.GetLength(); i++)
							{
								asCScriptFunction *f = engine->scriptFunctions[func.objectType->beh.constructors[i]];
								
								// Find the id of the real constructor and not the generated stub
								asUINT id = 0;
								asDWORD *bc = f->scriptData->byteCode.AddressOf();
								while (bc)
								{
									if ((*(asBYTE*)bc) == asBC_CALLSYS)
									{
										id = asBC_INTARG(bc);
										break;
									}
									bc += asBCTypeSize[asBCInfo[*(asBYTE*)bc].type];
								}

								f = engine->scriptFunctions[id];
								if (f == 0 ||
									!func.IsSignatureExceptNameAndObjectTypeEqual(f))
									continue;

								usedFunctions[n] = f;
								break;
							}
						}

						if( usedFunctions[n] == 0 )
						{
							// This is a class constructor, so we can search directly in the object type's constructors
							for (asUINT i = 0; i < func.objectType->beh.constructors.GetLength(); i++)
							{
								asCScriptFunction *f = engine->scriptFunctions[func.objectType->beh.constructors[i]];
								if (f == 0 ||
									!func.IsSignatureExceptNameAndObjectTypeEqual(f))
									continue;

								usedFunctions[n] = f;
								break;
							}
						}
					}
					else if( func.name == "$fact" || func.name == "$beh3" )
					{
						// This is a factory (or stub), so look for the function in the return type's factories
						asCObjectType *objType = CastToObjectType(func.returnType.GetTypeInfo());
						if( objType )
						{
							for( asUINT i = 0; i < objType->beh.factories.GetLength(); i++ )
							{
								asCScriptFunction *f = engine->scriptFunctions[objType->beh.factories[i]];
								if( f == 0 ||
									!func.IsSignatureExceptNameAndObjectTypeEqual(f) )
									continue;

								usedFunctions[n] = f;
								break;
							}
						}
					}
					else if( func.name == "$list" )
					{
						// listFactory is used for both factory is global and returns a handle and constructor that is a method
						asCObjectType *objType = func.objectType ? func.objectType : CastToObjectType(func.returnType.GetTypeInfo());
						if( objType )
						{
							asCScriptFunction *f = engine->scriptFunctions[objType->beh.listFactory];
							if( f && func.IsSignatureExceptNameAndObjectTypeEqual(f) )
								usedFunctions[n] = f;
						}
					}
					else if( func.name == "$beh2" )
					{
						// This is a destructor, so check the object type's destructor
						asCObjectType *objType = func.objectType;
						if( objType )
						{
							asCScriptFunction *f = engine->scriptFunctions[objType->beh.destruct];
							if( f && func.IsSignatureExceptNameAndObjectTypeEqual(f) )
								usedFunctions[n] = f;
						}
					}
					else if( func.name == "$dlgte" )
					{
						// This is the delegate factory
						asCScriptFunction *f = engine->registeredGlobalFuncs.GetFirst(engine->nameSpaces[0], DELEGATE_FACTORY);
						asASSERT( f && func.IsSignatureEqual(f) );
						usedFunctions[n] = f;
					}
					else
					{
						// Must match one of the above cases
						asASSERT(false);
					}
				}
				else if( func.objectType == 0 )
				{
					// This is a global function
					const asCArray<asUINT> &funcs = engine->registeredGlobalFuncs.GetIndexes(func.nameSpace, func.name);
					for( asUINT i = 0; i < funcs.GetLength(); i++ )
					{
						asCScriptFunction *f = engine->registeredGlobalFuncs.Get(funcs[i]);
						if( f == 0 ||
							!func.IsSignatureExceptNameAndObjectTypeEqual(f) )
							continue;

						usedFunctions[n] = f;
						break;
					}
				}
				else if( func.objectType )
				{
					// It is a class member, so we can search directly in the object type's members
					// TODO: virtual function is different that implemented method
					for( asUINT i = 0; i < func.objectType->methods.GetLength(); i++ )
					{
						asCScriptFunction *f = engine->scriptFunctions[func.objectType->methods[i]];
						if( f == 0 ||
							!func.IsSignatureEqual(f) )
							continue;

						usedFunctions[n] = f;
						break;
					}
				}

				if( usedFunctions[n] == 0 )
				{
					// TODO: clean up: This part of the code should never happen. All functions should
					//                 be found in the above logic. The only valid reason to come here
					//                 is if the bytecode is wrong and the function doesn't exist anyway.
					//                 This loop is kept temporarily until we can be certain all scenarios
					//                 are covered.
					for( asUINT i = 0; i < engine->scriptFunctions.GetLength(); i++ )
					{
						asCScriptFunction *f = engine->scriptFunctions[i];
						if( f == 0 ||
							func.objectType != f->objectType ||
							func.nameSpace != f->nameSpace ||
							!func.IsSignatureEqual(f) )
							continue;

						usedFunctions[n] = f;
						break;
					}

					// No function is expected to be found
					asASSERT(usedFunctions[n] == 0);
				}
			}

			// Set the type to dummy so it won't try to release the id
			func.funcType = asFUNC_DUMMY;

			if( usedFunctions[n] == 0 )
			{
				Error(TXT_INVALID_BYTECODE_d);
				return;
			}
		}
	}
}

void asCReader::ReadFunctionSignature(asCScriptFunction *func, asCObjectType **parentClass)
{
	asUINT i, count;
	asCDataType dt;
	int num;

	ReadString(&func->name);
	if( func->name == DELEGATE_FACTORY )
	{
		// It's not necessary to read anymore, everything is known
		asCScriptFunction *f = engine->registeredGlobalFuncs.GetFirst(engine->nameSpaces[0], DELEGATE_FACTORY);
		asASSERT( f );
		func->returnType     = f->returnType;
		func->parameterTypes = f->parameterTypes;
		func->inOutFlags     = f->inOutFlags;
		func->funcType       = f->funcType;
		func->defaultArgs    = f->defaultArgs;
		func->nameSpace      = f->nameSpace;
		return;
	}

	ReadDataType(&func->returnType);

	count = SanityCheck(ReadEncodedUInt(), 256);
	func->parameterTypes.Allocate(count, false);
	for( i = 0; i < count; ++i )
	{
		ReadDataType(&dt);
		func->parameterTypes.PushLast(dt);
	}

	func->inOutFlags.SetLength(func->parameterTypes.GetLength());
	if( func->inOutFlags.GetLength() != func->parameterTypes.GetLength() )
	{
		// Out of memory
		error = true;
		return;
	}
	memset(func->inOutFlags.AddressOf(), 0, sizeof(asETypeModifiers)*func->inOutFlags.GetLength());
	if (func->parameterTypes.GetLength() > 0)
	{
		count = ReadEncodedUInt();
		if (count > func->parameterTypes.GetLength())
		{
			// Cannot be more than the number of arguments
			Error(TXT_INVALID_BYTECODE_d);
			return;
		}
		for (i = 0; i < count; ++i)
		{
			num = ReadEncodedUInt();
			func->inOutFlags[i] = static_cast<asETypeModifiers>(num);
		}
	}

	func->funcType = (asEFuncType)ReadEncodedUInt();

	// Read the default args, from last to first
	if (func->parameterTypes.GetLength() > 0)
	{
		count = ReadEncodedUInt();
		if (count > func->parameterTypes.GetLength())
		{
			// Cannot be more than the number of arguments
			Error(TXT_INVALID_BYTECODE_d);
			return;
		}
		if (count)
		{
			func->defaultArgs.SetLength(func->parameterTypes.GetLength());
			if (func->defaultArgs.GetLength() != func->parameterTypes.GetLength())
			{
				// Out of memory
				error = true;
				return;
			}
			memset(func->defaultArgs.AddressOf(), 0, sizeof(asCString*)*func->defaultArgs.GetLength());
			for (i = 0; i < count; i++)
			{
				asCString *str = asNEW(asCString);
				if (str == 0)
				{
					// Out of memory
					error = true;
					return;
				}
				func->defaultArgs[func->defaultArgs.GetLength() - 1 - i] = str;
				ReadString(str);
			}
		}
	}

	func->objectType = CastToObjectType(ReadTypeInfo());
	if( func->objectType )
	{
		func->objectType->AddRefInternal();

		asBYTE b;
		ReadData(&b, 1);
		func->SetReadOnly((b & 1) ? true : false);
		func->SetPrivate((b & 2) ? true : false);
		func->SetProtected((b & 4) ? true : false);
		func->nameSpace = func->objectType->nameSpace;
	}
	else
	{
		if (func->funcType == asFUNC_FUNCDEF)
		{
			asBYTE b;
			ReadData(&b, 1);
			if (b == 'n')
			{
				asCString ns;
				ReadString(&ns);
				func->nameSpace = engine->AddNameSpace(ns.AddressOf());
			}
			else if (b == 'o')
			{
				func->nameSpace = 0;
				if (parentClass)
					*parentClass = CastToObjectType(ReadTypeInfo());
				else
					error = true;
			}
			else
				error = true;
		}
		else
		{
			asCString ns;
			ReadString(&ns);
			func->nameSpace = engine->AddNameSpace(ns.AddressOf());
		}
	}
}

asCScriptFunction *asCReader::ReadFunction(bool &isNew, bool addToModule, bool addToEngine, bool addToGC, bool *isExternal)
{
	isNew = false;
	if (isExternal) *isExternal = false;
	if( error ) return 0;

	char c;
	ReadData(&c, 1);

	if( c == '\0' )
	{
		// There is no function, so return a null pointer
		return 0;
	}

	if( c == 'r' )
	{
		// This is a reference to a previously saved function
		asUINT index = ReadEncodedUInt();
		if( index < savedFunctions.GetLength() )
			return savedFunctions[index];
		else
		{
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}
	}

	// Load the new function
	isNew = true;
	asCScriptFunction *func = asNEW(asCScriptFunction)(engine,0,asFUNC_DUMMY);
	if( func == 0 )
	{
		// Out of memory
		error = true;
		return 0;
	}
	savedFunctions.PushLast(func);

	int i, count;
	asCDataType dt;
	int num;

	asCObjectType *parentClass = 0;
	ReadFunctionSignature(func, &parentClass);
	if( error )
	{
		func->DestroyHalfCreated();
		return 0;
	}

	if( func->funcType == asFUNC_SCRIPT )
	{
		// Skip this for external shared entities
		if (module->m_externalTypes.IndexOf(func->objectType) >= 0)
		{
			// Replace with the real function from the existing entity
			isNew = false;

			asCObjectType *ot = func->objectType;
			for (asUINT n = 0; n < ot->methods.GetLength(); n++)
			{
				asCScriptFunction *func2 = engine->scriptFunctions[ot->methods[n]];
				if (func2->funcType == asFUNC_VIRTUAL)
					func2 = ot->virtualFunctionTable[func2->vfTableIdx];

				if (func->IsSignatureEqual(func2))
				{
					func->DestroyHalfCreated();

					// as this is an existing function it shouldn't be translated as if just loaded
					dontTranslate.Insert(func2, true);

					// update the saved functions for future references
					savedFunctions[savedFunctions.GetLength() - 1] = func2;

					// As it is an existing function it shouldn't be added to the module or the engine
					return func2;
				}
			}
		}
		else
		{
			char bits;
			ReadData(&bits, 1);
			func->SetShared((bits & 1) ? true : false);
			func->SetExplicit((bits & 32) ? true : false);
			func->dontCleanUpOnException = (bits & 2) ? true : false;
			if ((bits & 4) && isExternal)
				*isExternal = true;

			// for external shared functions the rest is not needed
			if (!(bits & 4))
			{
				func->AllocateScriptFunctionData();
				if (func->scriptData == 0)
				{
					// Out of memory
					error = true;
					func->DestroyHalfCreated();
					return 0;
				}

				if (addToGC && !addToModule)
					engine->gc.AddScriptObjectToGC(func, &engine->functionBehaviours);

				ReadByteCode(func);

				func->scriptData->variableSpace = SanityCheck(ReadEncodedUInt(), 1000000);

				func->scriptData->objVariablesOnHeap = 0;
				if (bits & 8)
				{
					count = SanityCheck(ReadEncodedUInt(), 1000000);
					func->scriptData->objVariablePos.Allocate(count, false);
					func->scriptData->objVariableTypes.Allocate(count, false);
					for (i = 0; i < count; ++i)
					{
						func->scriptData->objVariableTypes.PushLast(ReadTypeInfo());
						num = ReadEncodedUInt();
						func->scriptData->objVariablePos.PushLast(num);

						if (error)
						{
							// No need to continue (the error has already been reported before)
							func->DestroyHalfCreated();
							return 0;
						}
					}
					if (count > 0)
						func->scriptData->objVariablesOnHeap = SanityCheck(ReadEncodedUInt(), 10000);

					int length = SanityCheck(ReadEncodedUInt(), 1000000);
					func->scriptData->objVariableInfo.SetLength(length);
					for (i = 0; i < length; ++i)
					{
						func->scriptData->objVariableInfo[i].programPos = SanityCheck(ReadEncodedUInt(), 1000000);
						func->scriptData->objVariableInfo[i].variableOffset = SanityCheck(ReadEncodedInt(), 10000);
						asEObjVarInfoOption option = (asEObjVarInfoOption)ReadEncodedUInt();
						func->scriptData->objVariableInfo[i].option = option;
						if (option != asOBJ_INIT && 
							option != asOBJ_UNINIT && 
							option != asBLOCK_BEGIN && 
							option != asBLOCK_END && 
							option != asOBJ_VARDECL)
						{
							error = true;
							func->DestroyHalfCreated();
							return 0;
						}
					}
				}

				if (bits & 16)
				{
					// Read info on try/catch blocks
					int length = SanityCheck(ReadEncodedUInt(), 1000000);
					func->scriptData->tryCatchInfo.SetLength(length);
					for (i = 0; i < length; ++i)
					{
						// The program position must be adjusted to be in number of instructions
						func->scriptData->tryCatchInfo[i].tryPos = SanityCheck(ReadEncodedUInt(), 1000000);
						func->scriptData->tryCatchInfo[i].catchPos = SanityCheck(ReadEncodedUInt(), 1000000);
					}
				}

				if (!noDebugInfo)
				{
					int length = SanityCheck(ReadEncodedUInt(), 1000000);
					func->scriptData->lineNumbers.SetLength(length);
					if (int(func->scriptData->lineNumbers.GetLength()) != length)
					{
						// Out of memory
						error = true;
						func->DestroyHalfCreated();
						return 0;
					}
					for (i = 0; i < length; ++i)
						func->scriptData->lineNumbers[i] = ReadEncodedUInt();

					// Read the array of script sections
					length = SanityCheck(ReadEncodedUInt(), 1000000);
					func->scriptData->sectionIdxs.SetLength(length);
					if (int(func->scriptData->sectionIdxs.GetLength()) != length)
					{
						// Out of memory
						error = true;
						func->DestroyHalfCreated();
						return 0;
					}
					for (i = 0; i < length; ++i)
					{
						if ((i & 1) == 0)
							func->scriptData->sectionIdxs[i] = ReadEncodedUInt();
						else
						{
							asCString str;
							ReadString(&str);
							func->scriptData->sectionIdxs[i] = engine->GetScriptSectionNameIndex(str.AddressOf());
						}
					}
				}

				// Read the variable information
				if (!noDebugInfo)
				{
					int length = SanityCheck(ReadEncodedUInt(), 1000000);
					func->scriptData->variables.Allocate(length, false);
					for (i = 0; i < length; i++)
					{
						asSScriptVariable *var = asNEW(asSScriptVariable);
						if (var == 0)
						{
							// Out of memory
							error = true;
							func->DestroyHalfCreated();
							return 0;
						}
						func->scriptData->variables.PushLast(var);

						var->declaredAtProgramPos = ReadEncodedUInt();
						var->stackOffset = SanityCheck(ReadEncodedInt(),10000);
						ReadString(&var->name);
						ReadDataType(&var->type);

						if (error)
						{
							// No need to continue (the error has already been reported before)
							func->DestroyHalfCreated();
							return 0;
						}
					}
				}

				// Read script section name
				if (!noDebugInfo)
				{
					asCString name;
					ReadString(&name);
					func->scriptData->scriptSectionIdx = engine->GetScriptSectionNameIndex(name.AddressOf());
					func->scriptData->declaredAt = ReadEncodedUInt();
				}

				// Read parameter names
				if (!noDebugInfo)
				{
					asUINT countParam = asUINT(ReadEncodedUInt64());
					if (countParam > func->parameterTypes.GetLength())
					{
						error = true;
						func->DestroyHalfCreated();
						return 0;
					}
					func->parameterNames.SetLength(countParam);
					for (asUINT n = 0; n < countParam; n++)
						ReadString(&func->parameterNames[n]);
				}
			}
		}
	}
	else if( func->funcType == asFUNC_VIRTUAL || func->funcType == asFUNC_INTERFACE )
	{
		func->vfTableIdx = ReadEncodedUInt();
	}
	else if( func->funcType == asFUNC_FUNCDEF )
	{
		asBYTE bits;
		ReadData(&bits, 1);
		if( bits & 1 )
			func->SetShared(true);
		if ((bits & 2) && isExternal)
			*isExternal = true;

		// The asCFuncdefType constructor adds itself to the func->funcdefType member
		asCFuncdefType *fdt = asNEW(asCFuncdefType)(engine, func);
		fdt->parentClass = parentClass;
	}

	// Methods loaded for shared objects, owned by other modules should not be created as new functions
	if( func->objectType && func->objectType->module != module )
	{
		// Return the real function from the object
		asCScriptFunction *realFunc = 0;
		bool found = false;
		if( func->funcType == asFUNC_SCRIPT )
		{
			realFunc = engine->scriptFunctions[func->objectType->beh.destruct];
			if( realFunc && realFunc->funcType != asFUNC_VIRTUAL && func->IsSignatureEqual(realFunc) )
			{
				found = true;
			}
			for( asUINT n = 0; !found && n < func->objectType->beh.constructors.GetLength(); n++ )
			{
				realFunc = engine->scriptFunctions[func->objectType->beh.constructors[n]];
				if( realFunc && realFunc->funcType != asFUNC_VIRTUAL && func->IsSignatureEqual(realFunc) )
				{
					found = true;
					break;
				}
			}	
			for( asUINT n = 0; !found && n < func->objectType->beh.factories.GetLength(); n++ )
			{
				realFunc = engine->scriptFunctions[func->objectType->beh.factories[n]];
				if( realFunc && realFunc->funcType != asFUNC_VIRTUAL && func->IsSignatureEqual(realFunc) )
				{
					found = true;
					break;
				}
			}	
			for( asUINT n = 0; !found && n < func->objectType->methods.GetLength(); n++ )
			{
				realFunc = engine->scriptFunctions[func->objectType->methods[n]];
				if( realFunc && realFunc->funcType == func->funcType && func->IsSignatureEqual(realFunc) )
				{
					found = true;
					break;
				}
			}
			for( asUINT n = 0; !found && n < func->objectType->virtualFunctionTable.GetLength(); n++ )
			{
				realFunc = func->objectType->virtualFunctionTable[n];
				if( realFunc && realFunc->funcType == func->funcType && func->IsSignatureEqual(realFunc) )
				{
					found = true;
					break;
				}
			}
		}
		else if( func->funcType == asFUNC_VIRTUAL || func->funcType == asFUNC_INTERFACE )
		{
			// If the loaded function is a virtual function, then look for the identical virtual function in the methods array
			for( asUINT n = 0; n < func->objectType->methods.GetLength(); n++ )
			{
				realFunc = engine->scriptFunctions[func->objectType->methods[n]];
				if( realFunc && realFunc->funcType == func->funcType && func->IsSignatureEqual(realFunc) )
				{
					asASSERT( func->vfTableIdx == realFunc->vfTableIdx );
					found = true;
					break;
				}
			}
		}

		if( found )
		{
			// as this is an existing function it shouldn't be translated as if just loaded
			dontTranslate.Insert(realFunc, true);

			// update the saved functions for future references
			savedFunctions[savedFunctions.GetLength() - 1] = realFunc;

			if( realFunc->funcType == asFUNC_VIRTUAL && addToModule )
			{
				// Virtual methods must be added to the module's script functions array, 
				// even if they are not owned by the module
				module->m_scriptFunctions.PushLast(realFunc);
				realFunc->AddRefInternal();		
			}				
		}
		else
		{
			asCString str;
			str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, func->objectType->GetName());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			
			Error(TXT_INVALID_BYTECODE_d);
			savedFunctions.PopLast();
			realFunc = 0;
		}

		// Destroy the newly created function instance since it has been replaced by an existing function
		isNew = false;
		func->DestroyHalfCreated();
		
		// As it is an existing function it shouldn't be added to the module or the engine
		return realFunc;
	}	

	if( addToModule )
	{
		// The refCount is already 1
		module->m_scriptFunctions.PushLast(func);
		func->module = module;
	}
	if( addToEngine )
	{
		func->id = engine->GetNextScriptFunctionId();
		engine->AddScriptFunction(func);
	}
	if( func->objectType )
		func->ComputeSignatureId();

	return func;
}

void asCReader::ReadTypeDeclaration(asCTypeInfo *type, int phase, bool *isExternal)
{
	if( phase == 1 )
	{
		asASSERT(isExternal);
		if (isExternal)
			*isExternal = false;

		// Read the initial attributes
		ReadString(&type->name);
		ReadData(&type->flags, 4);
		type->size = SanityCheck(ReadEncodedUInt(), 1000000);
		asCString ns;
		ReadString(&ns);
		type->nameSpace = engine->AddNameSpace(ns.AddressOf());

		// Verify that the flags match the asCTypeInfo
		if ((CastToEnumType(type) && !(type->flags & asOBJ_ENUM)) ||
			(CastToFuncdefType(type) && !(type->flags & asOBJ_FUNCDEF)) ||
			(CastToObjectType(type) && !(type->flags & (asOBJ_REF | asOBJ_VALUE))))
		{
			error = true;
			return;
		}

		// Reset the size of script classes, since it will be recalculated as properties are added
		if( (type->flags & asOBJ_SCRIPT_OBJECT) && type->size != 0 )
			type->size = sizeof(asCScriptObject);

		asCObjectType *ot = CastToObjectType(type);
		if (ot)
		{
			// Use the default script class behaviours
			ot->beh = engine->scriptTypeBehaviours.beh;
			ot->beh.construct = 0;
			ot->beh.factory = 0;
			ot->beh.constructors.PopLast(); // These will be read from the file
			ot->beh.factories.PopLast(); // These will be read from the file
			engine->scriptFunctions[ot->beh.addref]->AddRefInternal();
			engine->scriptFunctions[ot->beh.release]->AddRefInternal();
			engine->scriptFunctions[ot->beh.gcEnumReferences]->AddRefInternal();
			engine->scriptFunctions[ot->beh.gcGetFlag]->AddRefInternal();
			engine->scriptFunctions[ot->beh.gcGetRefCount]->AddRefInternal();
			engine->scriptFunctions[ot->beh.gcReleaseAllReferences]->AddRefInternal();
			engine->scriptFunctions[ot->beh.gcSetFlag]->AddRefInternal();
			engine->scriptFunctions[ot->beh.copy]->AddRefInternal();
			// TODO: weak: Should not do this if the class has been declared with 'noweak'
			engine->scriptFunctions[ot->beh.getWeakRefFlag]->AddRefInternal();
		}

		// external shared flag
		if (type->flags & asOBJ_SHARED)
		{
			char c;
			ReadData(&c, 1);
			if (c == 'e')
				*isExternal = true;
			else if (c != ' ')
			{
				error = true;
				return;
			}
		}
	}
	else if( phase == 2 )
	{
		// external shared types doesn't store this
		if ((type->flags & asOBJ_SHARED) && module->m_externalTypes.IndexOf(type) >= 0)
			return;

		if( type->flags & asOBJ_ENUM )
		{
			asCEnumType *t = CastToEnumType(type);
			int count = SanityCheck(ReadEncodedUInt(), 1000000);
			bool sharedExists = existingShared.MoveTo(0, type);
			if( !sharedExists )
			{
				t->enumValues.Allocate(count, false);
				for( int n = 0; n < count; n++ )
				{
					asSEnumValue *e = asNEW(asSEnumValue);
					if( e == 0 )
					{
						// Out of memory
						error = true;
						return;
					}
					ReadString(&e->name);
					ReadData(&e->value, 4); // TODO: Should be encoded
					t->enumValues.PushLast(e);
				}
			}
			else
			{
				// Verify that the enum values exists in the original
				asCString name;
				int value;
				for( int n = 0; n < count; n++ )
				{
					ReadString(&name);
					ReadData(&value, 4); // TODO: Should be encoded
					bool found = false;
					for( asUINT e = 0; e < t->enumValues.GetLength(); e++ )
					{
						if( t->enumValues[e]->name == name &&
							t->enumValues[e]->value == value )
						{
							found = true;
							break;
						}
					}
					if( !found )
					{
						asCString str;
						str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
						engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
						Error(TXT_INVALID_BYTECODE_d);
					}
				}
			}
		}
		else if( type->flags & asOBJ_TYPEDEF )
		{
			asCTypedefType *td = CastToTypedefType(type);
			asASSERT(td);
			eTokenType t = (eTokenType)ReadEncodedUInt();
			td->aliasForType = asCDataType::CreatePrimitive(t, false);
		}
		else
		{
			asCObjectType *ot = CastToObjectType(type);
			asASSERT(ot);

			// If the type is shared and pre-existing, we should just
			// validate that the loaded methods match the original
			bool sharedExists = existingShared.MoveTo(0, type);
			if( sharedExists )
			{
				asCObjectType *dt = CastToObjectType(ReadTypeInfo());
				if( ot->derivedFrom != dt )
				{
					asCString str;
					str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
					engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
					Error(TXT_INVALID_BYTECODE_d);
				}
			}
			else
			{
				ot->derivedFrom = CastToObjectType(ReadTypeInfo());
				if( ot->derivedFrom )
					ot->derivedFrom->AddRefInternal();
			}

			// interfaces[] / interfaceVFTOffsets[]
			int size = SanityCheck(ReadEncodedUInt(), 1000000);
			if( sharedExists )
			{
				for( int n = 0; n < size; n++ )
				{
					asCObjectType *intf = CastToObjectType(ReadTypeInfo());
					if (!ot->IsInterface())
						ReadEncodedUInt();

					if( !type->Implements(intf) )
					{
						asCString str;
						str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
						engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
						Error(TXT_INVALID_BYTECODE_d);
					}
				}
			}
			else
			{
				ot->interfaces.Allocate(size, false);
				if( !ot->IsInterface() )
					ot->interfaceVFTOffsets.Allocate(size, false);
				for( int n = 0; n < size; n++ )
				{
					asCObjectType *intf = CastToObjectType(ReadTypeInfo());
					ot->interfaces.PushLast(intf);

					if (!ot->IsInterface())
					{
						asUINT offset = SanityCheck(ReadEncodedUInt(), 1000000);
						ot->interfaceVFTOffsets.PushLast(offset);
					}
				}
			}

			// behaviours
			if( !ot->IsInterface() && type->flags != asOBJ_TYPEDEF && type->flags != asOBJ_ENUM )
			{
				bool isNew;
				asCScriptFunction *func = ReadFunction(isNew, !sharedExists, !sharedExists, !sharedExists);
				if( sharedExists )
				{
					// Find the real function in the object, and update the savedFunctions array
					asCScriptFunction *realFunc = engine->GetScriptFunction(ot->beh.destruct);
					if( (realFunc == 0 && func == 0) || realFunc->IsSignatureEqual(func) )
					{
						// If the function is not the last, then the substitution has already occurred before
						if( func && savedFunctions[savedFunctions.GetLength()-1] == func )
							savedFunctions[savedFunctions.GetLength()-1] = realFunc;
					}
					else
					{
						asCString str;
						str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
						engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
						Error(TXT_INVALID_BYTECODE_d);
					}
					if( func )
					{
						if( isNew )
						{
							// Destroy the function without releasing any references
							func->id = 0;
							func->scriptData->byteCode.SetLength(0);
							func->ReleaseInternal();
						}
						dontTranslate.Insert(realFunc, true);
					}
				}
				else
				{
					if( func )
					{
						ot->beh.destruct = func->id;
						func->AddRefInternal();
					}
					else
						ot->beh.destruct = 0;
				}

				size = SanityCheck(ReadEncodedUInt(), 1000000);
				for( int n = 0; n < size; n++ )
				{
					func = ReadFunction(isNew, !sharedExists, !sharedExists, !sharedExists);
					if( func )
					{
						if( sharedExists )
						{
							// Find the real function in the object, and update the savedFunctions array
							bool found = false;
							for( asUINT f = 0; f < ot->beh.constructors.GetLength(); f++ )
							{
								asCScriptFunction *realFunc = engine->GetScriptFunction(ot->beh.constructors[f]);
								if( realFunc->IsSignatureEqual(func) )
								{
									// If the function is not the last, then the substitution has already occurred before
									if( savedFunctions[savedFunctions.GetLength()-1] == func )
										savedFunctions[savedFunctions.GetLength()-1] = realFunc;
									found = true;
									dontTranslate.Insert(realFunc, true);
									break;
								}
							}
							if( !found )
							{
								asCString str;
								str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
								engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
								Error(TXT_INVALID_BYTECODE_d);
							}
							if( isNew )
							{
								// Destroy the function without releasing any references
								func->id = 0;
								func->scriptData->byteCode.SetLength(0);
								func->ReleaseInternal();
							}
						}
						else
						{
							ot->beh.constructors.PushLast(func->id);
							func->AddRefInternal();

							if( func->parameterTypes.GetLength() == 0 )
								ot->beh.construct = func->id;
						}
					}
					else
					{
						Error(TXT_INVALID_BYTECODE_d);
					}

					func = ReadFunction(isNew, !sharedExists, !sharedExists, !sharedExists);
					if( func )
					{
						if( sharedExists )
						{
							// Find the real function in the object, and update the savedFunctions array
							bool found = false;
							for( asUINT f = 0; f < ot->beh.factories.GetLength(); f++ )
							{
								asCScriptFunction *realFunc = engine->GetScriptFunction(ot->beh.factories[f]);
								if( realFunc->IsSignatureEqual(func) )
								{
									// If the function is not the last, then the substitution has already occurred before
									if( savedFunctions[savedFunctions.GetLength()-1] == func )
										savedFunctions[savedFunctions.GetLength()-1] = realFunc;
									found = true;
									dontTranslate.Insert(realFunc, true);
									break;
								}
							}
							if( !found )
							{
								asCString str;
								str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
								engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
								Error(TXT_INVALID_BYTECODE_d);
							}
							if( isNew )
							{
								// Destroy the function without releasing any references
								func->id = 0;
								func->scriptData->byteCode.SetLength(0);
								func->ReleaseInternal();
							}
						}
						else
						{
							ot->beh.factories.PushLast(func->id);
							func->AddRefInternal();

							if( func->parameterTypes.GetLength() == 0 )
								ot->beh.factory = func->id;
						}
					}
					else
					{
						Error(TXT_INVALID_BYTECODE_d);
					}
				}
			}

			// methods[]
			size = SanityCheck(ReadEncodedUInt(), 1000000);
			int n;
			for( n = 0; n < size; n++ )
			{
				bool isNew;
				asCScriptFunction *func = ReadFunction(isNew, !sharedExists, !sharedExists, !sharedExists);
				if( func )
				{
					if( sharedExists )
					{
						// Find the real function in the object, and update the savedFunctions array
						bool found = false;
						for( asUINT f = 0; f < ot->methods.GetLength(); f++ )
						{
							asCScriptFunction *realFunc = engine->GetScriptFunction(ot->methods[f]);
							if( realFunc->IsSignatureEqual(func) )
							{
								// If the function is not the last, then the substitution has already occurred before
								if( savedFunctions[savedFunctions.GetLength()-1] == func )
									savedFunctions[savedFunctions.GetLength()-1] = realFunc;
								found = true;
								dontTranslate.Insert(realFunc, true);
								break;
							}
						}
						if( !found )
						{
							asCString str;
							str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
							engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
							Error(TXT_INVALID_BYTECODE_d);
						}
						if( isNew )
						{
							// Destroy the function without releasing any references
							if( func->id == func->signatureId )
								engine->signatureIds.RemoveValue(func);
							func->id = 0;
							if( func->scriptData )
								func->scriptData->byteCode.SetLength(0);
							func->ReleaseInternal();
						}
					}
					else
					{
						// If the method is the assignment operator we need to replace the default implementation
						if( func->name == "opAssign" && func->parameterTypes.GetLength() == 1 &&
							func->parameterTypes[0].GetTypeInfo() == func->objectType &&
							(func->inOutFlags[0] & asTM_INREF) )
						{
							engine->scriptFunctions[ot->beh.copy]->ReleaseInternal();
							ot->beh.copy = func->id;
							func->AddRefInternal();
						}

						ot->methods.PushLast(func->id);
						func->AddRefInternal();
					}
				}
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
				}
			}

			// virtualFunctionTable[]
			size = SanityCheck(ReadEncodedUInt(), 1000000);
			for( n = 0; n < size; n++ )
			{
				bool isNew;
				asCScriptFunction *func = ReadFunction(isNew, !sharedExists, !sharedExists, !sharedExists);
				if( func )
				{
					if( sharedExists )
					{
						// Find the real function in the object, and update the savedFunctions array
						bool found = false;
						for( asUINT f = 0; f < ot->virtualFunctionTable.GetLength(); f++ )
						{
							asCScriptFunction *realFunc = ot->virtualFunctionTable[f];
							if( realFunc->IsSignatureEqual(func) )
							{
								// If the function is not the last, then the substitution has already occurred before
								if( savedFunctions[savedFunctions.GetLength()-1] == func )
									savedFunctions[savedFunctions.GetLength()-1] = realFunc;
								found = true;
								dontTranslate.Insert(realFunc, true);
								break;
							}
						}
						if( !found )
						{
							asCString str;
							str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, type->GetName());
							engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
							Error(TXT_INVALID_BYTECODE_d);
						}
						if( isNew )
						{
							// Destroy the function without releasing any references
							func->id = 0;
							if( func->scriptData )
								func->scriptData->byteCode.SetLength(0);
							func->ReleaseInternal();
						}
					}
					else
					{
						ot->virtualFunctionTable.PushLast(func);
						func->AddRefInternal();
					}
				}
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
				}
			}
		}
	}
	else if( phase == 3 )
	{
		// external shared types doesn't store this
		if ((type->flags & asOBJ_SHARED) && module->m_externalTypes.IndexOf(type) >= 0)
			return;

		asCObjectType *ot = CastToObjectType(type);

		// This is only done for object types
		asASSERT(ot);

		// properties[]
		asUINT size = SanityCheck(ReadEncodedUInt(), 1000000);
		for( asUINT n = 0; n < size; n++ )
			ReadObjectProperty(ot);
	}
}

asWORD asCReader::ReadEncodedUInt16()
{
	asDWORD dw = ReadEncodedUInt();
	if( (dw>>16) != 0 && (dw>>16) != 0xFFFF )
	{
		Error(TXT_INVALID_BYTECODE_d);
	}

	return asWORD(dw & 0xFFFF);
}

asUINT asCReader::ReadEncodedUInt()
{
	asQWORD qw = ReadEncodedUInt64();
	if( (qw>>32) != 0 && (qw>>32) != 0xFFFFFFFF )
	{
		Error(TXT_INVALID_BYTECODE_d);
	}

	return asUINT(qw & 0xFFFFFFFFu);
}

int asCReader::ReadEncodedInt()
{
	return int(ReadEncodedUInt());
}

asQWORD asCReader::ReadEncodedUInt64()
{
	asQWORD i = 0;
	asBYTE b = 0xFF; // set to 0xFF to better catch if the stream doesn't update the value
	ReadData(&b, 1);
	bool isNegative = ( b & 0x80 ) ? true : false;
	b &= 0x7F;

	if( (b & 0x7F) == 0x7F )
	{
		ReadData(&b, 1); i = asQWORD(b) << 56;
		ReadData(&b, 1); i += asQWORD(b) << 48;
		ReadData(&b, 1); i += asQWORD(b) << 40;
		ReadData(&b, 1); i += asQWORD(b) << 32;
		ReadData(&b, 1); i += asUINT(b) << 24;
		ReadData(&b, 1); i += asUINT(b) << 16;
		ReadData(&b, 1); i += asUINT(b) << 8;
		ReadData(&b, 1); i += b;
	}
	else if( (b & 0x7E) == 0x7E )
	{
		i = asQWORD(b & 0x01) << 48;
		ReadData(&b, 1); i += asQWORD(b) << 40;
		ReadData(&b, 1); i += asQWORD(b) << 32;
		ReadData(&b, 1); i += asUINT(b) << 24;
		ReadData(&b, 1); i += asUINT(b) << 16;
		ReadData(&b, 1); i += asUINT(b) << 8;
		ReadData(&b, 1); i += b;
	}
	else if( (b & 0x7C) == 0x7C )
	{
		i = asQWORD(b & 0x03) << 40;
		ReadData(&b, 1); i += asQWORD(b) << 32;
		ReadData(&b, 1); i += asUINT(b) << 24;
		ReadData(&b, 1); i += asUINT(b) << 16;
		ReadData(&b, 1); i += asUINT(b) << 8;
		ReadData(&b, 1); i += b;
	}
	else if( (b & 0x78) == 0x78 )
	{
		i = asQWORD(b & 0x07) << 32;
		ReadData(&b, 1); i += asUINT(b) << 24;
		ReadData(&b, 1); i += asUINT(b) << 16;
		ReadData(&b, 1); i += asUINT(b) << 8;
		ReadData(&b, 1); i += b;
	}
	else if( (b & 0x70) == 0x70 )
	{
		i = asUINT(b & 0x0F) << 24;
		ReadData(&b, 1); i += asUINT(b) << 16;
		ReadData(&b, 1); i += asUINT(b) << 8;
		ReadData(&b, 1); i += b;
	}
	else if( (b & 0x60) == 0x60 )
	{
		i = asUINT(b & 0x1F) << 16;
		ReadData(&b, 1); i += asUINT(b) << 8;
		ReadData(&b, 1); i += b;
	}
	else if( (b & 0x40) == 0x40 )
	{
		i = asUINT(b & 0x3F) << 8;
		ReadData(&b, 1); i += b;
	}
	else
	{
		i = b;
	}
	if( isNegative )
		i = (asQWORD)(-asINT64(i));

	return i;
}

asUINT asCReader::SanityCheck(asUINT val, asUINT max)
{
	if (val > max)
	{
		Error(TXT_INVALID_BYTECODE_d);

		// Return 0 as default value
		return 0;
	}

	return val;
}

int asCReader::SanityCheck(int val, asUINT max)
{
	if (val > int(max) || val < -int(max))
	{
		Error(TXT_INVALID_BYTECODE_d);

		// Return 0 as default value
		return 0;
	}

	return val;
}

void asCReader::ReadString(asCString* str)
{
	asUINT len = SanityCheck(ReadEncodedUInt(), 1000000);
	if( len & 1 )
	{
		asUINT idx = len/2;
		if( idx < savedStrings.GetLength() )
			*str = savedStrings[idx];
		else
			Error(TXT_INVALID_BYTECODE_d);
	}
	else if( len > 0 )
	{
		len /= 2;
		str->SetLength(len);
		int r = stream->Read(str->AddressOf(), len);
		if (r < 0)
			Error(TXT_UNEXPECTED_END_OF_FILE);

		savedStrings.PushLast(*str);
	}
	else
		str->SetLength(0);
}

void asCReader::ReadGlobalProperty()
{
	asCString name;
	asCDataType type;

	ReadString(&name);

	asCString ns;
	ReadString(&ns);
	asSNameSpace *nameSpace = engine->AddNameSpace(ns.AddressOf());

	ReadDataType(&type);

	asCGlobalProperty *prop = module->AllocateGlobalProperty(name.AddressOf(), type, nameSpace);

	// Read the initialization function
	bool isNew;
	// Do not add the function to the GC at this time. It will
	// only be added to the GC when the module releases the property
	asCScriptFunction *func = ReadFunction(isNew, false, true, false);
	if( func )
	{
		// Make sure the function knows it is owned by the module
		func->module = module;

		prop->SetInitFunc(func);
		func->ReleaseInternal();
	}
}

void asCReader::ReadObjectProperty(asCObjectType *ot)
{
	asCString name;
	ReadString(&name);
	asCDataType dt;
	ReadDataType(&dt);
	int flags = ReadEncodedUInt();
	bool isPrivate = (flags & 1) ? true : false;
	bool isProtected = (flags & 2) ? true : false;
	bool isInherited = (flags & 4) ? true : false;

	// TODO: shared: If the type is shared and pre-existing, we should just
	//               validate that the loaded methods match the original
	if( !existingShared.MoveTo(0, ot) )
		ot->AddPropertyToClass(name, dt, isPrivate, isProtected, isInherited);
}

void asCReader::ReadDataType(asCDataType *dt)
{
	// Check if this is a previously used type
	asUINT idx = ReadEncodedUInt();
	if( idx != 0 )
	{
		// Get the datatype from the cache
		*dt = savedDataTypes[idx-1];
		return;
	}

	// Read the type definition
	eTokenType tokenType = (eTokenType)ReadEncodedUInt();

	// Reserve a spot in the savedDataTypes
	asUINT saveSlot = savedDataTypes.GetLength();
	savedDataTypes.PushLast(asCDataType());

	// Read the datatype for the first time
	asCTypeInfo *ti = 0;
	if( tokenType == ttIdentifier )
		ti = ReadTypeInfo();

	// Read type flags as a bitmask
	// Endian-safe code
	bool isObjectHandle, isHandleToConst, isReference, isReadOnly;
	char b = 0;
	ReadData(&b, 1);
	LOAD_FROM_BIT(isObjectHandle, b, 0);
	LOAD_FROM_BIT(isHandleToConst, b, 1);
	LOAD_FROM_BIT(isReference, b, 2);
	LOAD_FROM_BIT(isReadOnly, b, 3);

	if( tokenType == ttIdentifier )
		*dt = asCDataType::CreateType(ti, false);
	else
		*dt = asCDataType::CreatePrimitive(tokenType, false);
	if( isObjectHandle )
	{
		dt->MakeReadOnly(isHandleToConst ? true : false);

		// Here we must allow a scoped type to be a handle
		// e.g. if the datatype is for a system function
		dt->MakeHandle(true, true);
	}
	dt->MakeReadOnly(isReadOnly ? true : false);
	dt->MakeReference(isReference ? true : false);

	// Update the previously saved slot
	savedDataTypes[saveSlot] = *dt;
}

asCTypeInfo* asCReader::ReadTypeInfo()
{
	asCTypeInfo *ot = 0;
	char ch;
	ReadData(&ch, 1);
	if( ch == 'a' )
	{
		// Read the name of the template type
		asCString typeName, ns;
		ReadString(&typeName);
		ReadString(&ns);
		asSNameSpace *nameSpace = engine->AddNameSpace(ns.AddressOf());

		asCTypeInfo *tmp = engine->GetRegisteredType(typeName.AddressOf(), nameSpace);
		asCObjectType *tmpl = CastToObjectType(tmp);
		if( tmpl == 0 )
		{
			asCString str;
			str.Format(TXT_TEMPLATE_TYPE_s_DOESNT_EXIST, typeName.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}

		asUINT numSubTypes = SanityCheck(ReadEncodedUInt(), 100);
		asCArray<asCDataType> subTypes;
		for( asUINT n = 0; n < numSubTypes; n++ )
		{
			ReadData(&ch, 1);
			if( ch == 's' )
			{
				asCDataType dt;
				ReadDataType(&dt);
				subTypes.PushLast(dt);
			}
			else
			{
				eTokenType tokenType = (eTokenType)ReadEncodedUInt();
				asCDataType dt = asCDataType::CreatePrimitive(tokenType, false);
				subTypes.PushLast(dt);
			}
		}

		// Return the actual template if the subtypes are the template's dummy types
		if( tmpl->templateSubTypes == subTypes )
			ot = tmpl;
		else
		{
			// Get the template instance type based on the loaded subtypes
			ot = engine->GetTemplateInstanceType(tmpl, subTypes, module);
		}

		if( ot == 0 )
		{
			// Show all subtypes in error message
			asCString sub = subTypes[0].Format(nameSpace);
			for( asUINT n = 1; n < subTypes.GetLength(); n++ )
			{
				sub += ",";
				sub += subTypes[n].Format(nameSpace);
			}
			asCString str;
			str.Format(TXT_INSTANCING_INVLD_TMPL_TYPE_s_s, typeName.AddressOf(), sub.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}
	}
	else if( ch == 'l' )
	{
		asCObjectType *st = CastToObjectType(ReadTypeInfo());
		if( st == 0 || st->beh.listFactory == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}
		ot = engine->GetListPatternType(st->beh.listFactory);
	}
	else if( ch == 's' )
	{
		// Read the name of the template subtype
		asCString typeName;
		ReadString(&typeName);

		// Find the template subtype
		ot = 0;
		for( asUINT n = 0; n < engine->templateSubTypes.GetLength(); n++ )
		{
			if( engine->templateSubTypes[n] && engine->templateSubTypes[n]->name == typeName )
			{
				ot = engine->templateSubTypes[n];
				break;
			}
		}

		if( ot == 0 )
		{
			asCString str;
			str.Format(TXT_TEMPLATE_SUBTYPE_s_DOESNT_EXIST, typeName.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}
	}
	else if( ch == 'o' )
	{
		// Read the object type name
		asCString typeName, ns;
		ReadString(&typeName);
		ReadString(&ns);
		asSNameSpace *nameSpace = engine->AddNameSpace(ns.AddressOf());

		if( typeName.GetLength() && typeName != "$obj" && typeName != "$func" )
		{
			// Find the object type
			ot = module->GetType(typeName.AddressOf(), nameSpace);
			if (!ot)
				ot = engine->GetRegisteredType(typeName.AddressOf(), nameSpace);

			if( ot == 0 )
			{
				asCString str;
				str.Format(TXT_OBJECT_TYPE_s_DOESNT_EXIST, typeName.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				Error(TXT_INVALID_BYTECODE_d);
				return 0;
			}
		}
		else if( typeName == "$obj" )
		{
			ot = &engine->scriptTypeBehaviours;
		}
		else if( typeName == "$func" )
		{
			ot = &engine->functionBehaviours;
		}
		else
			asASSERT( false );
	}
	else if (ch == 'c')
	{
		// Read the object type name
		asCString typeName, ns;
		ReadString(&typeName);

		// Read the parent class
		asCObjectType *parentClass = CastToObjectType(ReadTypeInfo());
		if (parentClass == 0)
		{
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}

		// Find the child type in the parentClass
		for (asUINT n = 0; n < parentClass->childFuncDefs.GetLength(); n++)
		{
			if (parentClass->childFuncDefs[n]->name == typeName)
				ot = parentClass->childFuncDefs[n];
		}

		if (ot == 0)
		{
			asCString str;
			str.Format(TXT_OBJECT_TYPE_s_DOESNT_EXIST, typeName.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}
	}
	else
	{
		// No object type
		asASSERT( ch == '\0' || error );
		ot = 0;
	}

	return ot;
}

void asCReader::ReadByteCode(asCScriptFunction *func)
{
	asASSERT( func->scriptData );

	// Read number of instructions
	asUINT total, numInstructions;
	total = numInstructions = SanityCheck(ReadEncodedUInt(), 1000000);

	// Reserve some space for the instructions
	func->scriptData->byteCode.AllocateNoConstruct(numInstructions, false);

	asUINT pos = 0;
	while( numInstructions )
	{
		asBYTE b;
		ReadData(&b, 1);

		// Allocate the space for the instruction
		asUINT len = asBCTypeSize[asBCInfo[b].type];
		asUINT newSize = asUINT(func->scriptData->byteCode.GetLength()) + len;
		if( func->scriptData->byteCode.GetCapacity() < newSize )
		{
			// Determine the average size of the loaded instructions and re-estimate the final size
			asUINT size = asUINT(float(newSize) / (total - numInstructions) * total) + 1;
			func->scriptData->byteCode.AllocateNoConstruct(size, true);
		}
		if( !func->scriptData->byteCode.SetLengthNoConstruct(newSize) )
		{
			// Out of memory
			error = true;
			return;
		}

		asDWORD *bc = func->scriptData->byteCode.AddressOf() + pos;
		pos += len;

		switch( asBCInfo[b].type )
		{
		case asBCTYPE_NO_ARG:
			{
				*(asBYTE*)(bc) = b;
				bc++;
			}
			break;
		case asBCTYPE_W_ARG:
		case asBCTYPE_wW_ARG:
		case asBCTYPE_rW_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;

				bc++;
			}
			break;
		case asBCTYPE_rW_DW_ARG:
		case asBCTYPE_wW_DW_ARG:
		case asBCTYPE_W_DW_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the word argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;
				bc++;

				// Read the dword argument
				*bc++ = ReadEncodedUInt();
			}
			break;
		case asBCTYPE_DW_ARG:
			{
				*(asBYTE*)(bc) = b;
				bc++;

				// Read the argument
				*bc++ = ReadEncodedUInt();
			}
			break;
		case asBCTYPE_DW_DW_ARG:
			{
				*(asBYTE*)(bc) = b;
				bc++;

				// Read the first argument
				*bc++ = ReadEncodedUInt();

				// Read the second argument
				*bc++ = ReadEncodedUInt();
			}
			break;
		case asBCTYPE_wW_rW_rW_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the first argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;
				bc++;

				// Read the second argument
				w = ReadEncodedUInt16();
				*(asWORD*)bc = w;

				// Read the third argument
				w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;

				bc++;
			}
			break;
		case asBCTYPE_wW_rW_ARG:
		case asBCTYPE_rW_rW_ARG:
		case asBCTYPE_wW_W_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the first argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;
				bc++;

				// Read the second argument
				w = ReadEncodedUInt16();
				*(asWORD*)bc = w;

				bc++;
			}
			break;
		case asBCTYPE_wW_rW_DW_ARG:
		case asBCTYPE_rW_W_DW_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the first argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;
				bc++;

				// Read the second argument
				w = ReadEncodedUInt16();
				*(asWORD*)bc = w;
				bc++;

				// Read the third argument
				asDWORD dw = ReadEncodedUInt();
				*bc++ = dw;
			}
			break;
		case asBCTYPE_QW_ARG:
			{
				*(asBYTE*)(bc) = b;
				bc++;

				// Read the argument
				asQWORD qw = ReadEncodedUInt64();
				*(asQWORD*)bc = qw;
				bc += 2;
			}
			break;
		case asBCTYPE_QW_DW_ARG:
			{
				*(asBYTE*)(bc) = b;
				bc++;

				// Read the first argument
				asQWORD qw = ReadEncodedUInt64();
				*(asQWORD*)bc = qw;
				bc += 2;

				// Read the second argument
				asDWORD dw = ReadEncodedUInt();
				*bc++ = dw;
			}
			break;
		case asBCTYPE_rW_QW_ARG:
		case asBCTYPE_wW_QW_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the first argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;
				bc++;

				// Read the argument
				asQWORD qw = ReadEncodedUInt64();
				*(asQWORD*)bc = qw;
				bc += 2;
			}
			break;
		case asBCTYPE_rW_DW_DW_ARG:
			{
				*(asBYTE*)(bc) = b;

				// Read the 1st argument
				asWORD w = ReadEncodedUInt16();
				*(((asWORD*)bc)+1) = w;
				bc++;

				// Read the 2nd argument
				*bc++ = ReadEncodedUInt();

				// Read the 3rd argument
				*bc++ = ReadEncodedUInt();
			}
			break;
		default:
			{
				// This should never happen
				asASSERT(false);

				// Read the next 3 bytes
				asDWORD c; asBYTE t;
#if defined(AS_BIG_ENDIAN)
				c = b << 24;
				ReadData(&t, 1); c += t << 16;
				ReadData(&t, 1); c += t << 8;
				ReadData(&t, 1); c += t;
#else
				c = b;
				ReadData(&t, 1); c += t << 8;
				ReadData(&t, 1); c += t << 16;
				ReadData(&t, 1); c += t << 24;
#endif

				*bc++ = c;
				c = *(asBYTE*)&c;

				// Read the bc as is
				for( int n = 1; n < asBCTypeSize[asBCInfo[c].type]; n++ )
					ReadData(&*bc++, 4);
			}
		}

		numInstructions--;
	}

	// Correct the final size in case we over-estimated it
	func->scriptData->byteCode.SetLengthNoConstruct(pos);
}

void asCReader::ReadUsedTypeIds()
{
	TimeIt("asCReader::ReadUsedTypeIds");

	asUINT count = SanityCheck(ReadEncodedUInt(), 1000000);
	usedTypeIds.Allocate(count, false);
	for( asUINT n = 0; n < count; n++ )
	{
		asCDataType dt;
		ReadDataType(&dt);
		usedTypeIds.PushLast(engine->GetTypeIdFromDataType(dt));
	}
}

void asCReader::ReadUsedGlobalProps()
{
	TimeIt("asCReader::ReadUsedGlobalProps");

	int c = SanityCheck(ReadEncodedUInt(), 1000000);

	usedGlobalProperties.Allocate(c, false);

	for( int n = 0; n < c; n++ )
	{
		asCString name, ns;
		asCDataType type;
		char moduleProp;

		ReadString(&name);
		ReadString(&ns);
		ReadDataType(&type);
		ReadData(&moduleProp, 1);

		asSNameSpace *nameSpace = engine->AddNameSpace(ns.AddressOf());

		// Find the real property
		asCGlobalProperty *globProp = 0;
		if( moduleProp )
			globProp = module->m_scriptGlobals.GetFirst(nameSpace, name);
		else
			globProp = engine->registeredGlobalProps.GetFirst(nameSpace, name);

		void *prop = 0;
		if( globProp && globProp->type == type )
			prop = globProp->GetAddressOfValue();

		usedGlobalProperties.PushLast(prop);

		if( prop == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
		}
	}
}

void asCReader::ReadUsedObjectProps()
{
	TimeIt("asCReader::ReadUsedObjectProps");

	asUINT c = SanityCheck(ReadEncodedUInt(), 1000000);

	usedObjectProperties.SetLength(c);
	for( asUINT n = 0; n < c; n++ )
	{
		asCObjectType *objType = CastToObjectType(ReadTypeInfo());
		if( objType == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			break;
		}

		asCString name;
		ReadString(&name);

		// Find the property
		bool found = false;
		for( asUINT p = 0; p < objType->properties.GetLength(); p++ )
		{
			if( objType->properties[p]->name == name )
			{
				usedObjectProperties[n].objType = objType;
				usedObjectProperties[n].prop = objType->properties[p];
				found = true;
				break;
			}
		}

		if( !found )
		{
			Error(TXT_INVALID_BYTECODE_d);
			return;
		}
	}
}

short asCReader::FindObjectPropOffset(asWORD index)
{
	static asCObjectProperty *lastCompositeProp = 0;
	if (lastCompositeProp)
	{
		if (index != 0)
		{
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}

		short offset = (short)lastCompositeProp->byteOffset;
		lastCompositeProp = 0;
		return offset;
	}

	if( index >= usedObjectProperties.GetLength() )
	{
		Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}

	if (usedObjectProperties[index].prop->compositeOffset || usedObjectProperties[index].prop->isCompositeIndirect)
	{
		lastCompositeProp = usedObjectProperties[index].prop;
		return (short)lastCompositeProp->compositeOffset;
	}
	return (short)usedObjectProperties[index].prop->byteOffset;
}

asCScriptFunction *asCReader::FindFunction(int idx)
{
	if( idx >= 0 && idx < (int)usedFunctions.GetLength() )
		return usedFunctions[idx];
	else
	{
		Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}
}

void asCReader::TranslateFunction(asCScriptFunction *func)
{
	// Skip this if the function is part of an pre-existing shared object
	if( dontTranslate.MoveTo(0, func) ) return;

	asASSERT( func->scriptData );

	// Pre-compute the size of each instruction in order to translate jump offsets
	asUINT n;
	asDWORD *bc = func->scriptData->byteCode.AddressOf();
	asUINT bcLength = (asUINT)func->scriptData->byteCode.GetLength();
	asCArray<asUINT> bcSizes(bcLength);
	asCArray<asUINT> instructionNbrToPos(bcLength);
	for( n = 0; n < bcLength; )
	{
		int c = *(asBYTE*)&bc[n];
		asUINT size = asBCTypeSize[asBCInfo[c].type];
		if( size == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			return;
		}
		bcSizes.PushLast(size);
		instructionNbrToPos.PushLast(n);
		n += size;
	}

	asUINT bcNum = 0;
	for( n = 0; n < bcLength; bcNum++ )
	{
		int c = *(asBYTE*)&bc[n];
		if( c == asBC_REFCPY ||
			c == asBC_RefCpyV ||
			c == asBC_OBJTYPE )
		{
			// Translate the index to the true object type
			asPWORD *ot = (asPWORD*)&bc[n+1];
			*(asCObjectType**)ot = CastToObjectType(FindType(int(*ot)));
		}
		else if( c == asBC_TYPEID ||
			     c == asBC_Cast )
		{
			// Translate the index to the type id
			int *tid = (int*)&bc[n+1];
			*tid = FindTypeId(*tid);
		}
		else if( c == asBC_ADDSi ||
			     c == asBC_LoadThisR )
		{
			// Translate the index to the type id
			int *tid = (int*)&bc[n+1];
			*tid = FindTypeId(*tid);

			// Translate the prop index into the property offset
			*(((short*)&bc[n])+1) = FindObjectPropOffset(*(((short*)&bc[n])+1));
		}
		else if( c == asBC_LoadRObjR ||
			     c == asBC_LoadVObjR )
		{
			// Translate the index to the type id
			int *tid = (int*)&bc[n+2];
			*tid = FindTypeId(*tid);

			asCObjectType *ot = engine->GetObjectTypeFromTypeId(*tid);
			if( ot && (ot->flags & asOBJ_LIST_PATTERN) )
			{
				// List patterns have a different way of adjusting the offsets
				SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
				*(((short*)&bc[n])+2) = (short)listAdj->AdjustOffset(*(((short*)&bc[n])+2));
			}
			else
			{
				// Translate the prop index into the property offset
				*(((short*)&bc[n])+2) = FindObjectPropOffset(*(((short*)&bc[n])+2));
			}
		}
		else if( c == asBC_COPY )
		{
			// Translate the index to the type id
			int *tid = (int*)&bc[n+1];
			*tid = FindTypeId(*tid);

			// COPY is used to copy POD types that don't have the opAssign method. It is
			// also used to copy references to scoped types during variable initializations.
			// Update the number of dwords to copy as it may be different on the target platform
			if( (*tid) & asTYPEID_OBJHANDLE )
			{
				// It is the actual reference that is being copied, not the object itself
				asBC_SWORDARG0(&bc[n]) = AS_PTR_SIZE;
			}
			else
			{
				asCDataType dt = engine->GetDataTypeFromTypeId(*tid);
				if( !dt.IsValid() )
				{
					Error(TXT_INVALID_BYTECODE_d);
				}
				else
					asBC_SWORDARG0(&bc[n]) = (short)dt.GetSizeInMemoryDWords();
			}
		}
		else if( c == asBC_RET )
		{
			// Determine the correct amount of DWORDs to pop
			asWORD dw = (asWORD)func->GetSpaceNeededForArguments();
			if( func->DoesReturnOnStack() ) dw += AS_PTR_SIZE;
			if( func->objectType ) dw += AS_PTR_SIZE;
			asBC_WORDARG0(&bc[n]) = dw;
		}
		else if( c == asBC_CALL ||
				 c == asBC_CALLINTF ||
				 c == asBC_CALLSYS ||
				 c == asBC_Thiscall1 )
		{
			// Translate the index to the func id
			int *fid = (int*)&bc[n+1];
			asCScriptFunction *f = FindFunction(*fid);
			if( f )
				*fid = f->id;
			else
			{
				Error(TXT_INVALID_BYTECODE_d);
				return;
			}
		}
		else if( c == asBC_FuncPtr )
		{
			// Translate the index to the func pointer
			asPWORD *fid = (asPWORD*)&bc[n+1];
			*fid = (asPWORD)FindFunction(int(*fid));
		}
		else if( c == asBC_ALLOC )
		{
			// Translate the index to the true object type
			asPWORD *arg = (asPWORD*)&bc[n+1];
			*(asCObjectType**)arg = CastToObjectType(FindType(int(*arg)));

			// The constructor function id must be translated, unless it is zero
			int *fid = (int*)&bc[n+1+AS_PTR_SIZE];
			if( *fid != 0 )
			{
				// Subtract 1 from the id, as it was incremented during the writing
				asCScriptFunction *f = FindFunction(*fid-1);
				if( f )
					*fid = f->id;
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
					return;
				}
			}
		}
		else if( c == asBC_STR )
		{
			Error(TXT_INVALID_BYTECODE_d);
			return;
		}
		else if( c == asBC_CALLBND )
		{
			// Translate the function id
			asUINT *fid = (asUINT*)&bc[n+1];
			if( *fid < module->m_bindInformations.GetLength() )
			{
				sBindInfo *bi = module->m_bindInformations[*fid];
				if( bi )
					*fid = bi->importedFunctionSignature->id;
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
					return;
				}
			}
			else
			{
				Error(TXT_INVALID_BYTECODE_d);
				return;
			}
		}
		else if( c == asBC_PGA      ||
			     c == asBC_PshGPtr  ||
			     c == asBC_LDG      ||
				 c == asBC_PshG4    ||
				 c == asBC_LdGRdR4  ||
				 c == asBC_CpyGtoV4 ||
				 c == asBC_CpyVtoG4 ||
				 c == asBC_SetG4    )
		{
			// Translate the index to pointer
			asPWORD *index = (asPWORD*)&bc[n + 1];
			if ((*index & 1))
			{
				if ((asUINT(*index)>>1) < usedGlobalProperties.GetLength())
					*(void**)index = usedGlobalProperties[asUINT(*index)>>1];
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
					return;
				}
			}
			else
			{
				// Only PGA and PshGPtr can hold string constants
				asASSERT(c == asBC_PGA || c == asBC_PshGPtr);

				if ((asUINT(*index)>>1) < usedStringConstants.GetLength())
					*(void**)index = usedStringConstants[asUINT(*index)>>1];
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
					return;
				}
			}
		}
		else if( c == asBC_JMP    ||
			     c == asBC_JZ     ||
				 c == asBC_JNZ    ||
			     c == asBC_JLowZ  ||
				 c == asBC_JLowNZ ||
				 c == asBC_JS     ||
				 c == asBC_JNS    ||
				 c == asBC_JP     ||
				 c == asBC_JNP    ) // The JMPP instruction doesn't need modification
		{
			// Get the offset
			int offset = int(bc[n+1]);

			// Count the instruction sizes to the destination instruction
			int size = 0;
			if( offset >= 0 )
				// If moving ahead, then start from next instruction
				for( asUINT num = bcNum+1; offset-- > 0; num++ )
					size += bcSizes[num];
			else
				// If moving backwards, then start at current instruction
				for( asUINT num = bcNum; offset++ < 0; num-- )
					size -= bcSizes[num];

			// The size is dword offset
			bc[n+1] = size;
		}
		else if( c == asBC_AllocMem )
		{
			// The size of the allocated memory is only known after all the elements has been seen.
			// This helper class will collect this information and adjust the size when the
			// corresponding asBC_FREE is encountered

			// The adjuster also needs to know the list type so it can know the type of the elements
			asCObjectType *ot = CastToObjectType(func->GetTypeInfoOfLocalVar(asBC_SWORDARG0(&bc[n])));
			listAdjusters.PushLast(asNEW(SListAdjuster)(this, &bc[n], ot));
		}
		else if( c == asBC_FREE )
		{
			// Translate the index to the true object type
			asPWORD *pot = (asPWORD*)&bc[n+1];
			*(asCObjectType**)pot = CastToObjectType(FindType(int(*pot)));

			asCObjectType *ot = *(asCObjectType**)pot;
			if( ot && (ot->flags & asOBJ_LIST_PATTERN) )
			{
				if( listAdjusters.GetLength() == 0 )
				{
					Error(TXT_INVALID_BYTECODE_d);
					return;
				}

				// Finalize the adjustment of the list buffer that was initiated with asBC_AllocMem
				SListAdjuster *list = listAdjusters.PopLast();
				list->AdjustAllocMem();
				asDELETE(list, SListAdjuster);
			}
		}
		else if( c == asBC_SetListSize )
		{
			// Adjust the offset in the list where the size is informed
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			bc[n+1] = listAdj->AdjustOffset(bc[n+1]);

			// Inform the list adjuster how many values will be repeated
			listAdj->SetRepeatCount(bc[n+2]);
		}
		else if( c == asBC_PshListElmnt )
		{
			// Adjust the offset in the list where the size is informed
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			bc[n+1] = listAdj->AdjustOffset(bc[n+1]);
		}
		else if( c == asBC_SetListType )
		{
			// Adjust the offset in the list where the typeid is informed
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			bc[n+1] = listAdj->AdjustOffset(bc[n+1]);

			// Translate the type id
			bc[n+2] = FindTypeId(bc[n+2]);

			// Inform the list adjuster the type id of the next element
			listAdj->SetNextType(bc[n+2]);
		}

		n += asBCTypeSize[asBCInfo[c].type];
	}

	// Calculate the stack adjustments
	CalculateAdjustmentByPos(func);

	// Adjust all variable positions in the bytecode
	bc = func->scriptData->byteCode.AddressOf();
	for( n = 0; n < bcLength; )
	{
		int c = *(asBYTE*)&bc[n];
		switch( asBCInfo[c].type )
		{
		case asBCTYPE_wW_ARG:
		case asBCTYPE_rW_DW_ARG:
		case asBCTYPE_wW_QW_ARG:
		case asBCTYPE_rW_ARG:
		case asBCTYPE_wW_DW_ARG:
		case asBCTYPE_wW_W_ARG:
		case asBCTYPE_rW_QW_ARG:
		case asBCTYPE_rW_W_DW_ARG:
		case asBCTYPE_rW_DW_DW_ARG:
			{
				asBC_SWORDARG0(&bc[n]) = (short)AdjustStackPosition(asBC_SWORDARG0(&bc[n]));
			}
			break;

		case asBCTYPE_wW_rW_ARG:
		case asBCTYPE_wW_rW_DW_ARG:
		case asBCTYPE_rW_rW_ARG:
			{
				asBC_SWORDARG0(&bc[n]) = (short)AdjustStackPosition(asBC_SWORDARG0(&bc[n]));
				asBC_SWORDARG1(&bc[n]) = (short)AdjustStackPosition(asBC_SWORDARG1(&bc[n]));
			}
			break;

		case asBCTYPE_wW_rW_rW_ARG:
			{
				asBC_SWORDARG0(&bc[n]) = (short)AdjustStackPosition(asBC_SWORDARG0(&bc[n]));
				asBC_SWORDARG1(&bc[n]) = (short)AdjustStackPosition(asBC_SWORDARG1(&bc[n]));
				asBC_SWORDARG2(&bc[n]) = (short)AdjustStackPosition(asBC_SWORDARG2(&bc[n]));
			}
			break;

		default:
			// The other types don't treat variables so won't be modified
			break;
		}

		n += asBCTypeSize[asBCInfo[c].type];
	}

	// Adjust the space needed for local variables
	func->scriptData->variableSpace = AdjustStackPosition(func->scriptData->variableSpace);

	// Adjust the variable information. This will be used during the adjustment below
	for( n = 0; n < func->scriptData->variables.GetLength(); n++ )
	{
		func->scriptData->variables[n]->declaredAtProgramPos = instructionNbrToPos[func->scriptData->variables[n]->declaredAtProgramPos];
		func->scriptData->variables[n]->stackOffset = AdjustStackPosition(func->scriptData->variables[n]->stackOffset);
	}

	// objVariablePos
	for( n = 0; n < func->scriptData->objVariablePos.GetLength(); n++ )
		func->scriptData->objVariablePos[n] = AdjustStackPosition(func->scriptData->objVariablePos[n]);

	// Adjust the get offsets. This must be done in the second iteration because
	// it relies on the function ids and variable position already being correct in the
	// bytecodes that come after the GET instructions.
	// TODO: optimize: Instead of doing a full extra loop. We can push the GET instructions
	//                 on a stack, and then when a call instruction is found update all of them.
	//                 This will also make the AdjustGetOffset() function quicker as it can
	//                 receive the called function directly instead of having to search for it.
	bc = func->scriptData->byteCode.AddressOf();
	for( n = 0; n < bcLength; )
	{
		int c = *(asBYTE*)&bc[n];

		if( c == asBC_GETREF ||
		    c == asBC_GETOBJ ||
		    c == asBC_GETOBJREF ||
		    c == asBC_ChkNullS )
		{
			asBC_WORDARG0(&bc[n]) = (asWORD)AdjustGetOffset(asBC_WORDARG0(&bc[n]), func, n);
		}

		n += asBCTypeSize[asBCInfo[c].type];
	}

	for( n = 0; n < func->scriptData->objVariableInfo.GetLength(); n++ )
	{
		// The program position must be adjusted as it is stored in number of instructions
		func->scriptData->objVariableInfo[n].programPos = instructionNbrToPos[func->scriptData->objVariableInfo[n].programPos];
		func->scriptData->objVariableInfo[n].variableOffset = AdjustStackPosition(func->scriptData->objVariableInfo[n].variableOffset);
	}

	for (n = 0; n < func->scriptData->tryCatchInfo.GetLength(); n++)
	{
		func->scriptData->tryCatchInfo[n].tryPos = instructionNbrToPos[func->scriptData->tryCatchInfo[n].tryPos];
		func->scriptData->tryCatchInfo[n].catchPos = instructionNbrToPos[func->scriptData->tryCatchInfo[n].catchPos];
	}

	// The program position (every even number) needs to be adjusted
	// for the line numbers to be in number of dwords instead of number of instructions
	for( n = 0; n < func->scriptData->lineNumbers.GetLength(); n += 2 )
		func->scriptData->lineNumbers[n] = instructionNbrToPos[func->scriptData->lineNumbers[n]];
	for( n = 0; n < func->scriptData->sectionIdxs.GetLength(); n += 2 )
		func->scriptData->sectionIdxs[n] = instructionNbrToPos[func->scriptData->sectionIdxs[n]];

	CalculateStackNeeded(func);
}

asCReader::SListAdjuster::SListAdjuster(asCReader *rd, asDWORD *bc, asCObjectType *listType) :
	reader(rd), allocMemBC(bc), maxOffset(0), patternType(listType), repeatCount(0), lastOffset(-1), nextOffset(0), nextTypeId(-1)
{
	asASSERT( patternType && (patternType->flags & asOBJ_LIST_PATTERN) );

	// Find the first expected value in the list
	asSListPatternNode *node = patternType->engine->scriptFunctions[patternType->templateSubTypes[0].GetBehaviour()->listFactory]->listPattern;
	asASSERT( node && node->type == asLPT_START );
	patternNode = node->next;
}

int asCReader::SListAdjuster::AdjustOffset(int offset)
{
	if( offset < lastOffset )
	{
		reader->Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}

	// If it is the same offset being accessed again, just return the same adjusted value
	if( lastOffset == offset )
		return lastAdjustedOffset;

	lastOffset = offset;
	lastAdjustedOffset = maxOffset;

	// What is being expected at this position?
	if( patternNode->type == asLPT_REPEAT || patternNode->type == asLPT_REPEAT_SAME )
	{
		// Align the offset to 4 bytes boundary
		if( maxOffset & 0x3 )
		{
			maxOffset += 4 - (maxOffset & 0x3);
			lastAdjustedOffset = maxOffset;
		}

		// Don't move the patternNode yet because the caller must make a call to SetRepeatCount too
		maxOffset += 4;
		nextOffset = offset+1;
		return lastAdjustedOffset;
	}
	else if( patternNode->type == asLPT_TYPE )
	{
		const asCDataType &dt = reinterpret_cast<asSListPatternDataTypeNode*>(patternNode)->dataType;
		if( dt.GetTokenType() == ttQuestion )
		{
			if( nextTypeId != -1 )
			{
				if( repeatCount > 0 )
					repeatCount--;

				asCDataType nextdt = patternType->engine->GetDataTypeFromTypeId(nextTypeId);
				asUINT size;
				if(nextdt.IsObjectHandle() || (nextdt.GetTypeInfo() && (nextdt.GetTypeInfo()->flags & asOBJ_REF)) )
					size = AS_PTR_SIZE*4;
				else
					size = nextdt.GetSizeInMemoryBytes();

				// Align the offset to 4 bytes boundary
				if( size >= 4 && (maxOffset & 0x3) )
				{
					maxOffset += 4 - (maxOffset & 0x3);
					lastAdjustedOffset = maxOffset;
				}

				// Only move the patternNode if we're not expecting any more repeated entries
				if( repeatCount == 0 )
					patternNode = patternNode->next;

				nextTypeId = -1;

				maxOffset += size;
				nextOffset = offset+1;
				return lastAdjustedOffset;
			}
			else
			{
				// Align the offset to 4 bytes boundary
				if( maxOffset & 0x3 )
				{
					maxOffset += 4 - (maxOffset & 0x3);
					lastAdjustedOffset = maxOffset;
				}

				// The first adjustment is for the typeId
				maxOffset += 4;
				nextOffset = offset+1;
				return lastAdjustedOffset;
			}
		}
		else
		{
			// Determine the size of the element
			asUINT size;
			if( dt.IsObjectHandle() || (dt.GetTypeInfo() && (dt.GetTypeInfo()->flags & asOBJ_REF)) )
				size = AS_PTR_SIZE*4;
			else
				size = dt.GetSizeInMemoryBytes();

			// If values are skipped, the offset needs to be incremented
			while( nextOffset <= offset )
			{
				if( repeatCount > 0 )
					repeatCount--;

				// Align the offset to 4 bytes boundary
				if( size >= 4 && (maxOffset & 0x3) )
					maxOffset += 4 - (maxOffset & 0x3);

				lastAdjustedOffset = maxOffset;
				nextOffset += 1;
				maxOffset += size;
			}

			// Only move the patternNode if we're not expecting any more repeated entries
			if( repeatCount == 0 )
				patternNode = patternNode->next;

			nextOffset = offset+1;
			return lastAdjustedOffset;
		}
	}
	else if( patternNode->type == asLPT_START )
	{
		if( repeatCount > 0 )
			repeatCount--;
		SInfo info = {repeatCount, patternNode};
		stack.PushLast(info);

		repeatCount = 0;
		patternNode = patternNode->next;

		lastOffset--;
		return AdjustOffset(offset);
	}
	else if( patternNode->type == asLPT_END )
	{
		if( stack.GetLength() == 0 )
		{
			reader->Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}

		SInfo info = stack.PopLast();
		repeatCount = info.repeatCount;
		if( repeatCount )
			patternNode = info.startNode;
		else
			patternNode = patternNode->next;

		lastOffset--;
		return AdjustOffset(offset);
	}
	else
	{
		// Something is wrong with the pattern list declaration
		reader->Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}

	UNREACHABLE_RETURN;
}

void asCReader::SListAdjuster::SetRepeatCount(asUINT rc)
{
	// Make sure the list is expecting a repeat at this location
	asASSERT( patternNode->type == asLPT_REPEAT || patternNode->type == asLPT_REPEAT_SAME );

	// Now move to the next patternNode
	patternNode = patternNode->next;

	repeatCount = rc;
}

void asCReader::SListAdjuster::AdjustAllocMem()
{
	allocMemBC[1] = maxOffset;
}

void asCReader::SListAdjuster::SetNextType(int typeId)
{
	asASSERT( nextTypeId == -1 );

	nextTypeId = typeId;
}

void asCReader::CalculateStackNeeded(asCScriptFunction *func)
{
	asASSERT( func->scriptData );

	int largestStackUsed = 0;

	// Clear the known stack size for each bytecode
	asCArray<int> stackSize;
	stackSize.SetLength(func->scriptData->byteCode.GetLength());
	memset(&stackSize[0], -1, stackSize.GetLength()*4);

	// Add the first instruction to the list of unchecked code
	// paths and set the stack size at that instruction to variableSpace
	asCArray<asUINT> paths;
	paths.PushLast(0);
	stackSize[0] = func->scriptData->variableSpace;

	// Go through each of the code paths
	for( asUINT p = 0; p < paths.GetLength(); ++p )
	{
		asUINT pos = paths[p];
		int currStackSize = stackSize[pos];

		asBYTE bc = *(asBYTE*)&func->scriptData->byteCode[pos];
		if( bc == asBC_RET )
			continue;

		// Determine the change in stack size for this instruction
		int stackInc = asBCInfo[bc].stackInc;
		if( stackInc == 0xFFFF )
		{
			// Determine the true delta from the instruction arguments
			if( bc == asBC_CALL ||
				bc == asBC_CALLSYS ||
				bc == asBC_Thiscall1 ||
				bc == asBC_CALLBND ||
				bc == asBC_ALLOC ||
				bc == asBC_CALLINTF ||
				bc == asBC_CallPtr )
			{
				asCScriptFunction *called = GetCalledFunction(func, pos);
				if( called )
				{
					stackInc = -called->GetSpaceNeededForArguments();
					if( called->objectType )
						stackInc -= AS_PTR_SIZE;
					if( called->DoesReturnOnStack() )
						stackInc -= AS_PTR_SIZE;
				}
				else
				{
					// It is an allocation for an object without a constructor
					asASSERT( bc == asBC_ALLOC );
					stackInc = -AS_PTR_SIZE;
				}
			}
		}

		currStackSize += stackInc;
		asASSERT( currStackSize >= 0 );

		if( currStackSize > largestStackUsed )
			largestStackUsed = currStackSize;

		if( bc == asBC_JMP )
		{
			// Find the label that we should jump to
			int offset = asBC_INTARG(&func->scriptData->byteCode[pos]);
			pos += 2 + offset;

			// Add the destination as a new path
			if( stackSize[pos] == -1 )
			{
				stackSize[pos] = currStackSize;
				paths.PushLast(pos);
			}
			else
				asASSERT(stackSize[pos] == currStackSize);
			continue;
		}
		else if( bc == asBC_JZ    || bc == asBC_JNZ    ||
				 bc == asBC_JLowZ || bc == asBC_JLowNZ ||
				 bc == asBC_JS    || bc == asBC_JNS    ||
				 bc == asBC_JP    || bc == asBC_JNP )
		{
			// Find the label that is being jumped to
			int offset = asBC_INTARG(&func->scriptData->byteCode[pos]);

			// Add both paths to the code paths
			pos += 2;
			if( stackSize[pos] == -1 )
			{
				stackSize[pos] = currStackSize;
				paths.PushLast(pos);
			}
			else
				asASSERT(stackSize[pos] == currStackSize);

			pos += offset;
			if( stackSize[pos] == -1 )
			{
				stackSize[pos] = currStackSize;
				paths.PushLast(pos);
			}
			else
				asASSERT(stackSize[pos] == currStackSize);

			continue;
		}
		else if( bc == asBC_JMPP )
		{
			pos++;

			// Add all subsequent JMP instructions to the path
			while( *(asBYTE*)&func->scriptData->byteCode[pos] == asBC_JMP )
			{
				if( stackSize[pos] == -1 )
				{
					stackSize[pos] = currStackSize;
					paths.PushLast(pos);
				}
				else
					asASSERT(stackSize[pos] == currStackSize);
				pos += 2;
			}
			continue;
		}
		else
		{
			// Add next instruction to the paths
			pos += asBCTypeSize[asBCInfo[bc].type];
			if( stackSize[pos] == -1 )
			{
				stackSize[pos] = currStackSize;
				paths.PushLast(pos);
			}
			else
				asASSERT(stackSize[pos] == currStackSize);

			continue;
		}
	}

	func->scriptData->stackNeeded = largestStackUsed;
}

void asCReader::CalculateAdjustmentByPos(asCScriptFunction *func)
{
	// Adjust the offset of all negative variables (parameters) as
	// all pointers have been stored as having a size of 1 dword
	asUINT n;
	asCArray<int> adjustments;
	asUINT offset = 0;
	if( func->objectType )
	{
		adjustments.PushLast(offset);
		adjustments.PushLast(1-AS_PTR_SIZE);
		offset += 1;
	}
	if( func->DoesReturnOnStack() )
	{
		adjustments.PushLast(offset);
		adjustments.PushLast(1-AS_PTR_SIZE);
		offset += 1;
	}
	for( n = 0; n < func->parameterTypes.GetLength(); n++ )
	{
		if( !func->parameterTypes[n].IsPrimitive() ||
			func->parameterTypes[n].IsReference() )
		{
			adjustments.PushLast(offset);
			adjustments.PushLast(1-AS_PTR_SIZE);
			offset += 1;
		}
		else
		{
			asASSERT( func->parameterTypes[n].IsPrimitive() );
			offset += func->parameterTypes[n].GetSizeOnStackDWords();
		}
	}

	// Build look-up table with the adjustments for each stack position
	adjustNegativeStackByPos.SetLength(offset);
	memset(adjustNegativeStackByPos.AddressOf(), 0, adjustNegativeStackByPos.GetLength()*sizeof(int));
	for( n = 0; n < adjustments.GetLength(); n+=2 )
	{
		int pos    = adjustments[n];
		int adjust = adjustments[n+1];

		for( asUINT i = pos+1; i < adjustNegativeStackByPos.GetLength(); i++ )
			adjustNegativeStackByPos[i] += adjust;
	}

	// The bytecode has been stored as if all object variables take up only 1 dword.
	// It is necessary to adjust to the size according to the current platform.
	adjustments.SetLength(0);
	int highestPos = 0;
	for( n = 0; n < func->scriptData->objVariableTypes.GetLength(); n++ )
	{
		// Determine the size the variable currently occupies on the stack
		int size = AS_PTR_SIZE;

		// objVariableTypes is null if the type is a null pointer
		if( func->scriptData->objVariableTypes[n] &&
			(func->scriptData->objVariableTypes[n]->GetFlags() & asOBJ_VALUE) &&
			n >= func->scriptData->objVariablesOnHeap )
		{
			size = func->scriptData->objVariableTypes[n]->GetSize();
			if( size < 4 )
				size = 1;
			else
				size /= 4;
		}

		// Check if type has a different size than stored
		if( size > 1 )
		{
			if( func->scriptData->objVariablePos[n] > highestPos )
				highestPos = func->scriptData->objVariablePos[n];

			adjustments.PushLast(func->scriptData->objVariablePos[n]);
			adjustments.PushLast(size-1);
		}
	}

	// Count position 0 too
	adjustByPos.SetLength(highestPos+1);
	memset(adjustByPos.AddressOf(), 0, adjustByPos.GetLength()*sizeof(int));

	// Build look-up table with the adjustments for each stack position
	for( n = 0; n < adjustments.GetLength(); n+=2 )
	{
		int pos    = adjustments[n];
		int adjust = adjustments[n+1];

		for( asUINT i = pos; i < adjustByPos.GetLength(); i++ )
			adjustByPos[i] += adjust;
	}
}

int asCReader::AdjustStackPosition(int pos)
{
	if( pos >= (int)adjustByPos.GetLength() )
	{
		// It can be higher for primitives allocated on top of highest object variable
		if( adjustByPos.GetLength() )
			pos += (short)adjustByPos[adjustByPos.GetLength()-1];
	}
	else if( pos >= 0 )
		pos += (short)adjustByPos[pos];
	else if( -pos >= (int)adjustNegativeStackByPos.GetLength() )
		Error(TXT_INVALID_BYTECODE_d);
	else
		pos += (short)adjustNegativeStackByPos[-pos];

	return pos;
}

asCScriptFunction *asCReader::GetCalledFunction(asCScriptFunction *func, asDWORD programPos)
{
	asBYTE bc = *(asBYTE*)&func->scriptData->byteCode[programPos];

	if( bc == asBC_CALL ||
		bc == asBC_CALLSYS ||
		bc == asBC_Thiscall1 ||
		bc == asBC_CALLINTF )
	{
		// Find the function from the function id in bytecode
		int funcId = asBC_INTARG(&func->scriptData->byteCode[programPos]);
		return engine->scriptFunctions[funcId];
	}
	else if( bc == asBC_ALLOC )
	{
		// Find the function from the function id in the bytecode
		int funcId = asBC_INTARG(&func->scriptData->byteCode[programPos+AS_PTR_SIZE]);
		return engine->scriptFunctions[funcId];
	}
	else if( bc == asBC_CALLBND )
	{
		// Find the function from the engine's bind array
		int funcId = asBC_INTARG(&func->scriptData->byteCode[programPos]);
		return engine->importedFunctions[funcId & ~FUNC_IMPORTED]->importedFunctionSignature;
	}
	else if( bc == asBC_CallPtr )
	{
		asUINT v;
		int var = asBC_SWORDARG0(&func->scriptData->byteCode[programPos]);

		// Find the funcdef from the local variable
		for( v = 0; v < func->scriptData->objVariablePos.GetLength(); v++ )
			if( func->scriptData->objVariablePos[v] == var )
				return CastToFuncdefType(func->scriptData->objVariableTypes[v])->funcdef;

		// Look in parameters
		int paramPos = 0;
		if( func->objectType )
			paramPos -= AS_PTR_SIZE;
		if( func->DoesReturnOnStack() )
			paramPos -= AS_PTR_SIZE;
		for( v = 0; v < func->parameterTypes.GetLength(); v++ )
		{
			if (var == paramPos)
			{
				if (func->parameterTypes[v].IsFuncdef())
					return CastToFuncdefType(func->parameterTypes[v].GetTypeInfo())->funcdef;
				else
				{
					error = true;
					return 0;
				}
			}
			paramPos -= func->parameterTypes[v].GetSizeOnStackDWords();
		}
	}

	return 0;
}

int asCReader::AdjustGetOffset(int offset, asCScriptFunction *func, asDWORD programPos)
{
	// TODO: optimize: multiple instructions for the same function doesn't need to look for the function everytime
	//                 the function can remember where it found the function and check if the programPos is still valid

	// Get offset 0 doesn't need adjustment
	if( offset == 0 ) return 0;

	bool bcAlloc = false;

	// Find out which function that will be called
	asCScriptFunction *calledFunc = 0;
	int stackDelta = 0;
	for( asUINT n = programPos; func->scriptData->byteCode.GetLength(); )
	{
		asBYTE bc = *(asBYTE*)&func->scriptData->byteCode[n];
		if( bc == asBC_CALL ||
			bc == asBC_CALLSYS ||
			bc == asBC_Thiscall1 ||
			bc == asBC_CALLINTF ||
			bc == asBC_ALLOC ||
			bc == asBC_CALLBND ||
			bc == asBC_CallPtr )
		{
			// The alloc instruction allocates the object memory
			// so it doesn't take the this pointer as input
			if (bc == asBC_ALLOC)
				bcAlloc = true;

			calledFunc = GetCalledFunction(func, n);
			break;
		}
		else if( bc == asBC_REFCPY ||
				 bc == asBC_COPY )
		{
			// In this case we know there is only 1 pointer on the stack above
			asASSERT( offset == 1 );
			return offset - (1 - AS_PTR_SIZE);
		}

		// Keep track of the stack size between the
		// instruction that needs to be adjusted and the call
		stackDelta += asBCInfo[bc].stackInc;

		n += asBCTypeSize[asBCInfo[bc].type];
	}

	if( calledFunc == 0 )
	{
		Error(TXT_INVALID_BYTECODE_d);
		return offset;
	}

	// Count the number of pointers pushed on the stack above the
	// current offset, and then adjust the offset accordingly
	asUINT numPtrs = 0;
	int currOffset = -stackDelta;
	if( offset > currOffset && calledFunc->GetObjectType() && !bcAlloc )
	{
		currOffset++;
		if( currOffset > 0 )
			numPtrs++;
#if AS_PTR_SIZE == 2
		// For 64bit platforms it is necessary to increment the currOffset by one more
		// DWORD since the stackDelta was counting the full 64bit size of the pointer
		else if( stackDelta )
			currOffset++;
#endif
	}
	if( offset > currOffset && calledFunc->DoesReturnOnStack() )
	{
		currOffset++;
		if( currOffset > 0 )
			numPtrs++;
#if AS_PTR_SIZE == 2
		// For 64bit platforms it is necessary to increment the currOffset by one more
		// DWORD since the stackDelta was counting the full 64bit size of the pointer
		else if( stackDelta )
			currOffset++;
#endif
	}
	for( asUINT p = 0; p < calledFunc->parameterTypes.GetLength(); p++ )
	{
		if( offset <= currOffset ) break;

		if( !calledFunc->parameterTypes[p].IsPrimitive() ||
			calledFunc->parameterTypes[p].IsReference() )
		{
			currOffset++;
			if( currOffset > 0 )
				numPtrs++;
#if AS_PTR_SIZE == 2
			// For 64bit platforms it is necessary to increment the currOffset by one more
			// DWORD since the stackDelta was counting the full 64bit size of the pointer
			else if( stackDelta )
				currOffset++;
#endif

			// The variable arg ? has an additiona 32bit integer with the typeid
			if( calledFunc->parameterTypes[p].IsAnyType() )
				currOffset += 1;
		}
		else
		{
			// Enums or built-in primitives are passed by value
			asASSERT( calledFunc->parameterTypes[p].IsPrimitive() );
			currOffset += calledFunc->parameterTypes[p].GetSizeOnStackDWords();
		}
	}

	return offset - numPtrs * (1 - AS_PTR_SIZE);
}

int asCReader::FindTypeId(int idx)
{
	if( idx >= 0 && idx < (int)usedTypeIds.GetLength() )
		return usedTypeIds[idx];
	else
	{
		Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}
}

asCTypeInfo *asCReader::FindType(int idx)
{
	if( idx < 0 || idx >= (int)usedTypes.GetLength() )
	{
		Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}

	return usedTypes[idx];
}

#ifndef AS_NO_COMPILER

asCWriter::asCWriter(asCModule* _module, asIBinaryStream* _stream, asCScriptEngine* _engine, bool _stripDebug)
	: module(_module), stream(_stream), engine(_engine), stripDebugInfo(_stripDebug), error(false), bytesWritten(0)
{
}

int asCWriter::Error(const char *msg)
{
	// Don't write if it has already been reported an error earlier
	if (!error)
	{
		asCString str;
		str.Format(msg, bytesWritten);
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		error = true;
	}

	return asERROR;
}

int asCWriter::WriteData(const void *data, asUINT size)
{
	asASSERT(size == 1 || size == 2 || size == 4 || size == 8);
	int ret = 0;
#if defined(AS_BIG_ENDIAN)
	for( asUINT n = 0; ret >= 0 && n < size; n++ )
		ret = stream->Write(((asBYTE*)data)+n, 1);
#else
	for( int n = size-1; ret >= 0 && n >= 0; n-- )
		ret = stream->Write(((asBYTE*)data)+n, 1);
#endif
	if (ret < 0)
		Error(TXT_UNEXPECTED_END_OF_FILE);
	bytesWritten += size;
	return ret;
}

int asCWriter::Write()
{
	TimeIt("asCWriter::Write");

	unsigned long i, count;

	// Store everything in the same order that the builder parses scripts

	// TODO: Should be possible to skip saving the enum values. They are usually not needed after the script is compiled anyway
	// TODO: Should be possible to skip saving the typedefs. They are usually not needed after the script is compiled anyway
	// TODO: Should be possible to skip saving constants. They are usually not needed after the script is compiled anyway

	// Write the flag as 1byte even on platforms with 4byte booleans
	WriteEncodedInt64(stripDebugInfo ? 1 : 0);

	// Store enums
	{
		TimeIt("store enums");

		count = (asUINT)module->m_enumTypes.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; i++ )
		{
			WriteTypeDeclaration(module->m_enumTypes[i], 1);
			WriteTypeDeclaration(module->m_enumTypes[i], 2);
		}
	}

	// Store type declarations first
	{
		TimeIt("type declarations");

		count = (asUINT)module->m_classTypes.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; i++ )
		{
			// Store only the name of the class/interface types
			WriteTypeDeclaration(module->m_classTypes[i], 1);
		}
	}

	// Store func defs
	{
		TimeIt("func defs");

		count = (asUINT)module->m_funcDefs.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; i++ )
			WriteFunction(module->m_funcDefs[i]->funcdef);
	}

	// Now store all interface methods
	{
		TimeIt("interface methods");

		count = (asUINT)module->m_classTypes.GetLength();
		for( i = 0; i < count; i++ )
		{
			if( module->m_classTypes[i]->IsInterface() )
				WriteTypeDeclaration(module->m_classTypes[i], 2);
		}
	}

	// Then store the class methods and behaviours
	{
		TimeIt("class methods and behaviours");

		for( i = 0; i < count; ++i )
		{
			if( !module->m_classTypes[i]->IsInterface() )
				WriteTypeDeclaration(module->m_classTypes[i], 2);
		}
	}

	// Then store the class properties
	{
		TimeIt("class properties");

		for( i = 0; i < count; ++i )
		{
			if( !module->m_classTypes[i]->IsInterface() )
				WriteTypeDeclaration(module->m_classTypes[i], 3);
		}
	}

	// Store typedefs
	{
		TimeIt("type defs");

		count = (asUINT)module->m_typeDefs.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; i++ )
		{
			WriteTypeDeclaration(module->m_typeDefs[i], 1);
			WriteTypeDeclaration(module->m_typeDefs[i], 2);
		}
	}

	// scriptGlobals[]
	{
		TimeIt("script globals");

		count = (asUINT)module->m_scriptGlobals.GetSize();
		WriteEncodedInt64(count);
		asCSymbolTable<asCGlobalProperty>::iterator it = module->m_scriptGlobals.List();
		for( ; it; it++ )
			WriteGlobalProperty(*it);
	}

	// scriptFunctions[]
	{
		TimeIt("scriptFunctions");

		count = 0;
		for( i = 0; i < module->m_scriptFunctions.GetLength(); i++ )
			if( module->m_scriptFunctions[i]->objectType == 0 )
				count++;
		WriteEncodedInt64(count);
		for( i = 0; i < module->m_scriptFunctions.GetLength(); ++i )
			if( module->m_scriptFunctions[i]->objectType == 0 )
				WriteFunction(module->m_scriptFunctions[i]);
	}

	// globalFunctions[]
	{
		TimeIt("globalFunctions");

		count = (int)module->m_globalFunctions.GetSize();
		asCSymbolTable<asCScriptFunction>::iterator funcIt = module->m_globalFunctions.List();
		WriteEncodedInt64(count);
		while( funcIt )
		{
			WriteFunction(*funcIt);
			funcIt++;
		}
	}

	// bindInformations[]
	{
		TimeIt("bindInformations");

		count = (asUINT)module->m_bindInformations.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; ++i )
		{
			WriteFunction(module->m_bindInformations[i]->importedFunctionSignature);
			WriteString(&module->m_bindInformations[i]->importFromModule);
		}
	}

	// usedTypes[]
	{
		TimeIt("usedTypes");

		count = (asUINT)usedTypes.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; ++i )
			WriteTypeInfo(usedTypes[i]);
	}

	// usedTypeIds[]
	WriteUsedTypeIds();

	// usedFunctions[]
	WriteUsedFunctions();

	// usedGlobalProperties[]
	WriteUsedGlobalProps();

	// usedStringConstants[]
	WriteUsedStringConstants();

	// usedObjectProperties[]
	WriteUsedObjectProps();

	return error ? asERROR : asSUCCESS;
}

int asCWriter::FindStringConstantIndex(void *str)
{
	asSMapNode<void*, int> *cursor = 0;
	if (stringToIndexMap.MoveTo(&cursor, str))
		return cursor->value;

	usedStringConstants.PushLast(str);
	int index = int(usedStringConstants.GetLength() - 1);
	stringToIndexMap.Insert(str, index);
	return index;
}

void asCWriter::WriteUsedStringConstants()
{
	TimeIt("asCWriter::WriteUsedStringConstants");

	asUINT count = (asUINT)usedStringConstants.GetLength();
	WriteEncodedInt64(count);

	asCString str;
	for (asUINT i = 0; i < count; ++i)
	{
		asUINT length;
		engine->stringFactory->GetRawStringData(usedStringConstants[i], 0, &length);
		str.SetLength(length);
		engine->stringFactory->GetRawStringData(usedStringConstants[i], str.AddressOf(), &length);
		WriteString(&str);
	}
}

void asCWriter::WriteUsedFunctions()
{
	TimeIt("asCWriter::WriteUsedFunctions");

	asUINT count = (asUINT)usedFunctions.GetLength();
	WriteEncodedInt64(count);

	for( asUINT n = 0; n < usedFunctions.GetLength(); n++ )
	{
		char c;

		// Write enough data to be able to uniquely identify the function upon load
		asCScriptFunction *func = usedFunctions[n];
		if(func)
		{
			// Is the function from the module or the application?
			c = func->module ? 'm' : 'a';

			// Functions and methods that are shared should be stored as 's' as the bytecode
			// may be imported from other modules (even if the current module have received ownership)
			if (c == 'm' && func->IsShared() )
				c = 's';

			WriteData(&c, 1);
			WriteFunctionSignature(func);
		}
		else
		{
			// null function pointer
			c = 'n';
			WriteData(&c, 1);
		}
	}
}

void asCWriter::WriteFunctionSignature(asCScriptFunction *func)
{
	asUINT i, count;

	WriteString(&func->name);
	if( func->name == DELEGATE_FACTORY )
	{
		// It's not necessary to write anything else
		return;
	}

	WriteDataType(&func->returnType);

	count = (asUINT)func->parameterTypes.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; ++i )
		WriteDataType(&func->parameterTypes[i]);

	// Only write the inout flags if any of them are set
	// If the number of parameters is 0, then no need to save this
	if (func->parameterTypes.GetLength() > 0)
	{
		count = 0;
		for (i = asUINT(func->inOutFlags.GetLength()); i > 0; i--)
			if (func->inOutFlags[i - 1] != asTM_NONE)
			{
				count = i;
				break;
			}
		WriteEncodedInt64(count);
		for (i = 0; i < count; ++i)
			WriteEncodedInt64(func->inOutFlags[i]);
	}

	WriteEncodedInt64(func->funcType);

	// Write the default args, from last to first
	// If the number of parameters is 0, then no need to save this
	if (func->parameterTypes.GetLength() > 0)
	{
		count = 0;
		for (i = (asUINT)func->defaultArgs.GetLength(); i-- > 0; )
			if (func->defaultArgs[i])
				count++;
		WriteEncodedInt64(count);
		for (i = (asUINT)func->defaultArgs.GetLength(); i-- > 0; )
			if (func->defaultArgs[i])
				WriteString(func->defaultArgs[i]);
	}

	WriteTypeInfo(func->objectType);

	if( func->objectType )
	{
		asBYTE b = 0;
		b += func->IsReadOnly() ? 1 : 0;
		b += func->IsPrivate() ? 2 : 0;
		b += func->IsProtected() ? 4 : 0;
		WriteData(&b, 1);
	}
	else
	{
		if (func->funcType == asFUNC_FUNCDEF)
		{
			if (func->nameSpace)
			{
				// This funcdef was declared as global entity
				asBYTE b = 'n';
				WriteData(&b, 1);
				WriteString(&func->nameSpace->name);
			}
			else
			{
				// This funcdef was declared as class member
				asBYTE b = 'o';
				WriteData(&b, 1);
				WriteTypeInfo(func->funcdefType->parentClass);
			}
		}
		else
			WriteString(&func->nameSpace->name);
	}
}

void asCWriter::WriteFunction(asCScriptFunction* func)
{
	char c;

	// If there is no function, then store a null char
	if( func == 0 )
	{
		c = '\0';
		WriteData(&c, 1);
		return;
	}

	// First check if the function has been saved already
	for( asUINT f = 0; f < savedFunctions.GetLength(); f++ )
	{
		if( savedFunctions[f] == func )
		{
			c = 'r';
			WriteData(&c, 1);
			WriteEncodedInt64(f);
			return;
		}
	}

	// Keep a reference to the function in the list
	savedFunctions.PushLast(func);

	c = 'f';
	WriteData(&c, 1);

	asUINT i, count;

	WriteFunctionSignature(func);

	if( func->funcType == asFUNC_SCRIPT )
	{
		// Skip this for external shared entities
		if (module->m_externalTypes.IndexOf(func->objectType) >= 0)
			return;

		char bits = 0;
		bits += func->IsShared() ? 1 : 0;
		bits += func->dontCleanUpOnException ? 2 : 0;
		if (module->m_externalFunctions.IndexOf(func) >= 0)
			bits += 4;
		if (func->scriptData->objVariablePos.GetLength() || func->scriptData->objVariableInfo.GetLength())
			bits += 8;
		if (func->scriptData->tryCatchInfo.GetLength())
			bits += 16;
		bits += func->IsExplicit() ? 32 : 0;
		WriteData(&bits, 1);

		// For external shared functions the rest is not needed
		if (bits & 4)
			return;

		// Calculate the adjustment by position lookup table
		CalculateAdjustmentByPos(func);

		WriteByteCode(func);

		asDWORD varSpace = AdjustStackPosition(func->scriptData->variableSpace);
		WriteEncodedInt64(varSpace);

		if (bits & 8)
		{
			count = (asUINT)func->scriptData->objVariablePos.GetLength();
			WriteEncodedInt64(count);
			for (i = 0; i < count; ++i)
			{
				WriteTypeInfo(func->scriptData->objVariableTypes[i]);
				WriteEncodedInt64(AdjustStackPosition(func->scriptData->objVariablePos[i]));
			}
			if (count > 0)
				WriteEncodedInt64(func->scriptData->objVariablesOnHeap);

			WriteEncodedInt64((asUINT)func->scriptData->objVariableInfo.GetLength());
			for (i = 0; i < func->scriptData->objVariableInfo.GetLength(); ++i)
			{
				// The program position must be adjusted to be in number of instructions
				WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->objVariableInfo[i].programPos]);
				WriteEncodedInt64(AdjustStackPosition(func->scriptData->objVariableInfo[i].variableOffset));
				WriteEncodedInt64(func->scriptData->objVariableInfo[i].option);
			}
		}

		if (bits & 16)
		{
			// Write info on try/catch blocks
			WriteEncodedInt64((asUINT)func->scriptData->tryCatchInfo.GetLength());
			for (i = 0; i < func->scriptData->tryCatchInfo.GetLength(); ++i)
			{
				// The program position must be adjusted to be in number of instructions
				WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->tryCatchInfo[i].tryPos]);
				WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->tryCatchInfo[i].catchPos]);
			}
		}

		// The program position (every even number) needs to be adjusted
		// to be in number of instructions instead of DWORD offset
		if( !stripDebugInfo )
		{
			asUINT length = (asUINT)func->scriptData->lineNumbers.GetLength();
			WriteEncodedInt64(length);
			for( i = 0; i < length; ++i )
			{
				if( (i & 1) == 0 )
					WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->lineNumbers[i]]);
				else
					WriteEncodedInt64(func->scriptData->lineNumbers[i]);
			}

			// Write the array of script sections
			length = (asUINT)func->scriptData->sectionIdxs.GetLength();
			WriteEncodedInt64(length);
			for( i = 0; i < length; ++i )
			{
				if( (i & 1) == 0 )
					WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->sectionIdxs[i]]);
				else
				{
					if( func->scriptData->sectionIdxs[i] >= 0 )
						WriteString(engine->scriptSectionNames[func->scriptData->sectionIdxs[i]]);
					else
					{
						c = 0;
						WriteData(&c, 1);
					}
				}
			}
		}

		// Write the variable information
		if( !stripDebugInfo )
		{
			WriteEncodedInt64((asUINT)func->scriptData->variables.GetLength());
			for( i = 0; i < func->scriptData->variables.GetLength(); i++ )
			{
				// The program position must be adjusted to be in number of instructions
				WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->variables[i]->declaredAtProgramPos]);
				// The stack position must be adjusted according to the pointer sizes
				WriteEncodedInt64(AdjustStackPosition(func->scriptData->variables[i]->stackOffset));
				WriteString(&func->scriptData->variables[i]->name);
				WriteDataType(&func->scriptData->variables[i]->type);
			}
		}

		// Store script section name
		if( !stripDebugInfo )
		{
			if( func->scriptData->scriptSectionIdx >= 0 )
				WriteString(engine->scriptSectionNames[func->scriptData->scriptSectionIdx]);
			else
			{
				c = 0;
				WriteData(&c, 1);
			}
			WriteEncodedInt64(func->scriptData->declaredAt);
		}

		// Store the parameter names
		if( !stripDebugInfo )
		{
			count = asUINT(func->parameterNames.GetLength());
			WriteEncodedInt64(count);
			for( asUINT n = 0; n < count; n++ )
				WriteString(&func->parameterNames[n]);
		}
	}
	else if( func->funcType == asFUNC_VIRTUAL || func->funcType == asFUNC_INTERFACE )
	{
		// TODO: Do we really need to store this? It can probably be reconstructed by the reader
		WriteEncodedInt64(func->vfTableIdx);
	}
	else if( func->funcType == asFUNC_FUNCDEF )
	{
		char bits = 0;
		bits += func->IsShared() ? 1 : 0;
		if (module->m_externalTypes.IndexOf(func->funcdefType) >= 0)
			bits += 2;
		WriteData(&bits,1);
	}
}

void asCWriter::WriteTypeDeclaration(asCTypeInfo *type, int phase)
{
	if( phase == 1 )
	{
		// name
		WriteString(&type->name);
		// flags
		WriteData(&type->flags, 4);

		// size
		// TODO: Do we really need to store this? The reader should be able to
		//       determine the correct size from the object type's flags
		if( (type->flags & asOBJ_SCRIPT_OBJECT) && type->size > 0 )
		{
			// The size for script objects may vary from platform to platform so
			// only store 1 to diferentiate from interfaces that have size 0.
			WriteEncodedInt64(1);
		}
		else
		{
			// Enums, typedefs, and interfaces have fixed sizes independently
			// of platform so it is safe to serialize the size directly.
			WriteEncodedInt64(type->size);
		}

		// namespace
		WriteString(&type->nameSpace->name);

		// external shared flag
		if ((type->flags & asOBJ_SHARED))
		{
			char c = ' ';
			if (module->m_externalTypes.IndexOf(type) >= 0)
				c = 'e';
			WriteData(&c, 1);
		}
	}
	else if( phase == 2 )
	{
		// external shared types doesn't need to save this
		if ((type->flags & asOBJ_SHARED) && module->m_externalTypes.IndexOf(type) >= 0)
			return;

		if(type->flags & asOBJ_ENUM )
		{
			// enumValues[]
			asCEnumType *t = CastToEnumType(type);
			int size = (int)t->enumValues.GetLength();
			WriteEncodedInt64(size);

			for( int n = 0; n < size; n++ )
			{
				WriteString(&t->enumValues[n]->name);
				WriteData(&t->enumValues[n]->value, 4);
			}
		}
		else if(type->flags & asOBJ_TYPEDEF )
		{
			asCTypedefType *td = CastToTypedefType(type);
			eTokenType t = td->aliasForType.GetTokenType();
			WriteEncodedInt64(t);
		}
		else
		{
			asCObjectType *t = CastToObjectType(type);
			WriteTypeInfo(t->derivedFrom);

			// interfaces[] / interfaceVFTOffsets[]
			// TOOD: Is it really necessary to store the VFTOffsets? Can't the reader calculate those?
			int size = (asUINT)t->interfaces.GetLength();
			WriteEncodedInt64(size);
			asUINT n;
			asASSERT( t->IsInterface() || t->interfaces.GetLength() == t->interfaceVFTOffsets.GetLength() );
			for( n = 0; n < t->interfaces.GetLength(); n++ )
			{
				WriteTypeInfo(t->interfaces[n]);
				if( !t->IsInterface() )
					WriteEncodedInt64(t->interfaceVFTOffsets[n]);
			}

			// behaviours
			// TODO: Default behaviours should just be stored as a indicator
			//       to avoid storing the actual function object
			if( !t->IsInterface() && type->flags != asOBJ_TYPEDEF && type->flags != asOBJ_ENUM )
			{
				WriteFunction(engine->scriptFunctions[t->beh.destruct]);
				size = (int)t->beh.constructors.GetLength();
				WriteEncodedInt64(size);
				for( n = 0; n < t->beh.constructors.GetLength(); n++ )
				{
					WriteFunction(engine->scriptFunctions[t->beh.constructors[n]]);
					WriteFunction(engine->scriptFunctions[t->beh.factories[n]]);
				}
			}

			// methods[]
			// TODO: Avoid storing inherited methods in interfaces, as the reader
			//       can add those directly from the base interface
			size = (int)t->methods.GetLength();
			WriteEncodedInt64(size);
			for( n = 0; n < t->methods.GetLength(); n++ )
			{
				WriteFunction(engine->scriptFunctions[t->methods[n]]);
			}

			// virtualFunctionTable[]
			// TODO: Is it really necessary to store this? Can't it be easily rebuilt by the reader
			size = (int)t->virtualFunctionTable.GetLength();
			WriteEncodedInt64(size);
			for( n = 0; n < (asUINT)size; n++ )
			{
				WriteFunction(t->virtualFunctionTable[n]);
			}
		}
	}
	else if( phase == 3 )
	{
		// external shared types doesn't need to save this
		if ((type->flags & asOBJ_SHARED) && module->m_externalTypes.IndexOf(type) >= 0)
			return;

		// properties[]
		asCObjectType *t = CastToObjectType(type);

		// This is only done for object types
		asASSERT(t);

		asUINT size = (asUINT)t->properties.GetLength();
		WriteEncodedInt64(size);
		for (asUINT n = 0; n < t->properties.GetLength(); n++)
		{
			WriteObjectProperty(t->properties[n]);
		}
	}
}

void asCWriter::WriteEncodedInt64(asINT64 i)
{
	asBYTE signBit = ( i & asINT64(1)<<63 ) ? 0x80 : 0;
	if( signBit ) i = -i;

	asBYTE b;
	if( i < (1<<6) )
	{
		b = (asBYTE)(signBit + i); WriteData(&b, 1);
	}
	else if( i < (1<<13) )
	{
		b = asBYTE(0x40 + signBit + (i >> 8)); WriteData(&b, 1);
		b = asBYTE(i & 0xFF);                  WriteData(&b, 1);
	}
	else if( i < (1<<20) )
	{
		b = asBYTE(0x60 + signBit + (i >> 16)); WriteData(&b, 1);
		b = asBYTE((i >> 8) & 0xFF);            WriteData(&b, 1);
		b = asBYTE(i & 0xFF);                   WriteData(&b, 1);
	}
	else if( i < (1<<27) )
	{
		b = asBYTE(0x70 + signBit + (i >> 24)); WriteData(&b, 1);
		b = asBYTE((i >> 16) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 8) & 0xFF);            WriteData(&b, 1);
		b = asBYTE(i & 0xFF);                   WriteData(&b, 1);
	}
	else if( i < (asINT64(1)<<34) )
	{
		b = asBYTE(0x78 + signBit + (i >> 32)); WriteData(&b, 1);
		b = asBYTE((i >> 24) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 16) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 8) & 0xFF);            WriteData(&b, 1);
		b = asBYTE(i & 0xFF);                   WriteData(&b, 1);
	}
	else if( i < (asINT64(1)<<41) )
	{
		b = asBYTE(0x7C + signBit + (i >> 40)); WriteData(&b, 1);
		b = asBYTE((i >> 32) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 24) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 16) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 8) & 0xFF);            WriteData(&b, 1);
		b = asBYTE(i & 0xFF);                   WriteData(&b, 1);
	}
	else if( i < (asINT64(1)<<48) )
	{
		b = asBYTE(0x7E + signBit + (i >> 48)); WriteData(&b, 1);
		b = asBYTE((i >> 40) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 32) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 24) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 16) & 0xFF);           WriteData(&b, 1);
		b = asBYTE((i >> 8) & 0xFF);            WriteData(&b, 1);
		b = asBYTE(i & 0xFF);                   WriteData(&b, 1);
	}
	else
	{
		b = asBYTE(0x7F + signBit);   WriteData(&b, 1);
		b = asBYTE((i >> 56) & 0xFF); WriteData(&b, 1);
		b = asBYTE((i >> 48) & 0xFF); WriteData(&b, 1);
		b = asBYTE((i >> 40) & 0xFF); WriteData(&b, 1);
		b = asBYTE((i >> 32) & 0xFF); WriteData(&b, 1);
		b = asBYTE((i >> 24) & 0xFF); WriteData(&b, 1);
		b = asBYTE((i >> 16) & 0xFF); WriteData(&b, 1);
		b = asBYTE((i >> 8) & 0xFF);  WriteData(&b, 1);
		b = asBYTE(i & 0xFF);         WriteData(&b, 1);
	}
}

void asCWriter::WriteString(asCString* str)
{
	// First check if the string hasn't been saved already
	asSMapNode<asCString, int> *cursor = 0;
	if (stringToIdMap.MoveTo(&cursor, *str))
	{
		// Save a reference to the existing string
		// The lowest bit is set to 1 to indicate a reference
		WriteEncodedInt64(cursor->value*2+1);
		return;
	}

	// Save a new string
	// The lowest bit is set to 0 to indicate a new string
	asUINT len = (asUINT)str->GetLength();
	WriteEncodedInt64(len*2);

	if( len > 0 )
	{
		stream->Write(str->AddressOf(), (asUINT)len);
		bytesWritten += len;

		savedStrings.PushLast(*str);
		stringToIdMap.Insert(*str, int(savedStrings.GetLength()) - 1);
	}
}

void asCWriter::WriteGlobalProperty(asCGlobalProperty* prop)
{
	// TODO: We might be able to avoid storing the name and type of the global
	//       properties twice if we merge this with the WriteUsedGlobalProperties.
	WriteString(&prop->name);
	WriteString(&prop->nameSpace->name);
	WriteDataType(&prop->type);

	// Store the initialization function
	WriteFunction(prop->GetInitFunc());
}

void asCWriter::WriteObjectProperty(asCObjectProperty* prop)
{
	WriteString(&prop->name);
	WriteDataType(&prop->type);
	int flags = 0;
	if( prop->isPrivate ) flags |= 1;
	if( prop->isProtected ) flags |= 2;
	if( prop->isInherited ) flags |= 4;
	WriteEncodedInt64(flags);
}

void asCWriter::WriteDataType(const asCDataType *dt)
{
	// First check if the datatype has already been saved
	for( asUINT n = 0; n < savedDataTypes.GetLength(); n++ )
	{
		if( *dt == savedDataTypes[n] )
		{
			WriteEncodedInt64(n+1);
			return;
		}
	}

	// Indicate a new type with a null byte
	asUINT c = 0;
	WriteEncodedInt64(c);

	// Save the new datatype
	savedDataTypes.PushLast(*dt);

	int t = dt->GetTokenType();
	WriteEncodedInt64(t);
	if( t == ttIdentifier )
		WriteTypeInfo(dt->GetTypeInfo());

	// Endianess safe bitmask
	char bits = 0;
	SAVE_TO_BIT(bits, dt->IsObjectHandle(), 0);
	SAVE_TO_BIT(bits, dt->IsHandleToConst(), 1);
	SAVE_TO_BIT(bits, dt->IsReference(), 2);
	SAVE_TO_BIT(bits, dt->IsReadOnly(), 3);
	WriteData(&bits, 1);
}

void asCWriter::WriteTypeInfo(asCTypeInfo* ti)
{
	char ch;

	if( ti )
	{
		// Check for template instances/specializations
		asCObjectType *ot = CastToObjectType(ti);
		if( ot && ot->templateSubTypes.GetLength() )
		{
			// Check for list pattern type or template type
			if( ot->flags & asOBJ_LIST_PATTERN )
			{
				ch = 'l'; // list
				WriteData(&ch, 1);
				WriteTypeInfo(ot->templateSubTypes[0].GetTypeInfo());
			}
			else
			{
				ch = 'a'; // array
				WriteData(&ch, 1);
				WriteString(&ot->name);
				WriteString(&ot->nameSpace->name);

				WriteEncodedInt64(ot->templateSubTypes.GetLength());
				for( asUINT n = 0; n < ot->templateSubTypes.GetLength(); n++ )
				{
					if( !ot->templateSubTypes[n].IsPrimitive() || ot->templateSubTypes[n].IsEnumType() )
					{
						ch = 's'; // sub type
						WriteData(&ch, 1);
						WriteDataType(&ot->templateSubTypes[n]);
					}
					else
					{
						ch = 't'; // token
						WriteData(&ch, 1);
						eTokenType t = ot->templateSubTypes[n].GetTokenType();
						WriteEncodedInt64(t);
					}
				}
			}
		}
		else if( ti->flags & asOBJ_TEMPLATE_SUBTYPE )
		{
			ch = 's'; // sub type
			WriteData(&ch, 1);
			WriteString(&ti->name);
		}
		else if( !ti->GetParentType() )
		{
			ch = 'o'; // object
			WriteData(&ch, 1);
			WriteString(&ti->name);
			WriteString(&ti->nameSpace->name);
		}
		else
		{
			asASSERT(ti->flags & asOBJ_FUNCDEF);

			ch = 'c'; // child type
			WriteData(&ch, 1);
			WriteString(&ti->name);
			WriteTypeInfo(CastToFuncdefType(ti)->parentClass);
		}
	}
	else
	{
		ch = '\0';
		WriteData(&ch, 1);
	}
}

void asCWriter::CalculateAdjustmentByPos(asCScriptFunction *func)
{
	// Adjust the offset of all negative variables (parameters) so all pointers will have a size of 1 dword
	asUINT n;
	asCArray<int> adjustments;
	asUINT offset = 0;
	if( func->objectType )
	{
		adjustments.PushLast(offset);
		adjustments.PushLast(1-AS_PTR_SIZE);
		offset += AS_PTR_SIZE;
	}
	if( func->DoesReturnOnStack() )
	{
		adjustments.PushLast(offset);
		adjustments.PushLast(1-AS_PTR_SIZE);
		offset += AS_PTR_SIZE;
	}
	for( n = 0; n < func->parameterTypes.GetLength(); n++ )
	{
		if( !func->parameterTypes[n].IsPrimitive() ||
			func->parameterTypes[n].IsReference() )
		{
			adjustments.PushLast(offset);
			adjustments.PushLast(1-AS_PTR_SIZE);
			offset += AS_PTR_SIZE;
		}
		else
		{
			asASSERT( func->parameterTypes[n].IsPrimitive() );
			offset += func->parameterTypes[n].GetSizeOnStackDWords();
		}
	}

	// Build look-up table with the adjustments for each stack position
	adjustNegativeStackByPos.SetLength(offset);
	memset(adjustNegativeStackByPos.AddressOf(), 0, adjustNegativeStackByPos.GetLength()*sizeof(int));
	for( n = 0; n < adjustments.GetLength(); n+=2 )
	{
		int pos    = adjustments[n];
		int adjust = adjustments[n+1];

		for( asUINT i = pos+1; i < adjustNegativeStackByPos.GetLength(); i++ )
			adjustNegativeStackByPos[i] += adjust;
	}

	// Adjust the offset of all positive variables so that all object types and handles have a size of 1 dword
	// This is similar to how the adjustment is done in the asCReader::TranslateFunction, only the reverse
	adjustments.SetLength(0);
	for( n = 0; n < func->scriptData->objVariableTypes.GetLength(); n++ )
	{
		// Determine the size the variable currently occupies on the stack
		int size = AS_PTR_SIZE;

		// objVariableTypes is null if the variable type is a null pointer
		if( func->scriptData->objVariableTypes[n] &&
			(func->scriptData->objVariableTypes[n]->GetFlags() & asOBJ_VALUE) &&
			n >= func->scriptData->objVariablesOnHeap )
		{
			size = func->scriptData->objVariableTypes[n]->GetSize();
			if( size < 4 )
				size = 1;
			else
				size /= 4;
		}

		// If larger than 1 dword, adjust the offsets accordingly
		if (size > 1)
		{
			// How much needs to be adjusted?
			adjustments.PushLast(func->scriptData->objVariablePos[n]);
			adjustments.PushLast(-(size - 1));
		}
	}

	// Build look-up table with the adjustments for each stack position
	adjustStackByPos.SetLength(func->scriptData->stackNeeded);
	memset(adjustStackByPos.AddressOf(), 0, adjustStackByPos.GetLength()*sizeof(int));
	for( n = 0; n < adjustments.GetLength(); n+=2 )
	{
		int pos    = adjustments[n];
		int adjust = adjustments[n+1];

		for( asUINT i = pos; i < adjustStackByPos.GetLength(); i++ )
			adjustStackByPos[i] += adjust;
	}

	// Compute the sequence number of each bytecode instruction in order to update the jump offsets
	asUINT length = func->scriptData->byteCode.GetLength();
	asDWORD *bc = func->scriptData->byteCode.AddressOf();
	bytecodeNbrByPos.SetLength(length);
	asUINT num;
	for( offset = 0, num = 0; offset < length; )
	{
		bytecodeNbrByPos[offset] = num;
		offset += asBCTypeSize[asBCInfo[*(asBYTE*)(bc+offset)].type];
		num++;
	}

	// Store the number of instructions in the last position of bytecodeNbrByPos, 
	// so this can be easily queried in SaveBytecode. Normally this is already done
	// as most functions end with BC_RET, but in some cases the last instruction in 
	// the function is not a BC_RET, e.g. when a function has a never ending loop.
	bytecodeNbrByPos[length - 1] = num - 1;
}

int asCWriter::AdjustStackPosition(int pos)
{
	if( pos >= (int)adjustStackByPos.GetLength() )
	{
		// This happens for example if the function only have temporary variables
		// The adjustByPos can also be empty if the function doesn't have any variables at all, but receive a handle by parameter
		if( adjustStackByPos.GetLength() > 0 )
			pos += adjustStackByPos[adjustStackByPos.GetLength()-1];
	}
	else if( pos >= 0 )
		pos += adjustStackByPos[pos];
	else
	{
		asASSERT( -pos < (int)adjustNegativeStackByPos.GetLength() );
		pos -= (short)adjustNegativeStackByPos[-pos];
	}

	return pos;
}

int asCWriter::AdjustGetOffset(int offset, asCScriptFunction *func, asDWORD programPos)
{
	// TODO: optimize: multiple instructions for the same function doesn't need to look for the function everytime
	//                 the function can remember where it found the function and check if the programPos is still valid

	// Get offset 0 doesn't need adjustment
	if( offset == 0 ) return 0;

	bool bcAlloc = false;

	// Find out which function that will be called
	asCScriptFunction *calledFunc = 0;
	int stackDelta = 0;
	for( asUINT n = programPos; n < func->scriptData->byteCode.GetLength(); )
	{
		asBYTE bc = *(asBYTE*)&func->scriptData->byteCode[n];
		if( bc == asBC_CALL ||
			bc == asBC_CALLSYS ||
			bc == asBC_Thiscall1 ||
			bc == asBC_CALLINTF )
		{
			// Find the function from the function id in bytecode
			int funcId = asBC_INTARG(&func->scriptData->byteCode[n]);
			calledFunc = engine->scriptFunctions[funcId];
			break;
		}
		else if( bc == asBC_ALLOC )
		{
			// The alloc instruction doesn't take the object pointer on the stack,
			// as the memory will be allocated by the instruction itself
			bcAlloc = true;

			// Find the function from the function id in the bytecode
			int funcId = asBC_INTARG(&func->scriptData->byteCode[n+AS_PTR_SIZE]);
			calledFunc = engine->scriptFunctions[funcId];
			break;
		}
		else if( bc == asBC_CALLBND )
		{
			// Find the function from the engine's bind array
			int funcId = asBC_INTARG(&func->scriptData->byteCode[n]);
			calledFunc = engine->importedFunctions[funcId & ~FUNC_IMPORTED]->importedFunctionSignature;
			break;
		}
		else if( bc == asBC_CallPtr )
		{
			int var = asBC_SWORDARG0(&func->scriptData->byteCode[n]);
			asUINT v;
			// Find the funcdef from the local variable
			for( v = 0; v < func->scriptData->objVariablePos.GetLength(); v++ )
			{
				if( func->scriptData->objVariablePos[v] == var )
				{
					calledFunc = CastToFuncdefType(func->scriptData->objVariableTypes[v])->funcdef;
					break;
				}
			}
			if( !calledFunc )
			{
				// Look in parameters
				int paramPos = 0;
				if( func->objectType )
					paramPos -= AS_PTR_SIZE;
				if( func->DoesReturnOnStack() )
					paramPos -= AS_PTR_SIZE;
				for( v = 0; v < func->parameterTypes.GetLength(); v++ )
				{
					if( var == paramPos )
					{
						calledFunc = CastToFuncdefType(func->parameterTypes[v].GetTypeInfo())->funcdef;
						break;
					}
					paramPos -= func->parameterTypes[v].GetSizeOnStackDWords();
				}
			}
			break;
		}
		else if( bc == asBC_REFCPY ||
				 bc == asBC_COPY )
		{
			// In this case we know there is only 1 pointer on the stack above
			asASSERT( offset == AS_PTR_SIZE );
			return offset + (1 - AS_PTR_SIZE);
		}

		// Keep track of the stack size between the
		// instruction that needs to be adjusted and the call
		stackDelta += asBCInfo[bc].stackInc;

		n += asBCTypeSize[asBCInfo[bc].type];
	}

	asASSERT( calledFunc );

	// Count the number of pointers pushed on the stack above the
	// current offset, and then adjust the offset accordingly
	asUINT numPtrs = 0;
	int currOffset = -stackDelta;
	if( offset > currOffset && calledFunc->GetObjectType() && !bcAlloc )
	{
		currOffset += AS_PTR_SIZE;
		if( currOffset > 0 )
			numPtrs++;
	}
	if( offset > currOffset && calledFunc->DoesReturnOnStack() )
	{
		currOffset += AS_PTR_SIZE;
		if( currOffset > 0 )
			numPtrs++;
	}
	for( asUINT p = 0; p < calledFunc->parameterTypes.GetLength(); p++ )
	{
		if( offset <= currOffset ) break;

		if( !calledFunc->parameterTypes[p].IsPrimitive() ||
			calledFunc->parameterTypes[p].IsReference() )
		{
			// objects and references are passed by pointer
			currOffset += AS_PTR_SIZE;
			if( currOffset > 0 )
				numPtrs++;

			// The variable arg ? has an additional 32bit int with the typeid
			if( calledFunc->parameterTypes[p].IsAnyType() )
				currOffset += 1;
		}
		else
		{
			// built-in primitives or enums are passed by value
			asASSERT( calledFunc->parameterTypes[p].IsPrimitive() );
			currOffset += calledFunc->parameterTypes[p].GetSizeOnStackDWords();
		}
	}

	// The get offset must match one of the parameter offsets
	asASSERT( offset == currOffset );

	return offset + numPtrs * (1 - AS_PTR_SIZE);
}

void asCWriter::WriteByteCode(asCScriptFunction *func)
{
	asDWORD *bc   = func->scriptData->byteCode.AddressOf();
	size_t length = func->scriptData->byteCode.GetLength();

	// The length cannot be stored, because it is platform dependent,
	// instead we store the number of instructions
	asUINT count = bytecodeNbrByPos[bytecodeNbrByPos.GetLength()-1] + 1;
	WriteEncodedInt64(count);

	asDWORD *startBC = bc;
	while( length )
	{
		asDWORD tmpBC[4]; // The biggest instructions take up 4 DWORDs
		asDWORD c = *(asBYTE*)bc;

		// Copy the instruction to a temp buffer so we can work on it before saving
		memcpy(tmpBC, bc, asBCTypeSize[asBCInfo[c].type]*sizeof(asDWORD));

		if( c == asBC_ALLOC ) // PTR_DW_ARG
		{
			// Translate the object type
			asCObjectType *ot = *(asCObjectType**)(tmpBC+1);
			*(asPWORD*)(tmpBC+1) = FindTypeInfoIdx(ot);

			// Translate the constructor func id, unless it is 0
			if( *(int*)&tmpBC[1+AS_PTR_SIZE] != 0 )
			{
				// Increment 1 to the translated function id, as 0 will be reserved for no function
				*(int*)&tmpBC[1+AS_PTR_SIZE] = 1+FindFunctionIndex(engine->scriptFunctions[*(int*)&tmpBC[1+AS_PTR_SIZE]]);
			}
		}
		else if( c == asBC_REFCPY  || // PTR_ARG
				 c == asBC_RefCpyV || // wW_PTR_ARG
				 c == asBC_OBJTYPE )  // PTR_ARG
		{
			// Translate object type pointers into indices
			*(asPWORD*)(tmpBC+1) = FindTypeInfoIdx(*(asCObjectType**)(tmpBC+1));
		}
		else if( c == asBC_JitEntry ) // PTR_ARG
		{
			// We don't store the JIT argument
			*(asPWORD*)(tmpBC+1) = 0;
		}
		else if( c == asBC_TYPEID || // DW_ARG
			     c == asBC_Cast )    // DW_ARG
		{
			// Translate type ids into indices
			*(int*)(tmpBC+1) = FindTypeIdIdx(*(int*)(tmpBC+1));
		}
		else if( c == asBC_ADDSi ||      // W_DW_ARG
			     c == asBC_LoadThisR )   // W_DW_ARG
		{
			// Translate property offsets into indices
			*(((short*)tmpBC)+1) = (short)FindObjectPropIndex(*(((short*)tmpBC)+1), *(int*)(tmpBC+1), bc);

			// Translate type ids into indices
			*(int*)(tmpBC+1) = FindTypeIdIdx(*(int*)(tmpBC+1));
		}
		else if( c == asBC_LoadRObjR ||    // rW_W_DW_ARG
			     c == asBC_LoadVObjR )     // rW_W_DW_ARG
		{
			asCObjectType *ot = engine->GetObjectTypeFromTypeId(*(int*)(tmpBC+2));
			if( ot->flags & asOBJ_LIST_PATTERN )
			{
				// List patterns have a different way of translating the offsets
				SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
				*(((short*)tmpBC)+2) = (short)listAdj->AdjustOffset(*(((short*)tmpBC)+2), ot);
			}
			else
			{
				// Translate property offsets into indices
				*(((short*)tmpBC)+2) = (short)FindObjectPropIndex(*(((short*)tmpBC)+2), *(int*)(tmpBC+2), bc);
			}

			// Translate type ids into indices
			*(int*)(tmpBC+2) = FindTypeIdIdx(*(int*)(tmpBC+2));
		}
		else if( c == asBC_COPY )        // W_DW_ARG
		{
			// Translate type ids into indices
			*(int*)(tmpBC+1) = FindTypeIdIdx(*(int*)(tmpBC+1));

			// Update the WORDARG0 to 0, as this will be recalculated on the target platform
			asBC_WORDARG0(tmpBC) = 0;
		}
		else if( c == asBC_RET ) // W_ARG
		{
			// Save with arg 0, as this will be recalculated on the target platform
			asBC_WORDARG0(tmpBC) = 0;
		}
		else if( c == asBC_CALL ||     // DW_ARG
				 c == asBC_CALLINTF || // DW_ARG
				 c == asBC_CALLSYS ||  // DW_ARG
				 c == asBC_Thiscall1 ) // DW_ARG
		{
			// Translate the function id
			*(int*)(tmpBC+1) = FindFunctionIndex(engine->scriptFunctions[*(int*)(tmpBC+1)]);
		}
		else if( c == asBC_FuncPtr ) // PTR_ARG
		{
			// Translate the function pointer
			*(asPWORD*)(tmpBC+1) = FindFunctionIndex(*(asCScriptFunction**)(tmpBC+1));
		}
		else if( c == asBC_CALLBND ) // DW_ARG
		{
			// Translate the function id
			int funcId = tmpBC[1];
			for( asUINT n = 0; n < module->m_bindInformations.GetLength(); n++ )
				if( module->m_bindInformations[n]->importedFunctionSignature->id == funcId )
				{
					funcId = n;
					break;
				}

			tmpBC[1] = funcId;
		}
		else if( c == asBC_PGA      || // PTR_ARG
			     c == asBC_PshGPtr  || // PTR_ARG
			     c == asBC_LDG      || // PTR_ARG
				 c == asBC_PshG4    || // PTR_ARG
				 c == asBC_LdGRdR4  || // wW_PTR_ARG
				 c == asBC_CpyGtoV4 || // wW_PTR_ARG
				 c == asBC_CpyVtoG4 || // rW_PTR_ARG
				 c == asBC_SetG4    )  // PTR_DW_ARG
		{
			// Check if the address is a global property or a string constant
			void *ptr = *(void**)(tmpBC + 1);
			if (engine->varAddressMap.MoveTo(0, ptr))
			{
				// Translate global variable pointers into indices
				// Flag the first bit to signal global property
				*(asPWORD*)(tmpBC + 1) = (FindGlobalPropPtrIndex(*(void**)(tmpBC + 1)) << 1) + 1;
			}
			else
			{
				// Only PGA and PshGPtr can hold string constants
				asASSERT(c == asBC_PGA || c == asBC_PshGPtr);

				// Translate string constants into indices
				// Leave the first bit clear to signal string constant
				*(asPWORD*)(tmpBC + 1) = FindStringConstantIndex(*(void**)(tmpBC + 1)) << 1;
			}
		}
		else if( c == asBC_JMP    ||	// DW_ARG
			     c == asBC_JZ     ||
				 c == asBC_JNZ    ||
				 c == asBC_JLowZ  ||
				 c == asBC_JLowNZ ||
				 c == asBC_JS     ||
				 c == asBC_JNS    ||
				 c == asBC_JP     ||
				 c == asBC_JNP    ) // The JMPP instruction doesn't need modification
		{
			// Get the DWORD offset from arg
			int offset = *(int*)(tmpBC+1);

			// Determine instruction number for next instruction and destination
			int bcSeqNum = bytecodeNbrByPos[asUINT(bc - startBC)] + 1;
			asDWORD *targetBC = bc + 2 + offset;
			int targetBcSeqNum = bytecodeNbrByPos[asUINT(targetBC - startBC)];

			// Set the offset in number of instructions
			*(int*)(tmpBC+1) = targetBcSeqNum - bcSeqNum;
		}
		else if( c == asBC_GETOBJ ||    // W_ARG
			     c == asBC_GETOBJREF ||
			     c == asBC_GETREF ||
			     c == asBC_ChkNullS )
		{
			// Adjust the offset according to the function call that comes after
			asBC_WORDARG0(tmpBC) = (asWORD)AdjustGetOffset(asBC_WORDARG0(tmpBC), func, asDWORD(bc - startBC));
		}
		else if( c == asBC_AllocMem )
		{
			// It's not necessary to store the size of the list buffer, as it will be recalculated in the reader
			asBC_DWORDARG(tmpBC) = 0;

			// Determine the type of the list pattern from the variable
			short var = asBC_WORDARG0(tmpBC);
			asCObjectType *ot = CastToObjectType(func->GetTypeInfoOfLocalVar(var));

			// Create this helper object to adjust the offset of the elements accessed in the buffer
			listAdjusters.PushLast(asNEW(SListAdjuster)(ot));
		}
		else if( c == asBC_FREE ) // wW_PTR_ARG
		{
			// Translate object type pointers into indices
			asCObjectType *ot = *(asCObjectType**)(tmpBC+1);
			*(asPWORD*)(tmpBC+1) = FindTypeInfoIdx(ot);

			// Pop and destroy the list adjuster helper that was created with asBC_AllocMem
			if( ot && (ot->flags & asOBJ_LIST_PATTERN) )
			{
				SListAdjuster *list = listAdjusters.PopLast();
				asDELETE(list, SListAdjuster);
			}
		}
		else if( c == asBC_SetListSize )
		{
			// Adjust the offset in the initialization list
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			tmpBC[1] = listAdj->AdjustOffset(tmpBC[1], listAdj->patternType);

			// Tell the adjuster how many repeated values there are
			listAdj->SetRepeatCount(tmpBC[2]);
		}
		else if( c == asBC_PshListElmnt )   // W_DW_ARG
		{
			// Adjust the offset in the initialization list
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			tmpBC[1] = listAdj->AdjustOffset(tmpBC[1], listAdj->patternType);
		}
		else if( c == asBC_SetListType )
		{
			// Adjust the offset in the initialization list
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			tmpBC[1] = listAdj->AdjustOffset(tmpBC[1], listAdj->patternType);

			// Inform the adjuster of the type id of the next element
			listAdj->SetNextType(tmpBC[2]);

			// Translate the type id
			tmpBC[2] = FindTypeIdIdx(tmpBC[2]);
		}
		// Adjust the variable offsets
		switch( asBCInfo[c].type )
		{
		case asBCTYPE_wW_ARG:
		case asBCTYPE_rW_DW_ARG:
		case asBCTYPE_wW_QW_ARG:
		case asBCTYPE_rW_ARG:
		case asBCTYPE_wW_DW_ARG:
		case asBCTYPE_wW_W_ARG:
		case asBCTYPE_rW_QW_ARG:
		case asBCTYPE_rW_W_DW_ARG:
		case asBCTYPE_rW_DW_DW_ARG:
			{
				asBC_SWORDARG0(tmpBC) = (short)AdjustStackPosition(asBC_SWORDARG0(tmpBC));
			}
			break;

		case asBCTYPE_wW_rW_ARG:
		case asBCTYPE_wW_rW_DW_ARG:
		case asBCTYPE_rW_rW_ARG:
			{
				asBC_SWORDARG0(tmpBC) = (short)AdjustStackPosition(asBC_SWORDARG0(tmpBC));
				asBC_SWORDARG1(tmpBC) = (short)AdjustStackPosition(asBC_SWORDARG1(tmpBC));
			}
			break;

		case asBCTYPE_wW_rW_rW_ARG:
			{
				asBC_SWORDARG0(tmpBC) = (short)AdjustStackPosition(asBC_SWORDARG0(tmpBC));
				asBC_SWORDARG1(tmpBC) = (short)AdjustStackPosition(asBC_SWORDARG1(tmpBC));
				asBC_SWORDARG2(tmpBC) = (short)AdjustStackPosition(asBC_SWORDARG2(tmpBC));
			}
			break;

		default:
			// The other types don't treat variables so won't be modified
			break;
		}

		// TODO: bytecode: Must make sure that floats and doubles are always stored the same way regardless of platform.
		//                 Some platforms may not use the IEEE 754 standard, in which case it is necessary to encode the values

		// Now store the instruction in the smallest possible way
		switch( asBCInfo[c].type )
		{
		case asBCTYPE_NO_ARG:
			{
				// Just write 1 byte
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);
			}
			break;
		case asBCTYPE_W_ARG:
		case asBCTYPE_wW_ARG:
		case asBCTYPE_rW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);
			}
			break;
		case asBCTYPE_rW_DW_ARG:
		case asBCTYPE_wW_DW_ARG:
		case asBCTYPE_W_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the word argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);

				// Write the dword argument
				WriteEncodedInt64((int)tmpBC[1]);
			}
			break;
		case asBCTYPE_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				WriteEncodedInt64((int)tmpBC[1]);
			}
			break;
		case asBCTYPE_DW_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the dword argument
				WriteEncodedInt64((int)tmpBC[1]);

				// Write the dword argument
				WriteEncodedInt64((int)tmpBC[2]);
			}
			break;
		case asBCTYPE_wW_rW_rW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the first argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);

				// Write the second argument
				w = *(((short*)tmpBC)+2);
				WriteEncodedInt64(w);

				// Write the third argument
				w = *(((short*)tmpBC)+3);
				WriteEncodedInt64(w);
			}
			break;
		case asBCTYPE_wW_rW_ARG:
		case asBCTYPE_rW_rW_ARG:
		case asBCTYPE_wW_W_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the first argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);

				// Write the second argument
				w = *(((short*)tmpBC)+2);
				WriteEncodedInt64(w);
			}
			break;
		case asBCTYPE_wW_rW_DW_ARG:
		case asBCTYPE_rW_W_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the first argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);

				// Write the second argument
				w = *(((short*)tmpBC)+2);
				WriteEncodedInt64(w);

				// Write the third argument
				int dw = tmpBC[2];
				WriteEncodedInt64(dw);
			}
			break;
		case asBCTYPE_QW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				asQWORD qw = *(asQWORD*)&tmpBC[1];
				WriteEncodedInt64(qw);
			}
			break;
		case asBCTYPE_QW_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				asQWORD qw = *(asQWORD*)&tmpBC[1];
				WriteEncodedInt64(qw);

				// Write the second argument
				int dw = tmpBC[3];
				WriteEncodedInt64(dw);
			}
			break;
		case asBCTYPE_rW_QW_ARG:
		case asBCTYPE_wW_QW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the first argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);

				// Write the argument
				asQWORD qw = *(asQWORD*)&tmpBC[1];
				WriteEncodedInt64(qw);
			}
			break;
		case asBCTYPE_rW_DW_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the short argument
				short w = *(((short*)tmpBC)+1);
				WriteEncodedInt64(w);

				// Write the dword argument
				WriteEncodedInt64((int)tmpBC[1]);

				// Write the dword argument
				WriteEncodedInt64((int)tmpBC[2]);
			}
			break;
		default:
			{
				// This should never happen
				asASSERT(false);

				// Store the bc as is
				for( int n = 0; n < asBCTypeSize[asBCInfo[c].type]; n++ )
					WriteData(&tmpBC[n], 4);
			}
		}

		// Move to the next instruction
		bc += asBCTypeSize[asBCInfo[c].type];
		length -= asBCTypeSize[asBCInfo[c].type];
	}
}

asCWriter::SListAdjuster::SListAdjuster(asCObjectType *ot) : patternType(ot), repeatCount(0), entries(0), lastOffset(-1), nextOffset(0), nextTypeId(-1)
{
	asASSERT( ot && (ot->flags & asOBJ_LIST_PATTERN) );

	// Find the first expected value in the list
	asSListPatternNode *node = ot->engine->scriptFunctions[patternType->templateSubTypes[0].GetBehaviour()->listFactory]->listPattern;
	asASSERT( node && node->type == asLPT_START );
	patternNode = node->next;
}

int asCWriter::SListAdjuster::AdjustOffset(int offset, asCObjectType *listPatternType)
{
	// TODO: cleanup: The listPatternType parameter is not needed
	asASSERT( patternType == listPatternType );
	UNUSED_VAR(listPatternType);

	asASSERT( offset >= lastOffset );

	// If it is the same offset being accessed again, just return the same adjusted value
	if( offset == lastOffset )
		return entries-1;

	asASSERT( offset >= nextOffset );

	// Update last offset for next call
	lastOffset = offset;

	// What is being expected at this position?
	if( patternNode->type == asLPT_REPEAT || patternNode->type == asLPT_REPEAT_SAME )
	{
		// Don't move the patternNode yet because the caller must make a call to SetRepeatCount too
		nextOffset = offset + 4;
		return entries++;
	}
	else if( patternNode->type == asLPT_TYPE )
	{
		const asCDataType &dt = reinterpret_cast<asSListPatternDataTypeNode*>(patternNode)->dataType;
		if( dt.GetTokenType() == ttQuestion )
		{
			// The bytecode need to inform the type that will
			// come next and then adjust that position too before
			// we can move to the next node
			if( nextTypeId != -1 )
			{
				nextOffset = offset + 4;

				if( repeatCount > 0 )
					repeatCount--;

				// Only move the patternNode if we're not expecting any more repeated entries
				if( repeatCount == 0 )
					patternNode = patternNode->next;

				nextTypeId = -1;
			}
		}
		else
		{
			if( repeatCount > 0 )
			{
				// Was any value skipped?
				asUINT size;
				if( dt.IsObjectHandle() || (dt.GetTypeInfo() && (dt.GetTypeInfo()->flags & asOBJ_REF)) )
					size = AS_PTR_SIZE*4;
				else
					size = dt.GetSizeInMemoryBytes();

				int count = 0;
				while( nextOffset <= offset )
				{
					count++;
					nextOffset += size;

					// Align the offset on 4 byte boundaries
					if( size >= 4 && (nextOffset & 0x3) )
						nextOffset += 4 - (nextOffset & 0x3);
				}

				if( --count > 0 )
				{
					// Skip these values
					repeatCount -= count;
					entries += count;
				}

				nextOffset = offset + size;
				repeatCount--;
			}

			// Only move the patternNode if we're not expecting any more repeated entries
			if( repeatCount == 0 )
				patternNode = patternNode->next;
		}

		return entries++;
	}
	else if( patternNode->type == asLPT_START )
	{
		if( repeatCount > 0 )
			repeatCount--;
		SInfo info = {repeatCount, patternNode};
		stack.PushLast(info);

		repeatCount = 0;
		patternNode = patternNode->next;

		lastOffset--;
		return AdjustOffset(offset, listPatternType);
	}
	else if( patternNode->type == asLPT_END )
	{
		SInfo info = stack.PopLast();
		repeatCount = info.repeatCount;
		if( repeatCount )
			patternNode = info.startNode;
		else
			patternNode = patternNode->next;

		lastOffset--;
		return AdjustOffset(offset, listPatternType);
	}
	else
	{
		// Something is wrong with the pattern list declaration
		asASSERT( false );
	}

	return 0;
}

void asCWriter::SListAdjuster::SetRepeatCount(asUINT rc)
{
	// Make sure the list is expecting a repeat at this location
	asASSERT( patternNode->type == asLPT_REPEAT || patternNode->type == asLPT_REPEAT_SAME );

	// Now move to the next patternNode
	patternNode = patternNode->next;

	repeatCount = rc;
}

void asCWriter::SListAdjuster::SetNextType(int typeId)
{
	// Make sure the list is expecting a type at this location
	asASSERT( patternNode->type == asLPT_TYPE &&
	          reinterpret_cast<asSListPatternDataTypeNode*>(patternNode)->dataType.GetTokenType() == ttQuestion );

	// Inform the type id for the next adjustment
	nextTypeId = typeId;
}

void asCWriter::WriteUsedTypeIds()
{
	TimeIt("asCWriter::WriteUsedTypeIds");

	asUINT count = (asUINT)usedTypeIds.GetLength();
	WriteEncodedInt64(count);
	for( asUINT n = 0; n < count; n++ )
	{
		asCDataType dt = engine->GetDataTypeFromTypeId(usedTypeIds[n]);
		WriteDataType(&dt);
	}
}

int asCWriter::FindGlobalPropPtrIndex(void *ptr)
{
	int i = usedGlobalProperties.IndexOf(ptr);
	if( i >= 0 ) return i;

	usedGlobalProperties.PushLast(ptr);
	return (int)usedGlobalProperties.GetLength()-1;
}

void asCWriter::WriteUsedGlobalProps()
{
	TimeIt("asCWriter::WriteUsedGlobalProps");

	int c = (int)usedGlobalProperties.GetLength();
	WriteEncodedInt64(c);

	for( int n = 0; n < c; n++ )
	{
		asPWORD *p = (asPWORD*)usedGlobalProperties[n];

		// Find the property descriptor from the address
		asCGlobalProperty *prop = 0;
		asSMapNode<void*, asCGlobalProperty*> *cursor;
		if( engine->varAddressMap.MoveTo(&cursor, p) )
		{
			prop = engine->varAddressMap.GetValue(cursor);
		}

		asASSERT(prop);

		// Store the name and type of the property so we can find it again on loading
		WriteString(&prop->name);
		WriteString(&prop->nameSpace->name);
		WriteDataType(&prop->type);

		// Also store whether the property is a module property or a registered property
		char moduleProp = 0;
		if( prop->realAddress == 0 )
			moduleProp = 1;
		WriteData(&moduleProp, 1);
	}
}

void asCWriter::WriteUsedObjectProps()
{
	TimeIt("asCWriter::WriteUsedObjectProps");

	int c = (int)usedObjectProperties.GetLength();
	WriteEncodedInt64(c);

	for( asUINT n = 0; n < usedObjectProperties.GetLength(); n++ )
	{
		WriteTypeInfo(usedObjectProperties[n].objType);
		WriteString(&usedObjectProperties[n].prop->name);
	}
}

int asCWriter::FindObjectPropIndex(short offset, int typeId, asDWORD *bc)
{
	// If the last property was a composite property, then just return 0, because it won't be translated
	static bool lastWasComposite = false;
	if (lastWasComposite)
	{
		lastWasComposite = false;
		return 0;
	}
	
	asCObjectType *objType = engine->GetObjectTypeFromTypeId(typeId);
	asCObjectProperty *objProp = 0;

	// Look for composite properties first
	for (asUINT n = 0; objProp == 0 && n < objType->properties.GetLength(); n++)
	{
		// TODO: Composite: Perhaps it would be better to add metadata to the bytecode instruction to give the exact property. 
		//                  That would also allow me to remove the typeId from the bytecode instruction itself
		//                  Or perhaps a new bytecode instruction all together for accessing composite properties
		//                  One that would do both offsets and indirection in a single go.
		// TODO: Composite: Need to be able to handle instructions replaced in bytecode optimizations too
		if (objType->properties[n]->compositeOffset == offset)
		{
			// This is a potential composite property. Need to check the following instructions to be sure
			objProp = objType->properties[n];
			asDWORD *bcTemp = bc;
			bcTemp += asBCTypeSize[asBCInfo[*(asBYTE*)bcTemp].type];
			if (objProp->isCompositeIndirect)
			{
				// The next instruction would be a asBC_RDSPtr
				if ((*(asBYTE*)bcTemp) != asBC_RDSPtr)
				{
					objProp = 0;
					continue;
				}
				bcTemp += asBCTypeSize[asBCInfo[*(asBYTE*)bcTemp].type];
			}
			// The next instruction would be asBC_ADDSi
			if ((*(asBYTE*)bcTemp) != asBC_ADDSi)
			{
				objProp = 0;
				continue;
			}
			// Make sure the offset is the expected one
			if (*(((short*)bcTemp) + 1) != objProp->byteOffset)
			{
				objProp = 0;
				continue;
			}
		}
	}

	// If none of the composite properties matched, then look for ordinary property
	for (asUINT n = 0; objProp == 0 && n < objType->properties.GetLength(); n++)
	{
		if (objType->properties[n]->byteOffset == offset && !(objType->properties[n]->compositeOffset || objType->properties[n]->isCompositeIndirect))
			objProp = objType->properties[n];
	}

	asASSERT(objProp);

	// Remember if this is a composite property as the next call will then be for the same property
	if (objProp->compositeOffset || objProp->isCompositeIndirect)
		lastWasComposite = true;

	// Now check if the same property has already been accessed
	for( asUINT n = 0; n < usedObjectProperties.GetLength(); n++ )
	{
		if( usedObjectProperties[n].objType == objType &&
			usedObjectProperties[n].prop  == objProp )
			return n;
	}

	// Insert the new property
	SObjProp prop = {objType, objProp};
	usedObjectProperties.PushLast(prop);
	return (int)usedObjectProperties.GetLength() - 1;
}

int asCWriter::FindFunctionIndex(asCScriptFunction *func)
{
	for( asUINT n = 0; n < usedFunctions.GetLength(); n++ )
	{
		if( usedFunctions[n] == func )
			return n;
	}

	usedFunctions.PushLast(func);
	return (int)usedFunctions.GetLength() - 1;
}

int asCWriter::FindTypeIdIdx(int typeId)
{
	asUINT n;
	for( n = 0; n < usedTypeIds.GetLength(); n++ )
	{
		if( usedTypeIds[n] == typeId )
			return n;
	}

	usedTypeIds.PushLast(typeId);
	return (int)usedTypeIds.GetLength() - 1;
}

int asCWriter::FindTypeInfoIdx(asCTypeInfo *obj)
{
	asUINT n;
	for( n = 0; n < usedTypes.GetLength(); n++ )
	{
		if( usedTypes[n] == obj )
			return n;
	}

	usedTypes.PushLast(obj);
	return (int)usedTypes.GetLength() - 1;
}

#endif // AS_NO_COMPILER

END_AS_NAMESPACE

