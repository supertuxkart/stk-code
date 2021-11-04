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
// as_scriptengine.cpp
//
// The implementation of the script engine interface
//


#include <stdlib.h>

#include "as_config.h"
#include "as_scriptengine.h"
#include "as_builder.h"
#include "as_context.h"
#include "as_string_util.h"
#include "as_tokenizer.h"
#include "as_texts.h"
#include "as_module.h"
#include "as_callfunc.h"
#include "as_generic.h"
#include "as_scriptobject.h"
#include "as_compiler.h"
#include "as_bytecode.h"
#include "as_debug.h"

BEGIN_AS_NAMESPACE



#ifdef AS_PROFILE
// Instantiate the profiler once
CProfiler g_profiler;
#endif




extern "C"
{

AS_API const char * asGetLibraryVersion()
{
#ifdef _DEBUG
	return ANGELSCRIPT_VERSION_STRING " DEBUG";
#else
	return ANGELSCRIPT_VERSION_STRING;
#endif
}

AS_API const char * asGetLibraryOptions()
{
	const char *string = " "

	// Options
#ifdef AS_MAX_PORTABILITY
		"AS_MAX_PORTABILITY "
#endif
#ifdef AS_DEBUG
		"AS_DEBUG "
#endif
#ifdef AS_NO_CLASS_METHODS
		"AS_NO_CLASS_METHODS "
#endif
#ifdef AS_USE_DOUBLE_AS_FLOAT
		"AS_USE_DOUBLE_AS_FLOAT "
#endif
#ifdef AS_64BIT_PTR
		"AS_64BIT_PTR "
#endif
#ifdef AS_NO_THREADS
		"AS_NO_THREADS "
#endif
#ifdef AS_NO_ATOMIC
		"AS_NO_ATOMIC "
#endif
#ifdef AS_NO_COMPILER
		"AS_NO_COMPILER "
#endif
#ifdef AS_NO_MEMBER_INIT
		"AS_NO_MEMBER_INIT "
#endif
#ifdef AS_NO_THISCALL_FUNCTOR_METHOD
		"AS_NO_THISCALL_FUNCTOR_METHOD "
#endif
#ifdef AS_NO_EXCEPTIONS
		"AS_NO_EXCEPTIONS "
#endif
#ifdef WIP_16BYTE_ALIGN
		"WIP_16BYTE_ALIGN "
#endif
#ifdef AS_BIG_ENDIAN
		"AS_BIG_ENDIAN "
#endif

	// Target system
#ifdef AS_WIN
		"AS_WIN "
#endif
#ifdef AS_LINUX
		"AS_LINUX "
#endif
#ifdef AS_MAC
		"AS_MAC "
#endif
#ifdef AS_SUN
		"AS_SUN "
#endif
#ifdef AS_BSD
		"AS_BSD "
#endif
#ifdef AS_XBOX
		"AS_XBOX "
#endif
#ifdef AS_XBOX360
		"AS_XBOX360 "
#endif
#ifdef AS_PSP
		"AS_PSP "
#endif
#ifdef AS_PS2
		"AS_PS2 "
#endif
#ifdef AS_PS3
		"AS_PS3 "
#endif
#ifdef AS_PSVITA
		"AS_PSVITA "
#endif
#ifdef AS_DC
		"AS_DC "
#endif
#ifdef AS_GC
		"AS_GC "
#endif
#ifdef AS_WII
		"AS_WII "
#endif
#ifdef AS_WIIU
		"AS_WIIU "
#endif
#ifdef AS_IPHONE
		"AS_IPHONE "
#endif
#ifdef AS_ANDROID
		"AS_ANDROID "
#endif
#ifdef AS_HAIKU
		"AS_HAIKU "
#endif
#ifdef AS_ILLUMOS
		"AS_ILLUMOS "
#endif
#ifdef AS_MARMALADE
		"AS_MARMALADE "
#endif


	// CPU family
#ifdef AS_PPC
		"AS_PPC "
#endif
#ifdef AS_PPC_64
		"AS_PPC_64 "
#endif
#ifdef AS_X86
		"AS_X86 "
#endif
#ifdef AS_MIPS
		"AS_MIPS "
#endif
#ifdef AS_SH4
		"AS_SH4 "
#endif
#ifdef AS_XENON
		"AS_XENON "
#endif
#ifdef AS_ARM
		"AS_ARM "
#endif
#ifdef AS_SOFTFP
		"AS_SOFTFP "
#endif
#ifdef AS_X64_GCC
		"AS_X64_GCC "
#endif
#ifdef AS_X64_MSVC
		"AS_X64_MSVC "
#endif
#ifdef AS_SPARC
		"AS_SPARC "
#endif
#ifdef AS_ARM64
		"AS_ARM64 "
#endif
	;

	return string;
}

AS_API asIScriptEngine *asCreateScriptEngine(asDWORD version)
{
	// Verify the version that the application expects
	if( (version/10000) != (ANGELSCRIPT_VERSION/10000) )
		return 0;

	if( (version/100)%100 != (ANGELSCRIPT_VERSION/100)%100 )
		return 0;

	if( (version%100) > (ANGELSCRIPT_VERSION%100) )
		return 0;

	// Verify the size of the types
	asASSERT( sizeof(asBYTE)  == 1 );
	asASSERT( sizeof(asWORD)  == 2 );
	asASSERT( sizeof(asDWORD) == 4 );
	asASSERT( sizeof(asQWORD) == 8 );
	asASSERT( sizeof(asPWORD) == sizeof(void*) );

	// Verify the boolean type
	asASSERT( sizeof(bool) == AS_SIZEOF_BOOL );
	asASSERT( true == VALUE_OF_BOOLEAN_TRUE );

	// Verify endianess
#ifdef AS_BIG_ENDIAN
	asDWORD dw = 0x00010203;
	asQWORD qw = ((asQWORD(0x00010203)<<32)|asQWORD(0x04050607));
#else
	asDWORD dw = 0x03020100;
	// C++ didn't have a standard way of declaring 64bit literal constants until C++11, so
	// I'm forced to do it like this to avoid compilers warnings when compiling with the full
	// C++ compliance.
	asQWORD qw = ((asQWORD(0x07060504)<<32)|asQWORD(0x03020100));
#endif
	asASSERT( memcmp("\x00\x01\x02\x03", &dw, 4) == 0 );
	asASSERT( memcmp("\x00\x01\x02\x03\x04\x05\x06\x07", &qw, 8) == 0 );
	UNUSED_VAR(dw);
	UNUSED_VAR(qw);

	return asNEW(asCScriptEngine)();
}

} // extern "C"


// interface
int asCScriptEngine::SetEngineProperty(asEEngineProp property, asPWORD value)
{
	switch( property )
	{
	case asEP_ALLOW_UNSAFE_REFERENCES:
		ep.allowUnsafeReferences = value ? true : false;
		break;

	case asEP_OPTIMIZE_BYTECODE:
		ep.optimizeByteCode = value ? true : false;
		break;

	case asEP_COPY_SCRIPT_SECTIONS:
		ep.copyScriptSections = value ? true : false;
		break;

	case asEP_MAX_STACK_SIZE:
		if( value == 0 )
		{
			// Restore default: no limit and initially size 4KB
			ep.maximumContextStackSize = 0;
		}
		else
		{
			// The size is given in bytes, but we only store dwords
			ep.maximumContextStackSize = (asUINT)value/4;
		}
		break;

	case asEP_INIT_STACK_SIZE:
		if (value < 4)
		{
			// At least one dword
			ep.initContextStackSize = 1;
		}
		else
		{
			// The size is given in bytes, but we only store dwords
			ep.initContextStackSize = (asUINT)value / 4;
		}
		break;

	case asEP_USE_CHARACTER_LITERALS:
		ep.useCharacterLiterals = value ? true : false;
		break;

	case asEP_ALLOW_MULTILINE_STRINGS:
		ep.allowMultilineStrings = value ? true : false;
		break;

	case asEP_ALLOW_IMPLICIT_HANDLE_TYPES:
		ep.allowImplicitHandleTypes = value ? true : false;
		break;

	case asEP_BUILD_WITHOUT_LINE_CUES:
		ep.buildWithoutLineCues = value ? true : false;
		break;

	case asEP_INIT_GLOBAL_VARS_AFTER_BUILD:
		ep.initGlobalVarsAfterBuild = value ? true : false;
		break;

	case asEP_REQUIRE_ENUM_SCOPE:
		ep.requireEnumScope = value ? true : false;
		break;

	case asEP_SCRIPT_SCANNER:
		if( value <= 1 )
			ep.scanner = (int)value;
		else
			return asINVALID_ARG;
		break;

	case asEP_INCLUDE_JIT_INSTRUCTIONS:
		ep.includeJitInstructions = value ? true : false;
		break;

	case asEP_STRING_ENCODING:
		if( value <= 1 )
			ep.stringEncoding = (int)value;
		else
			return asINVALID_ARG;
		break;

	case asEP_PROPERTY_ACCESSOR_MODE:
		if( value <= 3 )
			ep.propertyAccessorMode = (int)value;
		else
			return asINVALID_ARG;
		break;

	case asEP_EXPAND_DEF_ARRAY_TO_TMPL:
		ep.expandDefaultArrayToTemplate = value ? true : false;
		break;

	case asEP_AUTO_GARBAGE_COLLECT:
		ep.autoGarbageCollect = value ? true : false;
		break;

	case asEP_DISALLOW_GLOBAL_VARS:
		ep.disallowGlobalVars = value ? true : false;
		break;

	case asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT:
		ep.alwaysImplDefaultConstruct = value ? true : false;
		break;

	case asEP_COMPILER_WARNINGS:
		if( value <= 2 )
			ep.compilerWarnings = (int)value;
		else
			return asINVALID_ARG;
		break;

	case asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE:
		ep.disallowValueAssignForRefType = value ? true : false;
		break;

	case asEP_ALTER_SYNTAX_NAMED_ARGS:
		if( value <= 2 )
			ep.alterSyntaxNamedArgs = (int)value;
		else
			return asINVALID_ARG;
		break;

	case asEP_DISABLE_INTEGER_DIVISION:
		ep.disableIntegerDivision = value ? true : false;
		break;

	case asEP_DISALLOW_EMPTY_LIST_ELEMENTS:
		ep.disallowEmptyListElements = value ? true : false;
		break;

	case asEP_PRIVATE_PROP_AS_PROTECTED:
		ep.privatePropAsProtected = value ? true : false;
		break;

	case asEP_ALLOW_UNICODE_IDENTIFIERS:
		ep.allowUnicodeIdentifiers = value ? true : false;
		break;

	case asEP_HEREDOC_TRIM_MODE:
		if (value <= 2)
			ep.heredocTrimMode = (int)value;
		else
			return asINVALID_ARG;
		break;

	case asEP_MAX_NESTED_CALLS:
		if (value > 0xFFFFFFFF)
			ep.maxNestedCalls = 0xFFFFFFFF;
		else
			ep.maxNestedCalls = (asUINT)value;
		break;

	case asEP_GENERIC_CALL_MODE:
		if (value > 1)
			ep.genericCallMode = 1;
		else
			ep.genericCallMode = (asUINT)value;
		break;

	case asEP_INIT_CALL_STACK_SIZE:
		ep.initCallStackSize = (asUINT)value;
		break;

	case asEP_MAX_CALL_STACK_SIZE:
		ep.maxCallStackSize = (asUINT)value;
		break;

	default:
		return asINVALID_ARG;
	}

	return asSUCCESS;
}

// interface
asPWORD asCScriptEngine::GetEngineProperty(asEEngineProp property) const
{
	switch( property )
	{
	case asEP_ALLOW_UNSAFE_REFERENCES:
		return ep.allowUnsafeReferences;

	case asEP_OPTIMIZE_BYTECODE:
		return ep.optimizeByteCode;

	case asEP_COPY_SCRIPT_SECTIONS:
		return ep.copyScriptSections;

	case asEP_MAX_STACK_SIZE:
		return ep.maximumContextStackSize * 4;

	case asEP_INIT_STACK_SIZE:
		return ep.initContextStackSize * 4;

	case asEP_USE_CHARACTER_LITERALS:
		return ep.useCharacterLiterals;

	case asEP_ALLOW_MULTILINE_STRINGS:
		return ep.allowMultilineStrings;

	case asEP_ALLOW_IMPLICIT_HANDLE_TYPES:
		return ep.allowImplicitHandleTypes;

	case asEP_BUILD_WITHOUT_LINE_CUES:
		return ep.buildWithoutLineCues;

	case asEP_INIT_GLOBAL_VARS_AFTER_BUILD:
		return ep.initGlobalVarsAfterBuild;

	case asEP_REQUIRE_ENUM_SCOPE:
		return ep.requireEnumScope;

	case asEP_SCRIPT_SCANNER:
		return ep.scanner;

	case asEP_INCLUDE_JIT_INSTRUCTIONS:
		return ep.includeJitInstructions;

	case asEP_STRING_ENCODING:
		return ep.stringEncoding;

	case asEP_PROPERTY_ACCESSOR_MODE:
		return ep.propertyAccessorMode;

	case asEP_EXPAND_DEF_ARRAY_TO_TMPL:
		return ep.expandDefaultArrayToTemplate;

	case asEP_AUTO_GARBAGE_COLLECT:
		return ep.autoGarbageCollect;

	case asEP_DISALLOW_GLOBAL_VARS:
		return ep.disallowGlobalVars;

	case asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT:
		return ep.alwaysImplDefaultConstruct;

	case asEP_COMPILER_WARNINGS:
		return ep.compilerWarnings;

	case asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE:
		return ep.disallowValueAssignForRefType;

	case asEP_ALTER_SYNTAX_NAMED_ARGS:
		return ep.alterSyntaxNamedArgs;

	case asEP_DISABLE_INTEGER_DIVISION:
		return ep.disableIntegerDivision;

	case asEP_DISALLOW_EMPTY_LIST_ELEMENTS:
		return ep.disallowEmptyListElements;

	case asEP_PRIVATE_PROP_AS_PROTECTED:
		return ep.privatePropAsProtected;

	case asEP_ALLOW_UNICODE_IDENTIFIERS:
		return ep.allowUnicodeIdentifiers;

	case asEP_HEREDOC_TRIM_MODE:
		return ep.heredocTrimMode;

	case asEP_MAX_NESTED_CALLS:
		return ep.maxNestedCalls;

	case asEP_GENERIC_CALL_MODE:
		return ep.genericCallMode;

	case asEP_INIT_CALL_STACK_SIZE:
		return ep.initCallStackSize;

	case asEP_MAX_CALL_STACK_SIZE:
		return ep.maxCallStackSize;

	default:
		return 0;
	}

	UNREACHABLE_RETURN;
}

// interface
asIScriptFunction *asCScriptEngine::CreateDelegate(asIScriptFunction *func, void *obj)
{
	if( func == 0 || obj == 0 )
		return 0;

	// The function must be a class method
	asITypeInfo *type = func->GetObjectType();
	if( type == 0 )
		return 0;

	// The object type must allow handles
	if( (type->GetFlags() & asOBJ_REF) == 0 || (type->GetFlags() & (asOBJ_SCOPED | asOBJ_NOHANDLE)) )
		return 0;

	// Create the delegate the same way it would be created by the scripts
	return AS_NAMESPACE_QUALIFIER CreateDelegate(reinterpret_cast<asCScriptFunction*>(func), obj);
}

asCScriptEngine::asCScriptEngine()
{
	asCThreadManager::Prepare(0);

	shuttingDown = false;
	inDestructor = false;

	// Engine properties
	{
		ep.allowUnsafeReferences         = false;
		ep.optimizeByteCode              = true;
		ep.copyScriptSections            = true;
		ep.maximumContextStackSize       = 0;         // no limit
		ep.initContextStackSize          = 1024;      // 4KB default init stack size
		ep.useCharacterLiterals          = false;
		ep.allowMultilineStrings         = false;
		ep.allowImplicitHandleTypes      = false;
		// TODO: optimize: Maybe this should be turned off by default? If a debugger is not used
		//                 then this is just slowing down the execution.
		ep.buildWithoutLineCues          = false;
		ep.initGlobalVarsAfterBuild      = true;
		ep.requireEnumScope              = false;
		ep.scanner                       = 1;         // utf8. 0 = ascii
		ep.includeJitInstructions        = false;
		ep.stringEncoding                = 0;         // utf8. 1 = utf16
		ep.propertyAccessorMode          = 3;         // 0 = disable, 1 = app registered only, 2 = app and script created, 3 = flag with 'property'
		ep.expandDefaultArrayToTemplate  = false;
		ep.autoGarbageCollect            = true;
		ep.disallowGlobalVars            = false;
		ep.alwaysImplDefaultConstruct    = false;
		ep.compilerWarnings              = 1;         // 0 = no warnings, 1 = warning, 2 = treat as error
		// TODO: 3.0.0: disallowValueAssignForRefType should be true by default
		ep.disallowValueAssignForRefType = false;
		ep.alterSyntaxNamedArgs          = 0;         // 0 = no alternate syntax, 1 = accept alternate syntax but warn, 2 = accept without warning
		ep.disableIntegerDivision        = false;
		ep.disallowEmptyListElements     = false;
		ep.privatePropAsProtected        = false;
		ep.allowUnicodeIdentifiers       = false;
		ep.heredocTrimMode               = 1;         // 0 = never trim, 1 = don't trim on single line, 2 = trim initial and final empty line
		ep.maxNestedCalls                = 100;
		ep.genericCallMode               = 1;         // 0 = old (pre 2.33.0) behavior where generic ignored auto handles, 1 = treat handles like in native call
		ep.initCallStackSize             = 10;        // 10 levels of calls
		ep.maxCallStackSize              = 0;         // 0 = no limit
	}

	gc.engine = this;
	tok.engine = this;

	refCount.set(1);
	stringFactory = 0;
	configFailed = false;
	isPrepared = false;
	isBuilding = false;
	deferValidationOfTemplateTypes = false;
	lastModule = 0;

	typeIdSeqNbr      = 0;
	currentGroup      = &defaultGroup;
	defaultAccessMask = 0xFFFFFFFF; // All bits set so that built-in functions/types will be available to all modules

	msgCallback = 0;
	jitCompiler = 0;

	// Create the global namespace
	defaultNamespace = AddNameSpace("");

	requestCtxFunc   = 0;
	returnCtxFunc    = 0;
	ctxCallbackParam = 0;

	// We must set the namespace in the built-in types explicitly as
	// this wasn't done by the default constructor. If we do not do
	// this we will get null pointer access in other parts of the code
	scriptTypeBehaviours.nameSpace     = defaultNamespace;
	functionBehaviours.nameSpace       = defaultNamespace;

	// Reserve function id 0 for no function
	scriptFunctions.PushLast(0);

	// Reserve the first typeIds for the primitive types
	typeIdSeqNbr = asTYPEID_DOUBLE + 1;

	// Make sure typeId for the built-in primitives are defined according to asETypeIdFlags
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttVoid,   false)) == asTYPEID_VOID   );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttBool,   false)) == asTYPEID_BOOL   );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttInt8,   false)) == asTYPEID_INT8   );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttInt16,  false)) == asTYPEID_INT16  );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttInt,    false)) == asTYPEID_INT32  );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttInt64,  false)) == asTYPEID_INT64  );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttUInt8,  false)) == asTYPEID_UINT8  );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttUInt16, false)) == asTYPEID_UINT16 );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttUInt,   false)) == asTYPEID_UINT32 );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttUInt64, false)) == asTYPEID_UINT64 );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttFloat,  false)) == asTYPEID_FLOAT  );
	asASSERT( GetTypeIdFromDataType(asCDataType::CreatePrimitive(ttDouble, false)) == asTYPEID_DOUBLE );

	defaultArrayObjectType = 0;

	RegisterScriptObject(this);
	RegisterScriptFunction(this);

#ifndef AS_NO_EXCEPTIONS
	translateExceptionCallback = false;
#endif
}

void asCScriptEngine::DeleteDiscardedModules()
{
	// TODO: redesign: Prevent more than one thread from entering this function at the same time.
	//                 If a thread is already doing the work for the clean-up the other thread should
	//                 simply return, as the first thread will continue.

	ACQUIRESHARED(engineRWLock);
	asUINT maxCount = discardedModules.GetLength();
	RELEASESHARED(engineRWLock);

	for( asUINT n = 0; n < maxCount; n++ )
	{
		ACQUIRESHARED(engineRWLock);
		asCModule *mod = discardedModules[n];
		RELEASESHARED(engineRWLock);

		if( !mod->HasExternalReferences(shuttingDown) )
		{
			asDELETE(mod, asCModule);
			n--;
		}

		ACQUIRESHARED(engineRWLock);
		// Determine the max count again, since another module may have been discarded during the processing
		maxCount = discardedModules.GetLength();
		RELEASESHARED(engineRWLock);
	}

	// Go over the list of global properties, to see if it is possible to clean
	// up some variables that are no longer referred to by any functions
	for( asUINT n = 0; n < globalProperties.GetLength(); n++ )
	{
		asCGlobalProperty *prop = globalProperties[n];
		if( prop && prop->refCount.get() == 1 )
			RemoveGlobalProperty(prop);
	}
}

asCScriptEngine::~asCScriptEngine()
{
	// TODO: clean-up: Clean up redundant code

	inDestructor = true;

	asASSERT(refCount.get() == 0);

	// If ShutDown hasn't been called yet do it now
	if( !shuttingDown )
	{
		AddRef();
		ShutDownAndRelease();
	}

	// Unravel the registered interface
	if( defaultArrayObjectType )
	{
		defaultArrayObjectType->ReleaseInternal();
		defaultArrayObjectType = 0;
	}

	// Delete the functions for generated template types that may references object types
	for( asUINT n = 0; n < generatedTemplateTypes.GetLength(); n++ )
	{
		asCObjectType *templateType = generatedTemplateTypes[n];
		if( templateType )
			templateType->DestroyInternal();
	}
	for( asUINT n = 0; n < listPatternTypes.GetLength(); n++ )
	{
		asCObjectType *type = listPatternTypes[n];
		if( type )
			type->ReleaseInternal();
	}
	listPatternTypes.SetLength(0);

	// No script types must have survived
	asASSERT( sharedScriptTypes.GetLength() == 0 );

	// It is allowed to create new references to the engine temporarily while destroying objects
	// but these references must be release immediately or else something is can go wrong later on
	if( refCount.get() > 0 )
		WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ENGINE_REF_COUNT_ERROR_DURING_SHUTDOWN);

	mapTypeIdToTypeInfo.EraseAll();

	// First remove what is not used, so that other groups can be deleted safely
	defaultGroup.RemoveConfiguration(this, true);
	while( configGroups.GetLength() )
	{
		// Delete config groups in the right order
		asCConfigGroup *grp = configGroups.PopLast();
		if( grp )
		{
			grp->RemoveConfiguration(this);
			asDELETE(grp,asCConfigGroup);
		}
	}
	// Remove what is remaining
	defaultGroup.RemoveConfiguration(this);

	// Any remaining objects in templateInstanceTypes is from generated template instances
	for( asUINT n = 0; n < templateInstanceTypes.GetLength(); n++ )
	{
		asCObjectType *templateType = templateInstanceTypes[n];
		if( templateInstanceTypes[n] )
			templateType->ReleaseInternal();
	}
	templateInstanceTypes.SetLength(0);

	asCSymbolTable<asCGlobalProperty>::iterator it = registeredGlobalProps.List();
	for( ; it; it++ )
	{
		RemoveGlobalProperty(*it);
		(*it)->Release();
	}
	registeredGlobalProps.Clear();

	for( asUINT n = 0; n < templateSubTypes.GetLength(); n++ )
	{
		if( templateSubTypes[n] )
		{
			templateSubTypes[n]->DestroyInternal();
			templateSubTypes[n]->ReleaseInternal();
		}
	}
	templateSubTypes.SetLength(0);
	registeredTypeDefs.SetLength(0);
	registeredEnums.SetLength(0);
	registeredObjTypes.SetLength(0);

	asCSymbolTable<asCScriptFunction>::iterator funcIt = registeredGlobalFuncs.List();
	for( ; funcIt; funcIt++ )
		(*funcIt)->ReleaseInternal();
	registeredGlobalFuncs.Clear();

	scriptTypeBehaviours.ReleaseAllFunctions();
	functionBehaviours.ReleaseAllFunctions();

	for( asUINT n = 0; n < scriptFunctions.GetLength(); n++ )
		if( scriptFunctions[n] )
		{
			scriptFunctions[n]->DestroyInternal();

			// Set the engine pointer to null to signal that the function is no longer part of the engine
			scriptFunctions[n]->engine = 0;
		}
	scriptFunctions.SetLength(0);

	// Increase the internal ref count for these builtin object types, so the destructor is not called incorrectly
	scriptTypeBehaviours.AddRefInternal();
	functionBehaviours.AddRefInternal();

	// Destroy the funcdefs
	// As funcdefs are shared between modules it shouldn't be a problem to keep the objects until the engine is released
	for( asUINT n = 0; n < funcDefs.GetLength(); n++ )
		if( funcDefs[n] )
		{
			funcDefs[n]->DestroyInternal();
			funcDefs[n]->ReleaseInternal();
		}
	funcDefs.SetLength(0);

	// Free the global properties
	for( asUINT n = 0; n < globalProperties.GetLength(); n++ )
	{
		asCGlobalProperty *prop = globalProperties[n];
		if( prop )
		{
			asASSERT( prop->refCount.get() == 1 );
			RemoveGlobalProperty(prop);
		}
	}

	// Free the script section names
	for( asUINT n = 0; n < scriptSectionNames.GetLength(); n++ )
		asDELETE(scriptSectionNames[n],asCString);
	scriptSectionNames.SetLength(0);

	// Clean the user data
	for( asUINT n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n+1] )
		{
			for( asUINT c = 0; c < cleanEngineFuncs.GetLength(); c++ )
				if( cleanEngineFuncs[c].type == userData[n] )
					cleanEngineFuncs[c].cleanFunc(this);
		}
	}

	// Free namespaces
	for( asUINT n = 0; n < nameSpaces.GetLength(); n++ )
		asDELETE(nameSpaces[n], asSNameSpace);
	nameSpaces.SetLength(0);

	asCThreadManager::Unprepare();
}

// interface
int asCScriptEngine::SetContextCallbacks(asREQUESTCONTEXTFUNC_t requestCtx, asRETURNCONTEXTFUNC_t returnCtx, void *param)
{
	// Both callbacks or neither must be set
	if( (requestCtx == 0 && returnCtx != 0) || (requestCtx != 0 && returnCtx == 0) )
		return asINVALID_ARG;

	requestCtxFunc   = requestCtx;
	returnCtxFunc    = returnCtx;
	ctxCallbackParam = param;

	return 0;
}

