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
// as_compiler.cpp
//
// The class that does the actual compilation of the functions
//

#include <math.h> // fmodf() pow()

#include "as_config.h"

#ifndef AS_NO_COMPILER

#include "as_compiler.h"
#include "as_tokendef.h"
#include "as_tokenizer.h"
#include "as_string_util.h"
#include "as_texts.h"
#include "as_parser.h"
#include "as_debug.h"
#include "as_context.h"  // as_powi()

BEGIN_AS_NAMESPACE

//
// The calling convention rules for script functions:
// - If a class method returns a reference, the caller must guarantee the object pointer stays alive until the function returns, and the reference is no longer going to be used
// - If a class method doesn't return a reference, it must guarantee by itself that the this pointer stays alive during the function call. If no outside access is made, then the function is guaranteed to stay alive and nothing needs to be done
// - The object pointer is always passed as the first argument, position 0
// - If the function returns a value type the caller must reserve the memory for this and pass the pointer as the first argument after the object pointer
//





// TODO: I must correct the interpretation of a references to objects in the compiler.
//       A reference should mean that a pointer to the object is on the stack.
//       No expression should end up as non-references to objects, as the actual object is
//       never put on the stack.
//       Local variables are declared as non-references, but the expression should be a reference to the variable.
//       Function parameters of called functions can also be non-references, but in that case it means the
//       object will be passed by value (currently on the heap, which will be moved to the application stack).
//
//       The compiler shouldn't use the asCDataType::IsReference. The datatype should always be stored as non-references.
//       Instead the compiler should keep track of references in TypeInfo, where it should also state how the reference
//       is currently stored, i.e. in variable, in register, on stack, etc.

asCCompiler::asCCompiler(asCScriptEngine *engine) : byteCode(engine)
{
	builder = 0;
	script = 0;

	variables = 0;
	isProcessingDeferredParams = false;
	isCompilingDefaultArg = false;
	noCodeOutput = 0;
}

asCCompiler::~asCCompiler()
{
	while( variables )
	{
		asCVariableScope *var = variables;
		variables = variables->parent;

		asDELETE(var,asCVariableScope);
	}
}

void asCCompiler::Reset(asCBuilder *builder, asCScriptCode *script, asCScriptFunction *outFunc)
{
	this->builder = builder;
	this->engine = builder->engine;
	this->script = script;
	this->outFunc = outFunc;

	hasCompileErrors = false;

	m_isConstructor       = false;
	m_isConstructorCalled = false;
	m_classDecl           = 0;

	nextLabel = 0;
	breakLabels.SetLength(0);
	continueLabels.SetLength(0);

	byteCode.ClearAll();
}

int asCCompiler::CompileDefaultConstructor(asCBuilder *builder, asCScriptCode *script, asCScriptNode *node, asCScriptFunction *outFunc, sClassDeclaration *classDecl)
{
	Reset(builder, script, outFunc);

	m_classDecl = classDecl;

	// Insert a JitEntry at the start of the function for JIT compilers
	byteCode.InstrPTR(asBC_JitEntry, 0);

	// Add a variable scope that might be needed to declare dummy variables
	// in case the member initialization refers to undefined symbols.
	AddVariableScope();

	// Initialize the class members that have no explicit expression first. This will allow the
	// base class' constructor to access these members without worry they will be uninitialized.
	// This can happen if the base class' constructor calls a method that is overridden by the derived class
	CompileMemberInitialization(&byteCode, true);

	// If the class is derived from another, then the base class' default constructor must be called
	if( outFunc->objectType->derivedFrom )
	{
		// Make sure the base class really has a default constructor
		if( outFunc->objectType->derivedFrom->beh.construct == 0 )
			Error(TEXT_BASE_DOESNT_HAVE_DEF_CONSTR, node);

		// Call the base class' default constructor
		byteCode.InstrSHORT(asBC_PSF, 0);
		byteCode.Instr(asBC_RDSPtr);
		byteCode.Call(asBC_CALL, outFunc->objectType->derivedFrom->beh.construct, AS_PTR_SIZE);
	}

	// Initialize the class members that explicit expressions afterwards. This allow the expressions
	// to access the base class members without worry they will be uninitialized
	CompileMemberInitialization(&byteCode, false);
	byteCode.OptimizeLocally(tempVariableOffsets);

	// If there are compile errors, there is no reason to build the final code
	if( hasCompileErrors )
		return -1;

	// Pop the object pointer from the stack
	byteCode.Ret(AS_PTR_SIZE);

	// Count total variable size
	int varSize = GetVariableOffset((int)variableAllocations.GetLength()) - 1;
	outFunc->scriptData->variableSpace = varSize;

	FinalizeFunction();

#ifdef AS_DEBUG
	// DEBUG: output byte code
	byteCode.DebugOutput(("__" + outFunc->objectType->name + "_" + outFunc->name + "__defconstr.txt").AddressOf(), engine, outFunc);
#endif

	return 0;
}

int asCCompiler::CompileFactory(asCBuilder *builder, asCScriptCode *script, asCScriptFunction *outFunc)
{
	Reset(builder, script, outFunc);

	// Insert a JitEntry at the start of the function for JIT compilers
	byteCode.InstrPTR(asBC_JitEntry, 0);

	// Find the corresponding constructor
	asCDataType dt = asCDataType::CreateObject(outFunc->returnType.GetObjectType(), false);
	int constructor = 0;
	for( unsigned int n = 0; n < dt.GetBehaviour()->factories.GetLength(); n++ )
	{
		if( dt.GetBehaviour()->factories[n] == outFunc->id )
		{
			constructor = dt.GetBehaviour()->constructors[n];
			break;
		}
	}

	// Allocate the class and instanciate it with the constructor
	int varOffset = AllocateVariable(dt, true);

	outFunc->scriptData->variableSpace = AS_PTR_SIZE;
	byteCode.InstrSHORT(asBC_PSF, (short)varOffset);

	// Copy all arguments to the top of the stack
	// TODO: runtime optimize: Might be interesting to have a specific instruction for copying all arguments
	int offset = (int)outFunc->GetSpaceNeededForArguments();
	for( int a = int(outFunc->parameterTypes.GetLength()) - 1; a >= 0; a-- )
	{
		if( !outFunc->parameterTypes[a].IsPrimitive() ||
			outFunc->parameterTypes[a].IsReference() )
		{
			offset -= AS_PTR_SIZE;
			byteCode.InstrSHORT(asBC_PshVPtr, short(-offset));
		}
		else
		{
			if( outFunc->parameterTypes[a].GetSizeOnStackDWords() == 2 )
			{
				offset -= 2;
				byteCode.InstrSHORT(asBC_PshV8, short(-offset));
			}
			else
			{
				offset -= 1;
				byteCode.InstrSHORT(asBC_PshV4, short(-offset));
			}
		}
	}

	int argDwords = (int)outFunc->GetSpaceNeededForArguments();
	byteCode.Alloc(asBC_ALLOC, dt.GetObjectType(), constructor, argDwords + AS_PTR_SIZE);

	// Return a handle to the newly created object
	byteCode.InstrSHORT(asBC_LOADOBJ, (short)varOffset);

	byteCode.Ret(argDwords);

	FinalizeFunction();

	// Tell the virtual machine not to clean up parameters on exception
	outFunc->dontCleanUpOnException = true;

/*
#ifdef AS_DEBUG
	// DEBUG: output byte code
	asCString args;
	args.Format("%d", outFunc->parameterTypes.GetLength());
	byteCode.DebugOutput(("__" + outFunc->name + "__factory" + args + ".txt").AddressOf(), engine);
#endif
*/
	return 0;
}

void asCCompiler::FinalizeFunction()
{
	TimeIt("asCCompiler::FinalizeFunction");

	asASSERT( outFunc->scriptData );
	asUINT n;

	// Finalize the bytecode
	byteCode.Finalize(tempVariableOffsets);

	byteCode.ExtractObjectVariableInfo(outFunc);

	// Compile the list of object variables for the exception handler
	// Start with the variables allocated on the heap, and then the ones allocated on the stack
	for( n = 0; n < variableAllocations.GetLength(); n++ )
	{
		if( variableAllocations[n].IsObject() && !variableAllocations[n].IsReference() )
		{
			if( variableIsOnHeap[n] )
			{
				outFunc->scriptData->objVariableTypes.PushLast(variableAllocations[n].GetObjectType());
				outFunc->scriptData->funcVariableTypes.PushLast(variableAllocations[n].GetFuncDef());
				outFunc->scriptData->objVariablePos.PushLast(GetVariableOffset(n));
			}
		}
	}
	outFunc->scriptData->objVariablesOnHeap = asUINT(outFunc->scriptData->objVariablePos.GetLength());
	for( n = 0; n < variableAllocations.GetLength(); n++ )
	{
		if( variableAllocations[n].IsObject() && !variableAllocations[n].IsReference() )
		{
			if( !variableIsOnHeap[n] )
			{
				outFunc->scriptData->objVariableTypes.PushLast(variableAllocations[n].GetObjectType());
				outFunc->scriptData->funcVariableTypes.PushLast(variableAllocations[n].GetFuncDef());
				outFunc->scriptData->objVariablePos.PushLast(GetVariableOffset(n));
			}
		}
	}

	// Copy byte code to the function
	asASSERT( outFunc->scriptData->byteCode.GetLength() == 0 );
	outFunc->scriptData->byteCode.SetLength(byteCode.GetSize());
	byteCode.Output(outFunc->scriptData->byteCode.AddressOf());
	outFunc->AddReferences();
	outFunc->scriptData->stackNeeded = byteCode.largestStackUsed + outFunc->scriptData->variableSpace;
	outFunc->scriptData->lineNumbers = byteCode.lineNumbers;

	// Extract the script section indexes too if there are any entries that are different from the function's script section
	int lastIdx = outFunc->scriptData->scriptSectionIdx;
	for( n = 0; n < byteCode.sectionIdxs.GetLength(); n++ )
	{
		if( byteCode.sectionIdxs[n] != lastIdx )
		{
			lastIdx = byteCode.sectionIdxs[n];
			outFunc->scriptData->sectionIdxs.PushLast(byteCode.lineNumbers[n*2]);
			outFunc->scriptData->sectionIdxs.PushLast(lastIdx);
		}
	}
}

// internal
int asCCompiler::SetupParametersAndReturnVariable(asCArray<asCString> &parameterNames, asCScriptNode *func)
{
	int stackPos = 0;

	if( outFunc->objectType )
		stackPos = -AS_PTR_SIZE; // The first parameter is the pointer to the object

	// Add the first variable scope, which the parameters and
	// variables declared in the outermost statement block is
	// part of.
	AddVariableScope();

	bool isDestructor = false;
	asCDataType returnType;

	// Examine return type
	returnType = outFunc->returnType;

	// Check if this is a constructor or destructor
	if( returnType.GetTokenType() == ttVoid && outFunc->objectType )
	{
		if( outFunc->name[0] == '~' )
			isDestructor = true;
		else if( outFunc->objectType->name == outFunc->name )
			m_isConstructor = true;
	}

	// Is the return type allowed?
	if( (!returnType.CanBeInstanciated() && returnType != asCDataType::CreatePrimitive(ttVoid, false)) ||
		(returnType.IsReference() && !returnType.CanBeInstanciated()) )
	{
		// TODO: Hasn't this been validated by the builder already?
		asCString str;
		str.Format(TXT_RETURN_CANT_BE_s, returnType.Format().AddressOf());
		Error(str, func);
	}

	// If the return type is a value type returned by value the address of the
	// location where the value will be stored is pushed on the stack before
	// the arguments
	if( !(isDestructor || m_isConstructor) && outFunc->DoesReturnOnStack() )
		stackPos -= AS_PTR_SIZE;

	asCVariableScope vs(0);

	// Declare parameters
	asUINT n;
	for( n = 0; n < parameterNames.GetLength(); n++ )
	{
		// Get the parameter type
		asCDataType &type = outFunc->parameterTypes[n];
		asETypeModifiers inoutFlag = n < outFunc->inOutFlags.GetLength() ? outFunc->inOutFlags[n] : asTM_NONE;

		// Is the data type allowed?
		// TODO: Hasn't this been validated by the builder already?
		if( (type.IsReference() && inoutFlag != asTM_INOUTREF && !type.CanBeInstanciated()) ||
			(!type.IsReference() && !type.CanBeInstanciated()) )
		{
			asCString parm = type.Format();
			if( inoutFlag == asTM_INREF )
				parm += "in";
			else if( inoutFlag == asTM_OUTREF )
				parm += "out";

			asCString str;
			str.Format(TXT_PARAMETER_CANT_BE_s, parm.AddressOf());
			Error(str, func);
		}

		// If the parameter has a name then declare it as variable
		if( parameterNames[n] != "" )
		{
			asCString &name = parameterNames[n];
			if( vs.DeclareVariable(name.AddressOf(), type, stackPos, true) < 0 )
			{
				// TODO: It might be an out-of-memory too
				Error(TXT_PARAMETER_ALREADY_DECLARED, func);
			}

			// Add marker for variable declaration
			byteCode.VarDecl((int)outFunc->scriptData->variables.GetLength());
			outFunc->AddVariable(name, type, stackPos);
		}
		else
			vs.DeclareVariable("", type, stackPos, true);

		// Move to next parameter
		stackPos -= type.GetSizeOnStackDWords();
	}

	for( n = asUINT(vs.variables.GetLength()); n-- > 0; )
		variables->DeclareVariable(vs.variables[n]->name.AddressOf(), vs.variables[n]->type, vs.variables[n]->stackOffset, vs.variables[n]->onHeap);

	variables->DeclareVariable("return", returnType, stackPos, true);

	return stackPos;
}

void asCCompiler::CompileMemberInitialization(asCByteCode *byteCode, bool onlyDefaults)
{
	asASSERT( m_classDecl );

	// Initialize each member in the order they were declared
	for( asUINT n = 0; n < outFunc->objectType->properties.GetLength(); n++ )
	{
		asCObjectProperty *prop = outFunc->objectType->properties[n];

		// Check if the property has an initialization expression
		asCScriptNode *declNode = 0;
		asCScriptNode *initNode = 0;
		asCScriptCode *initScript = 0;
		for( asUINT m = 0; m < m_classDecl->propInits.GetLength(); m++ )
		{
			if( m_classDecl->propInits[m].name == prop->name )
			{
				declNode   = m_classDecl->propInits[m].declNode;
				initNode   = m_classDecl->propInits[m].initNode;
				initScript = m_classDecl->propInits[m].file;
				break;
			}
		}

		// If declNode is null, the property was inherited in which case
		// it was already initialized by the base class' constructor
		if( declNode )
		{
			if( initNode )
			{
				if( onlyDefaults )
					continue;

#ifdef AS_NO_MEMBER_INIT
				// Give an error as the initialization in the declaration has been disabled
				asCScriptCode *origScript = script;
				script = initScript;
				Error("Initialization of members in declaration is not supported", initNode);
				script = origScript;

				// Clear the initialization node
				initNode = 0;
				initScript = script;
#else
				// Re-parse the initialization expression as the parser now knows the types, which it didn't earlier
				asCParser parser(builder);
				int r = parser.ParseVarInit(initScript, initNode);
				if( r < 0 )
					continue;

				initNode = parser.GetScriptNode();
#endif
			}
			else
			{
				if( !onlyDefaults )
					continue;
			}

#ifdef AS_NO_MEMBER_INIT
			// The initialization will be done in the asCScriptObject constructor, so
			// here we should just validate that the member has a default constructor
			if( prop->type.IsObject() &&
				!prop->type.IsObjectHandle() &&
				(((prop->type.GetObjectType()->flags & asOBJ_REF) &&
				  prop->type.GetBehaviour()->factory == 0) ||
				 ((prop->type.GetObjectType()->flags & asOBJ_VALUE) &&
				  prop->type.GetBehaviour()->construct == 0 &&
				  !(prop->type.GetObjectType()->flags & asOBJ_POD))) )
			{
				// Class has no default factory/constructor.
				asCString str;
				// TODO: funcdef: asCDataType should have a GetTypeName()
				if( prop->type.GetFuncDef() )
					str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, prop->type.GetFuncDef()->GetName());
				else
					str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, prop->type.GetObjectType()->GetName());
				Error(str, declNode);
			}
#else
			// Temporarily set the script that is being compiled to where the member initialization is declared.
			// The script can be different when including mixin classes from a different script section
			asCScriptCode *origScript = script;
			script = initScript;

			// Add a line instruction with the position of the declaration
			LineInstr(byteCode, declNode->tokenPos);

			// Compile the initialization
			asQWORD constantValue;
			asCByteCode bc(engine);
			CompileInitialization(initNode, &bc, prop->type, declNode, prop->byteOffset, &constantValue, 2);
			byteCode->AddCode(&bc);

			script = origScript;
#endif
		}
	}
}

// Entry
int asCCompiler::CompileFunction(asCBuilder *builder, asCScriptCode *script, asCArray<asCString> &parameterNames, asCScriptNode *func, asCScriptFunction *outFunc, sClassDeclaration *classDecl)
{
	TimeIt("asCCompiler::CompileFunction");

	Reset(builder, script, outFunc);
	int buildErrors = builder->numErrors;

	int stackPos = SetupParametersAndReturnVariable(parameterNames, func);

	//--------------------------------------------
	// Compile the statement block

	if( m_isConstructor )
		m_classDecl = classDecl;

	// We need to parse the statement block now
	asCScriptNode *blockBegin;

	// If the function signature was implicit, e.g. virtual property
	// accessor, then the received node already is the statement block
	if( func->nodeType != snStatementBlock )
		blockBegin = func->lastChild;
	else
		blockBegin = func;

	// TODO: memory: We can parse the statement block one statement at a time, thus save even more memory
	// TODO: optimize: For large functions, the parsing of the statement block can take a long time. Presumably because a lot of memory needs to be allocated
	asCParser parser(builder);
	int r = parser.ParseStatementBlock(script, blockBegin);
	if( r < 0 ) return -1;
	asCScriptNode *block = parser.GetScriptNode();

	// Reserve a label for the cleanup code
	nextLabel++;

	bool hasReturn;
	asCByteCode bc(engine);
	LineInstr(&bc, blockBegin->tokenPos);
	CompileStatementBlock(block, false, &hasReturn, &bc);
	LineInstr(&bc, blockBegin->tokenPos + blockBegin->tokenLength);

	// Make sure there is a return in all paths (if not return type is void)
	// Don't bother with this check if there are compiler errors, e.g. Unreachable code
	if( !hasCompileErrors && outFunc->returnType != asCDataType::CreatePrimitive(ttVoid, false) )
	{
		if( hasReturn == false )
			Error(TXT_NOT_ALL_PATHS_RETURN, blockBegin);
	}

	//------------------------------------------------
	// Concatenate the bytecode

	// Insert a JitEntry at the start of the function for JIT compilers
	byteCode.InstrPTR(asBC_JitEntry, 0);

	if( outFunc->objectType )
	{
		if( m_isConstructor )
		{
			if( outFunc->objectType->derivedFrom )
			{
				// Call the base class' default constructor unless called manually in the code
				if( !m_isConstructorCalled )
				{
					if( outFunc->objectType->derivedFrom->beh.construct )
					{
						// Initialize members without explicit expression first
						CompileMemberInitialization(&byteCode, true);

						// Call base class' constructor
						asCByteCode tmpBC(engine);
						tmpBC.InstrSHORT(asBC_PSF, 0);
						tmpBC.Instr(asBC_RDSPtr);
						tmpBC.Call(asBC_CALL, outFunc->objectType->derivedFrom->beh.construct, AS_PTR_SIZE);
						tmpBC.OptimizeLocally(tempVariableOffsets);
						byteCode.AddCode(&tmpBC);

						// Add the initialization of the members with explicit expressions
						CompileMemberInitialization(&byteCode, false);
					}
					else
						Error(TEXT_BASE_DOESNT_HAVE_DEF_CONSTR, blockBegin);
				}
				else
				{
					// Only initialize members that don't have an explicit expression
					// The members that are explicitly initialized will be initialized after the call to base class' constructor
					CompileMemberInitialization(&byteCode, true);
				}
			}
			else
			{
				// Add the initialization of the members
				CompileMemberInitialization(&byteCode, true);
				CompileMemberInitialization(&byteCode, false);
			}
		}

		// Increase the reference for the object pointer, so that it is guaranteed to live during the entire call
		if( !m_isConstructor && !outFunc->returnType.IsReference() )
		{
			// TODO: runtime optimize: If the function is trivial, i.e. doesn't access any outside functions,
			//                         then this is not necessary. If I implement this, then the function needs
			//                         to set a flag so the exception handler doesn't try to release the handle.
			// It is not necessary to do this for constructors, as they have no outside references that can be released anyway
			// It is not necessary to do this for methods that return references, as the caller is guaranteed to hold a reference to the object
			asCByteCode tmpBC(engine);
			tmpBC.InstrSHORT(asBC_PSF, 0);
			tmpBC.Instr(asBC_RDSPtr);
			tmpBC.Call(asBC_CALLSYS, outFunc->objectType->beh.addref, AS_PTR_SIZE);
			tmpBC.OptimizeLocally(tempVariableOffsets);
			byteCode.AddCode(&tmpBC);
		}
	}

	// Add the code for the statement block
	byteCode.AddCode(&bc);

	// Count total variable size
	int varSize = GetVariableOffset((int)variableAllocations.GetLength()) - 1;
	outFunc->scriptData->variableSpace = varSize;

	// Deallocate all local variables
	int n;
	for( n = (int)variables->variables.GetLength() - 1; n >= 0; n-- )
	{
		sVariable *v = variables->variables[n];
		if( v->stackOffset > 0 )
		{
			// Call variables destructors
			if( v->name != "return" && v->name != "return address" )
				CallDestructor(v->type, v->stackOffset, v->onHeap, &byteCode);

			DeallocateVariable(v->stackOffset);
		}
	}

	// This is the label that return statements jump to
	// in order to exit the function
	byteCode.Label(0);

	// Call destructors for function parameters
	for( n = (int)variables->variables.GetLength() - 1; n >= 0; n-- )
	{
		sVariable *v = variables->variables[n];
		if( v->stackOffset <= 0 )
		{
			// Call variable destructors here, for variables not yet destroyed
			if( v->name != "return" && v->name != "return address" )
				CallDestructor(v->type, v->stackOffset, v->onHeap, &byteCode);
		}

		// Do not deallocate parameters
	}

	// Release the object pointer again
	if( outFunc->objectType && !m_isConstructor && !outFunc->returnType.IsReference() )
		byteCode.InstrW_PTR(asBC_FREE, 0, outFunc->objectType);

	// Check if the number of labels in the functions isn't too many to be handled
	if( nextLabel >= (1<<15) )
		Error(TXT_TOO_MANY_JUMP_LABELS, func);

	// If there are compile errors, there is no reason to build the final code
	if( hasCompileErrors || builder->numErrors != buildErrors )
		return -1;

	// At this point there should be no variables allocated
	asASSERT(variableAllocations.GetLength() == freeVariables.GetLength());

	// Remove the variable scope
	RemoveVariableScope();

	byteCode.Ret(-stackPos);

	FinalizeFunction();

#ifdef AS_DEBUG
	// DEBUG: output byte code
	if( outFunc->objectType )
		byteCode.DebugOutput(("__" + outFunc->objectType->name + "_" + outFunc->name + ".txt").AddressOf(), engine, outFunc);
	else
		byteCode.DebugOutput(("__" + outFunc->name + ".txt").AddressOf(), engine, outFunc);
#endif

	return 0;
}

int asCCompiler::CallCopyConstructor(asCDataType &type, int offset, bool isObjectOnHeap, asCByteCode *bc, asSExprContext *arg, asCScriptNode *node, bool isGlobalVar, bool derefDest)
{
	if( !type.IsObject() )
		return 0;

	// CallCopyConstructor should not be called for object handles.
	asASSERT( !type.IsObjectHandle() );

	asCArray<asSExprContext*> args;
	args.PushLast(arg);

	// The reference parameter must be pushed on the stack
	asASSERT( arg->type.dataType.GetObjectType() == type.GetObjectType() );

	// Since we're calling the copy constructor, we have to trust the function to not do
	// anything stupid otherwise we will just enter a loop, as we try to make temporary
	// copies of the argument in order to guarantee safety.


	if( type.GetObjectType()->flags & asOBJ_REF )
	{
		asSExprContext ctx(engine);

		int func = 0;
		asSTypeBehaviour *beh = type.GetBehaviour();
		if( beh ) func = beh->copyfactory;

		if( func > 0 )
		{
			if( !isGlobalVar )
			{
				// Call factory and store the handle in the given variable
				PerformFunctionCall(func, &ctx, false, &args, type.GetObjectType(), true, offset);

				// Pop the reference left by the function call
				ctx.bc.Instr(asBC_PopPtr);
			}
			else
			{
				// Call factory
				PerformFunctionCall(func, &ctx, false, &args, type.GetObjectType());

				// Store the returned handle in the global variable
				ctx.bc.Instr(asBC_RDSPtr);
				ctx.bc.InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
				ctx.bc.InstrPTR(asBC_REFCPY, type.GetObjectType());
				ctx.bc.Instr(asBC_PopPtr);
				ReleaseTemporaryVariable(ctx.type.stackOffset, &ctx.bc);
			}

			bc->AddCode(&ctx.bc);

			return 0;
		}
	}
	else
	{
		asSTypeBehaviour *beh = type.GetBehaviour();
		int func = beh ? beh->copyconstruct : 0;
		if( func > 0 )
		{
			// Push the address where the object will be stored on the stack, before the argument
			// TODO: When the context is serializable this probably has to be changed, since this
			//       pointer can remain on the stack while the context is suspended. There is no
			//       risk the pointer becomes invalid though, there is just no easy way to serialize it.
			asCByteCode tmp(engine);
			if( isGlobalVar )
				tmp.InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
			else if( isObjectOnHeap )
				tmp.InstrSHORT(asBC_PSF, (short)offset);
			tmp.AddCode(bc);
			bc->AddCode(&tmp);

			// When the object is allocated on the stack the object pointer
			// must be pushed on the stack after the arguments
			if( !isObjectOnHeap )
			{
				asASSERT( !isGlobalVar );
				bc->InstrSHORT(asBC_PSF, (short)offset);
				if( derefDest )
				{
					// The variable is a reference to the real location, so we need to dereference it
					bc->Instr(asBC_RDSPtr);
				}
			}

			asSExprContext ctx(engine);
			PerformFunctionCall(func, &ctx, isObjectOnHeap, &args, type.GetObjectType());

			bc->AddCode(&ctx.bc);

			// TODO: value on stack: This probably needs to be done in PerformFunctionCall
			// Mark the object as initialized
			if( !isObjectOnHeap )
				bc->ObjInfo(offset, asOBJ_INIT);


			return 0;
		}
	}

	// Class has no copy constructor/factory.
	asCString str;
	str.Format(TXT_NO_COPY_CONSTRUCTOR_FOR_s, type.GetObjectType()->GetName());
	Error(str, node);

	return -1;
}

int asCCompiler::CallDefaultConstructor(asCDataType &type, int offset, bool isObjectOnHeap, asCByteCode *bc, asCScriptNode *node, int isVarGlobOrMem, bool derefDest)
{
	if( !type.IsObject() || type.IsObjectHandle() )
		return 0;

	if( type.GetObjectType()->flags & asOBJ_REF )
	{
		asSExprContext ctx(engine);
		ctx.exprNode = node;

		int func = 0;
		asSTypeBehaviour *beh = type.GetBehaviour();
		if( beh ) 
		{
			func = beh->factory;

			// If no trivial default factory is found, look for a factory where all params have default args
			if( func == 0 )
			{
				for( asUINT n = 0; n < beh->factories.GetLength(); n++ )
				{
					asCScriptFunction *f = engine->scriptFunctions[beh->factories[n]];
					if( f->defaultArgs.GetLength() == f->parameterTypes.GetLength() &&
						f->defaultArgs[0] != 0 )
					{
						func = beh->factories[n];
						break;
					}
				}
			}
		}

		if( func > 0 )
		{
			asCArray<asSExprContext *> args;
			asCScriptFunction *f = engine->scriptFunctions[func];
			if( f->parameterTypes.GetLength() )
			{
				// Add the default values for arguments not explicitly supplied
				CompileDefaultArgs(node, args, f);

				PrepareFunctionCall(func, &ctx.bc, args);

				MoveArgsToStack(func, &ctx.bc, args, false);
			}

			if( isVarGlobOrMem == 0 )
			{
				// Call factory and store the handle in the given variable
				PerformFunctionCall(func, &ctx, false, &args, type.GetObjectType(), true, offset);

				// Pop the reference left by the function call
				ctx.bc.Instr(asBC_PopPtr);
			}
			else
			{
				// Call factory
				PerformFunctionCall(func, &ctx, false, &args, type.GetObjectType());

				// TODO: runtime optimize: Should have a way of storing the object pointer directly to the destination
				//                         instead of first storing it in a local variable and then copying it to the
				//                         destination.

				if( !(type.GetObjectType()->flags & asOBJ_SCOPED) )
				{
					// Only dereference the variable if not a scoped type
					ctx.bc.Instr(asBC_RDSPtr);
				}

				if( isVarGlobOrMem == 1 )
				{
					// Store the returned handle in the global variable
					ctx.bc.InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
				}
				else
				{
					// Store the returned handle in the class member
					ctx.bc.InstrSHORT(asBC_PSF, 0);
					ctx.bc.Instr(asBC_RDSPtr);
					ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
				}

				if( type.GetObjectType()->flags & asOBJ_SCOPED )
				{
					// For scoped typed we must move the reference from the local
					// variable rather than copy it as there is no AddRef behaviour
					ctx.bc.InstrSHORT_DW(asBC_COPY, AS_PTR_SIZE, asTYPEID_OBJHANDLE | engine->GetTypeIdFromDataType(type));

					// Clear the local variable so the reference isn't released
					ctx.bc.InstrSHORT(asBC_ClrVPtr, ctx.type.stackOffset);
				}
				else
				{
					ctx.bc.InstrPTR(asBC_REFCPY, type.GetObjectType());
				}
				ctx.bc.Instr(asBC_PopPtr);
				ReleaseTemporaryVariable(ctx.type.stackOffset, &ctx.bc);
			}

			bc->AddCode(&ctx.bc);

			// Cleanup
			for( asUINT n = 0; n < args.GetLength(); n++ )
				if( args[n] )
				{
					asDELETE(args[n],asSExprContext);
				}

			return 0;
		}
	}
	else
	{
		asSExprContext ctx(engine);
		ctx.exprNode = node;

		asSTypeBehaviour *beh = type.GetBehaviour();

		int func = 0;
		if( beh ) 
		{
			func = beh->construct;

			// If no trivial default constructor is found, look for a constructor where all params have default args
			if( func == 0 )
			{
				for( asUINT n = 0; n < beh->constructors.GetLength(); n++ )
				{
					asCScriptFunction *f = engine->scriptFunctions[beh->constructors[n]];
					if( f->defaultArgs.GetLength() == f->parameterTypes.GetLength() &&
						f->defaultArgs[0] != 0 )
					{
						func = beh->constructors[n];
						break;
					}
				}
			}
		}

		// Allocate and initialize with the default constructor
		if( func != 0 || (type.GetObjectType()->flags & asOBJ_POD) )
		{
			asCArray<asSExprContext *> args;
			asCScriptFunction *f = engine->scriptFunctions[func];
			if( f && f->parameterTypes.GetLength() )
			{
				// Add the default values for arguments not explicitly supplied
				CompileDefaultArgs(node, args, f);

				PrepareFunctionCall(func, &ctx.bc, args);

				MoveArgsToStack(func, &ctx.bc, args, false);
			}

			if( !isObjectOnHeap )
			{
				if( isVarGlobOrMem == 0 )
				{
					// There is nothing to do if there is no function,
					// as the memory is already allocated on the stack
					if( func )
					{
						// Call the constructor as a normal function
						bc->InstrSHORT(asBC_PSF, (short)offset);
						if( derefDest )
							bc->Instr(asBC_RDSPtr);

						asSExprContext ctx(engine);
						PerformFunctionCall(func, &ctx, false, 0, type.GetObjectType());
						bc->AddCode(&ctx.bc);

						// TODO: value on stack: This probably needs to be done in PerformFunctionCall
						// Mark the object as initialized
						bc->ObjInfo(offset, asOBJ_INIT);
					}
				}
				else if( isVarGlobOrMem == 2 )
				{
					// Only POD types can be allocated inline in script classes
					asASSERT( type.GetObjectType()->flags & asOBJ_POD );

					if( func )
					{
						// Call the constructor as a normal function
						bc->InstrSHORT(asBC_PSF, 0);
						bc->Instr(asBC_RDSPtr);
						bc->InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));

						asSExprContext ctx(engine);
						PerformFunctionCall(func, &ctx, false, 0, type.GetObjectType());
						bc->AddCode(&ctx.bc);
					}
				}
				else 
				{
					asASSERT( false );
				}
			}
			else
			{
				if( isVarGlobOrMem == 0 )
					bc->InstrSHORT(asBC_PSF, (short)offset);
				else if( isVarGlobOrMem == 1 )
					bc->InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
				else
				{
					bc->InstrSHORT(asBC_PSF, 0);
					bc->Instr(asBC_RDSPtr);
					bc->InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
				}

				bc->Alloc(asBC_ALLOC, type.GetObjectType(), func, AS_PTR_SIZE);
			}

			// Cleanup
			for( asUINT n = 0; n < args.GetLength(); n++ )
				if( args[n] )
				{
					asDELETE(args[n],asSExprContext);
				}

			return 0;
		}
	}

	// Class has no default factory/constructor.
	asCString str;
	// TODO: funcdef: asCDataType should have a GetTypeName()
	if( type.GetFuncDef() )
		str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, type.GetFuncDef()->GetName());
	else
		str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, type.GetObjectType()->GetName());
	Error(str, node);

	return -1;
}

void asCCompiler::CallDestructor(asCDataType &type, int offset, bool isObjectOnHeap, asCByteCode *bc)
{
	if( !type.IsReference() )
	{
		// Call destructor for the data type
		if( type.IsObject() )
		{
			// Nothing is done for list pattern types, as this is taken care of by the CompileInitList method
			if( type.GetObjectType()->flags & asOBJ_LIST_PATTERN )
				return;

			if( isObjectOnHeap || type.IsObjectHandle() )
			{
				// Free the memory
				bc->InstrW_PTR(asBC_FREE, (short)offset, type.GetObjectType());
			}
			else
			{
				asASSERT( type.GetObjectType()->GetFlags() & asOBJ_VALUE );

				if( type.GetBehaviour()->destruct )
				{
					// Call the destructor as a regular function
					asSExprContext ctx(engine);
					ctx.bc.InstrSHORT(asBC_PSF, (short)offset);
					PerformFunctionCall(type.GetBehaviour()->destruct, &ctx);
					ctx.bc.OptimizeLocally(tempVariableOffsets);
					bc->AddCode(&ctx.bc);
				}

				// TODO: Value on stack: This probably needs to be done in PerformFunctionCall
				// Mark the object as destroyed
				bc->ObjInfo(offset, asOBJ_UNINIT);
			}
		}
	}
}

void asCCompiler::LineInstr(asCByteCode *bc, size_t pos)
{
	int r, c;
	script->ConvertPosToRowCol(pos, &r, &c);
	bc->Line(r, c, script->idx);
}

void asCCompiler::CompileStatementBlock(asCScriptNode *block, bool ownVariableScope, bool *hasReturn, asCByteCode *bc)
{
	*hasReturn = false;
	bool isFinished = false;
	bool hasUnreachableCode = false;
	bool hasReturnBefore = false;

	if( ownVariableScope )
	{
		bc->Block(true);
		AddVariableScope();
	}

	asCScriptNode *node = block->firstChild;
	while( node )
	{
		if( !hasUnreachableCode && (*hasReturn || isFinished) )
		{
			// Empty statements don't count
			if( node->nodeType != snExpressionStatement || node->firstChild )
			{
				hasUnreachableCode = true;
				Warning(TXT_UNREACHABLE_CODE, node);
			}

			if( *hasReturn )
				hasReturnBefore = true;
		}

		if( node->nodeType == snBreak || node->nodeType == snContinue )
			isFinished = true;

		asCByteCode statement(engine);
		if( node->nodeType == snDeclaration )
			CompileDeclaration(node, &statement);
		else
			CompileStatement(node, hasReturn, &statement);

		// Ignore missing returns in unreachable code paths
		if( !(*hasReturn) && hasReturnBefore )
			*hasReturn = true;

		LineInstr(bc, node->tokenPos);
		bc->AddCode(&statement);

		if( !hasCompileErrors )
		{
			asASSERT( tempVariables.GetLength() == 0 );
			asASSERT( reservedVariables.GetLength() == 0 );
		}

		node = node->next;
	}

	if( ownVariableScope )
	{
		// Deallocate variables in this block, in reverse order
		for( int n = (int)variables->variables.GetLength() - 1; n >= 0; n-- )
		{
			sVariable *v = variables->variables[n];

			// Call variable destructors here, for variables not yet destroyed
			// If the block is terminated with a break, continue, or
			// return the variables are already destroyed
			if( !isFinished && !*hasReturn )
				CallDestructor(v->type, v->stackOffset, v->onHeap, bc);

			// Don't deallocate function parameters
			if( v->stackOffset > 0 )
				DeallocateVariable(v->stackOffset);
		}

		RemoveVariableScope();
		bc->Block(false);
	}
}

// Entry
int asCCompiler::CompileGlobalVariable(asCBuilder *builder, asCScriptCode *script, asCScriptNode *node, sGlobalVariableDescription *gvar, asCScriptFunction *outFunc)
{
	Reset(builder, script, outFunc);

	// Add a variable scope (even though variables can't be declared)
	AddVariableScope();

	gvar->isPureConstant = false;

	// Parse the initialization nodes
	asCParser parser(builder);
	if( node )
	{
		int r = parser.ParseVarInit(script, node);
		if( r < 0 )
			return r;

		node = parser.GetScriptNode();
	}

	// Compile the expression
	asSExprContext ctx(engine);
	asQWORD constantValue;
	if( CompileInitialization(node, &ctx.bc, gvar->datatype, gvar->declaredAtNode, gvar->index, &constantValue, 1) )
	{
		// Should the variable be marked as pure constant?
		if( gvar->datatype.IsPrimitive() && gvar->datatype.IsReadOnly() )
		{
			gvar->isPureConstant = true;
			gvar->constantValue = constantValue;
		}
	}

	// Concatenate the bytecode
	int varSize = GetVariableOffset((int)variableAllocations.GetLength()) - 1;

	// Add information on the line number for the global variable
	size_t pos = 0;
	if( gvar->declaredAtNode )
		pos = gvar->declaredAtNode->tokenPos;
	else if( gvar->initializationNode )
		pos = gvar->initializationNode->tokenPos;
	LineInstr(&byteCode, pos);

	// Reserve space for all local variables
	outFunc->scriptData->variableSpace = varSize;

	ctx.bc.OptimizeLocally(tempVariableOffsets);

	byteCode.AddCode(&ctx.bc);

	// Deallocate variables in this block, in reverse order
	for( int n = (int)variables->variables.GetLength() - 1; n >= 0; --n )
	{
		sVariable *v = variables->variables[n];

		// Call variable destructors here, for variables not yet destroyed
		CallDestructor(v->type, v->stackOffset, v->onHeap, &byteCode);

		DeallocateVariable(v->stackOffset);
	}

	if( hasCompileErrors ) return -1;

	// At this point there should be no variables allocated
	asASSERT(variableAllocations.GetLength() == freeVariables.GetLength());

	// Remove the variable scope again
	RemoveVariableScope();

	byteCode.Ret(0);

	FinalizeFunction();

#ifdef AS_DEBUG
	// DEBUG: output byte code
	byteCode.DebugOutput(("___init_" + gvar->name + ".txt").AddressOf(), engine, outFunc);
#endif

	return 0;
}

void asCCompiler::DetermineSingleFunc(asSExprContext *ctx, asCScriptNode *node)
{
	// Don't do anything if this is not a deferred global function
	if( !ctx->IsGlobalFunc() )
		return;

	// Determine the namespace
	asSNameSpace *ns = 0;
	asCString name = "";
	int pos = ctx->methodName.FindLast("::");
	if( pos >= 0 )
	{
		asCString nsName = ctx->methodName.SubString(0, pos+2);
		
		// Cut off the :: 
		if( nsName.GetLength() > 2 )
			nsName.SetLength(nsName.GetLength()-2);

		ns = DetermineNameSpace(nsName);
		name = ctx->methodName.SubString(pos+2);
	}
	else
	{
		DetermineNameSpace("");
		name = ctx->methodName;
	}

	asCArray<int> funcs;
	if( ns )
		builder->GetFunctionDescriptions(name.AddressOf(), funcs, ns);

	// CompileVariableAccess should guarantee that at least one function is exists
	asASSERT( funcs.GetLength() > 0 );

	if( funcs.GetLength() > 1 )
	{
		asCString str;
		str.Format(TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s, ctx->methodName.AddressOf());
		Error(str, node);

		// Fall through so the compiler can continue as if only one function was matching
	}

	// A shared object may not access global functions unless they too are shared (e.g. registered functions)
	if( !builder->GetFunctionDescription(funcs[0])->IsShared() &&
		outFunc->IsShared() )
	{
		asCString msg;
		msg.Format(TXT_SHARED_CANNOT_CALL_NON_SHARED_FUNC_s, builder->GetFunctionDescription(funcs[0])->GetDeclaration());
		Error(msg, node);
		
		// Fall through so the compiler can continue anyway
	}

	// Push the function pointer on the stack
	ctx->bc.InstrPTR(asBC_FuncPtr, builder->GetFunctionDescription(funcs[0]));
	ctx->type.Set(asCDataType::CreateFuncDef(builder->GetFunctionDescription(funcs[0])));
	ctx->type.dataType.MakeHandle(true);
	ctx->type.isExplicitHandle = true;
	ctx->methodName = "";
}

void asCCompiler::PrepareArgument(asCDataType *paramType, asSExprContext *ctx, asCScriptNode *node, bool isFunction, int refType, bool isMakingCopy)
{
	asCDataType param = *paramType;
	if( paramType->GetTokenType() == ttQuestion )
	{
		// The function is expecting a var type. If the argument is a function name, we must now decide which function it is
		DetermineSingleFunc(ctx, node);

		// Since the function is expecting a var type ?, then we don't want to convert the argument to anything else
		param = ctx->type.dataType;
		param.MakeHandle(ctx->type.isExplicitHandle || ctx->type.IsNullConstant());

		// If value assign is disabled for reference types, then make
		// sure to always pass the handle to ? parameters
		if( builder->engine->ep.disallowValueAssignForRefType && 
			ctx->type.dataType.GetObjectType() && (ctx->type.dataType.GetObjectType()->flags & asOBJ_REF) && !(ctx->type.dataType.GetObjectType()->flags & asOBJ_SCOPED) )
		{
			param.MakeHandle(true);
		}

		param.MakeReference(paramType->IsReference());
		param.MakeReadOnly(paramType->IsReadOnly());
	}
	else
		param = *paramType;

	asCDataType dt = param;

	// Need to protect arguments by reference
	if( isFunction && dt.IsReference() )
	{
		// Allocate a temporary variable of the same type as the argument
		dt.MakeReference(false);
		dt.MakeReadOnly(false);

		int offset;
		if( refType == 1 ) // &in
		{
			ProcessPropertyGetAccessor(ctx, node);

			// Add the type id as hidden arg if the parameter is a ? type
			if( paramType->GetTokenType() == ttQuestion )
			{
				asCByteCode tmpBC(engine);

				// Place the type id on the stack as a hidden parameter
				tmpBC.InstrDWORD(asBC_TYPEID, engine->GetTypeIdFromDataType(param));

				// Insert the code before the expression code
				tmpBC.AddCode(&ctx->bc);
				ctx->bc.AddCode(&tmpBC);
			}

			// If the reference is const, then it is not necessary to make a copy if the value already is a variable
			// Even if the same variable is passed in another argument as non-const then there is no problem
			if( dt.IsPrimitive() || dt.IsNullHandle() )
			{
				IsVariableInitialized(&ctx->type, node);

				if( ctx->type.dataType.IsReference() ) ConvertToVariable(ctx);
				ImplicitConversion(ctx, dt, node, asIC_IMPLICIT_CONV, true);

				if( !(param.IsReadOnly() && ctx->type.isVariable) )
					ConvertToTempVariable(ctx);

				PushVariableOnStack(ctx, true);
				ctx->type.dataType.MakeReadOnly(param.IsReadOnly());
			}
			else
			{
				IsVariableInitialized(&ctx->type, node);

				if( !isMakingCopy )
				{
					ImplicitConversion(ctx, param, node, asIC_IMPLICIT_CONV, true);

					if( !ctx->type.dataType.IsEqualExceptRef(param) )
					{
						asCString str;
						str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, ctx->type.dataType.Format().AddressOf(), param.Format().AddressOf());
						Error(str, node);

						ctx->type.Set(param);
					}
				}

				// If the argument already is a temporary
				// variable we don't need to allocate another

				// If the parameter is read-only and the object already is a local
				// variable then it is not necessary to make a copy either
				if( !ctx->type.isTemporary && !(param.IsReadOnly() && ctx->type.isVariable) && !isMakingCopy )
				{
					// TODO: 2.28.1: Need to reserve variables, as the default constructor may need 
					//               to allocate temporary variables to compute default args
					// Make sure the variable is not used in the expression
					offset = AllocateVariableNotIn(dt, true, false, ctx);

					// TODO: copy: Use copy constructor if available. See PrepareTemporaryObject()

					// Allocate and construct the temporary object
					asCByteCode tmpBC(engine);

					CallDefaultConstructor(dt, offset, IsVariableOnHeap(offset), &tmpBC, node);

					// Insert the code before the expression code
					tmpBC.AddCode(&ctx->bc);
					ctx->bc.AddCode(&tmpBC);

					// Assign the evaluated expression to the temporary variable
					PrepareForAssignment(&dt, ctx, node, true);

					dt.MakeReference(IsVariableOnHeap(offset));
					asCTypeInfo type;
					type.Set(dt);
					type.isTemporary = true;
					type.stackOffset = (short)offset;

					if( dt.IsObjectHandle() )
						type.isExplicitHandle = true;

					ctx->bc.InstrSHORT(asBC_PSF, (short)offset);

					PerformAssignment(&type, &ctx->type, &ctx->bc, node);

					ctx->bc.Instr(asBC_PopPtr);

					ReleaseTemporaryVariable(ctx->type, &ctx->bc);

					ctx->type.Set(dt);
					ctx->type.isTemporary = true;
					ctx->type.stackOffset = offset;
					if( dt.IsObjectHandle() )
						ctx->type.isExplicitHandle = true;
					ctx->type.dataType.MakeReference(false);

					ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
					if( dt.IsObject() && !dt.IsObjectHandle() )
						ctx->bc.Instr(asBC_RDSPtr);

					if( paramType->IsReadOnly() )
						ctx->type.dataType.MakeReadOnly(true);
				}
				else if( isMakingCopy )
				{
					// We must guarantee that the address to the value is on the stack
					if( ctx->type.dataType.IsObject() &&
						!ctx->type.dataType.IsObjectHandle() &&
						ctx->type.dataType.IsReference() )
						Dereference(ctx, true);
				}
			}
		}
		else if( refType == 2 ) // &out
		{
			// Add the type id as hidden arg if the parameter is a ? type
			if( paramType->GetTokenType() == ttQuestion )
			{
				asCByteCode tmpBC(engine);

				// Place the type id on the stack as a hidden parameter
				tmpBC.InstrDWORD(asBC_TYPEID, engine->GetTypeIdFromDataType(param));

				// Insert the code before the expression code
				tmpBC.AddCode(&ctx->bc);
				ctx->bc.AddCode(&tmpBC);
			}

			// Make sure the variable is not used in the expression
			offset = AllocateVariableNotIn(dt, true, false, ctx);

			if( dt.IsPrimitive() )
			{
				ctx->type.SetVariable(dt, offset, true);
				PushVariableOnStack(ctx, true);
			}
			else
			{
				// TODO: 2.28.1: Need to reserve variables, as the default constructor may need 
				//               to allocate temporary variables to compute default args

				// Allocate and construct the temporary object
				asCByteCode tmpBC(engine);
				CallDefaultConstructor(dt, offset, IsVariableOnHeap(offset), &tmpBC, node);

				// Insert the code before the expression code
				tmpBC.AddCode(&ctx->bc);
				ctx->bc.AddCode(&tmpBC);

				dt.MakeReference((!dt.IsObject() || dt.IsObjectHandle()));
				asCTypeInfo type;
				type.Set(dt);
				type.isTemporary = true;
				type.stackOffset = (short)offset;

				ctx->type = type;

				ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
				if( dt.IsObject() && !dt.IsObjectHandle() )
					ctx->bc.Instr(asBC_RDSPtr);
			}

			// After the function returns the temporary variable will
			// be assigned to the expression, if it is a valid lvalue
		}
		else if( refType == asTM_INOUTREF )
		{
			ProcessPropertyGetAccessor(ctx, node);

			// Add the type id as hidden arg if the parameter is a ? type
			if( paramType->GetTokenType() == ttQuestion )
			{
				asCByteCode tmpBC(engine);

				// Place the type id on the stack as a hidden parameter
				tmpBC.InstrDWORD(asBC_TYPEID, engine->GetTypeIdFromDataType(param));

				// Insert the code before the expression code
				tmpBC.AddCode(&ctx->bc);
				ctx->bc.AddCode(&tmpBC);
			}

			// Literal constants cannot be passed to inout ref arguments
			if( !ctx->type.isVariable && ctx->type.isConstant )
			{
				Error(TXT_NOT_VALID_REFERENCE, node);
			}

			// Only objects that support object handles
			// can be guaranteed to be safe. Local variables are
			// already safe, so there is no need to add an extra
			// references
			if( !engine->ep.allowUnsafeReferences &&
				!ctx->type.isVariable &&
				ctx->type.dataType.IsObject() &&
				!ctx->type.dataType.IsObjectHandle() &&
				((ctx->type.dataType.GetBehaviour()->addref &&
				  ctx->type.dataType.GetBehaviour()->release) ||
				 (ctx->type.dataType.GetObjectType()->flags & asOBJ_NOCOUNT)) )
			{
				// Store a handle to the object as local variable
				asSExprContext tmp(engine);
				asCDataType dt = ctx->type.dataType;
				dt.MakeHandle(true);
				dt.MakeReference(false);

				offset = AllocateVariableNotIn(dt, true, false, ctx);

				// Copy the handle
				if( !ctx->type.dataType.IsObjectHandle() && ctx->type.dataType.IsReference() )
					ctx->bc.Instr(asBC_RDSPtr);
				ctx->bc.InstrWORD(asBC_PSF, (asWORD)offset);
				ctx->bc.InstrPTR(asBC_REFCPY, ctx->type.dataType.GetObjectType());
				ctx->bc.Instr(asBC_PopPtr);
				ctx->bc.InstrWORD(asBC_PSF, (asWORD)offset);

				dt.MakeHandle(false);
				dt.MakeReference(true);

				// Release previous temporary variable stored in the context (if any)
				if( ctx->type.isTemporary )
				{
					ReleaseTemporaryVariable(ctx->type.stackOffset, &ctx->bc);
				}

				ctx->type.SetVariable(dt, offset, true);
			}

			// Make sure the reference to the value is on the stack
			// For objects, the reference needs to be dereferenced so the pointer on the stack is to the actual object
			// For handles, the reference shouldn't be changed because the pointer on the stack should be to the handle
			if( ctx->type.dataType.IsObject() && ctx->type.dataType.IsReference() && !param.IsObjectHandle() )
				Dereference(ctx, true);
			else if( ctx->type.isVariable && !ctx->type.dataType.IsObject() )
				ctx->bc.InstrSHORT(asBC_PSF, ctx->type.stackOffset);
			else if( ctx->type.dataType.IsPrimitive() )
				ctx->bc.Instr(asBC_PshRPtr);
			else if( ctx->type.dataType.IsObjectHandle() && !ctx->type.dataType.IsReference() )
				ImplicitConversion(ctx, param, node, asIC_IMPLICIT_CONV, true, false);
		}
	}
	else
	{
		ProcessPropertyGetAccessor(ctx, node);

		if( dt.IsPrimitive() )
		{
			IsVariableInitialized(&ctx->type, node);

			if( ctx->type.dataType.IsReference() ) ConvertToVariable(ctx);

			// Implicitly convert primitives to the parameter type
			ImplicitConversion(ctx, dt, node, asIC_IMPLICIT_CONV);

			if( ctx->type.isVariable )
			{
				PushVariableOnStack(ctx, dt.IsReference());
			}
			else if( ctx->type.isConstant )
			{
				ConvertToVariable(ctx);
				PushVariableOnStack(ctx, dt.IsReference());
			}
		}
		else
		{
			IsVariableInitialized(&ctx->type, node);

			// Implicitly convert primitives to the parameter type
			ImplicitConversion(ctx, dt, node, asIC_IMPLICIT_CONV);

			// Was the conversion successful?
			if( !ctx->type.dataType.IsEqualExceptRef(dt) )
			{
				asCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, ctx->type.dataType.Format().AddressOf(), dt.Format().AddressOf());
				Error(str, node);

				ctx->type.Set(dt);
			}

			if( dt.IsObjectHandle() )
				ctx->type.isExplicitHandle = true;

			if( dt.IsObject() )
			{
				if( !dt.IsReference() )
				{
					// Objects passed by value must be placed in temporary variables
					// so that they are guaranteed to not be referenced anywhere else.
					// The object must also be allocated on the heap, as the memory will
					// be deleted by in as_callfunc_xxx.
					// TODO: value on stack: How can we avoid this unnecessary allocation?

					// Local variables doesn't need to be copied into
					// a temp if we're already compiling an assignment
					if( !isMakingCopy || !ctx->type.dataType.IsObjectHandle() || !ctx->type.isVariable )
						PrepareTemporaryObject(node, ctx, true);

					// The implicit conversion shouldn't convert the object to
					// non-reference yet. It will be dereferenced just before the call.
					// Otherwise the object might be missed by the exception handler.
					dt.MakeReference(true);
				}
				else
				{
					// An object passed by reference should place the pointer to
					// the object on the stack.
					dt.MakeReference(false);
				}
			}
		}
	}

	// Don't put any pointer on the stack yet
	if( param.IsReference() || param.IsObject() )
	{
		// &inout parameter may leave the reference on the stack already
		if( refType != 3 )
		{
			asASSERT( ctx->type.isVariable || ctx->type.isTemporary || isMakingCopy );

			if( ctx->type.isVariable || ctx->type.isTemporary )
			{
				ctx->bc.Instr(asBC_PopPtr);
				ctx->bc.InstrSHORT(asBC_VAR, ctx->type.stackOffset);

				ProcessDeferredParams(ctx);
			}
		}
	}
}

void asCCompiler::PrepareFunctionCall(int funcId, asCByteCode *bc, asCArray<asSExprContext *> &args)
{
	// When a match has been found, compile the final byte code using correct parameter types
	asCScriptFunction *descr = builder->GetFunctionDescription(funcId);

	// If the function being called is the opAssign or copy constructor for the same type
	// as the argument, then we should avoid making temporary copy of the argument
	bool makingCopy = false;
	if( descr->parameterTypes.GetLength() == 1 &&
		descr->parameterTypes[0].IsEqualExceptRefAndConst(args[0]->type.dataType) &&
		((descr->name == "opAssign" && descr->objectType && descr->objectType == args[0]->type.dataType.GetObjectType()) ||
		 (args[0]->type.dataType.GetObjectType() && descr->name == args[0]->type.dataType.GetObjectType()->name)) )
		makingCopy = true;

	// Add code for arguments
	asSExprContext e(engine);
	for( int n = (int)args.GetLength()-1; n >= 0; n-- )
	{
		// Make sure PrepareArgument doesn't use any variable that is already
		// being used by any of the following argument expressions
		int l = int(reservedVariables.GetLength());
		for( int m = n-1; m >= 0; m-- )
			args[m]->bc.GetVarsUsed(reservedVariables);

		PrepareArgument2(&e, args[n], &descr->parameterTypes[n], true, descr->inOutFlags[n], makingCopy);
		reservedVariables.SetLength(l);
	}

	bc->AddCode(&e.bc);
}

void asCCompiler::MoveArgsToStack(int funcId, asCByteCode *bc, asCArray<asSExprContext *> &args, bool addOneToOffset)
{
	asCScriptFunction *descr = builder->GetFunctionDescription(funcId);

	int offset = 0;
	if( addOneToOffset )
		offset += AS_PTR_SIZE;

	// The address of where the return value should be stored is push on top of the arguments
	if( descr->DoesReturnOnStack() )
		offset += AS_PTR_SIZE;

#ifdef AS_DEBUG
	// If the function being called is the opAssign or copy constructor for the same type
	// as the argument, then we should avoid making temporary copy of the argument
	bool makingCopy = false;
	if( descr->parameterTypes.GetLength() == 1 &&
		descr->parameterTypes[0].IsEqualExceptRefAndConst(args[0]->type.dataType) &&
		((descr->name == "opAssign" && descr->objectType && descr->objectType == args[0]->type.dataType.GetObjectType()) ||
		 (args[0]->type.dataType.GetObjectType() && descr->name == args[0]->type.dataType.GetObjectType()->name)) )
		makingCopy = true;
#endif

	// Move the objects that are sent by value to the stack just before the call
	for( asUINT n = 0; n < descr->parameterTypes.GetLength(); n++ )
	{
		if( descr->parameterTypes[n].IsReference() )
		{
			if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() )
			{
				if( descr->inOutFlags[n] != asTM_INOUTREF )
				{
#ifdef AS_DEBUG
					asASSERT( args[n]->type.isVariable || args[n]->type.isTemporary || makingCopy );
#endif

					if( (args[n]->type.isVariable || args[n]->type.isTemporary) )
					{
						if( !IsVariableOnHeap(args[n]->type.stackOffset) )
							// TODO: runtime optimize: Actually the reference can be pushed on the stack directly
							//                         as the value allocated on the stack is guaranteed to be safe
							bc->InstrWORD(asBC_GETREF, (asWORD)offset);
						else
							bc->InstrWORD(asBC_GETOBJREF, (asWORD)offset);
					}
				}
				if( args[n]->type.dataType.IsObjectHandle() )
					bc->InstrWORD(asBC_ChkNullS, (asWORD)offset);
			}
			else if( descr->inOutFlags[n] != asTM_INOUTREF )
			{
				if( descr->parameterTypes[n].GetTokenType() == ttQuestion &&
					args[n]->type.dataType.IsObject() && !args[n]->type.dataType.IsObjectHandle() )
				{
					// Send the object as a reference to the object,
					// and not to the variable holding the object
					if( !IsVariableOnHeap(args[n]->type.stackOffset) )
						// TODO: runtime optimize: Actually the reference can be pushed on the stack directly
						//                         as the value allocated on the stack is guaranteed to be safe
						bc->InstrWORD(asBC_GETREF, (asWORD)offset);
					else
						bc->InstrWORD(asBC_GETOBJREF, (asWORD)offset);
				}
				else
				{
					bc->InstrWORD(asBC_GETREF, (asWORD)offset);
				}
			}
		}
		else if( descr->parameterTypes[n].IsObject() )
		{
			// TODO: value on stack: What can we do to avoid this unnecessary allocation?
			// The object must be allocated on the heap, because this memory will be deleted in as_callfunc_xxx
			asASSERT(IsVariableOnHeap(args[n]->type.stackOffset));

			bc->InstrWORD(asBC_GETOBJ, (asWORD)offset);

			// The temporary variable must not be freed as it will no longer hold an object
			DeallocateVariable(args[n]->type.stackOffset);
			args[n]->type.isTemporary = false;
		}

		offset += descr->parameterTypes[n].GetSizeOnStackDWords();
	}
}

int asCCompiler::CompileArgumentList(asCScriptNode *node, asCArray<asSExprContext*> &args)
{
	asASSERT(node->nodeType == snArgList);

	// Count arguments
	asCScriptNode *arg = node->firstChild;
	int argCount = 0;
	while( arg )
	{
		argCount++;
		arg = arg->next;
	}

	// Prepare the arrays
	args.SetLength(argCount);
	int n;
	for( n = 0; n < argCount; n++ )
		args[n] = 0;

	n = argCount-1;

	// Compile the arguments in reverse order (as they will be pushed on the stack)
	bool anyErrors = false;
	arg = node->lastChild;
	while( arg )
	{
		asSExprContext expr(engine);
		int r = CompileAssignment(arg, &expr);
		if( r < 0 ) anyErrors = true;

		args[n] = asNEW(asSExprContext)(engine);
		if( args[n] == 0 )
		{
			// Out of memory
			return -1;
		}
		MergeExprBytecodeAndType(args[n], &expr);

		n--;
		arg = arg->prev;
	}

	return anyErrors ? -1 : 0;
}

int asCCompiler::CompileDefaultArgs(asCScriptNode *node, asCArray<asSExprContext*> &args, asCScriptFunction *func)
{
	bool anyErrors = false;
	asCArray<int> varsUsed;
	int explicitArgs = (int)args.GetLength();

	for( int p = 0; p < explicitArgs; p++ )
		args[p]->bc.GetVarsUsed(varsUsed);

	// Compile the arguments in reverse order (as they will be pushed on the stack)
	args.SetLength(func->parameterTypes.GetLength());
	for( asUINT c = explicitArgs; c < args.GetLength(); c++ )
		args[c] = 0;
	for( int n = (int)func->parameterTypes.GetLength() - 1; n >= explicitArgs; n-- )
	{
		if( func->defaultArgs[n] == 0 ) { anyErrors = true; continue; }

		// Parse the default arg string
		asCParser parser(builder);
		asCScriptCode code;
		code.SetCode("default arg", func->defaultArgs[n]->AddressOf(), false);
		int r = parser.ParseExpression(&code);
		if( r < 0 )
		{
			asCString msg;
			msg.Format(TXT_FAILED_TO_COMPILE_DEF_ARG_d_IN_FUNC_s, n, func->GetDeclaration());
			Error(msg, node);
			anyErrors = true;
			continue;
		}

		asCScriptNode *arg = parser.GetScriptNode();

		// Temporarily set the script code to the default arg expression
		asCScriptCode *origScript = script;
		script = &code;

		// Don't allow the expression to access local variables
		// TODO: namespace: The default arg should see the symbols declared in the same scope as the function that is called
		isCompilingDefaultArg = true;
		asSExprContext expr(engine);
		r = CompileExpression(arg, &expr);

		// Don't allow address of class method
		if( expr.methodName != "" )
		{
			// TODO: Improve error message
			Error(TXT_DEF_ARG_TYPE_DOESNT_MATCH, arg);
			r = -1;
		}

		// Make sure the expression can be implicitly converted to the parameter type
		if( r >= 0 )
		{
			asCArray<int> funcs;
			funcs.PushLast(func->id);
			asCArray<asSOverloadCandidate> matches;
			if( MatchArgument(funcs, matches, &expr, n) == 0 )
			{
				Error(TXT_DEF_ARG_TYPE_DOESNT_MATCH, arg);
				r = -1;
			}
		}

		isCompilingDefaultArg = false;

		script = origScript;

		if( r < 0 )
		{
			asCString msg;
			msg.Format(TXT_FAILED_TO_COMPILE_DEF_ARG_d_IN_FUNC_s, n, func->GetDeclaration());
			Error(msg, node);
			anyErrors = true;
			continue;
		}

		args[n] = asNEW(asSExprContext)(engine);
		if( args[n] == 0 )
		{
			// Out of memory
			return -1;
		}

		MergeExprBytecodeAndType(args[n], &expr);

		// Make sure the default arg expression doesn't end up
		// with a variable that is used in a previous expression
		if( args[n]->type.isVariable )
		{
			int offset = args[n]->type.stackOffset;
			if( varsUsed.Exists(offset) )
			{
				// Release the current temporary variable
				ReleaseTemporaryVariable(args[n]->type, 0);

				asCDataType dt = args[n]->type.dataType;
				dt.MakeReference(false);

				// Reserve all variables already used in the expression so none of them will be used
				asCArray<int> used;
				args[n]->bc.GetVarsUsed(used);
				size_t prevReserved = reservedVariables.GetLength();
				reservedVariables.Concatenate(used);

				int newOffset = AllocateVariable(dt, true, IsVariableOnHeap(offset));
				asASSERT( IsVariableOnHeap(offset) == IsVariableOnHeap(newOffset) );

				reservedVariables.SetLength(prevReserved);

				// Replace the variable in the expression
				args[n]->bc.ExchangeVar(offset, newOffset);
				args[n]->type.stackOffset = (short)newOffset;
				args[n]->type.isTemporary = true;
				args[n]->type.isVariable = true;
			}
		}
	}

	return anyErrors ? -1 : 0;
}

asUINT asCCompiler::MatchFunctions(asCArray<int> &funcs, asCArray<asSExprContext*> &args, asCScriptNode *node, const char *name, asCObjectType *objectType, bool isConstMethod, bool silent, bool allowObjectConstruct, const asCString &scope)
{
	asCArray<int> origFuncs = funcs; // Keep the original list for error message
	asUINT cost = 0;
	asUINT n;

	if( funcs.GetLength() > 0 )
	{
		// Check the number of parameters in the found functions
		for( n = 0; n < funcs.GetLength(); ++n )
		{
			asCScriptFunction *desc = builder->GetFunctionDescription(funcs[n]);

			if( desc->parameterTypes.GetLength() != args.GetLength() )
			{
				bool noMatch = true;
				if( args.GetLength() < desc->parameterTypes.GetLength() )
				{
					// For virtual functions, the default args are defined in the real function of the object
					if( desc->funcType == asFUNC_VIRTUAL )
						desc = objectType->virtualFunctionTable[desc->vfTableIdx];

					// Count the number of default args
					asUINT defaultArgs = 0;
					for( asUINT d = 0; d < desc->defaultArgs.GetLength(); d++ )
						if( desc->defaultArgs[d] )
							defaultArgs++;

					if( args.GetLength() >= desc->parameterTypes.GetLength() - defaultArgs )
						noMatch = false;
				}

				if( noMatch )
				{
					// remove it from the list
					if( n == funcs.GetLength()-1 )
						funcs.PopLast();
					else
						funcs[n] = funcs.PopLast();
					n--;
				}
			}
		}

		// Match functions with the parameters, and discard those that do not match
		asCArray<asSOverloadCandidate> matchingFuncs;
		matchingFuncs.SetLengthNoConstruct( funcs.GetLength() );
		for ( n = 0; n < funcs.GetLength(); ++n )
		{
			matchingFuncs[n].funcId = funcs[n];
			matchingFuncs[n].cost = 0;
		}

		for( n = 0; n < args.GetLength(); ++n )
		{
			asCArray<asSOverloadCandidate> tempFuncs;
			MatchArgument(funcs, tempFuncs, args[n], n, allowObjectConstruct);

			// Intersect the found functions with the list of matching functions
			for( asUINT f = 0; f < matchingFuncs.GetLength(); f++ )
			{
				asUINT c;
				for( c = 0; c < tempFuncs.GetLength(); c++ )
				{
					if( matchingFuncs[f].funcId == tempFuncs[c].funcId )
					{
						// Sum argument cost
						matchingFuncs[f].cost += tempFuncs[c].cost;
						break;

					} // End if match
				}

				// Was the function a match?
				if( c == tempFuncs.GetLength() )
				{
					// No, remove it from the list
					if( f == matchingFuncs.GetLength()-1 )
						matchingFuncs.PopLast();
					else
						matchingFuncs[f] = matchingFuncs.PopLast();
					f--;
				}
			}
		}

		// Select the overload(s) with the lowest overall cost
		funcs.SetLength(0);
		asUINT bestCost = asUINT(-1);
		for( n = 0; n < matchingFuncs.GetLength(); ++n )
		{
			cost = matchingFuncs[n].cost;
			if( cost < bestCost )
			{
				funcs.SetLength(0);
				bestCost = cost;
			}
			if( cost == bestCost )
				funcs.PushLast( matchingFuncs[n].funcId );
		}

		// Cost returned is equivalent to the best cost discovered
		cost = bestCost;
	}

	if( !isConstMethod )
		FilterConst(funcs);

	if( funcs.GetLength() != 1 && !silent )
	{
		// Build a readable string of the function with parameter types
		asCString str;
		if( scope != "" )
		{
			if( scope == "::" )
				str = scope;
			else
				str = scope + "::";
		}
		str += name;
		str += "(";
		if( args.GetLength() )
		{
			if( args[0]->methodName != "" )
				str += args[0]->methodName;
			else
				str += args[0]->type.dataType.Format();
		}
		for( n = 1; n < args.GetLength(); n++ )
		{
			str += ", ";
			if( args[n]->methodName != "" )
				str += args[n]->methodName;
			else
				str += args[n]->type.dataType.Format();
		}
		str += ")";

		if( isConstMethod )
			str += " const";

		if( objectType && scope == "" )
			str = objectType->name + "::" + str;

		if( funcs.GetLength() == 0 )
		{
			str.Format(TXT_NO_MATCHING_SIGNATURES_TO_s, str.AddressOf());
			Error(str, node);

			// Print the list of candidates
			if( origFuncs.GetLength() > 0 )
			{
				int r = 0, c = 0;
				asASSERT( node );
				if( node ) script->ConvertPosToRowCol(node->tokenPos, &r, &c);
				builder->WriteInfo(script->name.AddressOf(), TXT_CANDIDATES_ARE, r, c, false);
				PrintMatchingFuncs(origFuncs, node);
			}
		}
		else
		{
			str.Format(TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s, str.AddressOf());
			Error(str, node);

			PrintMatchingFuncs(funcs, node);
		}
	}

	return cost;
}

void asCCompiler::CompileDeclaration(asCScriptNode *decl, asCByteCode *bc)
{
	// Get the data type
	asCDataType type = builder->CreateDataTypeFromNode(decl->firstChild, script, outFunc->nameSpace);

	// Declare all variables in this declaration
	asCScriptNode *node = decl->firstChild->next;
	while( node )
	{
		// Is the type allowed?
		if( !type.CanBeInstanciated() )
		{
			asCString str;
			// TODO: Change to "'type' cannot be declared as variable"
			str.Format(TXT_DATA_TYPE_CANT_BE_s, type.Format().AddressOf());
			Error(str, node);

			// Use int instead to avoid further problems
			type = asCDataType::CreatePrimitive(ttInt, false);
		}

		// A shared object may not declare variables of non-shared types
		if( outFunc->IsShared() )
		{
			asCObjectType *ot = type.GetObjectType();
			if( ot && !ot->IsShared() )
			{
				asCString msg;
				msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, ot->name.AddressOf());
				Error(msg, decl);
			}
		}

		// Get the name of the identifier
		asCString name(&script->code[node->tokenPos], node->tokenLength);

		// Verify that the name isn't used by a dynamic data type
		// TODO: Must check against registered funcdefs too
		if( engine->GetRegisteredObjectType(name.AddressOf(), outFunc->nameSpace) != 0 )
		{
			asCString str;
			str.Format(TXT_ILLEGAL_VARIABLE_NAME_s, name.AddressOf());
			Error(str, node);
		}

		int offset = AllocateVariable(type, false);
		if( variables->DeclareVariable(name.AddressOf(), type, offset, IsVariableOnHeap(offset)) < 0 )
		{
			// TODO: It might be an out-of-memory too

			asCString str;
			str.Format(TXT_s_ALREADY_DECLARED, name.AddressOf());
			Error(str, node);

			// Don't continue after this error, as it will just
			// lead to more errors that are likely false
			return;
		}

		// Add marker that the variable has been declared
		bc->VarDecl((int)outFunc->scriptData->variables.GetLength());
		outFunc->AddVariable(name, type, offset);

		// Keep the node for the variable decl
		asCScriptNode *varNode = node;

		node = node->next;

		if( node == 0 || node->nodeType == snIdentifier )
		{
			// Initialize with default constructor
			CompileInitialization(0, bc, type, varNode, offset, 0, 0);
		}
		else
		{
			// Compile the initialization expression
			asQWORD constantValue = 0;
			if( CompileInitialization(node, bc, type, varNode, offset, &constantValue, 0) )
			{
				// Check if the variable should be marked as pure constant
				if( type.IsPrimitive() && type.IsReadOnly() )
				{
					sVariable *v = variables->GetVariable(name.AddressOf());
					v->isPureConstant = true;
					v->constantValue = constantValue;
				}
			}
			node = node->next;
		}
	}

	bc->OptimizeLocally(tempVariableOffsets);
}

bool asCCompiler::CompileInitialization(asCScriptNode *node, asCByteCode *bc, asCDataType &type, asCScriptNode *errNode, int offset, asQWORD *constantValue, int isVarGlobOrMem)
{
	bool isConstantExpression = false;
	if( node && node->nodeType == snArgList )
	{
		// Make sure it is an object and not a handle
		if( type.GetObjectType() == 0 || type.IsObjectHandle() )
		{
			Error(TXT_MUST_BE_OBJECT, node);
		}
		else
		{
			// Compile the arguments
			asCArray<asSExprContext *> args;
			if( CompileArgumentList(node, args) >= 0 )
			{
				// Find all constructors
				asCArray<int> funcs;
				asSTypeBehaviour *beh = type.GetBehaviour();
				if( beh )
				{
					if( type.GetObjectType()->flags & asOBJ_REF )
						funcs = beh->factories;
					else
						funcs = beh->constructors;
				}

				asCString str = type.Format();
				MatchFunctions(funcs, args, node, str.AddressOf());

				if( funcs.GetLength() == 1 )
				{
					int r = asSUCCESS;

					// Add the default values for arguments not explicitly supplied
					asCScriptFunction *func = (funcs[0] & FUNC_IMPORTED) == 0 ? engine->scriptFunctions[funcs[0]] : 0;
					if( func && args.GetLength() < (asUINT)func->GetParamCount() )
						r = CompileDefaultArgs(node, args, func);

					if( r == asSUCCESS )
					{
						asSExprContext ctx(engine);
						if( type.GetObjectType() && (type.GetObjectType()->flags & asOBJ_REF) )
						{
							if( isVarGlobOrMem == 0 )
								MakeFunctionCall(&ctx, funcs[0], 0, args, node, true, offset);
							else
							{
								MakeFunctionCall(&ctx, funcs[0], 0, args, node);
								ctx.bc.Instr(asBC_RDSPtr);
								if( isVarGlobOrMem == 1 )
								{
									// Store the returned handle in the global variable
									ctx.bc.InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
								}
								else
								{
									// Store the returned handle in the member
									ctx.bc.InstrSHORT(asBC_PSF, 0);
									ctx.bc.Instr(asBC_RDSPtr);
									ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
								}
								ctx.bc.InstrPTR(asBC_REFCPY, type.GetObjectType());
								ReleaseTemporaryVariable(ctx.type.stackOffset, &ctx.bc);
							}

							// Pop the reference left by the function call
							ctx.bc.Instr(asBC_PopPtr);
						}
						else
						{
							bool onHeap = false;

							if( isVarGlobOrMem == 0 )
							{
								// When the object is allocated on the heap, the address where the
								// reference will be stored must be pushed on the stack before the
								// arguments. This reference on the stack is safe, even if the script
								// is suspended during the evaluation of the arguments.
								onHeap = IsVariableOnHeap(offset);
								if( onHeap )
									ctx.bc.InstrSHORT(asBC_PSF, (short)offset);
							}
							else if( isVarGlobOrMem == 1 )
							{
								// Push the address of the location where the variable will be stored on the stack.
								// This reference is safe, because the addresses of the global variables cannot change.
								onHeap = true;
								ctx.bc.InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
							}
							else
							{
								// Value types may be allocated inline if they are POD types
								onHeap = !type.IsObject() || type.IsReference() || (type.GetObjectType()->flags & asOBJ_REF);
								if( onHeap )
								{
									ctx.bc.InstrSHORT(asBC_PSF, 0);
									ctx.bc.Instr(asBC_RDSPtr);
									ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
								}
							}

							PrepareFunctionCall(funcs[0], &ctx.bc, args);
							MoveArgsToStack(funcs[0], &ctx.bc, args, false);

							// When the object is allocated on the stack, the address to the
							// object is pushed on the stack after the arguments as the object pointer
							if( !onHeap )
							{
								if( isVarGlobOrMem == 2 )
								{
									ctx.bc.InstrSHORT(asBC_PSF, 0);
									ctx.bc.Instr(asBC_RDSPtr);
									ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
								}
								else
								{
									ctx.bc.InstrSHORT(asBC_PSF, (short)offset);
								}
							}

							PerformFunctionCall(funcs[0], &ctx, onHeap, &args, type.GetObjectType());

							if( isVarGlobOrMem == 0 )
							{
								// Mark the object in the local variable as initialized
								ctx.bc.ObjInfo(offset, asOBJ_INIT);
							}
						}
						bc->AddCode(&ctx.bc);
					}
				}
			}

			// Cleanup
			for( asUINT n = 0; n < args.GetLength(); n++ )
				if( args[n] )
				{
					asDELETE(args[n],asSExprContext);
				}
		}
	}
	else if( node && node->nodeType == snInitList )
	{
		asCTypeInfo ti;
		ti.Set(type);
		ti.isVariable = (isVarGlobOrMem == 0);
		ti.isTemporary = false;
		ti.stackOffset = (short)offset;
		ti.isLValue = true;

		CompileInitList(&ti, node, bc, isVarGlobOrMem);
	}
	else if( node && node->nodeType == snAssignment )
	{
		asSExprContext ctx(engine);

		// TODO: copy: Here we should look for the best matching constructor, instead of
		//             just the copy constructor. Only if no appropriate constructor is
		//             available should the assignment operator be used.

		// Call the default constructor here
		if( isVarGlobOrMem == 0 )
			CallDefaultConstructor(type, offset, IsVariableOnHeap(offset), &ctx.bc, errNode);
		else if( isVarGlobOrMem == 1 )
			CallDefaultConstructor(type, offset, true, &ctx.bc, errNode, isVarGlobOrMem);
		else if( isVarGlobOrMem == 2 )
			CallDefaultConstructor(type, offset, type.IsReference(), &ctx.bc, errNode, isVarGlobOrMem);

		// Compile the expression
		asSExprContext expr(engine);
		int r = CompileAssignment(node, &expr);
		if( r >= 0 )
		{
			if( type.IsPrimitive() )
			{
				if( type.IsReadOnly() && expr.type.isConstant )
				{
					ImplicitConversion(&expr, type, node, asIC_IMPLICIT_CONV);

					// Tell caller that the expression is a constant so it can mark the variable as pure constant
					isConstantExpression = true;
					*constantValue = expr.type.qwordValue;
				}

				asSExprContext lctx(engine);
				if( isVarGlobOrMem == 0 )
					lctx.type.SetVariable(type, offset, false);
				else if( isVarGlobOrMem == 1 )
				{
					lctx.type.Set(type);
					lctx.type.dataType.MakeReference(true);

					// If it is an enum value, i.e. offset is negative, that is being compiled then
					// we skip this as the bytecode won't be used anyway, only the constant value
					if( offset >= 0 )
						lctx.bc.InstrPTR(asBC_LDG, engine->globalProperties[offset]->GetAddressOfValue());
				}
				else
				{
					asASSERT( isVarGlobOrMem == 2 );
					lctx.type.Set(type);
					lctx.type.dataType.MakeReference(true);

					// Load the reference of the primitive member into the register
					lctx.bc.InstrSHORT(asBC_PSF, 0);
					lctx.bc.Instr(asBC_RDSPtr);
					lctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
					lctx.bc.Instr(asBC_PopRPtr);
				}
				lctx.type.dataType.MakeReadOnly(false);
				lctx.type.isLValue = true;

				DoAssignment(&ctx, &lctx, &expr, node, node, ttAssignment, node);
				ProcessDeferredParams(&ctx);
			}
			else
			{
				// TODO: runtime optimize: Here we should look for the best matching constructor, instead of
				//                         just the copy constructor. Only if no appropriate constructor is
				//                         available should the assignment operator be used.

				asSExprContext lexpr(engine);
				lexpr.type.Set(type);
				if( isVarGlobOrMem == 0 )
					lexpr.type.dataType.MakeReference(IsVariableOnHeap(offset));
				else if( isVarGlobOrMem == 1 )
					lexpr.type.dataType.MakeReference(true);
				else if( isVarGlobOrMem == 2 )
				{
					if( !lexpr.type.dataType.IsObject() || (lexpr.type.dataType.GetObjectType()->flags & asOBJ_REF) )
						lexpr.type.dataType.MakeReference(true);
				}

				// Allow initialization of constant variables
				lexpr.type.dataType.MakeReadOnly(false);

				if( type.IsObjectHandle() )
					lexpr.type.isExplicitHandle = true;

				if( isVarGlobOrMem == 0 )
				{
					lexpr.bc.InstrSHORT(asBC_PSF, (short)offset);
					lexpr.type.stackOffset = (short)offset;
					lexpr.type.isVariable = true;
				}
				else if( isVarGlobOrMem == 1 )
				{
					lexpr.bc.InstrPTR(asBC_PGA, engine->globalProperties[offset]->GetAddressOfValue());
				}
				else
				{
					lexpr.bc.InstrSHORT(asBC_PSF, 0);
					lexpr.bc.Instr(asBC_RDSPtr);
					lexpr.bc.InstrSHORT_DW(asBC_ADDSi, (short)offset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
					lexpr.type.stackOffset = -1;
				}
				lexpr.type.isLValue = true;


				// If left expression resolves into a registered type
				// check if the assignment operator is overloaded, and check
				// the type of the right hand expression. If none is found
				// the default action is a direct copy if it is the same type
				// and a simple assignment.
				bool assigned = false;
				// Even though an ASHANDLE can be an explicit handle the overloaded operator needs to be called
				if( lexpr.type.dataType.IsObject() && (!lexpr.type.isExplicitHandle || (lexpr.type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE)) )
				{
					assigned = CompileOverloadedDualOperator(node, &lexpr, &expr, &ctx);
					if( assigned )
					{
						// Pop the resulting value
						if( !ctx.type.dataType.IsPrimitive() )
							ctx.bc.Instr(asBC_PopPtr);

						// Release the argument
						ProcessDeferredParams(&ctx);

						// Release temporary variable that may be allocated by the overloaded operator
						ReleaseTemporaryVariable(ctx.type, &ctx.bc);
					}
				}

				if( !assigned )
				{
					PrepareForAssignment(&lexpr.type.dataType, &expr, node, false);

					// If the expression is constant and the variable also is constant
					// then mark the variable as pure constant. This will allow the compiler
					// to optimize expressions with this variable.
					if( type.IsReadOnly() && expr.type.isConstant )
					{
						isConstantExpression = true;
						*constantValue = expr.type.qwordValue;
					}

					// Add expression code to bytecode
					MergeExprBytecode(&ctx, &expr);

					// Add byte code for storing value of expression in variable
					ctx.bc.AddCode(&lexpr.bc);

					PerformAssignment(&lexpr.type, &expr.type, &ctx.bc, errNode);

					// Release temporary variables used by expression
					ReleaseTemporaryVariable(expr.type, &ctx.bc);

					ctx.bc.Instr(asBC_PopPtr);

					ProcessDeferredParams(&ctx);
				}
			}
		}

		bc->AddCode(&ctx.bc);
	}
	else
	{
		asASSERT( node == 0 );

		// Call the default constructor here, as no explicit initialization is done
		if( isVarGlobOrMem == 0 )
			CallDefaultConstructor(type, offset, IsVariableOnHeap(offset), bc, errNode);
		else if( isVarGlobOrMem == 1 )
			CallDefaultConstructor(type, offset, true, bc, errNode, isVarGlobOrMem);
		else if( isVarGlobOrMem == 2 )
		{
			if( !type.IsObject() || type.IsReference() || (type.GetObjectType()->flags & asOBJ_REF) )
				CallDefaultConstructor(type, offset, true, bc, errNode, isVarGlobOrMem);
			else
				CallDefaultConstructor(type, offset, false, bc, errNode, isVarGlobOrMem);
		}
	}

	bc->OptimizeLocally(tempVariableOffsets);

	return isConstantExpression;
}

void asCCompiler::CompileInitList(asCTypeInfo *var, asCScriptNode *node, asCByteCode *bc, int isVarGlobOrMem)
{
	// Check if the type supports initialization lists
	if( var->dataType.GetObjectType() == 0 ||
		var->dataType.GetBehaviour()->listFactory == 0 ||
		var->dataType.IsObjectHandle() )
	{
		asCString str;
		str.Format(TXT_INIT_LIST_CANNOT_BE_USED_WITH_s, var->dataType.Format().AddressOf());
		Error(str, node);
		return;
	}

	// Construct the buffer with the elements

	// Find the list factory
	int funcId = var->dataType.GetBehaviour()->listFactory;
	asASSERT( engine->scriptFunctions[funcId]->listPattern );

	// TODO: runtime optimize: A future optimization should be to use the stack space directly
	//                         for small buffers so that the dynamic allocation is skipped

	// Create a new special object type for the lists. Both asCRestore and the 
	// context exception handler will need this to know how to parse the buffer.
	asCObjectType *listPatternType = engine->GetListPatternType(funcId);

	// Allocate a temporary variable to hold the pointer to the buffer
	int bufferVar = AllocateVariable(asCDataType::CreateObject(listPatternType, false), true);
	asUINT bufferSize = 0;

	// Evaluate all elements of the list
	asSExprContext valueExpr(engine);
	asCScriptNode *el = node;
	asSListPatternNode *patternNode = engine->scriptFunctions[listPatternType->templateSubTypes[0].GetBehaviour()->listFactory]->listPattern;
	int r = CompileInitListElement(patternNode, el, engine->GetTypeIdFromDataType(asCDataType::CreateObject(listPatternType, false)), bufferVar, bufferSize, valueExpr.bc);
	asASSERT( r || patternNode == 0 );
	UNUSED_VAR(r);

	// After all values have been evaluated we know the final size of the buffer
	asSExprContext allocExpr(engine);
	allocExpr.bc.InstrSHORT_DW(asBC_AllocMem, bufferVar, bufferSize);

	// Merge the bytecode into the final sequence
	bc->AddCode(&allocExpr.bc);
	bc->AddCode(&valueExpr.bc);

	// The object itself is the last to be created and will receive the pointer to the buffer
	asCArray<asSExprContext *> args;
	asSExprContext arg1(engine);
	arg1.type.Set(asCDataType::CreatePrimitive(ttUInt, false));
	arg1.type.dataType.MakeReference(true);
	arg1.bc.InstrSHORT(asBC_PshVPtr, bufferVar);
	args.PushLast(&arg1);

	asSExprContext ctx(engine);

	if( var->isVariable )
	{
		asASSERT( isVarGlobOrMem == 0 );

		if( var->dataType.GetObjectType()->GetFlags() & asOBJ_REF )
		{
			ctx.bc.AddCode(&arg1.bc);

			// Call factory and store the handle in the given variable
			PerformFunctionCall(funcId, &ctx, false, &args, 0, true, var->stackOffset);
			ctx.bc.Instr(asBC_PopPtr);
		}
		else
		{
			// Call the constructor

			// When the object is allocated on the heap, the address where the
			// reference will be stored must be pushed on the stack before the
			// arguments. This reference on the stack is safe, even if the script
			// is suspended during the evaluation of the arguments.
			bool onHeap = IsVariableOnHeap(var->stackOffset);
			if( onHeap )
				ctx.bc.InstrSHORT(asBC_PSF, var->stackOffset);

			ctx.bc.AddCode(&arg1.bc);

			// When the object is allocated on the stack, the address to the
			// object is pushed on the stack after the arguments as the object pointer
			if( !onHeap )
				ctx.bc.InstrSHORT(asBC_PSF, var->stackOffset);

			PerformFunctionCall(funcId, &ctx, onHeap, &args, var->dataType.GetObjectType());

			// Mark the object in the local variable as initialized
			ctx.bc.ObjInfo(var->stackOffset, asOBJ_INIT);
		}
	}
	else
	{
		if( var->dataType.GetObjectType()->GetFlags() & asOBJ_REF )
		{
			ctx.bc.AddCode(&arg1.bc);

			PerformFunctionCall(funcId, &ctx, false, &args);

			ctx.bc.Instr(asBC_RDSPtr);
			if( isVarGlobOrMem == 1 )
			{
				// Store the returned handle in the global variable
				ctx.bc.InstrPTR(asBC_PGA, engine->globalProperties[var->stackOffset]->GetAddressOfValue());
			}
			else
			{
				// Store the returned handle in the member
				ctx.bc.InstrSHORT(asBC_PSF, 0);
				ctx.bc.Instr(asBC_RDSPtr);
				ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)var->stackOffset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
			}
			ctx.bc.InstrPTR(asBC_REFCPY, var->dataType.GetObjectType());
			ctx.bc.Instr(asBC_PopPtr);
			ReleaseTemporaryVariable(ctx.type.stackOffset, &ctx.bc);
		}
		else
		{
			bool onHeap = true;

			// Put the address where the object pointer will be placed on the stack
			if( isVarGlobOrMem == 1 )
				ctx.bc.InstrPTR(asBC_PGA, engine->globalProperties[var->stackOffset]->GetAddressOfValue());
			else
			{
				onHeap = !var->dataType.IsObject() || var->dataType.IsReference() || (var->dataType.GetObjectType()->flags & asOBJ_REF);
				if( onHeap )
				{
					ctx.bc.InstrSHORT(asBC_PSF, 0);
					ctx.bc.Instr(asBC_RDSPtr);
					ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)var->stackOffset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
				}
			}

			// Add the address of the list buffer as the argument
			ctx.bc.AddCode(&arg1.bc);

			if( !onHeap )
			{
				ctx.bc.InstrSHORT(asBC_PSF, 0);
				ctx.bc.Instr(asBC_RDSPtr);
				ctx.bc.InstrSHORT_DW(asBC_ADDSi, (short)var->stackOffset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(outFunc->objectType, false)));
			}

			// Call the ALLOC instruction to allocate memory and invoke constructor
			PerformFunctionCall(funcId, &ctx, onHeap, &args, var->dataType.GetObjectType());
		}
	}

	bc->AddCode(&ctx.bc);

	// Free the temporary buffer. The FREE instruction will make sure to destroy
	// each element in the buffer so there is no need to do this manually
	bc->InstrW_PTR(asBC_FREE, bufferVar, listPatternType);
	ReleaseTemporaryVariable(bufferVar, bc);
}

int asCCompiler::CompileInitListElement(asSListPatternNode *&patternNode, asCScriptNode *&valueNode, int bufferTypeId, short bufferVar, asUINT &bufferSize, asCByteCode &byteCode)
{
	if( patternNode->type == asLPT_START )
	{
		if( valueNode->nodeType != snInitList )
		{
			Error(TXT_EXPECTED_LIST, valueNode);
			return -1;
		}

		// Compile all values until asLPT_END
		patternNode = patternNode->next;
		asCScriptNode *node = valueNode->firstChild;
		while( patternNode->type != asLPT_END )
		{
			if( node == 0 )
			{
				Error(TXT_NOT_ENOUGH_VALUES_FOR_LIST, valueNode);
				return -1;
			}

			int r = CompileInitListElement(patternNode, node, bufferTypeId, bufferVar, bufferSize, byteCode);
			if( r < 0 ) return r;
			asASSERT( patternNode );
		}

		if( node )
		{
			Error(TXT_TOO_MANY_VALUES_FOR_LIST, valueNode);
			return -1;
		}
		
		// Move to the next node
		valueNode = valueNode->next;
		patternNode = patternNode->next;
	}
	else if( patternNode->type == asLPT_REPEAT )
	{
		// The following values will be repeated N times
		patternNode = patternNode->next;

		// Keep track of the patternNode so it can be reset
		asSListPatternNode *nextNode = patternNode;

		// Align the buffer size to 4 bytes in case previous value was smaller than 4 bytes
		if( bufferSize & 0x3 )
			bufferSize += 4 - (bufferSize & 0x3);

		// The first dword will hold the number of elements in the list
		asDWORD currSize = bufferSize;
		bufferSize += 4;
		asUINT countElements = 0;

		asSExprContext ctx(engine);
		while( valueNode )
		{
			patternNode = nextNode;
			int r = CompileInitListElement(patternNode, valueNode, bufferTypeId, bufferVar, bufferSize, ctx.bc);
			if( r < 0 ) return r;

			countElements++;
		}

		// The first dword in the buffer will hold the number of elements
		byteCode.InstrSHORT_DW_DW(asBC_SetListSize, bufferVar, currSize, countElements);

		// Add the values
		byteCode.AddCode(&ctx.bc);
	}
	else if( patternNode->type == asLPT_TYPE )
	{
		// Determine the size of the element
		asUINT size = 0;

		asCDataType dt = reinterpret_cast<asSListPatternDataTypeNode*>(patternNode)->dataType;

		if( valueNode->nodeType == snAssignment || valueNode->nodeType == snInitList )
		{
			asSExprContext lctx(engine);
			asSExprContext rctx(engine);

			if( valueNode->nodeType == snAssignment )
			{
				// Compile the assignment expression
				CompileAssignment(valueNode, &rctx);

				if( dt.GetTokenType() == ttQuestion )
				{
					// We now know the type
					dt = rctx.type.dataType;
					dt.MakeReadOnly(false);
					dt.MakeReference(false);

					// Values on the list must be aligned to 32bit boundaries, except if the type is smaller than 32bit.
					if( bufferSize & 0x3 )
						bufferSize += 4 - (bufferSize & 0x3);

					// Place the type id in the buffer
					byteCode.InstrSHORT_DW_DW(asBC_SetListType, bufferVar, bufferSize, engine->GetTypeIdFromDataType(dt));
					bufferSize += 4;
				}
			}
			else if( valueNode->nodeType == snInitList )
			{
				if( dt.GetTokenType() == ttQuestion )
				{
					// Can't use init lists with var type as it is not possible to determine what type should be allocated
					asCString str; 
					str.Format(TXT_INIT_LIST_CANNOT_BE_USED_WITH_s, "?");
					Error(str.AddressOf(), valueNode);
					rctx.type.SetDummy();
					dt = rctx.type.dataType;
				}
				else
				{
					// Allocate a temporary variable that will be initialized with the list
					int offset = AllocateVariable(dt, true);

					rctx.type.Set(dt);
					rctx.type.isVariable = true;
					rctx.type.isTemporary = true;
					rctx.type.stackOffset = (short)offset;

					CompileInitList(&rctx.type, valueNode, &rctx.bc, 0);

					// Put the object on the stack
					rctx.bc.InstrSHORT(asBC_PSF, rctx.type.stackOffset);

					// It is a reference that we place on the stack
					rctx.type.dataType.MakeReference(true);
				}
			}

			// Determine size of the element
			if( dt.IsPrimitive() || (!dt.IsNullHandle() && (dt.GetObjectType()->flags & asOBJ_VALUE)) )
				size = dt.GetSizeInMemoryBytes();
			else
				size = AS_PTR_SIZE*4;

			// Values on the list must be aligned to 32bit boundaries, except if the type is smaller than 32bit.
			if( size >= 4 && (bufferSize & 0x3) )
				bufferSize += 4 - (bufferSize & 0x3);

			// Compile the lvalue
			lctx.bc.InstrSHORT_DW(asBC_PshListElmnt, bufferVar, bufferSize);
			lctx.type.Set(dt);
			lctx.type.isLValue = true;
			if( dt.IsPrimitive() )
			{
				lctx.bc.Instr(asBC_PopRPtr);
				lctx.type.dataType.MakeReference(true);
			}
			else if( dt.IsObjectHandle() ||
					 dt.GetObjectType()->flags & asOBJ_REF )
			{
				lctx.type.isExplicitHandle = true;
				lctx.type.dataType.MakeReference(true);
			}
			else
			{
				asASSERT( dt.GetObjectType()->flags & asOBJ_VALUE );

				// Make sure the object has been constructed before the assignment
				// TODO: runtime optimize: Use copy constructor instead of assignment to initialize the objects
				asSTypeBehaviour *beh = dt.GetBehaviour();
				int func = 0;
				if( beh ) func = beh->construct;
				if( func == 0 && (dt.GetObjectType()->flags & asOBJ_POD) == 0 )
				{
					asCString str;
					// TODO: funcdef: asCDataType should have a GetTypeName()
					if( dt.GetFuncDef() )
						str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, dt.GetFuncDef()->GetName());
					else
						str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, dt.GetObjectType()->GetName());
					Error(str, valueNode);
				}
				else if( func )
				{
					// Call the constructor as a normal function
					byteCode.InstrSHORT_DW(asBC_PshListElmnt, bufferVar, bufferSize);

					asSExprContext ctx(engine);
					PerformFunctionCall(func, &ctx, false, 0, dt.GetObjectType());
					byteCode.AddCode(&ctx.bc);
				}
			}

			asSExprContext ctx(engine);
			DoAssignment(&ctx, &lctx, &rctx, valueNode, valueNode, ttAssignment, valueNode);

			if( !lctx.type.dataType.IsPrimitive() )
				ctx.bc.Instr(asBC_PopPtr);

			// Release temporary variables used by expression
			ReleaseTemporaryVariable(ctx.type, &ctx.bc);

			ProcessDeferredParams(&ctx);

			byteCode.AddCode(&ctx.bc);
		}
		else
		{
			// There is no specific value so we need to fill it with a default value
			if( dt.GetTokenType() == ttQuestion )
			{
				// Values on the list must be aligned to 32bit boundaries, except if the type is smaller than 32bit.
				if( bufferSize & 0x3 )
					bufferSize += 4 - (bufferSize & 0x3);

				// Place the type id for a null handle in the buffer
				byteCode.InstrSHORT_DW_DW(asBC_SetListType, bufferVar, bufferSize, 0);
				bufferSize += 4;

				dt = asCDataType::CreateNullHandle();

				// No need to initialize the handle as the buffer is already initialized with zeroes
			}
			else if( dt.GetObjectType() && dt.GetObjectType()->flags & asOBJ_VALUE )
			{
				// For value types with default constructor we need to call the constructor
				asSTypeBehaviour *beh = dt.GetBehaviour();
				int func = 0;
				if( beh ) func = beh->construct;
				if( func == 0 && (dt.GetObjectType()->flags & asOBJ_POD) == 0 )
				{
					asCString str;
					// TODO: funcdef: asCDataType should have a GetTypeName()
					if( dt.GetFuncDef() )
						str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, dt.GetFuncDef()->GetName());
					else
						str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, dt.GetObjectType()->GetName());
					Error(str, valueNode);
				}
				else if( func )
				{
					// Values on the list must be aligned to 32bit boundaries, except if the type is smaller than 32bit.
					if( bufferSize & 0x3 )
						bufferSize += 4 - (bufferSize & 0x3);

					// Call the constructor as a normal function
					byteCode.InstrSHORT_DW(asBC_PshListElmnt, bufferVar, bufferSize);

					asSExprContext ctx(engine);
					PerformFunctionCall(func, &ctx, false, 0, dt.GetObjectType());
					byteCode.AddCode(&ctx.bc);
				}
			}
			else if( !dt.IsObjectHandle() && dt.GetObjectType() && dt.GetObjectType()->flags & asOBJ_REF )
			{
				// For ref types (not handles) we need to call the default factory
				asSTypeBehaviour *beh = dt.GetBehaviour();
				int func = 0;
				if( beh ) func = beh->factory;
				if( func == 0 )
				{
					asCString str;
					// TODO: funcdef: asCDataType should have a GetTypeName()
					if( dt.GetFuncDef() )
						str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, dt.GetFuncDef()->GetName());
					else
						str.Format(TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s, dt.GetObjectType()->GetName());
					Error(str, valueNode);
				}
				else if( func )
				{
					asSExprContext rctx(engine);
					PerformFunctionCall(func, &rctx, false, 0, dt.GetObjectType());

					// Values on the list must be aligned to 32bit boundaries, except if the type is smaller than 32bit.
					if( bufferSize & 0x3 )
						bufferSize += 4 - (bufferSize & 0x3);

					asSExprContext lctx(engine);
					lctx.bc.InstrSHORT_DW(asBC_PshListElmnt, bufferVar, bufferSize);
					lctx.type.Set(dt);
					lctx.type.isLValue = true;
					lctx.type.isExplicitHandle = true;
					lctx.type.dataType.MakeReference(true);

					asSExprContext ctx(engine);
					DoAssignment(&ctx, &lctx, &rctx, valueNode, valueNode, ttAssignment, valueNode);

					if( !lctx.type.dataType.IsPrimitive() )
						ctx.bc.Instr(asBC_PopPtr);

					// Release temporary variables used by expression
					ReleaseTemporaryVariable(ctx.type, &ctx.bc);

					ProcessDeferredParams(&ctx);

					byteCode.AddCode(&ctx.bc);
				}
			}
		}

		// Determine size of the element
		if( dt.IsPrimitive() || (!dt.IsNullHandle() && (dt.GetObjectType()->flags & asOBJ_VALUE)) )
			size = dt.GetSizeInMemoryBytes();
		else
			size = AS_PTR_SIZE*4;
		asASSERT( size <= 4 || (size & 0x3) == 0 );
		
		// Move to the next element
		bufferSize += size;
		patternNode = patternNode->next;
		valueNode = valueNode->next;
	}
	else
		asASSERT( false );

	return 0;
}

void asCCompiler::CompileStatement(asCScriptNode *statement, bool *hasReturn, asCByteCode *bc)
{
	// Don't clear the hasReturn flag if this is an empty statement
	// to avoid false errors of 'not all paths return'
	if( statement->nodeType != snExpressionStatement || statement->firstChild )
		*hasReturn = false;

	if( statement->nodeType == snStatementBlock )
		CompileStatementBlock(statement, true, hasReturn, bc);
	else if( statement->nodeType == snIf )
		CompileIfStatement(statement, hasReturn, bc);
	else if( statement->nodeType == snFor )
		CompileForStatement(statement, bc);
	else if( statement->nodeType == snWhile )
		CompileWhileStatement(statement, bc);
	else if( statement->nodeType == snDoWhile )
		CompileDoWhileStatement(statement, bc);
	else if( statement->nodeType == snExpressionStatement )
		CompileExpressionStatement(statement, bc);
	else if( statement->nodeType == snBreak )
		CompileBreakStatement(statement, bc);
	else if( statement->nodeType == snContinue )
		CompileContinueStatement(statement, bc);
	else if( statement->nodeType == snSwitch )
		CompileSwitchStatement(statement, hasReturn, bc);
	else if( statement->nodeType == snReturn )
	{
		CompileReturnStatement(statement, bc);
		*hasReturn = true;
	}
}

void asCCompiler::CompileSwitchStatement(asCScriptNode *snode, bool *, asCByteCode *bc)
{
	// TODO: inheritance: Must guarantee that all options in the switch case call a constructor, or that none call it.

	// Reserve label for break statements
	int breakLabel = nextLabel++;
	breakLabels.PushLast(breakLabel);

	// Add a variable scope that will be used by CompileBreak
	// to know where to stop deallocating variables
	AddVariableScope(true, false);

	//---------------------------
	// Compile the switch expression
	//-------------------------------

	// Compile the switch expression
	asSExprContext expr(engine);
	CompileAssignment(snode->firstChild, &expr);

	// Verify that the expression is a primitive type
	if( !expr.type.dataType.IsIntegerType() && !expr.type.dataType.IsUnsignedType() )
	{
		Error(TXT_SWITCH_MUST_BE_INTEGRAL, snode->firstChild);
		return;
	}

	ProcessPropertyGetAccessor(&expr, snode);

	// TODO: Need to support 64bit integers
	// Convert the expression to a 32bit variable
	asCDataType to;
	if( expr.type.dataType.IsIntegerType() )
		to.SetTokenType(ttInt);
	else if( expr.type.dataType.IsUnsignedType() )
		to.SetTokenType(ttUInt);

	// Make sure the value is in a variable
	if( expr.type.dataType.IsReference() )
		ConvertToVariable(&expr);

	ImplicitConversion(&expr, to, snode->firstChild, asIC_IMPLICIT_CONV, true);

	ConvertToVariable(&expr);
	int offset = expr.type.stackOffset;

	ProcessDeferredParams(&expr);

	//-------------------------------
	// Determine case values and labels
	//--------------------------------

	// Remember the first label so that we can later pass the
	// correct label to each CompileCase()
	int firstCaseLabel = nextLabel;
	int defaultLabel = 0;

	asCArray<int> caseValues;
	asCArray<int> caseLabels;

	// Compile all case comparisons and make them jump to the right label
	asCScriptNode *cnode = snode->firstChild->next;
	while( cnode )
	{
		// Each case should have a constant expression
		if( cnode->firstChild && cnode->firstChild->nodeType == snExpression )
		{
			// Compile expression
			asSExprContext c(engine);
			CompileExpression(cnode->firstChild, &c);

			// Verify that the result is a constant
			if( !c.type.isConstant )
				Error(TXT_SWITCH_CASE_MUST_BE_CONSTANT, cnode->firstChild);

			// Verify that the result is an integral number
			if( !c.type.dataType.IsIntegerType() && !c.type.dataType.IsUnsignedType() )
				Error(TXT_SWITCH_MUST_BE_INTEGRAL, cnode->firstChild);

			ImplicitConversion(&c, to, cnode->firstChild, asIC_IMPLICIT_CONV, true);

			// Has this case been declared already?
			if( caseValues.IndexOf(c.type.intValue) >= 0 )
			{
				Error(TXT_DUPLICATE_SWITCH_CASE, cnode->firstChild);
			}

			// TODO: Optimize: We can insert the numbers sorted already

			// Store constant for later use
			caseValues.PushLast(c.type.intValue);

			// Reserve label for this case
			caseLabels.PushLast(nextLabel++);
		}
		else
		{
			// TODO: It shouldn't be necessary for the default case to be the last one.
			// Is default the last case?
			if( cnode->next )
			{
				Error(TXT_DEFAULT_MUST_BE_LAST, cnode);
				break;
			}

			// Reserve label for this case
			defaultLabel = nextLabel++;
		}

		cnode = cnode->next;
	}

    // check for empty switch
	if (caseValues.GetLength() == 0)
	{
		Error(TXT_EMPTY_SWITCH, snode);
		return;
	}

	if( defaultLabel == 0 )
		defaultLabel = breakLabel;

	//---------------------------------
    // Output the optimized case comparisons
	// with jumps to the case code
	//------------------------------------

	// Sort the case values by increasing value. Do the sort together with the labels
	// A simple bubble sort is sufficient since we don't expect a huge number of values
	for( asUINT fwd = 1; fwd < caseValues.GetLength(); fwd++ )
	{
		for( int bck = fwd - 1; bck >= 0; bck-- )
		{
			int bckp = bck + 1;
			if( caseValues[bck] > caseValues[bckp] )
			{
				// Swap the values in both arrays
				int swap = caseValues[bckp];
				caseValues[bckp] = caseValues[bck];
				caseValues[bck] = swap;

				swap = caseLabels[bckp];
				caseLabels[bckp] = caseLabels[bck];
				caseLabels[bck] = swap;
			}
			else
				break;
		}
	}

	// Find ranges of consecutive numbers
	asCArray<int> ranges;
	ranges.PushLast(0);
	asUINT n;
	for( n = 1; n < caseValues.GetLength(); ++n )
	{
		// We can join numbers that are less than 5 numbers
		// apart since the output code will still be smaller
		if( caseValues[n] > caseValues[n-1] + 5 )
			ranges.PushLast(n);
	}

	// If the value is larger than the largest case value, jump to default
	int tmpOffset = AllocateVariable(asCDataType::CreatePrimitive(ttInt, false), true);
	expr.bc.InstrSHORT_DW(asBC_SetV4, (short)tmpOffset, caseValues[caseValues.GetLength()-1]);
	expr.bc.InstrW_W(asBC_CMPi, offset, tmpOffset);
	expr.bc.InstrDWORD(asBC_JP, defaultLabel);
	ReleaseTemporaryVariable(tmpOffset, &expr.bc);

	// TODO: runtime optimize: We could possibly optimize this even more by doing a
	//                         binary search instead of a linear search through the ranges

	// For each range
	int range;
	for( range = 0; range < (int)ranges.GetLength(); range++ )
	{
		// Find the largest value in this range
		int maxRange = caseValues[ranges[range]];
		int index = ranges[range];
		for( ; (index < (int)caseValues.GetLength()) && (caseValues[index] <= maxRange + 5); index++ )
			maxRange = caseValues[index];

		// If there are only 2 numbers then it is better to compare them directly
		if( index - ranges[range] > 2 )
		{
			// If the value is smaller than the smallest case value in the range, jump to default
			tmpOffset = AllocateVariable(asCDataType::CreatePrimitive(ttInt, false), true);
			expr.bc.InstrSHORT_DW(asBC_SetV4, (short)tmpOffset, caseValues[ranges[range]]);
			expr.bc.InstrW_W(asBC_CMPi, offset, tmpOffset);
			expr.bc.InstrDWORD(asBC_JS, defaultLabel);
			ReleaseTemporaryVariable(tmpOffset, &expr.bc);

			int nextRangeLabel = nextLabel++;
			// If this is the last range we don't have to make this test
			if( range < (int)ranges.GetLength() - 1 )
			{
				// If the value is larger than the largest case value in the range, jump to the next range
				tmpOffset = AllocateVariable(asCDataType::CreatePrimitive(ttInt, false), true);
				expr.bc.InstrSHORT_DW(asBC_SetV4, (short)tmpOffset, maxRange);
				expr.bc.InstrW_W(asBC_CMPi, offset, tmpOffset);
				expr.bc.InstrDWORD(asBC_JP, nextRangeLabel);
				ReleaseTemporaryVariable(tmpOffset, &expr.bc);
			}

			// Jump forward according to the value
			tmpOffset = AllocateVariable(asCDataType::CreatePrimitive(ttInt, false), true);
			expr.bc.InstrSHORT_DW(asBC_SetV4, (short)tmpOffset, caseValues[ranges[range]]);
			expr.bc.InstrW_W_W(asBC_SUBi, tmpOffset, offset, tmpOffset);
			ReleaseTemporaryVariable(tmpOffset, &expr.bc);
			expr.bc.JmpP(tmpOffset, maxRange - caseValues[ranges[range]]);

			// Add the list of jumps to the correct labels (any holes, jump to default)
			index = ranges[range];
			for( int n = caseValues[index]; n <= maxRange; n++ )
			{
				if( caseValues[index] == n )
					expr.bc.InstrINT(asBC_JMP, caseLabels[index++]);
				else
					expr.bc.InstrINT(asBC_JMP, defaultLabel);
			}

			expr.bc.Label((short)nextRangeLabel);
		}
		else
		{
			// Simply make a comparison with each value
			int n;
			for( n = ranges[range]; n < index; ++n )
			{
				tmpOffset = AllocateVariable(asCDataType::CreatePrimitive(ttInt, false), true);
				expr.bc.InstrSHORT_DW(asBC_SetV4, (short)tmpOffset, caseValues[n]);
				expr.bc.InstrW_W(asBC_CMPi, offset, tmpOffset);
				expr.bc.InstrDWORD(asBC_JZ, caseLabels[n]);
				ReleaseTemporaryVariable(tmpOffset, &expr.bc);
			}
		}
	}

	// Catch any value that falls trough
	expr.bc.InstrINT(asBC_JMP, defaultLabel);

	// Release the temporary variable previously stored
	ReleaseTemporaryVariable(expr.type, &expr.bc);

	// TODO: optimize: Should optimize each piece individually
	expr.bc.OptimizeLocally(tempVariableOffsets);

	//----------------------------------
    // Output case implementations
	//----------------------------------

	// Compile case implementations, each one with the label before it
	cnode = snode->firstChild->next;
	while( cnode )
	{
		// Each case should have a constant expression
		if( cnode->firstChild && cnode->firstChild->nodeType == snExpression )
		{
			expr.bc.Label((short)firstCaseLabel++);

			CompileCase(cnode->firstChild->next, &expr.bc);
		}
		else
		{
			expr.bc.Label((short)defaultLabel);

			// Is default the last case?
			if( cnode->next )
			{
				// We've already reported this error
				break;
			}

			CompileCase(cnode->firstChild, &expr.bc);
		}

		cnode = cnode->next;
	}

	//--------------------------------

	bc->AddCode(&expr.bc);

	// Add break label
	bc->Label((short)breakLabel);

	breakLabels.PopLast();
	RemoveVariableScope();
}

void asCCompiler::CompileCase(asCScriptNode *node, asCByteCode *bc)
{
	bool isFinished = false;
	bool hasReturn = false;
	bool hasUnreachableCode = false;
	while( node )
	{
		if( !hasUnreachableCode && (hasReturn || isFinished) )
		{
			hasUnreachableCode = true;
			Warning(TXT_UNREACHABLE_CODE, node);
			break;
		}

		if( node->nodeType == snBreak || node->nodeType == snContinue )
			isFinished = true;

		asCByteCode statement(engine);
		if( node->nodeType == snDeclaration )
		{
			Error(TXT_DECL_IN_SWITCH, node);

			// Compile it anyway to avoid further compiler errors
			CompileDeclaration(node, &statement);
		}
		else
			CompileStatement(node, &hasReturn, &statement);

		LineInstr(bc, node->tokenPos);
		bc->AddCode(&statement);

		if( !hasCompileErrors )
			asASSERT( tempVariables.GetLength() == 0 );

		node = node->next;
	}
}

void asCCompiler::CompileIfStatement(asCScriptNode *inode, bool *hasReturn, asCByteCode *bc)
{
	// We will use one label for the if statement
	// and possibly another for the else statement
	int afterLabel = nextLabel++;

	// Compile the expression
	asSExprContext expr(engine);
	CompileAssignment(inode->firstChild, &expr);
	if( !expr.type.dataType.IsEqualExceptRefAndConst(asCDataType::CreatePrimitive(ttBool, true)) )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, inode->firstChild);
		expr.type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), 1);
	}

	if( expr.type.dataType.IsReference() ) ConvertToVariable(&expr);
	ProcessDeferredParams(&expr);

	if( !expr.type.isConstant )
	{
		ProcessPropertyGetAccessor(&expr, inode);

		ConvertToVariable(&expr);

		// Add a test
		expr.bc.InstrSHORT(asBC_CpyVtoR4, expr.type.stackOffset);
		expr.bc.Instr(asBC_ClrHi);
		expr.bc.InstrDWORD(asBC_JZ, afterLabel);
		ReleaseTemporaryVariable(expr.type, &expr.bc);

		expr.bc.OptimizeLocally(tempVariableOffsets);
		bc->AddCode(&expr.bc);
	}
	else if( expr.type.dwordValue == 0 )
	{
		// Jump to the else case
		bc->InstrINT(asBC_JMP, afterLabel);

		// TODO: Should we warn that the expression will always go to the else?
	}

	// Compile the if statement
	bool origIsConstructorCalled = m_isConstructorCalled;

	bool hasReturn1;
	asCByteCode ifBC(engine);
	CompileStatement(inode->firstChild->next, &hasReturn1, &ifBC);

	// Add the byte code
	LineInstr(bc, inode->firstChild->next->tokenPos);
	bc->AddCode(&ifBC);

	if( inode->firstChild->next->nodeType == snExpressionStatement && inode->firstChild->next->firstChild == 0 )
	{
		// Don't allow  if( expr );
		Error(TXT_IF_WITH_EMPTY_STATEMENT, inode->firstChild->next);
	}

	// If one of the statements call the constructor, the other must as well
	// otherwise it is possible the constructor is never called
	bool constructorCall1 = false;
	bool constructorCall2 = false;
	if( !origIsConstructorCalled && m_isConstructorCalled )
		constructorCall1 = true;

	// Do we have an else statement?
	if( inode->firstChild->next != inode->lastChild )
	{
		// Reset the constructor called flag so the else statement can call the constructor too
		m_isConstructorCalled = origIsConstructorCalled;

		int afterElse = 0;
		if( !hasReturn1 )
		{
			afterElse = nextLabel++;

			// Add jump to after the else statement
			bc->InstrINT(asBC_JMP, afterElse);
		}

		// Add label for the else statement
		bc->Label((short)afterLabel);

		bool hasReturn2;
		asCByteCode elseBC(engine);
		CompileStatement(inode->lastChild, &hasReturn2, &elseBC);

		// Add byte code for the else statement
		LineInstr(bc, inode->lastChild->tokenPos);
		bc->AddCode(&elseBC);

		if( inode->lastChild->nodeType == snExpressionStatement && inode->lastChild->firstChild == 0 )
		{
			// Don't allow  if( expr ) {} else;
			Error(TXT_ELSE_WITH_EMPTY_STATEMENT, inode->lastChild);
		}

		if( !hasReturn1 )
		{
			// Add label for the end of else statement
			bc->Label((short)afterElse);
		}

		// The if statement only has return if both alternatives have
		*hasReturn = hasReturn1 && hasReturn2;

		if( !origIsConstructorCalled && m_isConstructorCalled )
			constructorCall2 = true;
	}
	else
	{
		// Add label for the end of if statement
		bc->Label((short)afterLabel);
		*hasReturn = false;
	}

	// Make sure both or neither conditions call a constructor
	if( (constructorCall1 && !constructorCall2) ||
		(constructorCall2 && !constructorCall1) )
	{
		Error(TXT_BOTH_CONDITIONS_MUST_CALL_CONSTRUCTOR, inode);
	}

	m_isConstructorCalled = origIsConstructorCalled || constructorCall1 || constructorCall2;
}

void asCCompiler::CompileForStatement(asCScriptNode *fnode, asCByteCode *bc)
{
	// Add a variable scope that will be used by CompileBreak/Continue to know where to stop deallocating variables
	AddVariableScope(true, true);

	// We will use three labels for the for loop
	int conditionLabel = nextLabel++;
	int afterLabel = nextLabel++;
	int continueLabel = nextLabel++;
	int insideLabel = nextLabel++;

	continueLabels.PushLast(continueLabel);
	breakLabels.PushLast(afterLabel);

	//---------------------------------------
	// Compile the initialization statement
	asCByteCode initBC(engine);
	LineInstr(&initBC, fnode->firstChild->tokenPos);
	if( fnode->firstChild->nodeType == snDeclaration )
		CompileDeclaration(fnode->firstChild, &initBC);
	else
		CompileExpressionStatement(fnode->firstChild, &initBC);

	//-----------------------------------
	// Compile the condition statement
	asSExprContext expr(engine);
	asCScriptNode *second = fnode->firstChild->next;
	if( second->firstChild )
	{
		int r = CompileAssignment(second->firstChild, &expr);
		if( r >= 0 )
		{
			if( !expr.type.dataType.IsEqualExceptRefAndConst(asCDataType::CreatePrimitive(ttBool, true)) )
				Error(TXT_EXPR_MUST_BE_BOOL, second);
			else
			{
				if( expr.type.dataType.IsReference() ) ConvertToVariable(&expr);
				ProcessDeferredParams(&expr);

				ProcessPropertyGetAccessor(&expr, second);

				// If expression is false exit the loop
				ConvertToVariable(&expr);
				expr.bc.InstrSHORT(asBC_CpyVtoR4, expr.type.stackOffset);
				expr.bc.Instr(asBC_ClrHi);
				expr.bc.InstrDWORD(asBC_JNZ, insideLabel);
				ReleaseTemporaryVariable(expr.type, &expr.bc);

				expr.bc.OptimizeLocally(tempVariableOffsets);

				// Prepend the line instruction for the condition
				asCByteCode tmp(engine);
				LineInstr(&tmp, second->firstChild->tokenPos);
				tmp.AddCode(&expr.bc);
				expr.bc.AddCode(&tmp);
			}
		}
	}

	//---------------------------
	// Compile the increment statement
	asCByteCode nextBC(engine);
	asCScriptNode *third = second->next;
	if( third->nodeType == snExpressionStatement )
	{
		LineInstr(&nextBC, third->tokenPos);
		CompileExpressionStatement(third, &nextBC);
	}

	//------------------------------
	// Compile loop statement
	bool hasReturn;
	asCByteCode forBC(engine);
	CompileStatement(fnode->lastChild, &hasReturn, &forBC);

	//-------------------------------
	// Join the code pieces
	bc->AddCode(&initBC);
	bc->InstrDWORD(asBC_JMP, conditionLabel);

	bc->Label((short)insideLabel);

	// Add a suspend bytecode inside the loop to guarantee
	// that the application can suspend the execution
	bc->Instr(asBC_SUSPEND);
	bc->InstrPTR(asBC_JitEntry, 0);

	LineInstr(bc, fnode->lastChild->tokenPos);
	bc->AddCode(&forBC);

	bc->Label((short)continueLabel);
	bc->AddCode(&nextBC);

	bc->Label((short)conditionLabel);
	if( expr.bc.GetLastInstr() == -1 )
		// There is no condition, so we just always jump
		bc->InstrDWORD(asBC_JMP, insideLabel);
	else
		bc->AddCode(&expr.bc);

	bc->Label((short)afterLabel);

	continueLabels.PopLast();
	breakLabels.PopLast();

	// Deallocate variables in this block, in reverse order
	for( int n = (int)variables->variables.GetLength() - 1; n >= 0; n-- )
	{
		sVariable *v = variables->variables[n];

		// Call variable destructors here, for variables not yet destroyed
		CallDestructor(v->type, v->stackOffset, v->onHeap, bc);

		// Don't deallocate function parameters
		if( v->stackOffset > 0 )
			DeallocateVariable(v->stackOffset);
	}

	RemoveVariableScope();
}

void asCCompiler::CompileWhileStatement(asCScriptNode *wnode, asCByteCode *bc)
{
	// Add a variable scope that will be used by CompileBreak/Continue to know where to stop deallocating variables
	AddVariableScope(true, true);

	// We will use two labels for the while loop
	int beforeLabel = nextLabel++;
	int afterLabel = nextLabel++;

	continueLabels.PushLast(beforeLabel);
	breakLabels.PushLast(afterLabel);

	// Add label before the expression
	bc->Label((short)beforeLabel);

	// Compile expression
	asSExprContext expr(engine);
	CompileAssignment(wnode->firstChild, &expr);
	if( !expr.type.dataType.IsEqualExceptRefAndConst(asCDataType::CreatePrimitive(ttBool, true)) )
		Error(TXT_EXPR_MUST_BE_BOOL, wnode->firstChild);
	else
	{
		if( expr.type.dataType.IsReference() ) ConvertToVariable(&expr);
		ProcessDeferredParams(&expr);

		ProcessPropertyGetAccessor(&expr, wnode);

		// Add byte code for the expression
		ConvertToVariable(&expr);

		// Jump to end of statement if expression is false
		expr.bc.InstrSHORT(asBC_CpyVtoR4, expr.type.stackOffset);
		expr.bc.Instr(asBC_ClrHi);
		expr.bc.InstrDWORD(asBC_JZ, afterLabel);
		ReleaseTemporaryVariable(expr.type, &expr.bc);

		expr.bc.OptimizeLocally(tempVariableOffsets);
		bc->AddCode(&expr.bc);
	}

	// Add a suspend bytecode inside the loop to guarantee
	// that the application can suspend the execution
	bc->Instr(asBC_SUSPEND);
	bc->InstrPTR(asBC_JitEntry, 0);

	// Compile statement
	bool hasReturn;
	asCByteCode whileBC(engine);
	CompileStatement(wnode->lastChild, &hasReturn, &whileBC);

	// Add byte code for the statement
	LineInstr(bc, wnode->lastChild->tokenPos);
	bc->AddCode(&whileBC);

	// Jump to the expression
	bc->InstrINT(asBC_JMP, beforeLabel);

	// Add label after the statement
	bc->Label((short)afterLabel);

	continueLabels.PopLast();
	breakLabels.PopLast();

	RemoveVariableScope();
}

void asCCompiler::CompileDoWhileStatement(asCScriptNode *wnode, asCByteCode *bc)
{
	// Add a variable scope that will be used by CompileBreak/Continue to know where to stop deallocating variables
	AddVariableScope(true, true);

	// We will use two labels for the while loop
	int beforeLabel = nextLabel++;
	int beforeTest = nextLabel++;
	int afterLabel = nextLabel++;

	continueLabels.PushLast(beforeTest);
	breakLabels.PushLast(afterLabel);

	// Add label before the statement
	bc->Label((short)beforeLabel);

	// Compile statement
	bool hasReturn;
	asCByteCode whileBC(engine);
	CompileStatement(wnode->firstChild, &hasReturn, &whileBC);

	// Add byte code for the statement
	LineInstr(bc, wnode->firstChild->tokenPos);
	bc->AddCode(&whileBC);

	// Add label before the expression
	bc->Label((short)beforeTest);

	// Add a suspend bytecode inside the loop to guarantee
	// that the application can suspend the execution
	bc->Instr(asBC_SUSPEND);
	bc->InstrPTR(asBC_JitEntry, 0);

	// Add a line instruction
	LineInstr(bc, wnode->lastChild->tokenPos);

	// Compile expression
	asSExprContext expr(engine);
	CompileAssignment(wnode->lastChild, &expr);
	if( !expr.type.dataType.IsEqualExceptRefAndConst(asCDataType::CreatePrimitive(ttBool, true)) )
		Error(TXT_EXPR_MUST_BE_BOOL, wnode->firstChild);
	else
	{
		if( expr.type.dataType.IsReference() ) ConvertToVariable(&expr);
		ProcessDeferredParams(&expr);

		ProcessPropertyGetAccessor(&expr, wnode);

		// Add byte code for the expression
		ConvertToVariable(&expr);

		// Jump to next iteration if expression is true
		expr.bc.InstrSHORT(asBC_CpyVtoR4, expr.type.stackOffset);
		expr.bc.Instr(asBC_ClrHi);
		expr.bc.InstrDWORD(asBC_JNZ, beforeLabel);
		ReleaseTemporaryVariable(expr.type, &expr.bc);

		expr.bc.OptimizeLocally(tempVariableOffsets);
		bc->AddCode(&expr.bc);
	}

	// Add label after the statement
	bc->Label((short)afterLabel);

	continueLabels.PopLast();
	breakLabels.PopLast();

	RemoveVariableScope();
}

void asCCompiler::CompileBreakStatement(asCScriptNode *node, asCByteCode *bc)
{
	if( breakLabels.GetLength() == 0 )
	{
		Error(TXT_INVALID_BREAK, node);
		return;
	}

	// Add destructor calls for all variables that will go out of scope
	// Put this clean up in a block to allow exception handler to understand them
	bc->Block(true);
	asCVariableScope *vs = variables;
	while( !vs->isBreakScope )
	{
		for( int n = (int)vs->variables.GetLength() - 1; n >= 0; n-- )
			CallDestructor(vs->variables[n]->type, vs->variables[n]->stackOffset, vs->variables[n]->onHeap, bc);

		vs = vs->parent;
	}
	bc->Block(false);

	bc->InstrINT(asBC_JMP, breakLabels[breakLabels.GetLength()-1]);
}

void asCCompiler::CompileContinueStatement(asCScriptNode *node, asCByteCode *bc)
{
	if( continueLabels.GetLength() == 0 )
	{
		Error(TXT_INVALID_CONTINUE, node);
		return;
	}

	// Add destructor calls for all variables that will go out of scope
	// Put this clean up in a block to allow exception handler to understand them
	bc->Block(true);
	asCVariableScope *vs = variables;
	while( !vs->isContinueScope )
	{
		for( int n = (int)vs->variables.GetLength() - 1; n >= 0; n-- )
			CallDestructor(vs->variables[n]->type, vs->variables[n]->stackOffset, vs->variables[n]->onHeap, bc);

		vs = vs->parent;
	}
	bc->Block(false);

	bc->InstrINT(asBC_JMP, continueLabels[continueLabels.GetLength()-1]);
}

void asCCompiler::CompileExpressionStatement(asCScriptNode *enode, asCByteCode *bc)
{
	if( enode->firstChild )
	{
		// Compile the expression
		asSExprContext expr(engine);
		CompileAssignment(enode->firstChild, &expr);

		// If we get here and there is still an unprocessed property
		// accessor, then process it as a get access. Don't call if there is
		// already a compile error, or we might report an error that is not valid
		if( !hasCompileErrors )
			ProcessPropertyGetAccessor(&expr, enode);

		// Pop the value from the stack
		if( !expr.type.dataType.IsPrimitive() )
			expr.bc.Instr(asBC_PopPtr);

		// Release temporary variables used by expression
		ReleaseTemporaryVariable(expr.type, &expr.bc);

		ProcessDeferredParams(&expr);

		expr.bc.OptimizeLocally(tempVariableOffsets);
		bc->AddCode(&expr.bc);
	}
}

void asCCompiler::PrepareTemporaryObject(asCScriptNode *node, asSExprContext *ctx, bool forceOnHeap)
{
	// If the object already is stored in temporary variable then nothing needs to be done
	// Note, a type can be temporary without being a variable, in which case it is holding off
	// on releasing a previously used object.
	if( ctx->type.isTemporary && ctx->type.isVariable &&
		!(forceOnHeap && !IsVariableOnHeap(ctx->type.stackOffset)) )
	{
		// If the temporary object is currently not a reference
		// the expression needs to be reevaluated to a reference
		if( !ctx->type.dataType.IsReference() )
		{
			ctx->bc.Instr(asBC_PopPtr);
			ctx->bc.InstrSHORT(asBC_PSF, ctx->type.stackOffset);
			ctx->type.dataType.MakeReference(true);
		}

		return;
	}

	// Allocate temporary variable
	asCDataType dt = ctx->type.dataType;
	dt.MakeReference(false);
	dt.MakeReadOnly(false);

	int offset = AllocateVariable(dt, true, forceOnHeap);

	// Objects stored on the stack are not considered references
	dt.MakeReference(IsVariableOnHeap(offset));

	asCTypeInfo lvalue;
	lvalue.Set(dt);
	lvalue.isExplicitHandle = ctx->type.isExplicitHandle;
	bool isExplicitHandle = ctx->type.isExplicitHandle;

	if( !dt.IsObjectHandle() &&
		dt.GetObjectType() && (dt.GetBehaviour()->copyconstruct || dt.GetBehaviour()->copyfactory) )
	{
		PrepareForAssignment(&lvalue.dataType, ctx, node, true);

		// Use the copy constructor/factory when available
		CallCopyConstructor(dt, offset, IsVariableOnHeap(offset), &ctx->bc, ctx, node);
	}
	else
	{
		// Allocate and construct the temporary object
		int r = CallDefaultConstructor(dt, offset, IsVariableOnHeap(offset), &ctx->bc, node);
		if( r < 0 )
		{
			Error(TXT_FAILED_TO_CREATE_TEMP_OBJ, node);
		}
		else
		{
			// Assign the object to the temporary variable
			PrepareForAssignment(&lvalue.dataType, ctx, node, true);

			ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
			r = PerformAssignment(&lvalue, &ctx->type, &ctx->bc, node);
			if( r < 0 )
			{
				Error(TXT_FAILED_TO_CREATE_TEMP_OBJ, node);
			}

			// Release any temp that may have been created as the result of opAssign
			ReleaseTemporaryVariable(lvalue, &ctx->bc);

			// Pop the original reference
			if( !lvalue.dataType.IsPrimitive() )
				ctx->bc.Instr(asBC_PopPtr);
		}

		// If the expression was holding off on releasing a
		// previously used object, we need to release it now
		if( ctx->type.isTemporary )
			ReleaseTemporaryVariable(ctx->type, &ctx->bc);
	}

	// Push the reference to the temporary variable on the stack
	ctx->bc.InstrSHORT(asBC_PSF, (short)offset);

	ctx->type.Set(dt);
	ctx->type.isTemporary = true;
	ctx->type.stackOffset = (short)offset;
	ctx->type.isVariable = true;
	ctx->type.isExplicitHandle = isExplicitHandle;
	ctx->type.dataType.MakeReference(IsVariableOnHeap(offset));
}

void asCCompiler::CompileReturnStatement(asCScriptNode *rnode, asCByteCode *bc)
{
	// Get return type and location
	sVariable *v = variables->GetVariable("return");

	// Basic validations
	if( v->type.GetSizeOnStackDWords() > 0 && !rnode->firstChild )
	{
		Error(TXT_MUST_RETURN_VALUE, rnode);
		return;
	}
	else if( v->type.GetSizeOnStackDWords() == 0 && rnode->firstChild )
	{
		Error(TXT_CANT_RETURN_VALUE, rnode);
		return;
	}

	// Compile the expression
	if( rnode->firstChild )
	{
		// Compile the expression
		asSExprContext expr(engine);
		int r = CompileAssignment(rnode->firstChild, &expr);
		if( r < 0 ) return;

		if( v->type.IsReference() )
		{
			// The expression that gives the reference must not use any of the
			// variables that must be destroyed upon exit, because then it means
			// reference will stay alive while the clean-up is done, which could
			// potentially mean that the reference is invalidated by the clean-up.
			//
			// When the function is returning a reference, the clean-up of the
			// variables must be done before the evaluation of the expression.
			//
			// A reference to a global variable, or a class member for class methods
			// should be allowed to be returned.

			if( !(expr.type.dataType.IsReference() ||
				  (expr.type.dataType.IsObject() && !expr.type.dataType.IsObjectHandle())) )
			{
				// Clean up the potential deferred parameters
				ProcessDeferredParams(&expr);
				Error(TXT_NOT_VALID_REFERENCE, rnode);
				return;
			}

			// No references to local variables, temporary variables, or parameters
			// are allowed to be returned, since they go out of scope when the function
			// returns. Even reference parameters are disallowed, since it is not possible
			// to know the scope of them. The exception is the 'this' pointer, which
			// is treated by the compiler as a local variable, but isn't really so.
			if( (expr.type.isVariable && !(expr.type.stackOffset == 0 && outFunc->objectType)) || expr.type.isTemporary )
			{
				// Clean up the potential deferred parameters
				ProcessDeferredParams(&expr);
				Error(TXT_CANNOT_RETURN_REF_TO_LOCAL, rnode);
				return;
			}

			// The type must match exactly as we cannot convert
			// the reference without loosing the original value
			if( !(v->type.IsEqualExceptConst(expr.type.dataType) ||
				  (expr.type.dataType.IsObject() &&
				   !expr.type.dataType.IsObjectHandle() &&
				   v->type.IsEqualExceptRefAndConst(expr.type.dataType))) ||
				(!v->type.IsReadOnly() && expr.type.dataType.IsReadOnly()) )
			{
				// Clean up the potential deferred parameters
				ProcessDeferredParams(&expr);
				asCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, expr.type.dataType.Format().AddressOf(), v->type.Format().AddressOf());
				Error(str, rnode);
				return;
			}

			// The expression must not have any deferred expressions, because the evaluation
			// of these cannot be done without keeping the reference which is not safe
			if( expr.deferredParams.GetLength() )
			{
				// Clean up the potential deferred parameters
				ProcessDeferredParams(&expr);
				Error(TXT_REF_CANT_BE_RETURNED_DEFERRED_PARAM, rnode);
				return;
			}

			// Make sure the expression isn't using any local variables that
			// will need to be cleaned up before the function completes
			asCArray<int> usedVars;
			expr.bc.GetVarsUsed(usedVars);
			for( asUINT n = 0; n < usedVars.GetLength(); n++ )
			{
				int var = GetVariableSlot(usedVars[n]);
				if( var != -1 )
				{
					asCDataType dt = variableAllocations[var];
					if( dt.IsObject() )
					{
						ProcessDeferredParams(&expr);
						Error(TXT_REF_CANT_BE_RETURNED_LOCAL_VARS, rnode);
						return;
					}
				}
			}

			// All objects in the function must be cleaned up before the expression
			// is evaluated, otherwise there is a possibility that the cleanup will
			// invalidate the reference.

			// Destroy the local variables before loading
			// the reference into the register. This will
			// be done before the expression is evaluated.
			DestroyVariables(bc);


			// For primitives the reference is already in the register,
			// but for non-primitives the reference is on the stack so we
			// need to load it into the register
			if( !expr.type.dataType.IsPrimitive() )
			{
				if( !expr.type.dataType.IsObjectHandle() &&
					expr.type.dataType.IsReference() )
					expr.bc.Instr(asBC_RDSPtr);

				expr.bc.Instr(asBC_PopRPtr);
			}

			// There are no temporaries to release so we're done
		}
		else // if( !v->type.IsReference() )
		{
			ProcessPropertyGetAccessor(&expr, rnode);

			// Prepare the value for assignment
			IsVariableInitialized(&expr.type, rnode->firstChild);

			if( v->type.IsPrimitive() )
			{
				if( expr.type.dataType.IsReference() ) ConvertToVariable(&expr);

				// Implicitly convert the value to the return type
				ImplicitConversion(&expr, v->type, rnode->firstChild, asIC_IMPLICIT_CONV);

				// Verify that the conversion was successful
				if( expr.type.dataType != v->type )
				{
					asCString str;
					str.Format(TXT_NO_CONVERSION_s_TO_s, expr.type.dataType.Format().AddressOf(), v->type.Format().AddressOf());
					Error(str, rnode);
					return;
				}
				else
				{
					ConvertToVariable(&expr);

					// Clean up the local variables and process deferred parameters
					DestroyVariables(&expr.bc);
					ProcessDeferredParams(&expr);

					ReleaseTemporaryVariable(expr.type, &expr.bc);

					// Load the variable in the register
					if( v->type.GetSizeOnStackDWords() == 1 )
						expr.bc.InstrSHORT(asBC_CpyVtoR4, expr.type.stackOffset);
					else
						expr.bc.InstrSHORT(asBC_CpyVtoR8, expr.type.stackOffset);
				}
			}
			else if( v->type.IsObject() )
			{
				// Value types are returned on the stack, in a location
				// that has been reserved by the calling function.
				if( outFunc->DoesReturnOnStack() )
				{
					// TODO: runtime optimize: If the return type has a constructor that takes the type of the expression,
					//                         it should be called directly instead of first converting the expression and
					//                         then copy the value.
					if( !v->type.IsEqualExceptRefAndConst(expr.type.dataType) )
					{
						ImplicitConversion(&expr, v->type, rnode->firstChild, asIC_IMPLICIT_CONV);
						if( !v->type.IsEqualExceptRefAndConst(expr.type.dataType) )
						{
							asCString str;
							str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, expr.type.dataType.Format().AddressOf(), v->type.Format().AddressOf());
							Error(str, rnode->firstChild);
							return;
						}
					}

					int offset = outFunc->objectType ? -AS_PTR_SIZE : 0;
					if( v->type.GetObjectType()->beh.copyconstruct )
					{
						PrepareForAssignment(&v->type, &expr, rnode->firstChild, false);
						CallCopyConstructor(v->type, offset, false, &expr.bc, &expr, rnode->firstChild, false, true);
					}
					else
					{
						// If the copy constructor doesn't exist, then a manual assignment needs to be done instead.
						CallDefaultConstructor(v->type, offset, false, &expr.bc, rnode->firstChild, 0, true);
						PrepareForAssignment(&v->type, &expr, rnode->firstChild, false);
						expr.bc.InstrSHORT(asBC_PSF, (short)offset);
						expr.bc.Instr(asBC_RDSPtr);

						asSExprContext lexpr(engine);
						lexpr.type.Set(v->type);
						lexpr.type.isLValue = true;
						PerformAssignment(&lexpr.type, &expr.type, &expr.bc, rnode->firstChild);
						expr.bc.Instr(asBC_PopPtr);

						// Release any temporary variable
						ReleaseTemporaryVariable(expr.type, &expr.bc);
					}

					// Clean up the local variables and process deferred parameters
					DestroyVariables(&expr.bc);
					ProcessDeferredParams(&expr);
				}
				else
				{
					asASSERT( v->type.GetObjectType()->flags & asOBJ_REF );

					// Prepare the expression to be loaded into the object
					// register. This will place the reference in local variable
					PrepareArgument(&v->type, &expr, rnode->firstChild, false, 0);

					// Pop the reference to the temporary variable
					expr.bc.Instr(asBC_PopPtr);

					// Clean up the local variables and process deferred parameters
					DestroyVariables(&expr.bc);
					ProcessDeferredParams(&expr);

					// Load the object pointer into the object register
					// LOADOBJ also clears the address in the variable
					expr.bc.InstrSHORT(asBC_LOADOBJ, expr.type.stackOffset);

					// LOADOBJ cleared the address in the variable so the object will not be freed
					// here, but the temporary variable must still be freed so the slot can be reused
					// By releasing without the bytecode we do just that.
					ReleaseTemporaryVariable(expr.type, 0);
				}
			}
		}

		expr.bc.OptimizeLocally(tempVariableOffsets);
		bc->AddCode(&expr.bc);
	}
	else
	{
		// For functions that don't return anything
		// we just detroy the local variables
		DestroyVariables(bc);
	}

	// Jump to the end of the function
	bc->InstrINT(asBC_JMP, 0);
}

void asCCompiler::DestroyVariables(asCByteCode *bc)
{
	// Call destructor on all variables except for the function parameters
	// Put the clean-up in a block to allow exception handler to understand this
	bc->Block(true);
	asCVariableScope *vs = variables;
	while( vs )
	{
		for( int n = (int)vs->variables.GetLength() - 1; n >= 0; n-- )
			if( vs->variables[n]->stackOffset > 0 )
				CallDestructor(vs->variables[n]->type, vs->variables[n]->stackOffset, vs->variables[n]->onHeap, bc);

		vs = vs->parent;
	}
	bc->Block(false);
}

void asCCompiler::AddVariableScope(bool isBreakScope, bool isContinueScope)
{
	variables = asNEW(asCVariableScope)(variables);
	if( variables == 0 )
	{
		// Out of memory
		return;
	}
	variables->isBreakScope    = isBreakScope;
	variables->isContinueScope = isContinueScope;
}

void asCCompiler::RemoveVariableScope()
{
	if( variables )
	{
		asCVariableScope *var = variables;
		variables = variables->parent;
		asDELETE(var,asCVariableScope);
	}
}

void asCCompiler::Error(const asCString &msg, asCScriptNode *node)
{
	asCString str;

	int r = 0, c = 0;
	asASSERT( node );
	if( node ) script->ConvertPosToRowCol(node->tokenPos, &r, &c);

	builder->WriteError(script->name, msg, r, c);

	hasCompileErrors = true;
}

void asCCompiler::Warning(const asCString &msg, asCScriptNode *node)
{
	asCString str;

	int r = 0, c = 0;
	asASSERT( node );
	if( node ) script->ConvertPosToRowCol(node->tokenPos, &r, &c);

	builder->WriteWarning(script->name, msg, r, c);
}

void asCCompiler::Information(const asCString &msg, asCScriptNode *node)
{
	asCString str;

	int r = 0, c = 0;
	asASSERT( node );
	if( node ) script->ConvertPosToRowCol(node->tokenPos, &r, &c);

	builder->WriteInfo(script->name, msg, r, c, false);
}

void asCCompiler::PrintMatchingFuncs(asCArray<int> &funcs, asCScriptNode *node)
{
	int r = 0, c = 0;
	asASSERT( node );
	if( node ) script->ConvertPosToRowCol(node->tokenPos, &r, &c);

	for( unsigned int n = 0; n < funcs.GetLength(); n++ )
	{
		asIScriptFunction *func = builder->GetFunctionDescription(funcs[n]);

		builder->WriteInfo(script->name, func->GetDeclaration(true), r, c, false);
	}
}

int asCCompiler::AllocateVariableNotIn(const asCDataType &type, bool isTemporary, bool forceOnHeap, asSExprContext *ctx)
{
	int l = int(reservedVariables.GetLength());
	ctx->bc.GetVarsUsed(reservedVariables);
	int var = AllocateVariable(type, isTemporary, forceOnHeap);
	reservedVariables.SetLength(l);
	return var;
}

int asCCompiler::AllocateVariable(const asCDataType &type, bool isTemporary, bool forceOnHeap)
{
	asCDataType t(type);

	if( t.IsPrimitive() && t.GetSizeOnStackDWords() == 1 )
		t.SetTokenType(ttInt);

	if( t.IsPrimitive() && t.GetSizeOnStackDWords() == 2 )
		t.SetTokenType(ttDouble);

	// Only null handles have the token type unrecognized token
	asASSERT( t.IsObjectHandle() || t.GetTokenType() != ttUnrecognizedToken );

	bool isOnHeap = true;
	if( t.IsPrimitive() ||
		(t.GetObjectType() && (t.GetObjectType()->GetFlags() & asOBJ_VALUE) && !forceOnHeap) )
	{
		// Primitives and value types (unless overridden) are allocated on the stack
		isOnHeap = false;
	}

	// Find a free location with the same type
	for( asUINT n = 0; n < freeVariables.GetLength(); n++ )
	{
		int slot = freeVariables[n];

		if( variableAllocations[slot].IsEqualExceptConst(t) &&
			variableIsTemporary[slot] == isTemporary &&
			variableIsOnHeap[slot] == isOnHeap )
		{
			// We can't return by slot, must count variable sizes
			int offset = GetVariableOffset(slot);

			// Verify that it is not in the list of reserved variables
			bool isUsed = false;
			if( reservedVariables.GetLength() )
				isUsed = reservedVariables.Exists(offset);

			if( !isUsed )
			{
				if( n != freeVariables.GetLength() - 1 )
					freeVariables[n] = freeVariables.PopLast();
				else
					freeVariables.PopLast();

				if( isTemporary )
					tempVariables.PushLast(offset);

				return offset;
			}
		}
	}

	variableAllocations.PushLast(t);
	variableIsTemporary.PushLast(isTemporary);
	variableIsOnHeap.PushLast(isOnHeap);

	int offset = GetVariableOffset((int)variableAllocations.GetLength()-1);

	if( isTemporary )
	{
		// Add offset to the currently allocated temporary variables
		tempVariables.PushLast(offset);

		// Add offset to all known offsets to temporary variables, whether allocated or not
		tempVariableOffsets.PushLast(offset);
	}

	return offset;
}

int asCCompiler::GetVariableOffset(int varIndex)
{
	// Return offset to the last dword on the stack

	// Start at 1 as offset 0 is reserved for the this pointer (or first argument for global functions)
	int varOffset = 1;

	// Skip lower variables
	for( int n = 0; n < varIndex; n++ )
	{
		if( !variableIsOnHeap[n] && variableAllocations[n].IsObject() )
			varOffset += variableAllocations[n].GetSizeInMemoryDWords();
		else
			varOffset += variableAllocations[n].GetSizeOnStackDWords();
	}

	if( varIndex < (int)variableAllocations.GetLength() )
	{
		// For variables larger than 1 dword the returned offset should be to the last dword
		int size;
		if( !variableIsOnHeap[varIndex] && variableAllocations[varIndex].IsObject() )
			size = variableAllocations[varIndex].GetSizeInMemoryDWords();
		else
			size = variableAllocations[varIndex].GetSizeOnStackDWords();
		if( size > 1 )
			varOffset += size-1;
	}

	return varOffset;
}


int asCCompiler::GetVariableSlot(int offset)
{
	int varOffset = 1;
	for( asUINT n = 0; n < variableAllocations.GetLength(); n++ )
	{
		if( !variableIsOnHeap[n] && variableAllocations[n].IsObject() )
			varOffset += -1 + variableAllocations[n].GetSizeInMemoryDWords();
		else
			varOffset += -1 + variableAllocations[n].GetSizeOnStackDWords();

		if( varOffset == offset )
			return n;

		varOffset++;
	}

	return -1;
}

bool asCCompiler::IsVariableOnHeap(int offset)
{
	int varSlot = GetVariableSlot(offset);
	if( varSlot < 0 )
	{
		// This happens for function arguments that are considered as on the heap
		return true;
	}

	return variableIsOnHeap[varSlot];
}

void asCCompiler::DeallocateVariable(int offset)
{
	// Remove temporary variable
	int n;
	for( n = 0; n < (int)tempVariables.GetLength(); n++ )
	{
		if( offset == tempVariables[n] )
		{
			if( n == (int)tempVariables.GetLength()-1 )
				tempVariables.PopLast();
			else
				tempVariables[n] = tempVariables.PopLast();
			break;
		}
	}

	n = GetVariableSlot(offset);
	if( n != -1 )
	{
		freeVariables.PushLast(n);
		return;
	}

	// We might get here if the variable was implicitly declared
	// because it was use before a formal declaration, in this case
	// the offset is 0x7FFF

	asASSERT(offset == 0x7FFF);
}

void asCCompiler::ReleaseTemporaryVariable(asCTypeInfo &t, asCByteCode *bc)
{
	if( t.isTemporary )
	{
		ReleaseTemporaryVariable(t.stackOffset, bc);
		t.isTemporary = false;
	}
}

void asCCompiler::ReleaseTemporaryVariable(int offset, asCByteCode *bc)
{
	if( bc )
	{
		// We need to call the destructor on the true variable type
		int n = GetVariableSlot(offset);
		asASSERT( n >= 0 );
		if( n >= 0 )
		{
			asCDataType dt = variableAllocations[n];
			bool isOnHeap = variableIsOnHeap[n];

			// Call destructor
			CallDestructor(dt, offset, isOnHeap, bc);
		}
	}

	DeallocateVariable(offset);
}

void asCCompiler::Dereference(asSExprContext *ctx, bool generateCode)
{
	if( ctx->type.dataType.IsReference() )
	{
		if( ctx->type.dataType.IsObject() )
		{
			ctx->type.dataType.MakeReference(false);
			if( generateCode )
				ctx->bc.Instr(asBC_RDSPtr);
		}
		else
		{
			// This should never happen as primitives are treated differently
			asASSERT(false);
		}
	}
}

bool asCCompiler::IsVariableInitialized(asCTypeInfo *type, asCScriptNode *node)
{
	// No need to check if there is no variable scope
	if( variables == 0 ) return true;

	// Temporary variables are assumed to be initialized
	if( type->isTemporary ) return true;

	// Verify that it is a variable
	if( !type->isVariable ) return true;

	// Find the variable
	sVariable *v = variables->GetVariableByOffset(type->stackOffset);

	// The variable isn't found if it is a constant, in which case it is guaranteed to be initialized
	if( v == 0 ) return true;

	if( v->isInitialized ) return true;

	// Complex types don't need this test
	if( v->type.IsObject() ) return true;

	// Mark as initialized so that the user will not be bothered again
	v->isInitialized = true;

	// Write warning
	asCString str;
	str.Format(TXT_s_NOT_INITIALIZED, (const char *)v->name.AddressOf());
	Warning(str, node);

	return false;
}

void asCCompiler::PrepareOperand(asSExprContext *ctx, asCScriptNode *node)
{
	// Check if the variable is initialized (if it indeed is a variable)
	IsVariableInitialized(&ctx->type, node);

	asCDataType to = ctx->type.dataType;
	to.MakeReference(false);

	ImplicitConversion(ctx, to, node, asIC_IMPLICIT_CONV);

	ProcessDeferredParams(ctx);
}

void asCCompiler::PrepareForAssignment(asCDataType *lvalue, asSExprContext *rctx, asCScriptNode *node, bool toTemporary, asSExprContext *lvalueExpr)
{
	// Reserve the temporary variables used in the lvalue expression so they won't end up being used by the rvalue too
	int l = int(reservedVariables.GetLength());
	if( lvalueExpr ) lvalueExpr->bc.GetVarsUsed(reservedVariables);


	ProcessPropertyGetAccessor(rctx, node);

	// Make sure the rvalue is initialized if it is a variable
	IsVariableInitialized(&rctx->type, node);

	if( lvalue->IsPrimitive() )
	{
		if( rctx->type.dataType.IsPrimitive() )
		{
			if( rctx->type.dataType.IsReference() )
			{
				// Cannot do implicit conversion of references so we first convert the reference to a variable
				ConvertToVariableNotIn(rctx, lvalueExpr);
			}
		}

		// Implicitly convert the value to the right type
		ImplicitConversion(rctx, *lvalue, node, asIC_IMPLICIT_CONV);

		// Check data type
		if( !lvalue->IsEqualExceptRefAndConst(rctx->type.dataType) )
		{
			asCString str;
			str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rctx->type.dataType.Format().AddressOf(), lvalue->Format().AddressOf());
			Error(str, node);

			rctx->type.SetDummy();
		}

		// Make sure the rvalue is a variable
		if( !rctx->type.isVariable )
			ConvertToVariableNotIn(rctx, lvalueExpr);
	}
	else
	{
		asCDataType to = *lvalue;
		to.MakeReference(false);

		// TODO: ImplicitConversion should know to do this by itself
		// First convert to a handle which will do a reference cast
		if( !lvalue->IsObjectHandle() &&
			(lvalue->GetObjectType()->flags & asOBJ_SCRIPT_OBJECT) )
			to.MakeHandle(true);

		// Don't allow the implicit conversion to create an object
		ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV, true, !toTemporary);

		if( !lvalue->IsObjectHandle() &&
			(lvalue->GetObjectType()->flags & asOBJ_SCRIPT_OBJECT) )
		{
			// Then convert to a reference, which will validate the handle
			to.MakeHandle(false);
			ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV, true, !toTemporary);
		}

		// Check data type
		if( !lvalue->IsEqualExceptRefAndConst(rctx->type.dataType) )
		{
			asCString str;
			str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rctx->type.dataType.Format().AddressOf(), lvalue->Format().AddressOf());
			Error(str, node);
		}
		else
		{
			// If the assignment will be made with the copy behaviour then the rvalue must not be a reference
			if( lvalue->IsObject() )
				asASSERT(!rctx->type.dataType.IsReference());
		}
	}

	// Unreserve variables
	reservedVariables.SetLength(l);
}

bool asCCompiler::IsLValue(asCTypeInfo &type)
{
	if( !type.isLValue ) return false;
	if( type.dataType.IsReadOnly() ) return false;
	if( !type.dataType.IsObject() && !type.isVariable && !type.dataType.IsReference() ) return false;
	return true;
}

int asCCompiler::PerformAssignment(asCTypeInfo *lvalue, asCTypeInfo *rvalue, asCByteCode *bc, asCScriptNode *node)
{
	if( lvalue->dataType.IsReadOnly() )
	{
		Error(TXT_REF_IS_READ_ONLY, node);
		return -1;
	}

	if( lvalue->dataType.IsPrimitive() )
	{
		if( lvalue->isVariable )
		{
			// Copy the value between the variables directly
			if( lvalue->dataType.GetSizeInMemoryDWords() == 1 )
				bc->InstrW_W(asBC_CpyVtoV4, lvalue->stackOffset, rvalue->stackOffset);
			else
				bc->InstrW_W(asBC_CpyVtoV8, lvalue->stackOffset, rvalue->stackOffset);

			// Mark variable as initialized
			sVariable *v = variables->GetVariableByOffset(lvalue->stackOffset);
			if( v ) v->isInitialized = true;
		}
		else if( lvalue->dataType.IsReference() )
		{
			// Copy the value of the variable to the reference in the register
			int s = lvalue->dataType.GetSizeInMemoryBytes();
			if( s == 1 )
				bc->InstrSHORT(asBC_WRTV1, rvalue->stackOffset);
			else if( s == 2 )
				bc->InstrSHORT(asBC_WRTV2, rvalue->stackOffset);
			else if( s == 4 )
				bc->InstrSHORT(asBC_WRTV4, rvalue->stackOffset);
			else if( s == 8 )
				bc->InstrSHORT(asBC_WRTV8, rvalue->stackOffset);
		}
		else
		{
			Error(TXT_NOT_VALID_LVALUE, node);
			return -1;
		}
	}
	else if( !lvalue->isExplicitHandle )
	{
		asSExprContext ctx(engine);
		ctx.type = *lvalue;
		Dereference(&ctx, true);
		*lvalue = ctx.type;
		bc->AddCode(&ctx.bc);

		// TODO: Should find the opAssign method that implements the default copy behaviour.
		//       The beh->copy member will be removed.
		asSTypeBehaviour *beh = lvalue->dataType.GetBehaviour();
		if( beh->copy && beh->copy != engine->scriptTypeBehaviours.beh.copy )
		{
			asSExprContext res(engine);
			PerformFunctionCall(beh->copy, &res, false, 0, lvalue->dataType.GetObjectType());

			bc->AddCode(&res.bc);
			*lvalue = res.type;
		}
		else if( beh->copy == engine->scriptTypeBehaviours.beh.copy )
		{
			// Call the default copy operator for script classes
			// This is done differently because the default copy operator
			// is registered as returning int&, but in reality it returns
			// a reference to the object. 
			// TODO: Avoid this special case by implementing a copystub for
			//       script classes that uses the default copy operator
			bc->Call(asBC_CALLSYS, beh->copy, 2*AS_PTR_SIZE);
			bc->Instr(asBC_PshRPtr);
		}
		else
		{
			// Default copy operator
			if( lvalue->dataType.GetSizeInMemoryDWords() == 0 ||
				!(lvalue->dataType.GetObjectType()->flags & asOBJ_POD) )
			{
				asCString msg;
				msg.Format(TXT_NO_DEFAULT_COPY_OP_FOR_s, lvalue->dataType.GetObjectType()->name.AddressOf());
				Error(msg, node);
				return -1;
			}

			// Copy larger data types from a reference
			// TODO: runtime optimize: COPY should pop both arguments and store the reference in the register.
			bc->InstrSHORT_DW(asBC_COPY, (short)lvalue->dataType.GetSizeInMemoryDWords(), engine->GetTypeIdFromDataType(lvalue->dataType));
		}
	}
	else
	{
		// TODO: The object handle can be stored in a variable as well
		if( !lvalue->dataType.IsReference() )
		{
			Error(TXT_NOT_VALID_REFERENCE, node);
			return -1;
		}

		bc->InstrPTR(asBC_REFCPY, lvalue->dataType.GetObjectType());

		// Mark variable as initialized
		if( variables )
		{
			sVariable *v = variables->GetVariableByOffset(lvalue->stackOffset);
			if( v ) v->isInitialized = true;
		}
	}

	return 0;
}

bool asCCompiler::CompileRefCast(asSExprContext *ctx, const asCDataType &to, bool isExplicit, asCScriptNode *node, bool generateCode)
{
	bool conversionDone = false;

	asCArray<int> ops;
	asUINT n;

	if( ctx->type.dataType.GetObjectType()->flags & asOBJ_SCRIPT_OBJECT )
	{
		// We need it to be a reference
		if( !ctx->type.dataType.IsReference() )
		{
			asCDataType to = ctx->type.dataType;
			to.MakeReference(true);
			ImplicitConversion(ctx, to, 0, isExplicit ? asIC_EXPLICIT_REF_CAST : asIC_IMPLICIT_CONV, generateCode);
		}

		if( isExplicit )
		{
			// Allow dynamic cast between object handles (only for script objects).
			// At run time this may result in a null handle,
			// which when used will throw an exception
			conversionDone = true;
			if( generateCode )
			{
				ctx->bc.InstrDWORD(asBC_Cast, engine->GetTypeIdFromDataType(to));

				// Allocate a temporary variable for the returned object
				int returnOffset = AllocateVariable(to, true);

				// Move the pointer from the object register to the temporary variable
				ctx->bc.InstrSHORT(asBC_STOREOBJ, (short)returnOffset);

				ctx->bc.InstrSHORT(asBC_PSF, (short)returnOffset);

				ReleaseTemporaryVariable(ctx->type, &ctx->bc);

				ctx->type.SetVariable(to, returnOffset, true);
				ctx->type.dataType.MakeReference(true);
			}
			else
			{
				ctx->type.dataType = to;
				ctx->type.dataType.MakeReference(true);
			}
		}
		else
		{
			if( ctx->type.dataType.GetObjectType()->DerivesFrom(to.GetObjectType()) )
			{
				conversionDone = true;
				ctx->type.dataType.SetObjectType(to.GetObjectType());
			}
		}
	}
	else
	{
		// Find a suitable registered behaviour
		asSTypeBehaviour *beh = &ctx->type.dataType.GetObjectType()->beh;
		for( n = 0; n < beh->operators.GetLength(); n+= 2 )
		{
			if( (isExplicit && asBEHAVE_REF_CAST == beh->operators[n]) ||
				asBEHAVE_IMPLICIT_REF_CAST == beh->operators[n] )
			{
				int funcId = beh->operators[n+1];

				// Is the operator for the output type?
				asCScriptFunction *func = engine->scriptFunctions[funcId];
				if( func->returnType.GetObjectType() != to.GetObjectType() )
					continue;

				ops.PushLast(funcId);
			}
		}

		// It shouldn't be possible to have more than one
		asASSERT( ops.GetLength() <= 1 );

		// Should only have one behaviour for each output type
		if( ops.GetLength() == 1 )
		{
			if( generateCode )
			{
				// TODO: runtime optimize: Instead of producing bytecode for checking if the handle is
				//                         null, we can create a special CALLSYS instruction that checks
				//                         if the object pointer is null and if so sets the object register
				//                         to null directly without executing the function.
				//
				//                         Alternatively I could force the ref cast behaviours be global
				//                         functions with 1 parameter, even though they should still be
				//                         registered with RegisterObjectBehaviour()

				// Add code to avoid calling the cast behaviour if the handle is already null,
				// because that will raise a null pointer exception due to the cast behaviour
				// being a class method, and the this pointer cannot be null.

				if( ctx->type.isVariable )
					ctx->bc.Instr(asBC_PopPtr);
				else
				{
					Dereference(ctx, true);
					ConvertToVariable(ctx);
				}

				// TODO: runtime optimize: should have immediate comparison for null pointer
				int offset = AllocateVariable(asCDataType::CreateNullHandle(), true);
				// TODO: runtime optimize: ClrVPtr is not necessary, because the VM should initialize the variable to null anyway (it is currently not done for null pointers though)
				ctx->bc.InstrSHORT(asBC_ClrVPtr, (asWORD)offset);
				ctx->bc.InstrW_W(asBC_CmpPtr, ctx->type.stackOffset, offset);
				DeallocateVariable(offset);

				int afterLabel = nextLabel++;
				ctx->bc.InstrDWORD(asBC_JZ, afterLabel);

				// Call the cast operator
				ctx->bc.InstrSHORT(asBC_PSF, ctx->type.stackOffset);
				ctx->bc.Instr(asBC_RDSPtr);
				ctx->type.dataType.MakeReference(false);

				asCTypeInfo objType = ctx->type;
				asCArray<asSExprContext *> args;
				MakeFunctionCall(ctx, ops[0], objType.dataType.GetObjectType(), args, node);

				ctx->bc.Instr(asBC_PopPtr);

				int endLabel = nextLabel++;

				ctx->bc.InstrINT(asBC_JMP, endLabel);
				ctx->bc.Label((short)afterLabel);

				// Make a NULL pointer
				ctx->bc.InstrSHORT(asBC_ClrVPtr, ctx->type.stackOffset);
				ctx->bc.Label((short)endLabel);

				// Since we're receiving a handle, we can release the original variable
				ReleaseTemporaryVariable(objType, &ctx->bc);

				// Push the reference to the handle on the stack
				ctx->bc.InstrSHORT(asBC_PSF, ctx->type.stackOffset);
			}
			else
			{
				asCScriptFunction *func = engine->scriptFunctions[ops[0]];
				ctx->type.Set(func->returnType);
			}
		}
		else if( ops.GetLength() == 0 )
		{
			// Check for the generic ref cast behaviour
			for( n = 0; n < beh->operators.GetLength(); n+= 2 )
			{
				if( (isExplicit && asBEHAVE_REF_CAST == beh->operators[n]) ||
					asBEHAVE_IMPLICIT_REF_CAST == beh->operators[n] )
				{
					int funcId = beh->operators[n+1];

					// Does the operator take the ?&out parameter?
					asCScriptFunction *func = engine->scriptFunctions[funcId];
					if( func->parameterTypes.GetLength() != 1 ||
						func->parameterTypes[0].GetTokenType() != ttQuestion ||
						func->inOutFlags[0] != asTM_OUTREF )
						continue;

					ops.PushLast(funcId);
				}
			}

			// It shouldn't be possible to have more than one
			asASSERT( ops.GetLength() <= 1 );

			if( ops.GetLength() == 1 )
			{
				if( generateCode )
				{
					asASSERT(to.IsObjectHandle());

					// Allocate a temporary variable of the requested handle type
					int stackOffset = AllocateVariableNotIn(to, true, false, ctx);

					// Pass the reference of that variable to the function as output parameter
					asCDataType toRef(to);
					toRef.MakeReference(true);
					asCArray<asSExprContext *> args;
					asSExprContext arg(engine);
					arg.bc.InstrSHORT(asBC_PSF, (short)stackOffset);
					// Don't mark the variable as temporary, so it won't be freed too early
					arg.type.SetVariable(toRef, stackOffset, false);
					arg.type.isLValue = true;
					arg.type.isExplicitHandle = true;
					args.PushLast(&arg);

					asCTypeInfo prev = ctx->type;

					// Call the behaviour method
					MakeFunctionCall(ctx, ops[0], ctx->type.dataType.GetObjectType(), args, node);

					// Release previous temporary variable
					ReleaseTemporaryVariable(prev, &ctx->bc);

					// Use the reference to the variable as the result of the expression
					// Now we can mark the variable as temporary
					ctx->type.SetVariable(toRef, stackOffset, true);
					ctx->bc.InstrSHORT(asBC_PSF, (short)stackOffset);
				}
				else
				{
					// All casts are legal
					ctx->type.Set(to);
				}
			}
		}
	}

	return conversionDone;
}

asUINT asCCompiler::ImplicitConvPrimitiveToPrimitive(asSExprContext *ctx, const asCDataType &toOrig, asCScriptNode *node, EImplicitConv convType, bool generateCode)
{
	asCDataType to = toOrig;
	to.MakeReference(false);
	asASSERT( !ctx->type.dataType.IsReference() );

	// Maybe no conversion is needed
	if( to.IsEqualExceptConst(ctx->type.dataType) )
	{
		// A primitive is const or not
		ctx->type.dataType.MakeReadOnly(to.IsReadOnly());
		return asCC_NO_CONV;
	}

	// Is the conversion an ambiguous enum value?
	if( ctx->enumValue != "" )
	{
		if( to.IsEnumType() )
		{
			// Attempt to resolve an ambiguous enum value
			asCDataType out;
			asDWORD value;
			if( builder->GetEnumValueFromObjectType(to.GetObjectType(), ctx->enumValue.AddressOf(), out, value) )
			{
				ctx->type.SetConstantDW(out, value);
				ctx->type.dataType.MakeReadOnly(to.IsReadOnly());

				// It wasn't really a conversion. The compiler just resolved the ambiguity (or not)
				return asCC_NO_CONV;
			}
		}

		// The enum value is ambiguous
		if( node && generateCode )
			Error(TXT_FOUND_MULTIPLE_ENUM_VALUES, node);

		// Set a dummy to allow the compiler to try to continue the conversion
		ctx->type.SetDummy();
	}

	// Determine the cost of this conversion
	asUINT cost = asCC_NO_CONV;
	if( (to.IsIntegerType() || to.IsUnsignedType()) && (ctx->type.dataType.IsFloatType() || ctx->type.dataType.IsDoubleType()) )
		cost = asCC_INT_FLOAT_CONV;
	else if( (to.IsFloatType() || to.IsDoubleType()) && (ctx->type.dataType.IsIntegerType() || ctx->type.dataType.IsUnsignedType()) )
		cost = asCC_INT_FLOAT_CONV;
	else if( to.IsUnsignedType() && ctx->type.dataType.IsIntegerType() )
		cost = asCC_SIGNED_CONV;
	else if( to.IsIntegerType() && ctx->type.dataType.IsUnsignedType() )
		cost = asCC_SIGNED_CONV;
	else if( to.GetSizeInMemoryBytes() || ctx->type.dataType.GetSizeInMemoryBytes() )
		cost = asCC_PRIMITIVE_SIZE_CONV;

	// Start by implicitly converting constant values
	if( ctx->type.isConstant )
	{
		ImplicitConversionConstant(ctx, to, node, convType);
		ctx->type.dataType.MakeReadOnly(to.IsReadOnly());
		return cost;
	}

	// Allow implicit conversion between numbers
	if( generateCode )
	{
		// When generating the code the decision has already been made, so we don't bother determining the cost

		// Convert smaller types to 32bit first
		int s = ctx->type.dataType.GetSizeInMemoryBytes();
		if( s < 4 )
		{
			ConvertToTempVariable(ctx);
			if( ctx->type.dataType.IsIntegerType() )
			{
				if( s == 1 )
					ctx->bc.InstrSHORT(asBC_sbTOi, ctx->type.stackOffset);
				else if( s == 2 )
					ctx->bc.InstrSHORT(asBC_swTOi, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(ttInt);
			}
			else if( ctx->type.dataType.IsUnsignedType() )
			{
				if( s == 1 )
					ctx->bc.InstrSHORT(asBC_ubTOi, ctx->type.stackOffset);
				else if( s == 2 )
					ctx->bc.InstrSHORT(asBC_uwTOi, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(ttUInt);
			}
		}

		if( (to.IsIntegerType() && to.GetSizeInMemoryDWords() == 1 && !to.IsEnumType()) ||
			(to.IsEnumType() && convType == asIC_EXPLICIT_VAL_CAST) )
		{
			if( ctx->type.dataType.IsIntegerType() ||
				ctx->type.dataType.IsUnsignedType() )
			{
				if( ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				{
					ctx->type.dataType.SetTokenType(to.GetTokenType());
					ctx->type.dataType.SetObjectType(to.GetObjectType());
				}
				else
				{
					ConvertToTempVariable(ctx);
					ReleaseTemporaryVariable(ctx->type, &ctx->bc);
					int offset = AllocateVariable(to, true);
					ctx->bc.InstrW_W(asBC_i64TOi, offset, ctx->type.stackOffset);
					ctx->type.SetVariable(to, offset, true);
				}
			}
			else if( ctx->type.dataType.IsFloatType() )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_fTOi, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}
			else if( ctx->type.dataType.IsDoubleType() )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_dTOi, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}

			// Convert to smaller integer if necessary
			int s = to.GetSizeInMemoryBytes();
			if( s < 4 )
			{
				ConvertToTempVariable(ctx);
				if( s == 1 )
					ctx->bc.InstrSHORT(asBC_iTOb, ctx->type.stackOffset);
				else if( s == 2 )
					ctx->bc.InstrSHORT(asBC_iTOw, ctx->type.stackOffset);
			}
		}
		else if( to.IsIntegerType() && to.GetSizeInMemoryDWords() == 2 )
		{
			if( ctx->type.dataType.IsIntegerType() ||
				ctx->type.dataType.IsUnsignedType() )
			{
				if( ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
				{
					ctx->type.dataType.SetTokenType(to.GetTokenType());
					ctx->type.dataType.SetObjectType(to.GetObjectType());
				}
				else
				{
					ConvertToTempVariable(ctx);
					ReleaseTemporaryVariable(ctx->type, &ctx->bc);
					int offset = AllocateVariable(to, true);
					if( ctx->type.dataType.IsUnsignedType() )
						ctx->bc.InstrW_W(asBC_uTOi64, offset, ctx->type.stackOffset);
					else
						ctx->bc.InstrW_W(asBC_iTOi64, offset, ctx->type.stackOffset);
					ctx->type.SetVariable(to, offset, true);
				}
			}
			else if( ctx->type.dataType.IsFloatType() )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_fTOi64, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}
			else if( ctx->type.dataType.IsDoubleType() )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_dTOi64, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}
		}
		else if( to.IsUnsignedType() && to.GetSizeInMemoryDWords() == 1  )
		{
			if( ctx->type.dataType.IsIntegerType() ||
				ctx->type.dataType.IsUnsignedType() )
			{
				if( ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				{
					ctx->type.dataType.SetTokenType(to.GetTokenType());
					ctx->type.dataType.SetObjectType(to.GetObjectType());
				}
				else
				{
					ConvertToTempVariable(ctx);
					ReleaseTemporaryVariable(ctx->type, &ctx->bc);
					int offset = AllocateVariable(to, true);
					ctx->bc.InstrW_W(asBC_i64TOi, offset, ctx->type.stackOffset);
					ctx->type.SetVariable(to, offset, true);
				}
			}
			else if( ctx->type.dataType.IsFloatType() )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_fTOu, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}
			else if( ctx->type.dataType.IsDoubleType() )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_dTOu, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}

			// Convert to smaller integer if necessary
			int s = to.GetSizeInMemoryBytes();
			if( s < 4 )
			{
				ConvertToTempVariable(ctx);
				if( s == 1 )
					ctx->bc.InstrSHORT(asBC_iTOb, ctx->type.stackOffset);
				else if( s == 2 )
					ctx->bc.InstrSHORT(asBC_iTOw, ctx->type.stackOffset);
			}
		}
		else if( to.IsUnsignedType() && to.GetSizeInMemoryDWords() == 2 )
		{
			if( ctx->type.dataType.IsIntegerType() ||
				ctx->type.dataType.IsUnsignedType() )
			{
				if( ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
				{
					ctx->type.dataType.SetTokenType(to.GetTokenType());
					ctx->type.dataType.SetObjectType(to.GetObjectType());
				}
				else
				{
					ConvertToTempVariable(ctx);
					ReleaseTemporaryVariable(ctx->type, &ctx->bc);
					int offset = AllocateVariable(to, true);
					if( ctx->type.dataType.IsUnsignedType() )
						ctx->bc.InstrW_W(asBC_uTOi64, offset, ctx->type.stackOffset);
					else
						ctx->bc.InstrW_W(asBC_iTOi64, offset, ctx->type.stackOffset);
					ctx->type.SetVariable(to, offset, true);
				}
			}
			else if( ctx->type.dataType.IsFloatType() )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_fTOu64, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}
			else if( ctx->type.dataType.IsDoubleType() )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_dTOu64, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());

				if( convType != asIC_EXPLICIT_VAL_CAST )
					Warning(TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC, node);
			}
		}
		else if( to.IsFloatType() )
		{
			if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_iTOf, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());
			}
			else if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_i64TOf, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);
			}
			else if( ctx->type.dataType.IsUnsignedType() && ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_uTOf, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());
			}
			else if( ctx->type.dataType.IsUnsignedType() && ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_u64TOf, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);
			}
			else if( ctx->type.dataType.IsDoubleType() )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_dTOf, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);
			}
		}
		else if( to.IsDoubleType() )
		{
			if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_iTOd, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);
			}
			else if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_i64TOd, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());
			}
			else if( ctx->type.dataType.IsUnsignedType() && ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_uTOd, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);
			}
			else if( ctx->type.dataType.IsUnsignedType() && ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				ConvertToTempVariable(ctx);
				ctx->bc.InstrSHORT(asBC_u64TOd, ctx->type.stackOffset);
				ctx->type.dataType.SetTokenType(to.GetTokenType());
				ctx->type.dataType.SetObjectType(to.GetObjectType());
			}
			else if( ctx->type.dataType.IsFloatType() )
			{
				ConvertToTempVariable(ctx);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);
				int offset = AllocateVariable(to, true);
				ctx->bc.InstrW_W(asBC_fTOd, offset, ctx->type.stackOffset);
				ctx->type.SetVariable(to, offset, true);
			}
		}
	}
	else
	{
		if( ((to.IsIntegerType() && !to.IsEnumType()) || to.IsUnsignedType() ||
			 to.IsFloatType()   || to.IsDoubleType() ||
			 (to.IsEnumType() && convType == asIC_EXPLICIT_VAL_CAST)) &&
			(ctx->type.dataType.IsIntegerType() || ctx->type.dataType.IsUnsignedType() ||
			 ctx->type.dataType.IsFloatType()   || ctx->type.dataType.IsDoubleType()) )
		{
			ctx->type.dataType.SetTokenType(to.GetTokenType());
			ctx->type.dataType.SetObjectType(to.GetObjectType());
		}
	}

	// Primitive types on the stack, can be const or non-const
	ctx->type.dataType.MakeReadOnly(to.IsReadOnly());
	return cost;
}

asUINT asCCompiler::ImplicitConversion(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode, bool allowObjectConstruct)
{
	asASSERT( ctx->type.dataType.GetTokenType() != ttUnrecognizedToken ||
		      ctx->type.dataType.IsNullHandle() );

	// No conversion from void to any other type
	if( ctx->type.dataType.GetTokenType() == ttVoid )
		return asCC_NO_CONV;

	// Do we want a var type?
	if( to.GetTokenType() == ttQuestion )
	{
		// Any type can be converted to a var type, but only when not generating code
		asASSERT( !generateCode );

		ctx->type.dataType = to;

		return asCC_VARIABLE_CONV;
	}
	// Do we want a primitive?
	else if( to.IsPrimitive() )
	{
		if( !ctx->type.dataType.IsPrimitive() )
			return ImplicitConvObjectToPrimitive(ctx, to, node, convType, generateCode);
		else
			return ImplicitConvPrimitiveToPrimitive(ctx, to, node, convType, generateCode);
	}
	else // The target is a complex type
	{
		if( ctx->type.dataType.IsPrimitive() )
			return ImplicitConvPrimitiveToObject(ctx, to, node, convType, generateCode, allowObjectConstruct);
		else if( ctx->type.IsNullConstant() || ctx->type.dataType.GetObjectType() )
			return ImplicitConvObjectToObject(ctx, to, node, convType, generateCode, allowObjectConstruct);
	}

	return asCC_NO_CONV;
}

asUINT asCCompiler::ImplicitConvObjectToPrimitive(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode)
{
	if( ctx->type.isExplicitHandle )
	{
		// An explicit handle cannot be converted to a primitive
		if( convType != asIC_IMPLICIT_CONV && node )
		{
			asCString str;
			str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, ctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
			Error(str, node);
		}
		return asCC_NO_CONV;
	}

	// TODO: Must use the const cast behaviour if the object is read-only

	// Find matching value cast behaviours
	// Here we're only interested in those that convert the type to a primitive type
	asCArray<int> funcs;
	asSTypeBehaviour *beh = ctx->type.dataType.GetBehaviour();
	if( beh )
	{
		if( convType == asIC_EXPLICIT_VAL_CAST )
		{
			for( unsigned int n = 0; n < beh->operators.GetLength(); n += 2 )
			{
				// accept both implicit and explicit cast
				if( (beh->operators[n] == asBEHAVE_VALUE_CAST ||
					 beh->operators[n] == asBEHAVE_IMPLICIT_VALUE_CAST) &&
					builder->GetFunctionDescription(beh->operators[n+1])->returnType.IsPrimitive() )
					funcs.PushLast(beh->operators[n+1]);
			}
		}
		else
		{
			for( unsigned int n = 0; n < beh->operators.GetLength(); n += 2 )
			{
				// accept only implicit cast
				if( beh->operators[n] == asBEHAVE_IMPLICIT_VALUE_CAST &&
					builder->GetFunctionDescription(beh->operators[n+1])->returnType.IsPrimitive() )
					funcs.PushLast(beh->operators[n+1]);
			}
		}
	}

	// This matrix describes the priorities of the types to search for, for each target type
	// The first column is the target type, the priorities goes from left to right
	eTokenType matchMtx[10][10] =
	{
		{ttDouble, ttFloat,  ttInt64,  ttUInt64, ttInt,    ttUInt,   ttInt16,  ttUInt16, ttInt8,   ttUInt8},
		{ttFloat,  ttDouble, ttInt64,  ttUInt64, ttInt,    ttUInt,   ttInt16,  ttUInt16, ttInt8,   ttUInt8},
		{ttInt64,  ttUInt64, ttInt,    ttUInt,   ttInt16,  ttUInt16, ttInt8,   ttUInt8,  ttDouble, ttFloat},
		{ttUInt64, ttInt64,  ttUInt,   ttInt,    ttUInt16, ttInt16,  ttUInt8,  ttInt8,   ttDouble, ttFloat},
		{ttInt,    ttUInt,   ttInt64,  ttUInt64, ttInt16,  ttUInt16, ttInt8,   ttUInt8,  ttDouble, ttFloat},
		{ttUInt,   ttInt,    ttUInt64, ttInt64,  ttUInt16, ttInt16,  ttUInt8,  ttInt8,   ttDouble, ttFloat},
		{ttInt16,  ttUInt16, ttInt,    ttUInt,   ttInt64,  ttUInt64, ttInt8,   ttUInt8,  ttDouble, ttFloat},
		{ttUInt16, ttInt16,  ttUInt,   ttInt,    ttUInt64, ttInt64,  ttUInt8,  ttInt8,   ttDouble, ttFloat},
		{ttInt8,   ttUInt8,  ttInt16,  ttUInt16, ttInt,    ttUInt,   ttInt64,  ttUInt64, ttDouble, ttFloat},
		{ttUInt8,  ttInt8,   ttUInt16, ttInt16,  ttUInt,   ttInt,    ttUInt64, ttInt64,  ttDouble, ttFloat},
	};

	// Which row to use?
	eTokenType *row = 0;
	for( unsigned int type = 0; type < 10; type++ )
	{
		if( to.GetTokenType() == matchMtx[type][0] )
		{
			row = &matchMtx[type][0];
			break;
		}
	}

	// Find the best matching cast operator
	int funcId = 0;
	if( row )
	{
		asCDataType target(to);

		// Priority goes from left to right in the matrix
		for( unsigned int attempt = 0; attempt < 10 && funcId == 0; attempt++ )
		{
			target.SetTokenType(row[attempt]);
			for( unsigned int n = 0; n < funcs.GetLength(); n++ )
			{
				asCScriptFunction *descr = builder->GetFunctionDescription(funcs[n]);
				if( descr->returnType.IsEqualExceptRefAndConst(target) )
				{
					funcId = funcs[n];
					break;
				}
			}
		}
	}

	// Did we find a suitable function?
	if( funcId != 0 )
	{
		asCScriptFunction *descr = builder->GetFunctionDescription(funcId);
		if( generateCode )
		{
			asCTypeInfo objType = ctx->type;

			Dereference(ctx, true);

			PerformFunctionCall(funcId, ctx);

			ReleaseTemporaryVariable(objType, &ctx->bc);
		}
		else
			ctx->type.Set(descr->returnType);

		// Allow one more implicit conversion to another primitive type
		return asCC_OBJ_TO_PRIMITIVE_CONV + ImplicitConversion(ctx, to, node, convType, generateCode, false);
	}
	else
	{
		if( convType != asIC_IMPLICIT_CONV && node )
		{
			asCString str;
			str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, ctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
			Error(str, node);
		}
	}

	return asCC_NO_CONV;
}


asUINT asCCompiler::ImplicitConvObjectRef(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode)
{
	// Convert null to any object type handle, but not to a non-handle type
	if( ctx->type.IsNullConstant() && ctx->methodName == "" )
	{
		if( to.IsObjectHandle() )
		{
			ctx->type.dataType = to;
			return asCC_REF_CONV;
		}
		return asCC_NO_CONV;
	}

	asASSERT(ctx->type.dataType.GetObjectType() || ctx->methodName != "");

	// First attempt to convert the base type without instanciating another instance
	if( to.GetObjectType() != ctx->type.dataType.GetObjectType() && ctx->methodName == "" )
	{
		// If the to type is an interface and the from type implements it, then we can convert it immediately
		if( ctx->type.dataType.GetObjectType()->Implements(to.GetObjectType()) )
		{
			ctx->type.dataType.SetObjectType(to.GetObjectType());
			return asCC_REF_CONV;
		}
		// If the to type is a class and the from type derives from it, then we can convert it immediately
		else if( ctx->type.dataType.GetObjectType()->DerivesFrom(to.GetObjectType()) )
		{
			ctx->type.dataType.SetObjectType(to.GetObjectType());
			return asCC_REF_CONV;
		}
		// If the types are not equal yet, then we may still be able to find a reference cast
		else if( ctx->type.dataType.GetObjectType() != to.GetObjectType() )
		{
			// A ref cast must not remove the constness
			bool isConst = false;
			if( (ctx->type.dataType.IsObjectHandle() && ctx->type.dataType.IsHandleToConst()) ||
				(!ctx->type.dataType.IsObjectHandle() && ctx->type.dataType.IsReadOnly()) )
				isConst = true;

			// We may still be able to find an implicit ref cast behaviour
			CompileRefCast(ctx, to, convType == asIC_EXPLICIT_REF_CAST, node, generateCode);

			ctx->type.dataType.MakeHandleToConst(isConst);

			// Was the conversion done?
			if( ctx->type.dataType.GetObjectType() == to.GetObjectType() )
				return asCC_REF_CONV;
		}
	}

	// Convert matching function types
	if( to.GetFuncDef() )
	{
		// If the input expression is already a funcdef, check if it can be converted
		if( ctx->type.dataType.GetFuncDef() &&
			to.GetFuncDef() != ctx->type.dataType.GetFuncDef() )
		{
			asCScriptFunction *toFunc = to.GetFuncDef();
			asCScriptFunction *fromFunc = ctx->type.dataType.GetFuncDef();
			if( toFunc->IsSignatureExceptNameEqual(fromFunc) )
			{
				ctx->type.dataType.SetFuncDef(toFunc);
				return asCC_REF_CONV;
			}
		}

		// If the input expression is a deferred function ref, check if there is a matching func
		if( ctx->methodName != "" )
		{
			// Determine the namespace
			asSNameSpace *ns = 0;
			asCString name = "";
			int pos = ctx->methodName.FindLast("::");
			if( pos >= 0 )
			{
				asCString nsName = ctx->methodName.SubString(0, pos+2);
				// Trim off the last ::
				if( nsName.GetLength() > 2 ) 
					nsName.SetLength(nsName.GetLength()-2);
				ns = DetermineNameSpace(nsName);
				name = ctx->methodName.SubString(pos+2);
			}
			else
			{
				DetermineNameSpace("");
				name = ctx->methodName;
			}

			asCArray<int> funcs;
			if( ns )
				builder->GetFunctionDescriptions(name.AddressOf(), funcs, ns);

			// Check if any of the functions have perfect match
			for( asUINT n = 0; n < funcs.GetLength(); n++ )
			{
				asCScriptFunction *func = builder->GetFunctionDescription(funcs[n]);
				if( to.GetFuncDef()->IsSignatureExceptNameEqual(func) )
				{
					if( generateCode )
					{
						ctx->bc.InstrPTR(asBC_FuncPtr, func);

						// Make sure the identified function is shared if we're compiling a shared function
						if( !func->IsShared() && outFunc->IsShared() )
						{
							asCString msg;
							msg.Format(TXT_SHARED_CANNOT_CALL_NON_SHARED_FUNC_s, func->GetDeclaration());
							Error(msg, node);
						}
					}

					ctx->type.dataType = asCDataType::CreateFuncDef(to.GetFuncDef());
					return asCC_REF_CONV;
				}
			}
		}
	}

	return asCC_NO_CONV;
}

asUINT asCCompiler::ImplicitConvObjectValue(asSExprContext *ctx, const asCDataType &to, asCScriptNode * /*node*/, EImplicitConv convType, bool generateCode)
{
	asUINT cost = asCC_NO_CONV;

	// If the base type is still different, and we are allowed to instance
	// another object then we can try an implicit value cast
	if( to.GetObjectType() != ctx->type.dataType.GetObjectType() )
	{
		// TODO: Implement support for implicit constructor/factory

		asCArray<int> funcs;
		asSTypeBehaviour *beh = ctx->type.dataType.GetBehaviour();
		if( beh )
		{
			if( convType == asIC_EXPLICIT_VAL_CAST )
			{
				for( unsigned int n = 0; n < beh->operators.GetLength(); n += 2 )
				{
					// accept both implicit and explicit cast
					if( (beh->operators[n] == asBEHAVE_VALUE_CAST ||
						 beh->operators[n] == asBEHAVE_IMPLICIT_VALUE_CAST) &&
						builder->GetFunctionDescription(beh->operators[n+1])->returnType.GetObjectType() == to.GetObjectType() )
						funcs.PushLast(beh->operators[n+1]);
				}
			}
			else
			{
				for( unsigned int n = 0; n < beh->operators.GetLength(); n += 2 )
				{
					// accept only implicit cast
					if( beh->operators[n] == asBEHAVE_IMPLICIT_VALUE_CAST &&
						builder->GetFunctionDescription(beh->operators[n+1])->returnType.GetObjectType() == to.GetObjectType() )
						funcs.PushLast(beh->operators[n+1]);
				}
			}
		}

		// TODO: If there are multiple valid value casts, then we must choose the most appropriate one
		asASSERT( funcs.GetLength() <= 1 );

		if( funcs.GetLength() == 1 )
		{
			asCScriptFunction *f = builder->GetFunctionDescription(funcs[0]);
			if( generateCode )
			{
				asCTypeInfo objType = ctx->type;
				Dereference(ctx, true);

				bool useVariable = false;
				int  stackOffset = 0;

				if( f->DoesReturnOnStack() )
				{
					useVariable = true;
					stackOffset = AllocateVariable(f->returnType, true);

					// Push the pointer to the pre-allocated space for the return value
					ctx->bc.InstrSHORT(asBC_PSF, short(stackOffset));

					// The object pointer is already on the stack, but should be the top
					// one, so we need to swap the pointers in order to get the correct
					ctx->bc.Instr(asBC_SwapPtr);
				}

				PerformFunctionCall(funcs[0], ctx, false, 0, 0, useVariable, stackOffset);
				ReleaseTemporaryVariable(objType, &ctx->bc);
			}
			else
				ctx->type.Set(f->returnType);

			cost = asCC_TO_OBJECT_CONV;
		}
	}

	return cost;
}

asUINT asCCompiler::ImplicitConvObjectToObject(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode, bool allowObjectConstruct)
{
	// First try a ref cast
	asUINT cost = ImplicitConvObjectRef(ctx, to, node, convType, generateCode);

	// If the desired type is an asOBJ_ASHANDLE then we'll assume it is allowed to implicitly
	// construct the object through any of the available constructors
	if( to.GetObjectType() && (to.GetObjectType()->flags & asOBJ_ASHANDLE) && to.GetObjectType() != ctx->type.dataType.GetObjectType() && allowObjectConstruct )
	{
		asCArray<int> funcs;
		funcs = to.GetObjectType()->beh.constructors;

		asCArray<asSExprContext *> args;
		args.PushLast(ctx);

		cost = asCC_TO_OBJECT_CONV + MatchFunctions(funcs, args, node, 0, 0, false, true, false);

		// Did we find a matching constructor?
		if( funcs.GetLength() == 1 )
		{
			if( generateCode )
			{
				// If the ASHANDLE receives a variable type parameter, then we need to 
				// make sure the expression is treated as a handle and not as a value
				asCScriptFunction *func = engine->scriptFunctions[funcs[0]];
				if( func->parameterTypes[0].GetTokenType() == ttQuestion )
				{
					if( !ctx->type.isExplicitHandle )
					{
						asCDataType toHandle = ctx->type.dataType;
						toHandle.MakeHandle(true);
						toHandle.MakeReference(true);
						toHandle.MakeHandleToConst(ctx->type.dataType.IsReadOnly());
						ImplicitConversion(ctx, toHandle, node, asIC_IMPLICIT_CONV, true, false);

						asASSERT( ctx->type.dataType.IsObjectHandle() );
					}
					ctx->type.isExplicitHandle = true;
				}

				// TODO: This should really reuse the code from CompileConstructCall

				// Allocate the new object
				asCTypeInfo tempObj;
				tempObj.dataType = to;
				tempObj.dataType.MakeReference(false);
				tempObj.stackOffset = (short)AllocateVariable(tempObj.dataType, true);
				tempObj.dataType.MakeReference(true);
				tempObj.isTemporary = true;
				tempObj.isVariable = true;

				bool onHeap = IsVariableOnHeap(tempObj.stackOffset);

				// Push the address of the object on the stack
				asSExprContext e(engine);
				if( onHeap )
					e.bc.InstrSHORT(asBC_VAR, tempObj.stackOffset);

				PrepareFunctionCall(funcs[0], &e.bc, args);
				MoveArgsToStack(funcs[0], &e.bc, args, false);

				// If the object is allocated on the stack, then call the constructor as a normal function
				if( onHeap )
				{
					int offset = 0;
					asCScriptFunction *descr = builder->GetFunctionDescription(funcs[0]);
					offset = descr->parameterTypes[0].GetSizeOnStackDWords();

					e.bc.InstrWORD(asBC_GETREF, (asWORD)offset);
				}
				else
					e.bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);

				PerformFunctionCall(funcs[0], &e, onHeap, &args, tempObj.dataType.GetObjectType());

				// Add tag that the object has been initialized
				e.bc.ObjInfo(tempObj.stackOffset, asOBJ_INIT);

				// The constructor doesn't return anything,
				// so we have to manually inform the type of
				// the return value
				e.type = tempObj;
				if( !onHeap )
					e.type.dataType.MakeReference(false);

				// Push the address of the object on the stack again
				e.bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);

				MergeExprBytecodeAndType(ctx, &e);
			}
			else
			{
				ctx->type.Set(asCDataType::CreateObject(to.GetObjectType(), false));
			}
		}
	}

	// If the base type is still different, and we are allowed to instance
	// another object then we can try an implicit value cast
	if( to.GetObjectType() != ctx->type.dataType.GetObjectType() && allowObjectConstruct )
	{
		// Attempt implicit value cast
		cost = ImplicitConvObjectValue(ctx, to, node, convType, generateCode);
	}

	// If we still haven't converted the base type to the correct type, then there is
	//  no need to continue as it is not possible to do the conversion
	if( to.GetObjectType() != ctx->type.dataType.GetObjectType() )
		return asCC_NO_CONV;


	if( to.IsObjectHandle() )
	{
		// There is no extra cost in converting to a handle

		// reference to handle -> handle
		// reference           -> handle
		// object              -> handle
		// handle              -> reference to handle
		// reference           -> reference to handle
		// object              -> reference to handle

		// TODO: If the type is handle, then we can't use IsReadOnly to determine the constness of the basetype

		// If the rvalue is a handle to a const object, then
		// the lvalue must also be a handle to a const object
		if( ctx->type.dataType.IsReadOnly() && !to.IsReadOnly() )
		{
			if( convType != asIC_IMPLICIT_CONV )
			{
				asASSERT(node);
				asCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, ctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
				Error(str, node);
			}
		}

		if( !ctx->type.dataType.IsObjectHandle() )
		{
			// An object type can be directly converted to a handle of the
			// same type by doing a ref copy to a new variable
			if( ctx->type.dataType.SupportHandles() )
			{
				asCDataType dt = ctx->type.dataType;
				dt.MakeHandle(true);
				dt.MakeReference(false);

				if( generateCode )
				{
					// If the expression is already a local variable, then it is not
					// necessary to do a ref copy, as the ref objects on the stack are
					// really handles, only the handles cannot be modified.
					if( ctx->type.isVariable )
					{
						bool isHandleToConst = ctx->type.dataType.IsReadOnly();
						ctx->type.dataType.MakeReadOnly(false);
						ctx->type.dataType.MakeHandle(true);
						ctx->type.dataType.MakeReadOnly(true);
						ctx->type.dataType.MakeHandleToConst(isHandleToConst);

						if( to.IsReference() && !ctx->type.dataType.IsReference() )
						{
							ctx->bc.Instr(asBC_PopPtr);
							ctx->bc.InstrSHORT(asBC_PSF, ctx->type.stackOffset);
							ctx->type.dataType.MakeReference(true);
						}
						else if( ctx->type.dataType.IsReference() )
						{
							ctx->bc.Instr(asBC_RDSPtr);
							ctx->type.dataType.MakeReference(false);
						}
					}
					else
					{
						int offset = AllocateVariable(dt, true);

						if( ctx->type.dataType.IsReference() )
							ctx->bc.Instr(asBC_RDSPtr);
						ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
						ctx->bc.InstrPTR(asBC_REFCPY, dt.GetObjectType());
						ctx->bc.Instr(asBC_PopPtr);
						ctx->bc.InstrSHORT(asBC_PSF, (short)offset);

						ReleaseTemporaryVariable(ctx->type, &ctx->bc);

						if( to.IsReference() )
							dt.MakeReference(true);
						else
							ctx->bc.Instr(asBC_RDSPtr);

						ctx->type.SetVariable(dt, offset, true);
					}
				}
				else
					ctx->type.dataType = dt;

				// When this conversion is done the expression is no longer an lvalue
				ctx->type.isLValue = false;
			}
		}

		if( ctx->type.dataType.IsObjectHandle() )
		{
			// A handle to non-const can be converted to a
			// handle to const, but not the other way
			if( to.IsHandleToConst() )
				ctx->type.dataType.MakeHandleToConst(true);

			// A const handle can be converted to a non-const
			// handle and vice versa as the handle is just a value
			ctx->type.dataType.MakeReadOnly(to.IsReadOnly());
		}

		if( to.IsReference() && !ctx->type.dataType.IsReference() )
		{
			if( generateCode )
			{
				asASSERT( ctx->type.dataType.IsObjectHandle() );

				// If the input type is a handle, then a simple ref copy is enough
				bool isExplicitHandle = ctx->type.isExplicitHandle;
				ctx->type.isExplicitHandle = ctx->type.dataType.IsObjectHandle();

				// If the input type is read-only we'll need to temporarily
				// remove this constness, otherwise the assignment will fail
				bool typeIsReadOnly = ctx->type.dataType.IsReadOnly();
				ctx->type.dataType.MakeReadOnly(false);

				// If the object already is a temporary variable, then the copy
				// doesn't have to be made as it is already a unique object
				PrepareTemporaryObject(node, ctx);

				ctx->type.dataType.MakeReadOnly(typeIsReadOnly);
				ctx->type.isExplicitHandle = isExplicitHandle;
			}

			// A non-reference can be converted to a reference,
			// by putting the value in a temporary variable
			ctx->type.dataType.MakeReference(true);

			// Since it is a new temporary variable it doesn't have to be const
			ctx->type.dataType.MakeReadOnly(to.IsReadOnly());
		}
		else if( !to.IsReference() && ctx->type.dataType.IsReference() )
		{
			Dereference(ctx, generateCode);
		}
	}
	else // if( !to.IsObjectHandle() )
	{
		if( !to.IsReference() )
		{
			// reference to handle -> object
			// handle              -> object
			// reference           -> object

			// An implicit handle can be converted to an object by adding a check for null pointer
			if( ctx->type.dataType.IsObjectHandle() && !ctx->type.isExplicitHandle )
			{
				if( generateCode )
				{
					if( ctx->type.dataType.IsReference() )
					{
						// The pointer on the stack refers to the handle
						ctx->bc.Instr(asBC_ChkRefS);
					}
					else
					{
						// The pointer on the stack refers to the object
						ctx->bc.Instr(asBC_CHKREF);
					}
				}

				ctx->type.dataType.MakeHandle(false);
			}

			// A const object can be converted to a non-const object through a copy
			if( ctx->type.dataType.IsReadOnly() && !to.IsReadOnly() &&
				allowObjectConstruct )
			{
				// Does the object type allow a copy to be made?
				if( ctx->type.dataType.CanBeCopied() )
				{
					if( generateCode )
					{
						// Make a temporary object with the copy
						PrepareTemporaryObject(node, ctx);
					}

					// In case the object was already in a temporary variable, then the function
					// didn't really do anything so we need to remove the constness here
					ctx->type.dataType.MakeReadOnly(false);

					// Add the cost for the copy
					cost += asCC_TO_OBJECT_CONV;
				}
			}

			if( ctx->type.dataType.IsReference() )
			{
				// This may look strange, but a value type allocated on the stack is already
				// correct, so nothing should be done other than remove the mark as reference.
				// For types allocated on the heap, it is necessary to dereference the pointer
				// that is currently on the stack
				if( IsVariableOnHeap(ctx->type.stackOffset) )
					Dereference(ctx, generateCode);
				else
					ctx->type.dataType.MakeReference(false);
			}

			// A non-const object can be converted to a const object directly
			if( !ctx->type.dataType.IsReadOnly() && to.IsReadOnly() )
			{
				ctx->type.dataType.MakeReadOnly(true);
			}
		}
		else // if( to.IsReference() )
		{
			// reference to handle -> reference
			// handle              -> reference
			// object              -> reference

			if( ctx->type.dataType.IsReference() )
			{
				if( ctx->type.isExplicitHandle && ctx->type.dataType.GetObjectType() && (ctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE) )
				{
					// ASHANDLE objects are really value types, so explicit handle can be removed
					ctx->type.isExplicitHandle = false;
					ctx->type.dataType.MakeHandle(false);
				}

				// A reference to a handle can be converted to a reference to an object
				// by first reading the address, then verifying that it is not null
				if( !to.IsObjectHandle() && ctx->type.dataType.IsObjectHandle() && !ctx->type.isExplicitHandle )
				{
					ctx->type.dataType.MakeHandle(false);
					if( generateCode )
						ctx->bc.Instr(asBC_ChkRefS);
				}

				// A reference to a non-const can be converted to a reference to a const
				if( to.IsReadOnly() )
					ctx->type.dataType.MakeReadOnly(true);
				else if( ctx->type.dataType.IsReadOnly() )
				{
					// A reference to a const can be converted to a reference to a
					// non-const by copying the object to a temporary variable
					ctx->type.dataType.MakeReadOnly(false);

					if( generateCode )
					{
						// If the object already is a temporary variable, then the copy
						// doesn't have to be made as it is already a unique object
						PrepareTemporaryObject(node, ctx);
					}

					// Add the cost for the copy
					cost += asCC_TO_OBJECT_CONV;
				}
			}
			else // if( !ctx->type.dataType.IsReference() )
			{
				// A non-reference handle can be converted to a non-handle reference by checking against null handle
				if( ctx->type.dataType.IsObjectHandle() )
				{
					bool readOnly = false;
					if( ctx->type.dataType.IsHandleToConst() )
						readOnly = true;

					if( generateCode )
					{
						if( ctx->type.isVariable )
							ctx->bc.InstrSHORT(asBC_ChkNullV, ctx->type.stackOffset);
						else
							ctx->bc.Instr(asBC_CHKREF);
					}
					ctx->type.dataType.MakeHandle(false);
					ctx->type.dataType.MakeReference(true);

					// Make sure a handle to const isn't converted to non-const reference
					if( readOnly )
						ctx->type.dataType.MakeReadOnly(true);
				}
				else
				{
					// A value type allocated on the stack is differentiated
					// by it not being a reference. But it can be handled as
					// reference by pushing the pointer on the stack
					if( (ctx->type.dataType.GetObjectType()->GetFlags() & asOBJ_VALUE) &&
						(ctx->type.isVariable || ctx->type.isTemporary) &&
						!IsVariableOnHeap(ctx->type.stackOffset) )
					{
						// Actually the pointer is already pushed on the stack in
						// CompileVariableAccess, so we don't need to do anything else
					}
					else if( generateCode )
					{
						// A non-reference can be converted to a reference,
						// by putting the value in a temporary variable

						// If the input type is read-only we'll need to temporarily
						// remove this constness, otherwise the assignment will fail
						bool typeIsReadOnly = ctx->type.dataType.IsReadOnly();
						ctx->type.dataType.MakeReadOnly(false);

						// If the object already is a temporary variable, then the copy
						// doesn't have to be made as it is already a unique object
						PrepareTemporaryObject(node, ctx);

						ctx->type.dataType.MakeReadOnly(typeIsReadOnly);

						// Add the cost for the copy
						cost += asCC_TO_OBJECT_CONV;
					}

					// This may look strange as the conversion was to make the expression a reference
					// but a value type allocated on the stack is a reference even without the type
					// being marked as such.
					ctx->type.dataType.MakeReference(IsVariableOnHeap(ctx->type.stackOffset));
				}

				// TODO: If the variable is an object allocated on the stack the following is not true as the copy may not have been made
				// Since it is a new temporary variable it doesn't have to be const
				ctx->type.dataType.MakeReadOnly(to.IsReadOnly());
			}
		}
	}

	return cost;
}

asUINT asCCompiler::ImplicitConvPrimitiveToObject(asSExprContext *ctx, const asCDataType &to, asCScriptNode * /*node*/, EImplicitConv /*isExplicit*/, bool generateCode, bool /*allowObjectConstruct*/)
{
	// Reference types currently don't allow implicit conversion from primitive to object
	// TODO: Allow implicit conversion to scoped reference types as they are supposed to appear like ordinary value types
	asCObjectType *objType = to.GetObjectType();
	asASSERT( objType );
	if( !objType || (objType->flags & asOBJ_REF) )
		return asCC_NO_CONV;

	// For value types the object must have a constructor that takes a single primitive argument either by value or as input reference
	asCArray<int> funcs;
	for( asUINT n = 0; n < objType->beh.constructors.GetLength(); n++ )
	{
		asCScriptFunction *func = engine->scriptFunctions[objType->beh.constructors[n]];
		if( func->parameterTypes.GetLength() == 1 &&
			func->parameterTypes[0].IsPrimitive() &&
			!(func->inOutFlags[0] & asTM_OUTREF) )
		{
			funcs.PushLast(func->id);
		}
	}

	if( funcs.GetLength() == 0 )
		return asCC_NO_CONV;

	// Check if it is possible to choose a best match
	asSExprContext arg(engine);
	arg.type = ctx->type;
	arg.exprNode = ctx->exprNode; // Use the same node for compiler messages
	asCArray<asSExprContext*> args;
	args.PushLast(&arg);
	asUINT cost = asCC_TO_OBJECT_CONV + MatchFunctions(funcs, args, 0, 0, objType, false, true, false);
	if( funcs.GetLength() != 1 )
		return asCC_NO_CONV;

	if( !generateCode )
	{
		ctx->type.Set(to);
		return cost;
	}

	// TODO: clean up: This part is similar to CompileConstructCall(). It should be put in a common function

	bool onHeap = true;

	// Value types and script types are allocated through the constructor
	asCTypeInfo tempObj;
	tempObj.dataType = to;
	tempObj.stackOffset = (short)AllocateVariable(to, true);
	tempObj.dataType.MakeReference(true);
	tempObj.isTemporary = true;
	tempObj.isVariable = true;

	onHeap = IsVariableOnHeap(tempObj.stackOffset);

	// Push the address of the object on the stack
	if( onHeap )
		ctx->bc.InstrSHORT(asBC_VAR, tempObj.stackOffset);

	PrepareFunctionCall(funcs[0], &ctx->bc, args);
	MoveArgsToStack(funcs[0], &ctx->bc, args, false);

	if( !(objType->flags & asOBJ_REF) )
	{
		// If the object is allocated on the stack, then call the constructor as a normal function
		if( onHeap )
		{
			int offset = 0;
			asCScriptFunction *descr = builder->GetFunctionDescription(funcs[0]);
			for( asUINT n = 0; n < args.GetLength(); n++ )
				offset += descr->parameterTypes[n].GetSizeOnStackDWords();

			ctx->bc.InstrWORD(asBC_GETREF, (asWORD)offset);
		}
		else
			ctx->bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);

		PerformFunctionCall(funcs[0], ctx, onHeap, &args, tempObj.dataType.GetObjectType());

		// Add tag that the object has been initialized
		ctx->bc.ObjInfo(tempObj.stackOffset, asOBJ_INIT);

		// The constructor doesn't return anything,
		// so we have to manually inform the type of
		// the return value
		ctx->type = tempObj;
		if( !onHeap )
			ctx->type.dataType.MakeReference(false);

		// Push the address of the object on the stack again
		ctx->bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);
	}
	else
	{
		asASSERT( objType->flags & asOBJ_SCOPED );

		// Call the factory to create the reference type
		PerformFunctionCall(funcs[0], ctx, false, &args);
	}

	return cost;
}

void asCCompiler::ImplicitConversionConstant(asSExprContext *from, const asCDataType &to, asCScriptNode *node, EImplicitConv convType)
{
	asASSERT(from->type.isConstant);

	// TODO: node should be the node of the value that is
	//       converted (not the operator that provokes the implicit
	//       conversion)

	// If the base type is correct there is no more to do
	if( to.IsEqualExceptRefAndConst(from->type.dataType) ) return;

	// References cannot be constants
	if( from->type.dataType.IsReference() ) return;

	if( (to.IsIntegerType() && to.GetSizeInMemoryDWords() == 1 && !to.IsEnumType()) ||
		(to.IsEnumType() && convType == asIC_EXPLICIT_VAL_CAST) )
	{
		if( from->type.dataType.IsFloatType() ||
			from->type.dataType.IsDoubleType() ||
			from->type.dataType.IsUnsignedType() ||
			from->type.dataType.IsIntegerType() )
		{
			// Transform the value
			// Float constants can be implicitly converted to int
			if( from->type.dataType.IsFloatType() )
			{
				float fc = from->type.floatValue;
				int ic = int(fc);

				if( float(ic) != fc )
				{
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
				}

				from->type.intValue = ic;
			}
			// Double constants can be implicitly converted to int
			else if( from->type.dataType.IsDoubleType() )
			{
				double fc = from->type.doubleValue;
				int ic = int(fc);

				if( double(ic) != fc )
				{
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
				}

				from->type.intValue = ic;
			}
			else if( from->type.dataType.IsUnsignedType() && from->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				// Verify that it is possible to convert to signed without getting negative
				if( from->type.intValue < 0 )
				{
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_CHANGE_SIGN, node);
				}

				// Convert to 32bit
				if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
					from->type.intValue = from->type.byteValue;
				else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
					from->type.intValue = from->type.wordValue;
			}
			else if( from->type.dataType.IsUnsignedType() && from->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				// Convert to 32bit
				from->type.intValue = int(from->type.qwordValue);
			}
			else if( from->type.dataType.IsIntegerType() &&
					from->type.dataType.GetSizeInMemoryBytes() < 4 )
			{
				// Convert to 32bit
				if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
					from->type.intValue = (signed char)from->type.byteValue;
				else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
					from->type.intValue = (short)from->type.wordValue;
			}

			// Set the resulting type
			if( to.IsEnumType() )
				from->type.dataType = to;
			else
				from->type.dataType = asCDataType::CreatePrimitive(ttInt, true);
		}

		// Check if a downsize is necessary
		if( to.IsIntegerType() &&
			from->type.dataType.IsIntegerType() &&
		    from->type.dataType.GetSizeInMemoryBytes() > to.GetSizeInMemoryBytes() )
		{
			// Verify if it is possible
			if( to.GetSizeInMemoryBytes() == 1 )
			{
				if( char(from->type.intValue) != from->type.intValue )
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);

				from->type.byteValue = char(from->type.intValue);
			}
			else if( to.GetSizeInMemoryBytes() == 2 )
			{
				if( short(from->type.intValue) != from->type.intValue )
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);

				from->type.wordValue = short(from->type.intValue);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
		}
	}
	else if( to.IsIntegerType() && to.GetSizeInMemoryDWords() == 2 )
	{
		// Float constants can be implicitly converted to int
		if( from->type.dataType.IsFloatType() )
		{
			float fc = from->type.floatValue;
			asINT64 ic = asINT64(fc);

			if( float(ic) != fc )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttInt64, true);
			from->type.qwordValue = ic;
		}
		// Double constants can be implicitly converted to int
		else if( from->type.dataType.IsDoubleType() )
		{
			double fc = from->type.doubleValue;
			asINT64 ic = asINT64(fc);

			if( double(ic) != fc )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttInt64, true);
			from->type.qwordValue = ic;
		}
		else if( from->type.dataType.IsUnsignedType() )
		{
			// Convert to 64bit
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				from->type.qwordValue = from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				from->type.qwordValue = from->type.wordValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 4 )
				from->type.qwordValue = from->type.dwordValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 8 )
			{
				if( asINT64(from->type.qwordValue) < 0 )
				{
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_CHANGE_SIGN, node);
				}
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttInt64, true);
		}
		else if( from->type.dataType.IsIntegerType() )
		{
			// Convert to 64bit
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				from->type.qwordValue = (signed char)from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				from->type.qwordValue = (short)from->type.wordValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 4 )
				from->type.qwordValue = from->type.intValue;

			from->type.dataType = asCDataType::CreatePrimitive(ttInt64, true);
		}
	}
	else if( to.IsUnsignedType() && to.GetSizeInMemoryDWords() == 1 )
	{
		if( from->type.dataType.IsFloatType() )
		{
			float fc = from->type.floatValue;
			// Some compilers set the value to 0 when converting a negative float to unsigned int.
			// To maintain a consistent behaviour across compilers we convert to int first.
			asUINT uic = asUINT(int(fc));

			if( float(uic) != fc )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt, true);
			from->type.intValue = uic;

			// Try once more, in case of a smaller type
			ImplicitConversionConstant(from, to, node, convType);
		}
		else if( from->type.dataType.IsDoubleType() )
		{
			double fc = from->type.doubleValue;
			// Some compilers set the value to 0 when converting a negative double to unsigned int.
			// To maintain a consistent behaviour across compilers we convert to int first.
			asUINT uic = asUINT(int(fc));

			if( double(uic) != fc )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt, true);
			from->type.intValue = uic;

			// Try once more, in case of a smaller type
			ImplicitConversionConstant(from, to, node, convType);
		}
		else if( from->type.dataType.IsIntegerType() )
		{
			// Verify that it is possible to convert to unsigned without loosing negative
			if( (from->type.dataType.GetSizeInMemoryBytes() > 4 && asINT64(from->type.qwordValue) < 0) || 
				(from->type.dataType.GetSizeInMemoryBytes() <= 4 && from->type.intValue < 0) )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_CHANGE_SIGN, node);
			}

			// Check if any data is lost
			if( from->type.dataType.GetSizeInMemoryBytes() > 4 && (from->type.qwordValue >> 32) != 0 && (from->type.qwordValue >> 32) != 0xFFFFFFFF )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);
			}

			// Convert to 32bit
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				from->type.intValue = (signed char)from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				from->type.intValue = (short)from->type.wordValue;

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt, true);

			// Try once more, in case of a smaller type
			ImplicitConversionConstant(from, to, node, convType);
		}
		else if( from->type.dataType.IsUnsignedType() &&
		         from->type.dataType.GetSizeInMemoryBytes() < 4 )
		{
			// Convert to 32bit
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				from->type.dwordValue = from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				from->type.dwordValue = from->type.wordValue;

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt, true);

			// Try once more, in case of a smaller type
			ImplicitConversionConstant(from, to, node, convType);
		}
		else if( from->type.dataType.IsUnsignedType() &&
		         from->type.dataType.GetSizeInMemoryBytes() > to.GetSizeInMemoryBytes() )
		{
			// Verify if it is possible
			if( to.GetSizeInMemoryBytes() == 1 )
			{
				if( asBYTE(from->type.dwordValue) != from->type.dwordValue )
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);

				from->type.byteValue = asBYTE(from->type.dwordValue);
			}
			else if( to.GetSizeInMemoryBytes() == 2 )
			{
				if( asWORD(from->type.dwordValue) != from->type.dwordValue )
					if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);

				from->type.wordValue = asWORD(from->type.dwordValue);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
		}
	}
	else if( to.IsUnsignedType() && to.GetSizeInMemoryDWords() == 2 )
	{
		if( from->type.dataType.IsFloatType() )
		{
			float fc = from->type.floatValue;
			// Convert first to int64 then to uint64 to avoid negative float becoming 0 on gnuc base compilers
			asQWORD uic = asQWORD(asINT64(fc));

#if !defined(_MSC_VER) || _MSC_VER > 1200 // MSVC++ 6
			// MSVC6 doesn't support this conversion
			if( float(uic) != fc )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}
#endif

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt64, true);
			from->type.qwordValue = uic;
		}
		else if( from->type.dataType.IsDoubleType() )
		{
			double fc = from->type.doubleValue;
			// Convert first to int64 then to uint64 to avoid negative float becoming 0 on gnuc base compilers
			asQWORD uic = asQWORD(asINT64(fc));

#if !defined(_MSC_VER) || _MSC_VER > 1200 // MSVC++ 6
			// MSVC6 doesn't support this conversion
			if( double(uic) != fc )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}
#endif

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt64, true);
			from->type.qwordValue = uic;
		}
		else if( from->type.dataType.IsIntegerType() && from->type.dataType.GetSizeInMemoryDWords() == 1 )
		{
			// Convert to 64bit
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				from->type.qwordValue = (asINT64)(signed char)from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				from->type.qwordValue = (asINT64)(short)from->type.wordValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 4 )
				from->type.qwordValue = (asINT64)from->type.intValue;

			// Verify that it is possible to convert to unsigned without loosing negative
			if( asINT64(from->type.qwordValue) < 0 )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_CHANGE_SIGN, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt64, true);
		}
		else if( from->type.dataType.IsIntegerType() && from->type.dataType.GetSizeInMemoryDWords() == 2 )
		{
			// Verify that it is possible to convert to unsigned without loosing negative
			if( asINT64(from->type.qwordValue) < 0 )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_CHANGE_SIGN, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt64, true);
		}
		else if( from->type.dataType.IsUnsignedType() )
		{
			// Convert to 64bit
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				from->type.qwordValue = from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				from->type.qwordValue = from->type.wordValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 4 )
				from->type.qwordValue = from->type.dwordValue;

			from->type.dataType = asCDataType::CreatePrimitive(ttUInt64, true);
		}
	}
	else if( to.IsFloatType() )
	{
		if( from->type.dataType.IsDoubleType() )
		{
			double ic = from->type.doubleValue;
			float fc = float(ic);

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.floatValue = fc;
		}
		else if( from->type.dataType.IsIntegerType() && from->type.dataType.GetSizeInMemoryDWords() == 1 )
		{
			// Must properly convert value in case the from value is smaller
			int ic;
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				ic = (signed char)from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				ic = (short)from->type.wordValue;
			else
				ic = from->type.intValue;
			float fc = float(ic);

			if( int(fc) != ic )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.floatValue = fc;
		}
		else if( from->type.dataType.IsIntegerType() && from->type.dataType.GetSizeInMemoryDWords() == 2 )
		{
			float fc = float(asINT64(from->type.qwordValue));
			if( asINT64(fc) != asINT64(from->type.qwordValue) )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.floatValue = fc;
		}
		else if( from->type.dataType.IsUnsignedType() && from->type.dataType.GetSizeInMemoryDWords() == 1 )
		{
			// Must properly convert value in case the from value is smaller
			unsigned int uic;
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				uic = from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				uic = from->type.wordValue;
			else
				uic = from->type.dwordValue;
			float fc = float(uic);

			if( (unsigned int)(fc) != uic )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.floatValue = fc;
		}
		else if( from->type.dataType.IsUnsignedType() && from->type.dataType.GetSizeInMemoryDWords() == 2 )
		{
			float fc = float((asINT64)from->type.qwordValue);

			if( asQWORD(fc) != from->type.qwordValue )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.floatValue = fc;
		}
	}
	else if( to.IsDoubleType() )
	{
		if( from->type.dataType.IsFloatType() )
		{
			float ic = from->type.floatValue;
			double fc = double(ic);

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.doubleValue = fc;
		}
		else if( from->type.dataType.IsIntegerType() && from->type.dataType.GetSizeInMemoryDWords() == 1 )
		{
			// Must properly convert value in case the from value is smaller
			int ic;
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				ic = (signed char)from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				ic = (short)from->type.wordValue;
			else
				ic = from->type.intValue;
			double fc = double(ic);

			if( int(fc) != ic )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.doubleValue = fc;
		}
		else if( from->type.dataType.IsIntegerType() && from->type.dataType.GetSizeInMemoryDWords() == 2 )
		{
			double fc = double(asINT64(from->type.qwordValue));

			if( asINT64(fc) != asINT64(from->type.qwordValue) )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.doubleValue = fc;
		}
		else if( from->type.dataType.IsUnsignedType() && from->type.dataType.GetSizeInMemoryDWords() == 1 )
		{
			// Must properly convert value in case the from value is smaller
			unsigned int uic;
			if( from->type.dataType.GetSizeInMemoryBytes() == 1 )
				uic = from->type.byteValue;
			else if( from->type.dataType.GetSizeInMemoryBytes() == 2 )
				uic = from->type.wordValue;
			else
				uic = from->type.dwordValue;
			double fc = double(uic);

			if( (unsigned int)(fc) != uic )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.doubleValue = fc;
		}
		else if( from->type.dataType.IsUnsignedType() && from->type.dataType.GetSizeInMemoryDWords() == 2 )
		{
			double fc = double((asINT64)from->type.qwordValue);

			if( asQWORD(fc) != from->type.qwordValue )
			{
				if( convType != asIC_EXPLICIT_VAL_CAST && node ) Warning(TXT_NOT_EXACT, node);
			}

			from->type.dataType = asCDataType::CreatePrimitive(to.GetTokenType(), true);
			from->type.doubleValue = fc;
		}
	}
}

int asCCompiler::DoAssignment(asSExprContext *ctx, asSExprContext *lctx, asSExprContext *rctx, asCScriptNode *lexpr, asCScriptNode *rexpr, int op, asCScriptNode *opNode)
{
	// Don't allow any operators on expressions that take address of class method
	// If methodName is set but the type is not an object, then it is a global function
	if( lctx->methodName != "" || rctx->IsClassMethod() )
	{
		Error(TXT_INVALID_OP_ON_METHOD, opNode);
		return -1;
	}

	// Implicit handle types should always be treated as handles in assignments
	if (lctx->type.dataType.GetObjectType() && (lctx->type.dataType.GetObjectType()->flags & asOBJ_IMPLICIT_HANDLE) )
	{
		lctx->type.dataType.MakeHandle(true);
		lctx->type.isExplicitHandle = true;
	}

	// If the left hand expression is a property accessor, then that should be used
	// to do the assignment instead of the ordinary operator. The exception is when
	// the property accessor is for a handle property, and the operation is a value
	// assignment.
	if( (lctx->property_get || lctx->property_set) &&
		!(lctx->type.dataType.IsObjectHandle() && !lctx->type.isExplicitHandle) )
	{
		if( op != ttAssignment )
		{
			// TODO: getset: We may actually be able to support this, if we can
			//               guarantee that the object reference will stay valid
			//               between the calls to the get and set accessors.

			// Process the property to free the memory
			ProcessPropertySetAccessor(lctx, rctx, opNode);

			// Compound assignments are not allowed for properties
			Error(TXT_COMPOUND_ASGN_WITH_PROP, opNode);
			return -1;
		}

		// It is not allowed to do a handle assignment on a property
		// accessor that doesn't take a handle in the set accessor.
		if( lctx->property_set && lctx->type.isExplicitHandle )
		{
			// set_opIndex has 2 arguments, where as normal setters have only 1
			asCArray<asCDataType>& parameterTypes =
				builder->GetFunctionDescription(lctx->property_set)->parameterTypes;
			if( !parameterTypes[parameterTypes.GetLength() - 1].IsObjectHandle() )
			{
				// Process the property to free the memory
				ProcessPropertySetAccessor(lctx, rctx, opNode);

				Error(TXT_HANDLE_ASSIGN_ON_NON_HANDLE_PROP, opNode);
				return -1;
			}
		}

		MergeExprBytecodeAndType(ctx, lctx);

		return ProcessPropertySetAccessor(ctx, rctx, opNode);
	}
	else if( lctx->property_get && lctx->type.dataType.IsObjectHandle() && !lctx->type.isExplicitHandle )
	{
		// Get the handle to the object that will be used for the value assignment
		ProcessPropertyGetAccessor(lctx, opNode);
	}

	if( lctx->type.dataType.IsPrimitive() )
	{
		if( !lctx->type.isLValue )
		{
			Error(TXT_NOT_LVALUE, lexpr);
			return -1;
		}

		if( op != ttAssignment )
		{
			// Compute the operator before the assignment
			asCTypeInfo lvalue = lctx->type;

			if( lctx->type.isTemporary && !lctx->type.isVariable )
			{
				// The temporary variable must not be freed until the
				// assignment has been performed. lvalue still holds
				// the information about the temporary variable
				lctx->type.isTemporary = false;
			}

			asSExprContext o(engine);
			CompileOperator(opNode, lctx, rctx, &o);
			MergeExprBytecode(rctx, &o);
			rctx->type = o.type;

			// Convert the rvalue to the right type and validate it
			PrepareForAssignment(&lvalue.dataType, rctx, rexpr, false);

			MergeExprBytecode(ctx, rctx);
			lctx->type = lvalue;

			// The lvalue continues the same, either it was a variable, or a reference in the register
		}
		else
		{
			// Convert the rvalue to the right type and validate it
			PrepareForAssignment(&lctx->type.dataType, rctx, rexpr, false, lctx);

			MergeExprBytecode(ctx, rctx);
			MergeExprBytecode(ctx, lctx);
		}

		ReleaseTemporaryVariable(rctx->type, &ctx->bc);

		PerformAssignment(&lctx->type, &rctx->type, &ctx->bc, opNode);

		ctx->type = lctx->type;
	}
	else if( lctx->type.isExplicitHandle )
	{
		if( !lctx->type.isLValue )
		{
			Error(TXT_NOT_LVALUE, lexpr);
			return -1;
		}

		// Object handles don't have any compound assignment operators
		if( op != ttAssignment )
		{
			asCString str;
			str.Format(TXT_ILLEGAL_OPERATION_ON_s, lctx->type.dataType.Format().AddressOf());
			Error(str, lexpr);
			return -1;
		}

		if( lctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE )
		{
			// The object is a value type but that should be treated as a handle

			// Make sure the right hand value is a handle
			if( !rctx->type.isExplicitHandle &&
				!(rctx->type.dataType.GetObjectType() && (rctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE)) )
			{
				// Function names can be considered handles already
				if( rctx->methodName == "" )
				{
					asCDataType dt = rctx->type.dataType;
					dt.MakeHandle(true);
					dt.MakeReference(false);

					PrepareArgument(&dt, rctx, rexpr, true, asTM_INREF);
					if( !dt.IsEqualExceptRefAndConst(rctx->type.dataType) )
					{
						asCString str;
						str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rctx->type.dataType.Format().AddressOf(), lctx->type.dataType.Format().AddressOf());
						Error(str, rexpr);
						return -1;
					}
				}
			}

			if( CompileOverloadedDualOperator(opNode, lctx, rctx, ctx) )
			{
				// An overloaded assignment operator was found (or a compilation error occured)
				return 0;
			}

			// The object must implement the opAssign method
			Error(TXT_NO_APPROPRIATE_OPASSIGN, opNode);
			return -1;
		}
		else
		{
			asCDataType dt = lctx->type.dataType;
			dt.MakeReference(false);

			PrepareArgument(&dt, rctx, rexpr, true, asTM_INREF , true);
			if( !dt.IsEqualExceptRefAndConst(rctx->type.dataType) )
			{
				asCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rctx->type.dataType.Format().AddressOf(), lctx->type.dataType.Format().AddressOf());
				Error(str, rexpr);
				return -1;
			}

			MergeExprBytecode(ctx, rctx);
			MergeExprBytecode(ctx, lctx);

			ctx->bc.InstrWORD(asBC_GETOBJREF, AS_PTR_SIZE);

			PerformAssignment(&lctx->type, &rctx->type, &ctx->bc, opNode);

			ReleaseTemporaryVariable(rctx->type, &ctx->bc);

			ctx->type = lctx->type;

			// After the handle assignment the original handle is left on the stack
			ctx->type.dataType.MakeReference(false);
		}
	}
	else // if( lctx->type.dataType.IsObject() )
	{
		// An ASHANDLE type must not allow a value assignment, as
		// the opAssign operator is used for the handle assignment
		if( lctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE )
		{
			asCString str;
			str.Format(TXT_ILLEGAL_OPERATION_ON_s, lctx->type.dataType.Format().AddressOf());
			Error(str, lexpr);
			return -1;
		}

		// The lvalue reference may be marked as a temporary, if for example
		// it was originated as a handle returned from a function. In such
		// cases it must be possible to assign values to it anyway.
		if( lctx->type.dataType.IsObjectHandle() && !lctx->type.isExplicitHandle )
		{
			// Convert the handle to a object reference
			asCDataType to;
			to = lctx->type.dataType;
			to.MakeHandle(false);
			ImplicitConversion(lctx, to, lexpr, asIC_IMPLICIT_CONV);
			lctx->type.isLValue = true; // Handle may not have been an lvalue, but the dereferenced object is
		}

		// Check for overloaded assignment operator
		if( CompileOverloadedDualOperator(opNode, lctx, rctx, ctx) )
		{
			// An overloaded assignment operator was found (or a compilation error occured)
			return 0;
		}

		// No registered operator was found. In case the operation is a direct
		// assignment and the rvalue is the same type as the lvalue, then we can
		// still use the byte-for-byte copy to do the assignment

		if( op != ttAssignment )
		{
			asCString str;
			str.Format(TXT_ILLEGAL_OPERATION_ON_s, lctx->type.dataType.Format().AddressOf());
			Error(str, lexpr);
			return -1;
		}

		// If the left hand expression is simple, i.e. without any
		// function calls or allocations of memory, then we can avoid
		// doing a copy of the right hand expression (done by PrepareArgument).
		// Instead the reference to the value can be placed directly on the
		// stack.
		//
		// This optimization should only be done for value types, where
		// the application developer is responsible for making the
		// implementation safe against unwanted destruction of the input
		// reference before the time.
		bool simpleExpr = (lctx->type.dataType.GetObjectType()->GetFlags() & asOBJ_VALUE) && lctx->bc.IsSimpleExpression();

		// Implicitly convert the rvalue to the type of the lvalue
		bool needConversion = false;
		if( !lctx->type.dataType.IsEqualExceptRefAndConst(rctx->type.dataType) )
			needConversion = true;

		if( !simpleExpr || needConversion )
		{
			asCDataType dt = lctx->type.dataType;
			dt.MakeReference(true);
			dt.MakeReadOnly(true);
			PrepareArgument(&dt, rctx, rexpr, true, 1, !needConversion);
			if( !dt.IsEqualExceptRefAndConst(rctx->type.dataType) )
			{
				asCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rctx->type.dataType.Format().AddressOf(), lctx->type.dataType.Format().AddressOf());
				Error(str, rexpr);
				return -1;
			}
		}
		else
		{
			// Process any property accessor first, before placing the final reference on the stack
			ProcessPropertyGetAccessor(rctx, rexpr);

			if( rctx->type.dataType.IsReference() && (!(rctx->type.isVariable || rctx->type.isTemporary) || IsVariableOnHeap(rctx->type.stackOffset)) )
				rctx->bc.Instr(asBC_RDSPtr);
		}

		MergeExprBytecode(ctx, rctx);
		MergeExprBytecode(ctx, lctx);

		if( !simpleExpr || needConversion )
		{
			if( (rctx->type.isVariable || rctx->type.isTemporary) )
			{
				if( !IsVariableOnHeap(rctx->type.stackOffset) )
					// TODO: runtime optimize: Actually the reference can be pushed on the stack directly
					//                         as the value allocated on the stack is guaranteed to be safe.
					//                         The bytecode optimizer should be able to determine this and optimize away the VAR + GETREF
					ctx->bc.InstrWORD(asBC_GETREF, AS_PTR_SIZE);
				else
					ctx->bc.InstrWORD(asBC_GETOBJREF, AS_PTR_SIZE);
			}
		}

		PerformAssignment(&lctx->type, &rctx->type, &ctx->bc, opNode);

		ReleaseTemporaryVariable(rctx->type, &ctx->bc);

		ctx->type = lctx->type;
	}

	return 0;
}

int asCCompiler::CompileAssignment(asCScriptNode *expr, asSExprContext *ctx)
{
	asCScriptNode *lexpr = expr->firstChild;
	if( lexpr->next )
	{
		// Compile the two expression terms
		asSExprContext lctx(engine), rctx(engine);
		int rr = CompileAssignment(lexpr->next->next, &rctx);
		int lr = CompileCondition(lexpr, &lctx);

		if( lr >= 0 && rr >= 0 )
			return DoAssignment(ctx, &lctx, &rctx, lexpr, lexpr->next->next, lexpr->next->tokenType, lexpr->next);

		// Since the operands failed, the assignment was not computed
		ctx->type.SetDummy();
		return -1;
	}

	return CompileCondition(lexpr, ctx);
}

int asCCompiler::CompileCondition(asCScriptNode *expr, asSExprContext *ctx)
{
	asCTypeInfo ctype;

	// Compile the conditional expression
	asCScriptNode *cexpr = expr->firstChild;
	if( cexpr->next )
	{
		//-------------------------------
		// Compile the condition
		asSExprContext e(engine);
		int r = CompileExpression(cexpr, &e);
		if( r < 0 )
			e.type.SetConstantB(asCDataType::CreatePrimitive(ttBool, true), true);
		if( r >= 0 && !e.type.dataType.IsEqualExceptRefAndConst(asCDataType::CreatePrimitive(ttBool, true)) )
		{
			Error(TXT_EXPR_MUST_BE_BOOL, cexpr);
			e.type.SetConstantB(asCDataType::CreatePrimitive(ttBool, true), true);
		}
		ctype = e.type;

		ProcessPropertyGetAccessor(&e, cexpr);

		if( e.type.dataType.IsReference() ) ConvertToVariable(&e);
		ProcessDeferredParams(&e);

		//-------------------------------
		// Compile the left expression
		asSExprContext le(engine);
		int lr = CompileAssignment(cexpr->next, &le);

		//-------------------------------
		// Compile the right expression
		asSExprContext re(engine);
		int rr = CompileAssignment(cexpr->next->next, &re);

		if( lr >= 0 && rr >= 0 )
		{
			// Don't allow any operators on expressions that take address of class method
			if( le.IsClassMethod() || re.IsClassMethod() )
			{
				Error(TXT_INVALID_OP_ON_METHOD, expr);
				return -1;
			}

			ProcessPropertyGetAccessor(&le, cexpr->next);
			ProcessPropertyGetAccessor(&re, cexpr->next->next);

			bool isExplicitHandle = le.type.isExplicitHandle || re.type.isExplicitHandle;

			// Allow a 0 or null in the first case to be implicitly converted to the second type
			if( le.type.isConstant && le.type.intValue == 0 && le.type.dataType.IsIntegerType() )
			{
				asCDataType to = re.type.dataType;
				to.MakeReference(false);
				to.MakeReadOnly(true);
				ImplicitConversionConstant(&le, to, cexpr->next, asIC_IMPLICIT_CONV);
			}
			else if( le.type.IsNullConstant() )
			{
				asCDataType to = re.type.dataType;
				to.MakeHandle(true);
				ImplicitConversion(&le, to, cexpr->next, asIC_IMPLICIT_CONV);
			}

			// Allow either case to be converted to const @ if the other is const @
			if( (le.type.dataType.IsHandleToConst() && !le.type.IsNullConstant()) || (re.type.dataType.IsHandleToConst() && !re.type.dataType.IsNullHandle()) )
			{
				le.type.dataType.MakeHandleToConst(true);
				re.type.dataType.MakeHandleToConst(true);
			}

			//---------------------------------
			// Output the byte code
			int afterLabel = nextLabel++;
			int elseLabel = nextLabel++;

			// If left expression is void, then we don't need to store the result
			if( le.type.dataType.IsEqualExceptConst(asCDataType::CreatePrimitive(ttVoid, false)) )
			{
				// Put the code for the condition expression on the output
				MergeExprBytecode(ctx, &e);

				// Added the branch decision
				ctx->type = e.type;
				ConvertToVariable(ctx);
				ctx->bc.InstrSHORT(asBC_CpyVtoR4, ctx->type.stackOffset);
				ctx->bc.Instr(asBC_ClrHi);
				ctx->bc.InstrDWORD(asBC_JZ, elseLabel);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);

				// Add the left expression
				MergeExprBytecode(ctx, &le);
				ctx->bc.InstrINT(asBC_JMP, afterLabel);

				// Add the right expression
				ctx->bc.Label((short)elseLabel);
				MergeExprBytecode(ctx, &re);
				ctx->bc.Label((short)afterLabel);

				// Make sure both expressions have the same type
				if( le.type.dataType != re.type.dataType )
					Error(TXT_BOTH_MUST_BE_SAME, expr);

				// Set the type of the result
				ctx->type = le.type;
			}
			else
			{
				// Allocate temporary variable and copy the result to that one
				asCTypeInfo temp;
				temp = le.type;
				temp.dataType.MakeReference(false);
				temp.dataType.MakeReadOnly(false);
				
				// Make sure the variable isn't used in any of the expressions, 
				// as it would be overwritten which may cause crashes or less visible bugs
				int l = int(reservedVariables.GetLength());
				e.bc.GetVarsUsed(reservedVariables);
				le.bc.GetVarsUsed(reservedVariables);
				re.bc.GetVarsUsed(reservedVariables);
				int offset = AllocateVariable(temp.dataType, true, false);
				reservedVariables.SetLength(l);
				
				temp.SetVariable(temp.dataType, offset, true);

				// TODO: copy: Use copy constructor if available. See PrepareTemporaryObject()

				CallDefaultConstructor(temp.dataType, offset, IsVariableOnHeap(offset), &ctx->bc, expr);

				// Put the code for the condition expression on the output
				MergeExprBytecode(ctx, &e);

				// Add the branch decision
				ctx->type = e.type;
				ConvertToVariable(ctx);
				ctx->bc.InstrSHORT(asBC_CpyVtoR4, ctx->type.stackOffset);
				ctx->bc.Instr(asBC_ClrHi);
				ctx->bc.InstrDWORD(asBC_JZ, elseLabel);
				ReleaseTemporaryVariable(ctx->type, &ctx->bc);

				// Assign the result of the left expression to the temporary variable
				asCTypeInfo rtemp;
				rtemp = temp;
				if( rtemp.dataType.IsObjectHandle() )
					rtemp.isExplicitHandle = true;

				PrepareForAssignment(&rtemp.dataType, &le, cexpr->next, true);
				MergeExprBytecode(ctx, &le);

				if( !rtemp.dataType.IsPrimitive() )
				{
					ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
					rtemp.dataType.MakeReference(IsVariableOnHeap(offset));
				}
				asCTypeInfo result;
				result = rtemp;
				PerformAssignment(&result, &le.type, &ctx->bc, cexpr->next);
				if( !result.dataType.IsPrimitive() )
					ctx->bc.Instr(asBC_PopPtr); // Pop the original value (always a pointer)

				// Release the old temporary variable
				ReleaseTemporaryVariable(le.type, &ctx->bc);

				ctx->bc.InstrINT(asBC_JMP, afterLabel);

				// Start of the right expression
				ctx->bc.Label((short)elseLabel);

				// Copy the result to the same temporary variable
				PrepareForAssignment(&rtemp.dataType, &re, cexpr->next, true);
				MergeExprBytecode(ctx, &re);

				if( !rtemp.dataType.IsPrimitive() )
				{
					ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
					rtemp.dataType.MakeReference(IsVariableOnHeap(offset));
				}
				result = rtemp;
				PerformAssignment(&result, &re.type, &ctx->bc, cexpr->next);
				if( !result.dataType.IsPrimitive() )
					ctx->bc.Instr(asBC_PopPtr); // Pop the original value (always a pointer)

				// Release the old temporary variable
				ReleaseTemporaryVariable(re.type, &ctx->bc);

				ctx->bc.Label((short)afterLabel);

				// Make sure both expressions have the same type
				if( !le.type.dataType.IsEqualExceptConst(re.type.dataType) )
					Error(TXT_BOTH_MUST_BE_SAME, expr);

				// Set the temporary variable as output
				ctx->type = rtemp;
				ctx->type.isExplicitHandle = isExplicitHandle;

				if( !ctx->type.dataType.IsPrimitive() )
				{
					ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
					ctx->type.dataType.MakeReference(IsVariableOnHeap(offset));
				}

				// Make sure the output isn't marked as being a literal constant
				ctx->type.isConstant = false;
			}
		}
		else
		{
			ctx->type.SetDummy();
			return -1;
		}
	}
	else
		return CompileExpression(cexpr, ctx);

	return 0;
}

int asCCompiler::CompileExpression(asCScriptNode *expr, asSExprContext *ctx)
{
	asASSERT(expr->nodeType == snExpression);

	// Convert to polish post fix, i.e: a+b => ab+

	// The algorithm that I've implemented here is similar to
	// Djikstra's Shunting Yard algorithm, though I didn't know it at the time.
	// ref: http://en.wikipedia.org/wiki/Shunting-yard_algorithm

	// Count the nodes in order to preallocate the buffers
	int count = 0;
	asCScriptNode *node = expr->firstChild;
	while( node )
	{
		count++;
		node = node->next;
	}

	asCArray<asCScriptNode *> stack(count);
	asCArray<asCScriptNode *> stack2(count);

	node = expr->firstChild;
	while( node )
	{
		int precedence = GetPrecedence(node);

		while( stack.GetLength() > 0 &&
			   precedence <= GetPrecedence(stack[stack.GetLength()-1]) )
			stack2.PushLast(stack.PopLast());

		stack.PushLast(node);

		node = node->next;
	}

	while( stack.GetLength() > 0 )
		stack2.PushLast(stack.PopLast());

	// Compile the postfix formatted expression
	return CompilePostFixExpression(&stack2, ctx);
}

int asCCompiler::CompilePostFixExpression(asCArray<asCScriptNode *> *postfix, asSExprContext *ctx)
{
	// Shouldn't send any byte code
	asASSERT(ctx->bc.GetLastInstr() == -1);

	// Set the context to a dummy type to avoid further
	// errors in case the expression fails to compile
	ctx->type.SetDummy();

	// Evaluate the operands and operators
	asCArray<asSExprContext*> free;
	asCArray<asSExprContext*> expr;
	int ret = 0;
	for( asUINT n = 0; ret == 0 && n < postfix->GetLength(); n++ )
	{
		asCScriptNode *node = (*postfix)[n];
		if( node->nodeType == snExprTerm )
		{
			asSExprContext *e = free.GetLength() ? free.PopLast() : asNEW(asSExprContext)(engine);
			expr.PushLast(e);
			e->exprNode = node;
			ret = CompileExpressionTerm(node, e);
		}
		else
		{
			asSExprContext *r = expr.PopLast();
			asSExprContext *l = expr.PopLast();

			// Now compile the operator
			asSExprContext *e = free.GetLength() ? free.PopLast() : asNEW(asSExprContext)(engine);
			ret = CompileOperator(node, l, r, e);

			expr.PushLast(e);

			// Free the operands
			l->Clear();
			free.PushLast(l);
			r->Clear();
			free.PushLast(r);
		}
	}

	if( ret == 0 )
	{
		asASSERT(expr.GetLength() == 1);

		// The final result should be moved to the output context
		MergeExprBytecodeAndType(ctx, expr[0]);
	}

	// Clean up
	for( asUINT e = 0; e < expr.GetLength(); e++ )
		asDELETE(expr[e], asSExprContext);
	for( asUINT f = 0; f < free.GetLength(); f++ )
		asDELETE(free[f], asSExprContext);

	return ret;
}

int asCCompiler::CompileExpressionTerm(asCScriptNode *node, asSExprContext *ctx)
{
	// Shouldn't send any byte code
	asASSERT(ctx->bc.GetLastInstr() == -1);

	// Set the type as a dummy by default, in case of any compiler errors
	ctx->type.SetDummy();

	// Compile the value node
	asCScriptNode *vnode = node->firstChild;
	while( vnode->nodeType != snExprValue )
		vnode = vnode->next;

	asSExprContext v(engine);
	int r = CompileExpressionValue(vnode, &v); if( r < 0 ) return r;

	// Compile post fix operators
	asCScriptNode *pnode = vnode->next;
	while( pnode )
	{
		r = CompileExpressionPostOp(pnode, &v); if( r < 0 ) return r;
		pnode = pnode->next;
	}

	// Compile pre fix operators
	pnode = vnode->prev;
	while( pnode )
	{
		r = CompileExpressionPreOp(pnode, &v); if( r < 0 ) return r;
		pnode = pnode->prev;
	}

	// Return the byte code and final type description
	MergeExprBytecodeAndType(ctx, &v);

	return 0;
}

int asCCompiler::CompileVariableAccess(const asCString &name, const asCString &scope, asSExprContext *ctx, asCScriptNode *errNode, bool isOptional, bool noFunction, bool noGlobal, asCObjectType *objType)
{
	bool found = false;

	// It is a local variable or parameter?
	// This is not accessible by default arg expressions
	sVariable *v = 0;
	if( !isCompilingDefaultArg && scope == "" && !objType && variables )
		v = variables->GetVariable(name.AddressOf());
	if( v )
	{
		found = true;

		if( v->isPureConstant )
			ctx->type.SetConstantQW(v->type, v->constantValue);
		else if( v->type.IsPrimitive() )
		{
			if( v->type.IsReference() )
			{
				// Copy the reference into the register
				ctx->bc.InstrSHORT(asBC_PshVPtr, (short)v->stackOffset);
				ctx->bc.Instr(asBC_PopRPtr);
				ctx->type.Set(v->type);
			}
			else
				ctx->type.SetVariable(v->type, v->stackOffset, false);

			ctx->type.isLValue = true;
		}
		else
		{
			ctx->bc.InstrSHORT(asBC_PSF, (short)v->stackOffset);
			ctx->type.SetVariable(v->type, v->stackOffset, false);

			// If the variable is allocated on the heap we have a reference,
			// otherwise the actual object pointer is pushed on the stack.
			if( v->onHeap || v->type.IsObjectHandle() ) ctx->type.dataType.MakeReference(true);

			// Implicitly dereference handle parameters sent by reference
			if( v->type.IsReference() && (!v->type.IsObject() || v->type.IsObjectHandle()) )
				ctx->bc.Instr(asBC_RDSPtr);

			ctx->type.isLValue = true;
		}
	}

	// Is it a class member?
	// This is not accessible by default arg expressions
	if( !isCompilingDefaultArg && !found && ((objType) || (outFunc && outFunc->objectType && scope == "")) )
	{
		if( name == THIS_TOKEN && !objType )
		{
			asCDataType dt = asCDataType::CreateObject(outFunc->objectType, outFunc->isReadOnly);

			// The object pointer is located at stack position 0
			ctx->bc.InstrSHORT(asBC_PSF, 0);
			ctx->type.SetVariable(dt, 0, false);
			ctx->type.dataType.MakeReference(true);
			ctx->type.isLValue = true;

			found = true;
		}

		if( !found )
		{
			// See if there are any matching property accessors
			asSExprContext access(engine);
			if( objType )
				access.type.Set(asCDataType::CreateObject(objType, false));
			else
				access.type.Set(asCDataType::CreateObject(outFunc->objectType, outFunc->isReadOnly));
			access.type.dataType.MakeReference(true);
			int r = 0;
			if( errNode->next && errNode->next->tokenType == ttOpenBracket )
			{
				// This is an index access, check if there is a property accessor that takes an index arg
				asSExprContext dummyArg(engine);
				r = FindPropertyAccessor(name, &access, &dummyArg, errNode, 0, true);
			}
			if( r == 0 )
			{
				// Normal property access
				r = FindPropertyAccessor(name, &access, errNode, 0, true);
			}
			if( r < 0 ) return -1;
			if( access.property_get || access.property_set )
			{
				if( !objType )
				{
					// Prepare the bytecode for the member access
					// This is only done when accessing through the implicit this pointer
					ctx->bc.InstrSHORT(asBC_PSF, 0);
				}
				MergeExprBytecodeAndType(ctx, &access);

				found = true;
			}
		}

		if( !found )
		{
			asCDataType dt;
			if( objType )
				dt = asCDataType::CreateObject(objType, false);
			else
				dt = asCDataType::CreateObject(outFunc->objectType, false);
			asCObjectProperty *prop = builder->GetObjectProperty(dt, name.AddressOf());
			if( prop )
			{
				if( !objType )
				{
					// The object pointer is located at stack position 0
					// This is only done when accessing through the implicit this pointer
					ctx->bc.InstrSHORT(asBC_PSF, 0);
					ctx->type.SetVariable(dt, 0, false);
					ctx->type.dataType.MakeReference(true);
					Dereference(ctx, true);
				}

				// TODO: This is the same as what is in CompileExpressionPostOp
				// Put the offset on the stack
				ctx->bc.InstrSHORT_DW(asBC_ADDSi, (short)prop->byteOffset, engine->GetTypeIdFromDataType(dt));

				if( prop->type.IsReference() )
					ctx->bc.Instr(asBC_RDSPtr);

				// Reference to primitive must be stored in the temp register
				if( prop->type.IsPrimitive() )
				{
					// TODO: runtime optimize: The ADD offset command should store the reference in the register directly
					ctx->bc.Instr(asBC_PopRPtr);
				}

				// Set the new type (keeping info about temp variable)
				ctx->type.dataType = prop->type;
				ctx->type.dataType.MakeReference(true);
				ctx->type.isVariable = false;
				ctx->type.isLValue = true;

				if( ctx->type.dataType.IsObject() && !ctx->type.dataType.IsObjectHandle() )
				{
					// Objects that are members are not references
					ctx->type.dataType.MakeReference(false);
				}

				// If the object reference is const, the property will also be const
				ctx->type.dataType.MakeReadOnly(outFunc->isReadOnly);

				found = true;
			}
			else if( outFunc->objectType )
			{
				// If it is not a property, it may still be the name of a method which can be used to create delegates
				asCObjectType *ot = outFunc->objectType;
				asCScriptFunction *func = 0;
				for( asUINT n = 0; n < ot->methods.GetLength(); n++ )
				{
					if( engine->scriptFunctions[ot->methods[n]]->name == name )
					{
						func = engine->scriptFunctions[ot->methods[n]];
						break;
					}
				}

				if( func )
				{
					// An object method was found. Keep the name of the method in the expression, but
					// don't actually modify the bytecode at this point since it is not yet known what 
					// the method will be used for, or even what overloaded method should be used.
					ctx->methodName = name;

					// Place the object pointer on the stack, as if the expression was this.func
					if( !objType )
					{
						// The object pointer is located at stack position 0
						// This is only done when accessing through the implicit this pointer
						ctx->bc.InstrSHORT(asBC_PSF, 0);
						ctx->type.SetVariable(asCDataType::CreateObject(outFunc->objectType, false), 0, false);
						ctx->type.dataType.MakeReference(true);
						Dereference(ctx, true);
					}

					found = true;
				}
			}
		}
	}

	// Recursively search parent namespaces for global entities
	asCString currScope = scope;
	if( scope == "" )
		currScope = outFunc->nameSpace->name;
	while( !found && !noGlobal && !objType )
	{
		asSNameSpace *ns = DetermineNameSpace(currScope);

		// Is it a global property?
		if( !found && ns )
		{
			// See if there are any matching global property accessors
			asSExprContext access(engine);
			int r = 0;
			if( errNode->next && errNode->next->tokenType == ttOpenBracket )
			{
				// This is an index access, check if there is a property accessor that takes an index arg
				asSExprContext dummyArg(engine);
				r = FindPropertyAccessor(name, &access, &dummyArg, errNode, ns);
			}
			if( r == 0 )
			{
				// Normal property access
				r = FindPropertyAccessor(name, &access, errNode, ns);
			}
			if( r < 0 ) return -1;
			if( access.property_get || access.property_set )
			{
				// Prepare the bytecode for the function call
				MergeExprBytecodeAndType(ctx, &access);

				found = true;
			}

			// See if there is any matching global property
			if( !found )
			{
				bool isCompiled = true;
				bool isPureConstant = false;
				bool isAppProp = false;
				asQWORD constantValue = 0;
				asCGlobalProperty *prop = builder->GetGlobalProperty(name.AddressOf(), ns, &isCompiled, &isPureConstant, &constantValue, &isAppProp);
				if( prop )
				{
					found = true;

					// Verify that the global property has been compiled already
					if( isCompiled )
					{
						if( ctx->type.dataType.GetObjectType() && (ctx->type.dataType.GetObjectType()->flags & asOBJ_IMPLICIT_HANDLE) )
						{
							ctx->type.dataType.MakeHandle(true);
							ctx->type.isExplicitHandle = true;
						}

						// If the global property is a pure constant
						// we can allow the compiler to optimize it. Pure
						// constants are global constant variables that were
						// initialized by literal constants.
						if( isPureConstant )
							ctx->type.SetConstantQW(prop->type, constantValue);
						else
						{
							// A shared type must not access global vars, unless they
							// too are shared, e.g. application registered vars
							if( outFunc->IsShared() )
							{
								if( !isAppProp )
								{
									asCString str;
									str.Format(TXT_SHARED_CANNOT_ACCESS_NON_SHARED_VAR_s, prop->name.AddressOf());
									Error(str, errNode);

									// Allow the compilation to continue to catch other problems
								}
							}

							ctx->type.Set(prop->type);
							ctx->type.isLValue = true;

							if( ctx->type.dataType.IsPrimitive() )
							{
								// Load the address of the variable into the register
								ctx->bc.InstrPTR(asBC_LDG, prop->GetAddressOfValue());

								ctx->type.dataType.MakeReference(true);
							}
							else
							{
								// Push the address of the variable on the stack
								ctx->bc.InstrPTR(asBC_PGA, prop->GetAddressOfValue());

								// If the object is a value type or a non-handle variable to a reference type,
								// then we must validate the existance as it could potentially be accessed
								// before it is initialized.
								if( (ctx->type.dataType.GetObjectType()->flags & asOBJ_VALUE) ||
									!ctx->type.dataType.IsObjectHandle() )
								{
									// TODO: runtime optimize: This is not necessary for application registered properties
									ctx->bc.Instr(asBC_ChkRefS);
								}

								// If the address pushed on the stack is to a value type or an object
								// handle, then mark the expression as a reference. Addresses to a reference
								// type aren't marked as references to get correct behaviour
								if( (ctx->type.dataType.GetObjectType()->flags & asOBJ_VALUE) ||
									ctx->type.dataType.IsObjectHandle() )
								{
									ctx->type.dataType.MakeReference(true);
								}
								else
								{
									asASSERT( (ctx->type.dataType.GetObjectType()->flags & asOBJ_REF) && !ctx->type.dataType.IsObjectHandle() );

									// It's necessary to dereference the pointer so the pointer on the stack will point to the actual object
									ctx->bc.Instr(asBC_RDSPtr);
								}
							}
						}
					}
					else
					{
						asCString str;
						str.Format(TXT_UNINITIALIZED_GLOBAL_VAR_s, prop->name.AddressOf());
						Error(str, errNode);
						return -1;
					}
				}
			}
		}

		// Is it the name of a global function?
		if( !noFunction && !found && ns )
		{
			asCArray<int> funcs;

			builder->GetFunctionDescriptions(name.AddressOf(), funcs, ns);

			if( funcs.GetLength() > 0 )
			{
				found = true;

				// Defer the evaluation of which function until it is actually used
				// Store the namespace and name of the function for later
				ctx->type.SetUndefinedFuncHandle(engine);
				ctx->methodName = ns ? ns->name + "::" + name : name;
			}
		}

		// Is it an enum value?
		if( !found )
		{
			// The enum type may be declared in a namespace too
			asCObjectType *scopeType = 0;
			if( currScope != "" && currScope != "::" )
			{
				// Use the last scope name as the enum type
				asCString enumType = currScope;
				asCString nsScope;
				int p = currScope.FindLast("::");
				if( p != -1 )
				{
					enumType = currScope.SubString(p+2);
					nsScope = currScope.SubString(0, p);
				}

				asSNameSpace *ns = engine->FindNameSpace(nsScope.AddressOf());
				if( ns )
					scopeType = builder->GetObjectType(enumType.AddressOf(), ns);
			}

			asDWORD value = 0;
			asCDataType dt;
			if( scopeType && builder->GetEnumValueFromObjectType(scopeType, name.AddressOf(), dt, value) )
			{
				// scoped enum value found
				found = true;
			}
			else if( !engine->ep.requireEnumScope )
			{
				// Look for the enum value without explicitly informing the enum type
				asSNameSpace *ns = DetermineNameSpace(currScope);
				int e = 0;
				if( ns )
					e = builder->GetEnumValue(name.AddressOf(), dt, value, ns);
				if( e )
				{
					found = true;
					if( e == 2 )
					{
						// Ambiguous enum value: Save the name for resolution later.
						// The ambiguity could be resolved now, but I hesitate
						// to store too much information in the context.
						ctx->enumValue = name.AddressOf();
						
						// We cannot set a dummy value because it will pass through
						// cleanly as an integer.
						ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttIdentifier, true), 0);
						return 0;
					}
				}
			}

			if( found )
			{
				// Even if the enum type is not shared, and we're compiling a shared object,
				// the use of the values are still allowed, since they are treated as constants.

				// an enum value was resolved
				ctx->type.SetConstantDW(dt, value);
			}
			else
			{
				// If nothing was found because the scope doesn't match a namespace or an enum 
				// then this should be reported as an error and the search interrupted
				if( !ns && !scopeType )
				{
					ctx->type.SetDummy();
					asCString str;
					str.Format(TXT_UNKNOWN_SCOPE_s, currScope.AddressOf());
					Error(str, errNode);
					return -1;
				}
			}
		}

		if( !found )
		{
			if( currScope == "" || currScope == "::" )
				break;

			// Move up to parent namespace
			int pos = currScope.FindLast("::");
			if( pos >= 0 )
				currScope = currScope.SubString(0, pos);
			else
				currScope = "::";
		}
	}

	// The name doesn't match any variable
	if( !found )
	{
		// Give dummy value
		ctx->type.SetDummy();

		if( !isOptional )
		{
			// Prepend the scope to the name for the error message
			asCString ename;
			if( scope != "" && scope != "::" )
				ename = scope + "::";
			else
				ename = scope;
			ename += name;

			asCString str;
			str.Format(TXT_s_NOT_DECLARED, ename.AddressOf());
			Error(str, errNode);

			// Declare the variable now so that it will not be reported again
			variables->DeclareVariable(name.AddressOf(), asCDataType::CreatePrimitive(ttInt, false), 0x7FFF, true);

			// Mark the variable as initialized so that the user will not be bother by it again
			sVariable *v = variables->GetVariable(name.AddressOf());
			asASSERT(v);
			if( v ) v->isInitialized = true;
		}

		// Return -1 to signal that the variable wasn't found
		return -1;
	}

	return 0;
}

int asCCompiler::CompileExpressionValue(asCScriptNode *node, asSExprContext *ctx)
{
	// Shouldn't receive any byte code
	asASSERT(ctx->bc.GetLastInstr() == -1);

	asCScriptNode *vnode = node->firstChild;
	ctx->exprNode = vnode;
	if( vnode->nodeType == snVariableAccess )
	{
		// Determine the scope resolution of the variable
		asCString scope = builder->GetScopeFromNode(vnode->firstChild, script, &vnode);

		// Determine the name of the variable
		asASSERT(vnode->nodeType == snIdentifier );
		asCString name(&script->code[vnode->tokenPos], vnode->tokenLength);

		return CompileVariableAccess(name, scope, ctx, node);
	}
	else if( vnode->nodeType == snConstant )
	{
		if( vnode->tokenType == ttIntConstant )
		{
			asCString value(&script->code[vnode->tokenPos], vnode->tokenLength);

			asQWORD val = asStringScanUInt64(value.AddressOf(), 10, 0);

			// Do we need 64 bits?
			// If the 31st bit is set we'll treat the value as a signed 64bit number to avoid
			// incorrect warnings about changing signs if the value is assigned to a 64bit variable
			if( val>>31 )
			{
				// Only if the value uses the last bit of a 64bit word do we consider the number unsigned
				if( val>>63 )
					ctx->type.SetConstantQW(asCDataType::CreatePrimitive(ttUInt64, true), val);
				else
					ctx->type.SetConstantQW(asCDataType::CreatePrimitive(ttInt64, true), val);
			}
			else
				ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttInt, true), asDWORD(val));
		}
		else if( vnode->tokenType == ttBitsConstant )
		{
			asCString value(&script->code[vnode->tokenPos], vnode->tokenLength);

			// Let the function determine the radix from the prefix 0x = 16, 0d = 10, 0o = 8, or 0b = 2
			// TODO: Check for overflow
			asQWORD val = asStringScanUInt64(value.AddressOf(), 0, 0);

			// Do we need 64 bits?
			if( val>>32 )
				ctx->type.SetConstantQW(asCDataType::CreatePrimitive(ttUInt64, true), val);
			else
				ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttUInt, true), asDWORD(val));
		}
		else if( vnode->tokenType == ttFloatConstant )
		{
			asCString value(&script->code[vnode->tokenPos], vnode->tokenLength);

			// TODO: Check for overflow

			size_t numScanned;
			float v = float(asStringScanDouble(value.AddressOf(), &numScanned));
			ctx->type.SetConstantF(asCDataType::CreatePrimitive(ttFloat, true), v);
#ifndef AS_USE_DOUBLE_AS_FLOAT
			// Don't check this if we have double as float, because then the whole token would be scanned (i.e. no f suffix)
			asASSERT(numScanned == vnode->tokenLength - 1);
#endif
		}
		else if( vnode->tokenType == ttDoubleConstant )
		{
			asCString value(&script->code[vnode->tokenPos], vnode->tokenLength);

			// TODO: Check for overflow

			size_t numScanned;
			double v = asStringScanDouble(value.AddressOf(), &numScanned);
			ctx->type.SetConstantD(asCDataType::CreatePrimitive(ttDouble, true), v);
			asASSERT(numScanned == vnode->tokenLength);
		}
		else if( vnode->tokenType == ttTrue ||
			     vnode->tokenType == ttFalse )
		{
#if AS_SIZEOF_BOOL == 1
			ctx->type.SetConstantB(asCDataType::CreatePrimitive(ttBool, true), vnode->tokenType == ttTrue ? VALUE_OF_BOOLEAN_TRUE : 0);
#else
			ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), vnode->tokenType == ttTrue ? VALUE_OF_BOOLEAN_TRUE : 0);
#endif
		}
		else if( vnode->tokenType == ttStringConstant ||
			     vnode->tokenType == ttMultilineStringConstant ||
				 vnode->tokenType == ttHeredocStringConstant )
		{
			asCString str;
			asCScriptNode *snode = vnode->firstChild;
			if( script->code[snode->tokenPos] == '\'' && engine->ep.useCharacterLiterals )
			{
				// Treat the single quoted string as a single character literal
				str.Assign(&script->code[snode->tokenPos+1], snode->tokenLength-2);

				asDWORD val = 0;
				if( str.GetLength() && (unsigned char)str[0] > 127 && engine->ep.scanner == 1 )
				{
					// This is the start of a UTF8 encoded character. We need to decode it
					val = asStringDecodeUTF8(str.AddressOf(), 0);
					if( val == (asDWORD)-1 )
						Error(TXT_INVALID_CHAR_LITERAL, vnode);
				}
				else
				{
					val = ProcessStringConstant(str, snode);
					if( val == (asDWORD)-1 )
						Error(TXT_INVALID_CHAR_LITERAL, vnode);
				}

				ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttUInt, true), val);
			}
			else
			{
				// Process the string constants
				while( snode )
				{
					asCString cat;
					if( snode->tokenType == ttStringConstant )
					{
						cat.Assign(&script->code[snode->tokenPos+1], snode->tokenLength-2);
						ProcessStringConstant(cat, snode);
					}
					else if( snode->tokenType == ttMultilineStringConstant )
					{
						if( !engine->ep.allowMultilineStrings )
							Error(TXT_MULTILINE_STRINGS_NOT_ALLOWED, snode);

						cat.Assign(&script->code[snode->tokenPos+1], snode->tokenLength-2);
						ProcessStringConstant(cat, snode);
					}
					else if( snode->tokenType == ttHeredocStringConstant )
					{
						cat.Assign(&script->code[snode->tokenPos+3], snode->tokenLength-6);
						ProcessHeredocStringConstant(cat, snode);
					}

					str += cat;

					snode = snode->next;
				}

				// Call the string factory function to create a string object
				asCScriptFunction *descr = engine->stringFactory;
				if( descr == 0 )
				{
					// Error
					Error(TXT_STRINGS_NOT_RECOGNIZED, vnode);

					// Give dummy value
					ctx->type.SetDummy();
					return -1;
				}
				else
				{
					// Register the constant string with the engine
					int id = engine->AddConstantString(str.AddressOf(), str.GetLength());
					ctx->bc.InstrWORD(asBC_STR, (asWORD)id);

					bool useVariable = false;
					int stackOffset  = 0;

					if( descr->DoesReturnOnStack() )
					{
						useVariable = true;
						stackOffset = AllocateVariable(descr->returnType, true);
						ctx->bc.InstrSHORT(asBC_PSF, short(stackOffset));
					}

					PerformFunctionCall(descr->id, ctx, false, 0, 0, useVariable, stackOffset);
				}
			}
		}
		else if( vnode->tokenType == ttNull )
		{
			ctx->bc.Instr(asBC_PshNull);
			ctx->type.SetNullConstant();
		}
		else
			asASSERT(false);
	}
	else if( vnode->nodeType == snFunctionCall )
	{
		// Determine the scope resolution
		asCString scope = builder->GetScopeFromNode(vnode->firstChild, script);

		return CompileFunctionCall(vnode, ctx, 0, false, scope);
	}
	else if( vnode->nodeType == snConstructCall )
	{
		CompileConstructCall(vnode, ctx);
	}
	else if( vnode->nodeType == snAssignment )
	{
		asSExprContext e(engine);
		int r = CompileAssignment(vnode, &e);
		if( r < 0 )
		{
			ctx->type.SetDummy();
			return r;
		}
		MergeExprBytecodeAndType(ctx, &e);
	}
	else if( vnode->nodeType == snCast )
	{
		// Implement the cast operator
		CompileConversion(vnode, ctx);
	}
	else if( vnode->nodeType == snUndefined && vnode->tokenType == ttVoid )
	{
		// This is a void expression
		ctx->type.SetVoidExpression();
	}
	else
		asASSERT(false);

	return 0;
}

asUINT asCCompiler::ProcessStringConstant(asCString &cstr, asCScriptNode *node, bool processEscapeSequences)
{
	int charLiteral = -1;

	// Process escape sequences
	asCArray<char> str((int)cstr.GetLength());

	for( asUINT n = 0; n < cstr.GetLength(); n++ )
	{
#ifdef AS_DOUBLEBYTE_CHARSET
		// Double-byte charset is only allowed for ASCII and not UTF16 encoded strings
		if( (cstr[n] & 0x80) && engine->ep.scanner == 0 && engine->ep.stringEncoding != 1 )
		{
			// This is the lead character of a double byte character
			// include the trail character without checking it's value.
			str.PushLast(cstr[n]);
			n++;
			str.PushLast(cstr[n]);
			continue;
		}
#endif

		asUINT val;

		if( processEscapeSequences && cstr[n] == '\\' )
		{
			++n;
			if( n == cstr.GetLength() )
			{
				if( charLiteral == -1 ) charLiteral = 0;
				return charLiteral;
			}

			// Hexadecimal escape sequences will allow the construction of
			// invalid unicode sequences, but the string should also work as
			// a bytearray so we must support this. The code for working with
			// unicode text must be prepared to handle invalid unicode sequences
			if( cstr[n] == 'x' || cstr[n] == 'X' )
			{
				++n;
				if( n == cstr.GetLength() ) break;

				val = 0;
				int c = engine->ep.stringEncoding == 1 ? 4 : 2;
				for( ; c > 0 && n < cstr.GetLength(); c--, n++ )
				{
					if( cstr[n] >= '0' && cstr[n] <= '9' )
						val = val*16 +  cstr[n] - '0';
					else if( cstr[n] >= 'a' && cstr[n] <= 'f' )
						val = val*16 + cstr[n] - 'a' + 10;
					else if( cstr[n] >= 'A' && cstr[n] <= 'F' )
						val = val*16 + cstr[n] - 'A' + 10;
					else
						break;
				}

				// Rewind one, since the loop will increment it again
				n--;

				// Hexadecimal escape sequences produce exact value, even if it is not proper unicode chars
				if( engine->ep.stringEncoding == 0 )
				{
					str.PushLast((asBYTE)val);
				}
				else
				{
#ifndef AS_BIG_ENDIAN
					str.PushLast((asBYTE)val);
					str.PushLast((asBYTE)(val>>8));
#else
					str.PushLast((asBYTE)(val>>8));
					str.PushLast((asBYTE)val);
#endif
				}
				if( charLiteral == -1 ) charLiteral = val;
				continue;
			}
			else if( cstr[n] == 'u' || cstr[n] == 'U' )
			{
				// \u expects 4 hex digits
				// \U expects 8 hex digits
				bool expect2 = cstr[n] == 'u';
				int c = expect2 ? 4 : 8;

				val = 0;

				for( ; c > 0; c-- )
				{
					++n;
					if( n == cstr.GetLength() ) break;

					if( cstr[n] >= '0' && cstr[n] <= '9' )
						val = val*16 + cstr[n] - '0';
					else if( cstr[n] >= 'a' && cstr[n] <= 'f' )
						val = val*16 + cstr[n] - 'a' + 10;
					else if( cstr[n] >= 'A' && cstr[n] <= 'F' )
						val = val*16 + cstr[n] - 'A' + 10;
					else
						break;
				}

				if( c != 0 )
				{
					// Give warning about invalid code point
					// TODO: Need code position for warning
					asCString msg;
					msg.Format(TXT_INVALID_UNICODE_FORMAT_EXPECTED_d, expect2 ? 4 : 8);
					Warning(msg, node);
					continue;
				}
			}
			else
			{
				if( cstr[n] == '"' )
					val = '"';
				else if( cstr[n] == '\'' )
					val = '\'';
				else if( cstr[n] == 'n' )
					val = '\n';
				else if( cstr[n] == 'r' )
					val = '\r';
				else if( cstr[n] == 't' )
					val = '\t';
				else if( cstr[n] == '0' )
					val = '\0';
				else if( cstr[n] == '\\' )
					val = '\\';
				else
				{
					// Invalid escape sequence
					Warning(TXT_INVALID_ESCAPE_SEQUENCE, node);
					continue;
				}
			}
		}
		else
		{
			if( engine->ep.scanner == 1 && (cstr[n] & 0x80) )
			{
				unsigned int len;
				val = asStringDecodeUTF8(&cstr[n], &len);
				if( val == 0xFFFFFFFF )
				{
					// Incorrect UTF8 encoding. Use only the first byte
					// TODO: Need code position for warning
					Warning(TXT_INVALID_UNICODE_SEQUENCE_IN_SRC, node);
					val = (unsigned char)cstr[n];
				}
				else
					n += len-1;
			}
			else
				val = (unsigned char)cstr[n];
		}

		// Add the character to the final string
		char encodedValue[5];
		int len;
		if( engine->ep.scanner == 1 && engine->ep.stringEncoding == 0 )
		{
			// Convert to UTF8 encoded
			len = asStringEncodeUTF8(val, encodedValue);
		}
		else if( engine->ep.stringEncoding == 1 )
		{
			// Convert to 16bit wide character string (even if the script is scanned as ASCII)
			len = asStringEncodeUTF16(val, encodedValue);
		}
		else
		{
			// Do not convert ASCII characters
			encodedValue[0] = (asBYTE)val;
			len = 1;
		}

		if( len < 0 )
		{
			// Give warning about invalid code point
			// TODO: Need code position for warning
			Warning(TXT_INVALID_UNICODE_VALUE, node);
		}
		else
		{
			// Add the encoded value to the final string
			str.Concatenate(encodedValue, len);
			if( charLiteral == -1 ) charLiteral = val;
		}
	}

	cstr.Assign(str.AddressOf(), str.GetLength());
	return charLiteral;
}

void asCCompiler::ProcessHeredocStringConstant(asCString &str, asCScriptNode *node)
{
	// Remove first line if it only contains whitespace
	int start;
	for( start = 0; start < (int)str.GetLength(); start++ )
	{
		if( str[start] == '\n' )
		{
			// Remove the linebreak as well
			start++;
			break;
		}

		if( str[start] != ' '  &&
			str[start] != '\t' &&
			str[start] != '\r' )
		{
			// Don't remove anything
			start = 0;
			break;
		}
	}

	// Remove the line after the last line break if it only contains whitespaces
	int end;
	for( end = (int)str.GetLength() - 1; end >= 0; end-- )
	{
		if( str[end] == '\n' )
		{
			// Don't remove the last line break
			end++;
			break;
		}

		if( str[end] != ' '  &&
			str[end] != '\t' &&
			str[end] != '\r' )
		{
			// Don't remove anything
			end = (int)str.GetLength();
			break;
		}
	}

	if( end < 0 ) end = 0;

	asCString tmp;
	if( end > start )
		tmp.Assign(&str[start], end-start);

	ProcessStringConstant(tmp, node, false);

	str = tmp;
}

void asCCompiler::CompileConversion(asCScriptNode *node, asSExprContext *ctx)
{
	asSExprContext expr(engine);
	asCDataType to;
	bool anyErrors = false;
	EImplicitConv convType;
	if( node->nodeType == snConstructCall )
	{
		convType = asIC_EXPLICIT_VAL_CAST;

		// Verify that there is only one argument
		if( node->lastChild->firstChild == 0 ||
			node->lastChild->firstChild != node->lastChild->lastChild )
		{
			Error(TXT_ONLY_ONE_ARGUMENT_IN_CAST, node->lastChild);
			expr.type.SetDummy();
			anyErrors = true;
		}
		else
		{
			// Compile the expression
			int r = CompileAssignment(node->lastChild->firstChild, &expr);
			if( r < 0 )
				anyErrors = true;
		}

		// Determine the requested type
		to = builder->CreateDataTypeFromNode(node->firstChild, script, outFunc->nameSpace);
		to.MakeReadOnly(true); // Default to const
		asASSERT(to.IsPrimitive());
	}
	else
	{
		convType = asIC_EXPLICIT_REF_CAST;

		// Compile the expression
		int r = CompileAssignment(node->lastChild, &expr);
		if( r < 0 )
			anyErrors = true;

		// Determine the requested type
		to = builder->CreateDataTypeFromNode(node->firstChild, script, outFunc->nameSpace);
		to = builder->ModifyDataTypeFromNode(to, node->firstChild->next, script, 0, 0);

		// If the type support object handles, then use it
		if( to.SupportHandles() )
		{
			to.MakeHandle(true);
		}
		else if( !to.IsObjectHandle() )
		{
			// The cast<type> operator can only be used for reference casts
			Error(TXT_ILLEGAL_TARGET_TYPE_FOR_REF_CAST, node->firstChild);
			anyErrors = true;
		}
	}

	// Do not allow casting to non shared type if we're compiling a shared method
	if( outFunc->IsShared() &&
		to.GetObjectType() && !to.GetObjectType()->IsShared() )
	{
		asCString msg;
		msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, to.GetObjectType()->name.AddressOf());
		Error(msg, node);
		anyErrors = true;
	}

	if( anyErrors )
	{
		// Assume that the error can be fixed and allow the compilation to continue
		ctx->type.SetConstantDW(to, 0);
		return;
	}

	ProcessPropertyGetAccessor(&expr, node);

	// Don't allow any operators on expressions that take address of class method
	if( expr.IsClassMethod() )
	{
		Error(TXT_INVALID_OP_ON_METHOD, node);
		return;
	}

	// We don't want a reference for conversion casts
	if( convType == asIC_EXPLICIT_VAL_CAST && expr.type.dataType.IsReference() )
	{
		if( expr.type.dataType.IsObject() )
			Dereference(&expr, true);
		else
			ConvertToVariable(&expr);
	}

	ImplicitConversion(&expr, to, node, convType);

	IsVariableInitialized(&expr.type, node);

	// If no type conversion is really tried ignore it
	if( to == expr.type.dataType )
	{
		// This will keep information about constant type
		MergeExprBytecode(ctx, &expr);
		ctx->type = expr.type;
		return;
	}

	if( to.IsEqualExceptConst(expr.type.dataType) && to.IsPrimitive() )
	{
		MergeExprBytecode(ctx, &expr);
		ctx->type = expr.type;
		ctx->type.dataType.MakeReadOnly(true);
		return;
	}

	// The implicit conversion already does most of the conversions permitted,
	// here we'll only treat those conversions that require an explicit cast.

	bool conversionOK = false;
	if( !expr.type.isConstant && expr.type.dataType != asCDataType::CreatePrimitive(ttVoid, false) )
	{
		if( !expr.type.dataType.IsObject() )
			ConvertToTempVariable(&expr);

		if( to.IsObjectHandle() &&
			expr.type.dataType.IsObjectHandle() &&
			!(!to.IsHandleToConst() && expr.type.dataType.IsHandleToConst()) )
		{
			conversionOK = CompileRefCast(&expr, to, true, node);

			MergeExprBytecode(ctx, &expr);
			ctx->type = expr.type;
		}
	}

	if( conversionOK )
		return;

	// Conversion not available
	ctx->type.SetDummy();

	asCString strTo, strFrom;

	strTo = to.Format();
	strFrom = expr.type.dataType.Format();

	asCString msg;
	msg.Format(TXT_NO_CONVERSION_s_TO_s, strFrom.AddressOf(), strTo.AddressOf());

	Error(msg, node);
}

void asCCompiler::AfterFunctionCall(int funcID, asCArray<asSExprContext*> &args, asSExprContext *ctx, bool deferAll)
{
	asCScriptFunction *descr = builder->GetFunctionDescription(funcID);

	// Parameters that are sent by reference should be assigned
	// to the evaluated expression if it is an lvalue

	// Evaluate the arguments from last to first
	int n = (int)descr->parameterTypes.GetLength() - 1;
	for( ; n >= 0; n-- )
	{
		if( (descr->parameterTypes[n].IsReference() && (descr->inOutFlags[n] & asTM_OUTREF)) ||
		    (descr->parameterTypes[n].IsObject() && deferAll) )
		{
			asASSERT( !(descr->parameterTypes[n].IsReference() && (descr->inOutFlags[n] == asTM_OUTREF)) || args[n]->origExpr );

			// For &inout, only store the argument if it is for a temporary variable
			if( engine->ep.allowUnsafeReferences ||
				descr->inOutFlags[n] != asTM_INOUTREF || args[n]->type.isTemporary )
			{
				// Store the argument for later processing
				asSDeferredParam outParam;
				outParam.argNode = args[n]->exprNode;
				outParam.argType = args[n]->type;
				outParam.argInOutFlags = descr->inOutFlags[n];
				outParam.origExpr = args[n]->origExpr;

				ctx->deferredParams.PushLast(outParam);
			}
		}
		else
		{
			// Release the temporary variable now
			ReleaseTemporaryVariable(args[n]->type, &ctx->bc);
		}

		// Move the argument's deferred expressions over to the final expression
		for( asUINT m = 0; m < args[n]->deferredParams.GetLength(); m++ )
		{
			ctx->deferredParams.PushLast(args[n]->deferredParams[m]);
			args[n]->deferredParams[m].origExpr = 0;
		}
		args[n]->deferredParams.SetLength(0);
	}
}

void asCCompiler::ProcessDeferredParams(asSExprContext *ctx)
{
	if( isProcessingDeferredParams ) return;

	isProcessingDeferredParams = true;

	for( asUINT n = 0; n < ctx->deferredParams.GetLength(); n++ )
	{
		asSDeferredParam outParam = ctx->deferredParams[n];
		if( outParam.argInOutFlags < asTM_OUTREF ) // &in, or not reference
		{
			// Just release the variable
			ReleaseTemporaryVariable(outParam.argType, &ctx->bc);
		}
		else if( outParam.argInOutFlags == asTM_OUTREF )
		{
			asSExprContext *expr = outParam.origExpr;
			outParam.origExpr = 0;

			if( outParam.argType.dataType.IsObjectHandle() )
			{
				// Implicitly convert the value to a handle
				if( expr->type.dataType.IsObjectHandle() )
					expr->type.isExplicitHandle = true;
			}

			// Verify that the expression result in a lvalue, or a property accessor
			if( IsLValue(expr->type) || expr->property_get || expr->property_set )
			{
				asSExprContext rctx(engine);
				rctx.type = outParam.argType;
				if( rctx.type.dataType.IsPrimitive() )
					rctx.type.dataType.MakeReference(false);
				else
				{
					rctx.bc.InstrSHORT(asBC_PSF, outParam.argType.stackOffset);
					rctx.type.dataType.MakeReference(IsVariableOnHeap(outParam.argType.stackOffset));
					if( expr->type.isExplicitHandle )
						rctx.type.isExplicitHandle = true;
				}

				asSExprContext o(engine);
				DoAssignment(&o, expr, &rctx, outParam.argNode, outParam.argNode, ttAssignment, outParam.argNode);

				if( !o.type.dataType.IsPrimitive() ) o.bc.Instr(asBC_PopPtr);

				// The assignment may itself have resulted in a new temporary variable, e.g. if
				// the opAssign returns a non-reference. We must release this temporary variable
				// since it won't be used
				ReleaseTemporaryVariable(o.type, &o.bc);

				MergeExprBytecode(ctx, &o);
			}
			else
			{
				// We must still evaluate the expression
				MergeExprBytecode(ctx, expr);
				if( !expr->type.IsVoidExpression() && (!expr->type.isConstant || expr->type.IsNullConstant()) )
					ctx->bc.Instr(asBC_PopPtr);

				// Give a warning, except if the argument is void, null or 0 which indicate the argument is really to be ignored
				if( !expr->type.IsVoidExpression() && !expr->type.IsNullConstant() && !(expr->type.isConstant && expr->type.qwordValue == 0) )
					Warning(TXT_ARG_NOT_LVALUE, outParam.argNode);

				ReleaseTemporaryVariable(outParam.argType, &ctx->bc);
			}

			ReleaseTemporaryVariable(expr->type, &ctx->bc);

			// Delete the original expression context
			asDELETE(expr,asSExprContext);
		}
		else // &inout
		{
			if( outParam.argType.isTemporary )
				ReleaseTemporaryVariable(outParam.argType, &ctx->bc);
			else if( !outParam.argType.isVariable )
			{
				if( outParam.argType.dataType.IsObject() &&
					((outParam.argType.dataType.GetBehaviour()->addref &&
					  outParam.argType.dataType.GetBehaviour()->release) ||
					 (outParam.argType.dataType.GetObjectType()->flags & asOBJ_NOCOUNT)) )
				{
					// Release the object handle that was taken to guarantee the reference
					ReleaseTemporaryVariable(outParam.argType, &ctx->bc);
				}
			}
		}
	}

	ctx->deferredParams.SetLength(0);
	isProcessingDeferredParams = false;
}


void asCCompiler::CompileConstructCall(asCScriptNode *node, asSExprContext *ctx)
{
	// The first node is a datatype node
	asCString name;
	asCTypeInfo tempObj;
	bool onHeap = true;
	asCArray<int> funcs;

	// It is possible that the name is really a constructor
	asCDataType dt;
	dt = builder->CreateDataTypeFromNode(node->firstChild, script, outFunc->nameSpace);
	if( dt.IsPrimitive() )
	{
		// This is a cast to a primitive type
		CompileConversion(node, ctx);
		return;
	}

	// Do not allow constructing non-shared types in shared functions
	if( outFunc->IsShared() &&
		dt.GetObjectType() && !dt.GetObjectType()->IsShared() )
	{
		asCString msg;
		msg.Format(TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s, dt.GetObjectType()->name.AddressOf());
		Error(msg, node);
	}

	// Compile the arguments
	asCArray<asSExprContext *> args;
	asCArray<asCTypeInfo> temporaryVariables;
	if( CompileArgumentList(node->lastChild, args) >= 0 )
	{
		// Check for a value cast behaviour
		if( args.GetLength() == 1 && args[0]->type.dataType.GetObjectType() )
		{
			asSExprContext conv(engine);
			conv.type = args[0]->type;
			ImplicitConversion(&conv, dt, node->lastChild, asIC_EXPLICIT_VAL_CAST, false);

			if( conv.type.dataType.IsEqualExceptRef(dt) )
			{
				ImplicitConversion(args[0], dt, node->lastChild, asIC_EXPLICIT_VAL_CAST);

				ctx->bc.AddCode(&args[0]->bc);
				ctx->type = args[0]->type;

				asDELETE(args[0],asSExprContext);

				return;
			}
		}

		// Check for possible constructor/factory
		name = dt.Format();

		asSTypeBehaviour *beh = dt.GetBehaviour();

		if( !(dt.GetObjectType()->flags & asOBJ_REF) )
		{
			funcs = beh->constructors;

			// Value types and script types are allocated through the constructor
			tempObj.dataType = dt;
			tempObj.stackOffset = (short)AllocateVariable(dt, true);
			tempObj.dataType.MakeReference(true);
			tempObj.isTemporary = true;
			tempObj.isVariable = true;

			onHeap = IsVariableOnHeap(tempObj.stackOffset);

			// Push the address of the object on the stack
			if( onHeap )
				ctx->bc.InstrSHORT(asBC_VAR, tempObj.stackOffset);
		}
		else
		{
			funcs = beh->factories;
		}

		// Special case: Allow calling func(void) with a void expression.
		if( args.GetLength() == 1 && args[0]->type.dataType == asCDataType::CreatePrimitive(ttVoid, false) )
		{
			// Evaluate the expression before the function call
			MergeExprBytecode(ctx, args[0]);
			asDELETE(args[0],asSExprContext);
			args.SetLength(0);
		}

		// Special case: If this is an object constructor and there are no arguments use the default constructor.
		// If none has been registered, just allocate the variable and push it on the stack.
		if( args.GetLength() == 0 )
		{
			asSTypeBehaviour *beh = tempObj.dataType.GetBehaviour();
			if( beh && beh->construct == 0 && !(dt.GetObjectType()->flags & asOBJ_REF) )
			{
				// Call the default constructor
				ctx->type = tempObj;

				if( onHeap )
				{
					asASSERT(ctx->bc.GetLastInstr() == asBC_VAR);
					ctx->bc.RemoveLastInstr();
				}

				CallDefaultConstructor(tempObj.dataType, tempObj.stackOffset, IsVariableOnHeap(tempObj.stackOffset), &ctx->bc, node);

				// Push the reference on the stack
				ctx->bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);
				return;
			}
		}

		// Special case: If this is a construction of a delegate and the expression names an object method
		if( dt.GetFuncDef() && args.GetLength() == 1 && args[0]->methodName != "" )
		{
			// TODO: delegate: It is possible that the argument returns a function pointer already, in which
			//                 case no object delegate will be created, but instead a delegate for a function pointer
			//                 In theory a simple cast would be good in this case, but this is a construct call so it 
			//                 is expected that a new object is created.

			dt.MakeHandle(true);
			ctx->type.Set(dt);

			// The delegate must be able to hold on to a reference to the object
			if( !args[0]->type.dataType.SupportHandles() )
				Error(TXT_CANNOT_CREATE_DELEGATE_FOR_NOREF_TYPES, node);
			else
			{
				// Filter the available object methods to find the one that matches the func def
				asCObjectType *type = args[0]->type.dataType.GetObjectType();
				asCScriptFunction *bestMethod = 0;
				for( asUINT n = 0; n < type->methods.GetLength(); n++ )
				{
					asCScriptFunction *func = engine->scriptFunctions[type->methods[n]];
					
					if( func->name != args[0]->methodName )
						continue;

					// If the expression is for a const object, then only const methods should be accepted
					if( args[0]->type.dataType.IsReadOnly() && !func->IsReadOnly() )
						continue;

					if( func->IsSignatureExceptNameAndObjectTypeEqual(dt.GetFuncDef()) )
					{
						bestMethod = func;

						// If the expression is non-const the non-const overloaded method has priority
						if( args[0]->type.dataType.IsReadOnly() == func->IsReadOnly() )
							break;
					}
				}

				if( bestMethod )
				{
					// The object pointer is already on the stack
					MergeExprBytecode(ctx, args[0]);

					// Push the function pointer as an additional argument
					ctx->bc.InstrPTR(asBC_FuncPtr, bestMethod);

					// Call the factory function for the delegate
					asCArray<int> funcs;
					builder->GetFunctionDescriptions(DELEGATE_FACTORY, funcs, engine->nameSpaces[0]);
					asASSERT( funcs.GetLength() == 1 );
					ctx->bc.Call(asBC_CALLSYS , funcs[0], 2*AS_PTR_SIZE);

					// Store the returned delegate in a temporary variable
					int returnOffset = AllocateVariable(dt, true, false);
					dt.MakeReference(true);
					ctx->type.SetVariable(dt, returnOffset, true);
					ctx->bc.InstrSHORT(asBC_STOREOBJ, (short)returnOffset);

					// Push a reference to the temporary variable on the stack
					ctx->bc.InstrSHORT(asBC_PSF, (short)returnOffset);
				}
				else
				{
					asCString msg;
					msg.Format(TXT_NO_MATCHING_SIGNATURES_TO_s, dt.GetFuncDef()->GetDeclaration());
					Error(msg.AddressOf(), node);
				}
			}

			// Clean-up arg
			asDELETE(args[0],asSExprContext);
			return;
		}

		MatchFunctions(funcs, args, node, name.AddressOf(), NULL, false);

		if( funcs.GetLength() != 1 )
		{
			// The error was reported by MatchFunctions()

			// Dummy value
			ctx->type.SetDummy();
		}
		else
		{
			int r = asSUCCESS;

			// TODO: 2.28.1: Merge this with MakeFunctionCall

			// Add the default values for arguments not explicitly supplied
			asCScriptFunction *func = (funcs[0] & FUNC_IMPORTED) == 0 ? engine->scriptFunctions[funcs[0]] : 0;
			if( func && args.GetLength() < (asUINT)func->GetParamCount() )
				r = CompileDefaultArgs(node, args, func);

			if( r == asSUCCESS )
			{
				asCByteCode objBC(engine);

				PrepareFunctionCall(funcs[0], &ctx->bc, args);

				MoveArgsToStack(funcs[0], &ctx->bc, args, false);

				if( !(dt.GetObjectType()->flags & asOBJ_REF) )
				{
					// If the object is allocated on the stack, then call the constructor as a normal function
					if( onHeap )
					{
						int offset = 0;
						asCScriptFunction *descr = builder->GetFunctionDescription(funcs[0]);
						for( asUINT n = 0; n < args.GetLength(); n++ )
							offset += descr->parameterTypes[n].GetSizeOnStackDWords();

						ctx->bc.InstrWORD(asBC_GETREF, (asWORD)offset);
					}
					else
						ctx->bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);

					PerformFunctionCall(funcs[0], ctx, onHeap, &args, tempObj.dataType.GetObjectType());

					// Add tag that the object has been initialized
					ctx->bc.ObjInfo(tempObj.stackOffset, asOBJ_INIT);

					// The constructor doesn't return anything,
					// so we have to manually inform the type of
					// the return value
					ctx->type = tempObj;
					if( !onHeap )
						ctx->type.dataType.MakeReference(false);

					// Push the address of the object on the stack again
					ctx->bc.InstrSHORT(asBC_PSF, tempObj.stackOffset);
				}
				else
				{
					// Call the factory to create the reference type
					PerformFunctionCall(funcs[0], ctx, false, &args);
				}
			}
		}
	}
	else
	{
		// Failed to compile the argument list, set the result to the dummy type
		ctx->type.SetDummy();
	}

	// Cleanup
	for( asUINT n = 0; n < args.GetLength(); n++ )
		if( args[n] )
		{
			asDELETE(args[n],asSExprContext);
		}
}


int asCCompiler::CompileFunctionCall(asCScriptNode *node, asSExprContext *ctx, asCObjectType *objectType, bool objIsConst, const asCString &scope)
{
	asCString name;
	asCTypeInfo tempObj;
	asCArray<int> funcs;
	int localVar = -1;
	bool initializeMembers = false;

	asCScriptNode *nm = node->lastChild->prev;
	name.Assign(&script->code[nm->tokenPos], nm->tokenLength);

	// First check for a local variable of a function type as it would take precedence
	// Must not allow function names, nor global variables to be returned in this instance
	// If objectType is set then this is a post op expression and we shouldn't look for local variables
	asSExprContext funcPtr(engine);
	if( objectType == 0 )
	{
		localVar = CompileVariableAccess(name, scope, &funcPtr, node, true, true, true);
		if( localVar >= 0 && !funcPtr.type.dataType.GetFuncDef() && funcPtr.methodName == "" )
		{
			// The variable is not a function
			asCString msg;
			msg.Format(TXT_NOT_A_FUNC_s_IS_VAR, name.AddressOf());
			Error(msg, node);
			return -1;
		}

		// If the name matches a method name, then reset the indicator that nothing was found
		if( funcPtr.methodName != "" )
			localVar = -1;
	}

	if( localVar < 0 )
	{
		// If this is an expression post op, or if a class method is
		// being compiled, then we should look for matching class methods
		if( objectType || (outFunc && outFunc->objectType && scope != "::") )
		{
			// If we're compiling a constructor and the name of the function is super then
			// the constructor of the base class is being called.
			// super cannot be prefixed with a scope operator
			if( scope == "" && m_isConstructor && name == SUPER_TOKEN )
			{
				// If the class is not derived from anyone else, calling super should give an error
				if( outFunc && outFunc->objectType->derivedFrom )
					funcs = outFunc->objectType->derivedFrom->beh.constructors;

				// Must not allow calling base class' constructor multiple times
				if( continueLabels.GetLength() > 0 )
				{
					// If a continue label is set we are in a loop
					Error(TXT_CANNOT_CALL_CONSTRUCTOR_IN_LOOPS, node);
				}
				else if( breakLabels.GetLength() > 0 )
				{
					// TODO: inheritance: Should eventually allow constructors in switch statements
					// If a break label is set we are either in a loop or a switch statements
					Error(TXT_CANNOT_CALL_CONSTRUCTOR_IN_SWITCH, node);
				}
				else if( m_isConstructorCalled )
				{
					Error(TXT_CANNOT_CALL_CONSTRUCTOR_TWICE, node);
				}
				m_isConstructorCalled = true;

				// We need to initialize the class members, but only after all the deferred arguments have been completed
				initializeMembers = true;
			}
			else
			{
				// The scope is can be used to specify the base class
				builder->GetObjectMethodDescriptions(name.AddressOf(), objectType ? objectType : outFunc->objectType, funcs, objIsConst, scope);
			}

			// It is still possible that there is a class member of a function type
			if( funcs.GetLength() == 0 )
			{
				int r = CompileVariableAccess(name, scope, &funcPtr, node, true, true, true, objectType);
				if( r >= 0 && !funcPtr.type.dataType.GetFuncDef() && funcPtr.methodName == "" )
				{
					// The variable is not a function
					asCString msg;
					msg.Format(TXT_NOT_A_FUNC_s_IS_VAR, name.AddressOf());
					Error(msg, node);
					return -1;
				}
			}

			// If a class method is being called implicitly, then add the this pointer for the call
			if( funcs.GetLength() && !objectType )
			{
				objectType = outFunc->objectType;

				asCDataType dt = asCDataType::CreateObject(objectType, false);

				// The object pointer is located at stack position 0
				ctx->bc.InstrSHORT(asBC_PSF, 0);
				ctx->type.SetVariable(dt, 0, false);
				ctx->type.dataType.MakeReference(true);

				Dereference(ctx, true);
			}
		}

		// If it is not a class method or member function pointer,
		// then look for global functions or global function pointers,
		// unless this is an expression post op, incase only member
		// functions are expected
		if( objectType == 0 && funcs.GetLength() == 0 && funcPtr.type.dataType.GetFuncDef() == 0 )
		{
			// The scope is used to define the namespace
			asSNameSpace *ns = DetermineNameSpace(scope);
			if( ns )
			{
				// Search recursively in parent namespaces
				while( ns && funcs.GetLength() == 0 && funcPtr.type.dataType.GetFuncDef() == 0 )
				{
					builder->GetFunctionDescriptions(name.AddressOf(), funcs, ns);
					if( funcs.GetLength() == 0 )
					{
						int r = CompileVariableAccess(name, scope, &funcPtr, node, true, true);
						if( r >= 0 && !funcPtr.type.dataType.GetFuncDef() )
						{
							// The variable is not a function
							asCString msg;
							msg.Format(TXT_NOT_A_FUNC_s_IS_VAR, name.AddressOf());
							Error(msg, node);
							return -1;
						}
					}

					ns = builder->GetParentNameSpace(ns);
				}
			}
			else
			{
				asCString msg;
				msg.Format(TXT_NAMESPACE_s_DOESNT_EXIST, scope.AddressOf());
				Error(msg, node);
				return -1;
			}
		}
	}

	if( funcs.GetLength() == 0 && funcPtr.type.dataType.GetFuncDef() )
	{
		funcs.PushLast(funcPtr.type.dataType.GetFuncDef()->id);
	}

	// Compile the arguments
	asCArray<asSExprContext *> args;

	if( CompileArgumentList(node->lastChild, args) >= 0 )
	{
		// Special case: Allow calling func(void) with an expression that evaluates to no datatype, but isn't exactly 'void' 
		if( args.GetLength() == 1 && args[0]->type.dataType == asCDataType::CreatePrimitive(ttVoid, false) && !args[0]->type.IsVoidExpression() )
		{
			// Evaluate the expression before the function call
			MergeExprBytecode(ctx, args[0]);
			asDELETE(args[0],asSExprContext);
			args.SetLength(0);
		}

		MatchFunctions(funcs, args, node, name.AddressOf(), objectType, objIsConst, false, true, scope);

		if( funcs.GetLength() != 1 )
		{
			// The error was reported by MatchFunctions()

			// Dummy value
			ctx->type.SetDummy();
		}
		else
		{
			int r = asSUCCESS;

			// Add the default values for arguments not explicitly supplied
			asCScriptFunction *func = builder->GetFunctionDescription(funcs[0]);
			if( func && args.GetLength() < (asUINT)func->GetParamCount() )
			{
				// Make sure to use the real function for virtual functions
				asCScriptFunction *realFunc = func;
				if( realFunc->funcType == asFUNC_VIRTUAL )
					realFunc = objectType->virtualFunctionTable[realFunc->vfTableIdx];

				r = CompileDefaultArgs(node, args, realFunc);
			}

			// TODO: funcdef: Do we have to make sure the handle is stored in a temporary variable, or
			//                is it enough to make sure it is in a local variable?

			// For function pointer we must guarantee that the function is safe, i.e.
			// by first storing the function pointer in a local variable (if it isn't already in one)
			if( r == asSUCCESS )
			{
				if( func->funcType == asFUNC_FUNCDEF )
				{
					if( objectType && funcPtr.property_get <= 0 )
					{
						Dereference(ctx, true); // Dereference the object pointer to access the member

						// The actual function should be called as if a global function
						objectType = 0;
					}

					if( funcPtr.property_get > 0 )
					{
						ProcessPropertyGetAccessor(&funcPtr, node);
						Dereference(&funcPtr, true);

						// The function call will be made directly from the local variable so the function pointer shouldn't be on the stack
						funcPtr.bc.Instr(asBC_PopPtr);
					}
					else
					{
						Dereference(&funcPtr, true);
						ConvertToVariable(&funcPtr);

						// The function call will be made directly from the local variable so the function pointer shouldn't be on the stack
						if( !funcPtr.type.isTemporary )
							funcPtr.bc.Instr(asBC_PopPtr);
					}

					MergeExprBytecodeAndType(ctx, &funcPtr);
				}

				MakeFunctionCall(ctx, funcs[0], objectType, args, node, false, 0, funcPtr.type.stackOffset);

				// If the function pointer was copied to a local variable for the call, then
				// release it again (temporary local variable)
				if( (funcs[0] & FUNC_IMPORTED) == 0 && engine->scriptFunctions[funcs[0]]->funcType == asFUNC_FUNCDEF )
				{
					ReleaseTemporaryVariable(funcPtr.type, &ctx->bc);
				}
			}
		}
	}
	else
	{
		// Failed to compile the argument list, set the dummy type and continue compilation
		ctx->type.SetDummy();
	}

	// Cleanup
	for( asUINT n = 0; n < args.GetLength(); n++ )
		if( args[n] )
		{
			asDELETE(args[n],asSExprContext);
		}

	if( initializeMembers )
	{
		asASSERT( m_isConstructor );

		// Need to initialize members here, as they may use the properties of the base class
		// If there are multiple paths that call super(), then there will also be multiple
		// locations with initializations of the members. It is not possible to consolidate
		// these in one place, as the expressions for the initialization are evaluated where
		// they are compiled, which means that they may access different variables depending
		// on the scope where super() is called.
		// Members that don't have an explicit initialization expression will be initialized
		// beginning of the constructor as they are guaranteed not to use at the any
		// members of the base class.
		CompileMemberInitialization(&ctx->bc, false);
	}

	return 0;
}

asSNameSpace *asCCompiler::DetermineNameSpace(const asCString &scope)
{
	asSNameSpace *ns;

	if( scope == "" )
	{
		if( outFunc->nameSpace->name != "" )
			ns = outFunc->nameSpace;
		else if( outFunc->objectType && outFunc->objectType->nameSpace->name != "" )
			ns = outFunc->objectType->nameSpace;
		else
			ns = engine->nameSpaces[0];
	}
	else if( scope == "::" )
		ns = engine->nameSpaces[0];
	else
		ns = engine->FindNameSpace(scope.AddressOf());

	return ns;
}

int asCCompiler::CompileExpressionPreOp(asCScriptNode *node, asSExprContext *ctx)
{
	int op = node->tokenType;

	// Don't allow any prefix operators except handle on expressions that take address of class method
	if( ctx->IsClassMethod() && op != ttHandle )
	{
		Error(TXT_INVALID_OP_ON_METHOD, node);
		return -1;
	}

	// Don't allow any operators on void expressions
	if( ctx->type.IsVoidExpression() )
	{
		Error(TXT_VOID_CANT_BE_OPERAND, node);
		return -1;
	}

	IsVariableInitialized(&ctx->type, node);

	if( op == ttHandle )
	{
		if( ctx->methodName != "" )
		{
			// Don't allow taking the handle of a handle
			if( ctx->type.isExplicitHandle )
			{
				Error(TXT_OBJECT_HANDLE_NOT_SUPPORTED, node);
				return -1;
			}
		}
		else
		{
			// Don't allow taking handle of a handle, i.e. @@
			if( ctx->type.isExplicitHandle )
			{
				Error(TXT_OBJECT_HANDLE_NOT_SUPPORTED, node);
				return -1;
			}

			// @null is allowed even though it is implicit
			if( !ctx->type.IsNullConstant() )
			{
				// Verify that the type allow its handle to be taken
				if( !ctx->type.dataType.IsObject() ||
					!(((ctx->type.dataType.GetObjectType()->beh.addref && ctx->type.dataType.GetObjectType()->beh.release) || (ctx->type.dataType.GetObjectType()->flags & asOBJ_NOCOUNT)) ||
					  (ctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE)) )
				{
					Error(TXT_OBJECT_HANDLE_NOT_SUPPORTED, node);
					return -1;
				}

				// Objects that are not local variables are not references
				// Objects allocated on the stack are also not marked as references
				if( !ctx->type.dataType.IsReference() &&
					!(ctx->type.dataType.IsObject() && !ctx->type.isVariable) &&
					!(ctx->type.isVariable && !IsVariableOnHeap(ctx->type.stackOffset)) )
				{
					Error(TXT_NOT_VALID_REFERENCE, node);
					return -1;
				}

				// Convert the expression to a handle
				if( !ctx->type.dataType.IsObjectHandle() && !(ctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE) )
				{
					asCDataType to = ctx->type.dataType;
					to.MakeHandle(true);
					to.MakeReference(true);
					to.MakeHandleToConst(ctx->type.dataType.IsReadOnly());
					ImplicitConversion(ctx, to, node, asIC_IMPLICIT_CONV, true, false);

					asASSERT( ctx->type.dataType.IsObjectHandle() );
				}
				else if( ctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE )
				{
					// For the ASHANDLE type we'll simply set the expression as a handle
					ctx->type.dataType.MakeHandle(true);
				}
			}
		}

		// Mark the expression as an explicit handle to avoid implicit conversions to non-handle expressions
		ctx->type.isExplicitHandle = true;
	}
	else if( (op == ttMinus || op == ttPlus || op == ttBitNot || op == ttInc || op == ttDec) && ctx->type.dataType.IsObject() )
	{
		// Look for the appropriate method
		// There is no overloadable operator for unary plus
		const char *opName = 0;
		switch( op )
		{
		case ttMinus:  opName = "opNeg";    break;
		case ttBitNot: opName = "opCom";    break;
		case ttInc:    opName = "opPreInc"; break;
		case ttDec:    opName = "opPreDec"; break;
		}

		if( opName )
		{
			// TODO: Should convert this to something similar to CompileOverloadedDualOperator2
			ProcessPropertyGetAccessor(ctx, node);

			// Is it a const value?
			bool isConst = false;
			if( ctx->type.dataType.IsObjectHandle() )
				isConst = ctx->type.dataType.IsHandleToConst();
			else
				isConst = ctx->type.dataType.IsReadOnly();

			// TODO: If the value isn't const, then first try to find the non const method, and if not found try to find the const method

			// Find the correct method
			asCArray<int> funcs;
			asCObjectType *ot = ctx->type.dataType.GetObjectType();
			for( asUINT n = 0; n < ot->methods.GetLength(); n++ )
			{
				asCScriptFunction *func = engine->scriptFunctions[ot->methods[n]];
				if( func->name == opName &&
					func->parameterTypes.GetLength() == 0 &&
					(!isConst || func->isReadOnly) )
				{
					funcs.PushLast(func->id);
				}
			}

			// Did we find the method?
			if( funcs.GetLength() == 1 )
			{
				asCTypeInfo objType = ctx->type;
				asCArray<asSExprContext *> args;
				MakeFunctionCall(ctx, funcs[0], objType.dataType.GetObjectType(), args, node);
				ReleaseTemporaryVariable(objType, &ctx->bc);
				return 0;
			}
			else if( funcs.GetLength() == 0 )
			{
				asCString str;
				str = asCString(opName) + "()";
				if( isConst )
					str += " const";
				str.Format(TXT_FUNCTION_s_NOT_FOUND, str.AddressOf());
				Error(str, node);
				ctx->type.SetDummy();
				return -1;
			}
			else if( funcs.GetLength() > 1 )
			{
				Error(TXT_MORE_THAN_ONE_MATCHING_OP, node);
				PrintMatchingFuncs(funcs, node);

				ctx->type.SetDummy();
				return -1;
			}
		}
		else if( op == ttPlus )
		{
			Error(TXT_ILLEGAL_OPERATION, node);
			ctx->type.SetDummy();
			return -1;
		}
	}
	else if( op == ttPlus || op == ttMinus )
	{
		// This is only for primitives. Objects are treated in the above block

		// Make sure the type is a math type
		if( !(ctx->type.dataType.IsIntegerType()  ||
			  ctx->type.dataType.IsUnsignedType() ||
			  ctx->type.dataType.IsFloatType()    ||
			  ctx->type.dataType.IsDoubleType()     ) )
		{
			Error(TXT_ILLEGAL_OPERATION, node);
			return -1;
		}


		ProcessPropertyGetAccessor(ctx, node);

		asCDataType to = ctx->type.dataType;

		// TODO: The case -2147483648 gives an unecessary warning of changed sign for implicit conversion

		if( ctx->type.dataType.IsUnsignedType() )
		{
			if( ctx->type.dataType.GetSizeInMemoryBytes() == 1 )
				to = asCDataType::CreatePrimitive(ttInt8, false);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 2 )
				to = asCDataType::CreatePrimitive(ttInt16, false);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 4 )
				to = asCDataType::CreatePrimitive(ttInt, false);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 8 )
				to = asCDataType::CreatePrimitive(ttInt64, false);
			else
			{
				Error(TXT_INVALID_TYPE, node);
				return -1;
			}
		}

		if( ctx->type.dataType.IsReference() ) ConvertToVariable(ctx);
		ImplicitConversion(ctx, to, node, asIC_IMPLICIT_CONV);

		if( !ctx->type.isConstant )
		{
			ConvertToTempVariable(ctx);
			asASSERT(!ctx->type.isLValue);

			if( op == ttMinus )
			{
				if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
					ctx->bc.InstrSHORT(asBC_NEGi, ctx->type.stackOffset);
				else if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
					ctx->bc.InstrSHORT(asBC_NEGi64, ctx->type.stackOffset);
				else if( ctx->type.dataType.IsFloatType() )
					ctx->bc.InstrSHORT(asBC_NEGf, ctx->type.stackOffset);
				else if( ctx->type.dataType.IsDoubleType() )
					ctx->bc.InstrSHORT(asBC_NEGd, ctx->type.stackOffset);
				else
				{
					Error(TXT_ILLEGAL_OPERATION, node);
					return -1;
				}

				return 0;
			}
		}
		else
		{
			if( op == ttMinus )
			{
				if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
					ctx->type.intValue = -ctx->type.intValue;
				else if( ctx->type.dataType.IsIntegerType() && ctx->type.dataType.GetSizeInMemoryDWords() == 2 )
					ctx->type.qwordValue = -(asINT64)ctx->type.qwordValue;
				else if( ctx->type.dataType.IsFloatType() )
					ctx->type.floatValue = -ctx->type.floatValue;
				else if( ctx->type.dataType.IsDoubleType() )
					ctx->type.doubleValue = -ctx->type.doubleValue;
				else
				{
					Error(TXT_ILLEGAL_OPERATION, node);
					return -1;
				}

				return 0;
			}
		}
	}
	else if( op == ttNot )
	{
		if( ctx->type.dataType.IsEqualExceptRefAndConst(asCDataType::CreatePrimitive(ttBool, true)) )
		{
			if( ctx->type.isConstant )
			{
				ctx->type.dwordValue = (ctx->type.dwordValue == 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
				return 0;
			}

			ProcessPropertyGetAccessor(ctx, node);

			ConvertToTempVariable(ctx);
			asASSERT(!ctx->type.isLValue);

			ctx->bc.InstrSHORT(asBC_NOT, ctx->type.stackOffset);
		}
		else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
			return -1;
		}
	}
	else if( op == ttBitNot )
	{
		ProcessPropertyGetAccessor(ctx, node);

		asCDataType to = ctx->type.dataType;

		if( ctx->type.dataType.IsIntegerType() )
		{
			if( ctx->type.dataType.GetSizeInMemoryBytes() == 1 )
				to = asCDataType::CreatePrimitive(ttUInt8, false);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 2 )
				to = asCDataType::CreatePrimitive(ttUInt16, false);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 4 )
				to = asCDataType::CreatePrimitive(ttUInt, false);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 8 )
				to = asCDataType::CreatePrimitive(ttUInt64, false);
			else
			{
				Error(TXT_INVALID_TYPE, node);
				return -1;
			}
		}

		if( ctx->type.dataType.IsReference() ) ConvertToVariable(ctx);
		ImplicitConversion(ctx, to, node, asIC_IMPLICIT_CONV);

		if( ctx->type.dataType.IsUnsignedType() )
		{
			if( ctx->type.isConstant )
			{
				ctx->type.qwordValue = ~ctx->type.qwordValue;
				return 0;
			}

			ConvertToTempVariable(ctx);
			asASSERT(!ctx->type.isLValue);

			if( ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				ctx->bc.InstrSHORT(asBC_BNOT, ctx->type.stackOffset);
			else
				ctx->bc.InstrSHORT(asBC_BNOT64, ctx->type.stackOffset);
		}
		else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
			return -1;
		}
	}
	else if( op == ttInc || op == ttDec )
	{
		// Need a reference to the primitive that will be updated
		// The result of this expression is the same reference as before

		// Make sure the reference isn't a temporary variable
		if( ctx->type.isTemporary )
		{
			Error(TXT_REF_IS_TEMP, node);
			return -1;
		}
		if( ctx->type.dataType.IsReadOnly() )
		{
			Error(TXT_REF_IS_READ_ONLY, node);
			return -1;
		}
		if( ctx->property_get || ctx->property_set )
		{
			Error(TXT_INVALID_REF_PROP_ACCESS, node);
			return -1;
		}
		if( !ctx->type.isLValue )
		{
			Error(TXT_NOT_LVALUE, node);
			return -1;
		}

		if( ctx->type.isVariable && !ctx->type.dataType.IsReference() )
			ConvertToReference(ctx);
		else if( !ctx->type.dataType.IsReference() )
		{
			Error(TXT_NOT_VALID_REFERENCE, node);
			return -1;
		}

		if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt64, false)) ||
			ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt64, false)) )
		{
			if( op == ttInc )
				ctx->bc.Instr(asBC_INCi64);
			else
				ctx->bc.Instr(asBC_DECi64);
		}
		else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt, false)) ||
			ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt, false)) )
		{
			if( op == ttInc )
				ctx->bc.Instr(asBC_INCi);
			else
				ctx->bc.Instr(asBC_DECi);
		}
		else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt16, false)) ||
			     ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt16, false)) )
		{
			if( op == ttInc )
				ctx->bc.Instr(asBC_INCi16);
			else
				ctx->bc.Instr(asBC_DECi16);
		}
		else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt8, false)) ||
			     ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt8, false)) )
		{
			if( op == ttInc )
				ctx->bc.Instr(asBC_INCi8);
			else
				ctx->bc.Instr(asBC_DECi8);
		}
		else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttFloat, false)) )
		{
			if( op == ttInc )
				ctx->bc.Instr(asBC_INCf);
			else
				ctx->bc.Instr(asBC_DECf);
		}
		else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttDouble, false)) )
		{
			if( op == ttInc )
				ctx->bc.Instr(asBC_INCd);
			else
				ctx->bc.Instr(asBC_DECd);
		}
		else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
			return -1;
		}
	}
	else
	{
		// Unknown operator
		asASSERT(false);
		return -1;
	}

	return 0;
}

void asCCompiler::ConvertToReference(asSExprContext *ctx)
{
	if( ctx->type.isVariable && !ctx->type.dataType.IsReference() )
	{
		ctx->bc.InstrSHORT(asBC_LDV, ctx->type.stackOffset);
		ctx->type.dataType.MakeReference(true);
		ctx->type.SetVariable(ctx->type.dataType, ctx->type.stackOffset, ctx->type.isTemporary);
	}
}

int asCCompiler::FindPropertyAccessor(const asCString &name, asSExprContext *ctx, asCScriptNode *node, asSNameSpace *ns, bool isThisAccess)
{
	return FindPropertyAccessor(name, ctx, 0, node, ns, isThisAccess);
}

int asCCompiler::FindPropertyAccessor(const asCString &name, asSExprContext *ctx, asSExprContext *arg, asCScriptNode *node, asSNameSpace *ns, bool isThisAccess)
{
	if( engine->ep.propertyAccessorMode == 0 )
	{
		// Property accessors have been disabled by the application
		return 0;
	}

	int getId = 0, setId = 0;
	asCString getName = "get_" + name;
	asCString setName = "set_" + name;
	asCArray<int> multipleGetFuncs, multipleSetFuncs;

	if( ctx->type.dataType.IsObject() )
	{
		asASSERT( ns == 0 );

		// Don't look for property accessors in script classes if the script
		// property accessors have been disabled by the application
		if( !(ctx->type.dataType.GetObjectType()->flags & asOBJ_SCRIPT_OBJECT) ||
			engine->ep.propertyAccessorMode == 2 )
		{
			// Check if the object has any methods with the corresponding accessor name(s)
			asCObjectType *ot = ctx->type.dataType.GetObjectType();
			for( asUINT n = 0; n < ot->methods.GetLength(); n++ )
			{
				asCScriptFunction *f = engine->scriptFunctions[ot->methods[n]];
				// TODO: The type of the parameter should match the argument (unless the arg is a dummy)
				if( f->name == getName && (int)f->parameterTypes.GetLength() == (arg?1:0) )
				{
					if( getId == 0 )
						getId = ot->methods[n];
					else
					{
						if( multipleGetFuncs.GetLength() == 0 )
							multipleGetFuncs.PushLast(getId);

						multipleGetFuncs.PushLast(ot->methods[n]);
					}
				}
				// TODO: getset: If the parameter is a reference, it must not be an out reference. Should we allow inout ref?
				if( f->name == setName && (int)f->parameterTypes.GetLength() == (arg?2:1) )
				{
					if( setId == 0 )
						setId = ot->methods[n];
					else
					{
						if( multipleSetFuncs.GetLength() == 0 )
							multipleSetFuncs.PushLast(setId);

						multipleSetFuncs.PushLast(ot->methods[n]);
					}
				}
			}
		}
	}
	else
	{
		asASSERT( ns != 0 );

		// Look for appropriate global functions.
		asCArray<int> funcs;
		asUINT n;
		builder->GetFunctionDescriptions(getName.AddressOf(), funcs, ns);
		for( n = 0; n < funcs.GetLength(); n++ )
		{
			asCScriptFunction *f = builder->GetFunctionDescription(funcs[n]);
			// TODO: The type of the parameter should match the argument (unless the arg is a dummy)
			if( (int)f->parameterTypes.GetLength() == (arg?1:0) )
			{
				if( getId == 0 )
					getId = funcs[n];
				else
				{
					if( multipleGetFuncs.GetLength() == 0 )
						multipleGetFuncs.PushLast(getId);

					multipleGetFuncs.PushLast(funcs[n]);
				}
			}
		}

		funcs.SetLength(0);
		builder->GetFunctionDescriptions(setName.AddressOf(), funcs, ns);
		for( n = 0; n < funcs.GetLength(); n++ )
		{
			asCScriptFunction *f = builder->GetFunctionDescription(funcs[n]);
			// TODO: getset: If the parameter is a reference, it must not be an out reference. Should we allow inout ref?
			if( (int)f->parameterTypes.GetLength() == (arg?2:1) )
			{
				if( setId == 0 )
					setId = funcs[n];
				else
				{
					if( multipleSetFuncs.GetLength() == 0 )
						multipleSetFuncs.PushLast(getId);

					multipleSetFuncs.PushLast(funcs[n]);
				}
			}
		}
	}

	bool isConst = false;
	if( ctx->type.dataType.IsObjectHandle() )
		isConst = ctx->type.dataType.IsHandleToConst();
	else
		isConst = ctx->type.dataType.IsReadOnly();

	// Check for multiple matches
	if( multipleGetFuncs.GetLength() > 0 )
	{
		// Filter the list by constness
		FilterConst(multipleGetFuncs, !isConst);

		if( multipleGetFuncs.GetLength() > 1 )
		{
			asCString str;
			str.Format(TXT_MULTIPLE_PROP_GET_ACCESSOR_FOR_s, name.AddressOf());
			Error(str, node);

			PrintMatchingFuncs(multipleGetFuncs, node);

			return -1;
		}
		else
		{
			// The id may have changed
			getId = multipleGetFuncs[0];
		}
	}

	if( multipleSetFuncs.GetLength() > 0 )
	{
		// Filter the list by constness
		FilterConst(multipleSetFuncs, !isConst);

		if( multipleSetFuncs.GetLength() > 1 )
		{
			asCString str;
			str.Format(TXT_MULTIPLE_PROP_SET_ACCESSOR_FOR_s, name.AddressOf());
			Error(str, node);

			PrintMatchingFuncs(multipleSetFuncs, node);

			return -1;
		}
		else
		{
			// The id may have changed
			setId = multipleSetFuncs[0];
		}
	}

	// Check for type compatibility between get and set accessor
	if( getId && setId )
	{
		asCScriptFunction *getFunc = builder->GetFunctionDescription(getId);
		asCScriptFunction *setFunc = builder->GetFunctionDescription(setId);

		// It is permitted for a getter to return a handle and the setter to take a reference
		int idx = (arg?1:0);
		if( !getFunc->returnType.IsEqualExceptRefAndConst(setFunc->parameterTypes[idx]) &&
			!((getFunc->returnType.IsObjectHandle() && !setFunc->parameterTypes[idx].IsObjectHandle()) &&
			  (getFunc->returnType.GetObjectType() == setFunc->parameterTypes[idx].GetObjectType())) )
		{
			asCString str;
			str.Format(TXT_GET_SET_ACCESSOR_TYPE_MISMATCH_FOR_s, name.AddressOf());
			Error(str, node);

			asCArray<int> funcs;
			funcs.PushLast(getId);
			funcs.PushLast(setId);

			PrintMatchingFuncs(funcs, node);

			return -1;
		}
	}

	// Check if we are within one of the accessors
	int realGetId = getId;
	int realSetId = setId;
	if( outFunc->objectType && isThisAccess )
	{
		// The property accessors would be virtual functions, so we need to find the real implementation
		asCScriptFunction *getFunc = getId ? builder->GetFunctionDescription(getId) : 0;
		if( getFunc &&
			getFunc->funcType == asFUNC_VIRTUAL &&
			outFunc->objectType->DerivesFrom(getFunc->objectType) )
			realGetId = outFunc->objectType->virtualFunctionTable[getFunc->vfTableIdx]->id;
		asCScriptFunction *setFunc = setId ? builder->GetFunctionDescription(setId) : 0;
		if( setFunc &&
			setFunc->funcType == asFUNC_VIRTUAL &&
			outFunc->objectType->DerivesFrom(setFunc->objectType) )
			realSetId = outFunc->objectType->virtualFunctionTable[setFunc->vfTableIdx]->id;
	}

	// Avoid recursive call, by not treating this as a property accessor call.
	// This will also allow having the real property with the same name as the accessors.
	if( (isThisAccess || outFunc->objectType == 0) &&
		((realGetId && realGetId == outFunc->id) ||
		 (realSetId && realSetId == outFunc->id)) )
	{
		getId = 0;
		setId = 0;
	}

	// Check if the application has disabled script written property accessors
	if( engine->ep.propertyAccessorMode == 1 )
	{
		if( getId && builder->GetFunctionDescription(getId)->funcType != asFUNC_SYSTEM )
		  getId = 0;
		if( setId && builder->GetFunctionDescription(setId)->funcType != asFUNC_SYSTEM )
		  setId = 0;
	}

	if( getId || setId )
	{
		// Property accessors were found, but we don't know which is to be used yet, so
		// we just prepare the bytecode for the method call, and then store the function ids
		// so that the right one can be used when we get there.
		ctx->property_get = getId;
		ctx->property_set = setId;

		if( ctx->type.dataType.IsObject() )
		{
			// If the object is read-only then we need to remember that
			if( (!ctx->type.dataType.IsObjectHandle() && ctx->type.dataType.IsReadOnly()) ||
				(ctx->type.dataType.IsObjectHandle() && ctx->type.dataType.IsHandleToConst()) )
				ctx->property_const = true;
			else
				ctx->property_const = false;

			// If the object is a handle then we need to remember that
			ctx->property_handle = ctx->type.dataType.IsObjectHandle();
			ctx->property_ref    = ctx->type.dataType.IsReference();
		}

		// The setter's parameter type is used as the property type,
		// unless only the getter is available
		asCDataType dt;
		if( setId )
			dt = builder->GetFunctionDescription(setId)->parameterTypes[(arg?1:0)];
		else
			dt = builder->GetFunctionDescription(getId)->returnType;

		// Just change the type, the context must still maintain information
		// about previous variable offset and the indicator of temporary variable.
		int offset = ctx->type.stackOffset;
		bool isTemp = ctx->type.isTemporary;
		ctx->type.Set(dt);
		ctx->type.stackOffset = (short)offset;
		ctx->type.isTemporary = isTemp;
		ctx->exprNode = node;

		// Store the argument for later use
		if( arg )
		{
			ctx->property_arg = asNEW(asSExprContext)(engine);
			if( ctx->property_arg == 0 )
			{
				// Out of memory
				return -1;
			}

			MergeExprBytecodeAndType(ctx->property_arg, arg);
		}

		return 1;
	}

	// No accessor was found
	return 0;
}

int asCCompiler::ProcessPropertySetAccessor(asSExprContext *ctx, asSExprContext *arg, asCScriptNode *node)
{
	// TODO: A lot of this code is similar to ProcessPropertyGetAccessor. Can we unify them?

	if( !ctx->property_set )
	{
		Error(TXT_PROPERTY_HAS_NO_SET_ACCESSOR, node);
		return -1;
	}

	asCTypeInfo objType = ctx->type;
	asCScriptFunction *func = builder->GetFunctionDescription(ctx->property_set);

	// Make sure the arg match the property
	asCArray<int> funcs;
	funcs.PushLast(ctx->property_set);
	asCArray<asSExprContext *> args;
	if( ctx->property_arg )
		args.PushLast(ctx->property_arg);
	args.PushLast(arg);
	MatchFunctions(funcs, args, node, func->GetName(), func->objectType, ctx->property_const);
	if( funcs.GetLength() == 0 )
	{
		// MatchFunctions already reported the error
		if( ctx->property_arg )
		{
			asDELETE(ctx->property_arg, asSExprContext);
			ctx->property_arg = 0;
		}
		return -1;
	}

	if( func->objectType )
	{
		// Setup the context with the original type so the method call gets built correctly
		ctx->type.dataType = asCDataType::CreateObject(func->objectType, ctx->property_const);
		if( ctx->property_handle ) ctx->type.dataType.MakeHandle(true);
		if( ctx->property_ref )	ctx->type.dataType.MakeReference(true);

		// Don't allow the call if the object is read-only and the property accessor is not const
		if( ctx->property_const && !func->isReadOnly )
		{
			Error(TXT_NON_CONST_METHOD_ON_CONST_OBJ, node);
			asCArray<int> funcs;
			funcs.PushLast(ctx->property_set);
			PrintMatchingFuncs(funcs, node);
		}
	}

	// Call the accessor
	MakeFunctionCall(ctx, ctx->property_set, func->objectType, args, node);

	if( func->objectType )
	{
		// TODO: This is from CompileExpressionPostOp, can we unify the code?
		if( !objType.isTemporary ||
			!ctx->type.dataType.IsReference() ||
			ctx->type.isVariable ) // If the resulting type is a variable, then the reference is not a member
		{
			// As the method didn't return a reference to a member
			// we can safely release the original object now
			ReleaseTemporaryVariable(objType, &ctx->bc);
		}
	}

	ctx->property_get = 0;
	ctx->property_set = 0;
	if( ctx->property_arg )
	{
		asDELETE(ctx->property_arg, asSExprContext);
		ctx->property_arg = 0;
	}

	return 0;
}

void asCCompiler::ProcessPropertyGetAccessor(asSExprContext *ctx, asCScriptNode *node)
{
	// If no property accessor has been prepared then don't do anything
	if( !ctx->property_get && !ctx->property_set )
		return;

	if( !ctx->property_get )
	{
		// Raise error on missing accessor
		Error(TXT_PROPERTY_HAS_NO_GET_ACCESSOR, node);
		ctx->type.SetDummy();
		return;
	}

	asCTypeInfo objType = ctx->type;
	asCScriptFunction *func = builder->GetFunctionDescription(ctx->property_get);

	// Make sure the arg match the property
	asCArray<int> funcs;
	funcs.PushLast(ctx->property_get);
	asCArray<asSExprContext *> args;
	if( ctx->property_arg )
		args.PushLast(ctx->property_arg);
	MatchFunctions(funcs, args, node, func->GetName(), func->objectType, ctx->property_const);
	if( funcs.GetLength() == 0 )
	{
		// MatchFunctions already reported the error
		if( ctx->property_arg )
		{
			asDELETE(ctx->property_arg, asSExprContext);
			ctx->property_arg = 0;
		}
		ctx->type.SetDummy();
		return;
	}

	if( func->objectType )
	{
		// Setup the context with the original type so the method call gets built correctly
		ctx->type.dataType = asCDataType::CreateObject(func->objectType, ctx->property_const);
		if( ctx->property_handle ) ctx->type.dataType.MakeHandle(true);
		if( ctx->property_ref )	ctx->type.dataType.MakeReference(true);

		// Don't allow the call if the object is read-only and the property accessor is not const
		if( ctx->property_const && !func->isReadOnly )
		{
			Error(TXT_NON_CONST_METHOD_ON_CONST_OBJ, node);
			asCArray<int> funcs;
			funcs.PushLast(ctx->property_get);
			PrintMatchingFuncs(funcs, node);
		}
	}

	// Call the accessor
	MakeFunctionCall(ctx, ctx->property_get, func->objectType, args, node);

	if( func->objectType )
	{
		// TODO: This is from CompileExpressionPostOp, can we unify the code?

		// If the method returned a reference, then we can't release the original
		// object yet, because the reference may be to a member of it
		if( !objType.isTemporary ||
			!(ctx->type.dataType.IsReference() || (ctx->type.dataType.IsObject() && !ctx->type.dataType.IsObjectHandle())) ||
			ctx->type.isVariable ) // If the resulting type is a variable, then the reference is not a member
		{
			// As the method didn't return a reference to a member
			// we can safely release the original object now
			ReleaseTemporaryVariable(objType, &ctx->bc);
		}
	}

	ctx->property_get = 0;
	ctx->property_set = 0;
	if( ctx->property_arg )
	{
		asDELETE(ctx->property_arg, asSExprContext);
		ctx->property_arg = 0;
	}
}

int asCCompiler::CompileExpressionPostOp(asCScriptNode *node, asSExprContext *ctx)
{
	// Don't allow any postfix operators on expressions that take address of class method
	if( ctx->IsClassMethod() )
	{
		Error(TXT_INVALID_OP_ON_METHOD, node);
		return -1;
	}

	// Don't allow any operators on void expressions
	if( ctx->type.IsVoidExpression() )
	{
		Error(TXT_VOID_CANT_BE_OPERAND, node);
		return -1;
	}

	// Check if the variable is initialized (if it indeed is a variable)
	IsVariableInitialized(&ctx->type, node);

	int op = node->tokenType;
	if( (op == ttInc || op == ttDec) && ctx->type.dataType.IsObject() )
	{
		const char *opName = 0;
		switch( op )
		{
		case ttInc:    opName = "opPostInc"; break;
		case ttDec:    opName = "opPostDec"; break;
		}

		if( opName )
		{
			// TODO: Should convert this to something similar to CompileOverloadedDualOperator2
			ProcessPropertyGetAccessor(ctx, node);

			// Is it a const value?
			bool isConst = false;
			if( ctx->type.dataType.IsObjectHandle() )
				isConst = ctx->type.dataType.IsHandleToConst();
			else
				isConst = ctx->type.dataType.IsReadOnly();

			// TODO: If the value isn't const, then first try to find the non const method, and if not found try to find the const method

			// Find the correct method
			asCArray<int> funcs;
			asCObjectType *ot = ctx->type.dataType.GetObjectType();
			for( asUINT n = 0; n < ot->methods.GetLength(); n++ )
			{
				asCScriptFunction *func = engine->scriptFunctions[ot->methods[n]];
				if( func->name == opName &&
					func->parameterTypes.GetLength() == 0 &&
					(!isConst || func->isReadOnly) )
				{
					funcs.PushLast(func->id);
				}
			}

			// Did we find the method?
			if( funcs.GetLength() == 1 )
			{
				asCTypeInfo objType = ctx->type;
				asCArray<asSExprContext *> args;
				MakeFunctionCall(ctx, funcs[0], objType.dataType.GetObjectType(), args, node);
				ReleaseTemporaryVariable(objType, &ctx->bc);
				return 0;
			}
			else if( funcs.GetLength() == 0 )
			{
				asCString str;
				str = asCString(opName) + "()";
				if( isConst )
					str += " const";
				str.Format(TXT_FUNCTION_s_NOT_FOUND, str.AddressOf());
				Error(str, node);
				ctx->type.SetDummy();
				return -1;
			}
			else if( funcs.GetLength() > 1 )
			{
				Error(TXT_MORE_THAN_ONE_MATCHING_OP, node);
				PrintMatchingFuncs(funcs, node);

				ctx->type.SetDummy();
				return -1;
			}
		}
	}
	else if( op == ttInc || op == ttDec )
	{
		// Make sure the reference isn't a temporary variable
		if( ctx->type.isTemporary )
		{
			Error(TXT_REF_IS_TEMP, node);
			return -1;
		}
		if( ctx->type.dataType.IsReadOnly() )
		{
			Error(TXT_REF_IS_READ_ONLY, node);
			return -1;
		}
		if( ctx->property_get || ctx->property_set )
		{
			Error(TXT_INVALID_REF_PROP_ACCESS, node);
			return -1;
		}
		if( !ctx->type.isLValue )
		{
			Error(TXT_NOT_LVALUE, node);
			return -1;
		}

		if( ctx->type.isVariable && !ctx->type.dataType.IsReference() )
			ConvertToReference(ctx);
		else if( !ctx->type.dataType.IsReference() )
		{
			Error(TXT_NOT_VALID_REFERENCE, node);
			return -1;
		}

		// Copy the value to a temp before changing it
		ConvertToTempVariable(ctx);
		asASSERT(!ctx->type.isLValue);

		// Increment the value pointed to by the reference still in the register
		asEBCInstr iInc = asBC_INCi, iDec = asBC_DECi;
		if( ctx->type.dataType.IsDoubleType() )
		{
			iInc = asBC_INCd;
			iDec = asBC_DECd;
		}
		else if( ctx->type.dataType.IsFloatType() )
		{
			iInc = asBC_INCf;
			iDec = asBC_DECf;
		}
		else if( ctx->type.dataType.IsIntegerType() || ctx->type.dataType.IsUnsignedType() )
		{
			if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt16, false)) ||
			    ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt16, false)) )
			{
				iInc = asBC_INCi16;
				iDec = asBC_DECi16;
			}
			else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt8, false)) ||
			         ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt8, false)) )
			{
				iInc = asBC_INCi8;
				iDec = asBC_DECi8;
			}
			else if( ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttInt64, false)) ||
			         ctx->type.dataType.IsEqualExceptRef(asCDataType::CreatePrimitive(ttUInt64, false)) )
			{
				iInc = asBC_INCi64;
				iDec = asBC_DECi64;
			}
		}
		else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
			return -1;
		}

		if( op == ttInc ) ctx->bc.Instr(iInc); else ctx->bc.Instr(iDec);
	}
	else if( op == ttDot )
	{
		if( node->firstChild->nodeType == snIdentifier )
		{
			ProcessPropertyGetAccessor(ctx, node);

			// Get the property name
			asCString name(&script->code[node->firstChild->tokenPos], node->firstChild->tokenLength);

			if( ctx->type.dataType.IsObject() )
			{
				// We need to look for get/set property accessors.
				// If found, the context stores information on the get/set accessors
				// until it is known which is to be used.
				int r = 0;
				if( node->next && node->next->tokenType == ttOpenBracket )
				{
					// The property accessor should take an index arg
					asSExprContext dummyArg(engine);
					r = FindPropertyAccessor(name, ctx, &dummyArg, node, 0);
				}
				if( r == 0 )
					r = FindPropertyAccessor(name, ctx, node, 0);
				if( r != 0 )
					return r;

				if( !ctx->type.dataType.IsPrimitive() )
					Dereference(ctx, true);

				if( ctx->type.dataType.IsObjectHandle() )
				{
					// Convert the handle to a normal object
					asCDataType dt = ctx->type.dataType;
					dt.MakeHandle(false);

					ImplicitConversion(ctx, dt, node, asIC_IMPLICIT_CONV);

					// The handle may not have been an lvalue, but the dereferenced object is
					ctx->type.isLValue = true;
				}

				bool isConst = ctx->type.dataType.IsReadOnly();

				asCObjectProperty *prop = builder->GetObjectProperty(ctx->type.dataType, name.AddressOf());
				if( prop )
				{
					// Is the property access allowed?
					if( prop->isPrivate && (!outFunc || outFunc->objectType != ctx->type.dataType.GetObjectType()) )
					{
						asCString msg;
						msg.Format(TXT_PRIVATE_PROP_ACCESS_s, name.AddressOf());
						Error(msg, node);
					}

					// Put the offset on the stack
					ctx->bc.InstrSHORT_DW(asBC_ADDSi, (short)prop->byteOffset, engine->GetTypeIdFromDataType(asCDataType::CreateObject(ctx->type.dataType.GetObjectType(), false)));

					if( prop->type.IsReference() )
						ctx->bc.Instr(asBC_RDSPtr);

					// Reference to primitive must be stored in the temp register
					if( prop->type.IsPrimitive() )
					{
						ctx->bc.Instr(asBC_PopRPtr);
					}

					// Keep information about temporary variables as deferred expression
					if( ctx->type.isTemporary )
					{
						// Add the release of this reference, as a deferred expression
						asSDeferredParam deferred;
						deferred.origExpr = 0;
						deferred.argInOutFlags = asTM_INREF;
						deferred.argNode = 0;
						deferred.argType.SetVariable(ctx->type.dataType, ctx->type.stackOffset, true);

						ctx->deferredParams.PushLast(deferred);
					}

					// Set the new type and make sure it is not treated as a variable anymore
					ctx->type.dataType = prop->type;
					ctx->type.dataType.MakeReference(true);
					ctx->type.isVariable = false;
					ctx->type.isTemporary = false;

					if( ctx->type.dataType.IsObject() && !ctx->type.dataType.IsObjectHandle() )
					{
						// Objects that are members are not references
						ctx->type.dataType.MakeReference(false);
					}

					ctx->type.dataType.MakeReadOnly(isConst ? true : prop->type.IsReadOnly());
				}
				else
				{
					// If the name is not a property, the compiler must check if the name matches
					// a method, which can be used for constructing delegates
					asIScriptFunction *func = 0;
					asCObjectType *ot = ctx->type.dataType.GetObjectType();
					for( asUINT n = 0; n < ot->methods.GetLength(); n++ )
					{
						if( engine->scriptFunctions[ot->methods[n]]->name == name )
						{
							func = engine->scriptFunctions[ot->methods[n]];
							break;
						}
					}

					if( func )
					{
						// An object method was found. Keep the name of the method in the expression, but
						// don't actually modify the bytecode at this point since it is not yet known what 
						// the method will be used for, or even what overloaded method should be used.
						ctx->methodName = name;
					}
					else
					{
						asCString str;
						str.Format(TXT_s_NOT_MEMBER_OF_s, name.AddressOf(), ctx->type.dataType.Format().AddressOf());
						Error(str, node);
						return -1;
					}
				}
			}
			else
			{
				asCString str;
				str.Format(TXT_s_NOT_MEMBER_OF_s, name.AddressOf(), ctx->type.dataType.Format().AddressOf());
				Error(str, node);
				return -1;
			}
		}
		else
		{
			// Make sure it is an object we are accessing
			if( !ctx->type.dataType.IsObject() )
			{
				asCString str;
				str.Format(TXT_ILLEGAL_OPERATION_ON_s, ctx->type.dataType.Format().AddressOf());
				Error(str, node);
				return -1;
			}

			// Process the get property accessor
			ProcessPropertyGetAccessor(ctx, node);

			bool isConst = false;
			if( ctx->type.dataType.IsObjectHandle() )
				isConst = ctx->type.dataType.IsHandleToConst();
			else
				isConst = ctx->type.dataType.IsReadOnly();

			asCObjectType *trueObj = ctx->type.dataType.GetObjectType();

			asCTypeInfo objType = ctx->type;

			// Compile function call
			int r = CompileFunctionCall(node->firstChild, ctx, trueObj, isConst);
			if( r < 0 ) return r;

			// If the method returned a reference, then we can't release the original
			// object yet, because the reference may be to a member of it
			if( !objType.isTemporary ||
				!(ctx->type.dataType.IsReference() || (ctx->type.dataType.IsObject() && !ctx->type.dataType.IsObjectHandle())) ||
				ctx->type.isVariable ) // If the resulting type is a variable, then the reference is not a member
			{
				// As the method didn't return a reference to a member
				// we can safely release the original object now
				ReleaseTemporaryVariable(objType, &ctx->bc);
			}
		}
	}
	else if( op == ttOpenBracket )
	{
		// If the property access takes an index arg and the argument hasn't been evaluated yet,
		// then we should use that instead of processing it now. If the argument has already been
		// evaluated, then we should process the property accessor as a get access now as the new
		// index operator is on the result of that accessor.
		asCString propertyName;
		asSNameSpace *ns = 0;
		if( ((ctx->property_get && builder->GetFunctionDescription(ctx->property_get)->GetParamCount() == 1) ||
			 (ctx->property_set && builder->GetFunctionDescription(ctx->property_set)->GetParamCount() == 2)) &&
			(ctx->property_arg && ctx->property_arg->type.dataType.GetTokenType() == ttUnrecognizedToken) )
		{
			// Determine the name of the property accessor
			asCScriptFunction *func = 0;
			if( ctx->property_get )
				func = builder->GetFunctionDescription(ctx->property_get);
			else
				func = builder->GetFunctionDescription(ctx->property_set);
			propertyName = func->GetName();
			propertyName = propertyName.SubString(4);

			// Set the original type of the expression so we can re-evaluate the property accessor
			if( func->objectType )
			{
				ctx->type.dataType = asCDataType::CreateObject(func->objectType, ctx->property_const);
				if( ctx->property_handle ) ctx->type.dataType.MakeHandle(true);
				if( ctx->property_ref )	ctx->type.dataType.MakeReference(true);
			}
			else
			{
				// Store the namespace where the function is declared
				// so the same function can be found later
				ctx->type.SetDummy();
				ns = func->nameSpace;
			}

			ctx->property_get = ctx->property_set = 0;
			if( ctx->property_arg )
			{
				asDELETE(ctx->property_arg, asSExprContext);
				ctx->property_arg = 0;
			}
		}
		else
		{
			if( !ctx->type.dataType.IsObject() )
			{
				asCString str;
				str.Format(TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP, ctx->type.dataType.Format().AddressOf());
				Error(str, node);
				return -1;
			}

			ProcessPropertyGetAccessor(ctx, node);
		}

		Dereference(ctx, true);

		// Compile the expression
		asSExprContext expr(engine);
		CompileAssignment(node->firstChild, &expr);

		// Check for the existence of the opIndex method
		asSExprContext lctx(engine);
		MergeExprBytecodeAndType(&lctx, ctx);
		int r = 0;
		if( propertyName == "" )
			r = CompileOverloadedDualOperator2(node, "opIndex", &lctx, &expr, ctx);
		if( r == 0 )
		{
			// Check for accessors methods for the opIndex
			r = FindPropertyAccessor(propertyName == "" ? "opIndex" : propertyName.AddressOf(), &lctx, &expr, node, ns);
			if( r == 0 )
			{
				asCString str;
				str.Format(TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP, ctx->type.dataType.Format().AddressOf());
				Error(str, node);
				return -1;
			}
			else if( r < 0 )
				return -1;

			MergeExprBytecodeAndType(ctx, &lctx);
		}
	}
	else if( op == ttOpenParanthesis )
	{
		// TODO: Most of this is already done by CompileFunctionCall(). Can we share the code?

		// Make sure the expression is a funcdef
		if( !ctx->type.dataType.GetFuncDef() )
		{
			Error(TXT_EXPR_DOESNT_EVAL_TO_FUNC, node);
			return -1;
		}

		// Compile arguments
		asCArray<asSExprContext *> args;
		if( CompileArgumentList(node->lastChild, args) >= 0 )
		{
			// Match arguments with the funcdef
			asCArray<int> funcs;
			funcs.PushLast(ctx->type.dataType.GetFuncDef()->id);
			MatchFunctions(funcs, args, node, ctx->type.dataType.GetFuncDef()->name.AddressOf());

			if( funcs.GetLength() != 1 )
			{
				// The error was reported by MatchFunctions()

				// Dummy value
				ctx->type.SetDummy();
			}
			else
			{
				int r = asSUCCESS;

				// Add the default values for arguments not explicitly supplied
				asCScriptFunction *func = (funcs[0] & FUNC_IMPORTED) == 0 ? engine->scriptFunctions[funcs[0]] : 0;
				if( func && args.GetLength() < (asUINT)func->GetParamCount() )
					r = CompileDefaultArgs(node, args, func);

				// TODO: funcdef: Do we have to make sure the handle is stored in a temporary variable, or
				//                is it enough to make sure it is in a local variable?

				// For function pointer we must guarantee that the function is safe, i.e.
				// by first storing the function pointer in a local variable (if it isn't already in one)
				if( r == asSUCCESS )
				{
					Dereference(ctx, true);
					if( !ctx->type.isVariable )
						ConvertToVariable(ctx);
					else
					{
						// Remove the reference from the stack as the asBC_CALLPTR instruction takes the variable as argument
						ctx->bc.Instr(asBC_PopPtr);
					}
					asCTypeInfo t = ctx->type;

					MakeFunctionCall(ctx, funcs[0], 0, args, node, false, 0, ctx->type.stackOffset);

					ReleaseTemporaryVariable(t, &ctx->bc);
				}
			}
		}
		else
			ctx->type.SetDummy();

		// Cleanup
		for( asUINT n = 0; n < args.GetLength(); n++ )
			if( args[n] )
			{
				asDELETE(args[n],asSExprContext);
			}
	}

	return 0;
}

int asCCompiler::GetPrecedence(asCScriptNode *op)
{
	// x ** y
	// x * y, x / y, x % y
	// x + y, x - y
	// x <= y, x < y, x >= y, x > y
	// x = =y, x != y, x xor y, x is y, x !is y
	// x and y
	// x or y

	// The following are not used in this function,
	// but should have lower precedence than the above
	// x ? y : z
	// x = y

	// The expression term have the highest precedence
	if( op->nodeType == snExprTerm )
		return 1;

	// Evaluate operators by token
	int tokenType = op->tokenType;
	if( tokenType == ttStarStar )
		return 0;

	if( tokenType == ttStar || tokenType == ttSlash || tokenType == ttPercent )
		return -1;

	if( tokenType == ttPlus || tokenType == ttMinus )
		return -2;

	if( tokenType == ttBitShiftLeft ||
		tokenType == ttBitShiftRight ||
		tokenType == ttBitShiftRightArith )
		return -3;

	if( tokenType == ttAmp )
		return -4;

	if( tokenType == ttBitXor )
		return -5;

	if( tokenType == ttBitOr )
		return -6;

	if( tokenType == ttLessThanOrEqual ||
		tokenType == ttLessThan ||
		tokenType == ttGreaterThanOrEqual ||
		tokenType == ttGreaterThan )
		return -7;

	if( tokenType == ttEqual || tokenType == ttNotEqual || tokenType == ttXor || tokenType == ttIs || tokenType == ttNotIs )
		return -8;

	if( tokenType == ttAnd )
		return -9;

	if( tokenType == ttOr )
		return -10;

	// Unknown operator
	asASSERT(false);

	return 0;
}

asUINT asCCompiler::MatchArgument(asCArray<int> &funcs, asCArray<asSOverloadCandidate> &matches, const asSExprContext *argExpr, int paramNum, bool allowObjectConstruct)
{
	matches.SetLength(0);

	for( asUINT n = 0; n < funcs.GetLength(); n++ )
	{
		asCScriptFunction *desc = builder->GetFunctionDescription(funcs[n]);

		// Does the function have arguments enough?
		if( (int)desc->parameterTypes.GetLength() <= paramNum )
			continue;

		// void expressions can match any out parameter, but nothing else
		if( argExpr->type.IsVoidExpression() )
		{
			if( desc->inOutFlags[paramNum] == asTM_OUTREF )
				matches.PushLast(asSOverloadCandidate(funcs[n], 0));
			continue;
		}

		// Can we make the match by implicit conversion?
		asSExprContext ti(engine);
		ti.type = argExpr->type;
		ti.methodName = argExpr->methodName;
		ti.enumValue = argExpr->enumValue;
		if( argExpr->type.dataType.IsPrimitive() ) ti.type.dataType.MakeReference(false);
		asUINT cost = ImplicitConversion(&ti, desc->parameterTypes[paramNum], 0, asIC_IMPLICIT_CONV, false, allowObjectConstruct);

		// If the function parameter is an inout-reference then it must not be possible to call the
		// function with an incorrect argument type, even though the type can normally be converted.
		if( desc->parameterTypes[paramNum].IsReference() &&
			desc->inOutFlags[paramNum] == asTM_INOUTREF &&
			desc->parameterTypes[paramNum].GetTokenType() != ttQuestion )
		{
			// Observe, that the below checks are only necessary for when unsafe references have been
			// enabled by the application. Without this the &inout reference form wouldn't be allowed
			// for these value types.

			// Don't allow a primitive to be converted to a reference of another primitive type
			if( desc->parameterTypes[paramNum].IsPrimitive() &&
				desc->parameterTypes[paramNum].GetTokenType() != argExpr->type.dataType.GetTokenType() )
			{
				asASSERT( engine->ep.allowUnsafeReferences );
				continue;
			}

			// Don't allow an enum to be converted to a reference of another enum type
			if( desc->parameterTypes[paramNum].IsEnumType() &&
				desc->parameterTypes[paramNum].GetObjectType() != argExpr->type.dataType.GetObjectType() )
			{
				asASSERT( engine->ep.allowUnsafeReferences );
				continue;
			}

			// Don't allow a non-handle expression to be converted to a reference to a handle
			if( desc->parameterTypes[paramNum].IsObjectHandle() &&
				!argExpr->type.dataType.IsObjectHandle() )
			{
				asASSERT( engine->ep.allowUnsafeReferences );
				continue;
			}

			// Don't allow a value type to be converted
			if( (desc->parameterTypes[paramNum].GetObjectType() && (desc->parameterTypes[paramNum].GetObjectType()->GetFlags() & asOBJ_VALUE)) &&
				(desc->parameterTypes[paramNum].GetObjectType() != argExpr->type.dataType.GetObjectType()) )
			{
				asASSERT( engine->ep.allowUnsafeReferences );
				continue;
			}
		}

		// How well does the argument match the function parameter?
		if( desc->parameterTypes[paramNum].IsEqualExceptRef(ti.type.dataType) )
			matches.PushLast(asSOverloadCandidate(funcs[n], cost));
	}

	return (asUINT)matches.GetLength();
}

void asCCompiler::PrepareArgument2(asSExprContext *ctx, asSExprContext *arg, asCDataType *paramType, bool isFunction, int refType, bool isMakingCopy)
{
	// Reference parameters whose value won't be used don't evaluate the expression
	if( paramType->IsReference() && !(refType & asTM_INREF) )
	{
		// Store the original bytecode so that it can be reused when processing the deferred output parameter
		asSExprContext *orig = asNEW(asSExprContext)(engine);
		if( orig == 0 )
		{
			// Out of memory
			return;
		}
		MergeExprBytecodeAndType(orig, arg);
		arg->origExpr = orig;
	}

	PrepareArgument(paramType, arg, arg->exprNode, isFunction, refType, isMakingCopy);

	// arg still holds the original expression for output parameters
	ctx->bc.AddCode(&arg->bc);
}

bool asCCompiler::CompileOverloadedDualOperator(asCScriptNode *node, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx)
{
	DetermineSingleFunc(lctx, node);
	DetermineSingleFunc(rctx, node);

	ctx->exprNode = node;

	// What type of operator is it?
	int token = node->tokenType;
	if( token == ttUnrecognizedToken )
	{
		// This happens when the compiler is inferring an assignment
		// operation from another action, for example in preparing a value
		// as a function argument
		token = ttAssignment;
	}

	// boolean operators are not overloadable
	if( token == ttAnd ||
		token == ttOr ||
		token == ttXor )
		return false;

	// Dual operators can also be implemented as class methods
	if( token == ttEqual ||
		token == ttNotEqual )
	{
		// TODO: Should evaluate which of the two have the best match. If both have equal match, the first version should be used
		// Find the matching opEquals method
		int r = CompileOverloadedDualOperator2(node, "opEquals", lctx, rctx, ctx, true, asCDataType::CreatePrimitive(ttBool, false));
		if( r == 0 )
		{
			// Try again by switching the order of the operands
			r = CompileOverloadedDualOperator2(node, "opEquals", rctx, lctx, ctx, true, asCDataType::CreatePrimitive(ttBool, false));
		}

		if( r == 1 )
		{
			if( token == ttNotEqual )
				ctx->bc.InstrSHORT(asBC_NOT, ctx->type.stackOffset);

			// Success, don't continue
			return true;
		}
		else if( r < 0 )
		{
			// Compiler error, don't continue
			ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), true);
			return true;
		}
	}

	if( token == ttEqual ||
		token == ttNotEqual ||
		token == ttLessThan ||
		token == ttLessThanOrEqual ||
		token == ttGreaterThan ||
		token == ttGreaterThanOrEqual )
	{
		bool swappedOrder = false;

		// TODO: Should evaluate which of the two have the best match. If both have equal match, the first version should be used
		// Find the matching opCmp method
		int r = CompileOverloadedDualOperator2(node, "opCmp", lctx, rctx, ctx, true, asCDataType::CreatePrimitive(ttInt, false));
		if( r == 0 )
		{
			// Try again by switching the order of the operands
			swappedOrder = true;
			r = CompileOverloadedDualOperator2(node, "opCmp", rctx, lctx, ctx, true, asCDataType::CreatePrimitive(ttInt, false));
		}

		if( r == 1 )
		{
			ReleaseTemporaryVariable(ctx->type, &ctx->bc);

			int a = AllocateVariable(asCDataType::CreatePrimitive(ttBool, false), true);

			ctx->bc.InstrW_DW(asBC_CMPIi, ctx->type.stackOffset, 0);

			if( token == ttEqual )
				ctx->bc.Instr(asBC_TZ);
			else if( token == ttNotEqual )
				ctx->bc.Instr(asBC_TNZ);
			else if( (token == ttLessThan && !swappedOrder) ||
				     (token == ttGreaterThan && swappedOrder) )
				ctx->bc.Instr(asBC_TS);
			else if( (token == ttLessThanOrEqual && !swappedOrder) ||
				     (token == ttGreaterThanOrEqual && swappedOrder) )
				ctx->bc.Instr(asBC_TNP);
			else if( (token == ttGreaterThan && !swappedOrder) ||
				     (token == ttLessThan && swappedOrder) )
				ctx->bc.Instr(asBC_TP);
			else if( (token == ttGreaterThanOrEqual && !swappedOrder) ||
				     (token == ttLessThanOrEqual && swappedOrder) )
				ctx->bc.Instr(asBC_TNS);

			ctx->bc.InstrSHORT(asBC_CpyRtoV4, (short)a);

			ctx->type.SetVariable(asCDataType::CreatePrimitive(ttBool, false), a, true);

			// Success, don't continue
			return true;
		}
		else if( r < 0 )
		{
			// Compiler error, don't continue
			ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), true);
			return true;
		}
	}

	// The rest of the operators are not commutative, and doesn't require specific return type
	const char *op = 0, *op_r = 0;
	switch( token )
	{
	case ttPlus:               op = "opAdd";  op_r = "opAdd_r";  break;
	case ttMinus:              op = "opSub";  op_r = "opSub_r";  break;
	case ttStar:               op = "opMul";  op_r = "opMul_r";  break;
	case ttSlash:              op = "opDiv";  op_r = "opDiv_r";  break;
	case ttPercent:            op = "opMod";  op_r = "opMod_r";  break;
	case ttStarStar:           op = "opPow";  op_r = "opPow_r";  break;
	case ttBitOr:              op = "opOr";   op_r = "opOr_r";   break;
	case ttAmp:                op = "opAnd";  op_r = "opAnd_r";  break;
	case ttBitXor:             op = "opXor";  op_r = "opXor_r";  break;
	case ttBitShiftLeft:       op = "opShl";  op_r = "opShl_r";  break;
	case ttBitShiftRight:      op = "opShr";  op_r = "opShr_r";  break;
	case ttBitShiftRightArith: op = "opUShr"; op_r = "opUShr_r"; break;
	}

	// TODO: Might be interesting to support a concatenation operator, e.g. ~

	if( op && op_r )
	{
		// TODO: Should evaluate which of the two have the best match. If both have equal match, the first version should be used
		// Find the matching operator method
		int r = CompileOverloadedDualOperator2(node, op, lctx, rctx, ctx);
		if( r == 0 )
		{
			// Try again by switching the order of the operands, and using the reversed operator
			r = CompileOverloadedDualOperator2(node, op_r, rctx, lctx, ctx);
		}

		if( r == 1 )
		{
			// Success, don't continue
			return true;
		}
		else if( r < 0 )
		{
			// Compiler error, don't continue
			ctx->type.SetDummy();
			return true;
		}
	}

	// Assignment operators
	op = 0;
	switch( token )
	{
	case ttAssignment:        op = "opAssign";     break;
	case ttAddAssign:         op = "opAddAssign";  break;
	case ttSubAssign:         op = "opSubAssign";  break;
	case ttMulAssign:         op = "opMulAssign";  break;
	case ttDivAssign:         op = "opDivAssign";  break;
	case ttModAssign:         op = "opModAssign";  break;
	case ttPowAssign:         op = "opPowAssign";  break;
	case ttOrAssign:          op = "opOrAssign";   break;
	case ttAndAssign:         op = "opAndAssign";  break;
	case ttXorAssign:         op = "opXorAssign";  break;
	case ttShiftLeftAssign:   op = "opShlAssign";  break;
	case ttShiftRightLAssign: op = "opShrAssign";  break;
	case ttShiftRightAAssign: op = "opUShrAssign"; break;
	}

	if( op )
	{
		if( builder->engine->ep.disallowValueAssignForRefType && 
			lctx->type.dataType.GetObjectType() && (lctx->type.dataType.GetObjectType()->flags & asOBJ_REF) && !(lctx->type.dataType.GetObjectType()->flags & asOBJ_SCOPED) )
		{
			if( token == ttAssignment )
				Error(TXT_DISALLOW_ASSIGN_ON_REF_TYPE, node);
			else
				Error(TXT_DISALLOW_COMPOUND_ASSIGN_ON_REF_TYPE, node);

			// Set a dummy output
			ctx->type.Set(lctx->type.dataType);
			return true;
		}

		// TODO: Shouldn't accept const lvalue with the assignment operators

		// Find the matching operator method
		int r = CompileOverloadedDualOperator2(node, op, lctx, rctx, ctx);
		if( r == 1 )
		{
			// Success, don't continue
			return true;
		}
		else if( r < 0 )
		{
			// Compiler error, don't continue
			ctx->type.SetDummy();
			return true;
		}
	}

	// No suitable operator was found
	return false;
}

// Returns negative on compile error
//         zero on no matching operator
//         one on matching operator
int asCCompiler::CompileOverloadedDualOperator2(asCScriptNode *node, const char *methodName, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx, bool specificReturn, const asCDataType &returnType)
{
	// Find the matching method
	if( lctx->type.dataType.IsObject() &&
		(!lctx->type.isExplicitHandle ||
		 lctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE) )
	{
		asUINT n;

		// Is the left value a const?
		bool isConst = false;
		if( lctx->type.dataType.IsObjectHandle() )
			isConst = lctx->type.dataType.IsHandleToConst();
		else
			isConst = lctx->type.dataType.IsReadOnly();

		asCArray<int> funcs;
		asCObjectType *ot = lctx->type.dataType.GetObjectType();
		for( n = 0; n < ot->methods.GetLength(); n++ )
		{
			asCScriptFunction *func = engine->scriptFunctions[ot->methods[n]];
			if( func->name == methodName &&
				(!specificReturn || func->returnType == returnType) &&
				func->parameterTypes.GetLength() == 1 &&
				(!isConst || func->isReadOnly) )
			{
				// Make sure the method is accessible by the module
				if( builder->module->accessMask & func->accessMask )
				{
					funcs.PushLast(func->id);
				}
			}
		}

		// Which is the best matching function?
		asCArray<asSOverloadCandidate> tempFuncs;
		MatchArgument(funcs, tempFuncs, rctx, 0);

		// Find the lowest cost operator(s)
		asCArray<int> ops;
		asUINT bestCost = asUINT(-1);
		for( n = 0; n < tempFuncs.GetLength(); ++n )
		{
			asUINT cost = tempFuncs[n].cost;
			if( cost < bestCost )
			{
				ops.SetLength(0);
				bestCost = cost;
			}
			if( cost == bestCost )
				ops.PushLast(tempFuncs[n].funcId);
		}

		// If the object is not const, then we need to prioritize non-const methods
		if( !isConst )
			FilterConst(ops);

		// Did we find an operator?
		if( ops.GetLength() == 1 )
		{
			// Process the lctx expression as get accessor
			ProcessPropertyGetAccessor(lctx, node);

			// Merge the bytecode so that it forms lvalue.methodName(rvalue)
			asCTypeInfo objType = lctx->type;
			asCArray<asSExprContext *> args;
			args.PushLast(rctx);
			MergeExprBytecode(ctx, lctx);
			ctx->type = lctx->type;
			MakeFunctionCall(ctx, ops[0], objType.dataType.GetObjectType(), args, node);

			// If the method returned a reference, then we can't release the original
			// object yet, because the reference may be to a member of it
			if( !objType.isTemporary ||
				!(ctx->type.dataType.IsReference() || (ctx->type.dataType.IsObject() && !ctx->type.dataType.IsObjectHandle())) ||
				ctx->type.isVariable ) // If the resulting type is a variable, then the reference is not to a member
			{
				// As the index operator didn't return a reference to a
				// member we can release the original object now
				ReleaseTemporaryVariable(objType, &ctx->bc);
			}

			// Found matching operator
			return 1;
		}
		else if( ops.GetLength() > 1 )
		{
			Error(TXT_MORE_THAN_ONE_MATCHING_OP, node);
			PrintMatchingFuncs(ops, node);

			ctx->type.SetDummy();

			// Compiler error
			return -1;
		}
	}

	// No matching operator
	return 0;
}

void asCCompiler::MakeFunctionCall(asSExprContext *ctx, int funcId, asCObjectType *objectType, asCArray<asSExprContext*> &args, asCScriptNode * /*node*/, bool useVariable, int stackOffset, int funcPtrVar)
{
	if( objectType )
	{
		Dereference(ctx, true);

		// This following warning was removed as there may be valid reasons
		// for calling non-const methods on temporary objects, and we shouldn't
		// warn when there is no way of removing the warning.
/*
		// Warn if the method is non-const and the object is temporary
		// since the changes will be lost when the object is destroyed.
		// If the object is accessed through a handle, then it is assumed
		// the object is not temporary, even though the handle is.
		if( ctx->type.isTemporary &&
			!ctx->type.dataType.IsObjectHandle() &&
			!engine->scriptFunctions[funcId]->isReadOnly )
		{
			Warning("A non-const method is called on temporary object. Changes to the object may be lost.", node);
			Information(engine->scriptFunctions[funcId]->GetDeclaration(), node);
		}
*/	}

	asCByteCode objBC(engine);
	objBC.AddCode(&ctx->bc);

	PrepareFunctionCall(funcId, &ctx->bc, args);

	// Verify if any of the args variable offsets are used in the other code.
	// If they are exchange the offset for a new one
	asUINT n;
	for( n = 0; n < args.GetLength(); n++ )
	{
		if( args[n]->type.isTemporary && objBC.IsVarUsed(args[n]->type.stackOffset) )
		{
			// Release the current temporary variable
			ReleaseTemporaryVariable(args[n]->type, 0);

			asCDataType dt = args[n]->type.dataType;
			dt.MakeReference(false);

			int l = int(reservedVariables.GetLength());
			objBC.GetVarsUsed(reservedVariables);
			ctx->bc.GetVarsUsed(reservedVariables);
			int newOffset = AllocateVariable(dt, true, IsVariableOnHeap(args[n]->type.stackOffset));
			reservedVariables.SetLength(l);

			asASSERT( IsVariableOnHeap(args[n]->type.stackOffset) == IsVariableOnHeap(newOffset) );

			ctx->bc.ExchangeVar(args[n]->type.stackOffset, newOffset);
			args[n]->type.stackOffset = (short)newOffset;
			args[n]->type.isTemporary = true;
			args[n]->type.isVariable  = true;
		}
	}

	// If the function will return a value type on the stack, then we must allocate space
	// for that here and push the address on the stack as a hidden argument to the function
	asCScriptFunction *func = builder->GetFunctionDescription(funcId);
	if( func->DoesReturnOnStack() )
	{
		asASSERT(!useVariable);

		useVariable = true;
		stackOffset = AllocateVariable(func->returnType, true);
		ctx->bc.InstrSHORT(asBC_PSF, short(stackOffset));
	}

	ctx->bc.AddCode(&objBC);

	MoveArgsToStack(funcId, &ctx->bc, args, objectType ? true : false);

	PerformFunctionCall(funcId, ctx, false, &args, 0, useVariable, stackOffset, funcPtrVar);
}

int asCCompiler::CompileOperator(asCScriptNode *node, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx)
{
	// Don't allow any operators on expressions that take address of class method, but allow it on global functions
	if( (lctx->IsClassMethod()) || (rctx->IsClassMethod()) )
	{
		Error(TXT_INVALID_OP_ON_METHOD, node);
		return -1;
	}

	// Don't allow any operators on void expressions
	if( lctx->type.IsVoidExpression() || rctx->type.IsVoidExpression() )
	{
		Error(TXT_VOID_CANT_BE_OPERAND, node);
		return -1;
	}

	IsVariableInitialized(&lctx->type, node);
	IsVariableInitialized(&rctx->type, node);

	if( lctx->type.isExplicitHandle || rctx->type.isExplicitHandle ||
		lctx->type.IsNullConstant() || rctx->type.IsNullConstant() ||
		node->tokenType == ttIs || node->tokenType == ttNotIs )
	{
		CompileOperatorOnHandles(node, lctx, rctx, ctx);
		return 0;
	}
	else
	{
		// Compile an overloaded operator for the two operands
		if( CompileOverloadedDualOperator(node, lctx, rctx, ctx) )
			return 0;

		// If both operands are objects, then we shouldn't continue
		if( lctx->type.dataType.IsObject() && rctx->type.dataType.IsObject() )
		{
			asCString str;
			str.Format(TXT_NO_MATCHING_OP_FOUND_FOR_TYPES_s_AND_s, lctx->type.dataType.Format().AddressOf(), rctx->type.dataType.Format().AddressOf());
			Error(str, node);
			ctx->type.SetDummy();
			return -1;
		}

		// Process the property get accessors (if any)
		ProcessPropertyGetAccessor(lctx, node);
		ProcessPropertyGetAccessor(rctx, node);

		// Make sure we have two variables or constants
		if( lctx->type.dataType.IsReference() ) ConvertToVariableNotIn(lctx, rctx);
		if( rctx->type.dataType.IsReference() ) ConvertToVariableNotIn(rctx, lctx);

		// Make sure lctx doesn't end up with a variable used in rctx
		if( lctx->type.isTemporary && rctx->bc.IsVarUsed(lctx->type.stackOffset) )
		{
			int offset = AllocateVariableNotIn(lctx->type.dataType, true, false, rctx);
			rctx->bc.ExchangeVar(lctx->type.stackOffset, offset);
			ReleaseTemporaryVariable(offset, 0);
		}

		// Math operators
		// + - * / % ** += -= *= /= %= **=
		int op = node->tokenType;
		if( op == ttPlus     || op == ttAddAssign ||
			op == ttMinus    || op == ttSubAssign ||
			op == ttStar     || op == ttMulAssign ||
			op == ttSlash    || op == ttDivAssign ||
			op == ttPercent  || op == ttModAssign ||
			op == ttStarStar || op == ttPowAssign )
		{
			CompileMathOperator(node, lctx, rctx, ctx);
			return 0;
		}

		// Bitwise operators
		// << >> >>> & | ^ <<= >>= >>>= &= |= ^=
		if( op == ttAmp                || op == ttAndAssign         ||
			op == ttBitOr              || op == ttOrAssign          ||
			op == ttBitXor             || op == ttXorAssign         ||
			op == ttBitShiftLeft       || op == ttShiftLeftAssign   ||
			op == ttBitShiftRight      || op == ttShiftRightLAssign ||
			op == ttBitShiftRightArith || op == ttShiftRightAAssign )
		{
			CompileBitwiseOperator(node, lctx, rctx, ctx);
			return 0;
		}

		// Comparison operators
		// == != < > <= >=
		if( op == ttEqual       || op == ttNotEqual           ||
			op == ttLessThan    || op == ttLessThanOrEqual    ||
			op == ttGreaterThan || op == ttGreaterThanOrEqual )
		{
			CompileComparisonOperator(node, lctx, rctx, ctx);
			return 0;
		}

		// Boolean operators
		// && || ^^
		if( op == ttAnd || op == ttOr || op == ttXor )
		{
			CompileBooleanOperator(node, lctx, rctx, ctx);
			return 0;
		}
	}

	asASSERT(false);
	return -1;
}

void asCCompiler::ConvertToTempVariableNotIn(asSExprContext *ctx, asSExprContext *exclude)
{
	int l = int(reservedVariables.GetLength());
	if( exclude ) exclude->bc.GetVarsUsed(reservedVariables);
	ConvertToTempVariable(ctx);
	reservedVariables.SetLength(l);
}

void asCCompiler::ConvertToTempVariable(asSExprContext *ctx)
{
	// This is only used for primitive types and null handles
	asASSERT( ctx->type.dataType.IsPrimitive() || ctx->type.dataType.IsNullHandle() );

	ConvertToVariable(ctx);
	if( !ctx->type.isTemporary )
	{
		if( ctx->type.dataType.IsPrimitive() )
		{
			// Copy the variable to a temporary variable
			int offset = AllocateVariable(ctx->type.dataType, true);
			if( ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				ctx->bc.InstrW_W(asBC_CpyVtoV4, offset, ctx->type.stackOffset);
			else
				ctx->bc.InstrW_W(asBC_CpyVtoV8, offset, ctx->type.stackOffset);
			ctx->type.SetVariable(ctx->type.dataType, offset, true);
		}
		else
		{
			// We should never get here
			asASSERT(false);
		}
	}
}

void asCCompiler::ConvertToVariable(asSExprContext *ctx)
{
	// We should never get here while the context is still an unprocessed property accessor
	asASSERT(ctx->property_get == 0 && ctx->property_set == 0);

	int offset;
	if( !ctx->type.isVariable &&
		(ctx->type.dataType.IsObjectHandle() ||
		 (ctx->type.dataType.IsObject() && ctx->type.dataType.SupportHandles())) )
	{
		offset = AllocateVariable(ctx->type.dataType, true);
		if( ctx->type.IsNullConstant() )
		{
			if( ctx->bc.GetLastInstr() == asBC_PshNull )
				ctx->bc.Instr(asBC_PopPtr); // Pop the null constant pushed onto the stack
			ctx->bc.InstrSHORT(asBC_ClrVPtr, (short)offset);
		}
		else
		{
			// Copy the object handle to a variable
			ctx->bc.InstrSHORT(asBC_PSF, (short)offset);
			ctx->bc.InstrPTR(asBC_REFCPY, ctx->type.dataType.GetObjectType());
			ctx->bc.Instr(asBC_PopPtr);
		}

		ReleaseTemporaryVariable(ctx->type, &ctx->bc);
		ctx->type.SetVariable(ctx->type.dataType, offset, true);
		ctx->type.dataType.MakeHandle(true);
	}
	else if( (!ctx->type.isVariable || ctx->type.dataType.IsReference()) &&
		     ctx->type.dataType.IsPrimitive() )
	{
		if( ctx->type.isConstant )
		{
			offset = AllocateVariable(ctx->type.dataType, true);
			if( ctx->type.dataType.GetSizeInMemoryBytes() == 1 )
				ctx->bc.InstrSHORT_B(asBC_SetV1, (short)offset, ctx->type.byteValue);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 2 )
				ctx->bc.InstrSHORT_W(asBC_SetV2, (short)offset, ctx->type.wordValue);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 4 )
				ctx->bc.InstrSHORT_DW(asBC_SetV4, (short)offset, ctx->type.dwordValue);
			else
				ctx->bc.InstrSHORT_QW(asBC_SetV8, (short)offset, ctx->type.qwordValue);

			ctx->type.SetVariable(ctx->type.dataType, offset, true);
			return;
		}
		else
		{
			asASSERT(ctx->type.dataType.IsPrimitive());
			asASSERT(ctx->type.dataType.IsReference());

			ctx->type.dataType.MakeReference(false);
			offset = AllocateVariable(ctx->type.dataType, true);

			// Read the value from the address in the register directly into the variable
			if( ctx->type.dataType.GetSizeInMemoryBytes() == 1 )
				ctx->bc.InstrSHORT(asBC_RDR1, (short)offset);
			else if( ctx->type.dataType.GetSizeInMemoryBytes() == 2 )
				ctx->bc.InstrSHORT(asBC_RDR2, (short)offset);
			else if( ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				ctx->bc.InstrSHORT(asBC_RDR4, (short)offset);
			else
				ctx->bc.InstrSHORT(asBC_RDR8, (short)offset);
		}

		ReleaseTemporaryVariable(ctx->type, &ctx->bc);
		ctx->type.SetVariable(ctx->type.dataType, offset, true);
	}
}

void asCCompiler::ConvertToVariableNotIn(asSExprContext *ctx, asSExprContext *exclude)
{
	int l = int(reservedVariables.GetLength());
	if( exclude ) exclude->bc.GetVarsUsed(reservedVariables);
	ConvertToVariable(ctx);
	reservedVariables.SetLength(l);
}


void asCCompiler::CompileMathOperator(asCScriptNode *node, asSExprContext *lctx, asSExprContext  *rctx, asSExprContext *ctx)
{
	// TODO: If a constant is only using 32bits, then a 32bit operation is preferred

	// TODO: clean up: This initial part is identical to CompileComparisonOperator. Make a common function out of it

	// Implicitly convert the operands to a number type
	asCDataType to;

	// If either operand is a non-primitive then use the primitive type
	if( !lctx->type.dataType.IsPrimitive() )
		to.SetTokenType(rctx->type.dataType.GetTokenType());
	else if( !rctx->type.dataType.IsPrimitive() )
			to.SetTokenType(lctx->type.dataType.GetTokenType());
	else if( lctx->type.dataType.IsDoubleType() || rctx->type.dataType.IsDoubleType() )
		to.SetTokenType(ttDouble);
	else if( lctx->type.dataType.IsFloatType() || rctx->type.dataType.IsFloatType() )
		to.SetTokenType(ttFloat);
	else if( lctx->type.dataType.GetSizeInMemoryDWords() == 2 || rctx->type.dataType.GetSizeInMemoryDWords() == 2 )
	{
		// Convert to int64 if both are signed or if one is non-constant and signed
		if( (lctx->type.dataType.IsIntegerType() && rctx->type.dataType.IsIntegerType()) ||
			(lctx->type.dataType.IsIntegerType() && !lctx->type.isConstant) || 
			(rctx->type.dataType.IsIntegerType() && !rctx->type.isConstant) )
			to.SetTokenType(ttInt64);
		else
			to.SetTokenType(ttUInt64);
	}
	else
	{
		// Convert to int32 if both are signed or if one is non-constant and signed
		if( (lctx->type.dataType.IsIntegerType() && rctx->type.dataType.IsIntegerType()) ||
			(lctx->type.dataType.IsIntegerType() && !lctx->type.isConstant) || 
			(rctx->type.dataType.IsIntegerType() && !rctx->type.isConstant) )
			to.SetTokenType(ttInt);
		else if( lctx->type.dataType.IsUnsignedType() || rctx->type.dataType.IsUnsignedType() )
			to.SetTokenType(ttUInt);
		else if( lctx->type.dataType.IsBooleanType() || rctx->type.dataType.IsBooleanType() )
			to.SetTokenType(ttBool);
	}

	// If doing an operation with double constant and float variable, the constant should be converted to float
	if( (lctx->type.isConstant && lctx->type.dataType.IsDoubleType() && !rctx->type.isConstant && rctx->type.dataType.IsFloatType()) ||
		(rctx->type.isConstant && rctx->type.dataType.IsDoubleType() && !lctx->type.isConstant && lctx->type.dataType.IsFloatType()) )
		to.SetTokenType(ttFloat);

	// Do the actual conversion
	int l = int(reservedVariables.GetLength());
	rctx->bc.GetVarsUsed(reservedVariables);
	lctx->bc.GetVarsUsed(reservedVariables);

	if( lctx->type.dataType.IsReference() )
		ConvertToVariable(lctx);
	if( rctx->type.dataType.IsReference() )
		ConvertToVariable(rctx);

	int op = node->tokenType;
	if( to.IsPrimitive() )
	{
		// ttStarStar allows an integer, right-hand operand and a double
		// left-hand operand.
		if( (op == ttStarStar || op == ttPowAssign) &&
			lctx->type.dataType.IsDoubleType() &&
			(rctx->type.dataType.IsIntegerType() ||
			    rctx->type.dataType.IsUnsignedType()) )
		{
			to.SetTokenType(ttInt);
			ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV, true);
			to.SetTokenType(ttDouble);
		}
		else
		{
			ImplicitConversion(lctx, to, node, asIC_IMPLICIT_CONV, true);
			ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV, true);
		}
	}
	reservedVariables.SetLength(l);

	// Verify that the conversion was successful
	if( !lctx->type.dataType.IsIntegerType() &&
		!lctx->type.dataType.IsUnsignedType() &&
		!lctx->type.dataType.IsFloatType() &&
		!lctx->type.dataType.IsDoubleType() )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_MATH_TYPE, lctx->type.dataType.Format().AddressOf());
		Error(str, node);

		ctx->type.SetDummy();
		return;
	}

	if( !rctx->type.dataType.IsIntegerType() &&
		!rctx->type.dataType.IsUnsignedType() &&
		!rctx->type.dataType.IsFloatType() &&
		!rctx->type.dataType.IsDoubleType() )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_MATH_TYPE, rctx->type.dataType.Format().AddressOf());
		Error(str, node);

		ctx->type.SetDummy();
		return;
	}

	bool isConstant = lctx->type.isConstant && rctx->type.isConstant;

	// Verify if we are dividing with a constant zero
	if( rctx->type.isConstant && rctx->type.qwordValue == 0 &&
		(op == ttSlash   || op == ttDivAssign ||
		 op == ttPercent || op == ttModAssign) )
	{
		Error(TXT_DIVIDE_BY_ZERO, node);
	}

	if( !isConstant )
	{
		ConvertToVariableNotIn(lctx, rctx);
		ConvertToVariableNotIn(rctx, lctx);
		ReleaseTemporaryVariable(lctx->type, &lctx->bc);
		ReleaseTemporaryVariable(rctx->type, &rctx->bc);

		if( op == ttAddAssign || op == ttSubAssign ||
			op == ttMulAssign || op == ttDivAssign ||
			op == ttModAssign || op == ttPowAssign )
		{
			// Merge the operands in the different order so that they are evaluated correctly
			MergeExprBytecode(ctx, rctx);
			MergeExprBytecode(ctx, lctx);

			// We must not process the deferred parameters yet, as
			// it may overwrite the lvalue kept in the register
		}
		else
		{
			MergeExprBytecode(ctx, lctx);
			MergeExprBytecode(ctx, rctx);

			ProcessDeferredParams(ctx);
		}

		asEBCInstr instruction = asBC_ADDi;
		if( lctx->type.dataType.IsIntegerType() ||
			lctx->type.dataType.IsUnsignedType() )
		{
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				if( op == ttPlus || op == ttAddAssign )
					instruction = asBC_ADDi;
				else if( op == ttMinus || op == ttSubAssign )
					instruction = asBC_SUBi;
				else if( op == ttStar || op == ttMulAssign )
					instruction = asBC_MULi;
				else if( op == ttSlash || op == ttDivAssign )
				{
					if( lctx->type.dataType.IsIntegerType() )
						instruction = asBC_DIVi;
					else
						instruction = asBC_DIVu;
				}
				else if( op == ttPercent || op == ttModAssign )
				{
					if( lctx->type.dataType.IsIntegerType() )
						instruction = asBC_MODi;
					else
						instruction = asBC_MODu;
				}
				else if( op == ttStarStar || op == ttPowAssign )
				{
					if( lctx->type.dataType.IsIntegerType() )
						instruction = asBC_POWi;
					else
						instruction = asBC_POWu;
				}
			}
			else
			{
				if( op == ttPlus || op == ttAddAssign )
					instruction = asBC_ADDi64;
				else if( op == ttMinus || op == ttSubAssign )
					instruction = asBC_SUBi64;
				else if( op == ttStar || op == ttMulAssign )
					instruction = asBC_MULi64;
				else if( op == ttSlash || op == ttDivAssign )
				{
					if( lctx->type.dataType.IsIntegerType() )
						instruction = asBC_DIVi64;
					else
						instruction = asBC_DIVu64;
				}
				else if( op == ttPercent || op == ttModAssign )
				{
					if( lctx->type.dataType.IsIntegerType() )
						instruction = asBC_MODi64;
					else
						instruction = asBC_MODu64;
				}
				else if( op == ttStarStar || op == ttPowAssign )
				{
					if( lctx->type.dataType.IsIntegerType() )
						instruction = asBC_POWi64;
					else
						instruction = asBC_POWu64;
				}
			}
		}
		else if( lctx->type.dataType.IsFloatType() )
		{
			if( op == ttPlus || op == ttAddAssign )
				instruction = asBC_ADDf;
			else if( op == ttMinus || op == ttSubAssign )
				instruction = asBC_SUBf;
			else if( op == ttStar || op == ttMulAssign )
				instruction = asBC_MULf;
			else if( op == ttSlash || op == ttDivAssign )
				instruction = asBC_DIVf;
			else if( op == ttPercent || op == ttModAssign )
				instruction = asBC_MODf;
			else if( op == ttStarStar || op == ttPowAssign )
				instruction = asBC_POWf;
		}
		else if( lctx->type.dataType.IsDoubleType() )
		{
			if( rctx->type.dataType.IsIntegerType() )
			{
				asASSERT(rctx->type.dataType.GetSizeInMemoryDWords() == 1);

				if( op == ttStarStar || op == ttPowAssign )
					instruction = asBC_POWdi;
				else
					asASSERT(false);  // Should not be possible
			}
			else
			{
				if( op == ttPlus || op == ttAddAssign )
					instruction = asBC_ADDd;
				else if( op == ttMinus || op == ttSubAssign )
					instruction = asBC_SUBd;
				else if( op == ttStar || op == ttMulAssign )
					instruction = asBC_MULd;
				else if( op == ttSlash || op == ttDivAssign )
					instruction = asBC_DIVd;
				else if( op == ttPercent || op == ttModAssign )
					instruction = asBC_MODd;
				else if( op == ttStarStar || op == ttPowAssign )
					instruction = asBC_POWd;
			}
		}
		else
		{
			// Shouldn't be possible
			asASSERT(false);
		}

		// Do the operation
		int a = AllocateVariable(lctx->type.dataType, true);
		int b = lctx->type.stackOffset;
		int c = rctx->type.stackOffset;

		ctx->bc.InstrW_W_W(instruction, a, b, c);

		ctx->type.SetVariable(lctx->type.dataType, a, true);
	}
	else
	{
		// Both values are constants
		if( lctx->type.dataType.IsIntegerType() ||
			lctx->type.dataType.IsUnsignedType() )
		{
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				int v = 0;
				if( op == ttPlus )
					v = lctx->type.intValue + rctx->type.intValue;
				else if( op == ttMinus )
					v = lctx->type.intValue - rctx->type.intValue;
				else if( op == ttStar )
					v = lctx->type.intValue * rctx->type.intValue;
				else if( op == ttSlash )
				{
					// TODO: Should probably report an error, rather than silently convert the value to 0
					if( rctx->type.intValue == 0 || (rctx->type.intValue == -1 && lctx->type.dwordValue == 0x80000000) )
						v = 0;
					else
						if( lctx->type.dataType.IsIntegerType() )
							v = lctx->type.intValue / rctx->type.intValue;
						else
							v = lctx->type.dwordValue / rctx->type.dwordValue;
				}
				else if( op == ttPercent )
				{
					// TODO: Should probably report an error, rather than silently convert the value to 0
					if( rctx->type.intValue == 0 || (rctx->type.intValue == -1 && lctx->type.dwordValue == 0x80000000) )
						v = 0;
					else
						if( lctx->type.dataType.IsIntegerType() )
							v = lctx->type.intValue % rctx->type.intValue;
						else
							v = lctx->type.dwordValue % rctx->type.dwordValue;
				}
				else if( op == ttStarStar )
				{
					bool isOverflow;
					if( lctx->type.dataType.IsIntegerType() )
						v = as_powi(lctx->type.intValue, rctx->type.intValue, isOverflow);
					else
						v = as_powu(lctx->type.dwordValue, rctx->type.dwordValue, isOverflow);
				}

				ctx->type.SetConstantDW(lctx->type.dataType, v);

				// If the right value is greater than the left value in a minus operation, then we need to convert the type to int
				if( lctx->type.dataType.GetTokenType() == ttUInt && op == ttMinus && lctx->type.intValue < rctx->type.intValue )
					ctx->type.dataType.SetTokenType(ttInt);
			}
			else
			{
				asQWORD v = 0;
				if( op == ttPlus )
					v = lctx->type.qwordValue + rctx->type.qwordValue;
				else if( op == ttMinus )
					v = lctx->type.qwordValue - rctx->type.qwordValue;
				else if( op == ttStar )
					v = lctx->type.qwordValue * rctx->type.qwordValue;
				else if( op == ttSlash )
				{
					// TODO: Should probably report an error, rather than silently convert the value to 0
					if( rctx->type.qwordValue == 0 || (rctx->type.qwordValue == asQWORD(-1) && lctx->type.qwordValue == (asQWORD(1)<<63)) )
						v = 0;
					else
						if( lctx->type.dataType.IsIntegerType() )
							v = asINT64(lctx->type.qwordValue) / asINT64(rctx->type.qwordValue);
						else
							v = lctx->type.qwordValue / rctx->type.qwordValue;
				}
				else if( op == ttPercent )
				{
				    // TODO: Should probably report an error, rather than silently convert the value to 0
					if( rctx->type.qwordValue == 0 || (rctx->type.qwordValue == asQWORD(-1) && lctx->type.qwordValue == (asQWORD(1)<<63)) )
						v = 0;
					else
						if( lctx->type.dataType.IsIntegerType() )
							v = asINT64(lctx->type.qwordValue) % asINT64(rctx->type.qwordValue);
						else
							v = lctx->type.qwordValue % rctx->type.qwordValue;
				}
				else if( op == ttStarStar )
				{
					bool isOverflow;
					if( lctx->type.dataType.IsIntegerType() )
						v = as_powi64(asINT64(lctx->type.qwordValue), asINT64(rctx->type.qwordValue), isOverflow);
					else
						v = as_powu64(lctx->type.qwordValue, rctx->type.qwordValue, isOverflow);
				}

				ctx->type.SetConstantQW(lctx->type.dataType, v);

				// If the right value is greater than the left value in a minus operation, then we need to convert the type to int
				if( lctx->type.dataType.GetTokenType() == ttUInt64 && op == ttMinus && lctx->type.qwordValue < rctx->type.qwordValue )
					ctx->type.dataType.SetTokenType(ttInt64);
			}
		}
		else if( lctx->type.dataType.IsFloatType() )
		{
			float v = 0.0f;
			if( op == ttPlus )
				v = lctx->type.floatValue + rctx->type.floatValue;
			else if( op == ttMinus )
				v = lctx->type.floatValue - rctx->type.floatValue;
			else if( op == ttStar )
				v = lctx->type.floatValue * rctx->type.floatValue;
			else if( op == ttSlash )
			{
				if( rctx->type.floatValue == 0 )
					v = 0;
				else
					v = lctx->type.floatValue / rctx->type.floatValue;
			}
			else if( op == ttPercent )
			{
				if( rctx->type.floatValue == 0 )
					v = 0;
				else
					v = fmodf(lctx->type.floatValue, rctx->type.floatValue);
			}
			else if( op == ttStarStar )
				v = pow(lctx->type.floatValue, rctx->type.floatValue);

			ctx->type.SetConstantF(lctx->type.dataType, v);
		}
		else if( lctx->type.dataType.IsDoubleType() )
		{
			double v = 0.0;
			if( rctx->type.dataType.IsIntegerType() )
			{
				asASSERT(rctx->type.dataType.GetSizeInMemoryDWords() == 1);

				if( op == ttStarStar || op == ttPowAssign )
					v = pow(lctx->type.doubleValue, rctx->type.intValue);
				else
					asASSERT(false);  // Should not be possible
			}
			else
			{
				if( op == ttPlus )
					v = lctx->type.doubleValue + rctx->type.doubleValue;
				else if( op == ttMinus )
					v = lctx->type.doubleValue - rctx->type.doubleValue;
				else if( op == ttStar )
					v = lctx->type.doubleValue * rctx->type.doubleValue;
				else if( op == ttSlash )
				{
					if( rctx->type.doubleValue == 0 )
						v = 0;
					else
						v = lctx->type.doubleValue / rctx->type.doubleValue;
				}
				else if( op == ttPercent )
				{
					if( rctx->type.doubleValue == 0 )
						v = 0;
					else
						v = fmod(lctx->type.doubleValue, rctx->type.doubleValue);
				}
				else if( op == ttStarStar )
					v = pow(lctx->type.doubleValue, rctx->type.doubleValue);
			}

			ctx->type.SetConstantD(lctx->type.dataType, v);
		}
		else
		{
			// Shouldn't be possible
			asASSERT(false);
		}
	}
}

void asCCompiler::CompileBitwiseOperator(asCScriptNode *node, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx)
{
	// TODO: If a constant is only using 32bits, then a 32bit operation is preferred

	int op = node->tokenType;
	if( op == ttAmp    || op == ttAndAssign ||
		op == ttBitOr  || op == ttOrAssign  ||
		op == ttBitXor || op == ttXorAssign )
	{
		// Convert left hand operand to integer if it's not already one
		asCDataType to;
		if( lctx->type.dataType.GetSizeInMemoryDWords() == 2 ||
			rctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			to.SetTokenType(ttUInt64);
		else
			to.SetTokenType(ttUInt);

		// Do the actual conversion
		int l = int(reservedVariables.GetLength());
		rctx->bc.GetVarsUsed(reservedVariables);
		ImplicitConversion(lctx, to, node, asIC_IMPLICIT_CONV, true);
		reservedVariables.SetLength(l);

		// Verify that the conversion was successful
		if( !lctx->type.dataType.IsUnsignedType() )
		{
			asCString str;
			str.Format(TXT_NO_CONVERSION_s_TO_s, lctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
			Error(str, node);
		}

		// Convert right hand operand to same type as left hand operand
		l = int(reservedVariables.GetLength());
		lctx->bc.GetVarsUsed(reservedVariables);
		ImplicitConversion(rctx, lctx->type.dataType, node, asIC_IMPLICIT_CONV, true);
		reservedVariables.SetLength(l);
		if( !rctx->type.dataType.IsEqualExceptRef(lctx->type.dataType) )
		{
			asCString str;
			str.Format(TXT_NO_CONVERSION_s_TO_s, rctx->type.dataType.Format().AddressOf(), lctx->type.dataType.Format().AddressOf());
			Error(str, node);
		}

		bool isConstant = lctx->type.isConstant && rctx->type.isConstant;

		if( !isConstant )
		{
			ConvertToVariableNotIn(lctx, rctx);
			ConvertToVariableNotIn(rctx, lctx);
			ReleaseTemporaryVariable(lctx->type, &lctx->bc);
			ReleaseTemporaryVariable(rctx->type, &rctx->bc);

			if( op == ttAndAssign || op == ttOrAssign || op == ttXorAssign )
			{
				// Compound assignments execute the right hand value first
				MergeExprBytecode(ctx, rctx);
				MergeExprBytecode(ctx, lctx);
			}
			else
			{
				MergeExprBytecode(ctx, lctx);
				MergeExprBytecode(ctx, rctx);
			}
			ProcessDeferredParams(ctx);

			asEBCInstr instruction = asBC_BAND;
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				if( op == ttAmp || op == ttAndAssign )
					instruction = asBC_BAND;
				else if( op == ttBitOr || op == ttOrAssign )
					instruction = asBC_BOR;
				else if( op == ttBitXor || op == ttXorAssign )
					instruction = asBC_BXOR;
			}
			else
			{
				if( op == ttAmp || op == ttAndAssign )
					instruction = asBC_BAND64;
				else if( op == ttBitOr || op == ttOrAssign )
					instruction = asBC_BOR64;
				else if( op == ttBitXor || op == ttXorAssign )
					instruction = asBC_BXOR64;
			}

			// Do the operation
			int a = AllocateVariable(lctx->type.dataType, true);
			int b = lctx->type.stackOffset;
			int c = rctx->type.stackOffset;

			ctx->bc.InstrW_W_W(instruction, a, b, c);

			ctx->type.SetVariable(lctx->type.dataType, a, true);
		}
		else
		{
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				asQWORD v = 0;
				if( op == ttAmp )
					v = lctx->type.qwordValue & rctx->type.qwordValue;
				else if( op == ttBitOr )
					v = lctx->type.qwordValue | rctx->type.qwordValue;
				else if( op == ttBitXor )
					v = lctx->type.qwordValue ^ rctx->type.qwordValue;

				// Remember the result
				ctx->type.SetConstantQW(lctx->type.dataType, v);
			}
			else
			{
				asDWORD v = 0;
				if( op == ttAmp )
					v = lctx->type.dwordValue & rctx->type.dwordValue;
				else if( op == ttBitOr )
					v = lctx->type.dwordValue | rctx->type.dwordValue;
				else if( op == ttBitXor )
					v = lctx->type.dwordValue ^ rctx->type.dwordValue;

				// Remember the result
				ctx->type.SetConstantDW(lctx->type.dataType, v);
			}
		}
	}
	else if( op == ttBitShiftLeft       || op == ttShiftLeftAssign   ||
		     op == ttBitShiftRight      || op == ttShiftRightLAssign ||
			 op == ttBitShiftRightArith || op == ttShiftRightAAssign )
	{
		// Don't permit object to primitive conversion, since we don't know which integer type is the correct one
		if( lctx->type.dataType.IsObject() )
		{
			asCString str;
			str.Format(TXT_ILLEGAL_OPERATION_ON_s, lctx->type.dataType.Format().AddressOf());
			Error(str, node);

			// Set an integer value and allow the compiler to continue
			ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttInt, true), 0);
			return;
		}

		// Convert left hand operand to integer if it's not already one
		asCDataType to = lctx->type.dataType;
		if( lctx->type.dataType.IsUnsignedType() &&
			lctx->type.dataType.GetSizeInMemoryBytes() < 4 )
		{
			to = asCDataType::CreatePrimitive(ttUInt, false);
		}
		else if( !lctx->type.dataType.IsUnsignedType() )
		{
			asCDataType to;
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 2 )
				to.SetTokenType(ttInt64);
			else
				to.SetTokenType(ttInt);
		}

		// Do the actual conversion
		int l = int(reservedVariables.GetLength());
		rctx->bc.GetVarsUsed(reservedVariables);
		ImplicitConversion(lctx, to, node, asIC_IMPLICIT_CONV, true);
		reservedVariables.SetLength(l);

		// Verify that the conversion was successful
		if( lctx->type.dataType != to )
		{
			asCString str;
			str.Format(TXT_NO_CONVERSION_s_TO_s, lctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
			Error(str, node);
		}

		// Right operand must be 32bit uint
		l = int(reservedVariables.GetLength());
		lctx->bc.GetVarsUsed(reservedVariables);
		ImplicitConversion(rctx, asCDataType::CreatePrimitive(ttUInt, true), node, asIC_IMPLICIT_CONV, true);
		reservedVariables.SetLength(l);
		if( !rctx->type.dataType.IsUnsignedType() )
		{
			asCString str;
			str.Format(TXT_NO_CONVERSION_s_TO_s, rctx->type.dataType.Format().AddressOf(), "uint");
			Error(str, node);
		}

		bool isConstant = lctx->type.isConstant && rctx->type.isConstant;

		if( !isConstant )
		{
			ConvertToVariableNotIn(lctx, rctx);
			ConvertToVariableNotIn(rctx, lctx);
			ReleaseTemporaryVariable(lctx->type, &lctx->bc);
			ReleaseTemporaryVariable(rctx->type, &rctx->bc);

			if( op == ttShiftLeftAssign || op == ttShiftRightLAssign || op == ttShiftRightAAssign )
			{
				// Compound assignments execute the right hand value first
				MergeExprBytecode(ctx, rctx);
				MergeExprBytecode(ctx, lctx);
			}
			else
			{
				MergeExprBytecode(ctx, lctx);
				MergeExprBytecode(ctx, rctx);
			}
			ProcessDeferredParams(ctx);

			asEBCInstr instruction = asBC_BSLL;
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				if( op == ttBitShiftLeft || op == ttShiftLeftAssign )
					instruction = asBC_BSLL;
				else if( op == ttBitShiftRight || op == ttShiftRightLAssign )
					instruction = asBC_BSRL;
				else if( op == ttBitShiftRightArith || op == ttShiftRightAAssign )
					instruction = asBC_BSRA;
			}
			else
			{
				if( op == ttBitShiftLeft || op == ttShiftLeftAssign )
					instruction = asBC_BSLL64;
				else if( op == ttBitShiftRight || op == ttShiftRightLAssign )
					instruction = asBC_BSRL64;
				else if( op == ttBitShiftRightArith || op == ttShiftRightAAssign )
					instruction = asBC_BSRA64;
			}

			// Do the operation
			int a = AllocateVariable(lctx->type.dataType, true);
			int b = lctx->type.stackOffset;
			int c = rctx->type.stackOffset;

			ctx->bc.InstrW_W_W(instruction, a, b, c);

			ctx->type.SetVariable(lctx->type.dataType, a, true);
		}
		else
		{
			if( lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				asDWORD v = 0;
				if( op == ttBitShiftLeft )
					v = lctx->type.dwordValue << rctx->type.dwordValue;
				else if( op == ttBitShiftRight )
					v = lctx->type.dwordValue >> rctx->type.dwordValue;
				else if( op == ttBitShiftRightArith )
					v = lctx->type.intValue >> rctx->type.dwordValue;

				ctx->type.SetConstantDW(lctx->type.dataType, v);
			}
			else
			{
				asQWORD v = 0;
				if( op == ttBitShiftLeft )
					v = lctx->type.qwordValue << rctx->type.dwordValue;
				else if( op == ttBitShiftRight )
					v = lctx->type.qwordValue >> rctx->type.dwordValue;
				else if( op == ttBitShiftRightArith )
					v = asINT64(lctx->type.qwordValue) >> rctx->type.dwordValue;

				ctx->type.SetConstantQW(lctx->type.dataType, v);
			}
		}
	}
}

void asCCompiler::CompileComparisonOperator(asCScriptNode *node, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx)
{
	// Both operands must be of the same type

	// Implicitly convert the operands to a number type
	asCDataType to;

	// If either operand is a non-primitive then use the primitive type
	if( !lctx->type.dataType.IsPrimitive() )
		to.SetTokenType(rctx->type.dataType.GetTokenType());
	else if( !rctx->type.dataType.IsPrimitive() )
			to.SetTokenType(lctx->type.dataType.GetTokenType());
	else if( lctx->type.dataType.IsDoubleType() || rctx->type.dataType.IsDoubleType() )
		to.SetTokenType(ttDouble);
	else if( lctx->type.dataType.IsFloatType() || rctx->type.dataType.IsFloatType() )
		to.SetTokenType(ttFloat);
	else if( lctx->type.dataType.GetSizeInMemoryDWords() == 2 || rctx->type.dataType.GetSizeInMemoryDWords() == 2 )
	{
		// Convert to int64 if both are signed or if one is non-constant and signed
		if( (lctx->type.dataType.IsIntegerType() && rctx->type.dataType.IsIntegerType()) ||
			(lctx->type.dataType.IsIntegerType() && !lctx->type.isConstant) || 
			(rctx->type.dataType.IsIntegerType() && !rctx->type.isConstant) )
			to.SetTokenType(ttInt64);
		else
			to.SetTokenType(ttUInt64);
	}
	else
	{
		// Convert to int32 if both are signed or if one is non-constant and signed
		if( (lctx->type.dataType.IsIntegerType() && rctx->type.dataType.IsIntegerType()) ||
			(lctx->type.dataType.IsIntegerType() && !lctx->type.isConstant) || 
			(rctx->type.dataType.IsIntegerType() && !rctx->type.isConstant) )
			to.SetTokenType(ttInt);
		else if( lctx->type.dataType.IsUnsignedType() || rctx->type.dataType.IsUnsignedType() )
			to.SetTokenType(ttUInt);
		else if( lctx->type.dataType.IsBooleanType() || rctx->type.dataType.IsBooleanType() )
			to.SetTokenType(ttBool);
	}

	// If doing an operation with double constant and float variable, the constant should be converted to float
	if( (lctx->type.isConstant && lctx->type.dataType.IsDoubleType() && !rctx->type.isConstant && rctx->type.dataType.IsFloatType()) ||
		(rctx->type.isConstant && rctx->type.dataType.IsDoubleType() && !lctx->type.isConstant && lctx->type.dataType.IsFloatType()) )
		to.SetTokenType(ttFloat);

	asASSERT( to.GetTokenType() != ttUnrecognizedToken );

	// Do we have a mismatch between the sign of the operand?
	bool signMismatch = false;
	for( int n = 0; !signMismatch && n < 2; n++ )
	{
		asSExprContext *op = n ? rctx : lctx;

		if( op->type.dataType.IsUnsignedType() != to.IsUnsignedType() )
		{
			// We have a mismatch, unless the value is a literal constant and the conversion won't affect its value
			signMismatch = true;
			if( op->type.isConstant )
			{
				if( op->type.dataType.GetTokenType() == ttUInt64 || op->type.dataType.GetTokenType() == ttInt64 )
				{
					if( !(op->type.qwordValue & (asQWORD(1)<<63)) )
						signMismatch = false;
				}
				else
				{
					if( !(op->type.dwordValue & (1<<31)) )
						signMismatch = false;
				}

				// It's not necessary to check for floats or double, because if
				// it was then the types for the conversion will never be unsigned
			}
		}
	}

	// Check for signed/unsigned mismatch
	if( signMismatch )
		Warning(TXT_SIGNED_UNSIGNED_MISMATCH, node);

	// Do the actual conversion
	int l = int(reservedVariables.GetLength());
	rctx->bc.GetVarsUsed(reservedVariables);

	if( lctx->type.dataType.IsReference() )
		ConvertToVariable(lctx);
	if( rctx->type.dataType.IsReference() )
		ConvertToVariable(rctx);

	ImplicitConversion(lctx, to, node, asIC_IMPLICIT_CONV);
	ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV);
	reservedVariables.SetLength(l);

	// Verify that the conversion was successful
	bool ok = true;
	if( !lctx->type.dataType.IsEqualExceptConst(to) )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, lctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
		Error(str, node);
		ok = false;
	}

	if( !rctx->type.dataType.IsEqualExceptConst(to) )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, rctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
		Error(str, node);
		ok = false;
	}

	if( !ok )
	{
		// It wasn't possible to get two valid operands, so we just return
		// a boolean result and let the compiler continue.
		ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), true);
		return;
	}

	bool isConstant = lctx->type.isConstant && rctx->type.isConstant;
	int op = node->tokenType;

	if( !isConstant )
	{
		if( to.IsBooleanType() )
		{
			int op = node->tokenType;
			if( op == ttEqual || op == ttNotEqual )
			{
				// Must convert to temporary variable, because we are changing the value before comparison
				ConvertToTempVariableNotIn(lctx, rctx);
				ConvertToTempVariableNotIn(rctx, lctx);
				ReleaseTemporaryVariable(lctx->type, &lctx->bc);
				ReleaseTemporaryVariable(rctx->type, &rctx->bc);

				// Make sure they are equal if not false
				lctx->bc.InstrWORD(asBC_NOT, lctx->type.stackOffset);
				rctx->bc.InstrWORD(asBC_NOT, rctx->type.stackOffset);

				MergeExprBytecode(ctx, lctx);
				MergeExprBytecode(ctx, rctx);
				ProcessDeferredParams(ctx);

				int a = AllocateVariable(asCDataType::CreatePrimitive(ttBool, true), true);
				int b = lctx->type.stackOffset;
				int c = rctx->type.stackOffset;

				if( op == ttEqual )
				{
					ctx->bc.InstrW_W(asBC_CMPi,b,c);
					ctx->bc.Instr(asBC_TZ);
					ctx->bc.InstrSHORT(asBC_CpyRtoV4, (short)a);
				}
				else if( op == ttNotEqual )
				{
					ctx->bc.InstrW_W(asBC_CMPi,b,c);
					ctx->bc.Instr(asBC_TNZ);
					ctx->bc.InstrSHORT(asBC_CpyRtoV4, (short)a);
				}

				ctx->type.SetVariable(asCDataType::CreatePrimitive(ttBool, true), a, true);
			}
			else
			{
				// TODO: Use TXT_ILLEGAL_OPERATION_ON
				Error(TXT_ILLEGAL_OPERATION, node);
				ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), 0);
			}
		}
		else
		{
			ConvertToVariableNotIn(lctx, rctx);
			ConvertToVariableNotIn(rctx, lctx);
			ReleaseTemporaryVariable(lctx->type, &lctx->bc);
			ReleaseTemporaryVariable(rctx->type, &rctx->bc);

			MergeExprBytecode(ctx, lctx);
			MergeExprBytecode(ctx, rctx);
			ProcessDeferredParams(ctx);

			asEBCInstr iCmp = asBC_CMPi, iT = asBC_TZ;

			if( lctx->type.dataType.IsIntegerType() && lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				iCmp = asBC_CMPi;
			else if( lctx->type.dataType.IsUnsignedType() && lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
				iCmp = asBC_CMPu;
			else if( lctx->type.dataType.IsIntegerType() && lctx->type.dataType.GetSizeInMemoryDWords() == 2 )
				iCmp = asBC_CMPi64;
			else if( lctx->type.dataType.IsUnsignedType() && lctx->type.dataType.GetSizeInMemoryDWords() == 2 )
				iCmp = asBC_CMPu64;
			else if( lctx->type.dataType.IsFloatType() )
				iCmp = asBC_CMPf;
			else if( lctx->type.dataType.IsDoubleType() )
				iCmp = asBC_CMPd;
			else
				asASSERT(false);

			if( op == ttEqual )
				iT = asBC_TZ;
			else if( op == ttNotEqual )
				iT = asBC_TNZ;
			else if( op == ttLessThan )
				iT = asBC_TS;
			else if( op == ttLessThanOrEqual )
				iT = asBC_TNP;
			else if( op == ttGreaterThan )
				iT = asBC_TP;
			else if( op == ttGreaterThanOrEqual )
				iT = asBC_TNS;

			int a = AllocateVariable(asCDataType::CreatePrimitive(ttBool, true), true);
			int b = lctx->type.stackOffset;
			int c = rctx->type.stackOffset;

			ctx->bc.InstrW_W(iCmp, b, c);
			ctx->bc.Instr(iT);
			ctx->bc.InstrSHORT(asBC_CpyRtoV4, (short)a);

			ctx->type.SetVariable(asCDataType::CreatePrimitive(ttBool, true), a, true);
		}
	}
	else
	{
		if( to.IsBooleanType() )
		{
			int op = node->tokenType;
			if( op == ttEqual || op == ttNotEqual )
			{
				// Make sure they are equal if not false
				if( lctx->type.dwordValue != 0 ) lctx->type.dwordValue = VALUE_OF_BOOLEAN_TRUE;
				if( rctx->type.dwordValue != 0 ) rctx->type.dwordValue = VALUE_OF_BOOLEAN_TRUE;

				asDWORD v = 0;
				if( op == ttEqual )
				{
					v = lctx->type.intValue - rctx->type.intValue;
					if( v == 0 ) v = VALUE_OF_BOOLEAN_TRUE; else v = 0;
				}
				else if( op == ttNotEqual )
				{
					v = lctx->type.intValue - rctx->type.intValue;
					if( v != 0 ) v = VALUE_OF_BOOLEAN_TRUE; else v = 0;
				}

				ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), v);
			}
			else
			{
				// TODO: Use TXT_ILLEGAL_OPERATION_ON
				Error(TXT_ILLEGAL_OPERATION, node);
			}
		}
		else
		{
			int i = 0;
			if( lctx->type.dataType.IsIntegerType() && lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				int v = lctx->type.intValue - rctx->type.intValue;
				if( v < 0 ) i = -1;
				if( v > 0 ) i = 1;
			}
			else if( lctx->type.dataType.IsUnsignedType() && lctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			{
				asDWORD v1 = lctx->type.dwordValue;
				asDWORD v2 = rctx->type.dwordValue;
				if( v1 < v2 ) i = -1;
				if( v1 > v2 ) i = 1;
			}
			else if( lctx->type.dataType.IsIntegerType() && lctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				asINT64 v = asINT64(lctx->type.qwordValue) - asINT64(rctx->type.qwordValue);
				if( v < 0 ) i = -1;
				if( v > 0 ) i = 1;
			}
			else if( lctx->type.dataType.IsUnsignedType() && lctx->type.dataType.GetSizeInMemoryDWords() == 2 )
			{
				asQWORD v1 = lctx->type.qwordValue;
				asQWORD v2 = rctx->type.qwordValue;
				if( v1 < v2 ) i = -1;
				if( v1 > v2 ) i = 1;
			}
			else if( lctx->type.dataType.IsFloatType() )
			{
				float v = lctx->type.floatValue - rctx->type.floatValue;
				if( v < 0 ) i = -1;
				if( v > 0 ) i = 1;
			}
			else if( lctx->type.dataType.IsDoubleType() )
			{
				double v = lctx->type.doubleValue - rctx->type.doubleValue;
				if( v < 0 ) i = -1;
				if( v > 0 ) i = 1;
			}


			if( op == ttEqual )
				i = (i == 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
			else if( op == ttNotEqual )
				i = (i != 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
			else if( op == ttLessThan )
				i = (i < 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
			else if( op == ttLessThanOrEqual )
				i = (i <= 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
			else if( op == ttGreaterThan )
				i = (i > 0 ? VALUE_OF_BOOLEAN_TRUE : 0);
			else if( op == ttGreaterThanOrEqual )
				i = (i >= 0 ? VALUE_OF_BOOLEAN_TRUE : 0);

			ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), i);
		}
	}
}

void asCCompiler::PushVariableOnStack(asSExprContext *ctx, bool asReference)
{
	// Put the result on the stack
	if( asReference )
	{
		ctx->bc.InstrSHORT(asBC_PSF, ctx->type.stackOffset);
		ctx->type.dataType.MakeReference(true);
	}
	else
	{
		if( ctx->type.dataType.GetSizeInMemoryDWords() == 1 )
			ctx->bc.InstrSHORT(asBC_PshV4, ctx->type.stackOffset);
		else
			ctx->bc.InstrSHORT(asBC_PshV8, ctx->type.stackOffset);
	}
}

void asCCompiler::CompileBooleanOperator(asCScriptNode *node, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx)
{
	// Both operands must be booleans
	asCDataType to;
	to.SetTokenType(ttBool);

	// Do the actual conversion
	int l = int(reservedVariables.GetLength());
	rctx->bc.GetVarsUsed(reservedVariables);
	lctx->bc.GetVarsUsed(reservedVariables);
	ImplicitConversion(lctx, to, node, asIC_IMPLICIT_CONV);
	ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV);
	reservedVariables.SetLength(l);

	// Verify that the conversion was successful
	if( !lctx->type.dataType.IsBooleanType() )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, lctx->type.dataType.Format().AddressOf(), "bool");
		Error(str, node);
		// Force the conversion to allow compilation to proceed
		lctx->type.SetConstantB(asCDataType::CreatePrimitive(ttBool, true), true);
	}

	if( !rctx->type.dataType.IsBooleanType() )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, rctx->type.dataType.Format().AddressOf(), "bool");
		Error(str, node);
		// Force the conversion to allow compilation to proceed
		rctx->type.SetConstantB(asCDataType::CreatePrimitive(ttBool, true), true);
	}

	bool isConstant = lctx->type.isConstant && rctx->type.isConstant;

	ctx->type.Set(asCDataType::CreatePrimitive(ttBool, true));

	// What kind of operator is it?
	int op = node->tokenType;
	if( op == ttXor )
	{
		if( !isConstant )
		{
			// Must convert to temporary variable, because we are changing the value before comparison
			ConvertToTempVariableNotIn(lctx, rctx);
			ConvertToTempVariableNotIn(rctx, lctx);
			ReleaseTemporaryVariable(lctx->type, &lctx->bc);
			ReleaseTemporaryVariable(rctx->type, &rctx->bc);

			// Make sure they are equal if not false
			lctx->bc.InstrWORD(asBC_NOT, lctx->type.stackOffset);
			rctx->bc.InstrWORD(asBC_NOT, rctx->type.stackOffset);

			MergeExprBytecode(ctx, lctx);
			MergeExprBytecode(ctx, rctx);
			ProcessDeferredParams(ctx);

			int a = AllocateVariable(ctx->type.dataType, true);
			int b = lctx->type.stackOffset;
			int c = rctx->type.stackOffset;

			ctx->bc.InstrW_W_W(asBC_BXOR,a,b,c);

			ctx->type.SetVariable(asCDataType::CreatePrimitive(ttBool, true), a, true);
		}
		else
		{
			// Make sure they are equal if not false
#if AS_SIZEOF_BOOL == 1
			if( lctx->type.byteValue != 0 ) lctx->type.byteValue = VALUE_OF_BOOLEAN_TRUE;
			if( rctx->type.byteValue != 0 ) rctx->type.byteValue = VALUE_OF_BOOLEAN_TRUE;

			asBYTE v = 0;
			v = lctx->type.byteValue - rctx->type.byteValue;
			if( v != 0 ) v = VALUE_OF_BOOLEAN_TRUE; else v = 0;

			ctx->type.isConstant = true;
			ctx->type.byteValue = v;
#else
			if( lctx->type.dwordValue != 0 ) lctx->type.dwordValue = VALUE_OF_BOOLEAN_TRUE;
			if( rctx->type.dwordValue != 0 ) rctx->type.dwordValue = VALUE_OF_BOOLEAN_TRUE;

			asDWORD v = 0;
			v = lctx->type.intValue - rctx->type.intValue;
			if( v != 0 ) v = VALUE_OF_BOOLEAN_TRUE; else v = 0;

			ctx->type.isConstant = true;
			ctx->type.dwordValue = v;
#endif
		}
	}
	else if( op == ttAnd ||
			 op == ttOr )
	{
		if( !isConstant )
		{
			// If or-operator and first value is 1 the second value shouldn't be calculated
			// if and-operator and first value is 0 the second value shouldn't be calculated
			ConvertToVariable(lctx);
			ReleaseTemporaryVariable(lctx->type, &lctx->bc);
			MergeExprBytecode(ctx, lctx);

			int offset = AllocateVariable(asCDataType::CreatePrimitive(ttBool, false), true);

			int label1 = nextLabel++;
			int label2 = nextLabel++;

			ctx->bc.InstrSHORT(asBC_CpyVtoR4, lctx->type.stackOffset);
			ctx->bc.Instr(asBC_ClrHi);
			if( op == ttAnd )
			{
				ctx->bc.InstrDWORD(asBC_JNZ, label1);
				ctx->bc.InstrW_DW(asBC_SetV4, (asWORD)offset, 0);
				ctx->bc.InstrINT(asBC_JMP, label2);
			}
			else if( op == ttOr )
			{
				ctx->bc.InstrDWORD(asBC_JZ, label1);
#if AS_SIZEOF_BOOL == 1
				ctx->bc.InstrSHORT_B(asBC_SetV1, (short)offset, VALUE_OF_BOOLEAN_TRUE);
#else
				ctx->bc.InstrSHORT_DW(asBC_SetV4, (short)offset, VALUE_OF_BOOLEAN_TRUE);
#endif
				ctx->bc.InstrINT(asBC_JMP, label2);
			}

			ctx->bc.Label((short)label1);
			ConvertToVariable(rctx);
			ReleaseTemporaryVariable(rctx->type, &rctx->bc);
			rctx->bc.InstrW_W(asBC_CpyVtoV4, offset, rctx->type.stackOffset);
			MergeExprBytecode(ctx, rctx);
			ctx->bc.Label((short)label2);

			ctx->type.SetVariable(asCDataType::CreatePrimitive(ttBool, false), offset, true);
		}
		else
		{
#if AS_SIZEOF_BOOL == 1
			asBYTE v = 0;
			if( op == ttAnd )
				v = lctx->type.byteValue && rctx->type.byteValue;
			else if( op == ttOr )
				v = lctx->type.byteValue || rctx->type.byteValue;

			// Remember the result
			ctx->type.isConstant = true;
			ctx->type.byteValue = v;
#else
			asDWORD v = 0;
			if( op == ttAnd )
				v = lctx->type.dwordValue && rctx->type.dwordValue;
			else if( op == ttOr )
				v = lctx->type.dwordValue || rctx->type.dwordValue;

			// Remember the result
			ctx->type.isConstant = true;
			ctx->type.dwordValue = v;
#endif
		}
	}
}

void asCCompiler::CompileOperatorOnHandles(asCScriptNode *node, asSExprContext *lctx, asSExprContext *rctx, asSExprContext *ctx)
{
	// Process the property accessor as get
	ProcessPropertyGetAccessor(lctx, node);
	ProcessPropertyGetAccessor(rctx, node);

	DetermineSingleFunc(lctx, node);
	DetermineSingleFunc(rctx, node);

	// Make sure lctx doesn't end up with a variable used in rctx
	if( lctx->type.isTemporary && rctx->bc.IsVarUsed(lctx->type.stackOffset) )
	{
		asCArray<int> vars;
		rctx->bc.GetVarsUsed(vars);
		int offset = AllocateVariable(lctx->type.dataType, true);
		rctx->bc.ExchangeVar(lctx->type.stackOffset, offset);
		ReleaseTemporaryVariable(offset, 0);
	}

	// Warn if not both operands are explicit handles or null handles
	if( (node->tokenType == ttEqual || node->tokenType == ttNotEqual) &&
		((!(lctx->type.isExplicitHandle || lctx->type.IsNullConstant()) && !(lctx->type.dataType.GetObjectType() && (lctx->type.dataType.GetObjectType()->flags & asOBJ_IMPLICIT_HANDLE))) ||
		 (!(rctx->type.isExplicitHandle || rctx->type.IsNullConstant()) && !(rctx->type.dataType.GetObjectType() && (rctx->type.dataType.GetObjectType()->flags & asOBJ_IMPLICIT_HANDLE)))) )
	{
		Warning(TXT_HANDLE_COMPARISON, node);
	}

	// If one of the operands is a value type used as handle, we should look for the opEquals method
	if( ((lctx->type.dataType.GetObjectType() && (lctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE)) ||
		 (rctx->type.dataType.GetObjectType() && (rctx->type.dataType.GetObjectType()->flags & asOBJ_ASHANDLE))) &&
		(node->tokenType == ttEqual || node->tokenType == ttIs ||
		 node->tokenType == ttNotEqual || node->tokenType == ttNotIs) )
	{
		// TODO: Should evaluate which of the two have the best match. If both have equal match, the first version should be used
		// Find the matching opEquals method
		int r = CompileOverloadedDualOperator2(node, "opEquals", lctx, rctx, ctx, true, asCDataType::CreatePrimitive(ttBool, false));
		if( r == 0 )
		{
			// Try again by switching the order of the operands
			r = CompileOverloadedDualOperator2(node, "opEquals", rctx, lctx, ctx, true, asCDataType::CreatePrimitive(ttBool, false));
		}

		if( r == 1 )
		{
			if( node->tokenType == ttNotEqual || node->tokenType == ttNotIs )
				ctx->bc.InstrSHORT(asBC_NOT, ctx->type.stackOffset);

			// Success, don't continue
			return;
		}
		else if( r == 0 )
		{
			// Couldn't find opEquals method
			Error(TXT_NO_APPROPRIATE_OPEQUALS, node);
		}

		// Compiler error, don't continue
		ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), true);
		return;
	}


	// Implicitly convert null to the other type
	asCDataType to;
	if( lctx->type.IsNullConstant() )
		to = rctx->type.dataType;
	else if( rctx->type.IsNullConstant() )
		to = lctx->type.dataType;
	else
	{
		// TODO: Use the common base type
		to = lctx->type.dataType;
	}

	// Need to pop the value if it is a null constant
	if( lctx->type.IsNullConstant() )
		lctx->bc.Instr(asBC_PopPtr);
	if( rctx->type.IsNullConstant() )
		rctx->bc.Instr(asBC_PopPtr);

	// Convert both sides to explicit handles
	to.MakeHandle(true);
	to.MakeReference(false);

	if( !to.IsObjectHandle() )
	{
		// Compiler error, don't continue
		Error(TXT_OPERANDS_MUST_BE_HANDLES, node);
		ctx->type.SetConstantDW(asCDataType::CreatePrimitive(ttBool, true), true);
		return;
	}

	// Do the conversion
	ImplicitConversion(lctx, to, node, asIC_IMPLICIT_CONV);
	ImplicitConversion(rctx, to, node, asIC_IMPLICIT_CONV);

	// Both operands must be of the same type

	// Verify that the conversion was successful
	if( !lctx->type.dataType.IsEqualExceptConst(to) )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, lctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
		Error(str, node);
	}

	if( !rctx->type.dataType.IsEqualExceptConst(to) )
	{
		asCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, rctx->type.dataType.Format().AddressOf(), to.Format().AddressOf());
		Error(str, node);
	}

	// Make sure it really is handles that are being compared
	if( !lctx->type.dataType.IsObjectHandle() )
	{
		Error(TXT_OPERANDS_MUST_BE_HANDLES, node);
	}

	ctx->type.Set(asCDataType::CreatePrimitive(ttBool, true));

	int op = node->tokenType;
	if( op == ttEqual || op == ttNotEqual || op == ttIs || op == ttNotIs )
	{
		// If the object handle already is in a variable we must manually pop it from the stack
		if( lctx->type.isVariable )
			lctx->bc.Instr(asBC_PopPtr);
		if( rctx->type.isVariable )
			rctx->bc.Instr(asBC_PopPtr);

		// TODO: runtime optimize: don't do REFCPY
		ConvertToVariableNotIn(lctx, rctx);
		ConvertToVariable(rctx);

		MergeExprBytecode(ctx, lctx);
		MergeExprBytecode(ctx, rctx);

		int a = AllocateVariable(ctx->type.dataType, true);
		int b = lctx->type.stackOffset;
		int c = rctx->type.stackOffset;

		ctx->bc.InstrW_W(asBC_CmpPtr, b, c);

		if( op == ttEqual || op == ttIs )
			ctx->bc.Instr(asBC_TZ);
		else if( op == ttNotEqual || op == ttNotIs )
			ctx->bc.Instr(asBC_TNZ);

		ctx->bc.InstrSHORT(asBC_CpyRtoV4, (short)a);

		ctx->type.SetVariable(asCDataType::CreatePrimitive(ttBool, true), a, true);

		ReleaseTemporaryVariable(lctx->type, &ctx->bc);
		ReleaseTemporaryVariable(rctx->type, &ctx->bc);
		ProcessDeferredParams(ctx);
	}
	else
	{
		// TODO: Use TXT_ILLEGAL_OPERATION_ON
		Error(TXT_ILLEGAL_OPERATION, node);
	}
}


void asCCompiler::PerformFunctionCall(int funcId, asSExprContext *ctx, bool isConstructor, asCArray<asSExprContext*> *args, asCObjectType *objType, bool useVariable, int varOffset, int funcPtrVar)
{
	asCScriptFunction *descr = builder->GetFunctionDescription(funcId);

	// A shared object may not call non-shared functions
	if( outFunc->IsShared() && !descr->IsShared() )
	{
		asCString msg;
		msg.Format(TXT_SHARED_CANNOT_CALL_NON_SHARED_FUNC_s, descr->GetDeclarationStr().AddressOf());
		Error(msg, ctx->exprNode);
	}

	// Check if the function is private
	if( descr->isPrivate && descr->GetObjectType() != outFunc->GetObjectType() )
	{
		asCString msg;
		msg.Format(TXT_PRIVATE_METHOD_CALL_s, descr->GetDeclarationStr().AddressOf());
		Error(msg, ctx->exprNode);
	}

	int argSize = descr->GetSpaceNeededForArguments();

	if( descr->objectType && descr->returnType.IsReference() &&
		!(ctx->type.isVariable || ctx->type.isTemporary) &&
		(ctx->type.dataType.IsObjectHandle() || ctx->type.dataType.SupportHandles()) &&
		!(ctx->type.dataType.GetObjectType()->GetFlags() & asOBJ_SCOPED) &&
		!(ctx->type.dataType.GetObjectType()->GetFlags() & asOBJ_ASHANDLE) )
	{
		// The class method we're calling is returning a reference, which may be to a member of the object.
		// In order to guarantee the lifetime of the reference, we must hold a local reference to the object.
		// TODO: runtime optimize: This can be avoided for local variables (non-handles) as they have a well defined life time
		int tempRef = AllocateVariable(ctx->type.dataType, true);
		ctx->bc.InstrSHORT(asBC_PSF, (short)tempRef);
		ctx->bc.InstrPTR(asBC_REFCPY, ctx->type.dataType.GetObjectType());

		// Add the release of this reference, as a deferred expression
		asSDeferredParam deferred;
		deferred.origExpr = 0;
		deferred.argInOutFlags = asTM_INREF;
		deferred.argNode = 0;
		deferred.argType.SetVariable(ctx->type.dataType, tempRef, true);

		ctx->deferredParams.PushLast(deferred);

		// Forget the current type
		ctx->type.SetDummy();
	}

	if( isConstructor )
	{
		// Sometimes the value types are allocated on the heap,
		// which is when this way of constructing them is used.

		asASSERT(useVariable == false);

		ctx->bc.Alloc(asBC_ALLOC, objType, descr->id, argSize+AS_PTR_SIZE);

		// The instruction has already moved the returned object to the variable
		ctx->type.Set(asCDataType::CreatePrimitive(ttVoid, false));
		ctx->type.isLValue = false;

		// Clean up arguments
		if( args )
			AfterFunctionCall(funcId, *args, ctx, false);

		ProcessDeferredParams(ctx);

		return;
	}
	else
	{
		if( descr->objectType )
			argSize += AS_PTR_SIZE;

		// If the function returns an object by value the address of the location
		// where the value should be stored is passed as an argument too
		if( descr->DoesReturnOnStack() )
		{
			argSize += AS_PTR_SIZE;
		}

		// TODO: runtime optimize: If it is known that a class method cannot be overridden the call
		//                         should be made with asBC_CALL as it is faster. Examples where this
		//                         is known is for example finalled methods where the class doesn't derive
		//                         from any other, or even non-finalled methods but where it is known
		//                         at compile time the true type of the object. The first should be
		//                         quite easy to determine, but the latter will be quite complex and possibly
		//                         not worth it.
		if( descr->funcType == asFUNC_IMPORTED )
			ctx->bc.Call(asBC_CALLBND , descr->id, argSize);
		// TODO: Maybe we need two different byte codes
		else if( descr->funcType == asFUNC_INTERFACE || descr->funcType == asFUNC_VIRTUAL )
			ctx->bc.Call(asBC_CALLINTF, descr->id, argSize);
		else if( descr->funcType == asFUNC_SCRIPT )
			ctx->bc.Call(asBC_CALL    , descr->id, argSize);
		else if( descr->funcType == asFUNC_SYSTEM )
			ctx->bc.Call(asBC_CALLSYS , descr->id, argSize);
		else if( descr->funcType == asFUNC_FUNCDEF )
			ctx->bc.CallPtr(asBC_CallPtr, funcPtrVar, argSize);
	}

	if( descr->returnType.IsObject() && !descr->returnType.IsReference() )
	{
		int returnOffset = 0;

		if( descr->DoesReturnOnStack() )
		{
			asASSERT( useVariable );

			// The variable was allocated before the function was called
			returnOffset = varOffset;
			ctx->type.SetVariable(descr->returnType, returnOffset, true);

			// The variable was initialized by the function, so we need to mark it as initialized here
			ctx->bc.ObjInfo(varOffset, asOBJ_INIT);
		}
		else
		{
			if( useVariable )
			{
				// Use the given variable
				returnOffset = varOffset;
				ctx->type.SetVariable(descr->returnType, returnOffset, false);
			}
			else
			{
				// Allocate a temporary variable for the returned object
				// The returned object will actually be allocated on the heap, so
				// we must force the allocation of the variable to do the same
				returnOffset = AllocateVariable(descr->returnType, true, !descr->returnType.IsObjectHandle());
				ctx->type.SetVariable(descr->returnType, returnOffset, true);
			}

			// Move the pointer from the object register to the temporary variable
			ctx->bc.InstrSHORT(asBC_STOREOBJ, (short)returnOffset);
		}

		ctx->type.dataType.MakeReference(IsVariableOnHeap(returnOffset));
		ctx->type.isLValue = false; // It is a reference, but not an lvalue

		// Clean up arguments
		if( args )
			AfterFunctionCall(funcId, *args, ctx, false);

		ProcessDeferredParams(ctx);

		ctx->bc.InstrSHORT(asBC_PSF, (short)returnOffset);
	}
	else if( descr->returnType.IsReference() )
	{
		asASSERT(useVariable == false);

		// We cannot clean up the arguments yet, because the
		// reference might be pointing to one of them.
		if( args )
			AfterFunctionCall(funcId, *args, ctx, true);

		// Do not process the output parameters yet, because it
		// might invalidate the returned reference

		// If the context holds a variable that needs cleanup
		// store it as a deferred parameter so it will be cleaned up
		// afterwards.
		if( ctx->type.isTemporary )
		{
			asSDeferredParam defer;
			defer.argNode = 0;
			defer.argType = ctx->type;
			defer.argInOutFlags = asTM_INOUTREF;
			defer.origExpr = 0;
			ctx->deferredParams.PushLast(defer);
		}

		ctx->type.Set(descr->returnType);
		if( !descr->returnType.IsPrimitive() )
		{
			ctx->bc.Instr(asBC_PshRPtr);
			if( descr->returnType.IsObject() &&
				!descr->returnType.IsObjectHandle() )
			{
				// We are getting the pointer to the object
				// not a pointer to a object variable
				ctx->type.dataType.MakeReference(false);
			}
		}

		// A returned reference can be used as lvalue
		ctx->type.isLValue = true;
	}
	else
	{
		asASSERT(useVariable == false);

		if( descr->returnType.GetSizeInMemoryBytes() )
		{
			// Allocate a temporary variable to hold the value, but make sure
			// the temporary variable isn't used in any of the deferred arguments
			int l = int(reservedVariables.GetLength());
			for( asUINT n = 0; args && n < args->GetLength(); n++ )
			{
				asSExprContext *expr = (*args)[n]->origExpr;
				if( expr )
					expr->bc.GetVarsUsed(reservedVariables);
			}
			int offset = AllocateVariable(descr->returnType, true);
			reservedVariables.SetLength(l);

			ctx->type.SetVariable(descr->returnType, offset, true);

			// Move the value from the return register to the variable
			if( descr->returnType.GetSizeOnStackDWords() == 1 )
				ctx->bc.InstrSHORT(asBC_CpyRtoV4, (short)offset);
			else if( descr->returnType.GetSizeOnStackDWords() == 2 )
				ctx->bc.InstrSHORT(asBC_CpyRtoV8, (short)offset);
		}
		else
			ctx->type.Set(descr->returnType);

		ctx->type.isLValue = false;

		// Clean up arguments
		if( args )
			AfterFunctionCall(funcId, *args, ctx, false);

		ProcessDeferredParams(ctx);
	}
}

// This only merges the bytecode, but doesn't modify the type of the final context
void asCCompiler::MergeExprBytecode(asSExprContext *before, asSExprContext *after)
{
	before->bc.AddCode(&after->bc);

	for( asUINT n = 0; n < after->deferredParams.GetLength(); n++ )
	{
		before->deferredParams.PushLast(after->deferredParams[n]);
		after->deferredParams[n].origExpr = 0;
	}

	after->deferredParams.SetLength(0);
}

// This merges both bytecode and the type of the final context
void asCCompiler::MergeExprBytecodeAndType(asSExprContext *before, asSExprContext *after)
{
	MergeExprBytecode(before, after);

	before->type            = after->type;
	before->property_get    = after->property_get;
	before->property_set    = after->property_set;
	before->property_const  = after->property_const;
	before->property_handle = after->property_handle;
	before->property_ref    = after->property_ref;
	before->property_arg    = after->property_arg;
	before->exprNode        = after->exprNode;
	before->methodName      = after->methodName;
	before->enumValue       = after->enumValue;

	after->property_arg = 0;

	// Do not copy the origExpr member
}

void asCCompiler::FilterConst(asCArray<int> &funcs, bool removeConst)
{
	if( funcs.GetLength() == 0 ) return;

	// This is only done for object methods
	asCScriptFunction *desc = builder->GetFunctionDescription(funcs[0]);
	if( desc->objectType == 0 ) return;

	// Check if there are any non-const matches
	asUINT n;
	bool foundNonConst = false;
	for( n = 0; n < funcs.GetLength(); n++ )
	{
		desc = builder->GetFunctionDescription(funcs[n]);
		if( desc->isReadOnly != removeConst )
		{
			foundNonConst = true;
			break;
		}
	}

	if( foundNonConst )
	{
		// Remove all const methods
		for( n = 0; n < funcs.GetLength(); n++ )
		{
			desc = builder->GetFunctionDescription(funcs[n]);
			if( desc->isReadOnly == removeConst )
			{
				if( n == funcs.GetLength() - 1 )
					funcs.PopLast();
				else
					funcs[n] = funcs.PopLast();

				n--;
			}
		}
	}
}

END_AS_NAMESPACE

#endif // AS_NO_COMPILER



