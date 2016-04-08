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
// as_builder.cpp
//
// This is the class that manages the compilation of the scripts
//


#include "as_config.h"
#include "as_builder.h"
#include "as_parser.h"
#include "as_compiler.h"
#include "as_tokendef.h"
#include "as_string_util.h"
#include "as_outputbuffer.h"
#include "as_texts.h"
#include "as_scriptobject.h"
#include "as_debug.h"

BEGIN_AS_NAMESPACE

#ifndef AS_NO_COMPILER

// asCSymbolTable template specializations for sGlobalVariableDescription entries
template<>
void asCSymbolTable<sGlobalVariableDescription>::GetKey(const sGlobalVariableDescription *entry, asSNameSpaceNamePair &key) const
{
	asSNameSpace *ns = entry->ns;
	asCString name = entry->name;
	key = asSNameSpaceNamePair(ns, name);
}

// Comparator for exact variable search
class asCCompGlobVarType : public asIFilter
{
public:
	const asCDataType &m_type;
	asCCompGlobVarType(const asCDataType &type) : m_type(type) {}

	bool operator()(const void *p) const
	{
		const sGlobalVariableDescription* desc = reinterpret_cast<const sGlobalVariableDescription*>(p);
		return desc->datatype == m_type;
	}

private:
	// The assignment operator is required for MSVC9, otherwise it will complain that it is not possible to auto generate the operator
	asCCompGlobVarType &operator=(const asCCompGlobVarType &) {return *this;}
};

#endif

asCBuilder::asCBuilder(asCScriptEngine *_engine, asCModule *_module)
{
	this->engine = _engine;
	this->module = _module;
	silent = false;
}

asCBuilder::~asCBuilder()
{
#ifndef AS_NO_COMPILER
	asUINT n;

	// Free all functions
	for( n = 0; n < functions.GetLength(); n++ )
	{
		if( functions[n] )
		{
			if( functions[n]->node )
				functions[n]->node->Destroy(engine);

			asDELETE(functions[n],sFunctionDescription);
		}

		functions[n] = 0;
	}

	// Free all global variables
	asCSymbolTable<sGlobalVariableDescription>::iterator it = globVariables.List();
	while( it )
	{
		if( (*it)->declaredAtNode )
			(*it)->declaredAtNode->Destroy(engine);
		if( (*it)->initializationNode )
			(*it)->initializationNode->Destroy(engine);
		asDELETE((*it),sGlobalVariableDescription);
		it++;
	}
	globVariables.Clear();

	// Free all the loaded files
	for( n = 0; n < scripts.GetLength(); n++ )
	{
		if( scripts[n] )
			asDELETE(scripts[n],asCScriptCode);

		scripts[n] = 0;
	}

	// Free all class declarations
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		if( classDeclarations[n] )
		{
			if( classDeclarations[n]->node )
				classDeclarations[n]->node->Destroy(engine);

			asDELETE(classDeclarations[n],sClassDeclaration);
			classDeclarations[n] = 0;
		}
	}

	for( n = 0; n < interfaceDeclarations.GetLength(); n++ )
	{
		if( interfaceDeclarations[n] )
		{
			if( interfaceDeclarations[n]->node )
				interfaceDeclarations[n]->node->Destroy(engine);

			asDELETE(interfaceDeclarations[n],sClassDeclaration);
			interfaceDeclarations[n] = 0;
		}
	}

	for( n = 0; n < namedTypeDeclarations.GetLength(); n++ )
	{
		if( namedTypeDeclarations[n] )
		{
			if( namedTypeDeclarations[n]->node )
				namedTypeDeclarations[n]->node->Destroy(engine);

			asDELETE(namedTypeDeclarations[n],sClassDeclaration);
			namedTypeDeclarations[n] = 0;
		}
	}

	for( n = 0; n < funcDefs.GetLength(); n++ )
	{
		if( funcDefs[n] )
		{
			if( funcDefs[n]->node )
				funcDefs[n]->node->Destroy(engine);

			asDELETE(funcDefs[n],sFuncDef);
			funcDefs[n] = 0;
		}
	}

	for( n = 0; n < mixinClasses.GetLength(); n++ )
	{
		if( mixinClasses[n] )
		{
			if( mixinClasses[n]->node )
				mixinClasses[n]->node->Destroy(engine);

			asDELETE(mixinClasses[n],sMixinClass);
			mixinClasses[n] = 0;
		}
	}

#endif // AS_NO_COMPILER
}

void asCBuilder::Reset()
{
	numErrors = 0;
	numWarnings = 0;
	engine->preMessage.isSet = false;

#ifndef AS_NO_COMPILER
	// Clear the cache of known types
	hasCachedKnownTypes = false;
	knownTypes.EraseAll();
#endif
}

#ifndef AS_NO_COMPILER
int asCBuilder::AddCode(const char *name, const char *code, int codeLength, int lineOffset, int sectionIdx, bool makeCopy)
{
	asCScriptCode *script = asNEW(asCScriptCode);
	if( script == 0 )
		return asOUT_OF_MEMORY;

	int r = script->SetCode(name, code, codeLength, makeCopy);
	if( r < 0 )
	{
		asDELETE(script, asCScriptCode);
		return r;
	}

	script->lineOffset = lineOffset;
	script->idx = sectionIdx;
	scripts.PushLast(script);

	return 0;
}

void asCBuilder::EvaluateTemplateInstances(asUINT startIdx, bool keepSilent)
{
	// Backup the original message stream
	bool                       msgCallback     = engine->msgCallback;
	asSSystemFunctionInterface msgCallbackFunc = engine->msgCallbackFunc;
	void                      *msgCallbackObj  = engine->msgCallbackObj;

	// Set the new temporary message stream
	asCOutputBuffer outBuffer;
	if( keepSilent )
		engine->SetMessageCallback(asMETHOD(asCOutputBuffer, Callback), &outBuffer, asCALL_THISCALL);

	// Evaluate each of the template instances that have been created since the start of the build
	// TODO: This is not exactly correct, since another thread may have created template instances in parallel
	for( asUINT n = startIdx; n < engine->templateInstanceTypes.GetLength(); n++ )
	{
		bool dontGarbageCollect = false;
		asCObjectType *tmpl = engine->templateInstanceTypes[n];
		asCScriptFunction *callback = engine->scriptFunctions[tmpl->beh.templateCallback];
		if( callback && !engine->CallGlobalFunctionRetBool(tmpl, &dontGarbageCollect, callback->sysFuncIntf, callback) )
		{
			asCString sub = tmpl->templateSubTypes[0].Format(engine->nameSpaces[0]);
			for( asUINT n = 1; n < tmpl->templateSubTypes.GetLength(); n++ )
			{
				sub += ",";
				sub += tmpl->templateSubTypes[n].Format(engine->nameSpaces[0]);
			}
			asCString str;
			str.Format(TXT_INSTANCING_INVLD_TMPL_TYPE_s_s, tmpl->name.AddressOf(), sub.AddressOf());
			WriteError(tmpl->scriptSectionIdx >= 0 ? engine->scriptSectionNames[tmpl->scriptSectionIdx]->AddressOf() : "", str, tmpl->declaredAt&0xFFFFF, (tmpl->declaredAt>>20)&0xFFF);
		}
		else
		{
			// If the callback said this template instance won't be garbage collected then remove the flag
			if( dontGarbageCollect )
				tmpl->flags &= ~asOBJ_GC;
		}
	}

	// Restore message callback
	if( keepSilent )
	{
		engine->msgCallback     = msgCallback;
		engine->msgCallbackFunc = msgCallbackFunc;
		engine->msgCallbackObj  = msgCallbackObj;
	}
}

int asCBuilder::Build()
{
	Reset();

	// The template callbacks must only be called after the subtypes have a known structure,
	// otherwise the callback may think it is not possible to create the template instance, 
	// even though it is.
	// TODO: This flag shouldn't be set globally in the engine, as it would mean that another 
	//       thread requesting a template instance in parallel to the compilation wouldn't 
	//       evaluate the template instance. 
	engine->deferValidationOfTemplateTypes = true;
	asUINT numTempl = (asUINT)engine->templateInstanceTypes.GetLength();

	ParseScripts();

	// Compile the types first
	CompileInterfaces();
	CompileClasses(numTempl);

	// Evaluate the template instances one last time, this time with error messages, as we know 
	// all classes have been fully built and it is known which ones will need garbage collection.
	EvaluateTemplateInstances(numTempl, false);
	engine->deferValidationOfTemplateTypes = false;

	// Then the global variables. Here the variables declared with auto
	// will be resolved, so they can be accessed properly in the functions
	CompileGlobalVariables();

	// Finally the global functions and class methods
	CompileFunctions();

	// TODO: Attempt to reorder the initialization of global variables so that
	//       they do not access other uninitialized global variables out-of-order
	//       The builder needs to check for each of the global variable, what functions
	//       that are accessed, and what global variables are access by these functions.

	if( numWarnings > 0 && engine->ep.compilerWarnings == 2 )
		WriteError(TXT_WARNINGS_TREATED_AS_ERROR, 0, 0);

	if( numErrors > 0 )
		return asERROR;

	// Make sure something was compiled, otherwise return an error
	if( module->IsEmpty() )
	{
		WriteError(TXT_NOTHING_WAS_BUILT, 0, 0);
		return asERROR;
	}

	return asSUCCESS;
}

int asCBuilder::CompileGlobalVar(const char *sectionName, const char *code, int lineOffset)
{
	Reset();

	// Add the string to the script code
	asCScriptCode *script = asNEW(asCScriptCode);
	if( script == 0 )
		return asOUT_OF_MEMORY;

	script->SetCode(sectionName, code, true);
	script->lineOffset = lineOffset;
	script->idx = engine->GetScriptSectionNameIndex(sectionName ? sectionName : "");
	scripts.PushLast(script);

	// Parse the string
	asCParser parser(this);
	if( parser.ParseScript(scripts[0]) < 0 )
		return asERROR;

	asCScriptNode *node = parser.GetScriptNode();

	// Make sure there is nothing else than the global variable in the script code
	if( node == 0 ||
		node->firstChild == 0 ||
		node->firstChild != node->lastChild ||
		node->firstChild->nodeType != snDeclaration )
	{
		WriteError(TXT_ONLY_ONE_VARIABLE_ALLOWED, script, 0);
		return asERROR;
	}

	node = node->firstChild;
	node->DisconnectParent();
	RegisterGlobalVar(node, script, module->defaultNamespace);

	CompileGlobalVariables();

	// It is possible that the global variable initialization included anonymous functions that must be compiled too
	for( asUINT n = 0; n < functions.GetLength(); n++ )
	{
		asCCompiler compiler(engine);
		asCScriptFunction *func = engine->scriptFunctions[functions[n]->funcId];
		int r = compiler.CompileFunction(this, functions[n]->script, func->parameterNames, functions[n]->node, func, 0);
		if( r < 0 )
			break;
	}

	if( numWarnings > 0 && engine->ep.compilerWarnings == 2 )
		WriteError(TXT_WARNINGS_TREATED_AS_ERROR, 0, 0);

	// None of the functions should be added to the module if any error occurred, 
	// or it was requested that the functions wouldn't be added to the scope
	if( numErrors > 0 )
	{
		for( asUINT n = 0; n < functions.GetLength(); n++ )
		{
			asCScriptFunction *func = engine->scriptFunctions[functions[n]->funcId];
			if( module->globalFunctions.GetIndex(func) >= 0 )
			{
				module->globalFunctions.Erase(module->globalFunctions.GetIndex(func));
				module->scriptFunctions.RemoveValue(func);
				func->ReleaseInternal();
			}
		}
	}

	if( numErrors > 0 )
	{
		// Remove the variable from the module, if it was registered
		if( globVariables.GetSize() > 0 )
			module->RemoveGlobalVar(module->GetGlobalVarCount()-1);

		return asERROR;
	}

	return 0;
}
#endif

int asCBuilder::ValidateDefaultArgs(asCScriptCode *script, asCScriptNode *node, asCScriptFunction *func)
{
	int firstArgWithDefaultValue = -1;
	for( asUINT n = 0; n < func->defaultArgs.GetLength(); n++ )
	{
		if( func->defaultArgs[n] )
			firstArgWithDefaultValue = n;
		else if( firstArgWithDefaultValue >= 0 )
		{
			asCString str;
			str.Format(TXT_DEF_ARG_MISSING_IN_FUNC_s, func->GetDeclaration());
			WriteError(str, script, node);
			return asINVALID_DECLARATION;
		}
	}

	return 0;
}

#ifndef AS_NO_COMPILER
// This function will verify if the newly created function will conflict another overload due to having
// identical function arguments that are not default args, e.g: foo(int) and foo(int, int=0)
int asCBuilder::CheckForConflictsDueToDefaultArgs(asCScriptCode *script, asCScriptNode *node, asCScriptFunction *func, asCObjectType *objType)
{
	// TODO: Implement for global functions too
	if( func->objectType == 0 || objType == 0 ) return 0;

	asCArray<int> funcs;
	GetObjectMethodDescriptions(func->name.AddressOf(), objType, funcs, false);
	for( asUINT n = 0; n < funcs.GetLength(); n++ )
	{
		asCScriptFunction *func2 = engine->scriptFunctions[funcs[n]];
		if( func == func2 )
			continue;

		if( func->IsReadOnly() != func2->IsReadOnly() )
			continue;

		bool match = true;
		asUINT p = 0;
		for( ; p < func->parameterTypes.GetLength() && p < func2->parameterTypes.GetLength(); p++ )
		{
			// Only verify until the first argument with default args
			if( (func->defaultArgs.GetLength() > p && func->defaultArgs[p]) ||
				(func2->defaultArgs.GetLength() > p && func2->defaultArgs[p]) )
				break;

			if( func->parameterTypes[p] != func2->parameterTypes[p] ||
				func->inOutFlags[p] != func2->inOutFlags[p] )
			{
				match = false;
				break;
			}
		}

		if( match )
		{
			if( !((p >= func->parameterTypes.GetLength() && p < func2->defaultArgs.GetLength() && func2->defaultArgs[p]) ||
				  (p >= func2->parameterTypes.GetLength() && p < func->defaultArgs.GetLength() && func->defaultArgs[p])) )
			{
				// The argument lists match for the full length of the shorter, but the next
				// argument on the longer does not have a default arg so there is no conflict
				match = false;
			}
		}

		if( match )
		{
			WriteWarning(TXT_OVERLOAD_CONFLICTS_DUE_TO_DEFAULT_ARGS, script, node);
			WriteInfo(func->GetDeclaration(), script, node);
			WriteInfo(func2->GetDeclaration(), script, node);
			break;
		}
	}

	return 0;
}

int asCBuilder::CompileFunction(const char *sectionName, const char *code, int lineOffset, asDWORD compileFlags, asCScriptFunction **outFunc)
{
	asASSERT(outFunc != 0);

	Reset();

	// Add the string to the script code
	asCScriptCode *script = asNEW(asCScriptCode);
	if( script == 0 )
		return asOUT_OF_MEMORY;

	script->SetCode(sectionName, code, true);
	script->lineOffset = lineOffset;
	script->idx = engine->GetScriptSectionNameIndex(sectionName ? sectionName : "");
	scripts.PushLast(script);

	// Parse the string
	asCParser parser(this);
	if( parser.ParseScript(scripts[0]) < 0 )
		return asERROR;

	asCScriptNode *node = parser.GetScriptNode();

	// Make sure there is nothing else than the function in the script code
	if( node == 0 ||
		node->firstChild == 0 ||
		node->firstChild != node->lastChild ||
		node->firstChild->nodeType != snFunction )
	{
		WriteError(TXT_ONLY_ONE_FUNCTION_ALLOWED, script, 0);
		return asERROR;
	}

	// Find the function node
	node = node->firstChild;

	// Create the function
	bool isConstructor, isDestructor, isPrivate, isProtected, isFinal, isOverride, isShared;
	asCScriptFunction *func = asNEW(asCScriptFunction)(engine, compileFlags & asCOMP_ADD_TO_MODULE ? module : 0, asFUNC_SCRIPT);
	if( func == 0 )
		return asOUT_OF_MEMORY;

	GetParsedFunctionDetails(node, scripts[0], 0, func->name, func->returnType, func->parameterNames, func->parameterTypes, func->inOutFlags, func->defaultArgs, func->isReadOnly, isConstructor, isDestructor, isPrivate, isProtected, isFinal, isOverride, isShared, module->defaultNamespace);
	func->id                           = engine->GetNextScriptFunctionId();
	func->scriptData->scriptSectionIdx = engine->GetScriptSectionNameIndex(sectionName ? sectionName : "");
	int row, col;
	scripts[0]->ConvertPosToRowCol(node->tokenPos, &row, &col);
	func->scriptData->declaredAt       = (row & 0xFFFFF)|((col & 0xFFF)<<20);
	func->nameSpace                    = module->defaultNamespace;

	// Make sure the default args are declared correctly
	int r = ValidateDefaultArgs(script, node, func);
	if( r < 0 )
	{
		func->ReleaseInternal();
		return asERROR;
	}

	// Tell the engine that the function exists already so the compiler can access it
	if( compileFlags & asCOMP_ADD_TO_MODULE )
	{
		int r = CheckNameConflict(func->name.AddressOf(), node, scripts[0], module->defaultNamespace);
		if( r < 0 )
		{
			func->ReleaseInternal();
			return asERROR;
		}

		module->globalFunctions.Put(func);
		
		module->AddScriptFunction(func);
	}
	else
		engine->AddScriptFunction(func);

	// Fill in the function info for the builder too
	node->DisconnectParent();
	sFunctionDescription *funcDesc = asNEW(sFunctionDescription);
	if( funcDesc == 0 )
	{
		func->ReleaseInternal();
		return asOUT_OF_MEMORY;
	}

	functions.PushLast(funcDesc);
	funcDesc->script            = scripts[0];
	funcDesc->node              = node;
	funcDesc->name              = func->name;
	funcDesc->funcId            = func->id;
	funcDesc->paramNames        = func->parameterNames;
	funcDesc->isExistingShared  = false;

	// This must be done in a loop, as it is possible that additional functions get declared as lambda's in the code
	for( asUINT n = 0; n < functions.GetLength(); n++ )
	{
		asCCompiler compiler(engine);
		asCScriptFunction *func = engine->scriptFunctions[functions[n]->funcId];
		int r = compiler.CompileFunction(this, functions[n]->script, func->parameterNames, functions[n]->node, func, 0);
		if( r < 0 )
			break;
	}

	if( numWarnings > 0 && engine->ep.compilerWarnings == 2 )
		WriteError(TXT_WARNINGS_TREATED_AS_ERROR, 0, 0);

	// None of the functions should be added to the module if any error occurred, 
	// or it was requested that the functions wouldn't be added to the scope
	if( !(compileFlags & asCOMP_ADD_TO_MODULE) || numErrors > 0 )
	{
		for( asUINT n = 0; n < functions.GetLength(); n++ )
		{
			asCScriptFunction *func = engine->scriptFunctions[functions[n]->funcId];
			if( module->globalFunctions.GetIndex(func) >= 0 )
			{
				module->globalFunctions.Erase(module->globalFunctions.GetIndex(func));
				module->scriptFunctions.RemoveValue(func);
				func->ReleaseInternal();
			}
		}
	}

	if( numErrors > 0 )
	{
		// Release the function pointer that would otherwise be returned if no errors occured
		func->ReleaseInternal();

		return asERROR;
	}

	// Return the function
	*outFunc = func;

	return asSUCCESS;
}

void asCBuilder::ParseScripts()
{
	TimeIt("asCBuilder::ParseScripts");

	asCArray<asCParser*> parsers((int)scripts.GetLength());

	// Parse all the files as if they were one
	asUINT n = 0;
	for( n = 0; n < scripts.GetLength(); n++ )
	{
		asCParser *parser = asNEW(asCParser)(this);
		if( parser != 0 )
		{
			parsers.PushLast(parser);

			// Parse the script file
			parser->ParseScript(scripts[n]);
		}
	}

	if( numErrors == 0 )
	{
		// Find all type declarations
		for( n = 0; n < scripts.GetLength(); n++ )
		{
			asCScriptNode *node = parsers[n]->GetScriptNode();
			RegisterTypesFromScript(node, scripts[n], engine->nameSpaces[0]);
		}

		// Register the complete function definitions
		for( n = 0; n < funcDefs.GetLength(); n++ )
			CompleteFuncDef(funcDefs[n]);

		// Register script methods found in the interfaces
		for( n = 0; n < interfaceDeclarations.GetLength(); n++ )
		{
			sClassDeclaration *decl = interfaceDeclarations[n];
			asCScriptNode *node = decl->node->firstChild->next;

			// Skip list of inherited interfaces
			while( node && node->nodeType == snIdentifier )
				node = node->next;

			while( node )
			{
				asCScriptNode *next = node->next;
				if( node->nodeType == snFunction )
				{
					node->DisconnectParent();
					RegisterScriptFunctionFromNode(node, decl->script, decl->objType, true, false, 0, decl->isExistingShared);
				}
				else if( node->nodeType == snVirtualProperty )
				{
					node->DisconnectParent();
					RegisterVirtualProperty(node, decl->script, decl->objType, true, false, 0, decl->isExistingShared);
				}

				node = next;
			}
		}

		// Register script methods found in the classes
		for( n = 0; n < classDeclarations.GetLength(); n++ )
		{
			sClassDeclaration *decl = classDeclarations[n];

			asCScriptNode *node = decl->node->firstChild->next;

			// Skip list of classes and interfaces
			while( node && node->nodeType == snIdentifier )
				node = node->next;

			while( node )
			{
				asCScriptNode *next = node->next;
				if( node->nodeType == snFunction )
				{
					node->DisconnectParent();
					RegisterScriptFunctionFromNode(node, decl->script, decl->objType, false, false, 0, decl->isExistingShared);
				}
				else if( node->nodeType == snVirtualProperty )
				{
					node->DisconnectParent();
					RegisterVirtualProperty(node, decl->script, decl->objType, false, false, 0, decl->isExistingShared);
				}

				node = next;
			}

			// Make sure the default factory & constructor exists for classes
			if( decl->objType->beh.construct == engine->scriptTypeBehaviours.beh.construct )
			{
				if( decl->objType->beh.constructors.GetLength() == 1 || engine->ep.alwaysImplDefaultConstruct )
				{
					AddDefaultConstructor(decl->objType, decl->script);
				}
				else
				{
					// As the class has another constructor we shouldn't provide the default constructor
					if( decl->objType->beh.construct )
					{
						engine->scriptFunctions[decl->objType->beh.construct]->ReleaseInternal();
						decl->objType->beh.construct = 0;
						decl->objType->beh.constructors.RemoveIndex(0);
					}
					if( decl->objType->beh.factory )
					{
						engine->scriptFunctions[decl->objType->beh.factory]->ReleaseInternal();
						decl->objType->beh.factory = 0;
						decl->objType->beh.factories.RemoveIndex(0);
					}
					// Only remove the opAssign method if the script hasn't provided one
					if( decl->objType->beh.copy == engine->scriptTypeBehaviours.beh.copy )
					{
						engine->scriptFunctions[decl->objType->beh.copy]->ReleaseInternal();
						decl->objType->beh.copy = 0;
					}
				}
			}
		}

		// Find other global nodes
		for( n = 0; n < scripts.GetLength(); n++ )
		{
			// Find other global nodes
			asCScriptNode *node = parsers[n]->GetScriptNode();
			RegisterNonTypesFromScript(node, scripts[n], engine->nameSpaces[0]);
		}
	}

	for( n = 0; n < parsers.GetLength(); n++ )
	{
		asDELETE(parsers[n],asCParser);
	}
}