// interface
asIScriptContext *asCScriptEngine::RequestContext()
{
	if( requestCtxFunc )
	{
		// The return callback must also exist
		asASSERT( returnCtxFunc );

		asIScriptContext *ctx = requestCtxFunc(this, ctxCallbackParam);
		return ctx;
	}

	// As fallback we create a new context
	return CreateContext();
}

// internal
asCModule *asCScriptEngine::FindNewOwnerForSharedType(asCTypeInfo *in_type, asCModule *in_mod)
{
	asASSERT( in_type->IsShared() );

	if( in_type->module != in_mod)
		return in_type->module;

	for( asUINT n = 0; n < scriptModules.GetLength(); n++ )
	{
		// TODO: optimize: If the modules already stored the shared types separately, this would be quicker
		int foundIdx = -1;
		asCModule *mod = scriptModules[n];
		if( mod == in_type->module ) continue;
		if( in_type->flags & asOBJ_ENUM )
			foundIdx = mod->m_enumTypes.IndexOf(CastToEnumType(in_type));
		else if (in_type->flags & asOBJ_TYPEDEF)
			foundIdx = mod->m_typeDefs.IndexOf(CastToTypedefType(in_type));
		else if (in_type->flags & asOBJ_FUNCDEF)
			foundIdx = mod->m_funcDefs.IndexOf(CastToFuncdefType(in_type));
		else if (in_type->flags & asOBJ_TEMPLATE)
			foundIdx = mod->m_templateInstances.IndexOf(CastToObjectType(in_type));
		else
			foundIdx = mod->m_classTypes.IndexOf(CastToObjectType(in_type));

		if( foundIdx >= 0 )
		{
			in_type->module = mod;
			break;
		}
	}

	return in_type->module;
}

// internal
asCModule *asCScriptEngine::FindNewOwnerForSharedFunc(asCScriptFunction *in_func, asCModule *in_mod)
{
	asASSERT( in_func->IsShared() );
	asASSERT(!(in_func->funcType & asFUNC_FUNCDEF));

	if( in_func->module != in_mod)
		return in_func->module;

	// Check if this is a class method or class factory for a type that has already been moved to a different module
	if ((in_func->objectType && in_func->objectType->module && in_func->objectType->module != in_func->module) ||
		(in_func->IsFactory() && in_func->returnType.GetTypeInfo()->module && in_func->returnType.GetTypeInfo()->module != in_func->module))
	{
		// The object type for the method has already been transferred to 
		// another module, so transfer the method to the same module
		if (in_func->objectType)
			in_func->module = in_func->objectType->module;
		else
			in_func->module = in_func->returnType.GetTypeInfo()->module;

		// Make sure the function is listed in the module
		// The compiler may not have done this earlier, since the object
		// type is shared and originally compiled from another module
		if (in_func->module->m_scriptFunctions.IndexOf(in_func) < 0)
		{
			in_func->module->m_scriptFunctions.PushLast(in_func);
			in_func->AddRefInternal();
		}
	}

	for( asUINT n = 0; n < scriptModules.GetLength(); n++ )
	{
		// TODO: optimize: If the modules already stored the shared types separately, this would be quicker
		int foundIdx = -1;
		asCModule *mod = scriptModules[n];
		if( mod == in_func->module ) continue;
		foundIdx = mod->m_scriptFunctions.IndexOf(in_func);

		if( foundIdx >= 0 )
		{
			in_func->module = mod;
			break;
		}
	}

	return in_func->module;
}

// interface
void asCScriptEngine::ReturnContext(asIScriptContext *ctx)
{
	if( returnCtxFunc )
	{
		returnCtxFunc(this, ctx, ctxCallbackParam);
		return;
	}

	// As fallback we just release the context
	if( ctx )
		ctx->Release();
}

// interface
int asCScriptEngine::AddRef() const
{
	asASSERT( refCount.get() > 0 || inDestructor );
	return refCount.atomicInc();
}

// interface
int asCScriptEngine::Release() const
{
	int r = refCount.atomicDec();

	if( r == 0 )
	{
		// It is possible that some function will temporarily increment the engine ref count
		// during clean-up for example while destroying the objects in the garbage collector.
		if( !inDestructor )
			asDELETE(const_cast<asCScriptEngine*>(this),asCScriptEngine);
		return 0;
	}

	return r;
}

// interface
int asCScriptEngine::ShutDownAndRelease()
{
	// Do a full garbage collection cycle to clean up any object that may still hold on to the engine
	GarbageCollect();

	// Set the flag that the engine is being shutdown now. This will speed up
	// the process, and will also allow the engine to warn about invalid calls
	shuttingDown = true;

	// Clear the context callbacks. If new context's are needed for the clean-up the engine will take care of this itself.
	// Context callbacks are normally used for pooling contexts, and if we allow new contexts to be created without being
	// immediately destroyed afterwards it means the engine's refcount will increase. This is turn may cause memory access
	// violations later on when the pool releases its contexts.
	SetContextCallbacks(0, 0, 0);

	// The modules must be deleted first, as they may use
	// object types from the config groups
	for( asUINT n = (asUINT)scriptModules.GetLength(); n-- > 0; )
		if( scriptModules[n] )
			scriptModules[n]->Discard();
	scriptModules.SetLength(0);

	// Do another full garbage collection to destroy the object types/functions
	// that may have been placed in the gc when destroying the modules
	GarbageCollect();

	// Do another sweep to delete discarded modules, that may not have
	// been deleted earlier due to still having external references
	DeleteDiscardedModules();

	// If the application hasn't registered GC behaviours for all types
	// that can form circular references with script types, then there
	// may still be objects in the GC.
	gc.ReportAndReleaseUndestroyedObjects();

	// Release the engine reference
	return Release();
}

// internal
asSNameSpace *asCScriptEngine::AddNameSpace(const char *name)
{
	// First check if it doesn't exist already
	asSNameSpace *ns = FindNameSpace(name);
	if( ns ) return ns;

	ns = asNEW(asSNameSpace);
	if( ns == 0 )
	{
		// Out of memory
		return 0;
	}
	ns->name = name;

	nameSpaces.PushLast(ns);

	return ns;
}

// internal
asSNameSpace *asCScriptEngine::FindNameSpace(const char *name) const
{
	// TODO: optimize: Improve linear search
	for( asUINT n = 0; n < nameSpaces.GetLength(); n++ )
		if( nameSpaces[n]->name == name )
			return nameSpaces[n];

	return 0;
}

// interface
const char *asCScriptEngine::GetDefaultNamespace() const
{
	return defaultNamespace->name.AddressOf();
}

// interface
int asCScriptEngine::SetDefaultNamespace(const char *nameSpace)
{
	if( nameSpace == 0 )
		return ConfigError(asINVALID_ARG, "SetDefaultNamespace", nameSpace, 0);

	asCString ns = nameSpace;
	if( ns != "" )
	{
		// Make sure the namespace is composed of alternating identifier and ::
		size_t pos = 0;
		bool expectIdentifier = true;
		size_t len;
		eTokenType t = ttIdentifier;

		for( ; pos < ns.GetLength(); pos += len)
		{
			t = tok.GetToken(ns.AddressOf() + pos, ns.GetLength() - pos, &len);
			if( (expectIdentifier && t != ttIdentifier) || (!expectIdentifier && t != ttScope) )
				return ConfigError(asINVALID_DECLARATION, "SetDefaultNamespace", nameSpace, 0);

			// Make sure parent namespaces are registred in case of nested namespaces
			if (expectIdentifier)
				AddNameSpace(ns.SubString(0, pos + len).AddressOf());

			expectIdentifier = !expectIdentifier;
		}

		// If the namespace ends with :: then strip it off
		if( t == ttScope )
			ns.SetLength(ns.GetLength()-2);
	}

	defaultNamespace = AddNameSpace(ns.AddressOf());

	return 0;
}

// interface
void *asCScriptEngine::SetUserData(void *data, asPWORD type)
{
	// As a thread might add a new new user data at the same time as another
	// it is necessary to protect both read and write access to the userData member
	ACQUIREEXCLUSIVE(engineRWLock);

	// It is not intended to store a lot of different types of userdata,
	// so a more complex structure like a associative map would just have
	// more overhead than a simple array.
	for( asUINT n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n] == type )
		{
			void *oldData = reinterpret_cast<void*>(userData[n+1]);
			userData[n+1] = reinterpret_cast<asPWORD>(data);

			RELEASEEXCLUSIVE(engineRWLock);

			return oldData;
		}
	}

	userData.PushLast(type);
	userData.PushLast(reinterpret_cast<asPWORD>(data));

	RELEASEEXCLUSIVE(engineRWLock);

	return 0;
}

// interface
void *asCScriptEngine::GetUserData(asPWORD type) const
{
	// There may be multiple threads reading, but when
	// setting the user data nobody must be reading.
	ACQUIRESHARED(engineRWLock);

	for( asUINT n = 0; n < userData.GetLength(); n += 2 )
	{
		if( userData[n] == type )
		{
			RELEASESHARED(engineRWLock);
			return reinterpret_cast<void*>(userData[n+1]);
		}
	}

	RELEASESHARED(engineRWLock);

	return 0;
}

// interface
int asCScriptEngine::SetMessageCallback(const asSFuncPtr &callback, void *obj, asDWORD callConv)
{
	msgCallback = true;
	msgCallbackObj = obj;
	bool isObj = false;
	if( (unsigned)callConv == asCALL_GENERIC || (unsigned)callConv == asCALL_THISCALL_OBJFIRST || (unsigned)callConv == asCALL_THISCALL_OBJLAST )
	{
		msgCallback = false;
		return asNOT_SUPPORTED;
	}
	if( (unsigned)callConv >= asCALL_THISCALL )
	{
		isObj = true;
		if( obj == 0 )
		{
			msgCallback = false;
			return asINVALID_ARG;
		}
	}
	int r = DetectCallingConvention(isObj, callback, callConv, 0, &msgCallbackFunc);
	if( r < 0 ) msgCallback = false;
	return r;
}

// interface
int asCScriptEngine::ClearMessageCallback()
{
	msgCallback = false;
	return 0;
}

// interface
int asCScriptEngine::WriteMessage(const char *section, int row, int col, asEMsgType type, const char *message)
{
	// Validate input parameters
	if( section == 0 ||
		message == 0 )
		return asINVALID_ARG;

	// If there is no callback then there's nothing to do
	if( !msgCallback )
		return 0;

	// If a pre-message has been set, then write that first
	if( preMessage.isSet )
	{
		asSMessageInfo msg;
		msg.section = preMessage.scriptname.AddressOf();
		msg.row     = preMessage.r;
		msg.col     = preMessage.c;
		msg.type    = asMSGTYPE_INFORMATION;
		msg.message = preMessage.message.AddressOf();

		if( msgCallbackFunc.callConv < ICC_THISCALL )
			CallGlobalFunction(&msg, msgCallbackObj, &msgCallbackFunc, 0);
		else
			CallObjectMethod(msgCallbackObj, &msg, &msgCallbackFunc, 0);

		preMessage.isSet = false;
	}

	// Write the message to the callback
	asSMessageInfo msg;
	msg.section = section;
	msg.row     = row;
	msg.col     = col;
	msg.type    = type;
	msg.message = message;

	if( msgCallbackFunc.callConv < ICC_THISCALL )
		CallGlobalFunction(&msg, msgCallbackObj, &msgCallbackFunc, 0);
	else
		CallObjectMethod(msgCallbackObj, &msg, &msgCallbackFunc, 0);

	return 0;
}

int asCScriptEngine::SetJITCompiler(asIJITCompiler *compiler)
{
	jitCompiler = compiler;
	return asSUCCESS;
}

asIJITCompiler *asCScriptEngine::GetJITCompiler() const
{
	return jitCompiler;
}

// interface
asETokenClass asCScriptEngine::ParseToken(const char *string, size_t stringLength, asUINT *tokenLength) const
{
	if( stringLength == 0 )
		stringLength = strlen(string);

	size_t len;
	asETokenClass tc;
	tok.GetToken(string, stringLength, &len, &tc);

	if( tokenLength )
		*tokenLength = (asUINT)len;

	return tc;
}

// interface
asIScriptModule *asCScriptEngine::GetModule(const char *module, asEGMFlags flag)
{
	asCModule *mod = GetModule(module, false);

	if( flag == asGM_ALWAYS_CREATE )
	{
		if( mod != 0 )
			mod->Discard();

		return GetModule(module, true);
	}

	if( mod == 0 && flag == asGM_CREATE_IF_NOT_EXISTS )
		return GetModule(module, true);

	return mod;
}

// interface
int asCScriptEngine::DiscardModule(const char *module)
{
	asCModule *mod = GetModule(module, false);
	if( mod == 0 ) return asNO_MODULE;

	mod->Discard();

	return 0;
}

// interface
asUINT asCScriptEngine::GetModuleCount() const
{
	ACQUIRESHARED(engineRWLock);
	asUINT length = asUINT(scriptModules.GetLength());
	RELEASESHARED(engineRWLock);
	return length;
}

// interface
asIScriptModule *asCScriptEngine::GetModuleByIndex(asUINT index) const
{
	asIScriptModule *mod = 0;
	ACQUIRESHARED(engineRWLock);
	if( index < scriptModules.GetLength() )
		mod = scriptModules[index];
	RELEASESHARED(engineRWLock);
	return mod;
}

// internal
int asCScriptEngine::GetFactoryIdByDecl(const asCObjectType *ot, const char *decl)
{
	asCModule *mod = 0;

	// Is this a script class?
	if( (ot->flags & asOBJ_SCRIPT_OBJECT) && ot->size > 0 )
		mod = scriptFunctions[ot->beh.factories[0]]->module;

	asCBuilder bld(this, mod);

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(this, mod, asFUNC_DUMMY);
	int r = bld.ParseFunctionDeclaration(0, decl, &func, false, 0, 0, defaultNamespace);
	if( r < 0 )
		return asINVALID_DECLARATION;

	// Search for matching factory function
	int id = -1;
	for( asUINT n = 0; n < ot->beh.factories.GetLength(); n++ )
	{
		asCScriptFunction *f = scriptFunctions[ot->beh.factories[n]];

		// We don't really care if the name of the function is correct
		if( f->IsSignatureExceptNameEqual(&func) )
		{
			id = ot->beh.factories[n];
			break;
		}
	}

	if( id == -1 ) return asNO_FUNCTION;

	return id;
}


// internal
int asCScriptEngine::GetMethodIdByDecl(const asCObjectType *ot, const char *decl, asCModule *mod)
{
	asCBuilder bld(this, mod);

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(this, mod, asFUNC_DUMMY);

	// Set the object type so that the signature can be properly compared
	// This cast is OK, it will only be used for comparison
	func.objectType = const_cast<asCObjectType*>(ot);
	func.objectType->AddRefInternal();

	int r = bld.ParseFunctionDeclaration(func.objectType, decl, &func, false);
	if( r < 0 )
		return asINVALID_DECLARATION;

	// Search script functions for matching interface
	int id = -1;
	for( asUINT n = 0; n < ot->methods.GetLength(); ++n )
	{
		if( func.IsSignatureEqual(scriptFunctions[ot->methods[n]]) )
		{
			if( id == -1 )
				id = ot->methods[n];
			else
				return asMULTIPLE_FUNCTIONS;
		}
	}

	if( id == -1 ) return asNO_FUNCTION;

	return id;
}


// internal
asCString asCScriptEngine::GetFunctionDeclaration(int funcId)
{
	asCString str;
	asCScriptFunction *func = GetScriptFunction(funcId);
	if( func )
		str = func->GetDeclarationStr();

	return str;
}

// internal
asCScriptFunction *asCScriptEngine::GetScriptFunction(int funcId) const
{
	if( funcId < 0 || funcId >= (int)scriptFunctions.GetLength() )
		return 0;

	return scriptFunctions[funcId];
}


// interface
asIScriptContext *asCScriptEngine::CreateContext()
{
	asIScriptContext *ctx = 0;
	CreateContext(&ctx, false);
	return ctx;
}

// internal
int asCScriptEngine::CreateContext(asIScriptContext **context, bool isInternal)
{
	*context = asNEW(asCContext)(this, !isInternal);
	if( *context == 0 )
		return asOUT_OF_MEMORY;

	// We need to make sure the engine has been
	// prepared before any context is executed
	PrepareEngine();

	return 0;
}

