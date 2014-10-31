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
// as_restore.cpp
//
// Functions for saving and restoring module bytecode
// asCRestore was originally written by Dennis Bollyn, dennis@gyrbo.be

#include "as_config.h"
#include "as_restore.h"
#include "as_bytecode.h"
#include "as_scriptobject.h"
#include "as_texts.h"

BEGIN_AS_NAMESPACE

asCReader::asCReader(asCModule* _module, asIBinaryStream* _stream, asCScriptEngine* _engine)
 : module(_module), stream(_stream), engine(_engine)
{
	error = false;
	bytesRead = 0;
}

void asCReader::ReadData(void *data, asUINT size)
{
	asASSERT(size == 1 || size == 2 || size == 4 || size == 8);
#if defined(AS_BIG_ENDIAN)
	for( asUINT n = 0; n < size; n++ )
		stream->Read(((asBYTE*)data)+n, 1);
#else
	for( int n = size-1; n >= 0; n-- )
		stream->Read(((asBYTE*)data)+n, 1);
#endif
	bytesRead += size;
}

int asCReader::Read(bool *wasDebugInfoStripped)
{
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
		for( i = 0; i < module->scriptFunctions.GetLength(); i++ )
			if( !dontTranslate.MoveTo(0, module->scriptFunctions[i]) )
				if( module->scriptFunctions[i]->scriptData )
					module->scriptFunctions[i]->scriptData->byteCode.SetLength(0);

		asCSymbolTable<asCGlobalProperty>::iterator it = module->scriptGlobals.List();
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

	return -1;
}

int asCReader::ReadInner() 
{
	// This function will load each entity one by one from the stream.
	// If any error occurs, it will return to the caller who is 
	// responsible for cleaning up the partially loaded entities.

	engine->deferValidationOfTemplateTypes = true;

	unsigned long i, count;
	asCScriptFunction* func;

	ReadData(&noDebugInfo, 1);

	// Read enums
	count = ReadEncodedUInt();
	module->enumTypes.Allocate(count, false);
	for( i = 0; i < count && !error; i++ )
	{
		asCObjectType *ot = asNEW(asCObjectType)(engine);
		if( ot == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		ReadObjectTypeDeclaration(ot, 1);

		// If the type is shared then we should use the original if it exists
		bool sharedExists = false;
		if( ot->IsShared() )
		{
			for( asUINT n = 0; n < engine->classTypes.GetLength(); n++ )
			{
				asCObjectType *t = engine->classTypes[n];
				if( t && 
					t->IsShared() &&
					t->name == ot->name &&
					t->nameSpace == ot->nameSpace &&
					(t->flags & asOBJ_ENUM) )
				{
					asDELETE(ot, asCObjectType);
					ot = t;
					sharedExists = true;
					break;
				}
			}
		}

		if( sharedExists )
			existingShared.Insert(ot, true);
		else
			engine->classTypes.PushLast(ot);
		module->enumTypes.PushLast(ot);
		ot->AddRef();
		ReadObjectTypeDeclaration(ot, 2);
	}

	if( error ) return asERROR;

	// classTypes[]
	// First restore the structure names, then the properties
	count = ReadEncodedUInt();
	module->classTypes.Allocate(count, false);
	for( i = 0; i < count && !error; ++i )
	{
		asCObjectType *ot = asNEW(asCObjectType)(engine);
		if( ot == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		ReadObjectTypeDeclaration(ot, 1);

		// If the type is shared, then we should use the original if it exists
		bool sharedExists = false;
		if( ot->IsShared() )
		{
			for( asUINT n = 0; n < engine->classTypes.GetLength(); n++ )
			{
				asCObjectType *t = engine->classTypes[n];
				if( t &&
					t->IsShared() &&
					t->name == ot->name &&
					t->nameSpace == ot->nameSpace &&
					t->IsInterface() == ot->IsInterface() )
				{
					asDELETE(ot, asCObjectType);
					ot = t;
					sharedExists = true;
					break;
				}
			}
		}

		if( sharedExists )
			existingShared.Insert(ot, true);
		else
		{
			engine->classTypes.PushLast(ot);

			// Set this module as the owner
			ot->module = module;
		}
		module->classTypes.PushLast(ot);
		ot->AddRef();
	}

	if( error ) return asERROR;

	// Read func defs
	count = ReadEncodedUInt();
	module->funcDefs.Allocate(count, false);
	for( i = 0; i < count && !error; i++ )
	{
		bool isNew;
		asCScriptFunction *func = ReadFunction(isNew, false, true);
		if( func )
		{
			module->funcDefs.PushLast(func);
			engine->funcDefs.PushLast(func);

			// TODO: clean up: This is also done by the builder. It should probably be moved to a method in the module
			// Check if there is another identical funcdef from another module and if so reuse that instead
			for( asUINT n = 0; n < engine->funcDefs.GetLength(); n++ )
			{
				asCScriptFunction *f2 = engine->funcDefs[n];
				if( f2 == 0 || func == f2 )
					continue;

				if( f2->name == func->name &&
					f2->nameSpace == func->nameSpace &&
					f2->IsSignatureExceptNameEqual(func) )
				{
					// Replace our funcdef for the existing one
					module->funcDefs[module->funcDefs.IndexOf(func)] = f2;
					f2->AddRef();

					engine->funcDefs.RemoveValue(func);

					savedFunctions[savedFunctions.IndexOf(func)] = f2;

					func->Release();

					// Funcdefs aren't deleted when the ref count reaches zero so we must manually delete it here
					asDELETE(func,asCScriptFunction);
					break;
				}
			}
		}
		else
			Error(TXT_INVALID_BYTECODE_d);
	}

	// Read interface methods
	for( i = 0; i < module->classTypes.GetLength() && !error; i++ )
	{
		if( module->classTypes[i]->IsInterface() )
			ReadObjectTypeDeclaration(module->classTypes[i], 2);
	}

	// Read class methods and behaviours
	for( i = 0; i < module->classTypes.GetLength() && !error; ++i )
	{
		if( !module->classTypes[i]->IsInterface() )
			ReadObjectTypeDeclaration(module->classTypes[i], 2);
	}

	// Read class properties
	for( i = 0; i < module->classTypes.GetLength() && !error; ++i )
	{
		if( !module->classTypes[i]->IsInterface() )
			ReadObjectTypeDeclaration(module->classTypes[i], 3);
	}

	if( error ) return asERROR;

	// Read typedefs
	count = ReadEncodedUInt();
	module->typeDefs.Allocate(count, false);
	for( i = 0; i < count && !error; i++ )
	{
		asCObjectType *ot = asNEW(asCObjectType)(engine);
		if( ot == 0 )
		{
			error = true;
			return asOUT_OF_MEMORY;
		}

		ReadObjectTypeDeclaration(ot, 1);
		engine->classTypes.PushLast(ot);
		module->typeDefs.PushLast(ot);
		ot->AddRef();
		ReadObjectTypeDeclaration(ot, 2);
	}

	if( error ) return asERROR;

	// scriptGlobals[]
	count = ReadEncodedUInt();
	if( count && engine->ep.disallowGlobalVars )
	{
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_GLOBAL_VARS_NOT_ALLOWED);
		Error(TXT_INVALID_BYTECODE_d);
	}
	module->scriptGlobals.Allocate(count, false);
	for( i = 0; i < count && !error; ++i ) 
	{
		ReadGlobalProperty();
	}

	// scriptFunctions[]
	count = ReadEncodedUInt();
	for( i = 0; i < count && !error; ++i ) 
	{
		size_t len = module->scriptFunctions.GetLength();
		bool isNew;
		func = ReadFunction(isNew);
		if( func == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			break;
		}
		
		// Is the function shared and was it created now?
		if( func->isShared && len != module->scriptFunctions.GetLength() )
		{
			// If the function already existed in another module, then
			// we need to replace it with previously existing one
			for( asUINT n = 0; n < engine->scriptFunctions.GetLength() && !error; n++ )
			{
				asCScriptFunction *realFunc = engine->scriptFunctions[n];
				if( realFunc &&
					realFunc != func &&
					realFunc->IsShared() &&
					realFunc->IsSignatureEqual(func) )
				{
					// Replace the recently created function with the pre-existing function
					module->scriptFunctions[module->scriptFunctions.GetLength()-1] = realFunc;
					realFunc->AddRef();
					savedFunctions[savedFunctions.GetLength()-1] = realFunc;
					engine->FreeScriptFunctionId(func->id);

					// Insert the function in the dontTranslate array
					dontTranslate.Insert(realFunc, true);

					// Release the function, but make sure nothing else is released
					func->id = 0;
					func->scriptData->byteCode.SetLength(0);
					func->Release();
					break;
				}
			}
		}
	}

	// globalFunctions[]
	count = ReadEncodedUInt();
	for( i = 0; i < count && !error; ++i )
	{
		bool isNew;
		func = ReadFunction(isNew, false, false);
		if( func )
		{
			module->globalFunctions.Put(func);
			func->AddRef();
		}
		else
			Error(TXT_INVALID_BYTECODE_d);
	}

	if( error ) return asERROR;

	// bindInformations[]
	count = ReadEncodedUInt();
	module->bindInformations.Allocate(count, false);
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
		module->bindInformations.PushLast(info);
	}

	if( error ) return asERROR;

	// usedTypes[]
	count = ReadEncodedUInt();
	usedTypes.Allocate(count, false);
	for( i = 0; i < count && !error; ++i )
	{
		asCObjectType *ot = ReadObjectType();
		usedTypes.PushLast(ot);
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
			if( !(usedTypes[i]->flags & asOBJ_TEMPLATE) || 
				!usedTypes[i]->beh.templateCallback )
				continue;
			
			bool dontGarbageCollect = false;
			asCScriptFunction *callback = engine->scriptFunctions[usedTypes[i]->beh.templateCallback];
			if( !engine->CallGlobalFunctionRetBool(usedTypes[i], &dontGarbageCollect, callback->sysFuncIntf, callback) )
			{
				asCString sub = usedTypes[i]->templateSubTypes[0].Format();
				for( asUINT n = 1; n < usedTypes[i]->templateSubTypes.GetLength(); n++ )
				{
					sub += ",";
					sub += usedTypes[i]->templateSubTypes[n].Format();
				}
				asCString str;
				str.Format(TXT_INSTANCING_INVLD_TMPL_TYPE_s_s, usedTypes[i]->name.AddressOf(), sub.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				Error(TXT_INVALID_BYTECODE_d);
			}
		}
	}
	engine->deferValidationOfTemplateTypes = false;

	if( error ) return asERROR;

	// Update the loaded bytecode to point to the correct types, property offsets,
	// function ids, etc. This is basically a linking stage.
	for( i = 0; i < module->scriptFunctions.GetLength() && !error; i++ )
		if( module->scriptFunctions[i]->funcType == asFUNC_SCRIPT )
			TranslateFunction(module->scriptFunctions[i]);

	asCSymbolTable<asCGlobalProperty>::iterator globIt = module->scriptGlobals.List();
	while( globIt && !error )
	{
		asCScriptFunction *initFunc = (*globIt)->GetInitFunc();
		if( initFunc )
			TranslateFunction(initFunc);
		globIt++;
	}

	if( error ) return asERROR;

	// Add references for all functions (except for the pre-existing shared code)
	for( i = 0; i < module->scriptFunctions.GetLength(); i++ )
		if( !dontTranslate.MoveTo(0, module->scriptFunctions[i]) )
			module->scriptFunctions[i]->AddReferences();

	globIt = module->scriptGlobals.List();
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
	asCString str;

	asUINT count;
	count = ReadEncodedUInt();
	usedStringConstants.Allocate(count, false);
	for( asUINT i = 0; i < count; ++i ) 
	{
		ReadString(&str);
		usedStringConstants.PushLast(engine->AddConstantString(str.AddressOf(), str.GetLength()));
	}
}