void asCBuilder::RegisterTypesFromScript(asCScriptNode *node, asCScriptCode *script, asSNameSpace *ns)
{
	asASSERT(node->nodeType == snScript);

	// Find structure definitions first
	node = node->firstChild;
	while( node )
	{
		asCScriptNode *next = node->next;
		if( node->nodeType == snNamespace )
		{
			// Recursively register the entities defined in the namespace
			asCString nsName;
			nsName.Assign(&script->code[node->firstChild->tokenPos], node->firstChild->tokenLength);
			if( ns->name != "" )
				nsName = ns->name + "::" + nsName;

			asSNameSpace *nsChild = engine->AddNameSpace(nsName.AddressOf());
			RegisterTypesFromScript(node->lastChild, script, nsChild);
		}
		else
		{
			if( node->nodeType == snClass )
			{
				node->DisconnectParent();
				RegisterClass(node, script, ns);
			}
			else if( node->nodeType == snInterface )
			{
				node->DisconnectParent();
				RegisterInterface(node, script, ns);
			}
			else if( node->nodeType == snEnum )
			{
				node->DisconnectParent();
				RegisterEnum(node, script, ns);
			}
			else if( node->nodeType == snTypedef )
			{
				node->DisconnectParent();
				RegisterTypedef(node, script, ns);
			}
			else if( node->nodeType == snFuncDef )
			{
				node->DisconnectParent();
				RegisterFuncDef(node, script, ns);
			}
			else if( node->nodeType == snMixin )
			{
				node->DisconnectParent();
				RegisterMixinClass(node, script, ns);
			}
		}

		node = next;
	}
}

void asCBuilder::RegisterNonTypesFromScript(asCScriptNode *node, asCScriptCode *script, asSNameSpace *ns)
{
	node = node->firstChild;
	while( node )
	{
		asCScriptNode *next = node->next;
		if( node->nodeType == snNamespace )
		{
			// Determine the name of the namespace
			asCString nsName;
			nsName.Assign(&script->code[node->firstChild->tokenPos], node->firstChild->tokenLength);
			if( ns->name != "" )
				nsName = ns->name + "::" + nsName;

			// Declare the namespace, then add the entities
			asSNameSpace *nsChild = engine->AddNameSpace(nsName.AddressOf());
			RegisterNonTypesFromScript(node->lastChild, script, nsChild);
		}
		else
		{
			node->DisconnectParent();
			if( node->nodeType == snFunction )
				RegisterScriptFunctionFromNode(node, script, 0, false, true, ns);
			else if( node->nodeType == snDeclaration )
				RegisterGlobalVar(node, script, ns);
			else if( node->nodeType == snVirtualProperty )
				RegisterVirtualProperty(node, script, 0, false, true, ns);
			else if( node->nodeType == snImport )
				RegisterImportedFunction(module->GetNextImportedFunctionId(), node, script, ns);
			else
			{
				// Unused script node
				int r, c;
				script->ConvertPosToRowCol(node->tokenPos, &r, &c);

				WriteWarning(script->name, TXT_UNUSED_SCRIPT_NODE, r, c);

				node->Destroy(engine);
			}
		}

		node = next;
	}
}

void asCBuilder::CompileFunctions()
{
	// Compile each function
	for( asUINT n = 0; n < functions.GetLength(); n++ )
	{
		sFunctionDescription *current = functions[n];
		if( current == 0 ) continue;

		// Don't compile the function again if it was an existing shared function
		if( current->isExistingShared ) continue;

		asCCompiler compiler(engine);
		asCScriptFunction *func = engine->scriptFunctions[current->funcId];

		// Find the class declaration for constructors
		sClassDeclaration *classDecl = 0;
		if( current->objType && current->name == current->objType->name )
		{
			for( asUINT n = 0; n < classDeclarations.GetLength(); n++ )
			{
				if( classDeclarations[n]->objType == current->objType )
				{
					classDecl = classDeclarations[n];
					break;
				}
			}

			asASSERT( classDecl );
		}

		if( current->node )
		{
			int r, c;
			current->script->ConvertPosToRowCol(current->node->tokenPos, &r, &c);

			asCString str = func->GetDeclarationStr();
			str.Format(TXT_COMPILING_s, str.AddressOf());
			WriteInfo(current->script->name, str, r, c, true);

			// When compiling a constructor need to pass the class declaration for member initializations
			compiler.CompileFunction(this, current->script, current->paramNames, current->node, func, classDecl);

			engine->preMessage.isSet = false;
		}
		else if( current->objType && current->name == current->objType->name )
		{
			asCScriptNode *node = classDecl->node;

			int r = 0, c = 0;
			if( node )
				current->script->ConvertPosToRowCol(node->tokenPos, &r, &c);

			asCString str = func->GetDeclarationStr();
			str.Format(TXT_COMPILING_s, str.AddressOf());
			WriteInfo(current->script->name, str, r, c, true);

			// This is the default constructor that is generated
			// automatically if not implemented by the user.
			compiler.CompileDefaultConstructor(this, current->script, node, func, classDecl);

			engine->preMessage.isSet = false;
		}
		else
		{
			asASSERT( false );
		}
	}
}
#endif

// Called from module and engine
int asCBuilder::ParseDataType(const char *datatype, asCDataType *result, asSNameSpace *implicitNamespace, bool isReturnType)
{
	Reset();

	asCScriptCode source;
	source.SetCode("", datatype, true);

	asCParser parser(this);
	int r = parser.ParseDataType(&source, isReturnType);
	if( r < 0 )
		return asINVALID_TYPE;

	// Get data type and property name
	asCScriptNode *dataType = parser.GetScriptNode()->firstChild;

	*result = CreateDataTypeFromNode(dataType, &source, implicitNamespace, true);
	if( isReturnType )
		*result = ModifyDataTypeFromNode(*result, dataType->next, &source, 0, 0);

	if( numErrors > 0 )
		return asINVALID_TYPE;

	return asSUCCESS;
}

int asCBuilder::ParseTemplateDecl(const char *decl, asCString *name, asCArray<asCString> &subtypeNames)
{
	Reset();

	asCScriptCode source;
	source.SetCode("", decl, true);

	asCParser parser(this);
	int r = parser.ParseTemplateDecl(&source);
	if( r < 0 )
		return asINVALID_TYPE;

	// Get the template name and subtype names
	asCScriptNode *node = parser.GetScriptNode()->firstChild;

	name->Assign(&decl[node->tokenPos], node->tokenLength);
	while( (node = node->next) != 0 )
	{
		asCString subtypeName;
		subtypeName.Assign(&decl[node->tokenPos], node->tokenLength);
		subtypeNames.PushLast(subtypeName);
	}

	// TODO: template: check for name conflicts

	if( numErrors > 0 )
		return asINVALID_DECLARATION;

	return asSUCCESS;
}

int asCBuilder::VerifyProperty(asCDataType *dt, const char *decl, asCString &name, asCDataType &type, asSNameSpace *ns)
{
	// Either datatype or namespace must be informed
	asASSERT( dt || ns );

	Reset();

	if( dt )
	{
		// Verify that the object type exist
		if( dt->GetObjectType() == 0 )
			return asINVALID_OBJECT;
	}

	// Check property declaration and type
	asCScriptCode source;
	source.SetCode(TXT_PROPERTY, decl, true);

	asCParser parser(this);
	int r = parser.ParsePropertyDeclaration(&source);
	if( r < 0 )
		return asINVALID_DECLARATION;

	// Get data type
	asCScriptNode *dataType = parser.GetScriptNode()->firstChild;

	// Check if the property is declared 'by reference'
	bool isReference = (dataType->next->tokenType == ttAmp);

	// Get the name of the property
	asCScriptNode *nameNode = isReference ? dataType->next->next : dataType->next;

	// If an object property is registered, then use the
	// object's namespace, otherwise use the specified namespace
	type = CreateDataTypeFromNode(dataType, &source, dt ? dt->GetObjectType()->nameSpace : ns);
	name.Assign(&decl[nameNode->tokenPos], nameNode->tokenLength);
	type.MakeReference(isReference);

	// Validate that the type really can be a registered property
	// We cannot use CanBeInstantiated, as it is allowed to register
	// properties of type that cannot otherwise be instantiated
	if( type.GetFuncDef() && !type.IsObjectHandle() )
	{
		// Function definitions must always be handles
		return asINVALID_DECLARATION;
	}

	// Verify property name
	if( dt )
	{
		if( CheckNameConflictMember(dt->GetObjectType(), name.AddressOf(), nameNode, &source, true) < 0 )
			return asNAME_TAKEN;
	}
	else
	{
		if( CheckNameConflict(name.AddressOf(), nameNode, &source, ns) < 0 )
			return asNAME_TAKEN;
	}

	if( numErrors > 0 )
		return asINVALID_DECLARATION;

	return asSUCCESS;
}

#ifndef AS_NO_COMPILER
asCObjectProperty *asCBuilder::GetObjectProperty(asCDataType &obj, const char *prop)
{
	asASSERT(obj.GetObjectType() != 0);

	// TODO: optimize: Improve linear search
	asCArray<asCObjectProperty *> &props = obj.GetObjectType()->properties;
	for( asUINT n = 0; n < props.GetLength(); n++ )
	{
		if( props[n]->name == prop )
		{
			if( module->accessMask & props[n]->accessMask )
				return props[n];
			else
				return 0;
		}
	}

	return 0;
}
#endif

bool asCBuilder::DoesGlobalPropertyExist(const char *prop, asSNameSpace *ns, asCGlobalProperty **outProp, sGlobalVariableDescription **outDesc, bool *isAppProp)
{
	if( outProp )   *outProp = 0;
	if( outDesc )   *outDesc = 0;
	if( isAppProp ) *isAppProp = false;

	// Check application registered properties
	asCString name(prop);
	asCGlobalProperty *globProp = engine->registeredGlobalProps.GetFirst(ns, name);
	if( globProp )
	{
		if( isAppProp ) *isAppProp = true;
		if( outProp )   *outProp   = globProp;
		return true;
	}

#ifndef AS_NO_COMPILER
	// Check properties being compiled now
	sGlobalVariableDescription* desc = globVariables.GetFirst(ns, prop);
	if( desc && !desc->isEnumValue )
	{
		if( outProp ) *outProp = desc->property;
		if( outDesc ) *outDesc = desc;
		return true;
	}
#endif

	// Check previously compiled global variables
	if( module )
	{
		globProp = module->scriptGlobals.GetFirst(ns, prop);
		if( globProp )
		{
			if( outProp ) *outProp = globProp;
			return true;
		}
	}

	return false;
}

asCGlobalProperty *asCBuilder::GetGlobalProperty(const char *prop, asSNameSpace *ns, bool *isCompiled, bool *isPureConstant, asQWORD *constantValue, bool *isAppProp)
{
	if( isCompiled )     *isCompiled     = true;
	if( isPureConstant ) *isPureConstant = false;
	if( isAppProp )      *isAppProp      = false;
	if( constantValue )  *constantValue  = 0;

	asCGlobalProperty          *globProp = 0;
	sGlobalVariableDescription *globDesc = 0;
	if( DoesGlobalPropertyExist(prop, ns, &globProp, &globDesc, isAppProp) )
	{
#ifndef AS_NO_COMPILER
		if( globDesc )
		{
			// The property was declared in this build call, check if it has been compiled successfully already
			if( isCompiled )     *isCompiled     = globDesc->isCompiled;
			if( isPureConstant ) *isPureConstant = globDesc->isPureConstant;
			if( constantValue  ) *constantValue  = globDesc->constantValue;
		}
		else 
#endif
		if( isAppProp )
		{
			// Don't return the property if the module doesn't have access to it
			if( !(module->accessMask & globProp->accessMask) )
				globProp = 0;
		}
		return globProp;
	}

	return 0;
}

int asCBuilder::ParseFunctionDeclaration(asCObjectType *objType, const char *decl, asCScriptFunction *func, bool isSystemFunction, asCArray<bool> *paramAutoHandles, bool *returnAutoHandle, asSNameSpace *ns, asCScriptNode **listPattern)
{
	asASSERT( objType || ns );

	// TODO: Can't we use GetParsedFunctionDetails to do most of what is done in this function?

	Reset();

	asCScriptCode source;
	source.SetCode(TXT_SYSTEM_FUNCTION, decl, true);

	asCParser parser(this);
	int r = parser.ParseFunctionDefinition(&source, listPattern != 0);
	if( r < 0 )
		return asINVALID_DECLARATION;

	asCScriptNode *node = parser.GetScriptNode();

	// Determine scope
	asCScriptNode *n = node->firstChild->next->next;
	asCString scope = GetScopeFromNode(n, &source, &n);
	if( scope == "::" )
		func->nameSpace = engine->nameSpaces[0];
	else
		func->nameSpace = engine->FindNameSpace(scope.AddressOf());
	if( func->nameSpace == 0 )
		return asINVALID_DECLARATION;

	// Find name
	func->name.Assign(&source.code[n->tokenPos], n->tokenLength);

	// Initialize a script function object for registration
	bool autoHandle;

	// Scoped reference types are allowed to use handle when returned from application functions
	func->returnType = CreateDataTypeFromNode(node->firstChild, &source, objType ? objType->nameSpace : ns, true, objType);
	func->returnType = ModifyDataTypeFromNode(func->returnType, node->firstChild->next, &source, 0, &autoHandle);
	if( autoHandle && (!func->returnType.IsObjectHandle() || func->returnType.IsReference()) )
		return asINVALID_DECLARATION;
	if( returnAutoHandle ) *returnAutoHandle = autoHandle;

	// Reference types cannot be returned by value from system functions
	if( isSystemFunction &&
		(func->returnType.GetObjectType() &&
		 (func->returnType.GetObjectType()->flags & asOBJ_REF)) &&
		!(func->returnType.IsReference() ||
		  func->returnType.IsObjectHandle()) )
		return asINVALID_DECLARATION;

	// Count number of parameters
	int paramCount = 0;
	asCScriptNode *paramList = n->next;
	n = paramList->firstChild;
	while( n )
	{
		paramCount++;
		n = n->next->next;
		if( n && n->nodeType == snIdentifier )
			n = n->next;

		if( n && n->nodeType == snExpression )
			n = n->next;
	}

	// Preallocate memory
	func->parameterTypes.Allocate(paramCount, false);
	func->parameterNames.SetLength(paramCount);
	func->inOutFlags.Allocate(paramCount, false);
	func->defaultArgs.Allocate(paramCount, false);
	if( paramAutoHandles ) paramAutoHandles->Allocate(paramCount, false);

	n = paramList->firstChild;
	asUINT index = 0;
	while( n )
	{
		asETypeModifiers inOutFlags;
		asCDataType type = CreateDataTypeFromNode(n, &source, objType ? objType->nameSpace : ns, false, objType);
		type = ModifyDataTypeFromNode(type, n->next, &source, &inOutFlags, &autoHandle);

		// Reference types cannot be passed by value to system functions
		if( isSystemFunction &&
			(type.GetObjectType() &&
			 (type.GetObjectType()->flags & asOBJ_REF)) &&
			!(type.IsReference() ||
			  type.IsObjectHandle()) )
			return asINVALID_DECLARATION;

		// Store the parameter type
		func->parameterTypes.PushLast(type);
		func->inOutFlags.PushLast(inOutFlags);

		// Don't permit void parameters
		if( type.GetTokenType() == ttVoid )
			return asINVALID_DECLARATION;

		if( autoHandle && (!type.IsObjectHandle() || type.IsReference()) )
			return asINVALID_DECLARATION;

		if( paramAutoHandles ) paramAutoHandles->PushLast(autoHandle);

		// Make sure that var type parameters are references
		if( type.GetTokenType() == ttQuestion &&
			!type.IsReference() )
			return asINVALID_DECLARATION;

		// Move to next parameter
		n = n->next->next;
		if( n && n->nodeType == snIdentifier ) 
		{
			func->parameterNames[index] = asCString(&source.code[n->tokenPos], n->tokenLength);
			n = n->next;
		}
		++index;

		if( n && n->nodeType == snExpression )
		{
			// Strip out white space and comments to better share the string
			asCString *defaultArgStr = asNEW(asCString);
			if( defaultArgStr )
			{
				*defaultArgStr = GetCleanExpressionString(n, &source);
				func->defaultArgs.PushLast(defaultArgStr);
			}

			n = n->next;
		}
		else
			func->defaultArgs.PushLast(0);
	}

	// Set the read-only flag if const is declared after parameter list
	n = paramList->next;
	if( n && n->nodeType == snUndefined && n->tokenType == ttConst )
	{
		if( objType == 0 )
			return asINVALID_DECLARATION;
		func->isReadOnly = true;

		n = n->next;
	}
	else
		func->isReadOnly = false;

	// If the caller expects a list pattern, check for the existence, else report an error if not
	if( listPattern )
	{
		if( n == 0 || n->nodeType != snListPattern ) 
			return asINVALID_DECLARATION;
		else
		{
			*listPattern = n;
			n->DisconnectParent();
		}
	}
	else
	{
		if( n )
			return asINVALID_DECLARATION;
	}
	
	// Make sure the default args are declared correctly
	ValidateDefaultArgs(&source, node, func);

	if( numErrors > 0 || numWarnings > 0 )
		return asINVALID_DECLARATION;

	return 0;
}

int asCBuilder::ParseVariableDeclaration(const char *decl, asSNameSpace *implicitNamespace, asCString &outName, asSNameSpace *&outNamespace, asCDataType &outDt)
{
	Reset();

	asCScriptCode source;
	source.SetCode(TXT_VARIABLE_DECL, decl, true);

	asCParser parser(this);

	int r = parser.ParsePropertyDeclaration(&source);
	if( r < 0 )
		return asINVALID_DECLARATION;

	asCScriptNode *node = parser.GetScriptNode();

	// Determine the scope from declaration
	asCScriptNode *n = node->firstChild->next;
	outNamespace = GetNameSpaceFromNode(n, &source, implicitNamespace, &n);
	if( outNamespace == 0 )
		return asINVALID_DECLARATION;

	// Find name
	outName.Assign(&source.code[n->tokenPos], n->tokenLength);

	// Initialize a script variable object for registration
	outDt = CreateDataTypeFromNode(node->firstChild, &source, implicitNamespace);

	if( numErrors > 0 || numWarnings > 0 )
		return asINVALID_DECLARATION;

	return 0;
}

int asCBuilder::CheckNameConflictMember(asCObjectType *t, const char *name, asCScriptNode *node, asCScriptCode *code, bool isProperty)
{
	// It's not necessary to check against object types

	// TODO: optimize: Improve linear search
	asCArray<asCObjectProperty *> &props = t->properties;
	for( asUINT n = 0; n < props.GetLength(); n++ )
	{
		if( props[n]->name == name )
		{
			if( code )
			{
				asCString str;
				str.Format(TXT_NAME_CONFLICT_s_OBJ_PROPERTY, name);
				WriteError(str, code, node);
			}

			return -1;
		}
	}

	// Property names must be checked against method names
	if( isProperty )
	{
		asCArray<int> methods = t->methods;
		for( asUINT n = 0; n < methods.GetLength(); n++ )
		{
			if( engine->scriptFunctions[methods[n]]->name == name )
			{
				if( code )
				{
					asCString str;
					str.Format(TXT_NAME_CONFLICT_s_METHOD, name);
					WriteError(str, code, node);
				}

				return -1;
			}
		}
	}

	return 0;
}

