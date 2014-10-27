/*
   AngelCode Scripting Library
   Copyright (c) 2003-2013 Andreas Jonsson

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
// as_compiler.h
//
// The class that does the actual compilation of the functions
//



#ifndef AS_COMPILER_H
#define AS_COMPILER_H

#include "as_config.h"

#ifndef AS_NO_COMPILER

#include "as_builder.h"
#include "as_scriptfunction.h"
#include "as_variablescope.h"
#include "as_bytecode.h"
#include "as_array.h"
#include "as_datatype.h"
#include "as_typeinfo.h"

BEGIN_AS_NAMESPACE

struct asSExprContext;

struct asSDeferredParam
{
	asSDeferredParam() {argNode = 0; origExpr = 0;}

	asCScriptNode  *argNode;
	asCTypeInfo     argType;
	int             argInOutFlags;
	asSExprContext *origExpr;
};

// TODO: refactor: asSExprContext should have indicators to inform where the value is, 
//                 i.e. if the reference to an object is pushed on the stack or not, etc

struct asSExprContext
{
	asSExprContext(asCScriptEngine *engine) : bc(engine) 
	{
		exprNode        = 0; 
		origExpr        = 0; 
		property_get    = 0; 
		property_set    = 0; 
		property_const  = false;
		property_handle = false;
		property_ref    = false;
		property_arg    = 0;
	}
	~asSExprContext() 
	{
		if( property_arg )
			asDELETE(property_arg, asSExprContext);
	}
	void Clear()
	{
		bc.ClearAll();
		type.SetDummy();
		if( property_arg )
			asDELETE(property_arg, asSExprContext);
		property_arg = 0;
		deferredParams.SetLength(0);
		exprNode        = 0; 
		origExpr        = 0; 
		property_get    = 0; 
		property_set    = 0; 
		property_const  = false;
		property_handle = false;
		property_ref    = false;
		methodName      = "";
	}
	bool IsClassMethod()
	{
		if( type.dataType.GetObjectType() == 0 ) return false;
		if( methodName == "" ) return false;
		if( type.dataType.GetObjectType() == &type.dataType.GetObjectType()->engine->functionBehaviours ) return false;
		return true;
	}
	bool IsGlobalFunc()
	{
		if( type.dataType.GetObjectType() == 0 ) return false;
		if( methodName == "" ) return false;
		if( type.dataType.GetObjectType() != &type.dataType.GetObjectType()->engine->functionBehaviours ) return false;
		return true;
	}

	asCByteCode bc;
	asCTypeInfo type;
	int  property_get;
	int  property_set;
	bool property_const;  // If the object that is being accessed through property accessor is read-only
	bool property_handle; // If the property accessor is called on an object stored in a handle
	bool property_ref;    // If the property accessor is called on a reference
	asSExprContext *property_arg;
	asCArray<asSDeferredParam> deferredParams;
	asCScriptNode  *exprNode;
	asSExprContext *origExpr;
	// TODO: cleanup: use ambiguousName and an enum to say if it is a method, global func, or enum value
	asCString methodName;
	asCString enumValue;
};

struct asSOverloadCandidate
{
	asSOverloadCandidate() : funcId(0), cost(0) {}
	asSOverloadCandidate(int _id, asUINT _cost ) : funcId(_id), cost(_cost) {}
	int funcId;
	asUINT cost;
};

enum EImplicitConv
{
	asIC_IMPLICIT_CONV,
	asIC_EXPLICIT_REF_CAST,
	asIC_EXPLICIT_VAL_CAST
};

enum EConvCost
{
	asCC_NO_CONV               = 0,
	asCC_CONST_CONV            = 1,
	asCC_PRIMITIVE_SIZE_CONV   = 2,
	asCC_SIGNED_CONV           = 3,
	asCC_INT_FLOAT_CONV        = 4,
	asCC_REF_CONV              = 5,
	asCC_OBJ_TO_PRIMITIVE_CONV = 6,
	asCC_TO_OBJECT_CONV        = 7,
	asCC_VARIABLE_CONV         = 8
};

class asCCompiler
{
public:
	asCCompiler(asCScriptEngine *engine);
	~asCCompiler();

	int CompileFunction(asCBuilder *builder, asCScriptCode *script, asCArray<asCString> &parameterNames, asCScriptNode *func, asCScriptFunction *outFunc, sClassDeclaration *classDecl);
	int CompileDefaultConstructor(asCBuilder *builder, asCScriptCode *script, asCScriptNode *node, asCScriptFunction *outFunc, sClassDeclaration *classDecl);
	int CompileFactory(asCBuilder *builder, asCScriptCode *script, asCScriptFunction *outFunc);
	int CompileGlobalVariable(asCBuilder *builder, asCScriptCode *script, asCScriptNode *expr, sGlobalVariableDescription *gvar, asCScriptFunction *outFunc);

protected:
	friend class asCBuilder;

	void Reset(asCBuilder *builder, asCScriptCode *script, asCScriptFunction *outFunc);

	// Statements
	void CompileStatementBlock(asCScriptNode *block, bool ownVariableScope, bool *hasReturn, asCByteCode *bc);
	void CompileDeclaration(asCScriptNode *decl, asCByteCode *bc);
	void CompileStatement(asCScriptNode *statement, bool *hasReturn, asCByteCode *bc);
	void CompileIfStatement(asCScriptNode *node, bool *hasReturn, asCByteCode *bc);
	void CompileSwitchStatement(asCScriptNode *node, bool *hasReturn, asCByteCode *bc);
	void CompileCase(asCScriptNode *node, asCByteCode *bc);
	void CompileForStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileWhileStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileDoWhileStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileBreakStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileContinueStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileReturnStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileExpressionStatement(asCScriptNode *node, asCByteCode *bc);

	// Expressions
	int  CompileAssignment(asCScriptNode *expr, asSExprContext *out);
	int  CompileCondition(asCScriptNode *expr, asSExprContext *out);
	int  CompileExpression(asCScriptNode *expr, asSExprContext *out);
	int  CompilePostFixExpression(asCArray<asCScriptNode *> *postfix, asSExprContext *out);
	int  CompileExpressionTerm(asCScriptNode *node, asSExprContext *out);
	int  CompileExpressionPreOp(asCScriptNode *node, asSExprContext *out);
	int  CompileExpressionPostOp(asCScriptNode *node, asSExprContext *out);
	int  CompileExpressionValue(asCScriptNode *node, asSExprContext *out);
	int  CompileFunctionCall(asCScriptNode *node, asSExprContext *out, asCObjectType *objectType, bool objIsConst, const asCString &scope = "");
	void CompileConstructCall(asCScriptNode *node, asSExprContext *out);
	void CompileConversion(asCScriptNode *node, asSExprContext *out);
	int  CompileOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileOperatorOnHandles(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileMathOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileBitwiseOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileComparisonOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileBooleanOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	bool CompileOverloadedDualOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	int  CompileOverloadedDualOperator2(asCScriptNode *node, const char *methodName, asSExprContext *l, asSExprContext *r, asSExprContext *out, bool specificReturn = false, const asCDataType &returnType = asCDataType::CreatePrimitive(ttVoid, false));

	void CompileInitList(asCTypeInfo *var, asCScriptNode *node, asCByteCode *bc, int isVarGlobOrMem);
	int  CompileInitListElement(asSListPatternNode *&patternNode, asCScriptNode *&valueNode, int bufferTypeId, short bufferVar, asUINT &bufferSize, asCByteCode &byteCode);

	int  CallDefaultConstructor(asCDataType &type, int offset, bool isObjectOnHeap, asCByteCode *bc, asCScriptNode *node, int isVarGlobOrMem = 0, bool derefDest = false);
	int  CallCopyConstructor(asCDataType &type, int offset, bool isObjectOnHeap, asCByteCode *bc, asSExprContext *arg, asCScriptNode *node, bool isGlobalVar = false, bool derefDestination = false);
	void CallDestructor(asCDataType &type, int offset, bool isObjectOnHeap, asCByteCode *bc);
	int  CompileArgumentList(asCScriptNode *node, asCArray<asSExprContext *> &args);
	int  CompileDefaultArgs(asCScriptNode *node, asCArray<asSExprContext*> &args, asCScriptFunction *func);
	asUINT MatchFunctions(asCArray<int> &funcs, asCArray<asSExprContext*> &args, asCScriptNode *node, const char *name, asCObjectType *objectType = NULL, bool isConstMethod = false, bool silent = false, bool allowObjectConstruct = true, const asCString &scope = "");
	int  CompileVariableAccess(const asCString &name, const asCString &scope, asSExprContext *ctx, asCScriptNode *errNode, bool isOptional = false, bool noFunction = false, bool noGlobal = false, asCObjectType *objType = 0);
	void CompileMemberInitialization(asCByteCode *bc, bool onlyDefaults);
	bool CompileInitialization(asCScriptNode *node, asCByteCode *bc, asCDataType &type, asCScriptNode *errNode, int offset, asQWORD *constantValue, int isVarGlobOrMem);

	// Helper functions
	void ProcessPropertyGetAccessor(asSExprContext *ctx, asCScriptNode *node);
	int  ProcessPropertySetAccessor(asSExprContext *ctx, asSExprContext *arg, asCScriptNode *node);
	int  FindPropertyAccessor(const asCString &name, asSExprContext *ctx, asCScriptNode *node, asSNameSpace *ns, bool isThisAccess = false);
	int  FindPropertyAccessor(const asCString &name, asSExprContext *ctx, asSExprContext *arg, asCScriptNode *node, asSNameSpace *ns, bool isThisAccess = false);
	void PrepareTemporaryObject(asCScriptNode *node, asSExprContext *ctx, bool forceOnHeap = false);
	void PrepareOperand(asSExprContext *ctx, asCScriptNode *node);
	void PrepareForAssignment(asCDataType *lvalue, asSExprContext *rvalue, asCScriptNode *node, bool toTemporary, asSExprContext *lvalueExpr = 0);
	int  PerformAssignment(asCTypeInfo *lvalue, asCTypeInfo *rvalue, asCByteCode *bc, asCScriptNode *node);
	bool IsVariableInitialized(asCTypeInfo *type, asCScriptNode *node);
	void Dereference(asSExprContext *ctx, bool generateCode);
	bool CompileRefCast(asSExprContext *ctx, const asCDataType &to, bool isExplicit, asCScriptNode *node, bool generateCode = true);
	asUINT MatchArgument(asCArray<int> &funcs, asCArray<asSOverloadCandidate> &matches, const asSExprContext *argExpr, int paramNum, bool allowObjectConstruct = true);
	void PerformFunctionCall(int funcId, asSExprContext *out, bool isConstructor = false, asCArray<asSExprContext*> *args = 0, asCObjectType *objTypeForConstruct = 0, bool useVariable = false, int varOffset = 0, int funcPtrVar = 0);
	void MoveArgsToStack(int funcId, asCByteCode *bc, asCArray<asSExprContext *> &args, bool addOneToOffset);
	void MakeFunctionCall(asSExprContext *ctx, int funcId, asCObjectType *objectType, asCArray<asSExprContext*> &args, asCScriptNode *node, bool useVariable = false, int stackOffset = 0, int funcPtrVar = 0);
	void PrepareFunctionCall(int funcId, asCByteCode *bc, asCArray<asSExprContext *> &args);
	void AfterFunctionCall(int funcId, asCArray<asSExprContext*> &args, asSExprContext *ctx, bool deferAll);
	void ProcessDeferredParams(asSExprContext *ctx);
	void PrepareArgument(asCDataType *paramType, asSExprContext *ctx, asCScriptNode *node, bool isFunction = false, int refType = 0, bool isMakingCopy = false);
	void PrepareArgument2(asSExprContext *ctx, asSExprContext *arg, asCDataType *paramType, bool isFunction = false, int refType = 0, bool isMakingCopy = false);
	bool IsLValue(asCTypeInfo &type);
	int  DoAssignment(asSExprContext *out, asSExprContext *lctx, asSExprContext *rctx, asCScriptNode *lexpr, asCScriptNode *rexpr, int op, asCScriptNode *opNode);
	void MergeExprBytecode(asSExprContext *before, asSExprContext *after);
	void MergeExprBytecodeAndType(asSExprContext *before, asSExprContext *after);
	void FilterConst(asCArray<int> &funcs, bool removeConst = true);
	void ConvertToVariable(asSExprContext *ctx);
	void ConvertToVariableNotIn(asSExprContext *ctx, asSExprContext *exclude);
	void ConvertToTempVariable(asSExprContext *ctx);
	void ConvertToTempVariableNotIn(asSExprContext *ctx, asSExprContext *exclude);
	void ConvertToReference(asSExprContext *ctx);
	void PushVariableOnStack(asSExprContext *ctx, bool asReference);
	void DestroyVariables(asCByteCode *bc);
	asSNameSpace *DetermineNameSpace(const asCString &scope);
	int  SetupParametersAndReturnVariable(asCArray<asCString> &parameterNames, asCScriptNode *func);

	void DetermineSingleFunc(asSExprContext *ctx, asCScriptNode *node);

	// Returns the cost of the conversion (the sum of the EConvCost performed)
	asUINT ImplicitConversion(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode = true, bool allowObjectConstruct = true);
	asUINT ImplicitConvPrimitiveToPrimitive(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode = true);
	asUINT ImplicitConvObjectToPrimitive(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode = true);
	asUINT ImplicitConvPrimitiveToObject(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode = true, bool allowObjectConstruct = true);
	asUINT ImplicitConvObjectToObject(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode = true, bool allowObjectConstruct = true);
	asUINT ImplicitConvObjectRef(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode);
	asUINT ImplicitConvObjectValue(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType, bool generateCode);
	void   ImplicitConversionConstant(asSExprContext *ctx, const asCDataType &to, asCScriptNode *node, EImplicitConv convType);

	void LineInstr(asCByteCode *bc, size_t pos);

	asUINT ProcessStringConstant(asCString &str, asCScriptNode *node, bool processEscapeSequences = true);
	void ProcessHeredocStringConstant(asCString &str, asCScriptNode *node);
	int  GetPrecedence(asCScriptNode *op);
	void Error(const asCString &msg, asCScriptNode *node);
	void Warning(const asCString &msg, asCScriptNode *node);
	void Information(const asCString &msg, asCScriptNode *node);
	void PrintMatchingFuncs(asCArray<int> &funcs, asCScriptNode *node);
	void AddVariableScope(bool isBreakScope = false, bool isContinueScope = false);
	void RemoveVariableScope();
	void FinalizeFunction();

	asCByteCode byteCode;

	bool hasCompileErrors;

	int nextLabel;

	asCVariableScope *variables;
	asCBuilder *builder;
	asCScriptEngine *engine;
	asCScriptCode *script;
	asCScriptFunction *outFunc;

	bool               m_isConstructor;
	bool               m_isConstructorCalled;
	sClassDeclaration *m_classDecl;

	asCArray<int> breakLabels;
	asCArray<int> continueLabels;

	int AllocateVariable(const asCDataType &type, bool isTemporary, bool forceOnHeap = false);
	int AllocateVariableNotIn(const asCDataType &type, bool isTemporary, bool forceOnHeap, asSExprContext *ctx);
	int GetVariableOffset(int varIndex);
	int GetVariableSlot(int varOffset);
	void DeallocateVariable(int pos);
	void ReleaseTemporaryVariable(asCTypeInfo &t, asCByteCode *bc);
	void ReleaseTemporaryVariable(int offset, asCByteCode *bc);
	bool IsVariableOnHeap(int offset);

	// This ordered array indicates the type of each variable
	asCArray<asCDataType> variableAllocations;

	// This ordered array indicates which variables are temporaries or not
	asCArray<bool>        variableIsTemporary;

	// This unordered array gives the offsets of all temporary variables, whether currently allocated or not
	asCArray<int>         tempVariableOffsets;

	// This ordered array indicated if the variable is on the heap or not
	asCArray<bool>        variableIsOnHeap;

	// This unordered array gives the indexes of the currently unused variables
	asCArray<int>         freeVariables;

	// This array holds the offsets of the currently allocated temporary variables
	asCArray<int>         tempVariables; 

	// This array holds the indices of variables that must not be used in an allocation
	asCArray<int>         reservedVariables;

	bool isCompilingDefaultArg;
	bool isProcessingDeferredParams;
	int noCodeOutput;
};

END_AS_NAMESPACE

#endif // AS_NO_COMPILER

#endif