// interface
int asCScriptEngine::RegisterObjectProperty(const char *obj, const char *declaration, int byteOffset, int compositeOffset, bool isCompositeIndirect)
{
	int r;
	asCDataType dt;
	asCBuilder bld(this, 0);
	r = bld.ParseDataType(obj, &dt, defaultNamespace);
	if( r < 0 )
		return ConfigError(r, "RegisterObjectProperty", obj, declaration);

	if (dt.GetTypeInfo() == 0 || (dt.IsObjectHandle() && !(dt.GetTypeInfo()->GetFlags() & asOBJ_IMPLICIT_HANDLE)))
		return ConfigError(asINVALID_OBJECT, "RegisterObjectProperty", obj, declaration);

	// Don't allow modifying generated template instances
	if( dt.GetTypeInfo() && (dt.GetTypeInfo()->flags & asOBJ_TEMPLATE) && generatedTemplateTypes.Exists(CastToObjectType(dt.GetTypeInfo())) )
		return ConfigError(asINVALID_TYPE, "RegisterObjectProperty", obj, declaration);

	// Verify that the correct config group is used
	if( currentGroup->FindType(dt.GetTypeInfo()->name.AddressOf()) == 0 )
		return ConfigError(asWRONG_CONFIG_GROUP, "RegisterObjectProperty", obj, declaration);

	asCDataType type;
	asCString name;

	if( (r = bld.VerifyProperty(&dt, declaration, name, type, 0)) < 0 )
		return ConfigError(r, "RegisterObjectProperty", obj, declaration);

	// The VM currently only supports 16bit offsets
	// TODO: The VM needs to have support for 32bit offsets. Probably with a second ADDSi instruction
	//       However, when implementing this it is necessary for the bytecode serialization to support
	//       the switch between the instructions upon loading bytecode as the offset may not be the
	//       same on all platforms
	if( byteOffset > 32767 || byteOffset < -32768 )
		return ConfigError(asINVALID_ARG, "RegisterObjectProperty", obj, declaration);
	// The composite offset must also obey the ADDSi restriction
	if (compositeOffset > 32767 || compositeOffset < -32768)
		return ConfigError(asINVALID_ARG, "RegisterObjectProperty", obj, declaration);

	asCObjectProperty *prop = asNEW(asCObjectProperty);
	if( prop == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterObjectProperty", obj, declaration);

	prop->name                = name;
	prop->type                = type;
	prop->byteOffset          = byteOffset;
	prop->isPrivate           = false;
	prop->isProtected         = false;
	prop->compositeOffset     = compositeOffset;
	prop->isCompositeIndirect = isCompositeIndirect;
	prop->accessMask          = defaultAccessMask;

	asCObjectType *ot = CastToObjectType(dt.GetTypeInfo());
	asUINT idx = ot->properties.GetLength();
	ot->properties.PushLast(prop);

	// Add references to types so they are not released too early
	if( type.GetTypeInfo() )
	{
		type.GetTypeInfo()->AddRefInternal();

		// Add template instances to the config group
		if( (type.GetTypeInfo()->flags & asOBJ_TEMPLATE) && !currentGroup->types.Exists(type.GetTypeInfo()) )
			currentGroup->types.PushLast(type.GetTypeInfo());
	}

	currentGroup->AddReferencesForType(this, type.GetTypeInfo());

	// Return the index of the property to signal success
	return idx;
}

// interface
int asCScriptEngine::RegisterInterface(const char *name)
{
	if( name == 0 ) return ConfigError(asINVALID_NAME, "RegisterInterface", 0, 0);

	// Verify if the name has been registered as a type already
	if( GetRegisteredType(name, defaultNamespace) )
		return asALREADY_REGISTERED;

	// Use builder to parse the datatype
	asCDataType dt;
	asCBuilder bld(this, 0);
	bool oldMsgCallback = msgCallback; msgCallback = false;
	int r = bld.ParseDataType(name, &dt, defaultNamespace);
	msgCallback = oldMsgCallback;
	if( r >= 0 )
	{
		// If it is not in the defaultNamespace then the type was successfully parsed because
		// it is declared in a parent namespace which shouldn't be treated as an error
		if( dt.GetTypeInfo() && dt.GetTypeInfo()->nameSpace == defaultNamespace )
			return ConfigError(asERROR, "RegisterInterface", name, 0);
	}

	// Make sure the name is not a reserved keyword
	size_t tokenLen;
	int token = tok.GetToken(name, strlen(name), &tokenLen);
	if( token != ttIdentifier || strlen(name) != tokenLen )
		return ConfigError(asINVALID_NAME, "RegisterInterface", name, 0);

	r = bld.CheckNameConflict(name, 0, 0, defaultNamespace, true, false);
	if( r < 0 )
		return ConfigError(asNAME_TAKEN, "RegisterInterface", name, 0);

	// Don't have to check against members of object
	// types as they are allowed to use the names

	// Register the object type for the interface
	asCObjectType *st = asNEW(asCObjectType)(this);
	if( st == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterInterface", name, 0);

	st->flags = asOBJ_REF | asOBJ_SCRIPT_OBJECT | asOBJ_SHARED;
	st->size = 0; // Cannot be instantiated
	st->name = name;
	st->nameSpace = defaultNamespace;

	// Use the default script class behaviours
	st->beh.factory = 0;
	st->beh.addref = scriptTypeBehaviours.beh.addref;
	scriptFunctions[st->beh.addref]->AddRefInternal();
	st->beh.release = scriptTypeBehaviours.beh.release;
	scriptFunctions[st->beh.release]->AddRefInternal();
	st->beh.copy = 0;

	allRegisteredTypes.Insert(asSNameSpaceNamePair(st->nameSpace, st->name), st);
	registeredObjTypes.PushLast(st);

	currentGroup->types.PushLast(st);

	return GetTypeIdByDecl(name);
}

// interface
int asCScriptEngine::RegisterInterfaceMethod(const char *intf, const char *declaration)
{
	// Verify that the correct config group is set.
	if( currentGroup->FindType(intf) == 0 )
		return ConfigError(asWRONG_CONFIG_GROUP, "RegisterInterfaceMethod", intf, declaration);

	asCDataType dt;
	asCBuilder bld(this, 0);
	int r = bld.ParseDataType(intf, &dt, defaultNamespace);
	if( r < 0 )
		return ConfigError(r, "RegisterInterfaceMethod", intf, declaration);

	asCScriptFunction *func = asNEW(asCScriptFunction)(this, 0, asFUNC_INTERFACE);
	if( func == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterInterfaceMethod", intf, declaration);

	func->objectType = CastToObjectType(dt.GetTypeInfo());
	func->objectType->AddRefInternal();

	r = bld.ParseFunctionDeclaration(func->objectType, declaration, func, false);
	if( r < 0 )
	{
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asINVALID_DECLARATION, "RegisterInterfaceMethod", intf, declaration);
	}

	// Check name conflicts
	r = bld.CheckNameConflictMember(dt.GetTypeInfo(), func->name.AddressOf(), 0, 0, false, false);
	if( r < 0 )
	{
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asNAME_TAKEN, "RegisterInterfaceMethod", intf, declaration);
	}

	func->id = GetNextScriptFunctionId();
	AddScriptFunction(func);

	// The index into the interface's vftable chunk should be
	// its index in the methods array.
	func->vfTableIdx = int(func->objectType->methods.GetLength());

	func->objectType->methods.PushLast(func->id);

	func->ComputeSignatureId();

	currentGroup->AddReferencesForFunc(this, func);

	// Return function id as success
	return func->id;
}

int asCScriptEngine::RegisterObjectType(const char *name, int byteSize, asDWORD flags)
{
	int r;

	isPrepared = false;

	// Verify flags
	// Must have either asOBJ_REF or asOBJ_VALUE
	if( flags & asOBJ_REF )
	{
		// Can optionally have the asOBJ_GC, asOBJ_NOHANDLE, asOBJ_SCOPED, or asOBJ_TEMPLATE flag set, but nothing else
		if( flags & ~(asOBJ_REF | asOBJ_GC | asOBJ_NOHANDLE | asOBJ_SCOPED | asOBJ_TEMPLATE | asOBJ_NOCOUNT | asOBJ_IMPLICIT_HANDLE) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);

		// flags are exclusive
		if( (flags & asOBJ_GC) && (flags & (asOBJ_NOHANDLE|asOBJ_SCOPED|asOBJ_NOCOUNT)) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		if( (flags & asOBJ_NOHANDLE) && (flags & (asOBJ_GC|asOBJ_SCOPED|asOBJ_NOCOUNT|asOBJ_IMPLICIT_HANDLE)) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		if( (flags & asOBJ_SCOPED) && (flags & (asOBJ_GC|asOBJ_NOHANDLE|asOBJ_NOCOUNT|asOBJ_IMPLICIT_HANDLE)) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		if( (flags & asOBJ_NOCOUNT) && (flags & (asOBJ_GC|asOBJ_NOHANDLE|asOBJ_SCOPED)) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);

		// Implicit handle is only allowed if the engine property for this is turned on
		if( !ep.allowImplicitHandleTypes && (flags & asOBJ_IMPLICIT_HANDLE) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
	}
	else if( flags & asOBJ_VALUE )
	{
		// Cannot use reference flags
		if( flags & (asOBJ_REF | asOBJ_NOHANDLE | asOBJ_SCOPED | asOBJ_NOCOUNT | asOBJ_IMPLICIT_HANDLE) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);

		// Flags are exclusive
		if( (flags & asOBJ_POD) && (flags & (asOBJ_ASHANDLE | asOBJ_TEMPLATE)) )
			return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);

		// If the app type is given, we must validate the flags
		if( flags & asOBJ_APP_CLASS )
		{
			// Must not set the primitive or float flag
			if( flags & (asOBJ_APP_PRIMITIVE | asOBJ_APP_FLOAT | asOBJ_APP_ARRAY) )
				return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		}
		else
		{
			// Must not set the class properties, without the class flag
			if( flags & (asOBJ_APP_CLASS_CONSTRUCTOR      |
						 asOBJ_APP_CLASS_DESTRUCTOR       |
						 asOBJ_APP_CLASS_ASSIGNMENT       |
						 asOBJ_APP_CLASS_COPY_CONSTRUCTOR |
						 asOBJ_APP_CLASS_ALLINTS          |
						 asOBJ_APP_CLASS_ALLFLOATS) )
			{
				return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
			}
		}

		if( flags & asOBJ_APP_PRIMITIVE )
		{
			if( flags & (asOBJ_APP_CLASS |
						 asOBJ_APP_FLOAT |
						 asOBJ_APP_ARRAY) )
				return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		}
		else if( flags & asOBJ_APP_FLOAT )
		{
			if( flags & (asOBJ_APP_CLASS     |
						 asOBJ_APP_PRIMITIVE |
						 asOBJ_APP_ARRAY) )
				return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		}
		else if( flags & asOBJ_APP_ARRAY )
		{
			if( flags & (asOBJ_APP_CLASS     |
						 asOBJ_APP_PRIMITIVE |
						 asOBJ_APP_FLOAT) )
				return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
		}
	}
	else
		return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);

	// Don't allow anything else than the defined flags
#ifndef WIP_16BYTE_ALIGN
	if( flags - (flags & asOBJ_MASK_VALID_FLAGS) )
#else
	if( flags - (flags & (asOBJ_MASK_VALID_FLAGS | asOBJ_APP_ALIGN16)) )
#endif
		return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);

	// Value types must have a defined size
	if( (flags & asOBJ_VALUE) && byteSize == 0 )
	{
		WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_VALUE_TYPE_MUST_HAVE_SIZE);
		return ConfigError(asINVALID_ARG, "RegisterObjectType", name, 0);
	}

	// Verify type name
	if( name == 0 )
		return ConfigError(asINVALID_NAME, "RegisterObjectType", name, 0);

	asCString typeName;
	asCBuilder bld(this, 0);
	if( flags & asOBJ_TEMPLATE )
	{
		asCArray<asCString> subtypeNames;
		r = bld.ParseTemplateDecl(name, &typeName, subtypeNames);
		if( r < 0 )
			return ConfigError(r, "RegisterObjectType", name, 0);

		// Verify that the template name hasn't been registered as a type already
		if( GetRegisteredType(typeName, defaultNamespace) )
			// This is not an irrepairable error, as it may just be that the same type is registered twice
			return asALREADY_REGISTERED;

		asCObjectType *type = asNEW(asCObjectType)(this);
		if( type == 0 )
			return ConfigError(asOUT_OF_MEMORY, "RegisterObjectType", name, 0);

		type->name       = typeName;
		type->nameSpace  = defaultNamespace;
		type->size       = byteSize;
#ifdef WIP_16BYTE_ALIGN
		// TODO: Types smaller than 4 don't need to be aligned to 4 byte boundaries
		type->alignment  = (flags & asOBJ_APP_ALIGN16) ? 16 : 4;
#endif
		type->flags      = flags;
		type->accessMask = defaultAccessMask;

		// Store it in the object types
		allRegisteredTypes.Insert(asSNameSpaceNamePair(type->nameSpace, type->name), type);
		currentGroup->types.PushLast(type);
		registeredObjTypes.PushLast(type);
		registeredTemplateTypes.PushLast(type);

		// Define the template subtypes
		for( asUINT subTypeIdx = 0; subTypeIdx < subtypeNames.GetLength(); subTypeIdx++ )
		{
			asCTypeInfo *subtype = 0;
			for( asUINT n = 0; n < templateSubTypes.GetLength(); n++ )
			{
				if( templateSubTypes[n]->name == subtypeNames[subTypeIdx] )
				{
					subtype = templateSubTypes[n];
					break;
				}
			}
			if( subtype == 0 )
			{
				// Create the new subtype if not already existing
				subtype = asNEW(asCTypeInfo)(this);
				if( subtype == 0 )
					return ConfigError(asOUT_OF_MEMORY, "RegisterObjectType", name, 0);

				subtype->name      = subtypeNames[subTypeIdx];
				subtype->size      = 0;
				subtype->flags     = asOBJ_TEMPLATE_SUBTYPE;
				templateSubTypes.PushLast(subtype);
			}
			type->templateSubTypes.PushLast(asCDataType::CreateType(subtype, false));
			subtype->AddRefInternal();
		}
	}
	else
	{
		typeName = name;

		// Verify if the name has been registered as a type already
		if( GetRegisteredType(typeName, defaultNamespace) )
			// This is not an irrepairable error, as it may just be that the same type is registered twice
			return asALREADY_REGISTERED;

		// Keep the most recent template generated instance type, so we know what it was before parsing the datatype
		asCObjectType *mostRecentTemplateInstanceType = 0;
		asUINT originalSizeOfGeneratedTemplateTypes = (asUINT)generatedTemplateTypes.GetLength();
		if( originalSizeOfGeneratedTemplateTypes )
			mostRecentTemplateInstanceType = generatedTemplateTypes[originalSizeOfGeneratedTemplateTypes-1];

		// Use builder to parse the datatype
		asCDataType dt;
		bool oldMsgCallback = msgCallback; msgCallback = false;
		r = bld.ParseDataType(name, &dt, defaultNamespace);
		msgCallback = oldMsgCallback;

		// If the builder fails or the namespace is different than the default
		// namespace, then the type name is new and it should be registered
		if( r < 0 || dt.GetTypeInfo()->nameSpace != defaultNamespace )
		{
			// Make sure the name is not a reserved keyword
			size_t tokenLen;
			int token = tok.GetToken(name, typeName.GetLength(), &tokenLen);
			if( token != ttIdentifier || typeName.GetLength() != tokenLen )
				return ConfigError(asINVALID_NAME, "RegisterObjectType", name, 0);

			r = bld.CheckNameConflict(name, 0, 0, defaultNamespace, true, false);
			if( r < 0 )
				return ConfigError(asNAME_TAKEN, "RegisterObjectType", name, 0);

			// Don't have to check against members of object
			// types as they are allowed to use the names

			// Put the data type in the list
			asCObjectType *type = asNEW(asCObjectType)(this);
			if( type == 0 )
				return ConfigError(asOUT_OF_MEMORY, "RegisterObjectType", name, 0);

			type->name       = typeName;
			type->nameSpace  = defaultNamespace;
			type->size       = byteSize;
#ifdef WIP_16BYTE_ALIGN
			// TODO: Types smaller than 4 don't need to be aligned to 4 byte boundaries
			type->alignment  = (flags & asOBJ_APP_ALIGN16) ? 16 : 4;
#endif
			type->flags      = flags;
			type->accessMask = defaultAccessMask;

			allRegisteredTypes.Insert(asSNameSpaceNamePair(type->nameSpace, type->name), type);
			registeredObjTypes.PushLast(type);

			currentGroup->types.PushLast(type);
		}
		else
		{
			// The application is registering a template specialization so we
			// need to replace the template instance type with the new type.

			// TODO: Template: We don't require the lower dimensions to be registered first for registered template types
			// int[][] must not be allowed to be registered
			// if int[] hasn't been registered first
			if( dt.GetSubType().IsTemplate() )
				return ConfigError(asLOWER_ARRAY_DIMENSION_NOT_REGISTERED, "RegisterObjectType", name, 0);

			if( dt.IsReadOnly() ||
				dt.IsReference() )
				return ConfigError(asINVALID_TYPE, "RegisterObjectType", name, 0);

			// Was the template instance type generated before?
			if( generatedTemplateTypes.Exists(CastToObjectType(dt.GetTypeInfo())) &&
				generatedTemplateTypes[generatedTemplateTypes.GetLength()-1] == mostRecentTemplateInstanceType )
			{
				asCString str;
				str.Format(TXT_TEMPLATE_s_ALREADY_GENERATED_CANT_REGISTER, typeName.AddressOf());
				WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				return ConfigError(asNOT_SUPPORTED, "RegisterObjectType", name, 0);
			}

			// If this is not a generated template instance type, then it means it is an
			// already registered template specialization
			if( !generatedTemplateTypes.Exists(CastToObjectType(dt.GetTypeInfo())) )
				return ConfigError(asALREADY_REGISTERED, "RegisterObjectType", name, 0);

			// TODO: Add this again. The type is used by the factory stubs so we need to discount that
			// Is the template instance type already being used?
//			if( dt.GetTypeInfo()->GetRefCount() > 1 )
//				return ConfigError(asNOT_SUPPORTED, "RegisterObjectType", name, 0);

			// Put the data type in the list
			asCObjectType *type = asNEW(asCObjectType)(this);
			if( type == 0 )
				return ConfigError(asOUT_OF_MEMORY, "RegisterObjectType", name, 0);

			type->name       = dt.GetTypeInfo()->name;
			// The namespace will be the same as the original template type
			type->nameSpace  = dt.GetTypeInfo()->nameSpace;
			type->templateSubTypes.PushLast(dt.GetSubType());
			for( asUINT s = 0; s < type->templateSubTypes.GetLength(); s++ )
				if( type->templateSubTypes[s].GetTypeInfo() )
					type->templateSubTypes[s].GetTypeInfo()->AddRefInternal();
			type->size       = byteSize;
#ifdef WIP_16BYTE_ALIGN
			// TODO: Types smaller than 4 don't need to be aligned to 4 byte boundaries
			type->alignment  = (flags & asOBJ_APP_ALIGN16) ? 16 : 4;
#endif
			type->flags      = flags;
			type->accessMask = defaultAccessMask;

			templateInstanceTypes.PushLast(type);

			currentGroup->types.PushLast(type);

			// Remove the template instance type, which will no longer be used.
			// It is possible that multiple template instances are generated if
			// they have any relationship, so all of them must be removed
			while( generatedTemplateTypes.GetLength() > originalSizeOfGeneratedTemplateTypes )
				RemoveTemplateInstanceType(generatedTemplateTypes[generatedTemplateTypes.GetLength()-1]);
		}
	}

	// Return the type id as the success (except for template types)
	if( flags & asOBJ_TEMPLATE )
		return asSUCCESS;

	return GetTypeIdByDecl(name);
}

// interface
int asCScriptEngine::RegisterObjectBehaviour(const char *datatype, asEBehaviours behaviour, const char *decl, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary, int compositeOffset, bool isCompositeIndirect)
{
	if( datatype == 0 ) return ConfigError(asINVALID_ARG, "RegisterObjectBehaviour", datatype, decl);

	// Determine the object type
	asCBuilder bld(this, 0);
	asCDataType type;
	int r = bld.ParseDataType(datatype, &type, defaultNamespace);
	if( r < 0 )
		return ConfigError(r, "RegisterObjectBehaviour", datatype, decl);

	if( type.GetTypeInfo() == 0 || (type.IsObjectHandle() && !(type.GetTypeInfo()->GetFlags() & asOBJ_IMPLICIT_HANDLE)) )
		return ConfigError(asINVALID_TYPE, "RegisterObjectBehaviour", datatype, decl);

	// Don't allow application to modify built-in types
	if( type.GetTypeInfo() == &functionBehaviours ||
		type.GetTypeInfo() == &scriptTypeBehaviours )
		return ConfigError(asINVALID_TYPE, "RegisterObjectBehaviour", datatype, decl);

	if( type.IsReadOnly() || type.IsReference() )
		return ConfigError(asINVALID_TYPE, "RegisterObjectBehaviour", datatype, decl);

	// Don't allow modifying generated template instances
	if( type.GetTypeInfo() && (type.GetTypeInfo()->flags & asOBJ_TEMPLATE) && generatedTemplateTypes.Exists(CastToObjectType(type.GetTypeInfo())) )
		return ConfigError(asINVALID_TYPE, "RegisterObjectBehaviour", datatype, decl);

	return RegisterBehaviourToObjectType(CastToObjectType(type.GetTypeInfo()), behaviour, decl, funcPointer, callConv, auxiliary, compositeOffset, isCompositeIndirect);
}

// internal
int asCScriptEngine::RegisterBehaviourToObjectType(asCObjectType *objectType, asEBehaviours behaviour, const char *decl, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary, int compositeOffset, bool isCompositeIndirect)
{
#ifdef AS_MAX_PORTABILITY
	if( callConv != asCALL_GENERIC )
		return ConfigError(asNOT_SUPPORTED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
#endif

	asSSystemFunctionInterface internal;
	bool isMethod = !(behaviour == asBEHAVE_FACTORY ||
		              behaviour == asBEHAVE_LIST_FACTORY ||
		              behaviour == asBEHAVE_TEMPLATE_CALLBACK);
	int r = DetectCallingConvention(isMethod, funcPointer, callConv, auxiliary, &internal);
	if( r < 0 )
		return ConfigError(r, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

	internal.compositeOffset = compositeOffset;
	internal.isCompositeIndirect = isCompositeIndirect;
	if( (compositeOffset || isCompositeIndirect) && callConv != asCALL_THISCALL )
		return ConfigError(asINVALID_ARG, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

	// TODO: cleanup: This is identical to what is in RegisterMethodToObjectType
	// If the object type is a template, make sure there are no generated instances already
	if( objectType->flags & asOBJ_TEMPLATE )
	{
		for( asUINT n = 0; n < generatedTemplateTypes.GetLength(); n++ )
		{
			asCObjectType *tmpl = generatedTemplateTypes[n];
			if( tmpl->name == objectType->name &&
				tmpl->nameSpace == objectType->nameSpace &&
				!(tmpl->templateSubTypes[0].GetTypeInfo() && (tmpl->templateSubTypes[0].GetTypeInfo()->flags & asOBJ_TEMPLATE_SUBTYPE)) )
			{
				asCString msg;
				msg.Format(TXT_TEMPLATE_s_ALREADY_GENERATED_CANT_REGISTER, asCDataType::CreateType(tmpl, false).Format(tmpl->nameSpace).AddressOf());
				WriteMessage("",0,0, asMSGTYPE_ERROR, msg.AddressOf());
				return ConfigError(asERROR, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
			}
		}
	}

	isPrepared = false;

	asSTypeBehaviour *beh = &objectType->beh;

	// Verify function declaration
	asCScriptFunction func(this, 0, asFUNC_DUMMY);

	bool expectListPattern = behaviour == asBEHAVE_LIST_FACTORY || behaviour == asBEHAVE_LIST_CONSTRUCT;
	asCScriptNode *listPattern = 0;
	asCBuilder bld(this, 0);
	r = bld.ParseFunctionDeclaration(objectType, decl, &func, true, &internal.paramAutoHandles, &internal.returnAutoHandle, 0, expectListPattern ? &listPattern : 0);
	if( r < 0 )
	{
		if( listPattern )
			listPattern->Destroy(this);
		return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
	}
	func.name.Format("$beh%d", behaviour);

	if( behaviour != asBEHAVE_FACTORY && behaviour != asBEHAVE_LIST_FACTORY )
	{
		func.objectType = objectType;
		func.objectType->AddRefInternal();
	}

	// Check if the method restricts that use of the template to value types or reference types
	if( objectType->flags & asOBJ_TEMPLATE )
	{
		r = SetTemplateRestrictions(objectType, &func, "RegisterObjectBehaviour", decl);
		if (r < 0)
			return r;
	}

	if( behaviour == asBEHAVE_CONSTRUCT )
	{
		// Verify that the return type is void
		if( func.returnType != asCDataType::CreatePrimitive(ttVoid, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		if( objectType->flags & asOBJ_SCRIPT_OBJECT )
		{
			// The script object is a special case
			asASSERT(func.parameterTypes.GetLength() == 1);

			beh->construct = AddBehaviourFunction(func, internal);
			beh->factory   = beh->construct;
			scriptFunctions[beh->factory]->AddRefInternal();
			beh->constructors.PushLast(beh->construct);
			beh->factories.PushLast(beh->factory);
			func.id = beh->construct;
		}
		else
		{
			// Verify that it is a value type
			if( !(func.objectType->flags & asOBJ_VALUE) )
			{
				WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
				return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
			}

			// The templates take a hidden parameter with the object type
			if( (objectType->flags & asOBJ_TEMPLATE) &&
				(func.parameterTypes.GetLength() == 0 ||
				 !func.parameterTypes[0].IsReference()) )
			{
				WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_FIRST_PARAM_MUST_BE_REF_FOR_TEMPLATE_FACTORY);
				return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
			}

			// TODO: Verify that the same constructor hasn't been registered already

			// Store all constructors in a list
			func.id = AddBehaviourFunction(func, internal);
			beh->constructors.PushLast(func.id);
			if( func.parameterTypes.GetLength() == 0 ||
				(func.parameterTypes.GetLength() == 1 && (objectType->flags & asOBJ_TEMPLATE)) )
			{
				beh->construct = func.id;
			}
			else if( func.parameterTypes.GetLength() == 1 )
			{
				// Is this the copy constructor?
				asCDataType paramType = func.parameterTypes[0];

				// If the parameter is object, and const reference for input or inout,
				// and same type as this class, then this is a copy constructor.
				if( paramType.IsObject() && paramType.IsReference() && paramType.IsReadOnly() &&
					(func.inOutFlags[0] & asTM_INREF) && paramType.GetTypeInfo() == objectType )
					beh->copyconstruct = func.id;
			}
		}
	}
	else if( behaviour == asBEHAVE_DESTRUCT )
	{
		// Must be a value type
		if( !(func.objectType->flags & asOBJ_VALUE) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		if( beh->destruct )
			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that the return type is void
		if( func.returnType != asCDataType::CreatePrimitive(ttVoid, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that there are no parameters
		if( func.parameterTypes.GetLength() > 0 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		func.id = beh->destruct = AddBehaviourFunction(func, internal);
	}
	else if( behaviour == asBEHAVE_LIST_CONSTRUCT )
	{
		func.name = "$list";
		
		// Verify that the return type is void
		if( func.returnType != asCDataType::CreatePrimitive(ttVoid, false) )
		{
			if( listPattern )
				listPattern->Destroy(this);

			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Verify that it is a value type
		if( !(func.objectType->flags & asOBJ_VALUE) )
		{
			if( listPattern )
				listPattern->Destroy(this);

			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Verify the parameters
		// The templates take a hidden parameter with the object type
		if( (!(objectType->flags & asOBJ_TEMPLATE) && (func.parameterTypes.GetLength() != 1 || !func.parameterTypes[0].IsReference())) || 
			((objectType->flags & asOBJ_TEMPLATE) && (func.parameterTypes.GetLength() != 2 || !func.parameterTypes[0].IsReference() || !func.parameterTypes[1].IsReference())) )
		{
			if( listPattern )
				listPattern->Destroy(this);

			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_LIST_FACTORY_EXPECTS_1_REF_PARAM);
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Don't accept duplicates
		if( beh->listFactory )
		{
			if( listPattern )
				listPattern->Destroy(this);

			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Add the function
		func.id = AddBehaviourFunction(func, internal);

		// Re-use the listFactory member, as it is not possible to have both anyway
		beh->listFactory = func.id;

		// Store the list pattern for this function
		r = scriptFunctions[func.id]->RegisterListPattern(decl, listPattern);

		if( listPattern )
			listPattern->Destroy(this);

		if( r < 0 )
			return ConfigError(r, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
	}
	else if( behaviour == asBEHAVE_FACTORY || behaviour == asBEHAVE_LIST_FACTORY )
	{
		if( behaviour == asBEHAVE_LIST_FACTORY )
			func.name = "$list";
		
		// Must be a ref type and must not have asOBJ_NOHANDLE
		if( !(objectType->flags & asOBJ_REF) || (objectType->flags & asOBJ_NOHANDLE) )
		{
			if( listPattern )
				listPattern->Destroy(this);
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Verify that the return type is a handle to the type
		if( func.returnType != asCDataType::CreateObjectHandle(objectType, false) )
		{
			if( listPattern )
				listPattern->Destroy(this);
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// The templates take a hidden parameter with the object type
		if( (objectType->flags & asOBJ_TEMPLATE) &&
			(func.parameterTypes.GetLength() == 0 ||
			 !func.parameterTypes[0].IsReference()) )
		{
			if( listPattern )
				listPattern->Destroy(this);

			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_FIRST_PARAM_MUST_BE_REF_FOR_TEMPLATE_FACTORY);
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		if( behaviour == asBEHAVE_LIST_FACTORY )
		{
			// Make sure the factory takes a reference as its last parameter
			if( objectType->flags & asOBJ_TEMPLATE )
			{
				if( func.parameterTypes.GetLength() != 2 || !func.parameterTypes[1].IsReference() )
				{
					if( listPattern )
						listPattern->Destroy(this);

					WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_TEMPLATE_LIST_FACTORY_EXPECTS_2_REF_PARAMS);
					return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
				}
			}
			else
			{
				if( func.parameterTypes.GetLength() != 1 || !func.parameterTypes[0].IsReference() )
				{
					if( listPattern )
						listPattern->Destroy(this);

					WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_LIST_FACTORY_EXPECTS_1_REF_PARAM);
					return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
				}
			}
		}

		// TODO: Verify that the same factory function hasn't been registered already

		// Don't accept duplicates
		if( behaviour == asBEHAVE_LIST_FACTORY && beh->listFactory )
		{
			if( listPattern )
				listPattern->Destroy(this);

			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Store all factory functions in a list
		func.id = AddBehaviourFunction(func, internal);

		// The list factory is a special factory and isn't stored together with the rest
		if( behaviour != asBEHAVE_LIST_FACTORY )
			beh->factories.PushLast(func.id);

		if( (func.parameterTypes.GetLength() == 0) ||
			(func.parameterTypes.GetLength() == 1 && (objectType->flags & asOBJ_TEMPLATE)) )
		{
			beh->factory = func.id;
		}
		else if( (func.parameterTypes.GetLength() == 1) ||
				 (func.parameterTypes.GetLength() == 2 && (objectType->flags & asOBJ_TEMPLATE)) )
		{
			if( behaviour == asBEHAVE_LIST_FACTORY )
			{
				beh->listFactory = func.id;

				// Store the list pattern for this function
				r = scriptFunctions[func.id]->RegisterListPattern(decl, listPattern);

				if( listPattern )
					listPattern->Destroy(this);

				if( r < 0 )
					return ConfigError(r, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
			}
			else
			{
				// Is this the copy factory?
				asCDataType paramType = func.parameterTypes[func.parameterTypes.GetLength()-1];

				// If the parameter is object, and const reference for input,
				// and same type as this class, then this is a copy constructor.
				if( paramType.IsObject() && paramType.IsReference() && paramType.IsReadOnly() && func.inOutFlags[func.parameterTypes.GetLength()-1] == asTM_INREF && paramType.GetTypeInfo() == objectType )
					beh->copyfactory = func.id;
			}
		}
	}
	else if( behaviour == asBEHAVE_ADDREF )
	{
		// Must be a ref type and must not have asOBJ_NOHANDLE, nor asOBJ_SCOPED
		if( !(func.objectType->flags & asOBJ_REF) ||
			(func.objectType->flags & asOBJ_NOHANDLE) ||
			(func.objectType->flags & asOBJ_SCOPED) ||
			(func.objectType->flags & asOBJ_NOCOUNT) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		if( beh->addref )
			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that the return type is void
		if( func.returnType != asCDataType::CreatePrimitive(ttVoid, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that there are no parameters
		if( func.parameterTypes.GetLength() > 0 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		func.id = beh->addref = AddBehaviourFunction(func, internal);
	}
	else if( behaviour == asBEHAVE_RELEASE )
	{
		// Must be a ref type and must not have asOBJ_NOHANDLE
		if( !(func.objectType->flags & asOBJ_REF) ||
			(func.objectType->flags & asOBJ_NOHANDLE) ||
			(func.objectType->flags & asOBJ_NOCOUNT) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		if( beh->release )
			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that the return type is void
		if( func.returnType != asCDataType::CreatePrimitive(ttVoid, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that there are no parameters
		if( func.parameterTypes.GetLength() > 0 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		func.id = beh->release = AddBehaviourFunction(func, internal);
	}
	else if( behaviour == asBEHAVE_TEMPLATE_CALLBACK )
	{
		// Must be a template type
		if( !(func.objectType->flags & asOBJ_TEMPLATE) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		if( beh->templateCallback )
			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that the return type is bool
		if( func.returnType != asCDataType::CreatePrimitive(ttBool, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that there are two parameters
		if( func.parameterTypes.GetLength() != 2 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// The first parameter must be an inref (to receive the object type), and
		// the second must be a bool out ref (to return if the type should or shouldn't be garbage collected)
		if( func.inOutFlags[0] != asTM_INREF || func.inOutFlags[1] != asTM_OUTREF || !func.parameterTypes[1].IsEqualExceptRef(asCDataType::CreatePrimitive(ttBool, false)) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		func.id = beh->templateCallback = AddBehaviourFunction(func, internal);
	}
	else if( behaviour >= asBEHAVE_FIRST_GC &&
		     behaviour <= asBEHAVE_LAST_GC )
	{
		// Only allow GC behaviours for types registered to be garbage collected
		if( !(func.objectType->flags & asOBJ_GC) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Verify parameter count
		if( (behaviour == asBEHAVE_GETREFCOUNT ||
			 behaviour == asBEHAVE_SETGCFLAG   ||
			 behaviour == asBEHAVE_GETGCFLAG) &&
			func.parameterTypes.GetLength() != 0 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		if( (behaviour == asBEHAVE_ENUMREFS ||
			 behaviour == asBEHAVE_RELEASEREFS) &&
			func.parameterTypes.GetLength() != 1 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify return type
		if( behaviour == asBEHAVE_GETREFCOUNT &&
			func.returnType != asCDataType::CreatePrimitive(ttInt, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		if( behaviour == asBEHAVE_GETGCFLAG &&
			func.returnType != asCDataType::CreatePrimitive(ttBool, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		if( (behaviour == asBEHAVE_SETGCFLAG ||
			 behaviour == asBEHAVE_ENUMREFS  ||
			 behaviour == asBEHAVE_RELEASEREFS) &&
			func.returnType != asCDataType::CreatePrimitive(ttVoid, false) )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		if( behaviour == asBEHAVE_GETREFCOUNT )
			func.id = beh->gcGetRefCount = AddBehaviourFunction(func, internal);
		else if( behaviour == asBEHAVE_SETGCFLAG )
			func.id = beh->gcSetFlag = AddBehaviourFunction(func, internal);
		else if( behaviour == asBEHAVE_GETGCFLAG )
			func.id = beh->gcGetFlag = AddBehaviourFunction(func, internal);
		else if( behaviour == asBEHAVE_ENUMREFS )
			func.id = beh->gcEnumReferences = AddBehaviourFunction(func, internal);
		else if( behaviour == asBEHAVE_RELEASEREFS )
			func.id = beh->gcReleaseAllReferences = AddBehaviourFunction(func, internal);
	}
	else if ( behaviour == asBEHAVE_GET_WEAKREF_FLAG )
	{
		// This behaviour is only allowed for reference types
		if( !(func.objectType->flags & asOBJ_REF) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Don't allow it if the type is registered with nohandle or scoped
		if( func.objectType->flags & (asOBJ_NOHANDLE|asOBJ_SCOPED) )
		{
			WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE);
			return ConfigError(asILLEGAL_BEHAVIOUR_FOR_TYPE, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
		}

		// Verify that the return type is a reference since it needs to return a pointer to an asISharedBool
		if( !func.returnType.IsReference() )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		// Verify that there are no parameters
		if( func.parameterTypes.GetLength() != 0 )
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		if( beh->getWeakRefFlag )
			return ConfigError(asALREADY_REGISTERED, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

		func.id = beh->getWeakRefFlag = AddBehaviourFunction(func, internal);
	}
	else
	{
		asASSERT(false);

		return ConfigError(asINVALID_ARG, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);
	}

	if( func.id < 0 )
		return ConfigError(func.id, "RegisterObjectBehaviour", objectType->name.AddressOf(), decl);

	// Return function id as success
	return func.id;
}

int asCScriptEngine::SetTemplateRestrictions(asCObjectType *templateType, asCScriptFunction *func, const char *caller, const char *decl)
{
	asASSERT(templateType->flags & asOBJ_TEMPLATE);

	for (asUINT subTypeIdx = 0; subTypeIdx < templateType->templateSubTypes.GetLength(); subTypeIdx++)
	{
		if (func->returnType.GetTypeInfo() == templateType->templateSubTypes[subTypeIdx].GetTypeInfo())
		{
			if (func->returnType.IsObjectHandle())
				templateType->acceptValueSubType = false;
			else if (!func->returnType.IsReference())
				templateType->acceptRefSubType = false;

			// Can't support template subtypes by value, since each type is treated differently in the ABI
			if (!func->returnType.IsObjectHandle() && !func->returnType.IsReference())
				return ConfigError(asNOT_SUPPORTED, caller, templateType->name.AddressOf(), decl);
		}

		for (asUINT n = 0; n < func->parameterTypes.GetLength(); n++)
		{
			if (func->parameterTypes[n].GetTypeInfo() == templateType->templateSubTypes[subTypeIdx].GetTypeInfo())
			{
				if (func->parameterTypes[n].IsObjectHandle() || 
					(!ep.allowUnsafeReferences && func->parameterTypes[n].IsReference() && func->inOutFlags[n] == asTM_INOUTREF))
					templateType->acceptValueSubType = false;
				else if (!func->parameterTypes[n].IsReference())
					templateType->acceptRefSubType = false;

				// Can't support template subtypes by value, since each type is treated differently in the ABI
				if (!func->parameterTypes[n].IsObjectHandle() && !func->parameterTypes[n].IsReference())
					return ConfigError(asNOT_SUPPORTED, caller, templateType->name.AddressOf(), decl);
			}
		}
	}

	return asSUCCESS;
}

int asCScriptEngine::VerifyVarTypeNotInFunction(asCScriptFunction *func)
{
	// Don't allow var type in this function
	if( func->returnType.GetTokenType() == ttQuestion )
		return asINVALID_DECLARATION;

	for( unsigned int n = 0; n < func->parameterTypes.GetLength(); n++ )
		if( func->parameterTypes[n].GetTokenType() == ttQuestion )
			return asINVALID_DECLARATION;

	return 0;
}

int asCScriptEngine::AddBehaviourFunction(asCScriptFunction &func, asSSystemFunctionInterface &internal)
{
	asUINT n;

	int id = GetNextScriptFunctionId();

	asSSystemFunctionInterface *newInterface = asNEW(asSSystemFunctionInterface)(internal);
	if( newInterface == 0 )
		return asOUT_OF_MEMORY;

	asCScriptFunction *f = asNEW(asCScriptFunction)(this, 0, asFUNC_SYSTEM);
	if( f == 0 )
	{
		asDELETE(newInterface, asSSystemFunctionInterface);
		return asOUT_OF_MEMORY;
	}

	asASSERT(func.name != "" && func.name != "f");
	f->name           = func.name;
	f->sysFuncIntf    = newInterface;
	f->returnType     = func.returnType;
	f->objectType     = func.objectType;
	if( f->objectType )
		f->objectType->AddRefInternal();
	f->id             = id;
	f->SetReadOnly(func.IsReadOnly());
	f->accessMask     = defaultAccessMask;
	f->parameterTypes = func.parameterTypes;
	f->parameterNames = func.parameterNames;
	f->inOutFlags     = func.inOutFlags;
	f->traits         = func.traits;
	for( n = 0; n < func.defaultArgs.GetLength(); n++ )
		if( func.defaultArgs[n] )
			f->defaultArgs.PushLast(asNEW(asCString)(*func.defaultArgs[n]));
		else
			f->defaultArgs.PushLast(0);

	AddScriptFunction(f);

	// If parameter type from other groups are used, add references
	currentGroup->AddReferencesForFunc(this, f);

	return id;
}

// interface
int asCScriptEngine::RegisterGlobalProperty(const char *declaration, void *pointer)
{
	// Don't accept a null pointer
	if( pointer == 0 )
		return ConfigError(asINVALID_ARG, "RegisterGlobalProperty", declaration, 0);

	asCDataType type;
	asCString name;

	int r;
	asCBuilder bld(this, 0);
	if( (r = bld.VerifyProperty(0, declaration, name, type, defaultNamespace)) < 0 )
		return ConfigError(r, "RegisterGlobalProperty", declaration, 0);

	// Don't allow registering references as global properties
	if( type.IsReference() )
		return ConfigError(asINVALID_TYPE, "RegisterGlobalProperty", declaration, 0);

	// Store the property info
	asCGlobalProperty *prop = AllocateGlobalProperty();
	prop->name        = name;
	prop->nameSpace   = defaultNamespace;
	prop->type        = type;
	prop->accessMask  = defaultAccessMask;

	prop->SetRegisteredAddress(pointer);
	varAddressMap.Insert(prop->GetAddressOfValue(), prop);

	asUINT idx = registeredGlobalProps.Put(prop);
	prop->AddRef();
	currentGroup->globalProps.PushLast(prop);

	currentGroup->AddReferencesForType(this, type.GetTypeInfo());

	// Return the index of the property to signal success
	return int(idx);
}

// internal
asCGlobalProperty *asCScriptEngine::AllocateGlobalProperty()
{
	asCGlobalProperty *prop = asNEW(asCGlobalProperty);
	if( prop == 0 )
	{
		// Out of memory
		return 0;
	}

	// First check the availability of a free slot
	if( freeGlobalPropertyIds.GetLength() )
	{
		prop->id = freeGlobalPropertyIds.PopLast();
		globalProperties[prop->id] = prop;
		return prop;
	}

	prop->id = (asUINT)globalProperties.GetLength();
	globalProperties.PushLast(prop);
	return prop;
}

// internal
void asCScriptEngine::RemoveGlobalProperty(asCGlobalProperty *prop)
{
	int index = globalProperties.IndexOf(prop);
	if( index >= 0 )
	{
		freeGlobalPropertyIds.PushLast(index);
		globalProperties[index] = 0;

		asSMapNode<void*, asCGlobalProperty*> *node;
		varAddressMap.MoveTo(&node, prop->GetAddressOfValue());
		asASSERT(node);
		if( node )
			varAddressMap.Erase(node);

		prop->Release();
	}
}

// interface
asUINT asCScriptEngine::GetGlobalPropertyCount() const
{
	return asUINT(registeredGlobalProps.GetSize());
}

// interface
// TODO: If the typeId ever encodes the const flag, then the isConst parameter should be removed
int asCScriptEngine::GetGlobalPropertyByIndex(asUINT index, const char **name, const char **nameSpace, int *typeId, bool *isConst, const char **configGroup, void **pointer, asDWORD *accessMask) const
{
	const asCGlobalProperty *prop = registeredGlobalProps.Get(index);
	if( !prop )
		return asINVALID_ARG;

	if( name )       *name       = prop->name.AddressOf();
	if( nameSpace )  *nameSpace  = prop->nameSpace->name.AddressOf();
	if( typeId )     *typeId     = GetTypeIdFromDataType(prop->type);
	if( isConst )    *isConst    = prop->type.IsReadOnly();
	if( pointer )    *pointer    = prop->GetRegisteredAddress();
	if( accessMask ) *accessMask = prop->accessMask;

	if( configGroup )
	{
		asCConfigGroup *group = FindConfigGroupForGlobalVar(index);
		if( group )
			*configGroup = group->groupName.AddressOf();
		else
			*configGroup = 0;
	}

	return asSUCCESS;
}

// interface
int asCScriptEngine::GetGlobalPropertyIndexByName(const char *in_name) const
{
	asCString name;
	asSNameSpace *ns = 0;
	if( DetermineNameAndNamespace(in_name, defaultNamespace, name, ns) < 0 )
		return asINVALID_ARG;
			
	// Find the global var id
	while( ns )
	{
		int id = registeredGlobalProps.GetFirstIndex(ns, name);
		if( id >= 0 )
			return id;

		// Recursively search parent namespace
		ns = GetParentNameSpace(ns);
	}

	return asNO_GLOBAL_VAR;
}

// interface
int asCScriptEngine::GetGlobalPropertyIndexByDecl(const char *decl) const
{
	// This const cast is OK. The builder won't modify the engine
	asCBuilder bld(const_cast<asCScriptEngine*>(this), 0);

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCString name;
	asSNameSpace *ns;
	asCDataType dt;
	int r = bld.ParseVariableDeclaration(decl, defaultNamespace, name, ns, dt);
	if( r < 0 )
		return r;

	// Search for a match
	while( ns )
	{
		int id = registeredGlobalProps.GetFirstIndex(ns, name, asCCompGlobPropType(dt));
		if( id >= 0 )
			return id;

		ns = GetParentNameSpace(ns);
	}

	return asNO_GLOBAL_VAR;
}

// interface
int asCScriptEngine::RegisterObjectMethod(const char *obj, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary, int compositeOffset, bool isCompositeIndirect)
{
	if( obj == 0 )
		return ConfigError(asINVALID_ARG, "RegisterObjectMethod", obj, declaration);

	// Determine the object type
	asCDataType dt;
	asCBuilder bld(this, 0);
	int r = bld.ParseDataType(obj, &dt, defaultNamespace);
	if( r < 0 )
		return ConfigError(r, "RegisterObjectMethod", obj, declaration);

	// Don't allow application to modify primitives or handles
	if( dt.GetTypeInfo() == 0 || (dt.IsObjectHandle() && !(dt.GetTypeInfo()->GetFlags() & asOBJ_IMPLICIT_HANDLE)))
		return ConfigError(asINVALID_ARG, "RegisterObjectMethod", obj, declaration);

	// Don't allow application to modify built-in types or funcdefs
	if( dt.GetTypeInfo() == &functionBehaviours ||
		dt.GetTypeInfo() == &scriptTypeBehaviours ||
		CastToFuncdefType(dt.GetTypeInfo()) )
		return ConfigError(asINVALID_ARG, "RegisterObjectMethod", obj, declaration);

	// Don't allow modifying generated template instances
	if( dt.GetTypeInfo() && (dt.GetTypeInfo()->flags & asOBJ_TEMPLATE) && generatedTemplateTypes.Exists(CastToObjectType(dt.GetTypeInfo())) )
		return ConfigError(asINVALID_TYPE, "RegisterObjectMethod", obj, declaration);

	return RegisterMethodToObjectType(CastToObjectType(dt.GetTypeInfo()), declaration, funcPointer, callConv, auxiliary, compositeOffset, isCompositeIndirect);
}

// internal
int asCScriptEngine::RegisterMethodToObjectType(asCObjectType *objectType, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary, int compositeOffset, bool isCompositeIndirect)
{
#ifdef AS_MAX_PORTABILITY
	if( callConv != asCALL_GENERIC )
		return ConfigError(asNOT_SUPPORTED, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
#endif

	asSSystemFunctionInterface internal;
	int r = DetectCallingConvention(true, funcPointer, callConv, auxiliary, &internal);
	if( r < 0 )
		return ConfigError(r, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);

	internal.compositeOffset = compositeOffset;
	internal.isCompositeIndirect = isCompositeIndirect;
	if( (compositeOffset || isCompositeIndirect) && callConv != asCALL_THISCALL )
		return ConfigError(asINVALID_ARG, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);

	// TODO: cleanup: This is identical to what is in RegisterMethodToObjectType
	// If the object type is a template, make sure there are no generated instances already
	if( objectType->flags & asOBJ_TEMPLATE )
	{
		for( asUINT n = 0; n < generatedTemplateTypes.GetLength(); n++ )
		{
			asCObjectType *tmpl = generatedTemplateTypes[n];
			if( tmpl->name == objectType->name &&
				tmpl->nameSpace == objectType->nameSpace &&
				!(tmpl->templateSubTypes[0].GetTypeInfo() && (tmpl->templateSubTypes[0].GetTypeInfo()->flags & asOBJ_TEMPLATE_SUBTYPE)) )
			{
				asCString msg;
				msg.Format(TXT_TEMPLATE_s_ALREADY_GENERATED_CANT_REGISTER, asCDataType::CreateType(tmpl, false).Format(tmpl->nameSpace).AddressOf());
				WriteMessage("",0,0, asMSGTYPE_ERROR, msg.AddressOf());
				return ConfigError(asERROR, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
			}
		}
	}

	isPrepared = false;

	// Put the system function in the list of system functions
	asSSystemFunctionInterface *newInterface = asNEW(asSSystemFunctionInterface)(internal);
	if( newInterface == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);

	asCScriptFunction *func = asNEW(asCScriptFunction)(this, 0, asFUNC_SYSTEM);
	if( func == 0 )
	{
		asDELETE(newInterface, asSSystemFunctionInterface);
		return ConfigError(asOUT_OF_MEMORY, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
	}

	func->sysFuncIntf = newInterface;
	func->objectType  = objectType;
	func->objectType->AddRefInternal();

	asCBuilder bld(this, 0);
	r = bld.ParseFunctionDeclaration(func->objectType, declaration, func, true, &newInterface->paramAutoHandles, &newInterface->returnAutoHandle);
	if( r < 0 )
	{
		// Set as dummy function before deleting
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asINVALID_DECLARATION, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
	}

	// Check name conflicts
	r = bld.CheckNameConflictMember(objectType, func->name.AddressOf(), 0, 0, false, false);
	if( r < 0 )
	{
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asNAME_TAKEN, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
	}

	// Validate property signature
	if( func->IsProperty() && (r = bld.ValidateVirtualProperty(func)) < 0 )
	{
		// Set as dummy function before deleting
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		if( r == -5 )
			return ConfigError(asNAME_TAKEN, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
		else
			return ConfigError(asINVALID_DECLARATION, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
	}
	
	// Check against duplicate methods
	if( func->name == "opConv" || func->name == "opImplConv" || func->name == "opCast" || func->name == "opImplCast" )
	{
		// opConv and opCast are special methods that the compiler differentiates between by the return type
		for( asUINT n = 0; n < func->objectType->methods.GetLength(); n++ )
		{
			asCScriptFunction *f = scriptFunctions[func->objectType->methods[n]];
			if( f->name == func->name &&
				f->IsSignatureExceptNameEqual(func) )
			{
				func->funcType = asFUNC_DUMMY;
				asDELETE(func,asCScriptFunction);
				return ConfigError(asALREADY_REGISTERED, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
			}
		}
	}
	else
	{
		for( asUINT n = 0; n < func->objectType->methods.GetLength(); n++ )
		{
			asCScriptFunction *f = scriptFunctions[func->objectType->methods[n]];
			if( f->name == func->name &&
				f->IsSignatureExceptNameAndReturnTypeEqual(func) )
			{
				func->funcType = asFUNC_DUMMY;
				asDELETE(func,asCScriptFunction);
				return ConfigError(asALREADY_REGISTERED, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);
			}
		}
	}

	func->id = GetNextScriptFunctionId();
	func->objectType->methods.PushLast(func->id);
	func->accessMask = defaultAccessMask;
	AddScriptFunction(func);

	// If parameter type from other groups are used, add references
	currentGroup->AddReferencesForFunc(this, func);

	// Check if the method restricts that use of the template to value types or reference types
	if( func->objectType->flags & asOBJ_TEMPLATE )
	{
		r = SetTemplateRestrictions(func->objectType, func, "RegisterObjectMethod", declaration);
		if (r < 0)
			return r;
	}

	// TODO: beh.copy member will be removed, so this is not necessary
	// Is this the default copy behaviour?
	if( func->name == "opAssign" && func->parameterTypes.GetLength() == 1 && !func->IsReadOnly() &&
		((objectType->flags & asOBJ_SCRIPT_OBJECT) || func->parameterTypes[0].IsEqualExceptRefAndConst(asCDataType::CreateType(func->objectType, false))) )
	{
		if( func->objectType->beh.copy != 0 )
			return ConfigError(asALREADY_REGISTERED, "RegisterObjectMethod", objectType->name.AddressOf(), declaration);

		func->objectType->beh.copy = func->id;
		func->AddRefInternal();
	}

	// Return the function id as success
	return func->id;
}

// interface
int asCScriptEngine::RegisterGlobalFunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *auxiliary)
{
#ifdef AS_MAX_PORTABILITY
	if( callConv != asCALL_GENERIC )
		return ConfigError(asNOT_SUPPORTED, "RegisterGlobalFunction", declaration, 0);
#endif

	asSSystemFunctionInterface internal;
	int r = DetectCallingConvention(false, funcPointer, callConv, auxiliary, &internal);
	if( r < 0 )
		return ConfigError(r, "RegisterGlobalFunction", declaration, 0);

	isPrepared = false;

	// Put the system function in the list of system functions
	asSSystemFunctionInterface *newInterface = asNEW(asSSystemFunctionInterface)(internal);
	if( newInterface == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterGlobalFunction", declaration, 0);

	asCScriptFunction *func = asNEW(asCScriptFunction)(this, 0, asFUNC_SYSTEM);
	if( func == 0 )
	{
		asDELETE(newInterface, asSSystemFunctionInterface);
		return ConfigError(asOUT_OF_MEMORY, "RegisterGlobalFunction", declaration, 0);
	}

	func->sysFuncIntf = newInterface;

	asCBuilder bld(this, 0);
	r = bld.ParseFunctionDeclaration(0, declaration, func, true, &newInterface->paramAutoHandles, &newInterface->returnAutoHandle, defaultNamespace);
	if( r < 0 )
	{
		// Set as dummy function before deleting
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asINVALID_DECLARATION, "RegisterGlobalFunction", declaration, 0);
	}

	// TODO: namespace: What if the declaration defined an explicit namespace?
	func->nameSpace = defaultNamespace;

	// Check name conflicts
	r = bld.CheckNameConflict(func->name.AddressOf(), 0, 0, defaultNamespace, false, false);
	if( r < 0 )
	{
		// Set as dummy function before deleting
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asNAME_TAKEN, "RegisterGlobalFunction", declaration, 0);
	}
	
	// Validate property signature
	if( func->IsProperty() && (r = bld.ValidateVirtualProperty(func)) < 0 )
	{
		// Set as dummy function before deleting
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		if( r == -5 )
			return ConfigError(asNAME_TAKEN, "RegisterGlobalFunction", declaration, 0);
		else
			return ConfigError(asINVALID_DECLARATION, "RegisterGlobalFunction", declaration, 0);
	}

	// Make sure the function is not identical to a previously registered function
	asUINT n;
	const asCArray<unsigned int> &idxs = registeredGlobalFuncs.GetIndexes(func->nameSpace, func->name);
	for( n = 0; n < idxs.GetLength(); n++ )
	{
		asCScriptFunction *f = registeredGlobalFuncs.Get(idxs[n]);
		if( f->IsSignatureExceptNameAndReturnTypeEqual(func) )
		{
			func->funcType = asFUNC_DUMMY;
			asDELETE(func,asCScriptFunction);
			return ConfigError(asALREADY_REGISTERED, "RegisterGlobalFunction", declaration, 0);
		}
	}

	func->id = GetNextScriptFunctionId();
	AddScriptFunction(func);

	currentGroup->scriptFunctions.PushLast(func);
	func->accessMask = defaultAccessMask;
	registeredGlobalFuncs.Put(func);

	// If parameter type from other groups are used, add references
	currentGroup->AddReferencesForFunc(this, func);

	// Return the function id as success
	return func->id;
}

// interface
asUINT asCScriptEngine::GetGlobalFunctionCount() const
{
	// Don't count the builtin delegate factory
	return asUINT(registeredGlobalFuncs.GetSize()-1);
}

// interface
asIScriptFunction *asCScriptEngine::GetGlobalFunctionByIndex(asUINT index) const
{
	// Don't count the builtin delegate factory
	index++;

	if( index >= registeredGlobalFuncs.GetSize() )
		return 0;

	return static_cast<asIScriptFunction*>(const_cast<asCScriptFunction*>(registeredGlobalFuncs.Get(index)));
}

// interface
asIScriptFunction *asCScriptEngine::GetGlobalFunctionByDecl(const char *decl) const
{
	asCBuilder bld(const_cast<asCScriptEngine*>(this), 0);

	// Don't write parser errors to the message callback
	bld.silent = true;

	asCScriptFunction func(const_cast<asCScriptEngine*>(this), 0, asFUNC_DUMMY);
	int r = bld.ParseFunctionDeclaration(0, decl, &func, false, 0, 0, defaultNamespace);
	if( r < 0 )
		return 0;

	asSNameSpace *ns = defaultNamespace;
	// Search script functions for matching interface
	while( ns )
	{
		asIScriptFunction *f = 0;
		const asCArray<unsigned int> &idxs = registeredGlobalFuncs.GetIndexes(ns, func.name);
		for( unsigned int n = 0; n < idxs.GetLength(); n++ )
		{
			const asCScriptFunction *funcPtr = registeredGlobalFuncs.Get(idxs[n]);
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

		// Recursively search parent namespaces
		ns = GetParentNameSpace(ns);
	}

	return 0;
}


asCTypeInfo *asCScriptEngine::GetRegisteredType(const asCString &type, asSNameSpace *ns) const
{
	asSMapNode<asSNameSpaceNamePair, asCTypeInfo *> *cursor;
	if( allRegisteredTypes.MoveTo(&cursor, asSNameSpaceNamePair(ns, type)) )
		return cursor->value;

	return 0;
}




void asCScriptEngine::PrepareEngine()
{
	if( isPrepared ) return;
	if( configFailed ) return;

	asUINT n;
	for( n = 0; n < scriptFunctions.GetLength(); n++ )
	{
		// Determine the host application interface
		if( scriptFunctions[n] && scriptFunctions[n]->funcType == asFUNC_SYSTEM )
		{
			if( scriptFunctions[n]->sysFuncIntf->callConv == ICC_GENERIC_FUNC ||
				scriptFunctions[n]->sysFuncIntf->callConv == ICC_GENERIC_METHOD )
				PrepareSystemFunctionGeneric(scriptFunctions[n], scriptFunctions[n]->sysFuncIntf, this);
			else
				PrepareSystemFunction(scriptFunctions[n], scriptFunctions[n]->sysFuncIntf, this);
		}
	}

	// Validate object type registrations
	for( n = 0; n < registeredObjTypes.GetLength(); n++ )
	{
		asCObjectType *type = registeredObjTypes[n];
		if( type && !(type->flags & asOBJ_SCRIPT_OBJECT) )
		{
			bool missingBehaviour = false;
			const char *infoMsg = 0;

			// Verify that GC types have all behaviours
			if( type->flags & asOBJ_GC )
			{
				if (type->flags & asOBJ_REF)
				{
					if (type->beh.addref == 0 ||
						type->beh.release == 0 ||
						type->beh.gcGetRefCount == 0 ||
						type->beh.gcSetFlag == 0 ||
						type->beh.gcGetFlag == 0 ||
						type->beh.gcEnumReferences == 0 ||
						type->beh.gcReleaseAllReferences == 0)
					{
						infoMsg = TXT_GC_REQUIRE_ADD_REL_GC_BEHAVIOUR;
						missingBehaviour = true;
					}
				}
				else
				{
					if (type->beh.gcEnumReferences == 0)
					{
						infoMsg = TXT_VALUE_GC_REQUIRE_GC_BEHAVIOUR;
						missingBehaviour = true;
					}
				}
			}
			// Verify that scoped ref types have the release behaviour
			if( type->flags & asOBJ_SCOPED )
			{
				if( type->beh.release == 0 )
				{
					infoMsg = TXT_SCOPE_REQUIRE_REL_BEHAVIOUR;
					missingBehaviour = true;
				}
			}
			// Verify that ref types have add ref and release behaviours
			if( (type->flags & asOBJ_REF) &&
				!(type->flags & asOBJ_SCOPED) &&
				!(type->flags & asOBJ_NOHANDLE) &&
				!(type->flags & asOBJ_NOCOUNT) )
			{
				if( type->beh.addref  == 0 ||
					type->beh.release == 0 )
				{
					infoMsg = TXT_REF_REQUIRE_ADD_REL_BEHAVIOUR;
					missingBehaviour = true;
				}
			}
			// Verify that non-pod value types have the constructor and destructor registered
			if( (type->flags & asOBJ_VALUE) &&
				!(type->flags & asOBJ_POD) )
			{
				if( type->beh.constructors.GetLength() == 0 ||
					type->beh.destruct  == 0 )
				{
					infoMsg = TXT_NON_POD_REQUIRE_CONSTR_DESTR_BEHAVIOUR;
					missingBehaviour = true;
				}
			}

			if( missingBehaviour )
			{
				asCString str;
				str.Format(TXT_TYPE_s_IS_MISSING_BEHAVIOURS, type->name.AddressOf());
				WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
				WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, infoMsg);
				ConfigError(asINVALID_CONFIGURATION, 0, 0, 0);
			}
		}
	}

	isPrepared = true;
}

int asCScriptEngine::ConfigError(int err, const char *funcName, const char *arg1, const char *arg2)
{
	configFailed = true;
	if( funcName )
	{
		asCString str;
		if( arg1 )
		{
			if( arg2 )
				str.Format(TXT_FAILED_IN_FUNC_s_WITH_s_AND_s_s_d, funcName, arg1, arg2, errorNames[-err], err);
			else
				str.Format(TXT_FAILED_IN_FUNC_s_WITH_s_s_d, funcName, arg1, errorNames[-err], err);
		}
		else
			str.Format(TXT_FAILED_IN_FUNC_s_s_d, funcName, errorNames[-err], err);

		WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
	}
	return err;
}

// interface
int asCScriptEngine::RegisterDefaultArrayType(const char *type)
{
	asCBuilder bld(this, 0);
	asCDataType dt;
	int r = bld.ParseDataType(type, &dt, defaultNamespace);
	if( r < 0 ) return r;

	if( dt.GetTypeInfo() == 0 ||
		!(dt.GetTypeInfo()->GetFlags() & asOBJ_TEMPLATE) )
		return asINVALID_TYPE;

	defaultArrayObjectType = CastToObjectType(dt.GetTypeInfo());
	defaultArrayObjectType->AddRefInternal();

	return 0;
}

// interface
int asCScriptEngine::GetDefaultArrayTypeId() const
{
	if( defaultArrayObjectType )
		return GetTypeIdFromDataType(asCDataType::CreateType(defaultArrayObjectType, false));

	return asINVALID_TYPE;
}

// interface
int asCScriptEngine::RegisterStringFactory(const char *datatype, asIStringFactory *factory)
{
	if (factory == 0)
		return ConfigError(asINVALID_ARG, "RegisterStringFactory", datatype, 0);

	// Parse the data type
	asCBuilder bld(this, 0);
	asCDataType dt;
	int r = bld.ParseDataType(datatype, &dt, defaultNamespace, true);
	if (r < 0)
		return ConfigError(asINVALID_TYPE, "RegisterStringFactory", datatype, 0);

	// Validate the type. It must not be reference or handle
	if (dt.IsReference() || dt.IsObjectHandle())
		return ConfigError(asINVALID_TYPE, "RegisterStringFactory", datatype, 0);

	// All string literals will be treated as const
	dt.MakeReadOnly(true);

	stringType = dt;
	stringFactory = factory;

	return asSUCCESS;
}

// interface
int asCScriptEngine::GetStringFactoryReturnTypeId(asDWORD *flags) const
{
	if( stringFactory == 0 )
		return asNO_FUNCTION;

	if( flags )
		*flags = 0;
	return GetTypeIdFromDataType(stringType);
}

// internal
asCModule *asCScriptEngine::GetModule(const char *name, bool create)
{
	// Accept null as well as zero-length string
	if( name == 0 ) name = "";

	asCModule *retModule = 0;

	ACQUIRESHARED(engineRWLock);
	if( lastModule && lastModule->m_name == name )
		retModule = lastModule;
	else
	{
		// TODO: optimize: Improve linear search
		for( asUINT n = 0; n < scriptModules.GetLength(); ++n )
			if( scriptModules[n] && scriptModules[n]->m_name == name )
			{
				retModule = scriptModules[n];
				break;
			}
	}
	RELEASESHARED(engineRWLock);

	if( retModule )
	{
		ACQUIREEXCLUSIVE(engineRWLock);
		lastModule = retModule;
		RELEASEEXCLUSIVE(engineRWLock);

		return retModule;
	}

	if( create )
	{
		retModule = asNEW(asCModule)(name, this);
		if( retModule == 0 )
		{
			// Out of memory
			return 0;
		}

		ACQUIREEXCLUSIVE(engineRWLock);
		scriptModules.PushLast(retModule);
		lastModule = retModule;
		RELEASEEXCLUSIVE(engineRWLock);
	}

	return retModule;
}

asCModule *asCScriptEngine::GetModuleFromFuncId(int id)
{
	if( id < 0 ) return 0;
	if( id >= (int)scriptFunctions.GetLength() ) return 0;
	asCScriptFunction *func = scriptFunctions[id];
	if( func == 0 ) return 0;
	return func->module;
}

// internal
int asCScriptEngine::RequestBuild()
{
	ACQUIREEXCLUSIVE(engineRWLock);
	if( isBuilding )
	{
		RELEASEEXCLUSIVE(engineRWLock);
		return asBUILD_IN_PROGRESS;
	}
	isBuilding = true;
	RELEASEEXCLUSIVE(engineRWLock);

	return 0;
}

// internal
void asCScriptEngine::BuildCompleted()
{
	// Always free up pooled memory after a completed build
	memoryMgr.FreeUnusedMemory();

	isBuilding = false;
}

void asCScriptEngine::RemoveTemplateInstanceType(asCObjectType *t)
{
	// If there is a module that still owns the generated type, then don't remove it
	if( t->module )
		return;

	// Don't remove it if there are external refernces
	if( t->externalRefCount.get() )
		return;

	// Only remove the template instance type if no config group is using it
	if( defaultGroup.generatedTemplateInstances.Exists(t) )
		return;
	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
		if( configGroups[n]->generatedTemplateInstances.Exists(t) )
			return;

	t->DestroyInternal();
	templateInstanceTypes.RemoveValue(t);
	generatedTemplateTypes.RemoveValue(t);
	t->ReleaseInternal();
}

// internal
asCObjectType *asCScriptEngine::GetTemplateInstanceType(asCObjectType *templateType, asCArray<asCDataType> &subTypes, asCModule *requestingModule)
{
	asUINT n;

	// Is there any template instance type or template specialization already with this subtype?
	for( n = 0; n < templateInstanceTypes.GetLength(); n++ )
	{
		asCObjectType *type = templateInstanceTypes[n];
		if( type &&
			type->name == templateType->name &&
			type->nameSpace == templateType->nameSpace &&
			type->templateSubTypes == subTypes )
		{
			// If the template instance is generated, then the module should hold a reference
			// to it so the config group can determine see that the template type is in use.
			// Template specializations will be treated as normal types
			if( requestingModule && generatedTemplateTypes.Exists(type) )
			{
				if( type->module == 0 )
				{
					// Set the ownership of this template type
					// It may be without ownership if it was previously created from application with for example GetTypeInfoByDecl
					type->module = requestingModule;
				}
				if( !requestingModule->m_templateInstances.Exists(type) )
				{
					requestingModule->m_templateInstances.PushLast(type);
					type->AddRefInternal();
				}
			}

			return templateInstanceTypes[n];
		}
	}

	// No previous template instance exists

	// Make sure this template supports the subtype
	for( n = 0; n < subTypes.GetLength(); n++ )
	{
		if( !templateType->acceptValueSubType && (subTypes[n].IsPrimitive() || (subTypes[n].GetTypeInfo()->flags & asOBJ_VALUE)) )
			return 0;

		if( !templateType->acceptRefSubType && (subTypes[n].IsObject() && (subTypes[n].GetTypeInfo()->flags & asOBJ_REF)) )
			return 0;
	}

	// Create a new template instance type based on the templateType
	asCObjectType *ot = asNEW(asCObjectType)(this);
	if( ot == 0 )
	{
		// Out of memory
		return 0;
	}

	ot->templateSubTypes = subTypes;
	ot->flags            = templateType->flags;
	ot->size             = templateType->size;
	ot->name             = templateType->name;
	ot->nameSpace        = templateType->nameSpace;

	// If the template is being requested from a module, then the module should hold a reference to the type
	if( requestingModule )
	{
		// Set the ownership of this template type
		ot->module = requestingModule;
		requestingModule->m_templateInstances.PushLast(ot);
		ot->AddRefInternal();
	}
	else
	{
		// If the template type is not requested directly from a module, then set the ownership
		// of it to the same module as one of the subtypes. If none of the subtypes are owned by]
		// any module, the template instance will be without ownership and can be removed from the
		// engine at any time (unless the application holds an external reference).
		for( n = 0; n < subTypes.GetLength(); n++ )
		{
			if( subTypes[n].GetTypeInfo() )
			{
				ot->module = subTypes[n].GetTypeInfo()->module;
				if( ot->module )
				{
					ot->module->m_templateInstances.PushLast(ot);
					ot->AddRefInternal();
					break;
				}
			}
		}
	}

	// Before filling in the methods, call the template instance callback behaviour to validate the type
	if( templateType->beh.templateCallback )
	{
		// If the validation is deferred then the validation will be done later,
		// so it is necessary to continue the preparation of the template instance type
		if( !deferValidationOfTemplateTypes )
		{
			asCScriptFunction *callback = scriptFunctions[templateType->beh.templateCallback];

			bool dontGarbageCollect = false;
			if( !CallGlobalFunctionRetBool(ot, &dontGarbageCollect, callback->sysFuncIntf, callback) )
			{
				// The type cannot be instantiated
				ot->templateSubTypes.SetLength(0);
				if( ot->module )
				{
					ot->module->m_templateInstances.RemoveValue(ot);
					ot->ReleaseInternal();
				}
				ot->ReleaseInternal();
				return 0;
			}

			// If the callback said this template instance won't be garbage collected then remove the flag
			if( dontGarbageCollect )
				ot->flags &= ~asOBJ_GC;
		}

		ot->beh.templateCallback = templateType->beh.templateCallback;
		scriptFunctions[ot->beh.templateCallback]->AddRefInternal();
	}

	ot->methods = templateType->methods;
	for( n = 0; n < ot->methods.GetLength(); n++ )
		scriptFunctions[ot->methods[n]]->AddRefInternal();

	if( templateType->flags & asOBJ_REF )
	{
		// Store the real factory in the constructor. This is used by the CreateScriptObject function.
		// Otherwise it wouldn't be necessary to store the real factory ids.
		ot->beh.construct = templateType->beh.factory;
		ot->beh.constructors = templateType->beh.factories;
	}
	else
	{
		ot->beh.construct = templateType->beh.construct;
		ot->beh.constructors = templateType->beh.constructors;
	}
	for( n = 0; n < ot->beh.constructors.GetLength(); n++ )
		scriptFunctions[ot->beh.constructors[n]]->AddRefInternal();


	// Before proceeding with the generation of the template functions for the template instance it is necessary
	// to include the new template instance type in the list of known types, otherwise it is possible that we get
	// a infinite recursive loop as the template instance type is requested again during the generation of the
	// template functions.
	templateInstanceTypes.PushLast(ot);

	// Store the template instance types that have been created automatically by the engine from a template type
	// The object types in templateInstanceTypes that are not also in generatedTemplateTypes are registered template specializations
	generatedTemplateTypes.PushLast(ot);

	// Any child funcdefs must be copied to the template instance (with adjustments in case of template subtypes)
	// This must be done before resolving other methods, to make sure the other methods that may refer to the
	// templated funcdef will resolve to the new funcdef
	for (n = 0; n < templateType->childFuncDefs.GetLength(); n++)
	{
		asCFuncdefType *funcdef = GenerateNewTemplateFuncdef(templateType, ot, templateType->childFuncDefs[n]);
		funcdef->parentClass = ot;
		ot->childFuncDefs.PushLast(funcdef);
	}

	// As the new template type is instantiated the engine should
	// generate new functions to substitute the ones with the template subtype.
	for( n = 0; n < ot->beh.constructors.GetLength(); n++ )
	{
		int funcId = ot->beh.constructors[n];
		asCScriptFunction *func = scriptFunctions[funcId];

		if( GenerateNewTemplateFunction(templateType, ot, func, &func) )
		{
			// Release the old function, the new one already has its ref count set to 1
			scriptFunctions[funcId]->ReleaseInternal();
			ot->beh.constructors[n] = func->id;

			if( ot->beh.construct == funcId )
				ot->beh.construct = func->id;
		}
	}

	ot->beh.factory = 0;

	if( templateType->flags & asOBJ_REF )
	{
		// Generate factory stubs for each of the factories
		for( n = 0; n < ot->beh.constructors.GetLength(); n++ )
		{
			asCScriptFunction *func = GenerateTemplateFactoryStub(templateType, ot, ot->beh.constructors[n]);

			ot->beh.factories.PushLast(func->id);

			// Set the default factory as well
			if( ot->beh.constructors[n] == ot->beh.construct )
				ot->beh.factory = func->id;
		}
	}
	else
	{
		// Generate factory stubs for each of the constructors
		for( n = 0; n < ot->beh.constructors.GetLength(); n++ )
		{
			asCScriptFunction *func = GenerateTemplateFactoryStub(templateType, ot, ot->beh.constructors[n]);

			if( ot->beh.constructors[n] == ot->beh.construct )
				ot->beh.construct = func->id;

			// Release previous constructor
			scriptFunctions[ot->beh.constructors[n]]->ReleaseInternal();

			ot->beh.constructors[n] = func->id;
		}
	}

	// Generate stub for the list factory as well
	if( templateType->beh.listFactory )
	{
		asCScriptFunction *func = GenerateTemplateFactoryStub(templateType, ot, templateType->beh.listFactory);

		// Rename the function to easily identify it in LoadByteCode
		func->name = "$list";

		ot->beh.listFactory = func->id;
	}

	ot->beh.addref = templateType->beh.addref;
	if( scriptFunctions[ot->beh.addref] ) scriptFunctions[ot->beh.addref]->AddRefInternal();
	ot->beh.release = templateType->beh.release;
	if( scriptFunctions[ot->beh.release] ) scriptFunctions[ot->beh.release]->AddRefInternal();
	ot->beh.destruct = templateType->beh.destruct;
	if( scriptFunctions[ot->beh.destruct] ) scriptFunctions[ot->beh.destruct]->AddRefInternal();
	ot->beh.copy = templateType->beh.copy;
	if( scriptFunctions[ot->beh.copy] ) scriptFunctions[ot->beh.copy]->AddRefInternal();
	ot->beh.gcGetRefCount = templateType->beh.gcGetRefCount;
	if( scriptFunctions[ot->beh.gcGetRefCount] ) scriptFunctions[ot->beh.gcGetRefCount]->AddRefInternal();
	ot->beh.gcSetFlag = templateType->beh.gcSetFlag;
	if( scriptFunctions[ot->beh.gcSetFlag] ) scriptFunctions[ot->beh.gcSetFlag]->AddRefInternal();
	ot->beh.gcGetFlag = templateType->beh.gcGetFlag;
	if( scriptFunctions[ot->beh.gcGetFlag] ) scriptFunctions[ot->beh.gcGetFlag]->AddRefInternal();
	ot->beh.gcEnumReferences = templateType->beh.gcEnumReferences;
	if( scriptFunctions[ot->beh.gcEnumReferences] ) scriptFunctions[ot->beh.gcEnumReferences]->AddRefInternal();
	ot->beh.gcReleaseAllReferences = templateType->beh.gcReleaseAllReferences;
	if( scriptFunctions[ot->beh.gcReleaseAllReferences] ) scriptFunctions[ot->beh.gcReleaseAllReferences]->AddRefInternal();
	ot->beh.getWeakRefFlag = templateType->beh.getWeakRefFlag;
	if( scriptFunctions[ot->beh.getWeakRefFlag] ) scriptFunctions[ot->beh.getWeakRefFlag]->AddRefInternal();

	// As the new template type is instantiated, the engine should
	// generate new functions to substitute the ones with the template subtype.
	for( n = 0; n < ot->methods.GetLength(); n++ )
	{
		int funcId = ot->methods[n];
		asCScriptFunction *func = scriptFunctions[funcId];

		if( GenerateNewTemplateFunction(templateType, ot, func, &func) )
		{
			// Release the old function, the new one already has its ref count set to 1
			scriptFunctions[funcId]->ReleaseInternal();
			ot->methods[n] = func->id;
		}
	}

	// Increase ref counter for sub type if it is an object type
	for( n = 0; n < ot->templateSubTypes.GetLength(); n++ )
		if( ot->templateSubTypes[n].GetTypeInfo() )
			ot->templateSubTypes[n].GetTypeInfo()->AddRefInternal();

	// Copy the properties to the template instance
	for( n = 0; n < templateType->properties.GetLength(); n++ )
	{
		asCObjectProperty *prop = templateType->properties[n];
		ot->properties.PushLast(asNEW(asCObjectProperty)(*prop));
		if( prop->type.GetTypeInfo() )
			prop->type.GetTypeInfo()->AddRefInternal();
	}

	return ot;
}

// interface
asILockableSharedBool *asCScriptEngine::GetWeakRefFlagOfScriptObject(void *obj, const asITypeInfo *type) const
{
	// Make sure it is not a null pointer
	if( obj == 0 || type == 0 ) return 0;

	const asCObjectType *objType = static_cast<const asCObjectType *>(type);
	asILockableSharedBool *dest = 0;
	if( objType->beh.getWeakRefFlag )
	{
		// Call the getweakrefflag behaviour
		dest = reinterpret_cast<asILockableSharedBool*>(CallObjectMethodRetPtr(obj, objType->beh.getWeakRefFlag));
	}
	return dest;
}

// internal
// orig is the parameter type that is to be replaced
// tmpl is the registered template. Used to find which subtype is being replaced
// ot is the new template instance that is being created. Used to find the target type
asCDataType asCScriptEngine::DetermineTypeForTemplate(const asCDataType &orig, asCObjectType *tmpl, asCObjectType *ot)
{
	asCDataType dt;
	if( orig.GetTypeInfo() && (orig.GetTypeInfo()->flags & asOBJ_TEMPLATE_SUBTYPE) )
	{
		bool found = false;
		for( asUINT n = 0; n < tmpl->templateSubTypes.GetLength(); n++ )
		{
			if( orig.GetTypeInfo() == tmpl->templateSubTypes[n].GetTypeInfo() )
			{
				found = true;
				dt = ot->templateSubTypes[n];
				if( orig.IsObjectHandle() && !ot->templateSubTypes[n].IsObjectHandle() )
				{
					dt.MakeHandle(true, true);
					asASSERT(dt.IsObjectHandle());
					if( orig.IsHandleToConst() )
						dt.MakeHandleToConst(true);
					dt.MakeReference(orig.IsReference());
					dt.MakeReadOnly(orig.IsReadOnly());
				}
				else
				{
					// The target type is a handle, then check if the application
					// wants this handle to be to a const object. This is done by
					// flagging the type with 'if_handle_then_const' in the declaration.
					if (dt.IsObjectHandle() && orig.HasIfHandleThenConst())
						dt.MakeHandleToConst(true);

					dt.MakeReference(orig.IsReference());
					dt.MakeReadOnly(ot->templateSubTypes[n].IsReadOnly() || orig.IsReadOnly());

					// If the target is a @& then don't make the handle const, 
					// as it is not possible to declare functions with @const &
					if (orig.IsReference() && dt.IsObjectHandle())
						dt.MakeReadOnly(false);
				}
				break;
			}
		}
		asASSERT( found );
		UNUSED_VAR( found );
	}
	else if( orig.GetTypeInfo() == tmpl )
	{
		if( orig.IsObjectHandle() )
			dt = asCDataType::CreateObjectHandle(ot, false);
		else
			dt = asCDataType::CreateType(ot, false);

		dt.MakeReference(orig.IsReference());
		dt.MakeReadOnly(orig.IsReadOnly());
	}
	else if( orig.GetTypeInfo() && (orig.GetTypeInfo()->flags & asOBJ_TEMPLATE) )
	{
		// The type is itself a template, so it is necessary to find the correct template instance type
		asCArray<asCDataType> tmplSubTypes;
		asCObjectType *origType = CastToObjectType(orig.GetTypeInfo());
		bool needInstance = true;

		// Find the matching replacements for the subtypes
		for( asUINT n = 0; n < origType->templateSubTypes.GetLength(); n++ )
		{
			if( origType->templateSubTypes[n].GetTypeInfo() == 0 ||
				!(origType->templateSubTypes[n].GetTypeInfo()->flags & asOBJ_TEMPLATE_SUBTYPE) )
			{
				// The template is already an instance so we shouldn't attempt to create another instance
				needInstance = false;
				break;
			}

			for( asUINT m = 0; m < tmpl->templateSubTypes.GetLength(); m++ )
				if( origType->templateSubTypes[n].GetTypeInfo() == tmpl->templateSubTypes[m].GetTypeInfo() )
					tmplSubTypes.PushLast(ot->templateSubTypes[m]);

			if( tmplSubTypes.GetLength() != n+1 )
			{
				asASSERT( false );
				return orig;
			}
		}

		asCObjectType *ntype = origType;
		if( needInstance )
		{
			// Always find the original template type when creating a new template instance otherwise the
			// generation will fail since it will attempt to create factory stubs when they already exists, etc
			for( asUINT n = 0; n < registeredTemplateTypes.GetLength(); n++ )
				if( registeredTemplateTypes[n]->name == origType->name &&
					registeredTemplateTypes[n]->nameSpace == origType->nameSpace )
				{
					origType = registeredTemplateTypes[n];
					break;
				}

			ntype = GetTemplateInstanceType(origType, tmplSubTypes, ot->module);
			if( ntype == 0 )
			{
				// It not possible to instantiate the subtype
				asASSERT( false );
				ntype = tmpl;
			}
		}

		if( orig.IsObjectHandle() )
			dt = asCDataType::CreateObjectHandle(ntype, false);
		else
			dt = asCDataType::CreateType(ntype, false);

		dt.MakeReference(orig.IsReference());
		dt.MakeReadOnly(orig.IsReadOnly());
	}
	else if (orig.GetTypeInfo() && (orig.GetTypeInfo()->flags & asOBJ_FUNCDEF) && CastToFuncdefType(orig.GetTypeInfo())->parentClass == tmpl)
	{
		// The type is a child funcdef. Find the corresponding child funcdef in the template instance
		for (asUINT n = 0; n < ot->childFuncDefs.GetLength(); n++)
		{
			if (ot->childFuncDefs[n]->name == orig.GetTypeInfo()->name)
			{
				dt = orig;
				dt.SetTypeInfo(ot->childFuncDefs[n]);
			}
		}
	}
	else
		dt = orig;

	return dt;
}

// internal
asCScriptFunction *asCScriptEngine::GenerateTemplateFactoryStub(asCObjectType *templateType, asCObjectType *ot, int factoryId)
{
	asCScriptFunction *factory = scriptFunctions[factoryId];

	// By first instantiating the function as a dummy and then changing it to be a script function
	// I avoid having it added to the garbage collector. As it is known that this object will stay
	// alive until the template instance is no longer used there is no need to have the GC check
	// this function all the time.
	asCScriptFunction *func = asNEW(asCScriptFunction)(this, 0, asFUNC_DUMMY);
	if( func == 0 )
	{
		// Out of memory
		return 0;
	}

	func->funcType = asFUNC_SCRIPT;
	func->AllocateScriptFunctionData();
	func->id = GetNextScriptFunctionId();
	AddScriptFunction(func);

	func->traits = factory->traits;
	func->SetShared(true);
	if( templateType->flags & asOBJ_REF )
	{
		func->name = "$fact";
		func->returnType = asCDataType::CreateObjectHandle(ot, false);
	}
	else
	{
		func->name = "$beh0";
		func->returnType = factory->returnType; // constructors return nothing
		func->objectType = ot;
		func->objectType->AddRefInternal();
	}

	// Skip the first parameter as this is the object type pointer that the stub will add
	func->parameterTypes.SetLength(factory->parameterTypes.GetLength()-1);
	func->parameterNames.SetLength(factory->parameterNames.GetLength()-1);
	func->inOutFlags.SetLength(factory->inOutFlags.GetLength()-1);
	func->defaultArgs.SetLength(factory->defaultArgs.GetLength()-1);
	for( asUINT p = 1; p < factory->parameterTypes.GetLength(); p++ )
	{
		func->parameterTypes[p-1] = factory->parameterTypes[p];
		func->parameterNames[p-1] = factory->parameterNames[p];
		func->inOutFlags[p-1] = factory->inOutFlags[p];
		func->defaultArgs[p-1] = factory->defaultArgs[p] ? asNEW(asCString)(*factory->defaultArgs[p]) : 0;
	}
	func->scriptData->objVariablesOnHeap = 0;

	// Generate the bytecode for the factory stub
	asUINT bcLength = asBCTypeSize[asBCInfo[asBC_OBJTYPE].type] +
	                  asBCTypeSize[asBCInfo[asBC_CALLSYS].type] +
	                  asBCTypeSize[asBCInfo[asBC_RET].type];

	if( ep.includeJitInstructions )
		bcLength += asBCTypeSize[asBCInfo[asBC_JitEntry].type];
	if( templateType->flags & asOBJ_VALUE )
		bcLength += asBCTypeSize[asBCInfo[asBC_SwapPtr].type];

	func->scriptData->byteCode.SetLength(bcLength);
	asDWORD *bc = func->scriptData->byteCode.AddressOf();

	if( ep.includeJitInstructions )
	{
		*(asBYTE*)bc = asBC_JitEntry;
		*(asPWORD*)(bc+1) = 0;
		bc += asBCTypeSize[asBCInfo[asBC_JitEntry].type];
	}

	*(asBYTE*)bc = asBC_OBJTYPE;
	*(asPWORD*)(bc+1) = (asPWORD)ot;
	bc += asBCTypeSize[asBCInfo[asBC_OBJTYPE].type];
	if( templateType->flags & asOBJ_VALUE )
	{
		// Swap the object pointer with the object type
		*(asBYTE*)bc = asBC_SwapPtr;
		bc += asBCTypeSize[asBCInfo[asBC_SwapPtr].type];
	}
	*(asBYTE*)bc = asBC_CALLSYS;
	*(asDWORD*)(bc+1) = factoryId;
	bc += asBCTypeSize[asBCInfo[asBC_CALLSYS].type];
	*(asBYTE*)bc = asBC_RET;
	*(((asWORD*)bc)+1) = (asWORD)func->GetSpaceNeededForArguments() + (func->objectType ? AS_PTR_SIZE : 0);

	func->AddReferences();
	func->scriptData->stackNeeded = AS_PTR_SIZE;

	// Tell the virtual machine not to clean up the object on exception
	func->dontCleanUpOnException = true;

	func->JITCompile();

	// Need to translate the list pattern too so the VM and compiler will know the correct type of the members
	if( factory->listPattern )
	{
		asSListPatternNode *n = factory->listPattern;
		asSListPatternNode *last = 0;
		while( n )
		{
			asSListPatternNode *newNode = n->Duplicate();
			if( newNode->type == asLPT_TYPE )
			{
				asSListPatternDataTypeNode *typeNode = reinterpret_cast<asSListPatternDataTypeNode*>(newNode);
				typeNode->dataType = DetermineTypeForTemplate(typeNode->dataType, templateType, ot);
			}

			if( last )
				last->next = newNode;
			else
				func->listPattern = newNode;

			last = newNode;

			n = n->next;
		}
	}

	return func;
}

bool asCScriptEngine::RequireTypeReplacement(asCDataType &type, asCObjectType *templateType)
{
	if( type.GetTypeInfo() == templateType ) return true;
	if( type.GetTypeInfo() && (type.GetTypeInfo()->flags & asOBJ_TEMPLATE_SUBTYPE) ) return true;
	if( type.GetTypeInfo() && (type.GetTypeInfo()->flags & asOBJ_TEMPLATE) )
	{
		asCObjectType *ot = CastToObjectType(type.GetTypeInfo());
		for( asUINT n = 0; n < ot->templateSubTypes.GetLength(); n++ )
			if( ot->templateSubTypes[n].GetTypeInfo() &&
				ot->templateSubTypes[n].GetTypeInfo()->flags & asOBJ_TEMPLATE_SUBTYPE )
				return true;
	}
	if (type.GetTypeInfo() && (type.GetTypeInfo()->flags & asOBJ_FUNCDEF) && CastToFuncdefType(type.GetTypeInfo())->parentClass == templateType)
		return true;

	return false;
}

bool asCScriptEngine::GenerateNewTemplateFunction(asCObjectType *templateType, asCObjectType *ot, asCScriptFunction *func, asCScriptFunction **newFunc)
{
	// Due to the objectType it is always required to generate a new function, 
	// even if none of the function arguments needs to be changed.
/*
	// TODO: Can we store the new function in some other optimized way to avoid
	//       duplicating all information just because of the different objectType member?
	bool needNewFunc = false;

	if( RequireTypeReplacement(func->returnType, templateType) )
		needNewFunc = true;
	else
	{
		for( asUINT p = 0; p < func->parameterTypes.GetLength(); p++ )
		{
			if( RequireTypeReplacement(func->parameterTypes[p], templateType) )
			{
				needNewFunc = true;
				break;
			}
		}
	}

	if( !needNewFunc )
		return false;
*/

	asCScriptFunction *func2 = asNEW(asCScriptFunction)(this, 0, func->funcType);
	if( func2 == 0 )
	{
		// Out of memory
		return false;
	}

	func2->name     = func->name;

	func2->returnType = DetermineTypeForTemplate(func->returnType, templateType, ot);
	func2->parameterTypes.SetLength(func->parameterTypes.GetLength());
	for (asUINT p = 0; p < func->parameterTypes.GetLength(); p++)
		func2->parameterTypes[p] = DetermineTypeForTemplate(func->parameterTypes[p], templateType, ot);

	for (asUINT n = 0; n < func->defaultArgs.GetLength(); n++)
		if (func->defaultArgs[n])
			func2->defaultArgs.PushLast(asNEW(asCString)(*func->defaultArgs[n]));
		else
			func2->defaultArgs.PushLast(0);

	// TODO: template: Must be careful when instantiating templates for garbage collected types
	//                 If the template hasn't been registered with the behaviours, it shouldn't
	//                 permit instantiation of garbage collected types that in turn may refer to
	//                 this instance.

	func2->parameterNames = func->parameterNames;
	func2->inOutFlags = func->inOutFlags;
	func2->traits = func->traits;
	func2->SetReadOnly(func->IsReadOnly());
	func2->objectType = ot;
	func2->objectType->AddRefInternal();
	func2->sysFuncIntf = asNEW(asSSystemFunctionInterface)(*func->sysFuncIntf);

	// Adjust the clean up instructions
	if( func2->sysFuncIntf->callConv == ICC_GENERIC_FUNC ||
		func2->sysFuncIntf->callConv == ICC_GENERIC_METHOD )
		PrepareSystemFunctionGeneric(func2, func2->sysFuncIntf, this);
	else
		PrepareSystemFunction(func2, func2->sysFuncIntf, this);

	func2->id       = GetNextScriptFunctionId();
	AddScriptFunction(func2);

	// Return the new function
	*newFunc = func2;

	return true;
}

asCFuncdefType *asCScriptEngine::GenerateNewTemplateFuncdef(asCObjectType *templateType, asCObjectType *ot, asCFuncdefType *func)
{
	// TODO: Only generate the new funcdef if it used the template subtypes.
	//       Remember to also update the clean up in asCObjectType::DestroyInternal so it doesn't delete
	//       child funcdefs that have not been created specificially for the template instance.
	//       Perhaps a new funcdef is always needed, since the funcdef will have a reference to the
	//       parent class (in this case the template instance).

	asCScriptFunction *func2 = asNEW(asCScriptFunction)(this, 0, func->funcdef->funcType);
	if (func2 == 0)
	{
		// Out of memory
		return 0;
	}

	func2->name = func->name;

	func2->returnType = DetermineTypeForTemplate(func->funcdef->returnType, templateType, ot);
	func2->parameterTypes.SetLength(func->funcdef->parameterTypes.GetLength());
	for (asUINT p = 0; p < func->funcdef->parameterTypes.GetLength(); p++)
		func2->parameterTypes[p] = DetermineTypeForTemplate(func->funcdef->parameterTypes[p], templateType, ot);

	// TODO: template: Must be careful when instantiating templates for garbage collected types
	//                 If the template hasn't been registered with the behaviours, it shouldn't
	//                 permit instantiation of garbage collected types that in turn may refer to
	//                 this instance.

	func2->inOutFlags = func->funcdef->inOutFlags;
	func2->SetReadOnly(func->funcdef->IsReadOnly());
	asASSERT(func->funcdef->objectType == 0);
	asASSERT(func->funcdef->sysFuncIntf == 0);

	func2->id = GetNextScriptFunctionId();
	AddScriptFunction(func2);

	asCFuncdefType *fdt2 = asNEW(asCFuncdefType)(this, func2);
	funcDefs.PushLast(fdt2); // don't increase refCount as the constructor already set it to 1

	// Return the new function
	return fdt2;
}

void asCScriptEngine::CallObjectMethod(void *obj, int func) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	CallObjectMethod(obj, s->sysFuncIntf, s);
}

void asCScriptEngine::CallObjectMethod(void *obj, asSSystemFunctionInterface *i, asCScriptFunction *s) const
{
#if defined(__GNUC__) || defined(AS_PSVITA)
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
	}
	else if( i->callConv == ICC_THISCALL || i->callConv == ICC_VIRTUAL_THISCALL )
	{
		// For virtual thiscalls we must call the method as a true class method
		// so that the compiler will lookup the function address in the vftable
		union
		{
			asSIMPLEMETHOD_t mthd;
			struct
			{
				asFUNCTION_t func;
				asPWORD baseOffset;  // Same size as the pointer
			} f;
		} p;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		p.f.func = (asFUNCTION_t)(i->func);
		p.f.baseOffset = asPWORD(i->baseOffset);
		void (asCSimpleDummy::*f)() = p.mthd;
		(((asCSimpleDummy*)obj)->*f)();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		void (*f)(void *) = (void (*)(void *))(i->func);
		f(obj);
	}
#else
#ifndef AS_NO_CLASS_METHODS
	if( i->callConv == ICC_THISCALL )
	{
		union
		{
			asSIMPLEMETHOD_t mthd;
			asFUNCTION_t func;
		} p;
		p.func = (asFUNCTION_t)(i->func);
		void (asCSimpleDummy::*f)() = p.mthd;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		obj = (void*)(asPWORD(obj) + i->baseOffset);
		(((asCSimpleDummy*)obj)->*f)();
	}
	else
#endif
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		void (*f)(void *) = (void (*)(void *))(i->func);
		f(obj);
	}
#endif
}

bool asCScriptEngine::CallObjectMethodRetBool(void *obj, int func) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	asSSystemFunctionInterface *i = s->sysFuncIntf;

#if defined(__GNUC__) || defined(AS_PSVITA)
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(bool*)gen.GetReturnPointer();
	}
	else if( i->callConv == ICC_THISCALL || i->callConv == ICC_VIRTUAL_THISCALL )
	{
		// For virtual thiscalls we must call the method as a true class method so that the compiler will lookup the function address in the vftable
		union
		{
			asSIMPLEMETHOD_t mthd;
			struct
			{
				asFUNCTION_t func;
				asPWORD baseOffset;
			} f;
		} p;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		p.f.func = (asFUNCTION_t)(i->func);
		p.f.baseOffset = asPWORD(i->baseOffset);
		bool (asCSimpleDummy::*f)() = (bool (asCSimpleDummy::*)())(p.mthd);
		return (((asCSimpleDummy*)obj)->*f)();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		bool (*f)(void *) = (bool (*)(void *))(i->func);
		return f(obj);
	}
#else
#ifndef AS_NO_CLASS_METHODS
	if( i->callConv == ICC_THISCALL )
	{
		union
		{
			asSIMPLEMETHOD_t mthd;
			asFUNCTION_t func;
		} p;
		p.func = (asFUNCTION_t)(i->func);
		bool (asCSimpleDummy::*f)() = (bool (asCSimpleDummy::*)())p.mthd;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		obj = (void*)(asPWORD(obj) + i->baseOffset);
		return (((asCSimpleDummy*)obj)->*f)();
	}
	else
#endif
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(bool*)gen.GetReturnPointer();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		bool (*f)(void *) = (bool (*)(void *))(i->func);
		return f(obj);
	}
#endif
}

int asCScriptEngine::CallObjectMethodRetInt(void *obj, int func) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	asSSystemFunctionInterface *i = s->sysFuncIntf;

#if defined(__GNUC__) || defined(AS_PSVITA)
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(int*)gen.GetReturnPointer();
	}
	else if( i->callConv == ICC_THISCALL || i->callConv == ICC_VIRTUAL_THISCALL )
	{
		// For virtual thiscalls we must call the method as a true class method so that the compiler will lookup the function address in the vftable
		union
		{
			asSIMPLEMETHOD_t mthd;
			struct
			{
				asFUNCTION_t func;
				asPWORD baseOffset;
			} f;
		} p;
		p.f.func = (asFUNCTION_t)(i->func);
		p.f.baseOffset = asPWORD(i->baseOffset);

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		int (asCSimpleDummy::*f)() = (int (asCSimpleDummy::*)())(p.mthd);
		return (((asCSimpleDummy*)obj)->*f)();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		int (*f)(void *) = (int (*)(void *))(i->func);
		return f(obj);
	}
#else
#ifndef AS_NO_CLASS_METHODS
	if( i->callConv == ICC_THISCALL )
	{
		union
		{
			asSIMPLEMETHOD_t mthd;
			asFUNCTION_t func;
		} p;
		p.func = (asFUNCTION_t)(i->func);
		int (asCSimpleDummy::*f)() = (int (asCSimpleDummy::*)())p.mthd;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		obj = (void*)(asPWORD(obj) + i->baseOffset);
		return (((asCSimpleDummy*)obj)->*f)();
	}
	else
#endif
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(int*)gen.GetReturnPointer();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		int (*f)(void *) = (int (*)(void *))(i->func);
		return f(obj);
	}
#endif
}

void *asCScriptEngine::CallObjectMethodRetPtr(void *obj, int func) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	asSSystemFunctionInterface *i = s->sysFuncIntf;

#if defined(__GNUC__) || defined(AS_PSVITA)
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(void**)gen.GetReturnPointer();
	}
	else if( i->callConv == ICC_THISCALL || i->callConv == ICC_VIRTUAL_THISCALL )
	{
		// For virtual thiscalls we must call the method as a true class method so that the compiler will lookup the function address in the vftable
		union
		{
			asSIMPLEMETHOD_t mthd;
			struct
			{
				asFUNCTION_t func;
				asPWORD baseOffset;
			} f;
		} p;
		p.f.func = (asFUNCTION_t)(i->func);
		p.f.baseOffset = asPWORD(i->baseOffset);

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		void *(asCSimpleDummy::*f)() = (void *(asCSimpleDummy::*)())(p.mthd);
		return (((asCSimpleDummy*)obj)->*f)();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		void *(*f)(void *) = (void *(*)(void *))(i->func);
		return f(obj);
	}
#else
#ifndef AS_NO_CLASS_METHODS
	if( i->callConv == ICC_THISCALL )
	{
		union
		{
			asSIMPLEMETHOD_t mthd;
			asFUNCTION_t func;
		} p;
		p.func = (asFUNCTION_t)(i->func);
		void *(asCSimpleDummy::*f)() = (void *(asCSimpleDummy::*)())p.mthd;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		obj = (void*)(asPWORD(obj) + i->baseOffset);
		return (((asCSimpleDummy*)obj)->*f)();
	}
	else
#endif
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(void **)gen.GetReturnPointer();
	}
	else /*if( i->callConv == ICC_CDECL_OBJLAST || i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		void *(*f)(void *) = (void *(*)(void *))(i->func);
		return f(obj);
	}
#endif
}

void *asCScriptEngine::CallObjectMethodRetPtr(void *obj, int param1, asCScriptFunction *func) const
{
	asASSERT( obj != 0 );
	asASSERT( func != 0 );
	asSSystemFunctionInterface *i = func->sysFuncIntf;

#ifndef AS_NO_CLASS_METHODS
	if( i->callConv == ICC_THISCALL || i->callConv == ICC_VIRTUAL_THISCALL )
	{
#if defined(__GNUC__) || defined(AS_PSVITA)
		// For virtual thiscalls we must call the method as a true class method so that the compiler will lookup the function address in the vftable
		union
		{
			asSIMPLEMETHOD_t mthd;
			struct
			{
				asFUNCTION_t func;
				asPWORD baseOffset;
			} f;
		} p;
		p.f.func = (asFUNCTION_t)(i->func);
		p.f.baseOffset = asPWORD(i->baseOffset);

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		void *(asCSimpleDummy::*f)(int) = (void *(asCSimpleDummy::*)(int))(p.mthd);
		return (((asCSimpleDummy*)obj)->*f)(param1);
#else
		union
		{
			asSIMPLEMETHOD_t mthd;
			asFUNCTION_t func;
		} p;
		p.func = (asFUNCTION_t)(i->func);
		void *(asCSimpleDummy::*f)(int) = (void *(asCSimpleDummy::*)(int))p.mthd;

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		obj = (void*)(asPWORD(obj) + i->baseOffset);
		return (((asCSimpleDummy*)obj)->*f)(param1);
#endif
	}
	else
#endif
	if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), func, obj, reinterpret_cast<asDWORD*>(&param1));
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(void **)gen.GetReturnPointer();
	}
	else if( i->callConv == ICC_CDECL_OBJLAST )
	{
		void *(*f)(int, void *) = (void *(*)(int, void *))(i->func);
		return f(param1, obj);
	}
	else /*if( i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		void *(*f)(void *, int) = (void *(*)(void *, int))(i->func);
		return f(obj, param1);
	}
}

void *asCScriptEngine::CallGlobalFunctionRetPtr(int func) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	return CallGlobalFunctionRetPtr(s->sysFuncIntf, s);
}

void *asCScriptEngine::CallGlobalFunctionRetPtr(int func, void *param1) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	return CallGlobalFunctionRetPtr(s->sysFuncIntf, s, param1);
}

void *asCScriptEngine::CallGlobalFunctionRetPtr(asSSystemFunctionInterface *i, asCScriptFunction *s) const
{
	if( i->callConv == ICC_CDECL )
	{
		void *(*f)() = (void *(*)())(i->func);
		return f();
	}
	else if( i->callConv == ICC_STDCALL )
	{
		typedef void *(STDCALL *func_t)();
		func_t f = (func_t)(i->func);
		return f();
	}
	else
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, 0, 0);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(void**)gen.GetReturnPointer();
	}
}

void *asCScriptEngine::CallGlobalFunctionRetPtr(asSSystemFunctionInterface *i, asCScriptFunction *s, void *param1) const
{
	if( i->callConv == ICC_CDECL )
	{
		void *(*f)(void *) = (void *(*)(void *))(i->func);
		return f(param1);
	}
	else if( i->callConv == ICC_STDCALL )
	{
		typedef void *(STDCALL *func_t)(void *);
		func_t f = (func_t)(i->func);
		return f(param1);
	}
	else
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, 0, (asDWORD*)&param1);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(void**)gen.GetReturnPointer();
	}
}

void asCScriptEngine::CallObjectMethod(void *obj, void *param, int func) const
{
	asCScriptFunction *s = scriptFunctions[func];
	asASSERT( s != 0 );
	CallObjectMethod(obj, param, s->sysFuncIntf, s);
}

void asCScriptEngine::CallObjectMethod(void *obj, void *param, asSSystemFunctionInterface *i, asCScriptFunction *s) const
{
#if defined(__GNUC__) || defined(AS_PSVITA)
	if( i->callConv == ICC_CDECL_OBJLAST )
	{
		void (*f)(void *, void *) = (void (*)(void *, void *))(i->func);
		f(param, obj);
	}
	else if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, (asDWORD*)&param);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
	}
	else if( i->callConv == ICC_VIRTUAL_THISCALL || i->callConv == ICC_THISCALL )
	{
		// For virtual thiscalls we must call the method as a true class method
		// so that the compiler will lookup the function address in the vftable
		union
		{
			asSIMPLEMETHOD_t mthd;
			struct
			{
				asFUNCTION_t func;
				asPWORD baseOffset;  // Same size as the pointer
			} f;
		} p;
		p.f.func = (asFUNCTION_t)(i->func);
		p.f.baseOffset = asPWORD(i->baseOffset);

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		void (asCSimpleDummy::*f)(void*) = (void (asCSimpleDummy::*)(void*))(p.mthd);
		(((asCSimpleDummy*)obj)->*f)(param);
	}
	else /*if( i->callConv == ICC_CDECL_OBJFIRST */
	{
		void (*f)(void *, void *) = (void (*)(void *, void *))(i->func);
		f(obj, param);
	}
#else
#ifndef AS_NO_CLASS_METHODS
	if( i->callConv == ICC_THISCALL )
	{
		union
		{
			asSIMPLEMETHOD_t mthd;
			asFUNCTION_t func;
		} p;
		p.func = (asFUNCTION_t)(i->func);
		void (asCSimpleDummy::*f)(void *) = (void (asCSimpleDummy::*)(void *))(p.mthd);

		obj = (void*) ((char*) obj +  i->compositeOffset);
		if(i->isCompositeIndirect)
			obj = *((void**)obj);

		obj = (void*)(asPWORD(obj) + i->baseOffset);
		(((asCSimpleDummy*)obj)->*f)(param);
	}
	else
#endif
	if( i->callConv == ICC_CDECL_OBJLAST )
	{
		void (*f)(void *, void *) = (void (*)(void *, void *))(i->func);
		f(param, obj);
	}
	else if( i->callConv == ICC_GENERIC_METHOD )
	{
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, obj, (asDWORD*)&param);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
	}
	else /*if( i->callConv == ICC_CDECL_OBJFIRST )*/
	{
		void (*f)(void *, void *) = (void (*)(void *, void *))(i->func);
		f(obj, param);
	}
#endif
}

void asCScriptEngine::CallGlobalFunction(void *param1, void *param2, asSSystemFunctionInterface *i, asCScriptFunction *s) const
{
	if( i->callConv == ICC_CDECL )
	{
		void (*f)(void *, void *) = (void (*)(void *, void *))(i->func);
		f(param1, param2);
	}
	else if( i->callConv == ICC_STDCALL )
	{
		typedef void (STDCALL *func_t)(void *, void *);
		func_t f = (func_t)(i->func);
		f(param1, param2);
	}
	else
	{
		// We must guarantee the order of the arguments which is why we copy them to this
		// array. Otherwise the compiler may put them anywhere it likes, or even keep them
		// in the registers which causes problem.
		void *params[2] = {param1, param2};

		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, 0, (asDWORD*)&params);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
	}
}

bool asCScriptEngine::CallGlobalFunctionRetBool(void *param1, void *param2, asSSystemFunctionInterface *i, asCScriptFunction *s) const
{
	if( i->callConv == ICC_CDECL )
	{
		bool (*f)(void *, void *) = (bool (*)(void *, void *))(i->func);
		return f(param1, param2);
	}
	else if( i->callConv == ICC_STDCALL )
	{
		typedef bool (STDCALL *func_t)(void *, void *);
		func_t f = (func_t)(i->func);
		return f(param1, param2);
	}
	else
	{
		// TODO: When simulating a 64bit environment by defining AS_64BIT_PTR on a 32bit platform this code
		//       fails, because the stack given to asCGeneric is not prepared with two 64bit arguments.

		// We must guarantee the order of the arguments which is why we copy them to this
		// array. Otherwise the compiler may put them anywhere it likes, or even keep them
		// in the registers which causes problem.
		void *params[2] = {param1, param2};
		asCGeneric gen(const_cast<asCScriptEngine*>(this), s, 0, (asDWORD*)params);
		void (*f)(asIScriptGeneric *) = (void (*)(asIScriptGeneric *))(i->func);
		f(&gen);
		return *(bool*)gen.GetReturnPointer();
	}
}

void *asCScriptEngine::CallAlloc(const asCObjectType *type) const
{
	// Allocate 4 bytes as the smallest size. Otherwise CallSystemFunction may try to
	// copy a DWORD onto a smaller memory block, in case the object type is return in registers.

	// Pad to the next even 4 bytes to avoid asBC_CPY writing outside of allocated buffer for registered POD types
	asUINT size = type->size;
	if( size & 0x3 )
		size += 4 - (size & 0x3);

#ifndef WIP_16BYTE_ALIGN
#if defined(AS_DEBUG)
	return ((asALLOCFUNCDEBUG_t)userAlloc)(size, __FILE__, __LINE__);
#else
	return userAlloc(size);
#endif
#else
#if defined(AS_DEBUG)
	return ((asALLOCALIGNEDFUNCDEBUG_t)userAllocAligned)(size, type->alignment, __FILE__, __LINE__);
#else
	return userAllocAligned(size, type->alignment);
#endif
#endif
}

void asCScriptEngine::CallFree(void *obj) const
{
#ifndef WIP_16BYTE_ALIGN
	userFree(obj);
#else
	userFreeAligned(obj);
#endif
}

// interface
int asCScriptEngine::NotifyGarbageCollectorOfNewObject(void *obj, asITypeInfo *type)
{
	return gc.AddScriptObjectToGC(obj, static_cast<asCObjectType*>(type));
}

// interface
int asCScriptEngine::GetObjectInGC(asUINT idx, asUINT *seqNbr, void **obj, asITypeInfo **type)
{
	return gc.GetObjectInGC(idx, seqNbr, obj, type);
}

// interface
int asCScriptEngine::GarbageCollect(asDWORD flags, asUINT iterations)
{
	int r = gc.GarbageCollect(flags, iterations);

	if( r == 0 )
	{
		// Delete any modules that have been discarded previously but not
		// removed due to being referred to by objects in the garbage collector
		DeleteDiscardedModules();
	}

	return r;
}

// interface
void asCScriptEngine::GetGCStatistics(asUINT *currentSize, asUINT *totalDestroyed, asUINT *totalDetected, asUINT *newObjects, asUINT *totalNewDestroyed) const
{
	gc.GetStatistics(currentSize, totalDestroyed, totalDetected, newObjects, totalNewDestroyed);
}

// interface
void asCScriptEngine::GCEnumCallback(void *reference)
{
	gc.GCEnumCallback(reference);
}

// interface
void asCScriptEngine::ForwardGCEnumReferences(void *ref, asITypeInfo *type)
{
	asCTypeInfo *t = reinterpret_cast<asCTypeInfo*>(type);
	if ((t->flags & asOBJ_VALUE) && (t->flags & asOBJ_GC))
	{
		CallObjectMethod(ref, this, CastToObjectType(t)->beh.gcEnumReferences);
	}
}

// interface
void asCScriptEngine::ForwardGCReleaseReferences(void *ref, asITypeInfo *type)
{
	asCTypeInfo *t = reinterpret_cast<asCTypeInfo*>(type);
	if ((t->flags & asOBJ_VALUE) && (t->flags & asOBJ_GC))
	{
		CallObjectMethod(ref, this, CastToObjectType(t)->beh.gcReleaseAllReferences);
	}
}

// interface
void asCScriptEngine::SetCircularRefDetectedCallback(asCIRCULARREFFUNC_t callback, void *param)
{
	gc.circularRefDetectCallbackFunc  = callback;
	gc.circularRefDetectCallbackParam = param;
}

int asCScriptEngine::GetTypeIdFromDataType(const asCDataType &dtIn) const
{
	if( dtIn.IsNullHandle() ) return asTYPEID_VOID;

	if( dtIn.GetTypeInfo() == 0 )
	{
		// Primitives have pre-fixed typeIds
		switch( dtIn.GetTokenType() )
		{
		case ttVoid:   return asTYPEID_VOID;
		case ttBool:   return asTYPEID_BOOL;
		case ttInt8:   return asTYPEID_INT8;
		case ttInt16:  return asTYPEID_INT16;
		case ttInt:    return asTYPEID_INT32;
		case ttInt64:  return asTYPEID_INT64;
		case ttUInt8:  return asTYPEID_UINT8;
		case ttUInt16: return asTYPEID_UINT16;
		case ttUInt:   return asTYPEID_UINT32;
		case ttUInt64: return asTYPEID_UINT64;
		case ttFloat:  return asTYPEID_FLOAT;
		case ttDouble: return asTYPEID_DOUBLE;
		default:
			// All types should be covered by the above. The variable type is not really a type
			asASSERT(dtIn.GetTokenType() == ttQuestion);
			return -1;
		}
	}

	int typeId = -1;
	asCTypeInfo *ot = dtIn.GetTypeInfo();
	asASSERT(ot != &functionBehaviours);
	// Object's hold the typeId themselves
	typeId = ot->typeId;

	if( typeId == -1 )
	{
		ACQUIREEXCLUSIVE(engineRWLock);
		// Make sure another thread didn't determine the typeId while we were waiting for the lock
		if( ot->typeId == -1 )
		{
			typeId = typeIdSeqNbr++;
			if( ot->flags & asOBJ_SCRIPT_OBJECT ) typeId |= asTYPEID_SCRIPTOBJECT;
			else if( ot->flags & asOBJ_TEMPLATE ) typeId |= asTYPEID_TEMPLATE;
			else if( ot->flags & asOBJ_ENUM ) {} // TODO: Should we have a specific bit for this?
			else typeId |= asTYPEID_APPOBJECT;

			ot->typeId = typeId;

			mapTypeIdToTypeInfo.Insert(typeId, ot);
		}
		RELEASEEXCLUSIVE(engineRWLock);
	}

	// Add flags according to the requested type
	if( dtIn.GetTypeInfo() && !(dtIn.GetTypeInfo()->flags & asOBJ_ASHANDLE) )
	{
		// The ASHANDLE types behave like handles, but are really
		// value types so the typeId is never returned as a handle
		if( dtIn.IsObjectHandle() )
			typeId |= asTYPEID_OBJHANDLE;
		if( dtIn.IsHandleToConst() )
			typeId |= asTYPEID_HANDLETOCONST;
	}

	return typeId;
}

asCDataType asCScriptEngine::GetDataTypeFromTypeId(int typeId) const
{
	int baseId = typeId & (asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR);

	if( typeId <= asTYPEID_DOUBLE )
	{
		eTokenType type[] = {ttVoid, ttBool, ttInt8, ttInt16, ttInt, ttInt64, ttUInt8, ttUInt16, ttUInt, ttUInt64, ttFloat, ttDouble};
		return asCDataType::CreatePrimitive(type[typeId], false);
	}

	// First check if the typeId is an object type
	asCTypeInfo *ot = 0;
	ACQUIRESHARED(engineRWLock);
	asSMapNode<int,asCTypeInfo*> *cursor = 0;
	if( mapTypeIdToTypeInfo.MoveTo(&cursor, baseId) )
		ot = mapTypeIdToTypeInfo.GetValue(cursor);
	RELEASESHARED(engineRWLock);

	if( ot )
	{
		asCDataType dt = asCDataType::CreateType(ot, false);
		if( typeId & asTYPEID_OBJHANDLE )
			dt.MakeHandle(true, true);
		if( typeId & asTYPEID_HANDLETOCONST )
			dt.MakeHandleToConst(true);

		return dt;
	}

	return asCDataType();
}

asCObjectType *asCScriptEngine::GetObjectTypeFromTypeId(int typeId) const
{
	asCDataType dt = GetDataTypeFromTypeId(typeId);
	return CastToObjectType(dt.GetTypeInfo());
}

void asCScriptEngine::RemoveFromTypeIdMap(asCTypeInfo *type)
{
	ACQUIREEXCLUSIVE(engineRWLock);
	asSMapNode<int,asCTypeInfo*> *cursor = 0;
	mapTypeIdToTypeInfo.MoveFirst(&cursor);
	while( cursor )
	{
		if(mapTypeIdToTypeInfo.GetValue(cursor) == type )
		{
			mapTypeIdToTypeInfo.Erase(cursor);
			break;
		}
		mapTypeIdToTypeInfo.MoveNext(&cursor, cursor);
	}
	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
asITypeInfo *asCScriptEngine::GetTypeInfoByDecl(const char *decl) const
{
	asCDataType dt;
	// This cast is ok, because we are not changing anything in the engine
	asCBuilder bld(const_cast<asCScriptEngine*>(this), 0);

	// Don't write parser errors to the message callback
	bld.silent = true;

	int r = bld.ParseDataType(decl, &dt, defaultNamespace);
	if (r < 0)
		return 0;

	return dt.GetTypeInfo();
}

// interface
int asCScriptEngine::GetTypeIdByDecl(const char *decl) const
{
	asCDataType dt;
	// This cast is ok, because we are not changing anything in the engine
	asCBuilder bld(const_cast<asCScriptEngine*>(this), 0);

	// Don't write parser errors to the message callback
	bld.silent = true;

	int r = bld.ParseDataType(decl, &dt, defaultNamespace);
	if( r < 0 )
		return asINVALID_TYPE;

	return GetTypeIdFromDataType(dt);
}

// interface
const char *asCScriptEngine::GetTypeDeclaration(int typeId, bool includeNamespace) const
{
	asCDataType dt = GetDataTypeFromTypeId(typeId);

	asCString *tempString = &asCThreadManager::GetLocalData()->string;
	*tempString = dt.Format(defaultNamespace, includeNamespace);

	return tempString->AddressOf();
}

// interface
int asCScriptEngine::GetSizeOfPrimitiveType(int typeId) const
{
	asCDataType dt = GetDataTypeFromTypeId(typeId);
	if( !dt.IsPrimitive() ) return 0;

	return dt.GetSizeInMemoryBytes();
}

// interface
int asCScriptEngine::RefCastObject(void *obj, asITypeInfo *fromType, asITypeInfo *toType, void **newPtr, bool useOnlyImplicitCast)
{
	if( newPtr == 0 ) return asINVALID_ARG;
	*newPtr = 0;

	if( fromType == 0 || toType == 0 ) return asINVALID_ARG;

	// A null-pointer can always be cast to another type, so it will always be successful
	if( obj == 0 )
		return asSUCCESS;

	if( fromType == toType )
	{
		*newPtr = obj;
		AddRefScriptObject(*newPtr, toType);
		return asSUCCESS;
	}

	// Check for funcdefs
	if ((fromType->GetFlags() & asOBJ_FUNCDEF) && (toType->GetFlags() & asOBJ_FUNCDEF))
	{
		asCFuncdefType *fromFunc = CastToFuncdefType(reinterpret_cast<asCTypeInfo*>(fromType));
		asCFuncdefType *toFunc = CastToFuncdefType(reinterpret_cast<asCTypeInfo*>(toType));

		if (fromFunc && toFunc && fromFunc->funcdef->IsSignatureExceptNameEqual(toFunc->funcdef))
		{
			*newPtr = obj;
			AddRefScriptObject(*newPtr, toType);
			return asSUCCESS;
		}

		return asSUCCESS;
	}

	// Look for ref cast behaviours
	asCScriptFunction *universalCastFunc = 0;
	asCObjectType *from = CastToObjectType(reinterpret_cast< asCTypeInfo*>(fromType));
	if( from == 0 ) return asINVALID_ARG;
	for( asUINT n = 0; n < from->methods.GetLength(); n++ )
	{
		asCScriptFunction *func = scriptFunctions[from->methods[n]];
		if( func->name == "opImplCast" ||
			(!useOnlyImplicitCast && func->name == "opCast") )
		{
			if( func->returnType.GetTypeInfo() == toType )
			{
				*newPtr = CallObjectMethodRetPtr(obj, func->id);
				// The ref cast behaviour returns a handle with incremented
				// ref counter, so there is no need to call AddRef explicitly
				// unless the function is registered with autohandle
				if( func->sysFuncIntf->returnAutoHandle )
					AddRefScriptObject(*newPtr, toType);
				return asSUCCESS;
			}
			else if( func->returnType.GetTokenType() == ttVoid &&
					 func->parameterTypes.GetLength() == 1 &&
					 func->parameterTypes[0].GetTokenType() == ttQuestion )
			{
				universalCastFunc = func;
			}
		}
	}

	// One last chance if the object has a void opCast(?&out) behaviour
	if( universalCastFunc )
	{
		// TODO: Add proper error handling
		asIScriptContext *ctx = RequestContext();
		ctx->Prepare(universalCastFunc);
		ctx->SetObject(obj);
		ctx->SetArgVarType(0, newPtr, toType->GetTypeId() | asTYPEID_OBJHANDLE);
		ctx->Execute();
		ReturnContext(ctx);

		// The opCast(?&out) method already incremented the
		// refCount so there is no need to do it manually
		return asSUCCESS;
	}

	// For script classes and interfaces there is a quick route
	if( (fromType->GetFlags() & asOBJ_SCRIPT_OBJECT) && (toType->GetFlags() & asOBJ_SCRIPT_OBJECT) )
	{
		if( fromType == toType )
		{
			*newPtr = obj;
			reinterpret_cast<asCScriptObject*>(*newPtr)->AddRef();
			return asSUCCESS;
		}

		// Up casts to base class or interface can be done implicitly
		if( fromType->DerivesFrom(toType) ||
			fromType->Implements(toType) )
		{
			*newPtr = obj;
			reinterpret_cast<asCScriptObject*>(*newPtr)->AddRef();
			return asSUCCESS;
		}
		// Down casts to derived class or from interface can only be done explicitly
		if( !useOnlyImplicitCast )
		{
			// Get the true type of the object so the explicit cast can evaluate all possibilities
			asITypeInfo *trueType = reinterpret_cast<asCScriptObject*>(obj)->GetObjectType();
			if (trueType->DerivesFrom(toType) ||
				trueType->Implements(toType))
			{
				*newPtr = obj;
				reinterpret_cast<asCScriptObject*>(*newPtr)->AddRef();
				return asSUCCESS;
			}
		}
	}

	// The cast is not available, but it is still a success
	return asSUCCESS;
}

// interface
void *asCScriptEngine::CreateScriptObject(const asITypeInfo *type)
{
	if( type == 0 ) return 0;

	asCObjectType *objType = CastToObjectType(const_cast<asCTypeInfo*>(reinterpret_cast<const asCTypeInfo*>(type)));
	if (objType == 0) return 0;
	void *ptr = 0;

	// Check that there is a default factory for ref types
	if( objType->beh.factory == 0 && (objType->flags & asOBJ_REF) )
	{
		// TODO: How to report the reason the object couldn't be created, without writing to the message callback? optional argument with return code?
		// TODO: Warn about the invalid call to message callback. Make it an optional, so the warning can be turned off
//		asCString str;
//		str.Format(TXT_FAILED_IN_FUNC_s_s_d, "CreateScriptObject", errorNames[-asNO_FUNCTION], asNO_FUNCTION);
//		WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
		return 0;
	}

	// Construct the object
	if( objType->flags & asOBJ_SCRIPT_OBJECT )
	{
		// Call the script class' default factory with a context
		ptr = ScriptObjectFactory(objType, this);
	}
	else if( (objType->flags & asOBJ_TEMPLATE) && (objType->flags & asOBJ_REF) )
	{
		// The registered factory that takes the object type is moved
		// to the construct behaviour when the type is instantiated
#ifdef AS_NO_EXCEPTIONS
		ptr = CallGlobalFunctionRetPtr(objType->beh.construct, objType);
#else
		try
		{
			ptr = CallGlobalFunctionRetPtr(objType->beh.construct, objType);
		}
		catch (...)
		{
			asCContext *ctx = reinterpret_cast<asCContext*>(asGetActiveContext());
			if (ctx)
				ctx->HandleAppException();
		}
#endif
	}
	else if( objType->flags & asOBJ_REF )
	{
		// Call the default factory directly
#ifdef AS_NO_EXCEPTIONS
		ptr = CallGlobalFunctionRetPtr(objType->beh.factory);
#else
		try
		{
			ptr = CallGlobalFunctionRetPtr(objType->beh.factory);
		}
		catch(...)
		{
			asCContext *ctx = reinterpret_cast<asCContext*>(asGetActiveContext());
			if( ctx )
				ctx->HandleAppException();
		}
#endif
	}
	else
	{
		// Make sure there is a default constructor or that it is a POD type
		if( objType->beh.construct == 0 && !(objType->flags & asOBJ_POD) )
		{
			// TODO: How to report the reason the object couldn't be created, without writing to the message callback? optional argument with return code?
			// TODO: Warn about the invalid call to message callback. Make it an optional, so the warning can be turned off
//			asCString str;
//			str.Format(TXT_FAILED_IN_FUNC_s_s_d, "CreateScriptObject", errorNames[-asNO_FUNCTION], asNO_FUNCTION);
//			WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.AddressOf());
			return 0;
		}

		// Manually allocate the memory, then call the default constructor
		ptr = CallAlloc(objType);
		int funcIndex = objType->beh.construct;
		if (funcIndex)
		{
			if (objType->flags & asOBJ_TEMPLATE)
			{
				// Templates of value types create script functions as the constructors
				CallScriptObjectMethod(ptr, funcIndex);
			}
			else
			{
#ifdef AS_NO_EXCEPTIONS
				CallObjectMethod(ptr, funcIndex);
#else
				try
				{
					CallObjectMethod(ptr, funcIndex);
				}
				catch (...)
				{
					asCContext *ctx = reinterpret_cast<asCContext*>(asGetActiveContext());
					if (ctx)
						ctx->HandleAppException();

					// Free the memory
					CallFree(ptr);
					ptr = 0;
				}
#endif
			}
		}
	}

	return ptr;
}

// internal
int asCScriptEngine::CallScriptObjectMethod(void *obj, int funcId)
{
	asIScriptContext *ctx = 0;
	int r = 0;
	bool isNested = false;

	// Use nested call in the context if there is an active context
	ctx = asGetActiveContext();
	if (ctx)
	{
		// It may not always be possible to reuse the current context,
		// in which case we'll have to create a new one any way.
		if (ctx->GetEngine() == this && ctx->PushState() == asSUCCESS)
			isNested = true;
		else
			ctx = 0;
	}

	if (ctx == 0)
	{
		// Request a context from the engine
		ctx = RequestContext();
		if (ctx == 0)
		{
			// TODO: How to best report this failure?
			return asERROR;
		}
	}

	r = ctx->Prepare(scriptFunctions[funcId]);
	if (r < 0)
	{
		if (isNested)
			ctx->PopState();
		else
			ReturnContext(ctx);
		// TODO: How to best report this failure?
		return asERROR;
	}

	// Set the object
	ctx->SetObject(obj);

	for (;;)
	{
		r = ctx->Execute();

		// We can't allow this execution to be suspended
		// so resume the execution immediately
		if (r != asEXECUTION_SUSPENDED)
			break;
	}

	if (r != asEXECUTION_FINISHED)
	{
		if (isNested)
		{
			ctx->PopState();

			// If the execution was aborted or an exception occurred,
			// then we should forward that to the outer execution.
			if (r == asEXECUTION_EXCEPTION)
			{
				// TODO: How to improve this exception
				ctx->SetException(TXT_EXCEPTION_IN_NESTED_CALL);
			}
			else if (r == asEXECUTION_ABORTED)
				ctx->Abort();
		}
		else
			ReturnContext(ctx);

		// TODO: How to best report the error?
		return asERROR;
	}

	if (isNested)
		ctx->PopState();
	else
		ReturnContext(ctx);

	return asSUCCESS;
}

// interface
void *asCScriptEngine::CreateUninitializedScriptObject(const asITypeInfo *type)
{
	// This function only works for script classes. Registered types cannot be created this way.
	if( type == 0 || !(type->GetFlags() & asOBJ_SCRIPT_OBJECT) )
		return 0;

	asCObjectType *objType = CastToObjectType(const_cast<asCTypeInfo*>(reinterpret_cast<const asCTypeInfo*>(type)));
	if (objType == 0)
		return 0;

	// Construct the object, but do not call the actual constructor that initializes the members
	// The initialization will be done by the application afterwards, e.g. through serialization.
	asCScriptObject *obj = reinterpret_cast<asCScriptObject*>(CallAlloc(objType));

	// Pre-initialize the memory so there are no invalid pointers
	ScriptObject_ConstructUnitialized(objType, obj);

	return obj;
}

// interface
void *asCScriptEngine::CreateScriptObjectCopy(void *origObj, const asITypeInfo *type)
{
	if( origObj == 0 || type == 0 ) return 0;

	const asCObjectType* ot = CastToObjectType(const_cast<asCTypeInfo*>(reinterpret_cast<const asCTypeInfo*>(type)));
	if (ot == 0) return 0;

	void *newObj = 0;

	if ((ot->flags & asOBJ_SCRIPT_OBJECT) && ot->beh.copyfactory)
	{
		// Call the script class' default factory with a context
		newObj = ScriptObjectCopyFactory(ot, origObj, this);
	}
	else if (ot->beh.copyfactory)
	{
		// Call the copy factory which will allocate the memory then copy the original object
#ifdef AS_NO_EXCEPTIONS
		newObj = CallGlobalFunctionRetPtr(ot->beh.copyfactory, origObj);
#else
		try
		{
			newObj = CallGlobalFunctionRetPtr(ot->beh.copyfactory, origObj);
		}
		catch (...)
		{
			asCContext *ctx = reinterpret_cast<asCContext*>(asGetActiveContext());
			if (ctx)
				ctx->HandleAppException();
		}
#endif
	}
	else if(ot->beh.copyconstruct )
	{
		// Manually allocate the memory, then call the copy constructor
		newObj = CallAlloc(ot);
#ifdef AS_NO_EXCEPTIONS
		CallObjectMethod(newObj, origObj, ot->beh.copyconstruct);
#else
		try
		{
			CallObjectMethod(newObj, origObj, ot->beh.copyconstruct);
		}
		catch(...)
		{
			asCContext *ctx = reinterpret_cast<asCContext*>(asGetActiveContext());
			if( ctx )
				ctx->HandleAppException();

			// Free the memory
			CallFree(newObj);
			newObj = 0;
		}
#endif
	}
	else
	{
		// Allocate the object and then do a value assign
		newObj = CreateScriptObject(type);
		if( newObj == 0 ) return 0;

		AssignScriptObject(newObj, origObj, type);
	}

	return newObj;
}

// internal
void asCScriptEngine::ConstructScriptObjectCopy(void *mem, void *obj, asCObjectType *type)
{
	if( type == 0 || mem == 0 || obj == 0 ) return;

	// This function is only meant to be used for value types
	asASSERT( type->flags & asOBJ_VALUE );

	// Call the copy constructor if available, else call the default constructor followed by the opAssign
	int funcIndex = type->beh.copyconstruct;
	if( funcIndex )
	{
		CallObjectMethod(mem, obj, funcIndex);
	}
	else
	{
		funcIndex = type->beh.construct;
		if( funcIndex )
			CallObjectMethod(mem, funcIndex);

		AssignScriptObject(mem, obj, type);
	}
}

// interface
int asCScriptEngine::AssignScriptObject(void *dstObj, void *srcObj, const asITypeInfo *type)
{
	// TODO: Warn about invalid call in message stream (make it optional)
	if( type == 0 || dstObj == 0 || srcObj == 0 ) return asINVALID_ARG;

	const asCObjectType *objType = CastToObjectType(const_cast<asCTypeInfo*>(reinterpret_cast<const asCTypeInfo*>(type)));
	if (objType == 0) return asINVALID_ARG;

	// If value assign for ref types has been disabled, then don't do anything if the type is a ref type
	if (ep.disallowValueAssignForRefType && (objType->flags & asOBJ_REF) && !(objType->flags & asOBJ_SCOPED))
	{
		asIScriptContext *ctx = asGetActiveContext();
		if (ctx)
			ctx->SetException("Cannot do value assignment");
		return asNOT_SUPPORTED;
	}

	// Must not copy if the opAssign is not available and the object is not a POD object
	if( objType->beh.copy )
	{
		asCScriptFunction *func = scriptFunctions[objType->beh.copy];
		if( func->funcType == asFUNC_SYSTEM )
			CallObjectMethod(dstObj, srcObj, objType->beh.copy);
		else
		{
			// Call the script class' opAssign method
			asASSERT( objType->flags & asOBJ_SCRIPT_OBJECT );
			reinterpret_cast<asCScriptObject*>(dstObj)->CopyFrom(reinterpret_cast<asCScriptObject*>(srcObj));
		}
	}
	else if( objType->size && (objType->flags & asOBJ_POD) )
	{
		memcpy(dstObj, srcObj, objType->size);
	}

	return asSUCCESS;
}

// interface
void asCScriptEngine::AddRefScriptObject(void *obj, const asITypeInfo *type)
{
	// Make sure it is not a null pointer
	if( obj == 0 || type == 0 ) return;

	const asCTypeInfo *ti = reinterpret_cast<const asCTypeInfo*>(type);
	if (ti->flags & asOBJ_FUNCDEF)
	{
		CallObjectMethod(obj, functionBehaviours.beh.addref);
	}
	else
	{
		asCObjectType *objType = CastToObjectType(const_cast<asCTypeInfo*>(ti));
		if (objType && objType->beh.addref)
		{
			// Call the addref behaviour
			CallObjectMethod(obj, objType->beh.addref);
		}
	}
}

// interface
void asCScriptEngine::ReleaseScriptObject(void *obj, const asITypeInfo *type)
{
	// Make sure it is not a null pointer
	if( obj == 0 || type == 0 ) return;

	const asCTypeInfo *ti = reinterpret_cast<const asCTypeInfo*>(type);
	if (ti->flags & asOBJ_FUNCDEF)
	{
		CallObjectMethod(obj, functionBehaviours.beh.release);
	}
	else
	{
		asCObjectType *objType = CastToObjectType(const_cast<asCTypeInfo*>(ti));
		if (objType && objType->flags & asOBJ_REF)
		{
			asASSERT((objType->flags & asOBJ_NOCOUNT) || objType->beh.release);
			if (objType->beh.release)
			{
				// Call the release behaviour
				CallObjectMethod(obj, objType->beh.release);
			}
		}
		else if( objType )
		{
			// Call the destructor
			if (objType->beh.destruct)
				CallObjectMethod(obj, objType->beh.destruct);
			else if (objType->flags & asOBJ_LIST_PATTERN)
				DestroyList((asBYTE*)obj, objType);

			// We'll have to trust that the memory for the object was allocated with CallAlloc.
			// This is true if the object was created in the context, or with CreateScriptObject.

			// Then free the memory
			CallFree(obj);
		}
	}
}

// interface
int asCScriptEngine::BeginConfigGroup(const char *groupName)
{
	// Make sure the group name doesn't already exist
	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
	{
		if( configGroups[n]->groupName == groupName )
			return asNAME_TAKEN;
	}

	if( currentGroup != &defaultGroup )
		return asNOT_SUPPORTED;

	asCConfigGroup *group = asNEW(asCConfigGroup)();
	if( group == 0 )
		return asOUT_OF_MEMORY;

	group->groupName = groupName;

	configGroups.PushLast(group);
	currentGroup = group;

	return 0;
}

// interface
int asCScriptEngine::EndConfigGroup()
{
	// Raise error if trying to end the default config
	if( currentGroup == &defaultGroup )
		return asERROR;

	currentGroup = &defaultGroup;

	return 0;
}

// interface
int asCScriptEngine::RemoveConfigGroup(const char *groupName)
{
	// It is not allowed to remove a group that is still in use.

	// It would be possible to change the code in such a way that
	// the group could be removed even though it was still in use,
	// but that would cause severe negative impact on runtime
	// performance, since the VM would then have to be able handle
	// situations where the types, functions, and global variables
	// can be removed at any time.

	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
	{
		if( configGroups[n]->groupName == groupName )
		{
			asCConfigGroup *group = configGroups[n];

			// Remove any unused generated template instances
			// before verifying if the config group is still in use.
			// RemoveTemplateInstanceType() checks if the instance is in use
			for( asUINT g = generatedTemplateTypes.GetLength(); g-- > 0; )
				RemoveTemplateInstanceType(generatedTemplateTypes[g]);

			// Make sure the group isn't referenced by anyone
			if( group->refCount > 0 )
				return asCONFIG_GROUP_IS_IN_USE;

			// Verify if any objects registered in this group is still alive
			if( group->HasLiveObjects() )
				return asCONFIG_GROUP_IS_IN_USE;

			// Remove the group from the list
			if( n == configGroups.GetLength() - 1 )
				configGroups.PopLast();
			else
				configGroups[n] = configGroups.PopLast();

			// Remove the configurations registered with this group
			group->RemoveConfiguration(this);

			asDELETE(group,asCConfigGroup);
		}
	}

	return 0;
}

asCConfigGroup *asCScriptEngine::FindConfigGroupForFunction(int funcId) const
{
	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
	{
		// Check global functions
		asUINT m;
		for( m = 0; m < configGroups[n]->scriptFunctions.GetLength(); m++ )
		{
			if( configGroups[n]->scriptFunctions[m]->id == funcId )
				return configGroups[n];
		}
	}

	return 0;
}


asCConfigGroup *asCScriptEngine::FindConfigGroupForGlobalVar(int gvarId) const
{
	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
	{
		for( asUINT m = 0; m < configGroups[n]->globalProps.GetLength(); m++ )
		{
			if( int(configGroups[n]->globalProps[m]->id) == gvarId )
				return configGroups[n];
		}
	}

	return 0;
}

asCConfigGroup *asCScriptEngine::FindConfigGroupForTypeInfo(const asCTypeInfo *objType) const
{
	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
	{
		for( asUINT m = 0; m < configGroups[n]->types.GetLength(); m++ )
		{
			if( configGroups[n]->types[m] == objType )
				return configGroups[n];
		}
	}

	return 0;
}

asCConfigGroup *asCScriptEngine::FindConfigGroupForFuncDef(const asCFuncdefType *funcDef) const
{
	for( asUINT n = 0; n < configGroups.GetLength(); n++ )
	{
		asCFuncdefType *f = const_cast<asCFuncdefType*>(funcDef);
		if( configGroups[n]->types.Exists(f) )
			return configGroups[n];
	}

	return 0;
}

// interface
asDWORD asCScriptEngine::SetDefaultAccessMask(asDWORD defaultMask)
{
	asDWORD old = defaultAccessMask;
	defaultAccessMask = defaultMask;
	return old;
}

int asCScriptEngine::GetNextScriptFunctionId()
{
	// This function only returns the next function id that
	// should be used. It doesn't update the internal arrays.
	if( freeScriptFunctionIds.GetLength() )
		return freeScriptFunctionIds[freeScriptFunctionIds.GetLength()-1];

	return (int)scriptFunctions.GetLength();
}

void asCScriptEngine::AddScriptFunction(asCScriptFunction *func)
{
	// Update the internal arrays with the function id that is now used
	if( freeScriptFunctionIds.GetLength() && freeScriptFunctionIds[freeScriptFunctionIds.GetLength()-1] == func->id )
		freeScriptFunctionIds.PopLast();

	if( asUINT(func->id) == scriptFunctions.GetLength() )
		scriptFunctions.PushLast(func);
	else
	{
		// The slot should be empty or already set with the function, which happens if an existing shared function is reused
		asASSERT( scriptFunctions[func->id] == 0 || scriptFunctions[func->id] == func );
		scriptFunctions[func->id] = func;
	}
}

void asCScriptEngine::RemoveScriptFunction(asCScriptFunction *func)
{
	if( func == 0 || func->id < 0 ) return;
	int id = func->id & ~FUNC_IMPORTED;
	if( func->funcType == asFUNC_IMPORTED )
	{
		if( id >= (int)importedFunctions.GetLength() ) return;

		if( importedFunctions[id] )
		{
			// Remove the function from the list of script functions
			if( id == (int)importedFunctions.GetLength() - 1 )
			{
				importedFunctions.PopLast();
			}
			else
			{
				importedFunctions[id] = 0;
				freeImportedFunctionIdxs.PushLast(id);
			}
		}
	}
	else
	{
		if( id >= (int)scriptFunctions.GetLength() ) return;
		asASSERT( func == scriptFunctions[id] );

		if( scriptFunctions[id] )
		{
			// Remove the function from the list of script functions
			if( id == (int)scriptFunctions.GetLength() - 1 )
			{
				scriptFunctions.PopLast();
			}
			else
			{
				scriptFunctions[id] = 0;
				freeScriptFunctionIds.PushLast(id);
			}

			// Is the function used as signature id?
			if( func->signatureId == id )
			{
				// Remove the signature id
				signatureIds.RemoveValue(func);

				// Update all functions using the signature id
				int newSigId = 0;
				for( asUINT n = 0; n < scriptFunctions.GetLength(); n++ )
				{
					if( scriptFunctions[n] && scriptFunctions[n]->signatureId == id )
					{
						if( newSigId == 0 )
						{
							newSigId = scriptFunctions[n]->id;
							signatureIds.PushLast(scriptFunctions[n]);
						}

						scriptFunctions[n]->signatureId = newSigId;
					}
				}
			}
		}
	}
}

// internal
void asCScriptEngine::RemoveFuncdef(asCFuncdefType *funcdef)
{
	funcDefs.RemoveValue(funcdef);
}

// interface
int asCScriptEngine::RegisterFuncdef(const char *decl)
{
	if( decl == 0 ) return ConfigError(asINVALID_ARG, "RegisterFuncdef", decl, 0);

	// Parse the function declaration
	asCScriptFunction *func = asNEW(asCScriptFunction)(this, 0, asFUNC_FUNCDEF);
	if( func == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterFuncdef", decl, 0);

	asCBuilder bld(this, 0);
	asCObjectType *parentClass = 0;
	int r = bld.ParseFunctionDeclaration(0, decl, func, false, 0, 0, defaultNamespace, 0, &parentClass);
	if( r < 0 )
	{
		// Set as dummy function before deleting
		func->funcType = asFUNC_DUMMY;
		asDELETE(func,asCScriptFunction);
		return ConfigError(asINVALID_DECLARATION, "RegisterFuncdef", decl, 0);
	}

	// Check name conflicts
	r = bld.CheckNameConflict(func->name.AddressOf(), 0, 0, defaultNamespace, true, false);
	if( r < 0 )
	{
		asDELETE(func,asCScriptFunction);
		return ConfigError(asNAME_TAKEN, "RegisterFuncdef", decl, 0);
	}

	func->id = GetNextScriptFunctionId();
	AddScriptFunction(func);

	asCFuncdefType *fdt = asNEW(asCFuncdefType)(this, func);
	funcDefs.PushLast(fdt); // doesn't increase refcount
	registeredFuncDefs.PushLast(fdt); // doesn't increase refcount
	allRegisteredTypes.Insert(asSNameSpaceNamePair(fdt->nameSpace, fdt->name), fdt); // constructor already set the ref count to 1

	currentGroup->types.PushLast(fdt);
	if (parentClass)
	{
		parentClass->childFuncDefs.PushLast(fdt);
		fdt->parentClass = parentClass;

		// Check if the method restricts that use of the template to value types or reference types
		if (parentClass->flags & asOBJ_TEMPLATE)
		{
			r = SetTemplateRestrictions(parentClass, func, "RegisterFuncdef", decl);
			if (r < 0)
				return r;
		}
	}

	// If parameter type from other groups are used, add references
	currentGroup->AddReferencesForFunc(this, func);

	// Return the type id as success
	return GetTypeIdFromDataType(asCDataType::CreateType(fdt, false));
}

// interface
asUINT asCScriptEngine::GetFuncdefCount() const
{
	return asUINT(registeredFuncDefs.GetLength());
}

// interface
asITypeInfo *asCScriptEngine::GetFuncdefByIndex(asUINT index) const
{
	if( index >= registeredFuncDefs.GetLength() )
		return 0;

	return registeredFuncDefs[index];
}

// internal
asCFuncdefType *asCScriptEngine::FindMatchingFuncdef(asCScriptFunction *func, asCModule *module)
{
	asCFuncdefType *funcDef = func->funcdefType;

	if (funcDef == 0)
	{
		// Check if there is any matching funcdefs already in the engine that can be reused
		for (asUINT n = 0; n < funcDefs.GetLength(); n++)
		{
			if (funcDefs[n]->funcdef->IsSignatureExceptNameEqual(func))
			{
				if (func->IsShared() && !funcDefs[n]->funcdef->IsShared())
					continue;
				funcDef = funcDefs[n];
				break;
			}
		}
	}

	if (funcDef == 0)
	{
		// Create a matching funcdef
		asCScriptFunction *fd = asNEW(asCScriptFunction)(this, 0, asFUNC_FUNCDEF);
		fd->name = func->name;
		fd->nameSpace = func->nameSpace;
		fd->SetShared(func->IsShared());

		fd->returnType = func->returnType;
		fd->parameterTypes = func->parameterTypes;
		fd->inOutFlags = func->inOutFlags;

		funcDef = asNEW(asCFuncdefType)(this, fd);
		funcDefs.PushLast(funcDef); // doesn't increase the refCount

		fd->id = GetNextScriptFunctionId();
		AddScriptFunction(fd);

		if (module)
		{
			// Add the new funcdef to the module so it will
			// be available when saving the bytecode
			funcDef->module = module;
			module->AddFuncDef(funcDef); // the refCount was already accounted for in the constructor
		}

		// Observe, if the funcdef is created without informing a module a reference will be stored in the
		// engine's funcDefs array, but it will not be owned by any module. This means that it will live on
		// until the engine is released.
	}

	if (funcDef && module && funcDef->module && funcDef->module != module)
	{
		// Unless this is a registered funcDef the returned funcDef must
		// be stored as part of the module for saving/loading bytecode
		if (!module->m_funcDefs.Exists(funcDef))
		{
			module->AddFuncDef(funcDef);
			funcDef->AddRefInternal();
		}
		else
		{
			asASSERT(funcDef->IsShared());
		}
	}

	return funcDef;
}

// interface
// TODO: typedef: Accept complex types for the typedefs
int asCScriptEngine::RegisterTypedef(const char *type, const char *decl)
{
	if( type == 0 ) return ConfigError(asINVALID_NAME, "RegisterTypedef", type, decl);

	// Verify if the name has been registered as a type already
	// TODO: Must check against registered funcdefs too
	if( GetRegisteredType(type, defaultNamespace) )
		// Let the application recover from this error, for example if the same typedef is registered twice
		return asALREADY_REGISTERED;

	// Grab the data type
	size_t tokenLen;
	eTokenType token;
	asCDataType dataType;

	//	Create the data type
	token = tok.GetToken(decl, strlen(decl), &tokenLen);
	switch(token)
	{
	case ttBool:
	case ttInt:
	case ttInt8:
	case ttInt16:
	case ttInt64:
	case ttUInt:
	case ttUInt8:
	case ttUInt16:
	case ttUInt64:
	case ttFloat:
	case ttDouble:
		if( strlen(decl) != tokenLen )
		{
			return ConfigError(asINVALID_TYPE, "RegisterTypedef", type, decl);
		}
		break;

	default:
		return ConfigError(asINVALID_TYPE, "RegisterTypedef", type, decl);
	}

	dataType = asCDataType::CreatePrimitive(token, false);

	// Make sure the name is not a reserved keyword
	token = tok.GetToken(type, strlen(type), &tokenLen);
	if( token != ttIdentifier || strlen(type) != tokenLen )
		return ConfigError(asINVALID_NAME, "RegisterTypedef", type, decl);

	asCBuilder bld(this, 0);
	int r = bld.CheckNameConflict(type, 0, 0, defaultNamespace, true, false);
	if( r < 0 )
		return ConfigError(asNAME_TAKEN, "RegisterTypedef", type, decl);

	// Don't have to check against members of object
	// types as they are allowed to use the names

	// Put the data type in the list
	asCTypedefType *td = asNEW(asCTypedefType)(this);
	if( td == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterTypedef", type, decl);

	td->flags           = asOBJ_TYPEDEF;
	td->size            = dataType.GetSizeInMemoryBytes();
	td->name            = type;
	td->nameSpace       = defaultNamespace;
	td->aliasForType    = dataType;

	allRegisteredTypes.Insert(asSNameSpaceNamePair(td->nameSpace, td->name), td);
	registeredTypeDefs.PushLast(td);

	currentGroup->types.PushLast(td);

	return GetTypeIdByDecl(type);
}

// interface
asUINT asCScriptEngine::GetTypedefCount() const
{
	return asUINT(registeredTypeDefs.GetLength());
}

// interface
asITypeInfo *asCScriptEngine::GetTypedefByIndex(asUINT index) const
{
	if( index >= registeredTypeDefs.GetLength() )
		return 0;

	return registeredTypeDefs[index];
}

// interface
int asCScriptEngine::RegisterEnum(const char *name)
{
	//	Check the name
	if( NULL == name )
		return ConfigError(asINVALID_NAME, "RegisterEnum", name, 0);

	// Verify if the name has been registered as a type already
	if( GetRegisteredType(name, defaultNamespace) )
		return asALREADY_REGISTERED;

	// Use builder to parse the datatype
	asCDataType dt;
	asCBuilder bld(this, 0);
	bool oldMsgCallback = msgCallback; msgCallback = false;
	int r = bld.ParseDataType(name, &dt, defaultNamespace);
	msgCallback = oldMsgCallback;
	if( r >= 0 )
	{
		// If it is not in the defaultNamespace then the type was successfully parsed because
		// it is declared in a parent namespace which shouldn't be treated as an error
		if( dt.GetTypeInfo() && dt.GetTypeInfo()->nameSpace == defaultNamespace )
			return ConfigError(asERROR, "RegisterEnum", name, 0);
	}

	// Make sure the name is not a reserved keyword
	size_t tokenLen;
	int token = tok.GetToken(name, strlen(name), &tokenLen);
	if( token != ttIdentifier || strlen(name) != tokenLen )
		return ConfigError(asINVALID_NAME, "RegisterEnum", name, 0);

	r = bld.CheckNameConflict(name, 0, 0, defaultNamespace, true, false);
	if( r < 0 )
		return ConfigError(asNAME_TAKEN, "RegisterEnum", name, 0);

	asCEnumType *st = asNEW(asCEnumType)(this);
	if( st == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterEnum", name, 0);

	asCDataType dataType;
	dataType.CreatePrimitive(ttInt, false);

	st->flags = asOBJ_ENUM | asOBJ_SHARED;
	st->size = 4;
	st->name = name;
	st->nameSpace = defaultNamespace;

	allRegisteredTypes.Insert(asSNameSpaceNamePair(st->nameSpace, st->name), st);
	registeredEnums.PushLast(st);

	currentGroup->types.PushLast(st);

	return GetTypeIdByDecl(name);
}

// interface
int asCScriptEngine::RegisterEnumValue(const char *typeName, const char *valueName, int value)
{
	// Verify that the correct config group is used
	if( currentGroup->FindType(typeName) == 0 )
		return ConfigError(asWRONG_CONFIG_GROUP, "RegisterEnumValue", typeName, valueName);

	asCDataType dt;
	int r;
	asCBuilder bld(this, 0);
	r = bld.ParseDataType(typeName, &dt, defaultNamespace);
	if( r < 0 )
		return ConfigError(r, "RegisterEnumValue", typeName, valueName);

	// Store the enum value
	asCEnumType *ot = CastToEnumType(dt.GetTypeInfo());
	if( ot == 0 )
		return ConfigError(asINVALID_TYPE, "RegisterEnumValue", typeName, valueName);

	if( NULL == valueName )
		return ConfigError(asINVALID_NAME, "RegisterEnumValue", typeName, valueName);

	asUINT tokenLen = 0;
	asETokenClass tokenClass = ParseToken(valueName, 0, &tokenLen);
	if( tokenClass != asTC_IDENTIFIER || tokenLen != strlen(valueName) )
		return ConfigError(asINVALID_NAME, "RegisterEnumValue", typeName, valueName);

	for( unsigned int n = 0; n < ot->enumValues.GetLength(); n++ )
	{
		if( ot->enumValues[n]->name == valueName )
			return ConfigError(asALREADY_REGISTERED, "RegisterEnumValue", typeName, valueName);
	}

	asSEnumValue *e = asNEW(asSEnumValue);
	if( e == 0 )
		return ConfigError(asOUT_OF_MEMORY, "RegisterEnumValue", typeName, valueName);

	e->name = valueName;
	e->value = value;

	ot->enumValues.PushLast(e);

	return asSUCCESS;
}

// interface
asUINT asCScriptEngine::GetEnumCount() const
{
	return registeredEnums.GetLength();
}

// interface
asITypeInfo *asCScriptEngine::GetEnumByIndex(asUINT index) const
{
	if( index >= registeredEnums.GetLength() )
		return 0;

	return registeredEnums[index];
}

// interface
asUINT asCScriptEngine::GetObjectTypeCount() const
{
	return asUINT(registeredObjTypes.GetLength());
}

// interface
asITypeInfo *asCScriptEngine::GetObjectTypeByIndex(asUINT index) const
{
	if( index >= registeredObjTypes.GetLength() )
		return 0;

	return registeredObjTypes[index];
}

// interface
asITypeInfo *asCScriptEngine::GetTypeInfoByName(const char *in_name) const
{
	asCString name;
	asSNameSpace *ns = 0;
	if( DetermineNameAndNamespace(in_name, defaultNamespace, name, ns) < 0 )
		return 0;
			
	while (ns)
	{
		// Check the object types
		for (asUINT n = 0; n < registeredObjTypes.GetLength(); n++)
		{
			if (registeredObjTypes[n]->name == name &&
				registeredObjTypes[n]->nameSpace == ns)
				return registeredObjTypes[n];
		}

		// Perhaps it is a template type? In this case
		// the returned type will be the generic type
		for (asUINT n = 0; n < registeredTemplateTypes.GetLength(); n++)
		{
			if (registeredTemplateTypes[n]->name == name &&
				registeredTemplateTypes[n]->nameSpace == ns)
				return registeredTemplateTypes[n];
		}

		// Check the enum types
		for (asUINT n = 0; n < registeredEnums.GetLength(); n++)
		{
			if (registeredEnums[n]->name == name &&
				registeredEnums[n]->nameSpace == ns)
				return registeredEnums[n];
		}

		// Check the typedefs
		for (asUINT n = 0; n < registeredTypeDefs.GetLength();n++)
		{
			if (registeredTypeDefs[n]->name == name &&
				registeredTypeDefs[n]->nameSpace == ns)
				return registeredTypeDefs[n];
		}

		// Recursively search parent namespace
		ns = GetParentNameSpace(ns);
	}

	return 0;
}

// internal
int asCScriptEngine::DetermineNameAndNamespace(const char *in_name, asSNameSpace *implicitNs, asCString &out_name, asSNameSpace *&out_ns) const
{
	if( in_name == 0 )
		return asINVALID_ARG;
	
	asCString name = in_name;
	asCString scope;
	asSNameSpace *ns = implicitNs;
	
	// Check if the given name contains a scope
	int pos = name.FindLast("::");
	if( pos >= 0 )
	{
		scope = name.SubString(0, pos);
		name = name.SubString(pos+2);
		if( pos == 0 )
		{
			// The scope is '::' so the search must start in the global namespace
			ns = nameSpaces[0];
		}
		else if( scope.SubString(0, 2) == "::" )
		{
			// The scope starts with '::' so the given scope is fully qualified
			ns = FindNameSpace(scope.SubString(2).AddressOf());
		}
		else
		{
			// The scope doesn't start with '::' so it is relative to the current namespace
			if( implicitNs->name == "" )
				ns = FindNameSpace(scope.AddressOf());
			else
				ns = FindNameSpace((implicitNs->name + "::" + scope).AddressOf());
		}
	}
	
	out_name = name;
	out_ns = ns;
	
	return 0;
}


// interface
asITypeInfo *asCScriptEngine::GetTypeInfoById(int typeId) const
{
	asCDataType dt = GetDataTypeFromTypeId(typeId);

	// Is the type id valid?
	if (!dt.IsValid()) return 0;

	return dt.GetTypeInfo();
}

// interface
asIScriptFunction *asCScriptEngine::GetFunctionById(int funcId) const
{
	return GetScriptFunction(funcId);
}

// internal
bool asCScriptEngine::IsTemplateType(const char *name) const
{
	// Only look in the list of template types (not instance types)
	for( unsigned int n = 0; n < registeredTemplateTypes.GetLength(); n++ )
	{
		asCObjectType *type = registeredTemplateTypes[n];
		if( type && type->name == name )
			return true;
	}

	return false;
}

// internal
int asCScriptEngine::GetScriptSectionNameIndex(const char *name)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	// TODO: These names are only released when the engine is freed. The assumption is that
	//       the same script section names will be reused instead of there always being new
	//       names. Is this assumption valid? Do we need to add reference counting?

	// Store the script section names for future reference
	for( asUINT n = 0; n < scriptSectionNames.GetLength(); n++ )
	{
		if( scriptSectionNames[n]->Compare(name) == 0 )
		{
			RELEASEEXCLUSIVE(engineRWLock);
			return n;
		}
	}

	asCString *str = asNEW(asCString)(name);
	if( str )
		scriptSectionNames.PushLast(str);
	int r = int(scriptSectionNames.GetLength()-1);

	RELEASEEXCLUSIVE(engineRWLock);

	return r;
}

// interface
void asCScriptEngine::SetEngineUserDataCleanupCallback(asCLEANENGINEFUNC_t callback, asPWORD type)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	for( asUINT n = 0; n < cleanEngineFuncs.GetLength(); n++ )
	{
		if( cleanEngineFuncs[n].type == type )
		{
			cleanEngineFuncs[n].cleanFunc = callback;

			RELEASEEXCLUSIVE(engineRWLock);

			return;
		}
	}
	SEngineClean otc = {type, callback};
	cleanEngineFuncs.PushLast(otc);

	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
void asCScriptEngine::SetModuleUserDataCleanupCallback(asCLEANMODULEFUNC_t callback, asPWORD type)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	for( asUINT n = 0; n < cleanModuleFuncs.GetLength(); n++ )
	{
		if( cleanModuleFuncs[n].type == type )
		{
			cleanModuleFuncs[n].cleanFunc = callback;

			RELEASEEXCLUSIVE(engineRWLock);

			return;
		}
	}
	SModuleClean otc = {type, callback};
	cleanModuleFuncs.PushLast(otc);

	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
void asCScriptEngine::SetContextUserDataCleanupCallback(asCLEANCONTEXTFUNC_t callback, asPWORD type)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	for( asUINT n = 0; n < cleanContextFuncs.GetLength(); n++ )
	{
		if( cleanContextFuncs[n].type == type )
		{
			cleanContextFuncs[n].cleanFunc = callback;

			RELEASEEXCLUSIVE(engineRWLock);

			return;
		}
	}
	SContextClean otc = {type, callback};
	cleanContextFuncs.PushLast(otc);

	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
void asCScriptEngine::SetFunctionUserDataCleanupCallback(asCLEANFUNCTIONFUNC_t callback, asPWORD type)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	for( asUINT n = 0; n < cleanFunctionFuncs.GetLength(); n++ )
	{
		if( cleanFunctionFuncs[n].type == type )
		{
			cleanFunctionFuncs[n].cleanFunc = callback;

			RELEASEEXCLUSIVE(engineRWLock);

			return;
		}
	}
	SFunctionClean otc = {type, callback};
	cleanFunctionFuncs.PushLast(otc);

	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
void asCScriptEngine::SetTypeInfoUserDataCleanupCallback(asCLEANTYPEINFOFUNC_t callback, asPWORD type)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	for( asUINT n = 0; n < cleanTypeInfoFuncs.GetLength(); n++ )
	{
		if( cleanTypeInfoFuncs[n].type == type )
		{
			cleanTypeInfoFuncs[n].cleanFunc = callback;

			RELEASEEXCLUSIVE(engineRWLock);

			return;
		}
	}
	STypeInfoClean otc = {type, callback};
	cleanTypeInfoFuncs.PushLast(otc);

	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
void asCScriptEngine::SetScriptObjectUserDataCleanupCallback(asCLEANSCRIPTOBJECTFUNC_t callback, asPWORD type)
{
	ACQUIREEXCLUSIVE(engineRWLock);

	for( asUINT n = 0; n < cleanScriptObjectFuncs.GetLength(); n++ )
	{
		if( cleanScriptObjectFuncs[n].type == type )
		{
			cleanScriptObjectFuncs[n].cleanFunc = callback;

			RELEASEEXCLUSIVE(engineRWLock);

			return;
		}
	}
	SScriptObjClean soc = {type, callback};
	cleanScriptObjectFuncs.PushLast(soc);

	RELEASEEXCLUSIVE(engineRWLock);
}

// interface
int asCScriptEngine::SetTranslateAppExceptionCallback(asSFuncPtr callback, void *param, int callConv)
{
#ifdef AS_NO_EXCEPTIONS
	return asNOT_SUPPORTED;
#else
	if (callback.ptr.f.func == 0)
	{
		// Clear the callback
		translateExceptionCallback = false;
		return asSUCCESS;
	}

	// Detect the new callback
	translateExceptionCallback = true;
	translateExceptionCallbackObj = param;
	bool isObj = false;
	if ((unsigned)callConv == asCALL_GENERIC || (unsigned)callConv == asCALL_THISCALL_OBJFIRST || (unsigned)callConv == asCALL_THISCALL_OBJLAST)
		return asNOT_SUPPORTED;
	if ((unsigned)callConv >= asCALL_THISCALL)
	{
		isObj = true;
		if (param == 0)
		{
			translateExceptionCallback = false;
			return asINVALID_ARG;
		}
	}
	int r = DetectCallingConvention(isObj, callback, callConv, 0, &translateExceptionCallbackFunc);
	if (r < 0) 
		translateExceptionCallback = false;

	return r;
#endif
}

// internal
asCObjectType *asCScriptEngine::GetListPatternType(int listPatternFuncId)
{
	// Get the object type either from the constructor's object for value types
	// or from the factory's return type for reference types
	asCObjectType *ot = scriptFunctions[listPatternFuncId]->objectType;
	if( ot == 0 )
		ot = CastToObjectType(scriptFunctions[listPatternFuncId]->returnType.GetTypeInfo());
	asASSERT( ot );

	// Check if this object type already has a list pattern type
	for( asUINT n = 0; n < listPatternTypes.GetLength(); n++ )
	{
		if( listPatternTypes[n]->templateSubTypes[0].GetTypeInfo() == ot )
			return listPatternTypes[n];
	}

	// Create a new list pattern type for the given object type
	asCObjectType *lpt = asNEW(asCObjectType)(this);
	lpt->templateSubTypes.PushLast(asCDataType::CreateType(ot, false));
	lpt->flags = asOBJ_LIST_PATTERN;
	listPatternTypes.PushLast(lpt);

	return lpt;
}

// internal
void asCScriptEngine::DestroyList(asBYTE *buffer, const asCObjectType *listPatternType)
{
	asASSERT( listPatternType && (listPatternType->flags & asOBJ_LIST_PATTERN) );

	// Get the list pattern from the listFactory function
	// TODO: runtime optimize: Store the used list factory in the listPatternType itself
	// TODO: runtime optimize: Keep a flag to indicate if there is really a need to free anything
	asCObjectType *ot = CastToObjectType(listPatternType->templateSubTypes[0].GetTypeInfo());
	asCScriptFunction *listFactory = scriptFunctions[ot->beh.listFactory];
	asASSERT( listFactory );

	asSListPatternNode *node = listFactory->listPattern;
	DestroySubList(buffer, node);

	asASSERT( node->type == asLPT_END );
}

// internal
void asCScriptEngine::DestroySubList(asBYTE *&buffer, asSListPatternNode *&node)
{
	asASSERT( node->type == asLPT_START );

	int count = 0;

	node = node->next;
	while( node )
	{
		if( node->type == asLPT_REPEAT || node->type == asLPT_REPEAT_SAME )
		{
			// Align the offset to 4 bytes boundary
			if( (asPWORD(buffer) & 0x3) )
				buffer += 4 - (asPWORD(buffer) & 0x3);

			// Determine how many times the pattern repeat
			count = *(asUINT*)buffer;
			buffer += 4;

			if( count == 0 )
			{
				// Skip the sub pattern that was expected to be repeated, otherwise
				// we'll try to delete things that don't exist in the buffer
				node = node->next;
				if( node->type == asLPT_START )
				{
					int subCount = 1;
					do
					{
						node = node->next;
						if( node->type == asLPT_START )
							subCount++;
						else if( node->type == asLPT_END )
							subCount--;
					} while( subCount > 0 );
					return;
				}
			}
		}
		else if( node->type == asLPT_TYPE )
		{
			// If we're not in a repeat iteration, then only 1 value should be destroyed
			if( count <= 0 )
				count = 1;

			asCDataType dt = reinterpret_cast<asSListPatternDataTypeNode*>(node)->dataType;
			bool isVarType = dt.GetTokenType() == ttQuestion;

			while( count-- )
			{
				if( isVarType )
				{
					// Align the offset to 4 bytes boundary
					if( (asPWORD(buffer) & 0x3) )
						buffer += 4 - (asPWORD(buffer) & 0x3);

					int typeId = *(int*)buffer;
					buffer += 4;
					dt = GetDataTypeFromTypeId(typeId);
				}

				asCTypeInfo *ti = dt.GetTypeInfo();
				if( ti && (ti->flags & asOBJ_ENUM) == 0 )
				{
					// Free all instances of this type
					if( ti->flags & asOBJ_VALUE )
					{
						asUINT size = ti->GetSize();

						// Align the offset to 4 bytes boundary
						if( size >= 4 && (asPWORD(buffer) & 0x3) )
							buffer += 4 - (asPWORD(buffer) & 0x3);

						asCObjectType *ot = CastToObjectType(ti);
						if( ot && ot->beh.destruct )
						{
							// Only call the destructor if the object has been created
							// We'll assume the object has been created if any byte in
							// the memory is different from 0.
							// TODO: This is not really correct, as bytes may have been
							//       modified by the constructor, but then an exception
							//       thrown aborting the initialization. The engine
							//       really should be keeping track of which objects has
							//       been successfully initialized.

							for( asUINT n = 0; n < size; n++ )
							{
								if( buffer[n] != 0 )
								{
									void *ptr = (void*)buffer;
									CallObjectMethod(ptr, ot->beh.destruct);
									break;
								}
							}
						}

						// Advance the pointer in the buffer
						buffer += size;
					}
					else
					{
						// Align the offset to 4 bytes boundary
						if( asPWORD(buffer) & 0x3 )
							buffer += 4 - (asPWORD(buffer) & 0x3);

						// Call the release behaviour
						void *ptr = *(void**)buffer;
						if( ptr )
							ReleaseScriptObject(ptr, ti);
						buffer += AS_PTR_SIZE*4;
					}
				}
				else
				{
					asUINT size = dt.GetSizeInMemoryBytes();

					// Align the offset to 4 bytes boundary
					if( size >= 4 && (asPWORD(buffer) & 0x3) )
						buffer += 4 - (asPWORD(buffer) & 0x3);

					// Advance the buffer
					buffer += size;
				}
			}
		}
		else if( node->type == asLPT_START )
		{
			// If we're not in a repeat iteration, then only 1 value should be destroyed
			if( count <= 0 )
				count = 1;

			while( count-- )
			{
				asSListPatternNode *subList = node;
				DestroySubList(buffer, subList);

				asASSERT( subList->type == asLPT_END );

				if( count == 0 )
					node = subList;
			}
		}
		else if( node->type == asLPT_END )
		{
			return;
		}
		else
		{
			asASSERT( false );
		}

		node = node->next;
	}
}

// internal
asSNameSpace *asCScriptEngine::GetParentNameSpace(asSNameSpace *ns) const
{
	if( ns == 0 ) return 0;
	if( ns == nameSpaces[0] ) return 0;

	asCString scope = ns->name;
	int pos = scope.FindLast("::");
	if( pos >= 0 )
	{
		scope = scope.SubString(0, pos);
		return FindNameSpace(scope.AddressOf());
	}

	return nameSpaces[0];
}

END_AS_NAMESPACE