int asCBuilder::CheckNameConflict(const char *name, asCScriptNode *node, asCScriptCode *code, asSNameSpace *ns)
{
	// Check against registered object types
	// TODO: Must check against registered funcdefs too
	if( engine->GetRegisteredObjectType(name, ns) != 0 )
	{
		if( code )
		{
			asCString str;
			str.Format(TXT_NAME_CONFLICT_s_EXTENDED_TYPE, name);
			WriteError(str, code, node);
		}

		return -1;
	}

	// Check against global properties
	if( DoesGlobalPropertyExist(name, ns) )
	{
		if( code )
		{
			asCString str;
			str.Format(TXT_NAME_CONFLICT_s_GLOBAL_PROPERTY, name);
			WriteError(str, code, node);
		}

		return -1;
	}

	// TODO: Property names must be checked against function names

#ifndef AS_NO_COMPILER
	// Check against class types
	asUINT n;
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		if( classDeclarations[n]->name == name &&
			classDeclarations[n]->objType->nameSpace == ns )
		{
			if( code )
			{
				asCString str;
				str.Format(TXT_NAME_CONFLICT_s_STRUCT, name);
				WriteError(str, code, node);
			}

			return -1;
		}
	}

	// Check against named types
	for( n = 0; n < namedTypeDeclarations.GetLength(); n++ )
	{
		if( namedTypeDeclarations[n]->name == name &&
			namedTypeDeclarations[n]->objType->nameSpace == ns )
		{
			if( code )
			{
				asCString str;
				str.Format(TXT_NAME_CONFLICT_s_IS_NAMED_TYPE, name);
				WriteError(str, code, node);
			}

			return -1;
		}
	}

	// Must check for name conflicts with funcdefs
	for( n = 0; n < funcDefs.GetLength(); n++ )
	{
		if( funcDefs[n]->name == name &&
			module->funcDefs[funcDefs[n]->idx]->nameSpace == ns )
		{
			if( code )
			{
				asCString str;
				str.Format(TXT_NAME_CONFLICT_s_IS_FUNCDEF, name);
				WriteError(str, code, node);
			}

			return -1;
		}
	}

	// Check against mixin classes
	if( GetMixinClass(name, ns) )
	{
		if( code )
		{
			asCString str;
			str.Format(TXT_NAME_CONFLICT_s_IS_MIXIN, name);
			WriteError(str, code, node);
		}

		return -1;
	}
#endif

	return 0;
}

#ifndef AS_NO_COMPILER
sMixinClass *asCBuilder::GetMixinClass(const char *name, asSNameSpace *ns)
{
	for( asUINT n = 0; n < mixinClasses.GetLength(); n++ )
		if( mixinClasses[n]->name == name &&
			mixinClasses[n]->ns == ns )
			return mixinClasses[n];

	return 0;
}

int asCBuilder::RegisterFuncDef(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	// TODO: redesign: Allow funcdefs to be explicitly declared as 'shared'. In this case
	//                 an error should be given if any of the arguments/return type is not
	//                 shared.

	// Find the name
	asASSERT( node->firstChild->nodeType == snDataType );
	asCScriptNode *n = node->firstChild->next->next;

	asCString name;
	name.Assign(&file->code[n->tokenPos], n->tokenLength);

	// Check for name conflict with other types
	int r = CheckNameConflict(name.AddressOf(), node, file, ns);
	if( asSUCCESS != r )
	{
		node->Destroy(engine);
		return r;
	}

	// The function definition should be stored as a asCScriptFunction so that the application
	// can use the asIScriptFunction interface to enumerate the return type and parameters

	// The return type and parameter types aren't determined in this function. A second pass is
	// necessary after all type declarations have been identified. The second pass is implemented
	// in CompleteFuncDef().

	sFuncDef *fd = asNEW(sFuncDef);
	if( fd == 0 )
	{
		node->Destroy(engine);
		return asOUT_OF_MEMORY;
	}

	fd->name   = name;
	fd->node   = node;
	fd->script = file;
	fd->idx    = module->AddFuncDef(name, ns);

	funcDefs.PushLast(fd);

	return 0;
}

void asCBuilder::CompleteFuncDef(sFuncDef *funcDef)
{
	asCArray<asCString *>      defaultArgs;
	bool                       isConstMethod;
	bool                       isConstructor;
	bool                       isDestructor;
	bool                       isProtected;
	bool                       isPrivate;
	bool                       isOverride;
	bool                       isFinal;
	bool                       isShared;

	asCScriptFunction *func = module->funcDefs[funcDef->idx];
	asASSERT( func );

	// TODO: It should be possible to declare funcdef as shared. In this case a compiler error will be given if any of the types it uses are not shared
	GetParsedFunctionDetails(funcDef->node, funcDef->script, 0, funcDef->name, func->returnType, func->parameterNames, func->parameterTypes, func->inOutFlags, defaultArgs, isConstMethod, isConstructor, isDestructor, isPrivate, isProtected, isOverride, isFinal, isShared, func->nameSpace);

	// There should not be any defaultArgs, but if there are any we need to delete them to avoid leaks
	for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
		if( defaultArgs[n] )
			asDELETE(defaultArgs[n], asCString);

	// All funcdefs are shared, unless one of the parameter types or return type is not shared
	isShared = true;
	if( func->returnType.GetObjectType() && !func->returnType.GetObjectType()->IsShared() )
		isShared = false;
	if( func->returnType.GetFuncDef() && !func->returnType.GetFuncDef()->IsShared() )
		isShared = false;
	for( asUINT n = 0; isShared && n < func->parameterTypes.GetLength(); n++ )
	{
		if( func->parameterTypes[n].GetObjectType() && !func->parameterTypes[n].GetObjectType()->IsShared() )
			isShared = false;
		if( func->parameterTypes[n].GetFuncDef() && !func->parameterTypes[n].GetFuncDef()->IsShared() )
			isShared = false;
	}
	func->isShared = isShared;

	// Check if there is another identical funcdef from another module and if so reuse that instead
	if( func->isShared )
	{
		for( asUINT n = 0; n < engine->funcDefs.GetLength(); n++ )
		{
			asCScriptFunction *f2 = engine->funcDefs[n];
			if( f2 == 0 || func == f2 )
				continue;

			if( !f2->isShared )
				continue;

			if( f2->name == func->name &&
				f2->nameSpace == func->nameSpace &&
				f2->IsSignatureExceptNameEqual(func) )
			{
				// Replace our funcdef for the existing one
				funcDef->idx = f2->id;
				module->funcDefs[module->funcDefs.IndexOf(func)] = f2;
				f2->AddRefInternal();

				engine->funcDefs.RemoveValue(func);

				func->ReleaseInternal();
				break;
			}
		}
	}
}

int asCBuilder::RegisterGlobalVar(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	// Has the application disabled global vars?
	if( engine->ep.disallowGlobalVars )
		WriteError(TXT_GLOBAL_VARS_NOT_ALLOWED, file, node);

	// What data type is it?
	asCDataType type = CreateDataTypeFromNode(node->firstChild, file, ns);

	if( !type.CanBeInstantiated() )
	{
		asCString str;
		if( type.IsAbstractClass() )
			str.Format(TXT_ABSTRACT_CLASS_s_CANNOT_BE_INSTANTIATED, type.Format(ns).AddressOf());
		else if( type.IsInterface() )
			str.Format(TXT_INTERFACE_s_CANNOT_BE_INSTANTIATED, type.Format(ns).AddressOf());
		else
			// TODO: Improve error message to explain why
			str.Format(TXT_DATA_TYPE_CANT_BE_s, type.Format(ns).AddressOf());

		WriteError(str, file, node);
	}

	asCScriptNode *n = node->firstChild->next;

	while( n )
	{
		// Verify that the name isn't taken
		asCString name(&file->code[n->tokenPos], n->tokenLength);
		CheckNameConflict(name.AddressOf(), n, file, ns);

		// Register the global variable
		sGlobalVariableDescription *gvar = asNEW(sGlobalVariableDescription);
		if( gvar == 0 )
		{
			node->Destroy(engine);
			return asOUT_OF_MEMORY;
		}

		gvar->script      = file;
		gvar->name        = name;
		gvar->isCompiled  = false;
		gvar->datatype    = type;
		gvar->isEnumValue = false;
		gvar->ns          = ns;

		// TODO: Give error message if wrong
		asASSERT(!gvar->datatype.IsReference());

		// Allocation is done when the variable is compiled, to allow for autos
		gvar->property = 0;
		gvar->index    = 0;

		globVariables.Put(gvar);


		gvar->declaredAtNode = n;
		n = n->next;
		gvar->declaredAtNode->DisconnectParent();
		gvar->initializationNode = 0;
		if( n &&
			( n->nodeType == snAssignment ||
			  n->nodeType == snArgList    ||
			  n->nodeType == snInitList     ) )
		{
			gvar->initializationNode = n;
			n = n->next;
			gvar->initializationNode->DisconnectParent();
		}
	}

	node->Destroy(engine);

	return 0;
}

int asCBuilder::RegisterMixinClass(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	asCScriptNode *cl = node->firstChild;
	asASSERT( cl->nodeType == snClass );

	asCScriptNode *n = cl->firstChild;

	// Skip potential 'final' and 'shared' tokens
	while( n->tokenType == ttIdentifier &&
		   (file->TokenEquals(n->tokenPos, n->tokenLength, FINAL_TOKEN) ||
		    file->TokenEquals(n->tokenPos, n->tokenLength, SHARED_TOKEN)) )
	{
		// Report error, because mixin class cannot be final or shared
		asCString msg;
		msg.Format(TXT_MIXIN_CANNOT_BE_DECLARED_AS_s, asCString(&file->code[n->tokenPos], n->tokenLength).AddressOf());
		WriteError(msg, file, n);

		asCScriptNode *tmp = n;
		n = n->next;

		// Remove the invalid node, so compilation can continue as if it wasn't there
		tmp->DisconnectParent();
		tmp->Destroy(engine);
	}

	asCString name(&file->code[n->tokenPos], n->tokenLength);

	int r, c;
	file->ConvertPosToRowCol(n->tokenPos, &r, &c);

	CheckNameConflict(name.AddressOf(), n, file, ns);

	sMixinClass *decl = asNEW(sMixinClass);
	if( decl == 0 )
	{
		node->Destroy(engine);
		return asOUT_OF_MEMORY;
	}

	mixinClasses.PushLast(decl);
	decl->name   = name;
	decl->ns     = ns;
	decl->node   = cl;
	decl->script = file;

	// Clean up memory
	cl->DisconnectParent();
	node->Destroy(engine);

	return 0;
}

int asCBuilder::RegisterClass(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	asCScriptNode *n = node->firstChild;
	bool isFinal = false;
	bool isShared = false;
	bool isAbstract = false;

	// Check the class modifiers
	while( n->tokenType == ttIdentifier )
	{
		if( file->TokenEquals(n->tokenPos, n->tokenLength, FINAL_TOKEN) )
		{
			if( isAbstract )
				WriteError(TXT_CLASS_CANT_BE_FINAL_AND_ABSTRACT, file, n);
			else
			{
				if( isFinal )
				{
					asCString msg;
					msg.Format(TXT_ATTR_s_INFORMED_MULTIPLE_TIMES, asCString(&file->code[n->tokenPos], n->tokenLength).AddressOf());
					WriteWarning(msg, file, n);
				}
				isFinal = true;
			}
		}
		else if( file->TokenEquals(n->tokenPos, n->tokenLength, SHARED_TOKEN) )
		{
			if( isShared )
			{
				asCString msg;
				msg.Format(TXT_ATTR_s_INFORMED_MULTIPLE_TIMES, asCString(&file->code[n->tokenPos], n->tokenLength).AddressOf());
				WriteWarning(msg, file, n);
			}
			isShared = true;
		}
		else if( file->TokenEquals(n->tokenPos, n->tokenLength, ABSTRACT_TOKEN) )
		{
			if( isFinal )
				WriteError(TXT_CLASS_CANT_BE_FINAL_AND_ABSTRACT, file, n);
			else
			{
				if( isAbstract )
				{
					asCString msg;
					msg.Format(TXT_ATTR_s_INFORMED_MULTIPLE_TIMES, asCString(&file->code[n->tokenPos], n->tokenLength).AddressOf());
					WriteWarning(msg, file, n);
				}
				isAbstract = true;
			}
		}
		else
		{
			// This is the name of the class
			break;
		}

		n = n->next;
	}

	asCString name(&file->code[n->tokenPos], n->tokenLength);

	int r, c;
	file->ConvertPosToRowCol(n->tokenPos, &r, &c);

	CheckNameConflict(name.AddressOf(), n, file, ns);

	sClassDeclaration *decl = asNEW(sClassDeclaration);
	if( decl == 0 )
	{
		node->Destroy(engine);
		return asOUT_OF_MEMORY;
	}

	classDeclarations.PushLast(decl);
	decl->name             = name;
	decl->script           = file;
	decl->node             = node;

	// If this type is shared and there already exist another shared
	// type of the same name, then that one should be used instead of
	// creating a new one.
	if( isShared )
	{
		for( asUINT n = 0; n < engine->sharedScriptTypes.GetLength(); n++ )
		{
			asCObjectType *st = engine->sharedScriptTypes[n];
			if( st &&
				st->IsShared() &&
				st->name == name &&
				st->nameSpace == ns &&
				!st->IsInterface() )
			{
				// We'll use the existing type
				decl->isExistingShared = true;
				decl->objType          = st;
				module->classTypes.PushLast(st);
				st->AddRefInternal();
				return 0;
			}
		}
	}

	// Create a new object type for this class
	asCObjectType *st = asNEW(asCObjectType)(engine);
	if( st == 0 )
		return asOUT_OF_MEMORY;

	// By default all script classes are marked as garbage collected.
	// Only after the complete structure and relationship between classes
	// is known, can the flag be cleared for those objects that truly cannot
	// form circular references. This is important because a template
	// callback may be called with a script class before the compilation
	// completes, and until it is known, the callback must assume the class
	// is garbage collected.
	st->flags = asOBJ_REF | asOBJ_SCRIPT_OBJECT | asOBJ_GC;

	if( isShared )
		st->flags |= asOBJ_SHARED;

	if( isFinal )
		st->flags |= asOBJ_NOINHERIT;

	if( isAbstract )
		st->flags |= asOBJ_ABSTRACT;

	if( node->tokenType == ttHandle )
		st->flags |= asOBJ_IMPLICIT_HANDLE;

	st->size      = sizeof(asCScriptObject);
	st->name      = name;
	st->nameSpace = ns;
	st->module    = module;
	module->classTypes.PushLast(st);
	if( isShared )
	{
		engine->sharedScriptTypes.PushLast(st);
		st->AddRefInternal();
	}
	decl->objType = st;

	// Use the default script class behaviours
	st->beh = engine->scriptTypeBehaviours.beh;

	// TODO: Move this to asCObjectType so that the asCRestore can reuse it
	engine->scriptFunctions[st->beh.addref]->AddRefInternal();
	engine->scriptFunctions[st->beh.release]->AddRefInternal();
	engine->scriptFunctions[st->beh.gcEnumReferences]->AddRefInternal();
	engine->scriptFunctions[st->beh.gcGetFlag]->AddRefInternal();
	engine->scriptFunctions[st->beh.gcGetRefCount]->AddRefInternal();
	engine->scriptFunctions[st->beh.gcReleaseAllReferences]->AddRefInternal();
	engine->scriptFunctions[st->beh.gcSetFlag]->AddRefInternal();
	engine->scriptFunctions[st->beh.copy]->AddRefInternal();
	engine->scriptFunctions[st->beh.factory]->AddRefInternal();
	engine->scriptFunctions[st->beh.construct]->AddRefInternal();
	// TODO: weak: Should not do this if the class has been declared with noweak
	engine->scriptFunctions[st->beh.getWeakRefFlag]->AddRefInternal();

	return 0;
}

int asCBuilder::RegisterInterface(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	asCScriptNode *n = node->firstChild;
	asCString name(&file->code[n->tokenPos], n->tokenLength);

	bool isShared = false;
	if( name == SHARED_TOKEN )
	{
		isShared = true;

		n = n->next;
		name.Assign(&file->code[n->tokenPos], n->tokenLength);
	}

	int r, c;
	file->ConvertPosToRowCol(n->tokenPos, &r, &c);

	CheckNameConflict(name.AddressOf(), n, file, ns);

	sClassDeclaration *decl = asNEW(sClassDeclaration);
	if( decl == 0 )
	{
		node->Destroy(engine);
		return asOUT_OF_MEMORY;
	}

	interfaceDeclarations.PushLast(decl);
	decl->name             = name;
	decl->script           = file;
	decl->node             = node;

	// If this type is shared and there already exist another shared
	// type of the same name, then that one should be used instead of
	// creating a new one.
	if( isShared )
	{
		for( asUINT n = 0; n < engine->sharedScriptTypes.GetLength(); n++ )
		{
			asCObjectType *st = engine->sharedScriptTypes[n];
			if( st &&
				st->IsShared() &&
				st->name == name &&
				st->nameSpace == ns &&
				st->IsInterface() )
			{
				// We'll use the existing type
				decl->isExistingShared = true;
				decl->objType          = st;
				module->classTypes.PushLast(st);
				st->AddRefInternal();
				return 0;
			}
		}
	}

	// Register the object type for the interface
	asCObjectType *st = asNEW(asCObjectType)(engine);
	if( st == 0 )
		return asOUT_OF_MEMORY;

	st->flags = asOBJ_REF | asOBJ_SCRIPT_OBJECT;

	if( isShared )
		st->flags |= asOBJ_SHARED;

	st->size = 0; // Cannot be instantiated
	st->name = name;
	st->nameSpace = ns;
	st->module = module;
	module->classTypes.PushLast(st);
	if( isShared )
	{
		engine->sharedScriptTypes.PushLast(st);
		st->AddRefInternal();
	}
	decl->objType = st;

	// Use the default script class behaviours
	st->beh.construct = 0;
	st->beh.addref = engine->scriptTypeBehaviours.beh.addref;
	engine->scriptFunctions[st->beh.addref]->AddRefInternal();
	st->beh.release = engine->scriptTypeBehaviours.beh.release;
	engine->scriptFunctions[st->beh.release]->AddRefInternal();
	st->beh.copy = 0;

	return 0;
}

