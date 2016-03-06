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
// as_parser.h
//
// This class parses the script code and builds a tree for compilation
//



#ifndef AS_PARSER_H
#define AS_PARSER_H

#include "as_scriptnode.h"
#include "as_scriptcode.h"
#include "as_builder.h"
#include "as_tokenizer.h"

BEGIN_AS_NAMESPACE

class asCParser
{
public:
	asCParser(asCBuilder *builder);
	~asCParser();

	int ParseFunctionDefinition(asCScriptCode *script, bool expectListPattern);
	int ParsePropertyDeclaration(asCScriptCode *script);
	int ParseDataType(asCScriptCode *script, bool isReturnType);
	int ParseTemplateDecl(asCScriptCode *script);

#ifndef AS_NO_COMPILER
	int ParseScript(asCScriptCode *script);

	// Called from compiler
	int ParseStatementBlock(asCScriptCode *script, asCScriptNode *block);
	int ParseVarInit(asCScriptCode *script, asCScriptNode *init);
	int ParseExpression(asCScriptCode *script);
#endif
	
	asCScriptNode *GetScriptNode();

protected:
	void Reset();

	void GetToken(sToken *token);
	void RewindTo(const sToken *token);
	void SetPos(size_t pos);
	void Error(const asCString &text, sToken *token);
	void Warning(const asCString &text, sToken *token);
	void Info(const asCString &text, sToken *token);

	asCScriptNode *CreateNode(eScriptNode type);

	asCScriptNode *ParseFunctionDefinition();
	asCScriptNode *ParseParameterList();
	asCScriptNode *SuperficiallyParseExpression();
	asCScriptNode *ParseType(bool allowConst, bool allowVariableType = false, bool allowAuto = false);
	asCScriptNode *ParseTypeMod(bool isParam);
	void           ParseOptionalScope(asCScriptNode *node);
	asCScriptNode *ParseRealType();
	asCScriptNode *ParseDataType(bool allowVariableType = false, bool allowAuto = false);
	asCScriptNode *ParseIdentifier();

	asCScriptNode *ParseListPattern();

	bool IsRealType(int tokenType);
	bool IsDataType(const sToken &token);
	bool IdentifierIs(const sToken &t, const char *str);

#ifndef AS_NO_COMPILER
	// Statements
	asCScriptNode *SuperficiallyParseStatementBlock();
	asCScriptNode *SuperficiallyParseVarInit();
	asCScriptNode *ParseStatementBlock();
	asCScriptNode *ParseStatement();
	asCScriptNode *ParseExpressionStatement();
	asCScriptNode *ParseSwitch();
	asCScriptNode *ParseCase();
	asCScriptNode *ParseIf();
	asCScriptNode *ParseFor();
	asCScriptNode *ParseWhile();
	asCScriptNode *ParseDoWhile();
	asCScriptNode *ParseReturn();
	asCScriptNode *ParseBreak();
	asCScriptNode *ParseContinue();

	// Declarations
	asCScriptNode *ParseDeclaration(bool isClassProp = false, bool isGlobalVar = false);
	asCScriptNode *ParseImport();
	asCScriptNode *ParseScript(bool inBlock);
	asCScriptNode *ParseNamespace();
	asCScriptNode *ParseFunction(bool isMethod = false);
	asCScriptNode *ParseFuncDef();
	asCScriptNode *ParseClass();
	asCScriptNode *ParseMixin();
	asCScriptNode *ParseInitList();
	asCScriptNode *ParseInterface();
	asCScriptNode *ParseInterfaceMethod();
	asCScriptNode *ParseVirtualPropertyDecl(bool isMethod, bool isInterface);
	asCScriptNode *ParseEnumeration();
	asCScriptNode *ParseTypedef();
	void ParseMethodOverrideBehaviors(asCScriptNode *funcNode);
	bool IsVarDecl();
	bool IsVirtualPropertyDecl();
	bool IsFuncDecl(bool isMethod);
	bool IsLambda();
	bool IsFunctionCall();

	// Expressions
	asCScriptNode *ParseAssignment();
	asCScriptNode *ParseAssignOperator();
	asCScriptNode *ParseCondition();
	asCScriptNode *ParseExpression();
	asCScriptNode *ParseExprTerm();
	asCScriptNode *ParseExprOperator();
	asCScriptNode *ParseExprPreOp();
	asCScriptNode *ParseExprPostOp();
	asCScriptNode *ParseExprValue();
	asCScriptNode *ParseArgList(bool withParenthesis = true);
	asCScriptNode *ParseFunctionCall();
	asCScriptNode *ParseVariableAccess();
	asCScriptNode *ParseConstructCall();
	asCScriptNode *ParseCast();
	asCScriptNode *ParseConstant();
	asCScriptNode *ParseStringConstant();
	asCScriptNode *ParseLambda();

	bool IsConstant(int tokenType);
	bool IsOperator(int tokenType);
	bool IsPreOperator(int tokenType);
	bool IsPostOperator(int tokenType);
	bool IsAssignOperator(int tokenType);

	bool CheckTemplateType(sToken &t);
#endif

	asCScriptNode *ParseToken(int token);
	asCScriptNode *ParseOneOf(int *tokens, int num);

	asCString ExpectedToken(const char *token);
	asCString ExpectedTokens(const char *token1, const char *token2);
	asCString ExpectedOneOf(int *tokens, int count);
	asCString ExpectedOneOf(const char **tokens, int count);
	asCString InsteadFound(sToken &t);

	bool errorWhileParsing;
	bool isSyntaxError;
	bool checkValidTypes;
	bool isParsingAppInterface;

	asCScriptEngine *engine;
	asCBuilder      *builder;
	asCScriptCode   *script;
	asCScriptNode   *scriptNode;

	asCString tempString; // Used for reduzing amount of dynamic allocations

	sToken       lastToken;
	size_t       sourcePos;
};

END_AS_NAMESPACE

#endif