void asCReader::ReadUsedFunctions()
{
	asUINT count;
	count = ReadEncodedUInt();
	usedFunctions.SetLength(count);
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
			ReadFunctionSignature(&func);
			if( error )
			{
				func.funcType = asFUNC_DUMMY;
				return;
			}

			// Find the correct function
			if( c == 'm' )
			{
				for( asUINT i = 0; i < module->scriptFunctions.GetLength(); i++ )
				{
					asCScriptFunction *f = module->scriptFunctions[i];
					if( !func.IsSignatureEqual(f) ||
						func.objectType != f->objectType ||
						func.funcType != f->funcType || 
						func.nameSpace != f->nameSpace )
						continue;

					usedFunctions[n] = f;
					break;
				}
			}
			else
			{
				for( asUINT i = 0; i < engine->scriptFunctions.GetLength(); i++ )
				{
					asCScriptFunction *f = engine->scriptFunctions[i];
					if( f == 0 ||
						!func.IsSignatureEqual(f) ||
						func.objectType != f->objectType ||
						func.nameSpace != f->nameSpace )
						continue;

					usedFunctions[n] = f;
					break;
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

void asCReader::ReadFunctionSignature(asCScriptFunction *func)
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

	count = ReadEncodedUInt();
	if( count > 256 )
	{
		// Too many arguments, must be something wrong in the file
		Error(TXT_INVALID_BYTECODE_d);
		return;
	}
	func->parameterTypes.Allocate(count, false);
	for( i = 0; i < count; ++i ) 
	{
		ReadDataType(&dt);
		func->parameterTypes.PushLast(dt);
	}

	func->inOutFlags.SetLength(func->parameterTypes.GetLength());
	memset(func->inOutFlags.AddressOf(), 0, sizeof(asETypeModifiers)*func->inOutFlags.GetLength());
	count = ReadEncodedUInt();
	if( count > func->parameterTypes.GetLength() )
	{
		// Cannot be more than the number of arguments
		Error(TXT_INVALID_BYTECODE_d);
		return;
	}
	for( i = 0; i < count; ++i )
	{
		num = ReadEncodedUInt();
		func->inOutFlags[i] = static_cast<asETypeModifiers>(num);
	}

	func->funcType = (asEFuncType)ReadEncodedUInt();

	// Read the default args, from last to first
	count = ReadEncodedUInt();
	if( count > func->parameterTypes.GetLength() )
	{
		// Cannot be more than the number of arguments
		Error(TXT_INVALID_BYTECODE_d);
		return;
	}
	if( count )
	{
		func->defaultArgs.SetLength(func->parameterTypes.GetLength());
		memset(func->defaultArgs.AddressOf(), 0, sizeof(asCString*)*func->defaultArgs.GetLength());
		for( i = 0; i < count; i++ )
		{
			asCString *str = asNEW(asCString);
			if( str == 0 )
			{
				// Out of memory
				error = true;
				return;
			}
			func->defaultArgs[func->defaultArgs.GetLength()-1-i] = str;
			ReadString(str);
		}
	}
	
	func->objectType = ReadObjectType();
	if( func->objectType )
	{
		asBYTE b;
		ReadData(&b, 1);
		func->isReadOnly = (b & 1) ? true : false;
		func->isPrivate  = (b & 2) ? true : false;
		func->nameSpace = engine->nameSpaces[0];
	}
	else
	{
		asCString ns;
		ReadString(&ns);
		func->nameSpace = engine->AddNameSpace(ns.AddressOf());
	}
}

asCScriptFunction *asCReader::ReadFunction(bool &isNew, bool addToModule, bool addToEngine, bool addToGC) 
{
	isNew = false;
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
	asCScriptFunction *func = asNEW(asCScriptFunction)(engine,module,asFUNC_DUMMY);
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

	ReadFunctionSignature(func);
	if( error )
	{
		asDELETE(func, asCScriptFunction);
		return 0;
	}

	if( func->funcType == asFUNC_SCRIPT )
	{
		func->AllocateScriptFunctionData();

		if( addToGC && !addToModule )
			engine->gc.AddScriptObjectToGC(func, &engine->functionBehaviours);
		
		ReadByteCode(func);

		func->scriptData->variableSpace = ReadEncodedUInt();

		count = ReadEncodedUInt();
		func->scriptData->objVariablePos.Allocate(count, false);
		func->scriptData->objVariableTypes.Allocate(count, false);
		func->scriptData->funcVariableTypes.Allocate(count, false);
		for( i = 0; i < count; ++i )
		{
			func->scriptData->objVariableTypes.PushLast(ReadObjectType());
			asUINT idx = ReadEncodedUInt();
			func->scriptData->funcVariableTypes.PushLast((asCScriptFunction*)(asPWORD)idx);
			num = ReadEncodedUInt();
			func->scriptData->objVariablePos.PushLast(num);
		}
		if( count > 0 )
			func->scriptData->objVariablesOnHeap = ReadEncodedUInt();
		else
			func->scriptData->objVariablesOnHeap = 0;

		int length = ReadEncodedUInt();
		func->scriptData->objVariableInfo.SetLength(length);
		for( i = 0; i < length; ++i )
		{
			func->scriptData->objVariableInfo[i].programPos     = ReadEncodedUInt();
			func->scriptData->objVariableInfo[i].variableOffset = ReadEncodedUInt();
			func->scriptData->objVariableInfo[i].option         = ReadEncodedUInt();
		}

		if( !noDebugInfo )
		{
			length = ReadEncodedUInt();
			func->scriptData->lineNumbers.SetLength(length);
			for( i = 0; i < length; ++i )
				func->scriptData->lineNumbers[i] = ReadEncodedUInt();

			// Read the array of script sections 
			length = ReadEncodedUInt();
			func->scriptData->sectionIdxs.SetLength(length);
			for( i = 0; i < length; ++i )
			{
				if( (i & 1) == 0 )
					func->scriptData->sectionIdxs[i] = ReadEncodedUInt();
				else
				{
					asCString str;
					ReadString(&str);
					func->scriptData->sectionIdxs[i] = engine->GetScriptSectionNameIndex(str.AddressOf());
				}
			}
		}

		ReadData(&func->isShared, 1);

		// Read the variable information
		if( !noDebugInfo )
		{
			length = ReadEncodedUInt();
			func->scriptData->variables.Allocate(length, false);
			for( i = 0; i < length; i++ )
			{
				asSScriptVariable *var = asNEW(asSScriptVariable);
				if( var == 0 )
				{
					// Out of memory
					error = true;
					asDELETE(func, asCScriptFunction);
					return 0;
				}
				func->scriptData->variables.PushLast(var);

				var->declaredAtProgramPos = ReadEncodedUInt();
				var->stackOffset = ReadEncodedUInt();
				ReadString(&var->name);
				ReadDataType(&var->type);
			}
		}

		ReadData(&func->dontCleanUpOnException, 1);

		// Read script section name
		if( !noDebugInfo )
		{
			asCString name;
			ReadString(&name);
			func->scriptData->scriptSectionIdx = engine->GetScriptSectionNameIndex(name.AddressOf());
			func->scriptData->declaredAt = ReadEncodedUInt();
		}
	}
	else if( func->funcType == asFUNC_VIRTUAL || func->funcType == asFUNC_INTERFACE )
	{
		func->vfTableIdx = ReadEncodedUInt();
	}

	if( addToModule )
	{
		// The refCount is already 1
		module->scriptFunctions.PushLast(func);
	}
	if( addToEngine )
	{
		func->id = engine->GetNextScriptFunctionId();
		engine->SetScriptFunction(func);
	}
	if( func->objectType )
		func->ComputeSignatureId();

	return func;
}

void asCReader::ReadObjectTypeDeclaration(asCObjectType *ot, int phase)
{
	if( phase == 1 )
	{
		// Read the initial attributes
		ReadString(&ot->name);
		ReadData(&ot->flags, 4);
		ot->size = ReadEncodedUInt();
		asCString ns;
		ReadString(&ns);
		ot->nameSpace = engine->AddNameSpace(ns.AddressOf());

		// Reset the size of script classes, since it will be recalculated as properties are added
		if( (ot->flags & asOBJ_SCRIPT_OBJECT) && ot->size != 0 )
			ot->size = sizeof(asCScriptObject);

		// Use the default script class behaviours
		ot->beh = engine->scriptTypeBehaviours.beh;
		ot->beh.construct = 0;
		ot->beh.factory = 0;
		ot->beh.constructors.PopLast(); // These will be read from the file
		ot->beh.factories.PopLast(); // These will be read from the file
		engine->scriptFunctions[ot->beh.addref]->AddRef();
		engine->scriptFunctions[ot->beh.release]->AddRef();
		engine->scriptFunctions[ot->beh.gcEnumReferences]->AddRef();
		engine->scriptFunctions[ot->beh.gcGetFlag]->AddRef();
		engine->scriptFunctions[ot->beh.gcGetRefCount]->AddRef();
		engine->scriptFunctions[ot->beh.gcReleaseAllReferences]->AddRef();
		engine->scriptFunctions[ot->beh.gcSetFlag]->AddRef();
		engine->scriptFunctions[ot->beh.copy]->AddRef();
		// TODO: weak: Should not do this if the class has been declared with 'noweak'
		engine->scriptFunctions[ot->beh.getWeakRefFlag]->AddRef();
		for( asUINT i = 1; i < ot->beh.operators.GetLength(); i += 2 )
			engine->scriptFunctions[ot->beh.operators[i]]->AddRef();
	}
	else if( phase == 2 )
	{
		if( ot->flags & asOBJ_ENUM )
		{
			int count = ReadEncodedUInt();
			bool sharedExists = existingShared.MoveTo(0, ot);
			if( !sharedExists )
			{
				ot->enumValues.Allocate(count, false);
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
					ot->enumValues.PushLast(e);
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
					for( asUINT e = 0; e < ot->enumValues.GetLength(); e++ )
					{
						if( ot->enumValues[e]->name == name &&
							ot->enumValues[e]->value == value )
						{
							found = true;
							break;
						}
					}
					if( !found )
					{
						asCString str;
						str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
						engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
						Error(TXT_INVALID_BYTECODE_d);
					}
				}
			}
		}
		else if( ot->flags & asOBJ_TYPEDEF )
		{
			eTokenType t = (eTokenType)ReadEncodedUInt();
			ot->templateSubTypes.PushLast(asCDataType::CreatePrimitive(t, false));
		}
		else
		{
			// If the type is shared and pre-existing, we should just 
			// validate that the loaded methods match the original 
			bool sharedExists = existingShared.MoveTo(0, ot);
			if( sharedExists )
			{
				asCObjectType *dt = ReadObjectType();
				if( ot->derivedFrom != dt )
				{
					asCString str;
					str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
					engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
					Error(TXT_INVALID_BYTECODE_d);
				}
			}
			else
			{
				ot->derivedFrom = ReadObjectType();
				if( ot->derivedFrom )
					ot->derivedFrom->AddRef();
			}

			// interfaces[] / interfaceVFTOffsets[]
			int size = ReadEncodedUInt();
			if( sharedExists )
			{
				for( int n = 0; n < size; n++ )
				{
					asCObjectType *intf = ReadObjectType();
					ReadEncodedUInt();

					if( !ot->Implements(intf) )
					{
						asCString str;
						str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
						engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
						Error(TXT_INVALID_BYTECODE_d);
					}
				}
			}
			else
			{
				ot->interfaces.Allocate(size, false);
				ot->interfaceVFTOffsets.Allocate(size, false);
				for( int n = 0; n < size; n++ )
				{
					asCObjectType *intf = ReadObjectType();
					ot->interfaces.PushLast(intf);

					asUINT offset = ReadEncodedUInt();
					ot->interfaceVFTOffsets.PushLast(offset);
				}
			}

			// behaviours
			if( !ot->IsInterface() && ot->flags != asOBJ_TYPEDEF && ot->flags != asOBJ_ENUM )
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
						str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
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
							func->Release();
						}
						module->scriptFunctions.PushLast(realFunc);
						realFunc->AddRef();
						dontTranslate.Insert(realFunc, true);
					}
				}
				else
				{
					if( func )
					{
						ot->beh.destruct = func->id;
						func->AddRef();
					}
					else
						ot->beh.destruct = 0;
				}

				size = ReadEncodedUInt();
				for( int n = 0; n < size; n++ )
				{
					bool isNew;
					asCScriptFunction *func = ReadFunction(isNew, !sharedExists, !sharedExists, !sharedExists);
					if( func )
					{
						if( sharedExists )
						{
							// Find the real function in the object, and update the savedFunctions array
							bool found = false;
							for( asUINT n = 0; n < ot->beh.constructors.GetLength(); n++ )
							{
								asCScriptFunction *realFunc = engine->GetScriptFunction(ot->beh.constructors[n]);
								if( realFunc->IsSignatureEqual(func) )
								{
									// If the function is not the last, then the substitution has already occurred before
									if( savedFunctions[savedFunctions.GetLength()-1] == func )
										savedFunctions[savedFunctions.GetLength()-1] = realFunc;
									found = true;
									module->scriptFunctions.PushLast(realFunc);
									realFunc->AddRef();
									dontTranslate.Insert(realFunc, true);
									break;
								}
							}
							if( !found )
							{
								asCString str;
								str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
								engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
								Error(TXT_INVALID_BYTECODE_d);
							}
							if( isNew )
							{
								// Destroy the function without releasing any references
								func->id = 0;
								func->scriptData->byteCode.SetLength(0);
								func->Release();
							}
						}
						else
						{
							ot->beh.constructors.PushLast(func->id);
							func->AddRef();

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
							for( asUINT n = 0; n < ot->beh.factories.GetLength(); n++ )
							{
								asCScriptFunction *realFunc = engine->GetScriptFunction(ot->beh.factories[n]);
								if( realFunc->IsSignatureEqual(func) )
								{
									// If the function is not the last, then the substitution has already occurred before
									if( savedFunctions[savedFunctions.GetLength()-1] == func )
										savedFunctions[savedFunctions.GetLength()-1] = realFunc;
									found = true;
									module->scriptFunctions.PushLast(realFunc);
									realFunc->AddRef();
									dontTranslate.Insert(realFunc, true);
									break;
								}
							}
							if( !found )
							{
								asCString str;
								str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
								engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
								Error(TXT_INVALID_BYTECODE_d);
							}
							if( isNew )
							{
								// Destroy the function without releasing any references
								func->id = 0;
								func->scriptData->byteCode.SetLength(0);
								func->Release();
							}
						}
						else
						{
							ot->beh.factories.PushLast(func->id);
							func->AddRef();

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
			size = ReadEncodedUInt();
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
						for( asUINT n = 0; n < ot->methods.GetLength(); n++ )
						{
							asCScriptFunction *realFunc = engine->GetScriptFunction(ot->methods[n]);
							if( realFunc->IsSignatureEqual(func) )
							{
								// If the function is not the last, then the substitution has already occurred before
								if( savedFunctions[savedFunctions.GetLength()-1] == func )
									savedFunctions[savedFunctions.GetLength()-1] = realFunc;
								found = true;
								module->scriptFunctions.PushLast(realFunc);
								realFunc->AddRef();
								dontTranslate.Insert(realFunc, true);
								break;
							}
						}
						if( !found )
						{
							asCString str;
							str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
							engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
							Error(TXT_INVALID_BYTECODE_d);
						}
						if( isNew )
						{
							// Destroy the function without releasing any references
							func->id = 0;
							if( func->scriptData )
								func->scriptData->byteCode.SetLength(0);
							func->Release();
						}
					}
					else
					{
						// If the method is the assignment operator we need to replace the default implementation
						if( func->name == "opAssign" && func->parameterTypes.GetLength() == 1 &&
							func->parameterTypes[0].GetObjectType() == func->objectType &&
							(func->inOutFlags[0] & asTM_INREF) )
						{
							engine->scriptFunctions[ot->beh.copy]->Release();
							ot->beh.copy = func->id;
							func->AddRef();
						}
						
						ot->methods.PushLast(func->id);
						func->AddRef();
					}
				}
				else
				{
					Error(TXT_INVALID_BYTECODE_d);
				}
			}

			// virtualFunctionTable[]
			size = ReadEncodedUInt();
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
						for( asUINT n = 0; n < ot->virtualFunctionTable.GetLength(); n++ )
						{
							asCScriptFunction *realFunc = ot->virtualFunctionTable[n];
							if( realFunc->IsSignatureEqual(func) )
							{
								// If the function is not the last, then the substitution has already occurred before
								if( savedFunctions[savedFunctions.GetLength()-1] == func )
									savedFunctions[savedFunctions.GetLength()-1] = realFunc;
								found = true;
								module->scriptFunctions.PushLast(realFunc);
								realFunc->AddRef();
								dontTranslate.Insert(realFunc, true);
								break;
							}
						}
						if( !found )
						{
							asCString str;
							str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, ot->GetName());
							engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
							Error(TXT_INVALID_BYTECODE_d);
						}
						if( isNew )
						{
							// Destroy the function without releasing any references
							func->id = 0;
							if( func->scriptData )
								func->scriptData->byteCode.SetLength(0);
							func->Release();
						}
					}
					else
					{
						ot->virtualFunctionTable.PushLast(func);
						func->AddRef();
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
		// properties[]
		asUINT size = ReadEncodedUInt();
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

asQWORD asCReader::ReadEncodedUInt64()
{
	asQWORD i = 0;
	asBYTE b;
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

void asCReader::ReadString(asCString* str) 
{
	char b;
	ReadData(&b, 1);
	if( b == '\0' )
	{
		str->SetLength(0);
	}
	else if( b == 'n' )
	{
		asUINT len = ReadEncodedUInt();
		str->SetLength(len);
		stream->Read(str->AddressOf(), len);

		savedStrings.PushLast(*str);
	}
	else
	{
		asUINT n = ReadEncodedUInt();
		if( n < savedStrings.GetLength() )
			*str = savedStrings[n];
		else
			Error(TXT_INVALID_BYTECODE_d);
	}
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
	bool f;
	ReadData(&f, 1);
	if( f )
	{
		bool isNew;
		// Do not add the function to the GC at this time. It will 
		// only be added to the GC when the module releases the property
		asCScriptFunction *func = ReadFunction(isNew, false, true, false);
		if( func )
		{
			prop->SetInitFunc(func);
			func->Release();
		}
		else
			Error(TXT_INVALID_BYTECODE_d);
	}
}

void asCReader::ReadObjectProperty(asCObjectType *ot) 
{
	asCString name;
	ReadString(&name);
	asCDataType dt;
	ReadDataType(&dt);
	bool isPrivate;
	ReadData(&isPrivate, 1);

	// TODO: shared: If the type is shared and pre-existing, we should just 
	//               validate that the loaded methods match the original 
	if( !existingShared.MoveTo(0, ot) )
		ot->AddPropertyToClass(name, dt, isPrivate);
}

void asCReader::ReadDataType(asCDataType *dt) 
{
	eTokenType tokenType;

	tokenType = (eTokenType)ReadEncodedUInt();
	if( tokenType == 0 )
	{
		// Get the datatype from the cache
		asUINT n = ReadEncodedUInt();
		*dt = savedDataTypes[n];
		return;
	}

	// Reserve a spot in the savedDataTypes
	size_t saveSlot = savedDataTypes.GetLength();
	savedDataTypes.PushLast(asCDataType());

	// Read the datatype for the first time
	asCObjectType *objType = 0;
	bool isObjectHandle  = false;
	bool isReadOnly      = false;
	bool isHandleToConst = false;
	bool isReference     = false;

	if( tokenType == ttIdentifier )
	{
		objType = ReadObjectType();
		ReadData(&isObjectHandle, 1);
		ReadData(&isHandleToConst, 1);
	}
	ReadData(&isReference, 1);
	ReadData(&isReadOnly, 1);

	asCScriptFunction *funcDef = 0;
	if( tokenType == ttIdentifier && objType && objType->name == "_builtin_function_" )
	{
		asCScriptFunction func(engine, module, asFUNC_DUMMY);
		ReadFunctionSignature(&func);
		if( error ) return;
		for( asUINT n = 0; n < engine->registeredFuncDefs.GetLength(); n++ )
		{
			// TODO: access: Only return the definitions that the module has access to
			if( engine->registeredFuncDefs[n]->name == func.name &&
				engine->registeredFuncDefs[n]->nameSpace == func.nameSpace )
			{
				funcDef = engine->registeredFuncDefs[n];
				break;
			}
		}

		if( !funcDef && module )
		{
			for( asUINT n = 0; n < module->funcDefs.GetLength(); n++ )
			{
				if( module->funcDefs[n]->name == func.name &&
					module->funcDefs[n]->nameSpace == func.nameSpace )
				{
					funcDef = module->funcDefs[n];
					break;
				}
			}
		}

		// Set to dummy to avoid unwanted release of resources
		func.funcType = asFUNC_DUMMY;
	}

	if( funcDef )
		*dt = asCDataType::CreateFuncDef(funcDef);
	else if( tokenType == ttIdentifier )
		*dt = asCDataType::CreateObject(objType, false);
	else
		*dt = asCDataType::CreatePrimitive(tokenType, false);
	if( isObjectHandle )
	{
		dt->MakeReadOnly(isHandleToConst);
		
		// Here we must allow a scoped type to be a handle 
		// e.g. if the datatype is for a system function
		dt->MakeHandle(true, true);
	}
	dt->MakeReadOnly(isReadOnly);
	dt->MakeReference(isReference);

	// Update the previously saved slot
	savedDataTypes[saveSlot] = *dt;
}

asCObjectType* asCReader::ReadObjectType() 
{
	asCObjectType *ot = 0;
	char ch;
	ReadData(&ch, 1);
	if( ch == 'a' )
	{
		// Read the name of the template type
		asCString typeName;
		ReadString(&typeName);
		asCObjectType *tmpl = engine->GetRegisteredObjectType(typeName.AddressOf(), engine->nameSpaces[0]);
		if( tmpl == 0 )
		{
			asCString str;
			str.Format(TXT_TEMPLATE_TYPE_s_DOESNT_EXIST, typeName.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			Error(TXT_INVALID_BYTECODE_d);
			return 0;
		}

		asUINT numSubTypes = ReadEncodedUInt();
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
			ot = engine->GetTemplateInstanceType(tmpl, subTypes);
		}

		if( ot == 0 )
		{
			// Show all subtypes in error message
			asCString sub = subTypes[0].Format();
			for( asUINT n = 1; n < subTypes.GetLength(); n++ )
			{
				sub += ",";
				sub += subTypes[n].Format();
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
		asCObjectType *st = ReadObjectType();
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

		if( typeName.GetLength() && typeName != "_builtin_object_" && typeName != "_builtin_function_" )
		{
			// Find the object type
			ot = module->GetObjectType(typeName.AddressOf(), nameSpace);
			if( !ot )
				ot = engine->GetRegisteredObjectType(typeName.AddressOf(), nameSpace);
			
			if( ot == 0 )
			{
				asCString str;
				str.Format(TXT_OBJECT_TYPE_s_DOESNT_EXIST, typeName.AddressOf());
				engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				Error(TXT_INVALID_BYTECODE_d);
				return 0;
			}
		}
		else if( typeName == "_builtin_object_" )
		{
			ot = &engine->scriptTypeBehaviours;
		}
		else if( typeName == "_builtin_function_" )
		{
			ot = &engine->functionBehaviours;
		}
		else
			asASSERT( false );
	}
	else
	{
		// No object type
		asASSERT( ch == '\0' );
		ot = 0;
	}

	return ot;
}

void asCReader::ReadByteCode(asCScriptFunction *func)
{
	asASSERT( func->scriptData );

	// Read number of instructions
	asUINT total, numInstructions;
	total = numInstructions = ReadEncodedUInt();

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
	asUINT count = ReadEncodedUInt();
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
	int c = ReadEncodedUInt();

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
			globProp = module->scriptGlobals.GetFirst(nameSpace, name);
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
	asUINT c = ReadEncodedUInt();

	usedObjectProperties.SetLength(c);
	for( asUINT n = 0; n < c; n++ )
	{
		asCObjectType *objType = ReadObjectType();
		if( objType == 0 )
		{
			Error(TXT_INVALID_BYTECODE_d);
			break;
		}

		asCString name;
		ReadString(&name);

		// Find the property offset
		bool found = false;
		for( asUINT p = 0; p < objType->properties.GetLength(); p++ )
		{
			if( objType->properties[p]->name == name )
			{
				usedObjectProperties[n].objType = objType;
				usedObjectProperties[n].offset = objType->properties[p]->byteOffset;
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
	if( index >= usedObjectProperties.GetLength() )
	{
		Error(TXT_INVALID_BYTECODE_d);
		return 0;
	}

	return (short)usedObjectProperties[index].offset;
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
	asCArray<asUINT> bcSizes(func->scriptData->byteCode.GetLength());
	asCArray<asUINT> instructionNbrToPos(func->scriptData->byteCode.GetLength());
	for( n = 0; n < func->scriptData->byteCode.GetLength(); )
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
	for( n = 0; n < func->scriptData->byteCode.GetLength(); bcNum++ )
	{
		int c = *(asBYTE*)&bc[n];
		if( c == asBC_REFCPY || 
			c == asBC_RefCpyV ||
			c == asBC_OBJTYPE )
		{
			// Translate the index to the true object type
			asPWORD *ot = (asPWORD*)&bc[n+1];
			*(asCObjectType**)ot = FindObjectType(*(int*)ot);
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
				*(((short*)&bc[n])+2) = (short)listAdj->AdjustOffset(*(((short*)&bc[n])+2), ot);
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
				 c == asBC_CALLSYS )
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
			*fid = (asPWORD)FindFunction((int)*fid);
		}
		else if( c == asBC_ALLOC )
		{
			// Translate the index to the true object type
			asPWORD *arg = (asPWORD*)&bc[n+1];
			*(asCObjectType**)arg = FindObjectType(*(int*)arg);

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
			// Translate the index to the true string id
			asWORD *arg = ((asWORD*)&bc[n])+1;

			if( *arg < usedStringConstants.GetLength() )	
				*arg = (asWORD)usedStringConstants[*arg];
			else
			{
				Error(TXT_INVALID_BYTECODE_d);
				return;
			}
		}
		else if( c == asBC_CALLBND )
		{
			// Translate the function id
			asUINT *fid = (asUINT*)&bc[n+1];
			if( *fid < module->bindInformations.GetLength() )
			{
				sBindInfo *bi = module->bindInformations[*fid];
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
			// Translate the global var index to pointer
			asPWORD *index = (asPWORD*)&bc[n+1];
			if( *(asUINT*)index < usedGlobalProperties.GetLength() )
				*(void**)index = usedGlobalProperties[*(asUINT*)index];
			else
			{
				Error(TXT_INVALID_BYTECODE_d);
				return;
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
			asCObjectType *ot = func->GetObjectTypeOfLocalVar(asBC_SWORDARG0(&bc[n]));
			listAdjusters.PushLast(asNEW(SListAdjuster)(&bc[n], ot));
		}
		else if( c == asBC_FREE )
		{
			// Translate the index to the true object type
			asPWORD *pot = (asPWORD*)&bc[n+1];
			*(asCObjectType**)pot = FindObjectType(*(int*)pot);

			asCObjectType *ot = *(asCObjectType**)pot;
			if( ot && (ot->flags & asOBJ_LIST_PATTERN) )
			{
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
			bc[n+1] = listAdj->AdjustOffset(bc[n+1], listAdj->patternType);

			// Inform the list adjuster how many values will be repeated
			listAdj->SetRepeatCount(bc[n+2]);
		}
		else if( c == asBC_PshListElmnt )
		{
			// Adjust the offset in the list where the size is informed
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			bc[n+1] = listAdj->AdjustOffset(bc[n+1], listAdj->patternType);
		}
		else if( c == asBC_SetListType )
		{
			// Adjust the offset in the list where the typeid is informed
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			bc[n+1] = listAdj->AdjustOffset(bc[n+1], listAdj->patternType);

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
	for( n = 0; n < func->scriptData->byteCode.GetLength(); )
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
	{
		func->scriptData->objVariablePos[n] = AdjustStackPosition(func->scriptData->objVariablePos[n]);
		func->scriptData->funcVariableTypes[n] = FindFunction((int)(asPWORD)func->scriptData->funcVariableTypes[n]);
	}

	// Adjust the get offsets. This must be done in the second iteration because
	// it relies on the function ids and variable position already being correct in the 
	// bytecodes that come after the GET instructions.
	// TODO: optimize: Instead of doing a full extra loop. We can push the GET instructions
	//                 on a stack, and then when a call instruction is found update all of them.
	//                 This will also make the AdjustGetOffset() function quicker as it can 
	//                 receive the called function directly instead of having to search for it.
	bc = func->scriptData->byteCode.AddressOf();
	for( n = 0; n < func->scriptData->byteCode.GetLength(); )
	{
		int c = *(asBYTE*)&bc[n];

		if( c == asBC_GETREF ||
		    c == asBC_GETOBJ ||
		    c == asBC_GETOBJREF )
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

	// The program position (every even number) needs to be adjusted
	// for the line numbers to be in number of dwords instead of number of instructions 
	for( n = 0; n < func->scriptData->lineNumbers.GetLength(); n += 2 )
		func->scriptData->lineNumbers[n] = instructionNbrToPos[func->scriptData->lineNumbers[n]];
	for( n = 0; n < func->scriptData->sectionIdxs.GetLength(); n += 2 )
		func->scriptData->sectionIdxs[n] = instructionNbrToPos[func->scriptData->sectionIdxs[n]];

	CalculateStackNeeded(func);
}

asCReader::SListAdjuster::SListAdjuster(asDWORD *bc, asCObjectType *listType) : 
	allocMemBC(bc), maxOffset(0), patternType(listType), repeatCount(0), lastOffset(-1), nextTypeId(-1)
{
	asASSERT( patternType && (patternType->flags & asOBJ_LIST_PATTERN) );

	// Find the first expected value in the list
	asSListPatternNode *node = patternType->engine->scriptFunctions[patternType->templateSubTypes[0].GetBehaviour()->listFactory]->listPattern;
	asASSERT( node && node->type == asLPT_START );
	patternNode = node->next;
}

int asCReader::SListAdjuster::AdjustOffset(int offset, asCObjectType *listPatternType)
{
	// TODO: cleanup: The listPatternType parameter is not needed
	asASSERT( listPatternType == patternType );
	UNUSED_VAR( listPatternType );

	asASSERT( offset >= lastOffset );

	// If it is the same offset being accessed again, just return the same adjusted value
	if( lastOffset == offset )
		return lastAdjustedOffset;

	lastOffset = offset;
	lastAdjustedOffset = maxOffset;

	// What is being expected at this position?
	if( patternNode->type == asLPT_REPEAT )
	{
		// Align the offset to 4 bytes boundary
		if( maxOffset & 0x3 )
		{
			maxOffset += 4 - (maxOffset & 0x3);
			lastAdjustedOffset = maxOffset;
		}

		// Don't move the patternNode yet because the caller must make a call to SetRepeatCount too
		maxOffset += 4;
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

				asCDataType dt = patternType->engine->GetDataTypeFromTypeId(nextTypeId);
				asUINT size;
				if( dt.IsObjectHandle() )
					size = AS_PTR_SIZE*4;
				else
					size = dt.GetSizeInMemoryBytes();

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

				return lastAdjustedOffset;
			}
		}
		else
		{
			if( repeatCount > 0 )
				repeatCount--;

			// Determine the size of the element
			asUINT size;
			asCDataType dt = reinterpret_cast<asSListPatternDataTypeNode*>(patternNode)->dataType;
			if( dt.IsObjectHandle() )
				size = AS_PTR_SIZE*4;
			else
				size = dt.GetSizeInMemoryBytes();

			// Align the offset to 4 bytes boundary
			if( size >= 4 && (maxOffset & 0x3) )
			{
				maxOffset += 4 - (maxOffset & 0x3);
				lastAdjustedOffset = maxOffset;
			}

			maxOffset += size;

			// Only move the patternNode if we're not expecting any more repeated entries
			if( repeatCount == 0 )
				patternNode = patternNode->next;

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

void asCReader::SListAdjuster::SetRepeatCount(asUINT rc)
{
	// Make sure the list is expecting a repeat at this location
	asASSERT( patternNode->type == asLPT_REPEAT );

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
		if( func->scriptData->objVariableTypes[n] )
		{
			// Determine the size the variable currently occupies on the stack
			int size = AS_PTR_SIZE;
			if( (func->scriptData->objVariableTypes[n]->GetFlags() & asOBJ_VALUE) &&
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
				return func->scriptData->funcVariableTypes[v];

		// Look in parameters
		int paramPos = 0;
		if( func->objectType )
			paramPos -= AS_PTR_SIZE;
		if( func->DoesReturnOnStack() )
			paramPos -= AS_PTR_SIZE;
		for( v = 0; v < func->parameterTypes.GetLength(); v++ )
		{
			if( var == paramPos )
				return func->parameterTypes[v].GetFuncDef();
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

	// Find out which function that will be called
	asCScriptFunction *calledFunc = 0;
	for( asUINT n = programPos; func->scriptData->byteCode.GetLength(); )
	{
		asBYTE bc = *(asBYTE*)&func->scriptData->byteCode[n];
		if( bc == asBC_CALL ||
			bc == asBC_CALLSYS ||
			bc == asBC_CALLINTF || 
			bc == asBC_ALLOC ||
			bc == asBC_CALLBND ||
			bc == asBC_CallPtr )
		{
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
	int currOffset = 0;
	if( offset > currOffset && calledFunc->GetObjectType() )
	{
		numPtrs++;
		currOffset++;
	}
	if( offset > currOffset && calledFunc->DoesReturnOnStack() )
	{
		numPtrs++;
		currOffset++;
	}
	for( asUINT p = 0; p < calledFunc->parameterTypes.GetLength(); p++ )
	{
		if( offset <= currOffset ) break;

		if( !calledFunc->parameterTypes[p].IsPrimitive() ||
			calledFunc->parameterTypes[p].IsReference() )
		{
			numPtrs++;
			currOffset++;

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

asCObjectType *asCReader::FindObjectType(int idx)
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
 : module(_module), stream(_stream), engine(_engine), stripDebugInfo(_stripDebug)
{
}

void asCWriter::WriteData(const void *data, asUINT size)
{
	asASSERT(size == 1 || size == 2 || size == 4 || size == 8);
#if defined(AS_BIG_ENDIAN)
	for( asUINT n = 0; n < size; n++ )
		stream->Write(((asBYTE*)data)+n, 1);
#else
	for( int n = size-1; n >= 0; n-- )
		stream->Write(((asBYTE*)data)+n, 1);
#endif
}

int asCWriter::Write() 
{
	unsigned long i, count;

	// Store everything in the same order that the builder parses scripts

	// TODO: Should be possible to skip saving the enum values. They are usually not needed after the script is compiled anyway
	// TODO: Should be possible to skip saving the typedefs. They are usually not needed after the script is compiled anyway
	// TODO: Should be possible to skip saving constants. They are usually not needed after the script is compiled anyway
	WriteData(&stripDebugInfo, sizeof(stripDebugInfo));

	// Store enums
	count = (asUINT)module->enumTypes.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; i++ )
	{
		WriteObjectTypeDeclaration(module->enumTypes[i], 1);
		WriteObjectTypeDeclaration(module->enumTypes[i], 2);
	}

	// Store type declarations first
	count = (asUINT)module->classTypes.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; i++ )
	{
		// Store only the name of the class/interface types
		WriteObjectTypeDeclaration(module->classTypes[i], 1);
	}

	// Store func defs
	count = (asUINT)module->funcDefs.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; i++ )
		WriteFunction(module->funcDefs[i]);

	// Now store all interface methods
	count = (asUINT)module->classTypes.GetLength();
	for( i = 0; i < count; i++ )
	{
		if( module->classTypes[i]->IsInterface() )
			WriteObjectTypeDeclaration(module->classTypes[i], 2);
	}

	// Then store the class methods and behaviours
	for( i = 0; i < count; ++i )
	{
		if( !module->classTypes[i]->IsInterface() )
			WriteObjectTypeDeclaration(module->classTypes[i], 2);
	}

	// Then store the class properties
	for( i = 0; i < count; ++i )
	{
		if( !module->classTypes[i]->IsInterface() )
			WriteObjectTypeDeclaration(module->classTypes[i], 3);
	}

	// Store typedefs
	count = (asUINT)module->typeDefs.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; i++ )
	{
		WriteObjectTypeDeclaration(module->typeDefs[i], 1);
		WriteObjectTypeDeclaration(module->typeDefs[i], 2);
	}

	// scriptGlobals[]
	count = (asUINT)module->scriptGlobals.GetSize();
	WriteEncodedInt64(count);
	asCSymbolTable<asCGlobalProperty>::iterator it = module->scriptGlobals.List();
	for( ; it; it++ )
		WriteGlobalProperty(*it);

	// scriptFunctions[]
	count = 0;
	for( i = 0; i < module->scriptFunctions.GetLength(); i++ )
		if( module->scriptFunctions[i]->objectType == 0 )
			count++;
	WriteEncodedInt64(count);
	for( i = 0; i < module->scriptFunctions.GetLength(); ++i )
		if( module->scriptFunctions[i]->objectType == 0 )
			WriteFunction(module->scriptFunctions[i]);

	// globalFunctions[]
	count = (int)module->globalFunctions.GetSize();
	asCSymbolTable<asCScriptFunction>::iterator funcIt = module->globalFunctions.List();
	WriteEncodedInt64(count);
	while( funcIt )
	{
		WriteFunction(*funcIt);
		funcIt++;
	}

	// bindInformations[]
	count = (asUINT)module->bindInformations.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; ++i )
	{
		WriteFunction(module->bindInformations[i]->importedFunctionSignature);
		WriteString(&module->bindInformations[i]->importFromModule);
	}

	// usedTypes[]
	count = (asUINT)usedTypes.GetLength();
	WriteEncodedInt64(count);
	for( i = 0; i < count; ++i )
		WriteObjectType(usedTypes[i]);

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

	return asSUCCESS;
}

int asCWriter::FindStringConstantIndex(int id)
{
	asSMapNode<int,int> *cursor = 0;
	if (stringIdToIndexMap.MoveTo(&cursor, id))
		return cursor->value;

	usedStringConstants.PushLast(id);
	int index = int(usedStringConstants.GetLength() - 1);
	stringIdToIndexMap.Insert(id, index);
	return index;
}

void asCWriter::WriteUsedStringConstants()
{
	asUINT count = (asUINT)usedStringConstants.GetLength();
	WriteEncodedInt64(count);
	for( asUINT i = 0; i < count; ++i )
		WriteString(engine->stringConstants[usedStringConstants[i]]);
}

void asCWriter::WriteUsedFunctions()
{
	asUINT count = (asUINT)usedFunctions.GetLength();
	WriteEncodedInt64(count);

	for( asUINT n = 0; n < usedFunctions.GetLength(); n++ )
	{
		char c;

		// Write enough data to be able to uniquely identify the function upon load

		if( usedFunctions[n] )
		{
			// Is the function from the module or the application?
			c = usedFunctions[n]->module ? 'm' : 'a';
			WriteData(&c, 1);
			WriteFunctionSignature(usedFunctions[n]);
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
	count = 0;
	for( i = asUINT(func->inOutFlags.GetLength()); i > 0; i-- )
		if( func->inOutFlags[i-1] != asTM_NONE )
		{
			count = i;
			break;
		}
	WriteEncodedInt64(count);
	for( i = 0; i < count; ++i )
		WriteEncodedInt64(func->inOutFlags[i]);

	WriteEncodedInt64(func->funcType);

	// Write the default args, from last to first
	count = 0;
	for( i = (asUINT)func->defaultArgs.GetLength(); i-- > 0; )
		if( func->defaultArgs[i] )
			count++;
	WriteEncodedInt64(count);
	for( i = (asUINT)func->defaultArgs.GetLength(); i-- > 0; )
		if( func->defaultArgs[i] )
			WriteString(func->defaultArgs[i]);

	WriteObjectType(func->objectType);

	if( func->objectType )
	{
		asBYTE b = 0;
		b += func->isReadOnly ? 1 : 0;
		b += func->isPrivate  ? 2 : 0;
		WriteData(&b, 1);
	}
	else
	{
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
		// Calculate the adjustment by position lookup table
		CalculateAdjustmentByPos(func);

		WriteByteCode(func);

		asDWORD varSpace = AdjustStackPosition(func->scriptData->variableSpace);
		WriteEncodedInt64(varSpace);

		count = (asUINT)func->scriptData->objVariablePos.GetLength();
		WriteEncodedInt64(count);
		for( i = 0; i < count; ++i )
		{
			WriteObjectType(func->scriptData->objVariableTypes[i]);
			// TODO: Only write this if the object type is the builtin function type
			WriteEncodedInt64(FindFunctionIndex(func->scriptData->funcVariableTypes[i]));
			WriteEncodedInt64(AdjustStackPosition(func->scriptData->objVariablePos[i]));
		}
		if( count > 0 )
			WriteEncodedInt64(func->scriptData->objVariablesOnHeap);

		WriteEncodedInt64((asUINT)func->scriptData->objVariableInfo.GetLength());
		for( i = 0; i < func->scriptData->objVariableInfo.GetLength(); ++i )
		{
			// The program position must be adjusted to be in number of instructions
			WriteEncodedInt64(bytecodeNbrByPos[func->scriptData->objVariableInfo[i].programPos]);
			WriteEncodedInt64(AdjustStackPosition(func->scriptData->objVariableInfo[i].variableOffset));
			WriteEncodedInt64(func->scriptData->objVariableInfo[i].option);
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
						char c = 0;
						WriteData(&c, 1);
					}
				}
			}
		}

		WriteData(&func->isShared, 1);

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

		WriteData(&func->dontCleanUpOnException, 1);

		// Store script section name
		if( !stripDebugInfo )
		{
			if( func->scriptData->scriptSectionIdx >= 0 )
				WriteString(engine->scriptSectionNames[func->scriptData->scriptSectionIdx]);
			else
			{
				char c = 0;
				WriteData(&c, 1);
			}
			WriteEncodedInt64(func->scriptData->declaredAt);
		}
	}
	else if( func->funcType == asFUNC_VIRTUAL || func->funcType == asFUNC_INTERFACE )
	{
		// TODO: Do we really need to store this? It can probably be reconstructed by the reader
		WriteEncodedInt64(func->vfTableIdx);
	}
}

void asCWriter::WriteObjectTypeDeclaration(asCObjectType *ot, int phase)
{
	if( phase == 1 )
	{
		// name
		WriteString(&ot->name);
		// flags
		WriteData(&ot->flags, 4);

		// size
		// TODO: Do we really need to store this? The reader should be able to 
		//       determine the correct size from the object type's flags
		if( (ot->flags & asOBJ_SCRIPT_OBJECT) && ot->size > 0 )
		{
			// The size for script objects may vary from platform to platform so 
			// only store 1 to diferentiate from interfaces that have size 0.
			WriteEncodedInt64(1); 
		}
		else
		{
			// Enums, typedefs, and interfaces have fixed sizes independently
			// of platform so it is safe to serialize the size directly.
			WriteEncodedInt64(ot->size);
		}

		// namespace
		WriteString(&ot->nameSpace->name);
	}
	else if( phase == 2 )
	{
		if( ot->flags & asOBJ_ENUM )
		{
			// enumValues[]
			int size = (int)ot->enumValues.GetLength();
			WriteEncodedInt64(size);

			for( int n = 0; n < size; n++ )
			{
				WriteString(&ot->enumValues[n]->name);
				WriteData(&ot->enumValues[n]->value, 4);
			}
		}
		else if( ot->flags & asOBJ_TYPEDEF )
		{
			eTokenType t = ot->templateSubTypes[0].GetTokenType();
			WriteEncodedInt64(t);
		}
		else
		{
			WriteObjectType(ot->derivedFrom);

			// interfaces[] / interfaceVFTOffsets[]
			// TOOD: Is it really necessary to store the VFTOffsets? Can't the reader calculate those?
			int size = (asUINT)ot->interfaces.GetLength();
			WriteEncodedInt64(size);
			asUINT n;
			for( n = 0; n < ot->interfaces.GetLength(); n++ )
			{
				WriteObjectType(ot->interfaces[n]);
				WriteEncodedInt64(ot->interfaceVFTOffsets[n]);
			}

			// behaviours
			// TODO: Default behaviours should just be stored as a indicator  
			//       to avoid storing the actual function object
			if( !ot->IsInterface() && ot->flags != asOBJ_TYPEDEF && ot->flags != asOBJ_ENUM )
			{
				WriteFunction(engine->scriptFunctions[ot->beh.destruct]);
				size = (int)ot->beh.constructors.GetLength();
				WriteEncodedInt64(size);
				for( n = 0; n < ot->beh.constructors.GetLength(); n++ )
				{
					WriteFunction(engine->scriptFunctions[ot->beh.constructors[n]]);
					WriteFunction(engine->scriptFunctions[ot->beh.factories[n]]);
				}
			}

			// methods[]
			// TODO: Avoid storing inherited methods in interfaces, as the reader
			//       can add those directly from the base interface
			size = (int)ot->methods.GetLength();
			WriteEncodedInt64(size);
			for( n = 0; n < ot->methods.GetLength(); n++ )
			{
				WriteFunction(engine->scriptFunctions[ot->methods[n]]);
			}

			// virtualFunctionTable[]
			// TODO: Is it really necessary to store this? Can't it be easily rebuilt by the reader
			size = (int)ot->virtualFunctionTable.GetLength();
			WriteEncodedInt64(size);
			for( n = 0; n < (asUINT)size; n++ )
			{
				WriteFunction(ot->virtualFunctionTable[n]);
			}
		}
	}
	else if( phase == 3 )
	{
		// properties[]
		asUINT size = (asUINT)ot->properties.GetLength();
		WriteEncodedInt64(size);
		for( asUINT n = 0; n < ot->properties.GetLength(); n++ )
		{
			WriteObjectProperty(ot->properties[n]);
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
	// TODO: All strings should be stored in a separate section, and when
	//       they are used an offset into that section should be stored.
	//       This will make it unnecessary to store the extra byte to 
	//       identify new versus old strings.

	if( str->GetLength() == 0 )
	{
		char z = '\0';
		WriteData(&z, 1);
		return;
	}

	// First check if the string hasn't been saved already
	asSMapNode<asCStringPointer, int> *cursor = 0;
	if (stringToIdMap.MoveTo(&cursor, asCStringPointer(str)))
	{
		// Save a reference to the existing string
		char b = 'r';
		WriteData(&b, 1);
		WriteEncodedInt64(cursor->value);
		return;
	}

	// Save a new string
	char b = 'n';
	WriteData(&b, 1);

	asUINT len = (asUINT)str->GetLength();
	WriteEncodedInt64(len);
	stream->Write(str->AddressOf(), (asUINT)len);

	savedStrings.PushLast(*str);
	stringToIdMap.Insert(asCStringPointer(str), int(savedStrings.GetLength()) - 1);
}

void asCWriter::WriteGlobalProperty(asCGlobalProperty* prop) 
{
	// TODO: We might be able to avoid storing the name and type of the global 
	//       properties twice if we merge this with the WriteUsedGlobalProperties. 
	WriteString(&prop->name);
	WriteString(&prop->nameSpace->name);
	WriteDataType(&prop->type);

	// Store the initialization function
	if( prop->GetInitFunc() )
	{
		bool f = true;
		WriteData(&f, 1);

		WriteFunction(prop->GetInitFunc());
	}
	else
	{
		bool f = false;
		WriteData(&f, 1);
	}
}

void asCWriter::WriteObjectProperty(asCObjectProperty* prop) 
{
	WriteString(&prop->name);
	WriteDataType(&prop->type);
	WriteData(&prop->isPrivate, 1);
}

void asCWriter::WriteDataType(const asCDataType *dt) 
{
	// First check if the datatype has already been saved
	for( asUINT n = 0; n < savedDataTypes.GetLength(); n++ )
	{
		if( *dt == savedDataTypes[n] )
		{
			asUINT c = 0;
			WriteEncodedInt64(c);
			WriteEncodedInt64(n);
			return;
		}
	}

	// Save the new datatype
	savedDataTypes.PushLast(*dt);

	bool b;
	int t = dt->GetTokenType();
	WriteEncodedInt64(t);
	if( t == ttIdentifier )
	{
		WriteObjectType(dt->GetObjectType());
		b = dt->IsObjectHandle();
		WriteData(&b, 1);
		b = dt->IsHandleToConst();
		WriteData(&b, 1);
	}
	b = dt->IsReference();
	WriteData(&b, 1);
	b = dt->IsReadOnly();
	WriteData(&b, 1);

	if( t == ttIdentifier && dt->GetObjectType()->name == "_builtin_function_" )
	{
		WriteFunctionSignature(dt->GetFuncDef());
	}
}

void asCWriter::WriteObjectType(asCObjectType* ot) 
{
	char ch;

	if( ot )
	{
		// Check for template instances/specializations
		if( ot->templateSubTypes.GetLength() )
		{
			// Check for list pattern type or template type
			if( ot->flags & asOBJ_LIST_PATTERN )
			{
				ch = 'l';
				WriteData(&ch, 1);
				WriteObjectType(ot->templateSubTypes[0].GetObjectType());
			}
			else
			{
				ch = 'a';
				WriteData(&ch, 1);
				WriteString(&ot->name);

				WriteEncodedInt64(ot->templateSubTypes.GetLength());
				for( asUINT n = 0; n < ot->templateSubTypes.GetLength(); n++ )
				{
					if( ot->templateSubTypes[0].IsObject() || ot->templateSubTypes[0].IsEnumType() )
					{
						ch = 's';
						WriteData(&ch, 1);
						WriteDataType(&ot->templateSubTypes[0]);
					}
					else
					{
						ch = 't';
						WriteData(&ch, 1);
						eTokenType t = ot->templateSubTypes[0].GetTokenType();
						WriteEncodedInt64(t);
					}
				}
			}
		}
		else if( ot->flags & asOBJ_TEMPLATE_SUBTYPE )
		{
			ch = 's';
			WriteData(&ch, 1);
			WriteString(&ot->name);
		}
		else
		{
			ch = 'o';
			WriteData(&ch, 1);
			WriteString(&ot->name);
			WriteString(&ot->nameSpace->name);
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
		if( func->scriptData->objVariableTypes[n] )
		{
			// Determine the size the variable currently occupies on the stack
			int size = AS_PTR_SIZE;
			if( (func->scriptData->objVariableTypes[n]->GetFlags() & asOBJ_VALUE) &&
				n >= func->scriptData->objVariablesOnHeap )
			{
				size = func->scriptData->objVariableTypes[n]->GetSize();
				if( size < 4 ) 
					size = 1; 
				else 
					size /= 4;
			}

			// If larger than 1 dword, adjust the offsets accordingly
			if( size > 1 )
			{
				// How much needs to be adjusted?
				adjustments.PushLast(func->scriptData->objVariablePos[n]);
				adjustments.PushLast(-(size-1));
			}
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
	size_t length = func->scriptData->byteCode.GetLength();
	asDWORD *bc = func->scriptData->byteCode.AddressOf();
	bytecodeNbrByPos.SetLength(length);
	asUINT num;
	for( offset = 0, num = 0; offset < length; )
	{
		bytecodeNbrByPos[offset] = num;
		offset += asBCTypeSize[asBCInfo[*(asBYTE*)(bc+offset)].type];
		num++;
	}
	// The last instruction is always a BC_RET. This make it possible to query 
	// the number of instructions by checking the last entry in bytecodeNbrByPos
	asASSERT(*(asBYTE*)(bc+length-1) == asBC_RET);
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

	// Find out which function that will be called
	asCScriptFunction *calledFunc = 0;
	for( asUINT n = programPos; n < func->scriptData->byteCode.GetLength(); )
	{
		asBYTE bc = *(asBYTE*)&func->scriptData->byteCode[n];
		if( bc == asBC_CALL ||
			bc == asBC_CALLSYS ||
			bc == asBC_CALLINTF )
		{
			// Find the function from the function id in bytecode
			int funcId = asBC_INTARG(&func->scriptData->byteCode[n]);
			calledFunc = engine->scriptFunctions[funcId];
			break;
		}
		else if( bc == asBC_ALLOC )
		{
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
					calledFunc = func->scriptData->funcVariableTypes[v];
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
						calledFunc = func->parameterTypes[v].GetFuncDef();
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

		n += asBCTypeSize[asBCInfo[bc].type];
	}

	asASSERT( calledFunc );

	// Count the number of pointers pushed on the stack above the 
	// current offset, and then adjust the offset accordingly
	asUINT numPtrs = 0;
	int currOffset = 0;
	if( offset > currOffset && calledFunc->GetObjectType() )
	{
		numPtrs++;
		currOffset += AS_PTR_SIZE;
	}
	if( offset > currOffset && calledFunc->DoesReturnOnStack() )
	{
		numPtrs++;
		currOffset += AS_PTR_SIZE;
	}
	for( asUINT p = 0; p < calledFunc->parameterTypes.GetLength(); p++ )
	{
		if( offset <= currOffset ) break;

		if( !calledFunc->parameterTypes[p].IsPrimitive() ||
			calledFunc->parameterTypes[p].IsReference() )
		{
			// objects and references are passed by pointer
			numPtrs++;
			currOffset += AS_PTR_SIZE;

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
		asDWORD tmp[4]; // The biggest instructions take up 4 DWORDs
		asDWORD c = *(asBYTE*)bc;

		// Copy the instruction to a temp buffer so we can work on it before saving
		memcpy(tmp, bc, asBCTypeSize[asBCInfo[c].type]*sizeof(asDWORD));

		if( c == asBC_ALLOC ) // PTR_DW_ARG
		{
			// Translate the object type 
			asCObjectType *ot = *(asCObjectType**)(tmp+1);
			*(asPWORD*)(tmp+1) = FindObjectTypeIdx(ot);

			// Translate the constructor func id, unless it is 0
			if( *(int*)&tmp[1+AS_PTR_SIZE] != 0 )
			{
				// Increment 1 to the translated function id, as 0 will be reserved for no function
				*(int*)&tmp[1+AS_PTR_SIZE] = 1+FindFunctionIndex(engine->scriptFunctions[*(int*)&tmp[1+AS_PTR_SIZE]]);
			}
		}
		else if( c == asBC_REFCPY  || // PTR_ARG
				 c == asBC_RefCpyV || // wW_PTR_ARG
				 c == asBC_OBJTYPE )  // PTR_ARG
		{
			// Translate object type pointers into indices
			*(asPWORD*)(tmp+1) = FindObjectTypeIdx(*(asCObjectType**)(tmp+1));
		}
		else if( c == asBC_JitEntry ) // PTR_ARG
		{
			// We don't store the JIT argument
			*(asPWORD*)(tmp+1) = 0;
		}
		else if( c == asBC_TYPEID || // DW_ARG
			     c == asBC_Cast )    // DW_ARG
		{
			// Translate type ids into indices
			*(int*)(tmp+1) = FindTypeIdIdx(*(int*)(tmp+1));
		}
		else if( c == asBC_ADDSi ||      // W_DW_ARG
			     c == asBC_LoadThisR )   // W_DW_ARG
		{
			// Translate property offsets into indices
			*(((short*)tmp)+1) = (short)FindObjectPropIndex(*(((short*)tmp)+1), *(int*)(tmp+1));

			// Translate type ids into indices
			*(int*)(tmp+1) = FindTypeIdIdx(*(int*)(tmp+1));
		}
		else if( c == asBC_LoadRObjR ||    // rW_W_DW_ARG
			     c == asBC_LoadVObjR )     // rW_W_DW_ARG
		{
			asCObjectType *ot = engine->GetObjectTypeFromTypeId(*(int*)(tmp+2));
			if( ot->flags & asOBJ_LIST_PATTERN )
			{
				// List patterns have a different way of translating the offsets
				SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
				*(((short*)tmp)+2) = (short)listAdj->AdjustOffset(*(((short*)tmp)+2), ot);
			}
			else
			{
				// Translate property offsets into indices
				// TODO: optimize: Pass the object type directly to the method instead of the type id
				*(((short*)tmp)+2) = (short)FindObjectPropIndex(*(((short*)tmp)+2), *(int*)(tmp+2));
			}

			// Translate type ids into indices
			*(int*)(tmp+2) = FindTypeIdIdx(*(int*)(tmp+2));
		}
		else if( c == asBC_COPY )        // W_DW_ARG
		{
			// Translate type ids into indices
			*(int*)(tmp+1) = FindTypeIdIdx(*(int*)(tmp+1));

			// Update the WORDARG0 to 0, as this will be recalculated on the target platform
			asBC_WORDARG0(tmp) = 0;
		}
		else if( c == asBC_RET ) // W_ARG
		{
			// Save with arg 0, as this will be recalculated on the target platform
			asBC_WORDARG0(tmp) = 0;
		}
		else if( c == asBC_CALL ||     // DW_ARG
				 c == asBC_CALLINTF || // DW_ARG
				 c == asBC_CALLSYS )   // DW_ARG
		{
			// Translate the function id
			*(int*)(tmp+1) = FindFunctionIndex(engine->scriptFunctions[*(int*)(tmp+1)]);
		}
		else if( c == asBC_FuncPtr ) // PTR_ARG
		{
			// Translate the function pointer
			*(asPWORD*)(tmp+1) = FindFunctionIndex(*(asCScriptFunction**)(tmp+1));
		}
		else if( c == asBC_STR ) // W_ARG
		{
			// Translate the string constant id
			asWORD *arg = ((asWORD*)tmp)+1;
			*arg = (asWORD)FindStringConstantIndex(*arg);
		}
		else if( c == asBC_CALLBND ) // DW_ARG
		{
			// Translate the function id
			int funcId = tmp[1];
			for( asUINT n = 0; n < module->bindInformations.GetLength(); n++ )
				if( module->bindInformations[n]->importedFunctionSignature->id == funcId )
				{
					funcId = n;
					break;
				}

			tmp[1] = funcId;
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
			// Translate global variable pointers into indices
			*(asPWORD*)(tmp+1) = FindGlobalPropPtrIndex(*(void**)(tmp+1));
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
			int offset = *(int*)(tmp+1);

			// Determine instruction number for next instruction and destination
			int bcSeqNum = bytecodeNbrByPos[bc - startBC] + 1;
			asDWORD *targetBC = bc + 2 + offset;
			int targetBcSeqNum = bytecodeNbrByPos[targetBC - startBC];

			// Set the offset in number of instructions
			*(int*)(tmp+1) = targetBcSeqNum - bcSeqNum;
		}
		else if( c == asBC_GETOBJ ||    // W_ARG
			     c == asBC_GETOBJREF ||
				 c == asBC_GETREF )
		{
			// Adjust the offset according to the function call that comes after
			asBC_WORDARG0(tmp) = (asWORD)AdjustGetOffset(asBC_WORDARG0(tmp), func, asDWORD(bc - startBC));
		}
		else if( c == asBC_AllocMem )
		{
			// It's not necessary to store the size of the list buffer, as it will be recalculated in the reader
			asBC_DWORDARG(tmp) = 0;

			// Determine the type of the list pattern from the variable
			short var = asBC_WORDARG0(tmp);
			asCObjectType *ot = func->GetObjectTypeOfLocalVar(var);

			// Create this helper object to adjust the offset of the elements accessed in the buffer
			listAdjusters.PushLast(asNEW(SListAdjuster)(ot));
		}
		else if( c == asBC_FREE ) // wW_PTR_ARG
		{
			// Translate object type pointers into indices
			asCObjectType *ot = *(asCObjectType**)(tmp+1);
			*(asPWORD*)(tmp+1) = FindObjectTypeIdx(ot);

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
			tmp[1] = listAdj->AdjustOffset(tmp[1], listAdj->patternType);

			// Tell the adjuster how many repeated values there are
			listAdj->SetRepeatCount(tmp[2]);
		}
		else if( c == asBC_PshListElmnt )   // W_DW_ARG
		{
			// Adjust the offset in the initialization list
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			tmp[1] = listAdj->AdjustOffset(tmp[1], listAdj->patternType);
		}
		else if( c == asBC_SetListType )
		{
			// Adjust the offset in the initialization list
			SListAdjuster *listAdj = listAdjusters[listAdjusters.GetLength()-1];
			tmp[1] = listAdj->AdjustOffset(tmp[1], listAdj->patternType);

			// Inform the adjuster of the type id of the next element
			listAdj->SetNextType(tmp[2]);

			// Translate the type id
			tmp[2] = FindTypeIdIdx(tmp[2]);
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
				asBC_SWORDARG0(tmp) = (short)AdjustStackPosition(asBC_SWORDARG0(tmp));
			}
			break;

		case asBCTYPE_wW_rW_ARG:
		case asBCTYPE_wW_rW_DW_ARG:
		case asBCTYPE_rW_rW_ARG:
			{
				asBC_SWORDARG0(tmp) = (short)AdjustStackPosition(asBC_SWORDARG0(tmp));
				asBC_SWORDARG1(tmp) = (short)AdjustStackPosition(asBC_SWORDARG1(tmp));
			}
			break;

		case asBCTYPE_wW_rW_rW_ARG:
			{
				asBC_SWORDARG0(tmp) = (short)AdjustStackPosition(asBC_SWORDARG0(tmp));
				asBC_SWORDARG1(tmp) = (short)AdjustStackPosition(asBC_SWORDARG1(tmp));
				asBC_SWORDARG2(tmp) = (short)AdjustStackPosition(asBC_SWORDARG2(tmp));
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
				short w = *(((short*)tmp)+1);
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
				short w = *(((short*)tmp)+1);
				WriteEncodedInt64(w);

				// Write the dword argument
				WriteEncodedInt64((int)tmp[1]);
			}
			break;
		case asBCTYPE_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				WriteEncodedInt64((int)tmp[1]);
			}
			break;
		case asBCTYPE_DW_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the dword argument
				WriteEncodedInt64((int)tmp[1]);

				// Write the dword argument
				WriteEncodedInt64((int)tmp[2]);
			}
			break;
		case asBCTYPE_wW_rW_rW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the first argument
				short w = *(((short*)tmp)+1);
				WriteEncodedInt64(w);

				// Write the second argument
				w = *(((short*)tmp)+2);
				WriteEncodedInt64(w);

				// Write the third argument
				w = *(((short*)tmp)+3);
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
				short w = *(((short*)tmp)+1);
				WriteEncodedInt64(w);

				// Write the second argument
				w = *(((short*)tmp)+2);
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
				short w = *(((short*)tmp)+1);
				WriteEncodedInt64(w);

				// Write the second argument
				w = *(((short*)tmp)+2);
				WriteEncodedInt64(w);

				// Write the third argument
				int dw = tmp[2];
				WriteEncodedInt64(dw);
			}
			break;
		case asBCTYPE_QW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				asQWORD qw = *(asQWORD*)&tmp[1];
				WriteEncodedInt64(qw);
			}
			break;
		case asBCTYPE_QW_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the argument
				asQWORD qw = *(asQWORD*)&tmp[1];
				WriteEncodedInt64(qw);

				// Write the second argument
				int dw = tmp[3];
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
				short w = *(((short*)tmp)+1);
				WriteEncodedInt64(w);

				// Write the argument
				asQWORD qw = *(asQWORD*)&tmp[1];
				WriteEncodedInt64(qw);
			}
			break;
		case asBCTYPE_rW_DW_DW_ARG:
			{
				// Write the instruction code
				asBYTE b = (asBYTE)c;
				WriteData(&b, 1);

				// Write the short argument
				short w = *(((short*)tmp)+1);
				WriteEncodedInt64(w);

				// Write the dword argument
				WriteEncodedInt64((int)tmp[1]);

				// Write the dword argument
				WriteEncodedInt64((int)tmp[2]);
			}
			break;
		default:
			{
				// This should never happen
				asASSERT(false);

				// Store the bc as is
				for( int n = 0; n < asBCTypeSize[asBCInfo[c].type]; n++ )
					WriteData(&tmp[n], 4);
			}
		}

		// Move to the next instruction
		bc += asBCTypeSize[asBCInfo[c].type];
		length -= asBCTypeSize[asBCInfo[c].type];
	}
}

asCWriter::SListAdjuster::SListAdjuster(asCObjectType *ot) : patternType(ot), repeatCount(0), entries(0), lastOffset(-1), nextTypeId(-1)
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

	lastOffset = offset;

	// What is being expected at this position?
	if( patternNode->type == asLPT_REPEAT )
	{
		// Don't move the patternNode yet because the caller must make a call to SetRepeatCount too
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
				repeatCount--;

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
	asASSERT( patternNode->type == asLPT_REPEAT );

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
	int c = (int)usedGlobalProperties.GetLength();
	WriteEncodedInt64(c);

	for( int n = 0; n < c; n++ )
	{
		asPWORD *p = (asPWORD*)usedGlobalProperties[n];
		
		// First search for the global in the module
		char moduleProp = 0;
		asCGlobalProperty *prop = 0;
		asCSymbolTable<asCGlobalProperty>::iterator it = module->scriptGlobals.List();
		for( ; it; it++ )
		{
			if( p == (*it)->GetAddressOfValue() )
			{
				prop = (*it);
				moduleProp = 1;
				break;
			}
		}

		// If it is not in the module, it must be an application registered property
		if( !prop )
		{
			asCSymbolTable<asCGlobalProperty>::iterator it = engine->registeredGlobalProps.List();
			for( ; it; it++ )
			{
				if( it->GetAddressOfValue() == p )
				{
					prop = *it;
					break;
				}
			}
		}

		asASSERT(prop);

		// Store the name and type of the property so we can find it again on loading
		WriteString(&prop->name);
		WriteString(&prop->nameSpace->name);
		WriteDataType(&prop->type);

		// Also store whether the property is a module property or a registered property
		WriteData(&moduleProp, 1);
	}
}

void asCWriter::WriteUsedObjectProps()
{
	int c = (int)usedObjectProperties.GetLength();
	WriteEncodedInt64(c);

	for( asUINT n = 0; n < usedObjectProperties.GetLength(); n++ )
	{
		asCObjectType *objType = usedObjectProperties[n].objType;
		WriteObjectType(objType);

		// Find the property name
		for( asUINT p = 0; p < objType->properties.GetLength(); p++ )
		{
			if( objType->properties[p]->byteOffset == usedObjectProperties[n].offset )
			{
				WriteString(&objType->properties[p]->name);
				break;
			}
		}
	}
}

int asCWriter::FindObjectPropIndex(short offset, int typeId)
{
	asCObjectType *objType = engine->GetObjectTypeFromTypeId(typeId);
	for( asUINT n = 0; n < usedObjectProperties.GetLength(); n++ )
	{
		if( usedObjectProperties[n].objType == objType &&
			usedObjectProperties[n].offset  == offset )
			return n;
	}

	SObjProp prop = {objType, offset};
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

int asCWriter::FindObjectTypeIdx(asCObjectType *obj)
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