void asCBuilder::CompileGlobalVariables()
{
	bool compileSucceeded = true;

	// Store state of compilation (errors, warning, output)
	int currNumErrors   = numErrors;
	int currNumWarnings = numWarnings;

	// Backup the original message stream
	bool                       msgCallback     = engine->msgCallback;
	asSSystemFunctionInterface msgCallbackFunc = engine->msgCallbackFunc;
	void                      *msgCallbackObj  = engine->msgCallbackObj;

	// Set the new temporary message stream
	asCOutputBuffer outBuffer;
	engine->SetMessageCallback(asMETHOD(asCOutputBuffer, Callback), &outBuffer, asCALL_THISCALL);

	asCOutputBuffer finalOutput;
	asCScriptFunction *initFunc = 0;

	asCSymbolTable<asCGlobalProperty> initOrder;

	// We first try to compile all the primitive global variables, and only after that
	// compile the non-primitive global variables. This permits the constructors
	// for the complex types to use the already initialized variables of primitive
	// type. Note, we currently don't know which global variables are used in the
	// constructors, so we cannot guarantee that variables of complex types are
	// initialized in the correct order, so we won't reorder those.
	bool compilingPrimitives = true;

	// Compile each global variable
	while( compileSucceeded )
	{
		compileSucceeded = false;

		int accumErrors = 0;
		int accumWarnings = 0;

		// Restore state of compilation
		finalOutput.Clear();
		asCSymbolTable<sGlobalVariableDescription>::iterator it = globVariables.List();
		for( ; it; it++ )
		{
			sGlobalVariableDescription *gvar = *it;
			if( gvar->isCompiled )
				continue;

			asCByteCode init(engine);
			numWarnings = 0;
			numErrors = 0;
			outBuffer.Clear();

			// Skip this for now if we're not compiling complex types yet
			if( compilingPrimitives && !gvar->datatype.IsPrimitive() )
				continue;

			if( gvar->declaredAtNode )
			{
				int r, c;
				gvar->script->ConvertPosToRowCol(gvar->declaredAtNode->tokenPos, &r, &c);
				asCString str = gvar->datatype.Format(gvar->ns);
				str += " " + gvar->name;
				str.Format(TXT_COMPILING_s, str.AddressOf());
				WriteInfo(gvar->script->name, str, r, c, true);
			}

			if( gvar->isEnumValue )
			{
				int r;
				if( gvar->initializationNode )
				{
					asCCompiler comp(engine);
					asCScriptFunction func(engine, module, asFUNC_SCRIPT);

					// Set the namespace that should be used during the compilation
					func.nameSpace = gvar->datatype.GetObjectType()->nameSpace;

					// Temporarily switch the type of the variable to int so it can be compiled properly
					asCDataType saveType;
					saveType = gvar->datatype;
					gvar->datatype = asCDataType::CreatePrimitive(ttInt, true);
					r = comp.CompileGlobalVariable(this, gvar->script, gvar->initializationNode, gvar, &func);
					gvar->datatype = saveType;

					// Make the function a dummy so it doesn't try to release objects while destroying the function
					func.funcType = asFUNC_DUMMY;
				}
				else
				{
					r = 0;

					// When there is no assignment the value is the last + 1
					int enumVal = 0;
					asCSymbolTable<sGlobalVariableDescription>::iterator prev_it = it;
					prev_it--;
					if( prev_it )
					{
						sGlobalVariableDescription *gvar2 = *prev_it;
						if(gvar2->datatype == gvar->datatype )
						{
							// The integer value is stored in the lower bytes
							enumVal = (*(int*)&gvar2->constantValue) + 1;

							if( !gvar2->isCompiled )
							{
								int row, col;
								gvar->script->ConvertPosToRowCol(gvar->declaredAtNode->tokenPos, &row, &col);

								asCString str = gvar->datatype.Format(gvar->ns);
								str += " " + gvar->name;
								str.Format(TXT_COMPILING_s, str.AddressOf());
								WriteInfo(gvar->script->name, str, row, col, true);

								str.Format(TXT_UNINITIALIZED_GLOBAL_VAR_s, gvar2->name.AddressOf());
								WriteError(gvar->script->name, str, row, col);
								r = -1;
							}
						}
					}

					// The integer value is stored in the lower bytes
					*(int*)&gvar->constantValue = enumVal;
				}

				if( r >= 0 )
				{
					// Set the value as compiled
					gvar->isCompiled = true;
					compileSucceeded = true;
				}
			}
			else
			{
				// Compile the global variable
				initFunc = asNEW(asCScriptFunction)(engine, module, asFUNC_SCRIPT);
				if( initFunc == 0 )
				{
					// Out of memory
					return;
				}

				// Set the namespace that should be used for this function
				initFunc->nameSpace = gvar->ns;

				asCCompiler comp(engine);
				int r = comp.CompileGlobalVariable(this, gvar->script, gvar->initializationNode, gvar, initFunc);
				if( r >= 0 )
				{
					// Compilation succeeded
					gvar->isCompiled = true;
					compileSucceeded = true;
				}
				else
				{
					// Compilation failed
					initFunc->funcType = asFUNC_DUMMY;
					asDELETE(initFunc, asCScriptFunction);
					initFunc = 0;
				}
			}

			if( gvar->isCompiled )
			{
				// Add warnings for this constant to the total build
				if( numWarnings )
				{
					currNumWarnings += numWarnings;
					if( msgCallback )
						outBuffer.SendToCallback(engine, &msgCallbackFunc, msgCallbackObj);
				}

				// Determine order of variable initializations
				if( gvar->property && !gvar->isEnumValue )
					initOrder.Put(gvar->property);

				// Does the function contain more than just a SUSPEND followed by a RET instruction?
				if( initFunc && initFunc->scriptData->byteCode.GetLength() > 2 )
				{
					// Create the init function for this variable
					initFunc->id = engine->GetNextScriptFunctionId();
					engine->AddScriptFunction(initFunc);

					// Finalize the init function for this variable
					initFunc->returnType = asCDataType::CreatePrimitive(ttVoid, false);
					initFunc->scriptData->scriptSectionIdx = engine->GetScriptSectionNameIndex(gvar->script->name.AddressOf());
					if( gvar->declaredAtNode )
					{
						int row, col;
						gvar->script->ConvertPosToRowCol(gvar->declaredAtNode->tokenPos, &row, &col);
						initFunc->scriptData->declaredAt = (row & 0xFFFFF)|((col & 0xFFF)<<20);
					}

					gvar->property->SetInitFunc(initFunc);

					initFunc->ReleaseInternal();
					initFunc = 0;
				}
				else if( initFunc )
				{
					// Destroy the function as it won't be used
					initFunc->funcType = asFUNC_DUMMY;
					asDELETE(initFunc, asCScriptFunction);
					initFunc = 0;
				}

				// Convert enums to true enum values, so subsequent compilations can access it as an enum
				if( gvar->isEnumValue )
				{
					asCObjectType *objectType = gvar->datatype.GetObjectType();
					asASSERT(NULL != objectType);

					asSEnumValue *e = asNEW(asSEnumValue);
					if( e == 0 )
					{
						// Out of memory
						numErrors++;
						return;
					}

					e->name = gvar->name;
					e->value = *(int*)&gvar->constantValue;

					objectType->enumValues.PushLast(e);
				}
			}
			else
			{
				// Add output to final output
				finalOutput.Append(outBuffer);
				accumErrors += numErrors;
				accumWarnings += numWarnings;
			}

			engine->preMessage.isSet = false;
		}

		if( !compileSucceeded )
		{
			if( compilingPrimitives )
			{
				// No more primitives could be compiled, so
				// switch to compiling the complex variables
				compilingPrimitives = false;
				compileSucceeded    = true;
			}
			else
			{
				// No more variables can be compiled
				// Add errors and warnings to total build
				currNumWarnings += accumWarnings;
				currNumErrors   += accumErrors;
				if( msgCallback )
					finalOutput.SendToCallback(engine, &msgCallbackFunc, msgCallbackObj);
			}
		}
	}

	// Restore states
	engine->msgCallback     = msgCallback;
	engine->msgCallbackFunc = msgCallbackFunc;
	engine->msgCallbackObj  = msgCallbackObj;

	numWarnings = currNumWarnings;
	numErrors   = currNumErrors;

	// Set the correct order of initialization
	if( numErrors == 0 )
	{
		// If the length of the arrays are not the same, then this is the compilation
		// of a single variable, in which case the initialization order of the previous
		// variables must be preserved.
		if( module->scriptGlobals.GetSize() == initOrder.GetSize() )
			module->scriptGlobals.SwapWith(initOrder);
	}

	// Delete the enum expressions
	asCSymbolTableIterator<sGlobalVariableDescription> it = globVariables.List();
	while( it )
	{
		sGlobalVariableDescription *gvar = *it;
		if( gvar->isEnumValue )
		{
			// Remove from symboltable. This has to be done prior to freeing the memeory
			globVariables.Erase(it.GetIndex());

			// Destroy the gvar property
			if( gvar->declaredAtNode )
			{
				gvar->declaredAtNode->Destroy(engine);
				gvar->declaredAtNode = 0;
			}
			if( gvar->initializationNode )
			{
				gvar->initializationNode->Destroy(engine);
				gvar->initializationNode = 0;
			}
			if( gvar->property )
			{
				asDELETE(gvar->property, asCGlobalProperty);
				gvar->property = 0;
			}

			asDELETE(gvar, sGlobalVariableDescription);
		}
		else
			it++;
	}
}

int asCBuilder::GetNamespaceAndNameFromNode(asCScriptNode *n, asCScriptCode *script, asSNameSpace *implicitNs, asSNameSpace *&outNs, asCString &outName)
{
	asASSERT( n->nodeType == snIdentifier );

	// Get the optional scope from the node
	asSNameSpace *ns = GetNameSpaceFromNode(n->firstChild, script, implicitNs, 0);
	if( ns == 0 )
		return -1;

	// Get the name
	asCString name(&script->code[n->lastChild->tokenPos], n->lastChild->tokenLength);

	outNs = ns;
	outName = name;

	return 0;
}

void asCBuilder::AddInterfaceFromMixinToClass(sClassDeclaration *decl, asCScriptNode *errNode, sMixinClass *mixin)
{
	// Determine what interfaces that the mixin implements
	asCScriptNode *node = mixin->node;
	asASSERT(node->nodeType == snClass);

	// Skip the name of the mixin
	node = node->firstChild->next;


	while( node && node->nodeType == snIdentifier )
	{
		bool ok = true;
		asSNameSpace *ns;
		asCString name;
		if( GetNamespaceAndNameFromNode(node, mixin->script, mixin->ns, ns, name) < 0 )
			ok = false;
		else
		{
			// Find the object type for the interface
			asCObjectType *objType = GetObjectType(name.AddressOf(), ns);

			// Check that the object type is an interface
			if( objType && objType->IsInterface() )
			{
				// Only add the interface if the class doesn't already implement it
				if( !decl->objType->Implements(objType) )
					AddInterfaceToClass(decl, errNode, objType);
			}
			else
			{
				WriteError(TXT_MIXIN_CLASS_CANNOT_INHERIT, mixin->script, node);
				ok = false;
			}
		}

		if( !ok )
		{
			// Remove this node so the error isn't reported again
			asCScriptNode *delNode = node;
			node = node->prev;
			delNode->DisconnectParent();
			delNode->Destroy(engine);
		}

		node = node->next;
	}
}

void asCBuilder::AddInterfaceToClass(sClassDeclaration *decl, asCScriptNode *errNode, asCObjectType *intfType)
{
	// A shared type may only implement from shared interfaces
	if( decl->objType->IsShared() && !intfType->IsShared() )
	{
		asCString msg;
		msg.Format(TXT_SHARED_CANNOT_IMPLEMENT_NON_SHARED_s, intfType->name.AddressOf());
		WriteError(msg, decl->script, errNode);
		return;
	}

	if( decl->isExistingShared )
	{
		// If the class is an existing shared class, then just check if the
		// interface exists in the original declaration too
		if( !decl->objType->Implements(intfType) )
		{
			asCString str;
			str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, decl->objType->GetName());
			WriteError(str, decl->script, errNode);
			return;
		}
	}
	else
	{
		// If the interface is already in the class then don't add it again
		if( decl->objType->Implements(intfType) )
			return;

		// Add the interface to the class
		decl->objType->interfaces.PushLast(intfType);

		// Add the inherited interfaces too
		// For interfaces this will be done outside to handle out-of-order declarations
		if( !decl->objType->IsInterface() )
		{
			for( asUINT n = 0; n < intfType->interfaces.GetLength(); n++ )
				AddInterfaceToClass(decl, errNode, intfType->interfaces[n]);
		}
	}
}

void asCBuilder::CompileInterfaces()
{
	asUINT n;
	for( n = 0; n < interfaceDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *intfDecl = interfaceDeclarations[n];
		asCObjectType *intfType = intfDecl->objType;

		asCScriptNode *node = intfDecl->node;
		asASSERT(node && node->nodeType == snInterface);
		node = node->firstChild;

		// Skip the 'shared' keyword
		if( intfType->IsShared() )
			node = node->next;

		// Skip the name
		node = node->next;

		// Verify the inherited interfaces
		while( node && node->nodeType == snIdentifier )
		{
			asSNameSpace *ns;
			asCString name;
			if( GetNamespaceAndNameFromNode(node, intfDecl->script, intfType->nameSpace, ns, name) < 0 )
			{
				node = node->next;
				continue;
			}

			// Find the object type for the interface
			asCObjectType *objType = 0;
			while( ns )
			{
				objType = GetObjectType(name.AddressOf(), ns);
				if( objType ) break;

				ns = engine->GetParentNameSpace(ns);
			}

			// Check that the object type is an interface
			bool ok = true;
			if( objType && objType->IsInterface() )
			{
				// Check that the implemented interface is shared if the base interface is shared
				if( intfType->IsShared() && !objType->IsShared() )
				{
					asCString str;
					str.Format(TXT_SHARED_CANNOT_IMPLEMENT_NON_SHARED_s, objType->GetName());
					WriteError(str, intfDecl->script, node);
					ok = false;
				}
			}
			else
			{
				WriteError(TXT_INTERFACE_CAN_ONLY_IMPLEMENT_INTERFACE, intfDecl->script, node);
				ok = false;
			}

			if( ok )
			{
				// Make sure none of the implemented interfaces implement from this one
				asCObjectType *base = objType;
				while( base != 0 )
				{
					if( base == intfType )
					{
						WriteError(TXT_CANNOT_IMPLEMENT_SELF, intfDecl->script, node);
						ok = false;
						break;
					}

					// At this point there is at most one implemented interface
					if( base->interfaces.GetLength() )
						base = base->interfaces[0];
					else
						break;
				}
			}

			if( ok )
				AddInterfaceToClass(intfDecl, node, objType);

			// Remove the nodes so they aren't parsed again
			asCScriptNode *delNode = node;
			node = node->next;
			delNode->DisconnectParent();
			delNode->Destroy(engine);
		}
	}

	// Order the interfaces with inheritances so that the inherited
	// of inherited interfaces can be added properly
	for( n = 0; n < interfaceDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *intfDecl = interfaceDeclarations[n];
		asCObjectType *intfType = intfDecl->objType;

		if( intfType->interfaces.GetLength() == 0 ) continue;

		// If any of the derived interfaces are found after this interface, then move this to the end of the list
		for( asUINT m = n+1; m < interfaceDeclarations.GetLength(); m++ )
		{
			if( intfType->Implements(interfaceDeclarations[m]->objType) )
			{
				interfaceDeclarations.RemoveIndex(n);
				interfaceDeclarations.PushLast(intfDecl);

				// Decrease index so that we don't skip an entry
				n--;
				break;
			}
		}
	}

	// Now recursively add the additional inherited interfaces
	for( n = 0; n < interfaceDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *intfDecl = interfaceDeclarations[n];
		asCObjectType *intfType = intfDecl->objType;

		// TODO: Is this really at the correct place? Hasn't the vfTableIdx already been set here?
		// Co-opt the vfTableIdx value in our own methods to indicate the
		// index the function should have in the table chunk for this interface.
		for( asUINT d = 0; d < intfType->methods.GetLength(); d++ )
		{
			asCScriptFunction *func = GetFunctionDescription(intfType->methods[d]);
			func->vfTableIdx = d;

			asASSERT(func->objectType == intfType);
		}

		// As new interfaces will be added to the end of the list, all
		// interfaces will be traversed the same as recursively
		for( asUINT m = 0; m < intfType->interfaces.GetLength(); m++ )
		{
			asCObjectType *base = intfType->interfaces[m];

			// Add any interfaces not already implemented
			for( asUINT l = 0; l < base->interfaces.GetLength(); l++ )
				AddInterfaceToClass(intfDecl, intfDecl->node, base->interfaces[l]);

			// Add the methods from the implemented interface
			for( asUINT m = 0; m < base->methods.GetLength(); m++ )
			{
				// If the derived interface implements the same method, then don't add the base interface' method
				asCScriptFunction *baseFunc = GetFunctionDescription(base->methods[m]);
				asCScriptFunction *derivedFunc = 0;
				bool found = false;
				for( asUINT d = 0; d < intfType->methods.GetLength(); d++ )
				{
					derivedFunc = GetFunctionDescription(intfType->methods[d]);
					if( derivedFunc->IsSignatureEqual(baseFunc) )
					{
						found = true;
						break;
					}
				}

				if( !found )
				{
					// Add the method
					intfType->methods.PushLast(baseFunc->id);
					baseFunc->AddRefInternal();
				}
			}
		}
	}
}

// numTempl is the number of template instances that existed in the engine before the build begun
void asCBuilder::CompileClasses(asUINT numTempl)
{
	asUINT n;
	asCArray<sClassDeclaration*> toValidate((int)classDeclarations.GetLength());

	// Determine class inheritances and interfaces
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *decl = classDeclarations[n];
		asCScriptCode *file = decl->script;

		// Find the base class that this class inherits from
		bool multipleInheritance = false;
		asCScriptNode *node = decl->node->firstChild;

		while( file->TokenEquals(node->tokenPos, node->tokenLength, FINAL_TOKEN) ||
			   file->TokenEquals(node->tokenPos, node->tokenLength, SHARED_TOKEN) ||
			   file->TokenEquals(node->tokenPos, node->tokenLength, ABSTRACT_TOKEN) )
		{
			node = node->next;
		}

		// Skip the name of the class
		asASSERT(node->tokenType == ttIdentifier);
		node = node->next;

		while( node && node->nodeType == snIdentifier )
		{
			asSNameSpace *ns;
			asCString name;
			if( GetNamespaceAndNameFromNode(node, file, decl->objType->nameSpace, ns, name) < 0 )
			{
				node = node->next;
				continue;
			}

			// Find the object type for the interface
			asCObjectType *objType = 0;
			sMixinClass *mixin = 0;
			asSNameSpace *origNs = ns;
			while( ns )
			{
				objType = GetObjectType(name.AddressOf(), ns);
				if( objType == 0 )
					mixin = GetMixinClass(name.AddressOf(), ns);

				if( objType || mixin )
					break;

				ns = engine->GetParentNameSpace(ns);
			}

			if( objType == 0 && mixin == 0 )
			{
				asCString str;
				if( origNs->name == "" )
					str.Format(TXT_IDENTIFIER_s_NOT_DATA_TYPE_IN_GLOBAL_NS, name.AddressOf());
				else
					str.Format(TXT_IDENTIFIER_s_NOT_DATA_TYPE_IN_NS_s, name.AddressOf(), origNs->name.AddressOf());
				WriteError(str, file, node);
			}
			else if( mixin )
			{
				AddInterfaceFromMixinToClass(decl, node, mixin);
			}
			else if( !(objType->flags & asOBJ_SCRIPT_OBJECT) ||
					 (objType->flags & asOBJ_NOINHERIT) )
			{
				// Either the class is not a script class or interface
				// or the class has been declared as 'final'
				asCString str;
				str.Format(TXT_CANNOT_INHERIT_FROM_s_FINAL, objType->name.AddressOf());
				WriteError(str, file, node);
			}
			else if( objType->size != 0 )
			{
				// The class inherits from another script class
				if( !decl->isExistingShared && decl->objType->derivedFrom != 0 )
				{
					if( !multipleInheritance )
					{
						WriteError(TXT_CANNOT_INHERIT_FROM_MULTIPLE_CLASSES, file, node);
						multipleInheritance = true;
					}
				}
				else
				{
					// Make sure none of the base classes inherit from this one
					asCObjectType *base = objType;
					bool error = false;
					while( base != 0 )
					{
						if( base == decl->objType )
						{
							WriteError(TXT_CANNOT_INHERIT_FROM_SELF, file, node);
							error = true;
							break;
						}

						base = base->derivedFrom;
					}

					if( !error )
					{
						// A shared type may only inherit from other shared types
						if( (decl->objType->IsShared()) && !(objType->IsShared()) )
						{
							asCString msg;
							msg.Format(TXT_SHARED_CANNOT_INHERIT_FROM_NON_SHARED_s, objType->name.AddressOf());
							WriteError(msg, file, node);
							error = true;
						}
					}

					if( !error )
					{
						if( decl->isExistingShared )
						{
							// Verify that the base class is the same as the original shared type
							if( decl->objType->derivedFrom != objType )
							{
								asCString str;
								str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, decl->objType->GetName());
								WriteError(str, file, node);
							}
						}
						else
						{
							// Set the base class
							decl->objType->derivedFrom = objType;
							objType->AddRefInternal();
						}
					}
				}
			}
			else
			{
				// The class implements an interface
				AddInterfaceToClass(decl, node, objType);
			}

			node = node->next;
		}
	}

	// Order class declarations so that base classes are compiled before derived classes.
	// This will allow the derived classes to copy properties and methods in the next step.
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *decl = classDeclarations[n];
		asCObjectType *derived = decl->objType;
		asCObjectType *base = derived->derivedFrom;

		if( base == 0 ) continue;

		// If the base class is found after the derived class, then move the derived class to the end of the list
		for( asUINT m = n+1; m < classDeclarations.GetLength(); m++ )
		{
			sClassDeclaration *declBase = classDeclarations[m];
			if( base == declBase->objType )
			{
				classDeclarations.RemoveIndex(n);
				classDeclarations.PushLast(decl);

				// Decrease index so that we don't skip an entry
				n--;
				break;
			}
		}
	}

	// Go through each of the classes and register the object type descriptions
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *decl = classDeclarations[n];
		if( decl->isExistingShared )
		{
			// Set the declaration as validated already, so that other
			// types that contain this will accept this type
			decl->validState = 1;

			// We'll still validate the declaration to make sure nothing new is
			// added to the shared class that wasn't there in the previous
			// compilation. We do not care if something that is there in the previous
			// declaration is not included in the new declaration though.

			asASSERT( decl->objType->interfaces.GetLength() == decl->objType->interfaceVFTOffsets.GetLength() );
		}

		// Methods included from mixin classes should take precedence over inherited methods
		IncludeMethodsFromMixins(decl);

		// Add all properties and methods from the base class
		if( !decl->isExistingShared && decl->objType->derivedFrom )
		{
			asCObjectType *baseType = decl->objType->derivedFrom;

			// The derived class inherits all interfaces from the base class
			for( unsigned int n = 0; n < baseType->interfaces.GetLength(); n++ )
			{
				if( !decl->objType->Implements(baseType->interfaces[n]) )
					decl->objType->interfaces.PushLast(baseType->interfaces[n]);
			}

			// TODO: Need to check for name conflict with new class methods

			// Copy properties from base class to derived class
			for( asUINT p = 0; p < baseType->properties.GetLength(); p++ )
			{
				asCObjectProperty *prop = AddPropertyToClass(decl, baseType->properties[p]->name, baseType->properties[p]->type, baseType->properties[p]->isPrivate, baseType->properties[p]->isProtected, true);

				// The properties must maintain the same offset
				asASSERT(prop && prop->byteOffset == baseType->properties[p]->byteOffset); UNUSED_VAR(prop);
			}

			// Copy methods from base class to derived class
			for( asUINT m = 0; m < baseType->methods.GetLength(); m++ )
			{
				// If the derived class implements the same method, then don't add the base class' method
				asCScriptFunction *baseFunc = GetFunctionDescription(baseType->methods[m]);
				asCScriptFunction *derivedFunc = 0;
				bool found = false;
				for( asUINT d = 0; d < decl->objType->methods.GetLength(); d++ )
				{
					derivedFunc = GetFunctionDescription(decl->objType->methods[d]);
					if( baseFunc->name == "opConv" || baseFunc->name == "opImplConv" ||
						baseFunc->name == "opCast" || baseFunc->name == "opImplCast" )
					{
						// For the opConv and opCast methods, the return type can differ if they are different methods
						if( derivedFunc->name == baseFunc->name &&
							derivedFunc->IsSignatureExceptNameEqual(baseFunc) )
						{
							if( baseFunc->IsFinal() )
							{
								asCString msg;
								msg.Format(TXT_METHOD_CANNOT_OVERRIDE_s, baseFunc->GetDeclaration());
								WriteError(msg, decl->script, decl->node);
							}

							// Move the function from the methods array to the virtualFunctionTable
							decl->objType->methods.RemoveIndex(d);
							decl->objType->virtualFunctionTable.PushLast(derivedFunc);
							found = true;
							break;
						}
					}
					else
					{
						if( derivedFunc->name == baseFunc->name &&
							derivedFunc->IsSignatureExceptNameAndReturnTypeEqual(baseFunc) )
						{
							if( baseFunc->returnType != derivedFunc->returnType )
							{
								asCString msg;
								msg.Format(TXT_DERIVED_METHOD_MUST_HAVE_SAME_RETTYPE_s, baseFunc->GetDeclaration());
								WriteError(msg, decl->script, decl->node);
							}

							if( baseFunc->IsFinal() )
							{
								asCString msg;
								msg.Format(TXT_METHOD_CANNOT_OVERRIDE_s, baseFunc->GetDeclaration());
								WriteError(msg, decl->script, decl->node);
							}

							// Move the function from the methods array to the virtualFunctionTable
							decl->objType->methods.RemoveIndex(d);
							decl->objType->virtualFunctionTable.PushLast(derivedFunc);
							found = true;
							break;
						}
					}
				}

				if( !found )
				{
					// Push the base class function on the virtual function table
					decl->objType->virtualFunctionTable.PushLast(baseType->virtualFunctionTable[m]);
					baseType->virtualFunctionTable[m]->AddRefInternal();

					CheckForConflictsDueToDefaultArgs(decl->script, decl->node, baseType->virtualFunctionTable[m], decl->objType);
				}

				decl->objType->methods.PushLast(baseType->methods[m]);
				engine->scriptFunctions[baseType->methods[m]]->AddRefInternal();
			}
		}

		if( !decl->isExistingShared )
		{
			// Move this class' methods into the virtual function table
			for( asUINT m = 0; m < decl->objType->methods.GetLength(); m++ )
			{
				asCScriptFunction *func = GetFunctionDescription(decl->objType->methods[m]);
				if( func->funcType != asFUNC_VIRTUAL )
				{
					// Move the reference from the method list to the virtual function list
					decl->objType->methods.RemoveIndex(m);
					decl->objType->virtualFunctionTable.PushLast(func);

					// Substitute the function description in the method list for a virtual method
					// Make sure the methods are in the same order as the virtual function table
					decl->objType->methods.PushLast(CreateVirtualFunction(func, (int)decl->objType->virtualFunctionTable.GetLength() - 1));
					m--;
				}
			}

			// Make virtual function table chunks for each implemented interface
			for( asUINT n = 0; n < decl->objType->interfaces.GetLength(); n++ )
			{
				asCObjectType *intf = decl->objType->interfaces[n];

				// Add all the interface's functions to the virtual function table
				asUINT offset = asUINT(decl->objType->virtualFunctionTable.GetLength());
				decl->objType->interfaceVFTOffsets.PushLast(offset);

				for( asUINT j = 0; j < intf->methods.GetLength(); j++ )
				{
					asCScriptFunction *intfFunc = GetFunctionDescription(intf->methods[j]);

					// Only create the table for functions that are explicitly from this interface,
					// inherited interface methods will be put in that interface's table.
					if( intfFunc->objectType != intf )
						continue;

					asASSERT((asUINT)intfFunc->vfTableIdx == j);

					//Find the interface function in the list of methods
					asCScriptFunction *realFunc = 0;
					for( asUINT p = 0; p < decl->objType->methods.GetLength(); p++ )
					{
						asCScriptFunction *func = GetFunctionDescription(decl->objType->methods[p]);

						if( func->signatureId == intfFunc->signatureId )
						{
							if( func->funcType == asFUNC_VIRTUAL )
							{
								realFunc = decl->objType->virtualFunctionTable[func->vfTableIdx];
							}
							else
							{
								// This should not happen, all methods were moved into the virtual table
								asASSERT(false);
							}
							break;
						}
					}

					// If realFunc is still null, the interface was not
					// implemented and we error out later in the checks.
					decl->objType->virtualFunctionTable.PushLast(realFunc);
					if( realFunc )
						realFunc->AddRefInternal();
				}
			}
		}

		// Enumerate each of the declared properties
		asCScriptNode *node = decl->node->firstChild->next;

		// Skip list of classes and interfaces
		while( node && node->nodeType == snIdentifier )
			node = node->next;

		while( node )
		{
			if( node->nodeType == snDeclaration )
			{
				asCScriptNode *n = node->firstChild;

				// Is the property declared as private or protected?
				bool isPrivate = false, isProtected = false;
				if( n && n->tokenType == ttPrivate )
				{
					isPrivate = true;
					n = n->next;
				}
				else if( n && n->tokenType == ttProtected )
				{
					isProtected = true;
					n = n->next;
				}

				// Determine the type of the property
				asCScriptCode *file = decl->script;
				asCDataType dt = CreateDataTypeFromNode(n, file, decl->objType->nameSpace);
				if( decl->objType->IsShared() && dt.GetObjectType() && !dt.GetObjectType()->IsShared() )
				{
					asCString msg;
					msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, dt.GetObjectType()->name.AddressOf());
					WriteError(msg, file, node);
				}

				if( dt.IsReadOnly() )
					WriteError(TXT_PROPERTY_CANT_BE_CONST, file, node);

				// Multiple properties can be declared separated by ,
				n = n->next;
				while( n )
				{
					asCString name(&file->code[n->tokenPos], n->tokenLength);

					if( !decl->isExistingShared )
					{
						CheckNameConflictMember(decl->objType, name.AddressOf(), n, file, true);
						AddPropertyToClass(decl, name, dt, isPrivate, isProtected, false, file, n);
					}
					else
					{
						// Verify that the property exists in the original declaration
						bool found = false;
						for( asUINT p = 0; p < decl->objType->properties.GetLength(); p++ )
						{
							asCObjectProperty *prop = decl->objType->properties[p];
							if( prop->isPrivate == isPrivate &&
								prop->isProtected == isProtected &&
								prop->name == name &&
								prop->type.IsEqualExceptRef(dt) )
							{
								found = true;
								break;
							}
						}
						if( !found )
						{
							asCString str;
							str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, decl->objType->GetName());
							WriteError(str, file, n);
						}
					}

					// Skip the initialization node
					if( n->next && n->next->nodeType != snIdentifier )
						n = n->next;

					n = n->next;
				}
			}
			else
				asASSERT(false);

			node = node->next;
		}

		// Add properties from included mixin classes that don't conflict with existing properties
		IncludePropertiesFromMixins(decl);

		if( !decl->isExistingShared )
			toValidate.PushLast(decl);

		asASSERT( decl->objType->interfaces.GetLength() == decl->objType->interfaceVFTOffsets.GetLength() );
	}

	// TODO: Warn if a method overrides a base method without marking it as 'override'.
	//       It must be possible to turn off this warning through engine property.

	// TODO: A base class should be able to mark a method as 'abstract'. This will
	//       allow a base class to provide a partial implementation, but still force
	//       derived classes to implement specific methods.

	// Verify that all interface methods are implemented in the classes
	// We do this here so the base class' methods have already been inherited
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		sClassDeclaration *decl = classDeclarations[n];
		if( decl->isExistingShared ) continue;

		asCArray<bool> overrideValidations(decl->objType->GetMethodCount());
		for( asUINT k = 0; k < decl->objType->methods.GetLength(); k++ )
			overrideValidations.PushLast( !static_cast<asCScriptFunction*>(decl->objType->GetMethodByIndex(k, false))->IsOverride() );

		for( asUINT m = 0; m < decl->objType->interfaces.GetLength(); m++ )
		{
			asCObjectType *objType = decl->objType->interfaces[m];
			for( asUINT i = 0; i < objType->methods.GetLength(); i++ )
			{
				// Only check the interface methods that was explicitly declared in this interface
				// Methods that was inherited from other interfaces will be checked in those interfaces
				if( objType != engine->scriptFunctions[objType->methods[i]]->objectType )
					continue;

				asUINT overrideIndex;
				if( !DoesMethodExist(decl->objType, objType->methods[i], &overrideIndex) )
				{
					asCString str;
					str.Format(TXT_MISSING_IMPLEMENTATION_OF_s,
						engine->GetFunctionDeclaration(objType->methods[i]).AddressOf());
					WriteError(str, decl->script, decl->node);
				}
				else
					overrideValidations[overrideIndex] = true;
			}
		}

		bool hasBaseClass = decl->objType->derivedFrom != 0;

		for( asUINT j = 0; j < overrideValidations.GetLength(); j++ )
		{
			if( !overrideValidations[j] && (!hasBaseClass || !DoesMethodExist(decl->objType->derivedFrom, decl->objType->methods[j])) )
			{
				asCString msg;
				msg.Format(TXT_METHOD_s_DOES_NOT_OVERRIDE, decl->objType->GetMethodByIndex(j, false)->GetDeclaration());
				WriteError(msg, decl->script, decl->node);
			}
		}
	}

	// Verify that the declared structures are valid, e.g. that the structure
	// doesn't contain a member of its own type directly or indirectly
	while( toValidate.GetLength() > 0 )
	{
		asUINT numClasses = (asUINT)toValidate.GetLength();

		asCArray<sClassDeclaration*> toValidateNext((int)toValidate.GetLength());
		while( toValidate.GetLength() > 0 )
		{
			sClassDeclaration *decl = toValidate[toValidate.GetLength()-1];
			int validState = 1;
			for( asUINT n = 0; n < decl->objType->properties.GetLength(); n++ )
			{
				// A valid structure is one that uses only primitives or other valid objects
				asCObjectProperty *prop = decl->objType->properties[n];
				asCDataType dt = prop->type;

				// TODO: Add this check again, once solving the issues commented below
				/* 
				if( dt.IsTemplate() )
				{
					// TODO: This must verify all sub types, not just the first one
					// TODO: Just because the subtype is not a handle doesn't mean the template will actually instance the object
					//       this it shouldn't automatically raise an error for this, e.g. weakref<Object> should be legal as member
					//       of the Object class
					asCDataType sub = dt;
					while( sub.IsTemplate() && !sub.IsObjectHandle() )
						sub = sub.GetSubType();

					dt = sub;
				}
				*/

				if( dt.IsObject() && !dt.IsObjectHandle() )
				{
					// Find the class declaration
					sClassDeclaration *pdecl = 0;
					for( asUINT p = 0; p < classDeclarations.GetLength(); p++ )
					{
						if( classDeclarations[p]->objType == dt.GetObjectType() )
						{
							pdecl = classDeclarations[p];
							break;
						}
					}

					if( pdecl )
					{
						if( pdecl->objType == decl->objType )
						{
							WriteError(TXT_ILLEGAL_MEMBER_TYPE, decl->script, decl->node);
							validState = 2;
							break;
						}
						else if( pdecl->validState != 1 )
						{
							validState = pdecl->validState;
							break;
						}
					}
				}
			}

			if( validState == 1 )
			{
				decl->validState = 1;
				toValidate.PopLast();
			}
			else if( validState == 2 )
			{
				decl->validState = 2;
				toValidate.PopLast();
			}
			else
			{
				toValidateNext.PushLast(toValidate.PopLast());
			}
		}

		toValidate = toValidateNext;
		toValidateNext.SetLength(0);

		if( numClasses == toValidate.GetLength() )
		{
			WriteError(TXT_ILLEGAL_MEMBER_TYPE, toValidate[0]->script, toValidate[0]->node);
			break;
		}
	}

	if( numErrors > 0 ) return;

	// Verify which script classes can really form circular references, and mark only those as garbage collected.
	// This must be done in the correct order, so that a class that contains another class isn't needlessly marked
	// as garbage collected, just because the contained class was evaluated afterwards.

	// TODO: runtime optimize: This algorithm can be further improved by checking the types that inherits from
	//                         a base class. If the base class is not shared all the classes that derive from it
	//                         are known at compile time, and can thus be checked for potential circular references too.
	//
	//                         Observe, that doing this would conflict with another potential future feature, which is to
	//                         allow incremental builds, i.e. allow application to add or replace classes in an
	//                         existing module. However, the applications that want to use that should use a special
	//                         build flag to not finalize the module.

	asCArray<asCObjectType*> typesToValidate;
	for( n = 0; n < classDeclarations.GetLength(); n++ )
	{
		// Existing shared classes won't need evaluating, nor interfaces
		sClassDeclaration *decl = classDeclarations[n];
		if( decl->isExistingShared ) continue;
		if( decl->objType->IsInterface() ) continue;

		typesToValidate.PushLast(decl->objType);
	}

	asUINT numReevaluations = 0;
	while( typesToValidate.GetLength() )
	{
		if( numReevaluations > typesToValidate.GetLength() )
		{
			// No types could be completely evaluated in the last iteration so 
			// we consider the remaining types in the array as garbage collected
			break;
		}

		asCObjectType *type = typesToValidate[0];
		typesToValidate.RemoveIndex(0);

		// If the type inherits from another type that is yet to be validated, then reinsert it at the end
		if( type->derivedFrom && typesToValidate.Exists(type->derivedFrom) )
		{
			typesToValidate.PushLast(type);
			numReevaluations++;
			continue;
		}

		// If the type inherits from a known garbage collected type, then this type must also be garbage collected
		if( type->derivedFrom && (type->derivedFrom->flags & asOBJ_GC) )
		{
			type->flags |= asOBJ_GC;
			continue;
		}

		// Evaluate template instances (silently) before verifying each of the classes, since it is possible that
		// a class will be marked as non-garbage collected, which in turn will mark the template instance that uses
		// it as non-garbage collected, which in turn means the class that contains the array also do not have to be 
		// garbage collected
		EvaluateTemplateInstances(numTempl, true);

		// Is there some path in which this structure is involved in circular references?
		// If the type contains a member of a type that is yet to be validated, then reinsert it at the end
		bool mustReevaluate = false;
		bool gc = false;
		for( asUINT p = 0; p < type->properties.GetLength(); p++ )
		{
			asCDataType dt = type->properties[p]->type;
			if( !dt.IsObject() )
				continue;

			if( typesToValidate.Exists(dt.GetObjectType()) )
				mustReevaluate = true;
			else
			{
				if( dt.IsTemplate() )
				{
					// Check if any of the subtypes are yet to be evaluated
					bool skip = false;
					for( asUINT s = 0; s < dt.GetObjectType()->GetSubTypeCount(); s++ )
					{
						asCObjectType *t = reinterpret_cast<asCObjectType*>(dt.GetObjectType()->GetSubType(s));
						if( typesToValidate.Exists(t) )
						{
							mustReevaluate = true;
							skip = true;
							break;
						}
					}
					if( skip )
						continue;
				}

				if( dt.IsObjectHandle() )
				{
					// If it is known that the handle can't be involved in a circular reference
					// then this object doesn't need to be marked as garbage collected.
					asCObjectType *prop = dt.GetObjectType();

					if( prop->flags & asOBJ_SCRIPT_OBJECT )
					{
						// For script objects, treat non-final classes as if they can contain references
						// as it is not known what derived classes might do. For final types, check all
						// properties to determine if any of those can cause a circular reference with this
						// class.
						if( prop->flags & asOBJ_NOINHERIT )
						{
							for( asUINT sp = 0; sp < prop->properties.GetLength(); sp++ )
							{
								asCDataType sdt = prop->properties[sp]->type;

								if( sdt.IsObject() )
								{
									if( sdt.IsObjectHandle() )
									{
										// TODO: runtime optimize: If the handle is again to a final class, then we can recursively check if the circular reference can occur
										if( sdt.GetObjectType()->flags & (asOBJ_SCRIPT_OBJECT | asOBJ_GC) )
										{
											gc = true;
											break;
										}
									}
									else if( sdt.GetObjectType()->flags & asOBJ_GC )
									{
										// TODO: runtime optimize: Just because the member type is a potential circle doesn't mean that this one is.
										//                         Only if the object is of a type that can reference this type, either directly or indirectly
										gc = true;
										break;
									}
								}
							}

							if( gc )
								break;
						}
						else
						{
							// Assume it is garbage collected as it is not known at compile time what might inherit from this type
							gc = true;
							break;
						}
					}
					else if( prop->flags & asOBJ_GC )
					{
						// If a type is not a script object, adopt its GC flag
						// TODO: runtime optimize: Just because an application registered class is garbage collected, doesn't mean it 
						//                         can form a circular reference with this script class. Perhaps need a flag to tell
						//                         if the script classes that contains the type should be garbage collected or not.
						gc = true;
						break;
					}
				}
				else if( dt.GetObjectType()->flags & asOBJ_GC )
				{
					// TODO: runtime optimize: Just because the member type is a potential circle doesn't mean that this one is.
					//                         Only if the object is of a type that can reference this type, either directly or indirectly
					gc = true;
					break;
				}
			}
		}

		// If the class wasn't found to require garbage collection, but it 
		// contains another type that has yet to be evaluated then it must be 
		// re-evaluated.
		if( !gc && mustReevaluate )
		{
			typesToValidate.PushLast(type);
			numReevaluations++;
			continue;
		}

		// Update the flag in the object type
		if( gc )
			type->flags |= asOBJ_GC;
		else
			type->flags &= ~asOBJ_GC;

		// Reset the counter
		numReevaluations = 0;
	}
}

void asCBuilder::IncludeMethodsFromMixins(sClassDeclaration *decl)
{
	asCScriptNode *node = decl->node->firstChild;

	// Skip the class attributes
	while( node->nodeType == snIdentifier &&
		   !decl->script->TokenEquals(node->tokenPos, node->tokenLength, decl->name.AddressOf()) )
		node = node->next;

	// Skip the name of the class
	node = node->next;

	// Find the included mixin classes
	while( node && node->nodeType == snIdentifier )
	{
		asSNameSpace *ns;
		asCString name;
		if( GetNamespaceAndNameFromNode(node, decl->script, decl->objType->nameSpace, ns, name) < 0 )
		{
			node = node->next;
			continue;
		}

		sMixinClass *mixin = 0;
		while( ns )
		{
			// Need to make sure the name is not an object type 
			asCObjectType *objType = GetObjectType(name.AddressOf(), ns);
			if( objType == 0 )
				mixin = GetMixinClass(name.AddressOf(), ns);
			
			if( objType || mixin )
				break;

			ns = engine->GetParentNameSpace(ns);
		}

		if( mixin )
		{
			// Find methods from mixin declaration
			asCScriptNode *n = mixin->node->firstChild;

			// Skip to the member declarations
			// Possible keywords 'final' and 'shared' are removed in RegisterMixinClass so we don't need to worry about those here
			while( n && n->nodeType == snIdentifier )
				n = n->next;

			// Add methods from the mixin that are not already existing in the class
			while( n )
			{
				if( n->nodeType == snFunction )
				{
					// Instead of disconnecting the node, we need to clone it, otherwise other
					// classes that include the same mixin will not see the methods
					asCScriptNode *copy = n->CreateCopy(engine);

					// Register the method, but only if it doesn't already exist in the class
					RegisterScriptFunctionFromNode(copy, mixin->script, decl->objType, false, false, mixin->ns, false, true);
				}
				else if( n->nodeType == snVirtualProperty )
				{
					// TODO: mixin: Support virtual properties too
					WriteError("The virtual property syntax is currently not supported for mixin classes", mixin->script, n);
					//RegisterVirtualProperty(node, decl->script, decl->objType, false, false);
				}

				n = n->next;
			}
		}

		node = node->next;
	}
}

void asCBuilder::IncludePropertiesFromMixins(sClassDeclaration *decl)
{
	asCScriptNode *node = decl->node->firstChild;

	// Skip the class attributes
	while( node->nodeType == snIdentifier &&
		   !decl->script->TokenEquals(node->tokenPos, node->tokenLength, decl->name.AddressOf()) )
		node = node->next;

	// Skip the name of the class
	node = node->next;

	// Find the included mixin classes
	while( node && node->nodeType == snIdentifier )
	{
		asSNameSpace *ns;
		asCString name;
		if( GetNamespaceAndNameFromNode(node, decl->script, decl->objType->nameSpace, ns, name) < 0 )
		{
			node = node->next;
			continue;
		}

		sMixinClass *mixin = 0;
		while( ns )
		{
			// Need to make sure the name is not an object type 
			asCObjectType *objType = GetObjectType(name.AddressOf(), ns);
			if( objType == 0 )
				mixin = GetMixinClass(name.AddressOf(), ns);
			
			if( objType || mixin )
				break;

			ns = engine->GetParentNameSpace(ns);
		}

		if( mixin )
		{
			// Find properties from mixin declaration
			asCScriptNode *n = mixin->node->firstChild;

			// Skip to the member declarations
			// Possible keywords 'final' and 'shared' are removed in RegisterMixinClass so we don't need to worry about those here
			while( n && n->nodeType == snIdentifier )
				n = n->next;

			// Add properties from the mixin that are not already existing in the class
			while( n )
			{
				if( n->nodeType == snDeclaration )
				{
					asCScriptNode *n2 = n->firstChild;
					bool isPrivate = false, isProtected = false;
					if( n2 && n2->tokenType == ttPrivate )
					{
						isPrivate = true;
						n2 = n2->next;
					}
					else if( n2 && n2->tokenType == ttProtected )
					{
						isProtected = true;
						n2 = n2->next;
					}

					asCScriptCode *file = mixin->script;
					asCDataType dt = CreateDataTypeFromNode(n2, file, mixin->ns);

					if( decl->objType->IsShared() && dt.GetObjectType() && !dt.GetObjectType()->IsShared() )
					{
						asCString msg;
						msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, dt.GetObjectType()->name.AddressOf());
						WriteError(msg, file, n);
						WriteInfo(TXT_WHILE_INCLUDING_MIXIN, decl->script, node);
					}

					if( dt.IsReadOnly() )
						WriteError(TXT_PROPERTY_CANT_BE_CONST, file, n);

					n2 = n2->next;
					while( n2 )
					{
						asCString name(&file->code[n2->tokenPos], n2->tokenLength);

						// Add the property only if it doesn't already exist in the class
						bool exists = false;
						for( asUINT p = 0; p < decl->objType->properties.GetLength(); p++ )
							if( decl->objType->properties[p]->name == name )
							{
								exists = true;
								break;
							}

						if( !exists )
						{
							if( !decl->isExistingShared )
							{
								// It must not conflict with the name of methods
								int r = CheckNameConflictMember(decl->objType, name.AddressOf(), n2, file, true);
								if( r < 0 )
									WriteInfo(TXT_WHILE_INCLUDING_MIXIN, decl->script, node);

								AddPropertyToClass(decl, name, dt, isPrivate, isProtected, false, file, n2);
							}
							else
							{
								// Verify that the property exists in the original declaration
								bool found = false;
								for( asUINT p = 0; p < decl->objType->properties.GetLength(); p++ )
								{
									asCObjectProperty *prop = decl->objType->properties[p];
									if( prop->isPrivate == isPrivate &&
										prop->isProtected == isProtected &&
										prop->name == name &&
										prop->type == dt )
									{
										found = true;
										break;
									}
								}
								if( !found )
								{
									asCString str;
									str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, decl->objType->GetName());
									WriteError(str, decl->script, decl->node);
									WriteInfo(TXT_WHILE_INCLUDING_MIXIN, decl->script, node);
								}
							}
						}

						// Skip the initialization expression
						if( n2->next && n2->next->nodeType != snIdentifier )
							n2 = n2->next;

						n2 = n2->next;
					}
				}

				n = n->next;
			}
		}

		node = node->next;
	}
}

int asCBuilder::CreateVirtualFunction(asCScriptFunction *func, int idx)
{
	asCScriptFunction *vf = asNEW(asCScriptFunction)(engine, module, asFUNC_VIRTUAL);
	if( vf == 0 )
		return asOUT_OF_MEMORY;

	vf->name             = func->name;
	vf->returnType       = func->returnType;
	vf->parameterTypes   = func->parameterTypes;
	vf->inOutFlags       = func->inOutFlags;
	vf->id               = engine->GetNextScriptFunctionId();
	vf->isReadOnly       = func->isReadOnly;
	vf->objectType       = func->objectType;
	vf->objectType->AddRefInternal();
	vf->signatureId      = func->signatureId;
	vf->isPrivate        = func->isPrivate;
	vf->isProtected      = func->isProtected;
	vf->isFinal          = func->isFinal;
	vf->isOverride       = func->isOverride;
	vf->vfTableIdx       = idx;

	// It is not necessary to copy the default args, as they have no meaning in the virtual function

	module->AddScriptFunction(vf);

	// Add a dummy to the builder so that it doesn't mix up function ids
	functions.PushLast(0);

	return vf->id;
}

asCObjectProperty *asCBuilder::AddPropertyToClass(sClassDeclaration *decl, const asCString &name, const asCDataType &dt, bool isPrivate, bool isProtected, bool isInherited, asCScriptCode *file, asCScriptNode *node)
{
	if( node )
	{
		asASSERT(!isInherited);

		// Check if the property is allowed
		if( !dt.CanBeInstantiated() )
		{
			if( file && node )
			{
				asCString str;
				if( dt.IsAbstractClass() )
					str.Format(TXT_ABSTRACT_CLASS_s_CANNOT_BE_INSTANTIATED, dt.Format(decl->objType->nameSpace).AddressOf());
				else if( dt.IsInterface() )
					str.Format(TXT_INTERFACE_s_CANNOT_BE_INSTANTIATED, dt.Format(decl->objType->nameSpace).AddressOf());
				else
					// TODO: Improve error message to explain why
					str.Format(TXT_DATA_TYPE_CANT_BE_s, dt.Format(decl->objType->nameSpace).AddressOf());
				WriteError(str, file, node);
			}
			return 0;
		}

		// Register the initialization expression (if any) to be compiled later
		asCScriptNode *declNode = node;
		asCScriptNode *initNode = 0;
		if( node->next && node->next->nodeType != snIdentifier )
		{
			asASSERT( node->next->nodeType == snAssignment );
			initNode = node->next;
		}

		sPropertyInitializer p(name, declNode, initNode, file);
		decl->propInits.PushLast(p);
	}
	else
	{
		// If the declaration node is not given, then
		// this property is inherited from a base class
		asASSERT(isInherited);
	}

	// Add the property to the object type
	return decl->objType->AddPropertyToClass(name, dt, isPrivate, isProtected, isInherited);
}

bool asCBuilder::DoesMethodExist(asCObjectType *objType, int methodId, asUINT *methodIndex)
{
	asCScriptFunction *method = GetFunctionDescription(methodId);

	for( asUINT n = 0; n < objType->methods.GetLength(); n++ )
	{
		asCScriptFunction *m = GetFunctionDescription(objType->methods[n]);

		if( m->name           != method->name           ) continue;
		if( m->returnType     != method->returnType     ) continue;
		if( m->isReadOnly     != method->isReadOnly     ) continue;
		if( m->parameterTypes != method->parameterTypes ) continue;
		if( m->inOutFlags     != method->inOutFlags     ) continue;

		if( methodIndex )
			*methodIndex = n;

		return true;
	}

	return false;
}

void asCBuilder::AddDefaultConstructor(asCObjectType *objType, asCScriptCode *file)
{
	int funcId = engine->GetNextScriptFunctionId();

	asCDataType returnType = asCDataType::CreatePrimitive(ttVoid, false);
	asCArray<asCDataType> parameterTypes;
	asCArray<asETypeModifiers> inOutFlags;
	asCArray<asCString *> defaultArgs;
	asCArray<asCString> parameterNames;

	// Add the script function
	// TODO: declaredAt should be set to where the class has been declared
	module->AddScriptFunction(file->idx, 0, funcId, objType->name, returnType, parameterTypes, parameterNames, inOutFlags, defaultArgs, false, objType);

	// Set it as default constructor
	if( objType->beh.construct )
		engine->scriptFunctions[objType->beh.construct]->ReleaseInternal();
	objType->beh.construct = funcId;
	objType->beh.constructors[0] = funcId;
	engine->scriptFunctions[funcId]->AddRefInternal();

	// The bytecode for the default constructor will be generated
	// only after the potential inheritance has been established
	sFunctionDescription *func = asNEW(sFunctionDescription);
	if( func == 0 )
	{
		// Out of memory
		return;
	}

	functions.PushLast(func);

	func->script            = file;
	func->node              = 0;
	func->name              = objType->name;
	func->objType           = objType;
	func->funcId            = funcId;
	func->isExistingShared  = false;

	// Add a default factory as well
	funcId = engine->GetNextScriptFunctionId();
	if( objType->beh.factory )
		engine->scriptFunctions[objType->beh.factory]->ReleaseInternal();
	objType->beh.factory = funcId;
	objType->beh.factories[0] = funcId;
	returnType = asCDataType::CreateObjectHandle(objType, false);
	// TODO: should be the same as the constructor
	module->AddScriptFunction(file->idx, 0, funcId, objType->name, returnType, parameterTypes, parameterNames, inOutFlags, defaultArgs, false);
	functions.PushLast(0);
	asCCompiler compiler(engine);
	compiler.CompileFactory(this, file, engine->scriptFunctions[funcId]);
	engine->scriptFunctions[funcId]->AddRefInternal();

	// If the object is shared, then the factory must also be marked as shared
	if( objType->flags & asOBJ_SHARED )
		engine->scriptFunctions[funcId]->isShared = true;
}

int asCBuilder::RegisterEnum(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	// Is it a shared enum?
	bool isShared = false;
	asCObjectType *existingSharedType = 0;
	asCScriptNode *tmp = node->firstChild;
	if( tmp->nodeType == snIdentifier && file->TokenEquals(tmp->tokenPos, tmp->tokenLength, SHARED_TOKEN) )
	{
		isShared = true;
		tmp = tmp->next;
	}

	// Grab the name of the enumeration
	asCString name;
	asASSERT(snDataType == tmp->nodeType);
	asASSERT(snIdentifier == tmp->firstChild->nodeType);
	name.Assign(&file->code[tmp->firstChild->tokenPos], tmp->firstChild->tokenLength);

	if( isShared )
	{
		// Look for a pre-existing shared enum with the same signature
		for( asUINT n = 0; n < engine->sharedScriptTypes.GetLength(); n++ )
		{
			asCObjectType *o = engine->sharedScriptTypes[n];
			if( o &&
				o->IsShared() &&
				(o->flags & asOBJ_ENUM) &&
				o->name == name &&
				o->nameSpace == ns )
			{
				existingSharedType = o;
				break;
			}
		}
	}

	// Check the name and add the enum
	int r = CheckNameConflict(name.AddressOf(), tmp->firstChild, file, ns);
	if( asSUCCESS == r )
	{
		asCObjectType *st;

		if( existingSharedType )
		{
			st = existingSharedType;
			st->AddRefInternal();
		}
		else
		{
			st = asNEW(asCObjectType)(engine);
			if( st == 0 )
				return asOUT_OF_MEMORY;

			st->flags     = asOBJ_ENUM;
			if( isShared )
				st->flags |= asOBJ_SHARED;
			st->size      = 4;
			st->name      = name;
			st->nameSpace = ns;
			st->module    = module;
		}
		module->enumTypes.PushLast(st);

		if( !existingSharedType && isShared )
		{
			engine->sharedScriptTypes.PushLast(st);
			st->AddRefInternal();
		}

		// Store the location of this declaration for reference in name collisions
		sClassDeclaration *decl = asNEW(sClassDeclaration);
		if( decl == 0 )
			return asOUT_OF_MEMORY;

		decl->name             = name;
		decl->script           = file;
		decl->objType          = st;
		namedTypeDeclarations.PushLast(decl);

		asCDataType type = CreateDataTypeFromNode(tmp, file, ns);
		asASSERT(!type.IsReference());

		// Register the enum values
		tmp = tmp->next;
		while( tmp )
		{
			asASSERT(snIdentifier == tmp->nodeType);

			asCString name(&file->code[tmp->tokenPos], tmp->tokenLength);

			if( existingSharedType )
			{
				// If this is a pre-existent shared enum, then just double check
				// that the value is already defined in the original declaration
				bool found = false;
				for( asUINT n = 0; n < st->enumValues.GetLength(); n++ )
					if( st->enumValues[n]->name == name )
					{
						found = true;
						break;
					}

				if( !found )
				{
					asCString str;
					str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, st->GetName());
					WriteError(str, file, tmp);
					break;
				}

				tmp = tmp->next;
				if( tmp && tmp->nodeType == snAssignment )
					tmp = tmp->next;
				continue;
			}
			else
			{
				// Check for name conflict errors with other values in the enum
				if( globVariables.GetFirst(ns, name, asCCompGlobVarType(type)) )
				{
					asCString str;
					str.Format(TXT_NAME_CONFLICT_s_ALREADY_USED, name.AddressOf());
					WriteError(str, file, tmp);

					tmp = tmp->next;
					if( tmp && tmp->nodeType == snAssignment )
						tmp = tmp->next;
					continue;
				}

				// Check for assignment
				asCScriptNode *asnNode = tmp->next;
				if( asnNode && snAssignment == asnNode->nodeType )
					asnNode->DisconnectParent();
				else
					asnNode = 0;

				// Create the global variable description so the enum value can be evaluated
				sGlobalVariableDescription *gvar = asNEW(sGlobalVariableDescription);
				if( gvar == 0 )
					return asOUT_OF_MEMORY;

				gvar->script             = file;
				gvar->declaredAtNode     = tmp;
				tmp = tmp->next;
				gvar->declaredAtNode->DisconnectParent();
				gvar->initializationNode = asnNode;
				gvar->name               = name;
				gvar->datatype           = type;
				gvar->ns                 = ns;
				// No need to allocate space on the global memory stack since the values are stored in the asCObjectType
				// Set the index to a negative to allow compiler to diferentiate from ordinary global var when compiling the initialization
				gvar->index              = -1; 
				gvar->isCompiled         = false;
				gvar->isPureConstant     = true;
				gvar->isEnumValue        = true;
				gvar->constantValue      = 0xdeadbeef;

				// Allocate dummy property so we can compile the value.
				// This will be removed later on so we don't add it to the engine.
				gvar->property = asNEW(asCGlobalProperty);
				if( gvar->property == 0 )
					return asOUT_OF_MEMORY;

				gvar->property->name      = name;
				gvar->property->nameSpace = ns;
				gvar->property->type      = gvar->datatype;
				gvar->property->id        = 0;

				globVariables.Put(gvar);
			}
		}
	}

	node->Destroy(engine);

	return r;
}

int asCBuilder::RegisterTypedef(asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	// Get the native data type
	asCScriptNode *tmp = node->firstChild;
	asASSERT(NULL != tmp && snDataType == tmp->nodeType);
	asCDataType dataType;
	dataType.CreatePrimitive(tmp->tokenType, false);
	dataType.SetTokenType(tmp->tokenType);
	tmp = tmp->next;

	// Grab the name of the typedef
	asASSERT(NULL != tmp && NULL == tmp->next);
	asCString name;
	name.Assign(&file->code[tmp->tokenPos], tmp->tokenLength);

	// If the name is not already in use add it
 	int r = CheckNameConflict(name.AddressOf(), tmp, file, ns);

	asCObjectType *st = 0;
	if( asSUCCESS == r )
	{
		// Create the new type
		st = asNEW(asCObjectType)(engine);
		if( st == 0 )
			r = asOUT_OF_MEMORY;
	}

	if( asSUCCESS == r )
	{
		st->flags           = asOBJ_TYPEDEF;
		st->size            = dataType.GetSizeInMemoryBytes();
		st->name            = name;
		st->nameSpace       = ns;
		st->templateSubTypes.PushLast(dataType);
		st->module          = module;

		module->typeDefs.PushLast(st);

		// Store the location of this declaration for reference in name collisions
		sClassDeclaration *decl = asNEW(sClassDeclaration);
		if( decl == 0 )
			r = asOUT_OF_MEMORY;
		else
		{
			decl->name             = name;
			decl->script           = file;
			decl->objType          = st;
			namedTypeDeclarations.PushLast(decl);
		}
	}

	node->Destroy(engine);

	return r;
}

void asCBuilder::GetParsedFunctionDetails(asCScriptNode *node, asCScriptCode *file, asCObjectType *objType, asCString &name, asCDataType &returnType, asCArray<asCString> &parameterNames, asCArray<asCDataType> &parameterTypes, asCArray<asETypeModifiers> &inOutFlags, asCArray<asCString *> &defaultArgs, bool &isConstMethod, bool &isConstructor, bool &isDestructor, bool &isPrivate, bool &isProtected, bool &isOverride, bool &isFinal, bool &isShared, asSNameSpace *implicitNamespace)
{
	node = node->firstChild;

	// Is the function a private or protected class method?
	isPrivate = false, isProtected = false;
	if( node->tokenType == ttPrivate )
	{
		isPrivate = true;
		node = node->next;
	}
	else if( node->tokenType == ttProtected )
	{
		isProtected = true;
		node = node->next;
	}

	// Is the function shared?
	isShared = false;
	if( node->tokenType == ttIdentifier && file->TokenEquals(node->tokenPos, node->tokenLength, SHARED_TOKEN) )
	{
		isShared = true;
		node = node->next;
	}

	// Find the name
	isConstructor = false;
	isDestructor = false;
	asCScriptNode *n = 0;
	if( node->nodeType == snDataType )
		n = node->next->next;
	else
	{
		// If the first node is a ~ token, then we know it is a destructor
		if( node->tokenType == ttBitNot )
		{
			n = node->next;
			isDestructor = true;
		}
		else
		{
			n = node;
			isConstructor = true;
		}
	}
	name.Assign(&file->code[n->tokenPos], n->tokenLength);

	// Initialize a script function object for registration
	if( !isConstructor && !isDestructor )
	{
		returnType = CreateDataTypeFromNode(node, file, implicitNamespace);
		returnType = ModifyDataTypeFromNode(returnType, node->next, file, 0, 0);

		if( engine->ep.disallowValueAssignForRefType &&
			returnType.GetObjectType() && 
			(returnType.GetObjectType()->flags & asOBJ_REF) && 
			!(returnType.GetObjectType()->flags & asOBJ_SCOPED) &&
			!returnType.IsReference() && 
			!returnType.IsObjectHandle() )
		{
			WriteError(TXT_REF_TYPE_CANT_BE_RETURNED_BY_VAL, file, node);
		}
	}
	else
		returnType = asCDataType::CreatePrimitive(ttVoid, false);

	isConstMethod = false;
	isFinal = false;
	isOverride = false;

	if( objType && n->next->next )
	{
		asCScriptNode *decorator = n->next->next;

		// Is this a const method?
		if( decorator->tokenType == ttConst )
		{
			isConstMethod = true;
			decorator = decorator->next;
		}

		while( decorator )
		{
			if( decorator->tokenType == ttIdentifier && file->TokenEquals(decorator->tokenPos, decorator->tokenLength, FINAL_TOKEN) )
				isFinal = true;
			else if( decorator->tokenType == ttIdentifier && file->TokenEquals(decorator->tokenPos, decorator->tokenLength, OVERRIDE_TOKEN) )
				isOverride = true;

			decorator = decorator->next;
		}
	}

	// Count the number of parameters
	int count = 0;
	asCScriptNode *c = n->next->firstChild;
	while( c )
	{
		count++;
		c = c->next->next;
		if( c && c->nodeType == snIdentifier )
			c = c->next;
		if( c && c->nodeType == snExpression )
			c = c->next;
	}

	// Get the parameter types
	parameterNames.Allocate(count, false);
	parameterTypes.Allocate(count, false);
	inOutFlags.Allocate(count, false);
	defaultArgs.Allocate(count, false);
	n = n->next->firstChild;
	while( n )
	{
		asETypeModifiers inOutFlag;
		asCDataType type = CreateDataTypeFromNode(n, file, implicitNamespace);
		type = ModifyDataTypeFromNode(type, n->next, file, &inOutFlag, 0);

		if( engine->ep.disallowValueAssignForRefType &&
			type.GetObjectType() && 
			(type.GetObjectType()->flags & asOBJ_REF) && 
			!(type.GetObjectType()->flags & asOBJ_SCOPED) &&
			!type.IsReference() && 
			!type.IsObjectHandle() )
		{
			WriteError(TXT_REF_TYPE_CANT_BE_PASSED_BY_VAL, file, node);
		}

		// Store the parameter type
		parameterTypes.PushLast(type);
		inOutFlags.PushLast(inOutFlag);

		// Move to next parameter
		n = n->next->next;
		if( n && n->nodeType == snIdentifier )
		{
			asCString name;
			name.Assign(&file->code[n->tokenPos], n->tokenLength);
			parameterNames.PushLast(name);
			n = n->next;
		}
		else
		{
			// No name was given for the parameter
			parameterNames.PushLast(asCString());
		}

		if( n && n->nodeType == snExpression )
		{
			// Strip out white space and comments to better share the string
			asCString *defaultArgStr = asNEW(asCString);
			if( defaultArgStr )
				*defaultArgStr = GetCleanExpressionString(n, file);
			defaultArgs.PushLast(defaultArgStr);

			n = n->next;
		}
		else
			defaultArgs.PushLast(0);
	}
}
#endif

asCString asCBuilder::GetCleanExpressionString(asCScriptNode *node, asCScriptCode *file)
{
	asASSERT(node && node->nodeType == snExpression);

	asCString str;
	str.Assign(file->code + node->tokenPos, node->tokenLength);

	asCString cleanStr;
	for( asUINT n = 0; n < str.GetLength(); )
	{
		asUINT len = 0;
		asETokenClass tok = engine->ParseToken(str.AddressOf() + n, str.GetLength() - n, &len);
		if( tok != asTC_COMMENT && tok != asTC_WHITESPACE )
		{
			if( cleanStr.GetLength() ) cleanStr += " ";
			cleanStr.Concatenate(str.AddressOf() + n, len);
		}
		n += len;
	}

	return cleanStr;
}

#ifndef AS_NO_COMPILER
int asCBuilder::RegisterScriptFunctionFromNode(asCScriptNode *node, asCScriptCode *file, asCObjectType *objType, bool isInterface, bool isGlobalFunction, asSNameSpace *ns, bool isExistingShared, bool isMixin)
{
	asCString                  name;
	asCDataType                returnType;
	asCArray<asCString>        parameterNames;
	asCArray<asCDataType>      parameterTypes;
	asCArray<asETypeModifiers> inOutFlags;
	asCArray<asCString *>      defaultArgs;
	bool                       isConstMethod;
	bool                       isOverride;
	bool                       isFinal;
	bool                       isConstructor;
	bool                       isDestructor;
	bool                       isPrivate;
	bool                       isProtected;
	bool                       isShared;

	asASSERT( (objType && ns == 0) || isGlobalFunction || isMixin );

	// Set the default namespace
	if( ns == 0 )
	{
		if( objType )
			ns = objType->nameSpace;
		else
			ns = engine->nameSpaces[0];
	}

	GetParsedFunctionDetails(node, file, objType, name, returnType, parameterNames, parameterTypes, inOutFlags, defaultArgs, isConstMethod, isConstructor, isDestructor, isPrivate, isProtected, isOverride, isFinal, isShared, ns);

	return RegisterScriptFunction(node, file, objType, isInterface, isGlobalFunction, ns, isExistingShared, isMixin, name, returnType, parameterNames, parameterTypes, inOutFlags, defaultArgs, isConstMethod, isConstructor, isDestructor, isPrivate, isProtected, isOverride, isFinal, isShared);
}

asCScriptFunction *asCBuilder::RegisterLambda(asCScriptNode *node, asCScriptCode *file, asCScriptFunction *funcDef, const asCString &name, asSNameSpace *ns)
{
	// Get the parameter names from the node
	asCArray<asCString> parameterNames;
	asCArray<asCString*> defaultArgs;
	asCScriptNode *args = node->firstChild;
	while( args && args->nodeType == snIdentifier )
	{
		asCString argName;
		argName.Assign(&file->code[args->tokenPos], args->tokenLength);
		parameterNames.PushLast(argName);
		defaultArgs.PushLast(0);
		args = args->next;
	}

	// The statement block for the function must be disconnected, as the builder is going to be the owner of it
	args->DisconnectParent();

	// Get the return and parameter types from the funcDef
	asCString funcName = name;
	int r = RegisterScriptFunction(args, file, 0, 0, true, ns, false, false, funcName, funcDef->returnType, parameterNames, funcDef->parameterTypes, funcDef->inOutFlags, defaultArgs, false, false, false, false, false, false, false, false);
	if( r < 0 )
		return 0;

	// Return the function that was just created (but that will be compiled later)
	return engine->scriptFunctions[functions[functions.GetLength()-1]->funcId];
}

int asCBuilder::RegisterScriptFunction(asCScriptNode *node, asCScriptCode *file, asCObjectType *objType, bool isInterface, bool isGlobalFunction, asSNameSpace *ns, bool isExistingShared, bool isMixin, asCString &name, asCDataType &returnType, asCArray<asCString> &parameterNames, asCArray<asCDataType> &parameterTypes, asCArray<asETypeModifiers> &inOutFlags, asCArray<asCString *> &defaultArgs, bool isConstMethod, bool isConstructor, bool isDestructor, bool isPrivate, bool isProtected, bool isOverride, bool isFinal, bool isShared)
{
	// Determine default namespace if not specified
	if( ns == 0 )
	{
		if( objType )
			ns = objType->nameSpace;
		else
			ns = engine->nameSpaces[0];
	}

	if( isExistingShared )
	{
		asASSERT( objType );

		// Should validate that the function really exists in the class/interface
		bool found = false;
		if( isConstructor || isDestructor )
		{
			// TODO: shared: Should check the existance of these too
			found = true;
		}
		else
		{
			for( asUINT n = 0; n < objType->methods.GetLength(); n++ )
			{
				asCScriptFunction *func = engine->scriptFunctions[objType->methods[n]];
				if( func->name == name &&
					func->IsSignatureExceptNameEqual(returnType, parameterTypes, inOutFlags, objType, isConstMethod) )
				{
					// Add the shared function in this module too
					module->AddScriptFunction(func);

					found = true;
					break;
				}
			}
		}

		if( !found )
		{
			asCString str;
			str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, objType->GetName());
			WriteError(str, file, node);
		}

		// Free the default args
		for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
			if( defaultArgs[n] )
				asDELETE(defaultArgs[n], asCString);

		node->Destroy(engine);
		return 0;
	}

	// Check for name conflicts
	if( !isConstructor && !isDestructor )
	{
		if( objType )
		{
			CheckNameConflictMember(objType, name.AddressOf(), node, file, false);

			if( name == objType->name )
				WriteError(TXT_METHOD_CANT_HAVE_NAME_OF_CLASS, file, node);
		}
		else
			CheckNameConflict(name.AddressOf(), node, file, ns);
	}
	else
	{
		if( isMixin )
		{
			// Mixins cannot implement constructors/destructors
			WriteError(TXT_MIXIN_CANNOT_HAVE_CONSTRUCTOR, file, node);

			// Free the default args
			for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
				if( defaultArgs[n] )
					asDELETE(defaultArgs[n], asCString);

			node->Destroy(engine);
			return 0;
		}

		// Verify that the name of the constructor/destructor is the same as the class
		if( name != objType->name )
		{
			asCString str;
			if( isDestructor )
				str.Format(TXT_DESTRUCTOR_s_s_NAME_ERROR, objType->name.AddressOf(), name.AddressOf());
			else
				str.Format(TXT_METHOD_s_s_HAS_NO_RETURN_TYPE, objType->name.AddressOf(), name.AddressOf());
			WriteError(str, file, node);
		}

		if( isDestructor )
			name = "~" + name;
	}

	isExistingShared = false;
	int funcId = engine->GetNextScriptFunctionId();
	if( !isInterface )
	{
		sFunctionDescription *func = asNEW(sFunctionDescription);
		if( func == 0 )
		{
			// Free the default args
			for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
				if( defaultArgs[n] )
					asDELETE(defaultArgs[n], asCString);

			return asOUT_OF_MEMORY;
		}

		functions.PushLast(func);

		func->script            = file;
		func->node              = node;
		func->name              = name;
		func->objType           = objType;
		func->funcId            = funcId;
		func->isExistingShared  = false;
		func->paramNames        = parameterNames;

		if( isShared )
		{
			// Look for a pre-existing shared function with the same signature
			for( asUINT n = 0; n < engine->scriptFunctions.GetLength(); n++ )
			{
				asCScriptFunction *f = engine->scriptFunctions[n];
				if( f &&
					f->isShared &&
					f->name == name &&
					f->nameSpace == ns &&
					f->objectType == objType &&
					f->IsSignatureExceptNameEqual(returnType, parameterTypes, inOutFlags, 0, false) )
				{
					funcId = func->funcId = f->id;
					isExistingShared = func->isExistingShared = true;
					break;
				}
			}
		}
	}

	// Destructors may not have any parameters
	if( isDestructor && parameterTypes.GetLength() > 0 )
		WriteError(TXT_DESTRUCTOR_MAY_NOT_HAVE_PARM, file, node);

	// If a function, class, or interface is shared then only shared types may be used in the signature
	if( (objType && objType->IsShared()) || isShared )
	{
		asCObjectType *ot = returnType.GetObjectType();
		if( ot && !ot->IsShared() )
		{
			asCString msg;
			msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, ot->name.AddressOf());
			WriteError(msg, file, node);
		}

		for( asUINT p = 0; p < parameterTypes.GetLength(); ++p )
		{
			asCObjectType *ot = parameterTypes[p].GetObjectType();
			if( ot && !ot->IsShared() )
			{
				asCString msg;
				msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, ot->name.AddressOf());
				WriteError(msg, file, node);
			}
		}
	}

	// Check that the same function hasn't been registered already in the namespace
	asCArray<int> funcs;
	if( objType )
		GetObjectMethodDescriptions(name.AddressOf(), objType, funcs, false);
	else
		GetFunctionDescriptions(name.AddressOf(), funcs, ns);
	if( objType && (name == "opConv" || name == "opImplConv" || name == "opCast" || name == "opImplCast") && parameterTypes.GetLength() == 0 )
	{
		// opConv and opCast are special methods used for type casts
		for( asUINT n = 0; n < funcs.GetLength(); ++n )
		{
			asCScriptFunction *func = GetFunctionDescription(funcs[n]);
			if( func->IsSignatureExceptNameEqual(returnType, parameterTypes, inOutFlags, objType, isConstMethod) )
			{
				// TODO: clean up: Reuse the same error handling for both opConv and normal methods
				if( isMixin )
				{
					// Clean up the memory, as the function will not be registered
					if( node )
						node->Destroy(engine);
					sFunctionDescription *func = functions.PopLast();
					asDELETE(func, sFunctionDescription);

					// Free the default args
					for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
						if( defaultArgs[n] )
							asDELETE(defaultArgs[n], asCString);

					return 0;
				}

				WriteError(TXT_FUNCTION_ALREADY_EXIST, file, node);
				break;
			}
		}
	}
	else
	{
		for( asUINT n = 0; n < funcs.GetLength(); ++n )
		{
			asCScriptFunction *func = GetFunctionDescription(funcs[n]);
			if( func->IsSignatureExceptNameAndReturnTypeEqual(parameterTypes, inOutFlags, objType, isConstMethod) )
			{
				if( isMixin )
				{
					// Clean up the memory, as the function will not be registered
					if( node )
						node->Destroy(engine);
					sFunctionDescription *func = functions.PopLast();
					asDELETE(func, sFunctionDescription);

					// Free the default args
					for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
						if( defaultArgs[n] )
							asDELETE(defaultArgs[n], asCString);

					return 0;
				}

				WriteError(TXT_FUNCTION_ALREADY_EXIST, file, node);
				break;
			}
		}
	}

	// Register the function
	if( isExistingShared )
	{
		// Delete the default args as they won't be used anymore
		for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
			if( defaultArgs[n] )
				asDELETE(defaultArgs[n], asCString);

		asCScriptFunction *f = engine->scriptFunctions[funcId];
		module->AddScriptFunction(f);

		// TODO: clean up: This should be done by AddScriptFunction() itself
		module->globalFunctions.Put(f);
	}
	else
	{
		int row = 0, col = 0;
		if( node )
			file->ConvertPosToRowCol(node->tokenPos, &row, &col);
		module->AddScriptFunction(file->idx, (row&0xFFFFF)|((col&0xFFF)<<20), funcId, name, returnType, parameterTypes, parameterNames, inOutFlags, defaultArgs, isInterface, objType, isConstMethod, isGlobalFunction, isPrivate, isProtected, isFinal, isOverride, isShared, ns);
	}

	// Make sure the default args are declared correctly
	ValidateDefaultArgs(file, node, engine->scriptFunctions[funcId]);
	CheckForConflictsDueToDefaultArgs(file, node, engine->scriptFunctions[funcId], objType);

	if( objType )
	{
		asASSERT( !isExistingShared );

		engine->scriptFunctions[funcId]->AddRefInternal();
		if( isConstructor )
		{
			int factoryId = engine->GetNextScriptFunctionId();
			if( parameterTypes.GetLength() == 0 )
			{
				// Overload the default constructor
				engine->scriptFunctions[objType->beh.construct]->ReleaseInternal();
				objType->beh.construct = funcId;
				objType->beh.constructors[0] = funcId;

				// Register the default factory as well
				engine->scriptFunctions[objType->beh.factory]->ReleaseInternal();
				objType->beh.factory = factoryId;
				objType->beh.factories[0] = factoryId;
			}
			else
			{
				objType->beh.constructors.PushLast(funcId);

				// Register the factory as well
				objType->beh.factories.PushLast(factoryId);
			}

			// We must copy the default arg strings to avoid deleting the same object multiple times
			for( asUINT n = 0; n < defaultArgs.GetLength(); n++ )
				if( defaultArgs[n] )
					defaultArgs[n] = asNEW(asCString)(*defaultArgs[n]);

			asCDataType dt = asCDataType::CreateObjectHandle(objType, false);
			module->AddScriptFunction(file->idx, engine->scriptFunctions[funcId]->scriptData->declaredAt, factoryId, name, dt, parameterTypes, parameterNames, inOutFlags, defaultArgs, false);

			// If the object is shared, then the factory must also be marked as shared
			if( objType->flags & asOBJ_SHARED )
				engine->scriptFunctions[factoryId]->isShared = true;

			// Add a dummy function to the builder so that it doesn't mix up the fund Ids
			functions.PushLast(0);

			// Compile the factory immediately
			asCCompiler compiler(engine);
			compiler.CompileFactory(this, file, engine->scriptFunctions[factoryId]);
			engine->scriptFunctions[factoryId]->AddRefInternal();
		}
		else if( isDestructor )
			objType->beh.destruct = funcId;
		else
		{
			// If the method is the assignment operator we need to replace the default implementation
			asCScriptFunction *f = engine->scriptFunctions[funcId];
			if( f->name == "opAssign" && f->parameterTypes.GetLength() == 1 &&
				f->parameterTypes[0].GetObjectType() == f->objectType &&
				(f->inOutFlags[0] & asTM_INREF) )
			{
				engine->scriptFunctions[objType->beh.copy]->ReleaseInternal();
				objType->beh.copy = funcId;
				f->AddRefInternal();
			}

			objType->methods.PushLast(funcId);
		}
	}

	// We need to delete the node already if this is an interface method
	if( isInterface && node )
		node->Destroy(engine);

	return 0;
}

int asCBuilder::RegisterVirtualProperty(asCScriptNode *node, asCScriptCode *file, asCObjectType *objType, bool isInterface, bool isGlobalFunction, asSNameSpace *ns, bool isExistingShared)
{
	if( engine->ep.propertyAccessorMode != 2 )
	{
		WriteError(TXT_PROPERTY_ACCESSOR_DISABLED, file, node);
		node->Destroy(engine);
		return 0;
	}

	asASSERT( (objType && ns == 0) || isGlobalFunction );

	if( ns == 0 )
	{
		if( objType )
			ns = objType->nameSpace;
		else
			ns = engine->nameSpaces[0];
	}

	bool isPrivate = false, isProtected = false;
	asCString emulatedName;
	asCDataType emulatedType;

	asCScriptNode *mainNode = node;
	node = node->firstChild;

	if( !isGlobalFunction && node->tokenType == ttPrivate )
	{
		isPrivate = true;
		node = node->next;
	}
	else if( !isGlobalFunction && node->tokenType == ttProtected )
	{
		isProtected = true;
		node = node->next;
	}

	emulatedType = CreateDataTypeFromNode(node, file, ns);
	emulatedType = ModifyDataTypeFromNode(emulatedType, node->next, file, 0, 0);
	node = node->next->next;
	emulatedName.Assign(&file->code[node->tokenPos], node->tokenLength);

	if( node->next == 0 )
		WriteError(TXT_PROPERTY_WITHOUT_ACCESSOR, file, node);

	node = node->next;
	while( node )
	{
		asCScriptNode             *next        = node->next;
		asCScriptNode             *funcNode    = 0;
		bool                       success     = false;
		bool                       isConst     = false;
		bool                       isFinal     = false;
		bool                       isOverride  = false;
		asCDataType                returnType;
		asCArray<asCString>        paramNames;
		asCArray<asCDataType>      paramTypes;
		asCArray<asETypeModifiers> paramModifiers;
		asCArray<asCString*>       defaultArgs;
		asCString                  name;

		// TODO: getset: Allow private for individual property accessors
		// TODO: getset: If the accessor uses its own name, then the property should be automatically declared

		if( node->firstChild->nodeType == snIdentifier && file->TokenEquals(node->firstChild->tokenPos, node->firstChild->tokenLength, GET_TOKEN) )
		{
			funcNode  = node->firstChild->next;

			if( funcNode && funcNode->tokenType == ttConst )
			{
				isConst = true;
				funcNode = funcNode->next;
			}

			while( funcNode && funcNode->nodeType != snStatementBlock )
			{
				if( funcNode->tokenType == ttIdentifier && file->TokenEquals(funcNode->tokenPos, funcNode->tokenLength, FINAL_TOKEN) )
					isFinal = true;
				else if( funcNode->tokenType == ttIdentifier && file->TokenEquals(funcNode->tokenPos, funcNode->tokenLength, OVERRIDE_TOKEN) )
					isOverride = true;

				funcNode = funcNode->next;
			}

			if( funcNode )
				funcNode->DisconnectParent();

			if( funcNode == 0 && (objType == 0 || !objType->IsInterface()) )
			{
				// TODO: getset: If no implementation is supplied the builder should provide an automatically generated implementation
				//               The compiler needs to be able to handle the different types, primitive, value type, and handle
				//               The code is also different for global property accessors
				WriteError(TXT_PROPERTY_ACCESSOR_MUST_BE_IMPLEMENTED, file, node);
			}

			// Setup the signature for the get accessor method
			returnType = emulatedType;
			name       = "get_" + emulatedName;
			success    = true;
		}
		else if( node->firstChild->nodeType == snIdentifier && file->TokenEquals(node->firstChild->tokenPos, node->firstChild->tokenLength, SET_TOKEN) )
		{
			funcNode  = node->firstChild->next;

			if( funcNode && funcNode->tokenType == ttConst )
			{
				isConst = true;
				funcNode = funcNode->next;
			}

			while( funcNode && funcNode->nodeType != snStatementBlock )
			{
				if( funcNode->tokenType == ttIdentifier && file->TokenEquals(funcNode->tokenPos, funcNode->tokenLength, FINAL_TOKEN) )
					isFinal = true;
				else if( funcNode->tokenType == ttIdentifier && file->TokenEquals(funcNode->tokenPos, funcNode->tokenLength, OVERRIDE_TOKEN) )
					isOverride = true;

				funcNode = funcNode->next;
			}

			if( funcNode )
				funcNode->DisconnectParent();

			if( funcNode == 0 && (objType == 0 || !objType->IsInterface()) )
				WriteError(TXT_PROPERTY_ACCESSOR_MUST_BE_IMPLEMENTED, file, node);

			// Setup the signature for the set accessor method
			returnType = asCDataType::CreatePrimitive(ttVoid, false);
			paramModifiers.PushLast(asTM_NONE);
			paramNames.PushLast("value");
			paramTypes.PushLast(emulatedType);
			defaultArgs.PushLast(0);
			name = "set_" + emulatedName;
			success = true;
		}
		else
			WriteError(TXT_UNRECOGNIZED_VIRTUAL_PROPERTY_NODE, file, node);

		if( success )
		{
			if( !isExistingShared )
				RegisterScriptFunction(funcNode, file, objType, isInterface, isGlobalFunction, ns, false, false, name, returnType, paramNames, paramTypes, paramModifiers, defaultArgs, isConst, false, false, isPrivate, isProtected, isOverride, isFinal, false);
			else
			{
				// Free the funcNode as it won't be used
				if( funcNode ) funcNode->Destroy(engine);

				// Should validate that the function really exists in the class/interface
				bool found = false;
				for( asUINT n = 0; n < objType->methods.GetLength(); n++ )
				{
					asCScriptFunction *func = engine->scriptFunctions[objType->methods[n]];
					if( func->name == name &&
						func->IsSignatureExceptNameEqual(returnType, paramTypes, paramModifiers, objType, isConst) )
					{
						found = true;
						break;
					}
				}

				if( !found )
				{
					asCString str;
					str.Format(TXT_SHARED_s_DOESNT_MATCH_ORIGINAL, objType->GetName());
					WriteError(str, file, node);
				}
			}
		}

		node = next;
	};

	mainNode->Destroy(engine);

	return 0;
}

int asCBuilder::RegisterImportedFunction(int importID, asCScriptNode *node, asCScriptCode *file, asSNameSpace *ns)
{
	asCString                  name;
	asCDataType                returnType;
	asCArray<asCString>        parameterNames;
	asCArray<asCDataType>      parameterTypes;
	asCArray<asETypeModifiers> inOutFlags;
	asCArray<asCString *>      defaultArgs;
	bool isConstMethod, isOverride, isFinal, isConstructor, isDestructor, isPrivate, isProtected, isShared;

	if( ns == 0 )
		ns = engine->nameSpaces[0];

	GetParsedFunctionDetails(node->firstChild, file, 0, name, returnType, parameterNames, parameterTypes, inOutFlags, defaultArgs, isConstMethod, isConstructor, isDestructor, isPrivate, isProtected, isOverride, isFinal, isShared, ns);
	CheckNameConflict(name.AddressOf(), node, file, ns);

	// Check that the same function hasn't been registered already in the namespace
	asCArray<int> funcs;
	GetFunctionDescriptions(name.AddressOf(), funcs, ns);
	for( asUINT n = 0; n < funcs.GetLength(); ++n )
	{
		asCScriptFunction *func = GetFunctionDescription(funcs[n]);
		if( func->IsSignatureExceptNameAndReturnTypeEqual(parameterTypes, inOutFlags, 0, false) )
		{
			WriteError(TXT_FUNCTION_ALREADY_EXIST, file, node);
			break;
		}
	}

	// Read the module name as well
	asCScriptNode *nd = node->lastChild;
	asASSERT( nd->nodeType == snConstant && nd->tokenType == ttStringConstant );
	asCString moduleName;
	moduleName.Assign(&file->code[nd->tokenPos+1], nd->tokenLength-2);

	node->Destroy(engine);

	// Register the function
	module->AddImportedFunction(importID, name, returnType, parameterTypes, inOutFlags, defaultArgs, ns, moduleName);

	return 0;
}

asCScriptFunction *asCBuilder::GetFunctionDescription(int id)
{
	// TODO: import: This should be improved when the imported functions are removed
	// Get the description from the engine
	if( (id & FUNC_IMPORTED) == 0 )
		return engine->scriptFunctions[id];
	else
		return engine->importedFunctions[id & ~FUNC_IMPORTED]->importedFunctionSignature;
}

void asCBuilder::GetFunctionDescriptions(const char *name, asCArray<int> &funcs, asSNameSpace *ns)
{
	asUINT n;

	// Get the script declared global functions
	const asCArray<unsigned int> &idxs = module->globalFunctions.GetIndexes(ns, name);
	for( n = 0; n < idxs.GetLength(); n++ )
	{
		const asCScriptFunction *f = module->globalFunctions.Get(idxs[n]);
		asASSERT( f->objectType == 0 );
		funcs.PushLast(f->id);
	}

	// Add the imported functions
	// TODO: optimize: Linear search: This is probably not that critial. Also bindInformation will probably be removed in near future
	for( n = 0; n < module->bindInformations.GetLength(); n++ )
	{
		if( module->bindInformations[n]->importedFunctionSignature->name == name &&
			module->bindInformations[n]->importedFunctionSignature->nameSpace == ns )
			funcs.PushLast(module->bindInformations[n]->importedFunctionSignature->id);
	}

	// Add the registered global functions
	const asCArray<unsigned int> &idxs2 = engine->registeredGlobalFuncs.GetIndexes(ns, name);
	for( n = 0; n < idxs2.GetLength(); n++ )
	{
		asCScriptFunction *f = engine->registeredGlobalFuncs.Get(idxs2[n]);

		// Verify if the module has access to the function
		if( module->accessMask & f->accessMask )
		{
			funcs.PushLast(f->id);
		}
	}
}

void asCBuilder::GetObjectMethodDescriptions(const char *name, asCObjectType *objectType, asCArray<int> &methods, bool objIsConst, const asCString &scope, asCScriptNode *errNode, asCScriptCode *script)
{
	if( scope != "" )
	{
		// If searching with a scope informed, then the node and script must also be informed for potential error reporting
		asASSERT( errNode && script );

		// If the scope contains ::identifier, then use the last identifier as the class name and the rest of it as the namespace
		int n = scope.FindLast("::");
		asCString className = n >= 0 ? scope.SubString(n+2) : scope;
		asCString nsName = n >= 0 ? scope.SubString(0, n) : "";

		// Check if the namespace actually exist, if not return silently as this cannot be the referring to a base class
		asSNameSpace *ns = GetNameSpaceByString(nsName, objectType->nameSpace, errNode, script, false);
		if( ns == 0 )
			return;

		// Find the base class with the specified scope
		while( objectType && (objectType->name != className || objectType->nameSpace != ns) )
			objectType = objectType->derivedFrom;

		// If the scope is not any of the base classes, then return no methods
		if( objectType == 0 )
			return;
	}

	// TODO: optimize: Improve linear search
	if( objIsConst )
	{
		// Only add const methods to the list
		for( asUINT n = 0; n < objectType->methods.GetLength(); n++ )
		{
			asCScriptFunction *func = engine->scriptFunctions[objectType->methods[n]];
			if( func->name == name &&
				func->isReadOnly &&
				(func->accessMask & module->accessMask) )
			{
				// When the scope is defined the returned methods should be the true methods, not the virtual method stubs
				if( scope == "" )
					methods.PushLast(engine->scriptFunctions[objectType->methods[n]]->id);
				else
				{
					asCScriptFunction *virtFunc = engine->scriptFunctions[objectType->methods[n]];
					asCScriptFunction *realFunc = objectType->virtualFunctionTable[virtFunc->vfTableIdx];
					methods.PushLast(realFunc->id);
				}
			}
		}
	}
	else
	{
		// TODO: Prefer non-const over const
		for( asUINT n = 0; n < objectType->methods.GetLength(); n++ )
		{
			asCScriptFunction *func = engine->scriptFunctions[objectType->methods[n]];
			if( func->name == name &&
				(func->accessMask & module->accessMask) )
			{
				// When the scope is defined the returned methods should be the true methods, not the virtual method stubs
				if( scope == "" )
					methods.PushLast(engine->scriptFunctions[objectType->methods[n]]->id);
				else
				{
					asCScriptFunction *virtFunc = engine->scriptFunctions[objectType->methods[n]];
					asCScriptFunction *realFunc = objectType->virtualFunctionTable[virtFunc->vfTableIdx];
					methods.PushLast(realFunc->id);
				}
			}
		}
	}
}
#endif

void asCBuilder::WriteInfo(const asCString &scriptname, const asCString &message, int r, int c, bool pre)
{
	// Need to store the pre message in a structure
	if( pre )
	{
		engine->preMessage.isSet      = true;
		engine->preMessage.c          = c;
		engine->preMessage.r          = r;
		engine->preMessage.message    = message;
		engine->preMessage.scriptname = scriptname;
	}
	else
	{
		engine->preMessage.isSet = false;

		if( !silent )
			engine->WriteMessage(scriptname.AddressOf(), r, c, asMSGTYPE_INFORMATION, message.AddressOf());
	}
}

void asCBuilder::WriteInfo(const asCString &message, asCScriptCode *file, asCScriptNode *node)
{
	int r = 0, c = 0;
	if( node )
		file->ConvertPosToRowCol(node->tokenPos, &r, &c);

	WriteInfo(file->name, message, r, c, false);
}

void asCBuilder::WriteError(const asCString &message, asCScriptCode *file, asCScriptNode *node)
{
	int r = 0, c = 0;
	if( node && file )
		file->ConvertPosToRowCol(node->tokenPos, &r, &c);

	WriteError(file ? file->name : asCString(""), message, r, c);
}

void asCBuilder::WriteError(const asCString &scriptname, const asCString &message, int r, int c)
{
	numErrors++;

	if( !silent )
		engine->WriteMessage(scriptname.AddressOf(), r, c, asMSGTYPE_ERROR, message.AddressOf());
}

void asCBuilder::WriteWarning(const asCString &scriptname, const asCString &message, int r, int c)
{
	if( engine->ep.compilerWarnings )
	{
		numWarnings++;

		if( !silent )
			engine->WriteMessage(scriptname.AddressOf(), r, c, asMSGTYPE_WARNING, message.AddressOf());
	}
}

void asCBuilder::WriteWarning(const asCString &message, asCScriptCode *file, asCScriptNode *node)
{
	int r = 0, c = 0;
	if( node && file )
		file->ConvertPosToRowCol(node->tokenPos, &r, &c);

	WriteWarning(file ? file->name : asCString(""), message, r, c);
}

asCString asCBuilder::GetScopeFromNode(asCScriptNode *node, asCScriptCode *script, asCScriptNode **next)
{
	asCString scope;
	asCScriptNode *sn = node;
	if( sn->tokenType == ttScope )
	{
		scope = "::";
		sn = sn->next;
	}

	while( sn && sn->next && sn->next->tokenType == ttScope )
	{
		asCString tmp;
		tmp.Assign(&script->code[sn->tokenPos], sn->tokenLength);
		if( scope != "" && scope != "::" )
			scope += "::";
		scope += tmp;
		sn = sn->next->next;
	}

	if( next )
		*next = sn;

	return scope;
}

asSNameSpace *asCBuilder::GetNameSpaceFromNode(asCScriptNode *node, asCScriptCode *script, asSNameSpace *implicitNs, asCScriptNode **next)
{
	asCString scope = GetScopeFromNode(node, script, next);
	return GetNameSpaceByString(scope, implicitNs, node, script);
}

asSNameSpace *asCBuilder::GetNameSpaceByString(const asCString &nsName, asSNameSpace *implicitNs, asCScriptNode *errNode, asCScriptCode *script, bool isRequired)
{
	asSNameSpace *ns = implicitNs;
	if( nsName == "::" )
		ns = engine->nameSpaces[0];
	else if( nsName != "" )
	{
		ns = engine->FindNameSpace(nsName.AddressOf());
		if( ns == 0 && isRequired )
		{
			asCString msg;
			msg.Format(TXT_NAMESPACE_s_DOESNT_EXIST, nsName.AddressOf());
			WriteError(msg, script, errNode);
		}
	}

	return ns;
}

asCDataType asCBuilder::CreateDataTypeFromNode(asCScriptNode *node, asCScriptCode *file, asSNameSpace *implicitNamespace, bool acceptHandleForScope, asCObjectType *currentType)
{
	asASSERT(node->nodeType == snDataType);

	asCDataType dt;

	asCScriptNode *n = node->firstChild;

	bool isConst = false;
	bool isImplicitHandle = false;
	if( n->tokenType == ttConst )
	{
		isConst = true;
		n = n->next;
	}

	// Determine namespace
	asSNameSpace *ns = GetNameSpaceFromNode(n, file, implicitNamespace, &n);
	if( ns == 0 )
	{
		// The namespace doesn't exist. Return a dummy type instead.
		dt = asCDataType::CreatePrimitive(ttInt, false);
		return dt;
	}

	if( n->tokenType == ttIdentifier )
	{
		bool found = false;

		asCScriptNode *nameToken = n;
		asCString str;
		str.Assign(&file->code[n->tokenPos], n->tokenLength);

		// Recursively search parent namespaces for matching type
		asSNameSpace *origNs = ns;
		while( ns && !found )
		{
			asCObjectType *ot = 0;

			// If this is for a template type, then we must first determine if the
			// identifier matches any of the template subtypes
			if( currentType && (currentType->flags & asOBJ_TEMPLATE) )
			{
				for( asUINT subtypeIndex = 0; subtypeIndex < currentType->templateSubTypes.GetLength(); subtypeIndex++)
				{
					asCObjectType *type = currentType->templateSubTypes[subtypeIndex].GetObjectType();
					if( type && str == type->name )
					{
						ot = type;
						break;
					}
				}
			}

			if( ot == 0 )
				ot = GetObjectType(str.AddressOf(), ns);
			if( ot == 0 && !module && currentType )
				ot = GetObjectTypeFromTypesKnownByObject(str.AddressOf(), currentType);

			if( ot )
			{
				found = true;

				if( ot->flags & asOBJ_IMPLICIT_HANDLE )
					isImplicitHandle = true;

				// Make sure the module has access to the object type
				if( !module || (module->accessMask & ot->accessMask) )
				{
					if( asOBJ_TYPEDEF == (ot->flags & asOBJ_TYPEDEF) )
					{
						// TODO: typedef: A typedef should be considered different from the original type (though with implicit conversions between the two)
						// Create primitive data type based on object flags
						dt = ot->templateSubTypes[0];
						dt.MakeReadOnly(isConst);
					}
					else
					{
						if( ot->flags & asOBJ_TEMPLATE )
						{
							// Check if the subtype is a type or the template's subtype
							// if it is the template's subtype then this is the actual template type,
							// orderwise it is a template instance.
							// Only do this for application registered interface, as the
							// scripts cannot implement templates.
							asCArray<asCDataType> subTypes;
							asUINT subtypeIndex;
							while( n && n->next && n->next->nodeType == snDataType )
							{
								n = n->next;

								// When parsing function definitions for template registrations (currentType != 0) it is necessary 
								// to pass in the current template type to the recursive call since it is this ones sub-template types
								// that should be allowed.
								asCDataType subType = CreateDataTypeFromNode(n, file, implicitNamespace, false, module ? 0 : (currentType ? currentType : ot));
								subTypes.PushLast(subType);

								if( subType.IsReadOnly() )
								{
									asCString msg;
									msg.Format(TXT_TMPL_SUBTYPE_MUST_NOT_BE_READ_ONLY);
									WriteError(msg, file, n);

									// Return a dummy
									return asCDataType::CreatePrimitive(ttInt, false);
								}
							}

							if( subTypes.GetLength() != ot->templateSubTypes.GetLength() )
							{
								asCString msg;
								msg.Format(TXT_TMPL_s_EXPECTS_d_SUBTYPES, ot->name.AddressOf(), int(ot->templateSubTypes.GetLength()));
								WriteError(msg, file, nameToken);

								// Return a dummy
								return asCDataType::CreatePrimitive(ttInt, false);
							}

							// Check if any of the given subtypes are different from the template's declared subtypes
							bool isDifferent = false;
							for( subtypeIndex = 0; subtypeIndex < subTypes.GetLength(); subtypeIndex++ )
							{
								if( subTypes[subtypeIndex].GetObjectType() != ot->templateSubTypes[subtypeIndex].GetObjectType() )
								{
									isDifferent = true;
									break;
								}
							}

							if( isDifferent )
							{
								// This is a template instance
								// Need to find the correct object type
								asCObjectType *otInstance = engine->GetTemplateInstanceType(ot, subTypes, module);

								if( otInstance && otInstance->scriptSectionIdx < 0 )
								{
									// If this is the first time the template instance is used, store where it was declared from
									otInstance->scriptSectionIdx = engine->GetScriptSectionNameIndex(file->name.AddressOf());
									int row, column;
									file->ConvertPosToRowCol(n->tokenPos, &row, &column);
									otInstance->declaredAt = (row&0xFFFFF)|(column<<20);
								}

								if( !otInstance )
								{
									asCString sub = subTypes[0].Format(ot->nameSpace);
									for( asUINT s = 1; s < subTypes.GetLength(); s++ )
									{
										sub += ",";
										sub += subTypes[s].Format(ot->nameSpace);
									}
									asCString str;
									str.Format(TXT_INSTANCING_INVLD_TMPL_TYPE_s_s, ot->name.AddressOf(), sub.AddressOf());
									WriteError(str, file, n);
								}

								ot = otInstance;
							}
						}
						else if( n && n->next && n->next->nodeType == snDataType )
						{
							asCString str;
							str.Format(TXT_TYPE_s_NOT_TEMPLATE, ot->name.AddressOf());
							WriteError(str, file, n);
						}

						// Create object data type
						if( ot )
							dt = asCDataType::CreateObject(ot, isConst);
						else
							dt = asCDataType::CreatePrimitive(ttInt, isConst);
					}
				}
				else
				{
					asCString msg;
					msg.Format(TXT_TYPE_s_NOT_AVAILABLE_FOR_MODULE, (const char *)str.AddressOf());
					WriteError(msg, file, n);

					dt.SetTokenType(ttInt);
				}
			}
			else if( ot == 0 )
			{
				// It can still be a function definition
				asCScriptFunction *funcdef = GetFuncDef(str.AddressOf());

				if( funcdef )
				{
					dt = asCDataType::CreateFuncDef(funcdef);
					found = true;
				}
			}

			if( !found )
			{
				// Try to find it in the parent namespace
				ns = engine->GetParentNameSpace(ns);
			}
		}

		if( !found )
		{
			asCString msg;
			if( origNs->name == "" )
				msg.Format(TXT_IDENTIFIER_s_NOT_DATA_TYPE_IN_GLOBAL_NS, str.AddressOf());
			else
				msg.Format(TXT_IDENTIFIER_s_NOT_DATA_TYPE_IN_NS_s, str.AddressOf(), origNs->name.AddressOf());
			WriteError(msg, file, n);

			dt = asCDataType::CreatePrimitive(ttInt, isConst);
			return dt;
		}
	}
	else if( n->tokenType == ttAuto )
	{
		dt = asCDataType::CreateAuto(isConst);
	}
	else
	{
		// Create primitive data type
		dt = asCDataType::CreatePrimitive(n->tokenType, isConst);
	}

	// Determine array dimensions and object handles
	n = n->next;
	while( n && (n->tokenType == ttOpenBracket || n->tokenType == ttHandle) )
	{
		if( n->tokenType == ttOpenBracket )
		{
			// Make sure the sub type can be instantiated
			if( !dt.CanBeInstantiated() )
			{
				asCString str;
				if( dt.IsAbstractClass() )
					str.Format(TXT_ABSTRACT_CLASS_s_CANNOT_BE_INSTANTIATED, dt.Format(ns).AddressOf());
				else if( dt.IsInterface() )
					str.Format(TXT_INTERFACE_s_CANNOT_BE_INSTANTIATED, dt.Format(ns).AddressOf());
				else
					// TODO: Improve error message to explain why
					str.Format(TXT_DATA_TYPE_CANT_BE_s, dt.Format(ns).AddressOf());

				WriteError(str, file, n);
			}

			// Make the type an array (or multidimensional array)
			if( dt.MakeArray(engine, module) < 0 )
			{
				WriteError(TXT_NO_DEFAULT_ARRAY_TYPE, file, n);
				break;
			}
		}
		else
		{
			// Make the type a handle
			if( dt.IsObjectHandle() )
			{
				WriteError(TXT_HANDLE_OF_HANDLE_IS_NOT_ALLOWED, file, n);
				break;
			}
			else if( dt.MakeHandle(true, acceptHandleForScope) < 0 )
			{
				WriteError(TXT_OBJECT_HANDLE_NOT_SUPPORTED, file, n);
				break;
			}
		}
		n = n->next;
	}

	if( isImplicitHandle )
	{
		// Make the type a handle
		if( dt.MakeHandle(true, acceptHandleForScope) < 0 )
			WriteError(TXT_OBJECT_HANDLE_NOT_SUPPORTED, file, n);
	}

	return dt;
}

asCDataType asCBuilder::ModifyDataTypeFromNode(const asCDataType &type, asCScriptNode *node, asCScriptCode *file, asETypeModifiers *inOutFlags, bool *autoHandle)
{
	asCDataType dt = type;

	if( inOutFlags ) *inOutFlags = asTM_NONE;

	// Is the argument sent by reference?
	asCScriptNode *n = node->firstChild;
	if( n && n->tokenType == ttAmp )
	{
		dt.MakeReference(true);
		n = n->next;

		if( n )
		{
			if( inOutFlags )
			{
				if( n->tokenType == ttIn )
					*inOutFlags = asTM_INREF;
				else if( n->tokenType == ttOut )
					*inOutFlags = asTM_OUTREF;
				else if( n->tokenType == ttInOut )
					*inOutFlags = asTM_INOUTREF;
				else
					asASSERT(false);
			}

			n = n->next;
		}
		else
		{
			if( inOutFlags )
				*inOutFlags = asTM_INOUTREF; // ttInOut
		}

		if( !engine->ep.allowUnsafeReferences &&
			inOutFlags && *inOutFlags == asTM_INOUTREF )
		{
			// Verify that the base type support &inout parameter types
			if( !dt.IsObject() || dt.IsObjectHandle() || !((dt.GetObjectType()->flags & asOBJ_NOCOUNT) || (dt.GetObjectType()->beh.addref && dt.GetObjectType()->beh.release)) )
				WriteError(TXT_ONLY_OBJECTS_MAY_USE_REF_INOUT, file, node->firstChild);
		}
	}

	if( autoHandle ) *autoHandle = false;

	if( n && n->tokenType == ttPlus )
	{
		// Autohandles are not supported for types with NOCOUNT
		// If the type is not a handle then there was an error with building the type, but 
		// this error would already have been reported so no need to report another error here
		if( dt.IsObjectHandle() && (dt.GetObjectType()->flags & asOBJ_NOCOUNT) )
			WriteError(TXT_AUTOHANDLE_CANNOT_BE_USED_FOR_NOCOUNT, file, node->firstChild);

		if( autoHandle ) *autoHandle = true;
	}

	return dt;
}

asCObjectType *asCBuilder::GetObjectType(const char *type, asSNameSpace *ns)
{
	asCObjectType *ot = engine->GetRegisteredObjectType(type, ns);
	if( !ot && module )
		ot = module->GetObjectType(type, ns);

	return ot;
}

#ifndef AS_NO_COMPILER
// This function will return true if there are any types in the engine or module
// with the given name. The namespace is ignored in this verification.
bool asCBuilder::DoesTypeExist(const asCString &type)
{
	asUINT n;

	// This function is only used when parsing expressions for building bytecode
	// and this is only done after all types are known. For this reason the types
	// can be safely cached in a map for quick lookup. Once the builder is released
	// the cache will also be destroyed thus avoiding unnecessary memory consumption.
	if( !hasCachedKnownTypes )
	{
		// Only do this once
		hasCachedKnownTypes = true;

		// Add registered object types
		asSMapNode<asSNameSpaceNamePair, asCObjectType*> *cursor;
		engine->allRegisteredTypes.MoveFirst(&cursor);
		while( cursor )
		{
			if( !knownTypes.MoveTo(0, cursor->key.name) )
				knownTypes.Insert(cursor->key.name, true);

			engine->allRegisteredTypes.MoveNext(&cursor, cursor);
		}

		// Add registered funcdefs
		for( n = 0; n < engine->registeredFuncDefs.GetLength(); n++ )
			if( !knownTypes.MoveTo(0, engine->registeredFuncDefs[n]->name) )
				knownTypes.Insert(engine->registeredFuncDefs[n]->name, true);

		if( module )
		{
			// Add script classes and interfaces
			for( n = 0; n < module->classTypes.GetLength(); n++ )
				if( !knownTypes.MoveTo(0, module->classTypes[n]->name) )
					knownTypes.Insert(module->classTypes[n]->name, true);

			// Add script enums
			for( n = 0; n < module->enumTypes.GetLength(); n++ )
				if( !knownTypes.MoveTo(0, module->enumTypes[n]->name) )
					knownTypes.Insert(module->enumTypes[n]->name, true);

			// Add script typedefs
			for( n = 0; n < module->typeDefs.GetLength(); n++ )
				if( !knownTypes.MoveTo(0, module->typeDefs[n]->name) )
					knownTypes.Insert(module->typeDefs[n]->name, true);

			// Add script funcdefs
			for( n = 0; n < module->funcDefs.GetLength(); n++ )
				if( !knownTypes.MoveTo(0, module->funcDefs[n]->name) )
					knownTypes.Insert(module->funcDefs[n]->name, true);
		}
	}

	// Check if the type is known
	return knownTypes.MoveTo(0, type);
}
#endif

asCObjectType *asCBuilder::GetObjectTypeFromTypesKnownByObject(const char *type, asCObjectType *currentType)
{
	if( currentType->name == type )
		return currentType;

	asUINT n;

	asCObjectType *found = 0;

	for( n = 0; found == 0 && n < currentType->properties.GetLength(); n++ )
		if( currentType->properties[n]->type.GetObjectType() &&
			currentType->properties[n]->type.GetObjectType()->name == type )
			found = currentType->properties[n]->type.GetObjectType();

	for( n = 0; found == 0 && n < currentType->methods.GetLength(); n++ )
	{
		asCScriptFunction *func = engine->scriptFunctions[currentType->methods[n]];
		if( func->returnType.GetObjectType() &&
			func->returnType.GetObjectType()->name == type )
			found = func->returnType.GetObjectType();
		
		for( asUINT f = 0; found == 0 && f < func->parameterTypes.GetLength(); f++ )
			if( func->parameterTypes[f].GetObjectType() &&
				func->parameterTypes[f].GetObjectType()->name == type )
				found = func->parameterTypes[f].GetObjectType();
	}

	if( found )
	{
		// In case we find a template instance it mustn't be returned
		// because it is not known if the subtype is really matching
		if( found->flags & asOBJ_TEMPLATE )
			return 0;
	}

	return found;
}

asCScriptFunction *asCBuilder::GetFuncDef(const char *type)
{
	for( asUINT n = 0; n < engine->registeredFuncDefs.GetLength(); n++ )
		// TODO: access: Only return the definitions that the module has access to
		if( engine->registeredFuncDefs[n]->name == type )
			return engine->registeredFuncDefs[n];

	if( module )
	{
		for( asUINT n = 0; n < module->funcDefs.GetLength(); n++ )
			if( module->funcDefs[n]->name == type )
				return module->funcDefs[n];
	}

	return 0;
}

#ifndef AS_NO_COMPILER

int asCBuilder::GetEnumValueFromObjectType(asCObjectType *objType, const char *name, asCDataType &outDt, asDWORD &outValue)
{
	if( !objType || !(objType->flags & asOBJ_ENUM) )
		return 0;

	for( asUINT n = 0; n < objType->enumValues.GetLength(); ++n )
	{
		if( objType->enumValues[n]->name == name )
		{
			outDt = asCDataType::CreateObject(objType, true);
			outValue = objType->enumValues[n]->value;
			return 1;
		}
	}

	return 0;
}

int asCBuilder::GetEnumValue(const char *name, asCDataType &outDt, asDWORD &outValue, asSNameSpace *ns)
{
	bool found = false;

	// Search all available enum types
	asUINT t;
	for( t = 0; t < engine->registeredEnums.GetLength(); t++ )
	{
		asCObjectType *ot = engine->registeredEnums[t];
		if( ns != ot->nameSpace ) continue;

		// Don't bother with types the module doesn't have access to
		if( (ot->accessMask & module->accessMask) == 0 )
			continue;

		if( GetEnumValueFromObjectType(ot, name, outDt, outValue) )
		{
			if( !found )
				found = true;
			else
			{
				// Found more than one value in different enum types
				return 2;
			}
		}
	}

	for( t = 0; t < module->enumTypes.GetLength(); t++ )
	{
		asCObjectType *ot = module->enumTypes[t];
		if( ns != ot->nameSpace ) continue;

		if( GetEnumValueFromObjectType(ot, name, outDt, outValue) )
		{
			if( !found )
				found = true;
			else
			{
				// Found more than one value in different enum types
				return 2;
			}
		}
	}

	if( found )
		return 1;

	// Didn't find any value
	return 0;
}

#endif // AS_NO_COMPILER

END_AS_NAMESPACE
