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
// as_parser.cpp
//
// This class parses the script code and builds a tree for compilation
//
//
// I've documented the syntax in Extended BNF. You'll find it by doing a search in
// this file by "BNF:". The starting point for the script language is SCRIPT ::=.
//
// Ref: http://matt.might.net/articles/grammars-bnf-ebnf/
//
// ( )  - used for grouping
// { }  - 0 or more repetitions
// [ ]  - optional
//  |   - or
// ' '  - token
//



#include "as_config.h"
#include "as_parser.h"
#include "as_tokendef.h"
#include "as_texts.h"
#include "as_debug.h"

#ifdef _MSC_VER
#pragma warning(disable:4702) // unreachable code
#endif

BEGIN_AS_NAMESPACE

asCParser::asCParser(asCBuilder *builder)
{
	this->builder    = builder;
	this->engine     = builder->engine;

	script                = 0;
	scriptNode            = 0;
	checkValidTypes       = false;
	isParsingAppInterface = false;
}

asCParser::~asCParser()
{
	Reset();
}

void asCParser::Reset()
{
	errorWhileParsing     = false;
	isSyntaxError         = false;
	checkValidTypes       = false;
	isParsingAppInterface = false;

	sourcePos = 0;

	if( scriptNode )
	{
		scriptNode->Destroy(engine);
	}

	scriptNode = 0;

	script = 0;

	lastToken.pos = size_t(-1);
}

asCScriptNode *asCParser::GetScriptNode()
{
	return scriptNode;
}

int asCParser::ParseFunctionDefinition(asCScriptCode *in_script, bool in_expectListPattern)
{
	Reset();

	// Set flag that permits ? as datatype for parameters
	isParsingAppInterface = true;

	this->script = in_script;

	scriptNode = ParseFunctionDefinition();

	if( in_expectListPattern )
		scriptNode->AddChildLast(ParseListPattern());

	// The declaration should end after the definition
	if( !isSyntaxError )
	{
		sToken t;
		GetToken(&t);
		if( t.type != ttEnd )
		{
			Error(ExpectedToken(asCTokenizer::GetDefinition(ttEnd)), &t);
			Error(InsteadFound(t), &t);
			return -1;
		}
	}

	if( errorWhileParsing )
		return -1;

	return 0;
}

asCScriptNode *asCParser::CreateNode(eScriptNode type)
{
	void *ptr = engine->memoryMgr.AllocScriptNode();
	if( ptr == 0 )
	{
		// Out of memory
		errorWhileParsing = true;
		return 0;
	}

	return new(ptr) asCScriptNode(type);
}

int asCParser::ParseDataType(asCScriptCode *in_script, bool in_isReturnType)
{
	Reset();

	this->script = in_script;

	scriptNode = CreateNode(snDataType);
	if( scriptNode == 0 ) return -1;

	scriptNode->AddChildLast(ParseType(true));
	if( isSyntaxError ) return -1;

	if( in_isReturnType )
	{
		scriptNode->AddChildLast(ParseTypeMod(false));
		if( isSyntaxError ) return -1;
	}

	// The declaration should end after the type
	sToken t;
	GetToken(&t);
	if( t.type != ttEnd )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttEnd)), &t);
		Error(InsteadFound(t), &t);
		return -1;
	}

	if( errorWhileParsing )
		return -1;

	return 0;
}


// Parse a template declaration: IDENTIFIER '<' 'class'? IDENTIFIER '>'
int asCParser::ParseTemplateDecl(asCScriptCode *in_script)
{
	Reset();

	this->script = in_script;
	scriptNode = CreateNode(snUndefined);
	if( scriptNode == 0 ) return -1;

	scriptNode->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return -1;

	sToken t;
	GetToken(&t);
	if( t.type != ttLessThan )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttLessThan)), &t);
		Error(InsteadFound(t), &t);
		return -1;
	}

	// The class token is optional
	GetToken(&t);
	if( t.type != ttClass )
		RewindTo(&t);

	scriptNode->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return -1;

	// There can be multiple sub types
	GetToken(&t);

	// Parse template types by list separator
	while(t.type == ttListSeparator)
	{
		GetToken(&t);
		if( t.type != ttClass )
			RewindTo(&t);
		scriptNode->AddChildLast(ParseIdentifier());

		if( isSyntaxError ) return -1;
		GetToken(&t);
	}

	if( t.type != ttGreaterThan )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttGreaterThan)), &t);
		Error(InsteadFound(t), &t);
		return -1;
	}

	GetToken(&t);
	if( t.type != ttEnd )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttEnd)), &t);
		Error(InsteadFound(t), &t);
		return -1;
	}

	if( errorWhileParsing )
		return -1;

	return 0;
}

int asCParser::ParsePropertyDeclaration(asCScriptCode *in_script)
{
	Reset();

	this->script = in_script;

	scriptNode = CreateNode(snDeclaration);
	if( scriptNode == 0 ) return -1;

	scriptNode->AddChildLast(ParseType(true));
	if( isSyntaxError ) return -1;

	// Allow optional '&' to indicate that the property is indirect, i.e. stored as reference
	sToken t;
	GetToken(&t);
	RewindTo(&t);
	if( t.type == ttAmp )
		scriptNode->AddChildLast(ParseToken(ttAmp));

	// Allow optional namespace to be defined before the identifier in case
	// the declaration is to be used for searching for an existing property
	ParseOptionalScope(scriptNode);

	scriptNode->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return -1;

	// The declaration should end after the identifier
	GetToken(&t);
	if( t.type != ttEnd )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttEnd)), &t);
		Error(InsteadFound(t), &t);
		return -1;
	}

	return 0;
}

// BNF:5: SCOPE         ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
void asCParser::ParseOptionalScope(asCScriptNode *node)
{
	asCScriptNode *scope = CreateNode(snScope);

	sToken t1, t2;
	GetToken(&t1);
	GetToken(&t2);
	if( t1.type == ttScope )
	{
		RewindTo(&t1);
		scope->AddChildLast(ParseToken(ttScope));
		GetToken(&t1);
		GetToken(&t2);
	}
	while( t1.type == ttIdentifier && t2.type == ttScope )
	{
		RewindTo(&t1);
		scope->AddChildLast(ParseIdentifier());
		scope->AddChildLast(ParseToken(ttScope));
		GetToken(&t1);
		GetToken(&t2);
	}

	// The innermost scope may be a template type
	if( t1.type == ttIdentifier && t2.type == ttLessThan )
	{
		tempString.Assign(&script->code[t1.pos], t1.length);
		if (engine->IsTemplateType(tempString.AddressOf()))
		{
			RewindTo(&t1);
			asCScriptNode *restore = scope->lastChild;
			scope->AddChildLast(ParseIdentifier());
			if (ParseTemplTypeList(scope, false))
			{
				GetToken(&t2);
				if (t2.type == ttScope)
				{
					// Template type is part of the scope
					// Nothing more needs to be done
					node->AddChildLast(scope);
					return;
				}
				else
				{
					// The template type is not part of the scope
					// Rewind to the template type and end the scope
					RewindTo(&t1);

					// Restore the previously parsed node
					while (scope->lastChild != restore)
					{
						asCScriptNode *last = scope->lastChild;
						last->DisconnectParent();
						last->Destroy(engine);
					}
					if( scope->lastChild )
						node->AddChildLast(scope);
					else
						scope->Destroy(engine);
					return;
				}
			}
		}
	}

	// The identifier is not part of the scope
	RewindTo(&t1);

	if (scope->lastChild)
		node->AddChildLast(scope);
	else
		scope->Destroy(engine);
}

asCScriptNode *asCParser::ParseFunctionDefinition()
{
	asCScriptNode *node = CreateNode(snFunction);
	if( node == 0 ) return 0;

	node->AddChildLast(ParseType(true));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseTypeMod(false));
	if( isSyntaxError ) return node;

	ParseOptionalScope(node);

	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseParameterList());
	if( isSyntaxError ) return node;

	// Parse an optional 'const' after the function definition (used for object methods)
	sToken t1;
	GetToken(&t1);
	RewindTo(&t1);
	if( t1.type == ttConst )
		node->AddChildLast(ParseToken(ttConst));

	// Parse optional attributes
	ParseMethodAttributes(node);

	return node;
}

// BNF:4: TYPEMOD       ::= ['&' ['in' | 'out' | 'inout']]
asCScriptNode *asCParser::ParseTypeMod(bool isParam)
{
	asCScriptNode *node = CreateNode(snDataType);
	if( node == 0 ) return 0;

	sToken t;

	// Parse possible & token
	GetToken(&t);
	RewindTo(&t);
	if( t.type == ttAmp )
	{
		node->AddChildLast(ParseToken(ttAmp));
		if( isSyntaxError ) return node;

		if( isParam )
		{
			GetToken(&t);
			RewindTo(&t);

			if( t.type == ttIn || t.type == ttOut || t.type == ttInOut )
			{
				int tokens[3] = {ttIn, ttOut, ttInOut};
				node->AddChildLast(ParseOneOf(tokens, 3));
			}
		}
	}

	// Parse possible + token
	GetToken(&t);
	RewindTo(&t);
	if( t.type == ttPlus )
	{
		node->AddChildLast(ParseToken(ttPlus));
		if( isSyntaxError ) return node;
	}

	// Parse possible if_handle_then_const token
	GetToken(&t);
	RewindTo(&t);
	if (IdentifierIs(t, IF_HANDLE_TOKEN))
	{
		node->AddChildLast(ParseToken(ttIdentifier));
		if (isSyntaxError) return node;
	}

	return node;
}

// BNF:4: TYPE          ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
asCScriptNode *asCParser::ParseType(bool allowConst, bool allowVariableType, bool allowAuto)
{
	asCScriptNode *node = CreateNode(snDataType);
	if( node == 0 ) return 0;

	sToken t;

	if( allowConst )
	{
		GetToken(&t);
		RewindTo(&t);
		if( t.type == ttConst )
		{
			node->AddChildLast(ParseToken(ttConst));
			if( isSyntaxError ) return node;
		}
	}

	// Parse scope prefix
	ParseOptionalScope(node);

	// Parse the actual type
	node->AddChildLast(ParseDataType(allowVariableType, allowAuto));
	if( isSyntaxError ) return node;

	// If the datatype is a template type, then parse the subtype within the < >
	GetToken(&t);
	RewindTo(&t);
	asCScriptNode *type = node->lastChild;
	tempString.Assign(&script->code[type->tokenPos], type->tokenLength);
	if( engine->IsTemplateType(tempString.AddressOf()) && t.type == ttLessThan )
	{
		ParseTemplTypeList(node);
		if (isSyntaxError) return node;
	}

	// Parse [] and @
	GetToken(&t);
	RewindTo(&t);
	while( t.type == ttOpenBracket || t.type == ttHandle)
	{
		if( t.type == ttOpenBracket )
		{
			node->AddChildLast(ParseToken(ttOpenBracket));
			if( isSyntaxError ) return node;

			GetToken(&t);
			if( t.type != ttCloseBracket )
			{
				Error(ExpectedToken("]"), &t);
				Error(InsteadFound(t), &t);
				return node;
			}
		}
		else
		{
			node->AddChildLast(ParseToken(ttHandle));
			if( isSyntaxError ) return node;

			GetToken(&t);
			RewindTo(&t);
			if( t.type == ttConst )
			{
				node->AddChildLast(ParseToken(ttConst));
				if( isSyntaxError ) return node;
			}
		}

		GetToken(&t);
		RewindTo(&t);
	}

	return node;
}

// This parses a template type list, e.g. <type, type, type>
// If 'required' is false, and the template type list is not valid,
// then no change will be done and the function returns false. This
// can be used as do an optional parsing
bool asCParser::ParseTemplTypeList(asCScriptNode *node, bool required)
{
	sToken t;
	bool isValid = true;

	// Remember the last child, so we can restore the state if needed
	asCScriptNode *last = node->lastChild;

	// Starts with '<'
	GetToken(&t);
	if (t.type != ttLessThan)
	{
		if (required)
		{
			Error(ExpectedToken(asCTokenizer::GetDefinition(ttLessThan)), &t);
			Error(InsteadFound(t), &t);
		}
		return false;
	}

	// At least one type
	// TODO: child funcdef: Make this work with !required
	node->AddChildLast(ParseType(true, false));
	if (isSyntaxError) return false;

	GetToken(&t);

	// Parse template types by list separator
	while (t.type == ttListSeparator)
	{
		// TODO: child funcdef: Make this work with !required
		node->AddChildLast(ParseType(true, false));
		if (isSyntaxError) return false;
		GetToken(&t);
	}

	// End with '>'
	// Accept >> and >>> tokens too. But then force the tokenizer to move
	// only 1 character ahead (thus splitting the token in two).
	if (script->code[t.pos] != '>')
	{
		if (required)
		{
			Error(ExpectedToken(asCTokenizer::GetDefinition(ttGreaterThan)), &t);
			Error(InsteadFound(t), &t);
		}
		else
			isValid = false;
	}
	else
	{
		// Break the token so that only the first > is parsed
		SetPos(t.pos + 1);
	}

	if (!required && !isValid)
	{
		// Restore the original state before returning
		while (node->lastChild != last)
		{
			asCScriptNode *n = node->lastChild;
			n->DisconnectParent();
			n->Destroy(engine);
		}

		return false;
	}

	// The template type list was parsed OK
	return true;
}

asCScriptNode *asCParser::ParseToken(int token)
{
	asCScriptNode *node = CreateNode(snUndefined);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( t1.type != token )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(token)), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->SetToken(&t1);
	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

asCScriptNode *asCParser::ParseOneOf(int *tokens, int count)
{
	asCScriptNode *node = CreateNode(snUndefined);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	int n;
	for( n = 0; n < count; n++ )
	{
		if( tokens[n] == t1.type )
			break;
	}
	if( n == count )
	{
		Error(ExpectedOneOf(tokens, count), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->SetToken(&t1);
	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:5: DATATYPE      ::= (IDENTIFIER | PRIMTYPE | '?' | 'auto')
asCScriptNode *asCParser::ParseDataType(bool allowVariableType, bool allowAuto)
{
	asCScriptNode *node = CreateNode(snDataType);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( !IsDataType(t1) && !(allowVariableType && t1.type == ttQuestion) && !(allowAuto && t1.type == ttAuto) )
	{
		if( t1.type == ttIdentifier )
		{
			asCString errMsg;
			tempString.Assign(&script->code[t1.pos], t1.length);
			errMsg.Format(TXT_IDENTIFIER_s_NOT_DATA_TYPE, tempString.AddressOf());
			Error(errMsg, &t1);
		}
		else if( t1.type == ttAuto )
		{
			Error(TXT_AUTO_NOT_ALLOWED, &t1);
		}
		else
		{
			Error(TXT_EXPECTED_DATA_TYPE, &t1);
			Error(InsteadFound(t1), &t1);
		}
		return node;
	}

	node->SetToken(&t1);
	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:6: PRIMTYPE      ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
asCScriptNode *asCParser::ParseRealType()
{
	asCScriptNode *node = CreateNode(snDataType);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( !IsRealType(t1.type) )
	{
		Error(TXT_EXPECTED_DATA_TYPE, &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->SetToken(&t1);
	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:17: IDENTIFIER    ::= single token:  starts with letter or _, can include any letter and digit, same as in C++
asCScriptNode *asCParser::ParseIdentifier()
{
	asCScriptNode *node = CreateNode(snIdentifier);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( t1.type != ttIdentifier )
	{
		Error(TXT_EXPECTED_IDENTIFIER, &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->SetToken(&t1);
	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:3: PARAMLIST     ::= '(' ['void' | (TYPE TYPEMOD [IDENTIFIER] ['=' EXPR] {',' TYPE TYPEMOD [IDENTIFIER] ['=' EXPR]})] ')'
asCScriptNode *asCParser::ParseParameterList()
{
	asCScriptNode *node = CreateNode(snParameterList);
	if( node == 0 ) return 0;

	sToken t1;
	GetToken(&t1);
	if( t1.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	GetToken(&t1);
	if( t1.type == ttCloseParanthesis )
	{
		node->UpdateSourcePos(t1.pos, t1.length);

		// Statement block is finished
		return node;
	}
	else
	{
		// If the parameter list is just (void) then the void token should be ignored
		if( t1.type == ttVoid )
		{
			sToken t2;
			GetToken(&t2);
			if( t2.type == ttCloseParanthesis )
			{
				node->UpdateSourcePos(t2.pos, t2.length);
				return node;
			}
		}

		RewindTo(&t1);

		for(;;)
		{
			// Parse data type
			node->AddChildLast(ParseType(true, isParsingAppInterface));
			if( isSyntaxError ) return node;

			node->AddChildLast(ParseTypeMod(true));
			if( isSyntaxError ) return node;

			// Parse optional identifier
			GetToken(&t1);
			if( t1.type == ttIdentifier )
			{
				RewindTo(&t1);

				node->AddChildLast(ParseIdentifier());
				if( isSyntaxError ) return node;

				GetToken(&t1);
			}

			// Parse optional expression for the default arg
			if( t1.type == ttAssignment )
			{
				// Do a superficial parsing of the default argument
				// The actual parsing will be done when the argument is compiled for a function call
				node->AddChildLast(SuperficiallyParseExpression());
				if( isSyntaxError ) return node;

				GetToken(&t1);
			}

			// Check if list continues
			if( t1.type == ttCloseParanthesis )
			{
				node->UpdateSourcePos(t1.pos, t1.length);

				return node;
			}
			else if( t1.type == ttListSeparator )
				continue;
			else
			{
				Error(ExpectedTokens(")", ","), &t1);
				Error(InsteadFound(t1), &t1);
				return node;
			}
		}
	}
	UNREACHABLE_RETURN;
}

asCScriptNode *asCParser::SuperficiallyParseExpression()
{
	asCScriptNode *node = CreateNode(snExpression);
	if( node == 0 ) return 0;

	// Simply parse everything until the first , or ), whichever comes first.
	// Keeping in mind that () and {} can group expressions.

	sToken start;
	GetToken(&start);
	RewindTo(&start);

	asCString stack;
	sToken t;
	for(;;)
	{
		GetToken(&t);

		if( t.type == ttOpenParanthesis )
			stack += "(";
		else if( t.type == ttCloseParanthesis )
		{
			if( stack == "" )
			{
				// Expression has ended. This token is not part of expression
				RewindTo(&t);
				break;
			}
			else if( stack[stack.GetLength()-1] == '(' )
			{
				// Group has ended
				stack.SetLength(stack.GetLength()-1);
			}
			else
			{
				// Wrong syntax
				RewindTo(&t);
				asCString str;
				str.Format(TXT_UNEXPECTED_TOKEN_s, ")");
				Error(str, &t);
				return node;
			}
		}
		else if( t.type == ttListSeparator )
		{
			if( stack == "" )
			{
				// Expression has ended. This token is not part of expression
				RewindTo(&t);
				break;
			}
		}
		else if( t.type == ttStartStatementBlock )
			stack += "{";
		else if( t.type == ttEndStatementBlock )
		{
			if( stack == "" || stack[stack.GetLength()-1] != '{' )
			{
				// Wrong syntax
				RewindTo(&t);
				asCString str;
				str.Format(TXT_UNEXPECTED_TOKEN_s, "}");
				Error(str, &t);
				return node;
			}
			else
			{
				// Group has ended
				stack.SetLength(stack.GetLength()-1);
			}
		}
		else if( t.type == ttEndStatement )
		{
			// Wrong syntax (since we're parsing a default arg expression)
			RewindTo(&t);
			asCString str;
			str.Format(TXT_UNEXPECTED_TOKEN_s, ";");
			Error(str, &t);
			return node;
		}
		else if( t.type == ttNonTerminatedStringConstant )
		{
			RewindTo(&t);
			Error(TXT_NONTERMINATED_STRING, &t);
			return node;
		}
		else if( t.type == ttEnd )
		{
			// Wrong syntax
			RewindTo(&t);
			Error(TXT_UNEXPECTED_END_OF_FILE, &t);
			Info(TXT_WHILE_PARSING_EXPRESSION, &start);
			return node;
		}

		// Include the token in the node
		node->UpdateSourcePos(t.pos, t.length);
	}

	return node;
}

void asCParser::GetToken(sToken *token)
{
	// Check if the token has already been parsed
	if( lastToken.pos == sourcePos )
	{
		*token = lastToken;
		sourcePos += token->length;

		if( token->type == ttWhiteSpace ||
			token->type == ttOnelineComment ||
			token->type == ttMultilineComment )
			GetToken(token);

		return;
	}

	// Parse new token
	size_t sourceLength = script->codeLength;
	do
	{
		if( sourcePos >= sourceLength )
		{
			token->type = ttEnd;
			token->length = 0;
		}
		else
			token->type = engine->tok.GetToken(&script->code[sourcePos], sourceLength - sourcePos, &token->length);

		token->pos = sourcePos;

		// Update state
		sourcePos += token->length;
	}
	// Filter out whitespace and comments
	while( token->type == ttWhiteSpace ||
	       token->type == ttOnelineComment ||
	       token->type == ttMultilineComment );
}

void asCParser::SetPos(size_t pos)
{
	lastToken.pos = size_t(-1);
	sourcePos = pos;
}

void asCParser::RewindTo(const sToken *token)
{
	// TODO: optimize: Perhaps we can optimize this further by having the parser
	//                 set an explicit return point, after which each token will
	//                 be stored. That way not just one token will be reused but
	//                 no token will have to be tokenized more than once.

	// Store the token so it doesn't have to be tokenized again
	lastToken = *token;

	sourcePos = token->pos;
}

void asCParser::Error(const asCString &text, sToken *token)
{
	RewindTo(token);

	isSyntaxError     = true;
	errorWhileParsing = true;

	int row, col;
	script->ConvertPosToRowCol(token->pos, &row, &col);

	if( builder )
		builder->WriteError(script->name, text, row, col);
}

void asCParser::Warning(const asCString &text, sToken *token)
{
	int row, col;
	script->ConvertPosToRowCol(token->pos, &row, &col);

	if( builder )
		builder->WriteWarning(script->name, text, row, col);
}

void asCParser::Info(const asCString &text, sToken *token)
{
	RewindTo(token);

	isSyntaxError     = true;
	errorWhileParsing = true;

	int row, col;
	script->ConvertPosToRowCol(token->pos, &row, &col);

	if( builder )
		builder->WriteInfo(script->name, text, row, col, false);
}

bool asCParser::IsRealType(int tokenType)
{
	if( tokenType == ttVoid ||
		tokenType == ttInt ||
		tokenType == ttInt8 ||
		tokenType == ttInt16 ||
		tokenType == ttInt64 ||
		tokenType == ttUInt ||
		tokenType == ttUInt8 ||
		tokenType == ttUInt16 ||
		tokenType == ttUInt64 ||
		tokenType == ttFloat ||
		tokenType == ttBool ||
		tokenType == ttDouble )
		return true;

	return false;
}

bool asCParser::IsDataType(const sToken &token)
{
	if( token.type == ttIdentifier )
	{
#ifndef AS_NO_COMPILER
		if( checkValidTypes )
		{
			// Check if this is an existing type, regardless of namespace
			tempString.Assign(&script->code[token.pos], token.length);
			if( !builder->DoesTypeExist(tempString.AddressOf()) )
				return false;
		}
#endif
		return true;
	}

	if( IsRealType(token.type) )
		return true;

	return false;
}

asCString asCParser::ExpectedToken(const char *token)
{
	asCString str;

	str.Format(TXT_EXPECTED_s, token);

	return str;
}

asCString asCParser::ExpectedTokens(const char *t1, const char *t2)
{
	asCString str;

	str.Format(TXT_EXPECTED_s_OR_s, t1, t2);

	return str;
}

asCString asCParser::ExpectedOneOf(int *tokens, int count)
{
	asCString str;

	str = TXT_EXPECTED_ONE_OF;
	for( int n = 0; n < count; n++ )
	{
		str += asCTokenizer::GetDefinition(tokens[n]);
		if( n < count-1 )
			str += ", ";
	}

	return str;
}

asCString asCParser::ExpectedOneOf(const char **tokens, int count)
{
	asCString str;

	str = TXT_EXPECTED_ONE_OF;
	for( int n = 0; n < count; n++ )
	{
		str += tokens[n];
		if( n < count-1 )
			str += ", ";
	}

	return str;
}

asCString asCParser::InsteadFound(sToken &t)
{
	asCString str;
	if( t.type == ttIdentifier )
	{
		asCString id(&script->code[t.pos], t.length);
		str.Format(TXT_INSTEAD_FOUND_IDENTIFIER_s, id.AddressOf());
	}
	else if( t.type >= ttIf )
		str.Format(TXT_INSTEAD_FOUND_KEYWORD_s, asCTokenizer::GetDefinition(t.type));
	else
		str.Format(TXT_INSTEAD_FOUND_s, asCTokenizer::GetDefinition(t.type));

	return str;
}

asCScriptNode *asCParser::ParseListPattern()
{
	asCScriptNode *node = CreateNode(snListPattern);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( t1.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	sToken start = t1;

	bool isBeginning = true;
	bool afterType = false;
	while( !isSyntaxError )
	{
		GetToken(&t1);
		if( t1.type == ttEndStatementBlock )
		{
			if( !afterType )
			{
				Error(TXT_EXPECTED_DATA_TYPE, &t1);
				Error(InsteadFound(t1), &t1);
			}
			break;
		}
		else if( t1.type == ttStartStatementBlock )
		{
			if( afterType )
			{
				Error(ExpectedTokens(",","}"), &t1);
				Error(InsteadFound(t1), &t1);
			}
			RewindTo(&t1);
			node->AddChildLast(ParseListPattern());
			afterType = true;
		}
		else if( t1.type == ttIdentifier && (IdentifierIs(t1, "repeat") || IdentifierIs(t1, "repeat_same")) )
		{
			if( !isBeginning )
			{
				asCString msg;
				asCString token(&script->code[t1.pos], t1.length);
				msg.Format(TXT_UNEXPECTED_TOKEN_s, token.AddressOf());
				Error(msg.AddressOf(), &t1);
			}
			RewindTo(&t1);
			node->AddChildLast(ParseIdentifier());
		}
		else if( t1.type == ttEnd )
		{
			Error(TXT_UNEXPECTED_END_OF_FILE, &t1);
			Info(TXT_WHILE_PARSING_STATEMENT_BLOCK, &start);
			break;
		}
		else if( t1.type == ttListSeparator )
		{
			if( !afterType )
			{
				Error(TXT_EXPECTED_DATA_TYPE, &t1);
				Error(InsteadFound(t1), &t1);
			}
			afterType = false;
		}
		else
		{
			if( afterType )
			{
				Error(ExpectedTokens(",", "}"), &t1);
				Error(InsteadFound(t1), &t1);
			}
			RewindTo(&t1);
			node->AddChildLast(ParseType(true, true));
			afterType = true;
		}

		isBeginning = false;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

bool asCParser::IdentifierIs(const sToken &t, const char *str)
{
	if( t.type != ttIdentifier )
		return false;

	return script->TokenEquals(t.pos, t.length, str);
}

// BNF:6: FUNCATTR      ::= {'override' | 'final' | 'explicit' | 'property'}
void asCParser::ParseMethodAttributes(asCScriptNode *funcNode)
{
	sToken t1;

	for(;;)
	{
		GetToken(&t1);
		RewindTo(&t1);

		if( IdentifierIs(t1, FINAL_TOKEN) || 
			IdentifierIs(t1, OVERRIDE_TOKEN) || 
			IdentifierIs(t1, EXPLICIT_TOKEN) ||
			IdentifierIs(t1, PROPERTY_TOKEN) )
			funcNode->AddChildLast(ParseIdentifier());
		else
			break;
	}
}

#ifndef AS_NO_COMPILER

// nextToken is only modified if the current position can be interpreted as
// type, in this case it is set to the next token after the type tokens
bool asCParser::IsType(sToken &nextToken)
{
	// Set a rewind point
	sToken t, t1;
	GetToken(&t);

	// A type can start with a const
	t1 = t;
	if (t1.type == ttConst)
		GetToken(&t1);

	sToken t2;
	if (t1.type != ttAuto)
	{
		// The type may be initiated with the scope operator
		if (t1.type == ttScope)
			GetToken(&t1);

		// The type may be preceded with a multilevel scope
		GetToken(&t2);
		while (t1.type == ttIdentifier)
		{
			if (t2.type == ttScope)
			{
				GetToken(&t1);
				GetToken(&t2);
				continue;
			}
			else if (t2.type == ttLessThan)
			{
				// Template types can also be used as scope identifiers
				RewindTo(&t2);
				if (CheckTemplateType(t1))
				{
					sToken t3;
					GetToken(&t3);
					if (t3.type == ttScope)
					{
						GetToken(&t1);
						GetToken(&t2);
						continue;
					}
				}
			}

			break;
		}
		RewindTo(&t2);
	}

	// We don't validate if the identifier is an actual declared type at this moment
	// as it may wrongly identify the statement as a non-declaration if the user typed
	// the name incorrectly. The real type is validated in ParseDeclaration where a
	// proper error message can be given.
	if (!IsRealType(t1.type) && t1.type != ttIdentifier && t1.type != ttAuto)
	{
		RewindTo(&t);
		return false;
	}

	if (!CheckTemplateType(t1))
	{
		RewindTo(&t);
		return false;
	}

	// Object handles can be interleaved with the array brackets
	// Even though declaring variables with & is invalid we'll accept
	// it here to give an appropriate error message later
	GetToken(&t2);
	while (t2.type == ttHandle || t2.type == ttAmp || t2.type == ttOpenBracket)
	{
		if( t2.type == ttHandle )
		{
			// A handle can optionally be read-only
			sToken t3;
			GetToken(&t3);
			if(t3.type != ttConst )
				RewindTo(&t3);
		}
		else if (t2.type == ttOpenBracket)
		{
			GetToken(&t2);
			if (t2.type != ttCloseBracket)
			{
				RewindTo(&t);
				return false;
			}
		}

		GetToken(&t2);
	}

	// Return the next token so the caller can jump directly to it if desired
	nextToken = t2;

	// Rewind to start point
	RewindTo(&t);

	return true;
}

// This function will return true if the current token is not a template, or if it is and
// the following has a valid syntax for a template type. The source position will be left
// at the first token after the type in case of success
bool asCParser::CheckTemplateType(const sToken &t)
{
	// Is this a template type?
	tempString.Assign(&script->code[t.pos], t.length);
	if( engine->IsTemplateType(tempString.AddressOf()) )
	{
		// If the next token is a < then parse the sub-type too
		sToken t1;
		GetToken(&t1);
		if( t1.type != ttLessThan )
		{
			RewindTo(&t1);
			return true;
		}

		for(;;)
		{
			// There might optionally be a 'const'
			GetToken(&t1);
			if( t1.type == ttConst )
				GetToken(&t1);

			// The type may be initiated with the scope operator
			if( t1.type == ttScope )
				GetToken(&t1);

			// There may be multiple levels of scope operators
			sToken t2;
			GetToken(&t2);
			while( t1.type == ttIdentifier && t2.type == ttScope )
			{
				GetToken(&t1);
				GetToken(&t2);
			}
			RewindTo(&t2);

			// Now there must be a data type
			if( !IsDataType(t1) )
				return false;

			if( !CheckTemplateType(t1) )
				return false;

			GetToken(&t1);

			// Is it a handle or array?
			while( t1.type == ttHandle || t1.type == ttOpenBracket )
			{
				if( t1.type == ttOpenBracket )
				{
					GetToken(&t1);
					if( t1.type != ttCloseBracket )
						return false;
				}

				GetToken(&t1);
			}

			// Was this the last template subtype?
			if( t1.type != ttListSeparator )
				break;
		}

		// Accept >> and >>> tokens too. But then force the tokenizer to move
		// only 1 character ahead (thus splitting the token in two).
		if( script->code[t1.pos] != '>' )
			return false;
		else if( t1.length != 1 )
		{
			// We need to break the token, so that only the first character is parsed
			SetPos(t1.pos + 1);
		}
	}

	return true;
}

// BNF:12: CAST          ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
asCScriptNode *asCParser::ParseCast()
{
	asCScriptNode *node = CreateNode(snCast);
	if( node == 0 ) return 0;

	sToken t1;
	GetToken(&t1);
	if( t1.type != ttCast )
	{
		Error(ExpectedToken("cast"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	GetToken(&t1);
	if( t1.type != ttLessThan )
	{
		Error(ExpectedToken("<"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	// Parse the data type
	node->AddChildLast(ParseType(true));
	if( isSyntaxError ) return node;

	GetToken(&t1);
	if( t1.type != ttGreaterThan )
	{
		Error(ExpectedToken(">"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	GetToken(&t1);
	if( t1.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t1);
	if( t1.type != ttCloseParanthesis )
	{
		Error(ExpectedToken(")"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:11: EXPRVALUE     ::= 'void' | CONSTRUCTCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
asCScriptNode *asCParser::ParseExprValue()
{
	asCScriptNode *node = CreateNode(snExprValue);
	if( node == 0 ) return 0;

	sToken t1, t2;
	GetToken(&t1);
	GetToken(&t2);
	RewindTo(&t1);

	// 'void' is a special expression that doesn't do anything (normally used for skipping output arguments)
	if( t1.type == ttVoid )
		node->AddChildLast(ParseToken(ttVoid));
	else if( IsRealType(t1.type) )
		node->AddChildLast(ParseConstructCall());
	else if( t1.type == ttIdentifier || t1.type == ttScope )
	{
		// Check if the expression is an anonymous function
		if( IsLambda() )
		{
			node->AddChildLast(ParseLambda());
		}
		else
		{
			// Determine the last identifier in order to check if it is a type
			sToken t;
			if( t1.type == ttScope ) t = t2; else t = t1;
			RewindTo(&t);
			GetToken(&t2);
			while( t.type == ttIdentifier )
			{
				t2 = t;
				GetToken(&t);
				if( t.type == ttScope )
					GetToken(&t);
				else
					break;
			}

			bool isDataType = IsDataType(t2);
			bool isTemplateType = false;
			if( isDataType )
			{
				// Is this a template type?
				tempString.Assign(&script->code[t2.pos], t2.length);
				if( engine->IsTemplateType(tempString.AddressOf()) )
					isTemplateType = true;
			}

			GetToken(&t2);

			// Rewind so the real parsing can be done, after deciding what to parse
			RewindTo(&t1);

			// Check if this is a construct call
			// Just 'type()' isn't considered a construct call, because type may just be a function/method name.
			// The compiler will have to sort this out, since the parser doesn't have enough information.
			if( isDataType && (t.type == ttOpenBracket && t2.type == ttCloseBracket) )      // type[]()
				node->AddChildLast(ParseConstructCall());
			else if( isTemplateType && t.type == ttLessThan )  // type<t>()
				node->AddChildLast(ParseConstructCall());
			else if( IsFunctionCall() )
				node->AddChildLast(ParseFunctionCall());
			else
				node->AddChildLast(ParseVariableAccess());
		}
	}
	else if( t1.type == ttCast )
		node->AddChildLast(ParseCast());
	else if( IsConstant(t1.type) )
		node->AddChildLast(ParseConstant());
	else if( t1.type == ttOpenParanthesis )
	{
		GetToken(&t1);
		node->UpdateSourcePos(t1.pos, t1.length);

		node->AddChildLast(ParseAssignment());
		if( isSyntaxError ) return node;

		GetToken(&t1);
		if( t1.type != ttCloseParanthesis )
		{
			Error(ExpectedToken(")"), &t1);
			Error(InsteadFound(t1), &t1);
		}

		node->UpdateSourcePos(t1.pos, t1.length);
	}
	else
	{
		Error(TXT_EXPECTED_EXPRESSION_VALUE, &t1);
		Error(InsteadFound(t1), &t1);
	}

	return node;
}

// BNF:12: LITERAL       ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
// BNF:17: NUMBER        ::= single token:  includes integers and real numbers, same as C++
// BNF:17: STRING        ::= single token:  single quoted ', double quoted ", or heredoc multi-line string """
// BNF:17: BITS          ::= single token:  binary 0b or 0B, octal 0o or 0O, decimal 0d or 0D, hexadecimal 0x or 0X
asCScriptNode *asCParser::ParseConstant()
{
	asCScriptNode *node = CreateNode(snConstant);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( !IsConstant(t.type) )
	{
		Error(TXT_EXPECTED_CONSTANT, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	// We want to gather a list of string constants to concatenate as children
	if( t.type == ttStringConstant || t.type == ttMultilineStringConstant || t.type == ttHeredocStringConstant )
		RewindTo(&t);

	while( t.type == ttStringConstant || t.type == ttMultilineStringConstant || t.type == ttHeredocStringConstant )
	{
		node->AddChildLast(ParseStringConstant());

		GetToken(&t);
		RewindTo(&t);
	}

	return node;
}

bool asCParser::IsLambda()
{
	bool isLambda = false;
	sToken t;
	GetToken(&t);
	if( t.type == ttIdentifier && IdentifierIs(t, FUNCTION_TOKEN) )
	{
		sToken t2;
		GetToken(&t2);
		if( t2.type == ttOpenParanthesis )
		{
			// Skip until )
			while( t2.type != ttCloseParanthesis && t2.type != ttEnd )
				GetToken(&t2);

			// The next token must be a {
			GetToken(&t2);
			if( t2.type == ttStartStatementBlock )
				isLambda = true;
		}
	}

	RewindTo(&t);
	return isLambda;
}

// BNF:12: LAMBDA        ::= 'function' '(' [[TYPE TYPEMOD] IDENTIFIER {',' [TYPE TYPEMOD] IDENTIFIER}] ')' STATBLOCK
asCScriptNode *asCParser::ParseLambda()
{
	asCScriptNode *node = CreateNode(snFunction);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);

	if( t.type != ttIdentifier || !IdentifierIs(t, FUNCTION_TOKEN) )
	{
		Error(ExpectedToken("function"), &t);
		return node;
	}

	GetToken(&t);
	if( t.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t);
		return node;
	}

	// Parse optional type before parameter name
	if( IsType(t) && (t.type == ttAmp || t.type == ttIdentifier) )
	{
		node->AddChildLast(ParseType(true));
		if (isSyntaxError) return node;
		node->AddChildLast(ParseTypeMod(true));
		if (isSyntaxError) return node;
	}

	GetToken(&t);
	if( t.type == ttIdentifier )
	{
		RewindTo(&t);
		node->AddChildLast(ParseIdentifier());
		if (isSyntaxError) return node;

		GetToken(&t);
		while( t.type == ttListSeparator )
		{
			// Parse optional type before parameter name
			if (IsType(t) && (t.type == ttAmp || t.type == ttIdentifier)) 
			{
				node->AddChildLast(ParseType(true));
				if (isSyntaxError) return node;
				node->AddChildLast(ParseTypeMod(true));
				if (isSyntaxError) return node;
			}

			node->AddChildLast(ParseIdentifier());
			if( isSyntaxError ) return node;

			GetToken(&t);
		}
	}

	if( t.type != ttCloseParanthesis )
	{
		Error(ExpectedToken(")"), &t);
		return node;
	}

	// We should just find the end of the statement block here. The statements
	// will be parsed on request by the compiler once it starts the compilation.
	node->AddChildLast(SuperficiallyParseStatementBlock());

	return node;
}

asCScriptNode *asCParser::ParseStringConstant()
{
	asCScriptNode *node = CreateNode(snConstant);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttStringConstant && t.type != ttMultilineStringConstant && t.type != ttHeredocStringConstant )
	{
		Error(TXT_EXPECTED_STRING, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:12: FUNCCALL      ::= SCOPE IDENTIFIER ARGLIST
asCScriptNode *asCParser::ParseFunctionCall()
{
	asCScriptNode *node = CreateNode(snFunctionCall);
	if( node == 0 ) return 0;

	// Parse scope prefix
	ParseOptionalScope(node);

	// Parse the function name followed by the argument list
	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseArgList());

	return node;
}

// BNF:12: VARACCESS     ::= SCOPE IDENTIFIER
asCScriptNode *asCParser::ParseVariableAccess()
{
	asCScriptNode *node = CreateNode(snVariableAccess);
	if( node == 0 ) return 0;

	// Parse scope prefix
	ParseOptionalScope(node);

	// Parse the variable name
	node->AddChildLast(ParseIdentifier());

	return node;
}

// BNF:11: CONSTRUCTCALL ::= TYPE ARGLIST
asCScriptNode *asCParser::ParseConstructCall()
{
	asCScriptNode *node = CreateNode(snConstructCall);
	if( node == 0 ) return 0;

	node->AddChildLast(ParseType(false));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseArgList());

	return node;
}

// BNF:13: ARGLIST       ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
asCScriptNode *asCParser::ParseArgList(bool withParenthesis)
{
	asCScriptNode *node = CreateNode(snArgList);
	if( node == 0 ) return 0;

	sToken t1;
	if( withParenthesis )
	{
		GetToken(&t1);
		if( t1.type != ttOpenParanthesis )
		{
			Error(ExpectedToken("("), &t1);
			Error(InsteadFound(t1), &t1);
			return node;
		}

		node->UpdateSourcePos(t1.pos, t1.length);
	}

	GetToken(&t1);
	if( t1.type == ttCloseParanthesis || t1.type == ttCloseBracket )
	{
		if( withParenthesis )
		{
			if( t1.type == ttCloseParanthesis )
				node->UpdateSourcePos(t1.pos, t1.length);
			else
			{
				asCString str;
				str.Format(TXT_UNEXPECTED_TOKEN_s, asCTokenizer::GetDefinition(ttCloseBracket));

				Error(str.AddressOf(), &t1);
			}
		}
		else
			RewindTo(&t1);

		// Argument list has ended
		return node;
	}
	else
	{
		RewindTo(&t1);

		for(;;)
		{
			// Determine if this is a named argument
			sToken tl, t2;
			GetToken(&tl);
			GetToken(&t2);
			RewindTo(&tl);

			// Named arguments uses the syntax: arg : expr
			// This avoids confusion when the argument has the same name as a local variable, i.e. var = expr
			// It also avoids conflict with expressions to that creates anonymous objects initialized with lists, i.e. type = {...}
			// The alternate syntax: arg = expr, is supported to provide backwards compatibility with 2.29.0
			// TODO: 3.0.0: Remove the alternate syntax
			if( tl.type == ttIdentifier && (t2.type == ttColon || (engine->ep.alterSyntaxNamedArgs && t2.type == ttAssignment)) )
			{
				asCScriptNode *named = CreateNode(snNamedArgument);
				if( named == 0 ) return 0;
				node->AddChildLast(named);

				named->AddChildLast(ParseIdentifier());
				GetToken(&t2);

				if( engine->ep.alterSyntaxNamedArgs == 1 && t2.type == ttAssignment )
					Warning(TXT_NAMED_ARGS_WITH_OLD_SYNTAX, &t2);

				named->AddChildLast(ParseAssignment());
			}
			else
				node->AddChildLast(ParseAssignment());

			if( isSyntaxError ) return node;

			// Check if list continues
			GetToken(&t1);
			if( t1.type == ttListSeparator )
				continue;
			else
			{
				if( withParenthesis )
				{
					if( t1.type == ttCloseParanthesis )
						node->UpdateSourcePos(t1.pos, t1.length);
					else
					{
						Error(ExpectedTokens(")", ","), &t1);
						Error(InsteadFound(t1), &t1);
					}
				}
				else
					RewindTo(&t1);

				return node;
			}
		}
	}
}

bool asCParser::IsFunctionCall()
{
	sToken s;
	sToken t1, t2;

	GetToken(&s);
	t1 = s;

	// A function call may be prefixed with scope resolution
	if( t1.type == ttScope )
		GetToken(&t1);
	GetToken(&t2);

	while( t1.type == ttIdentifier && t2.type == ttScope )
	{
		GetToken(&t1);
		GetToken(&t2);
	}

	// A function call starts with an identifier followed by an argument list
	// The parser doesn't have enough information about scope to determine if the
	// identifier is a datatype, so even if it happens to be the parser will
	// identify the expression as a function call rather than a construct call.
	// The compiler will sort this out later
	if( t1.type != ttIdentifier )
	{
		RewindTo(&s);
		return false;
	}

	if( t2.type == ttOpenParanthesis )
	{
		RewindTo(&s);
		return true;
	}

	RewindTo(&s);
	return false;
}

// BNF:13: ASSIGN        ::= CONDITION [ ASSIGNOP ASSIGN ]
asCScriptNode *asCParser::ParseAssignment()
{
	asCScriptNode *node = CreateNode(snAssignment);
	if( node == 0 ) return 0;

	node->AddChildLast(ParseCondition());
	if( isSyntaxError ) return node;

	sToken t;
	GetToken(&t);
	RewindTo(&t);

	if( IsAssignOperator(t.type) )
	{
		node->AddChildLast(ParseAssignOperator());
		if( isSyntaxError ) return node;

		node->AddChildLast(ParseAssignment());
		if( isSyntaxError ) return node;
	}

	return node;
}

// BNF:14: CONDITION     ::= EXPR ['?' ASSIGN ':' ASSIGN]
asCScriptNode *asCParser::ParseCondition()
{
	asCScriptNode *node = CreateNode(snCondition);
	if( node == 0 ) return 0;

	node->AddChildLast(ParseExpression());
	if( isSyntaxError ) return node;

	sToken t;
	GetToken(&t);
	if( t.type == ttQuestion )
	{
		node->AddChildLast(ParseAssignment());
		if( isSyntaxError ) return node;

		GetToken(&t);
		if( t.type != ttColon )
		{
			Error(ExpectedToken(":"), &t);
			Error(InsteadFound(t), &t);
			return node;
		}

		node->AddChildLast(ParseAssignment());
		if( isSyntaxError ) return node;
	}
	else
		RewindTo(&t);

	return node;
}

// BNF:9: EXPR          ::= EXPRTERM {EXPROP EXPRTERM}
asCScriptNode *asCParser::ParseExpression()
{
	asCScriptNode *node = CreateNode(snExpression);
	if( node == 0 ) return 0;

	node->AddChildLast(ParseExprTerm());
	if( isSyntaxError ) return node;

	for(;;)
	{
		sToken t;
		GetToken(&t);
		RewindTo(&t);

		if( !IsOperator(t.type) )
			return node;

		node->AddChildLast(ParseExprOperator());
		if( isSyntaxError ) return node;

		node->AddChildLast(ParseExprTerm());
		if( isSyntaxError ) return node;
	}
	UNREACHABLE_RETURN;
}

// BNF:10: EXPRTERM      ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
asCScriptNode *asCParser::ParseExprTerm()
{
	asCScriptNode *node = CreateNode(snExprTerm);
	if( node == 0 ) return 0;

	// Check if the expression term is an initialization of a temp object with init list, i.e. type = {...}
	sToken t;
	GetToken(&t);
	sToken t2 = t, t3;
	if (IsDataType(t2) && CheckTemplateType(t2))
	{
		// The next token must be a = followed by a {
		GetToken(&t2);
		GetToken(&t3);
		if (t2.type == ttAssignment && t3.type == ttStartStatementBlock)
		{
			// It is an initialization, now parse it for real
			RewindTo(&t);
			node->AddChildLast(ParseType(false));
			GetToken(&t2);
			node->AddChildLast(ParseInitList());
			return node;
		}
	}
	// Or an anonymous init list, i.e. {...}
	else if( t.type == ttStartStatementBlock )
	{
		RewindTo(&t);
		node->AddChildLast(ParseInitList());
		return node;
	}

	// It wasn't an initialization, so it must be an ordinary expression term
	RewindTo(&t);

	for(;;)
	{
		GetToken(&t);
		RewindTo(&t);
		if( !IsPreOperator(t.type) )
			break;

		node->AddChildLast(ParseExprPreOp());
		if( isSyntaxError ) return node;
	}

	node->AddChildLast(ParseExprValue());
	if( isSyntaxError ) return node;


	for(;;)
	{
		GetToken(&t);
		RewindTo(&t);
		if( !IsPostOperator(t.type) )
			return node;

		node->AddChildLast(ParseExprPostOp());
		if( isSyntaxError ) return node;
	}
	UNREACHABLE_RETURN;
}

// BNF:11: EXPRPREOP     ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
asCScriptNode *asCParser::ParseExprPreOp()
{
	asCScriptNode *node = CreateNode(snExprPreOp);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( !IsPreOperator(t.type) )
	{
		Error(TXT_EXPECTED_PRE_OPERATOR, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:11: EXPRPOSTOP    ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':' ASSIGN} ']') | ARGLIST | '++' | '--'
asCScriptNode *asCParser::ParseExprPostOp()
{
	asCScriptNode *node = CreateNode(snExprPostOp);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( !IsPostOperator(t.type) )
	{
		Error(TXT_EXPECTED_POST_OPERATOR, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if( t.type == ttDot )
	{
		sToken t1, t2;
		GetToken(&t1);
		GetToken(&t2);
		RewindTo(&t1);
		if( t2.type == ttOpenParanthesis )
			node->AddChildLast(ParseFunctionCall());
		else
			node->AddChildLast(ParseIdentifier());
	}
	else if( t.type == ttOpenBracket )
	{
		node->AddChildLast(ParseArgList(false));

		GetToken(&t);
		if( t.type != ttCloseBracket )
		{
			Error(ExpectedToken("]"), &t);
			Error(InsteadFound(t), &t);
			return node;
		}

		node->UpdateSourcePos(t.pos, t.length);
	}
	else if( t.type == ttOpenParanthesis )
	{
		RewindTo(&t);
		node->AddChildLast(ParseArgList());
	}

	return node;
}

// BNF:15: EXPROP        ::= MATHOP | COMPOP | LOGICOP | BITOP
// BNF:16: MATHOP        ::= '+' | '-' | '*' | '/' | '%' | '**'
// BNF:16: COMPOP        ::= '==' | '!=' | '<' | '<=' | '>' | '>=' | 'is' | '!is'
// BNF:16: LOGICOP       ::= '&&' | '||' | '^^' | 'and' | 'or' | 'xor'
// BNF:16: BITOP         ::= '&' | '|' | '^' | '<<' | '>>' | '>>>'
asCScriptNode *asCParser::ParseExprOperator()
{
	asCScriptNode *node = CreateNode(snExprOperator);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( !IsOperator(t.type) )
	{
		Error(TXT_EXPECTED_OPERATOR, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:16: ASSIGNOP      ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
asCScriptNode *asCParser::ParseAssignOperator()
{
	asCScriptNode *node = CreateNode(snExprOperator);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( !IsAssignOperator(t.type) )
	{
		Error(TXT_EXPECTED_OPERATOR, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

bool asCParser::IsOperator(int tokenType)
{
	if( tokenType == ttPlus ||
		tokenType == ttMinus ||
		tokenType == ttStar ||
		tokenType == ttSlash ||
		tokenType == ttPercent ||
		tokenType == ttStarStar ||
		tokenType == ttAnd ||
		tokenType == ttOr ||
		tokenType == ttXor ||
		tokenType == ttEqual ||
		tokenType == ttNotEqual ||
		tokenType == ttLessThan ||
		tokenType == ttLessThanOrEqual ||
		tokenType == ttGreaterThan ||
		tokenType == ttGreaterThanOrEqual ||
		tokenType == ttAmp ||
		tokenType == ttBitOr ||
		tokenType == ttBitXor ||
		tokenType == ttBitShiftLeft ||
		tokenType == ttBitShiftRight ||
		tokenType == ttBitShiftRightArith ||
		tokenType == ttIs ||
		tokenType == ttNotIs )
		return true;

	return false;
}

bool asCParser::IsAssignOperator(int tokenType)
{
	if( tokenType == ttAssignment ||
		tokenType == ttAddAssign ||
		tokenType == ttSubAssign ||
		tokenType == ttMulAssign ||
		tokenType == ttDivAssign ||
		tokenType == ttModAssign ||
		tokenType == ttPowAssign ||
		tokenType == ttAndAssign ||
		tokenType == ttOrAssign ||
		tokenType == ttXorAssign ||
		tokenType == ttShiftLeftAssign ||
		tokenType == ttShiftRightLAssign ||
		tokenType == ttShiftRightAAssign )
		return true;

	return false;
}

bool asCParser::IsPreOperator(int tokenType)
{
	if( tokenType == ttMinus ||
		tokenType == ttPlus ||
		tokenType == ttNot ||
		tokenType == ttInc ||
		tokenType == ttDec ||
		tokenType == ttBitNot ||
		tokenType == ttHandle )
		return true;
	return false;
}

bool asCParser::IsPostOperator(int tokenType)
{
	if( tokenType == ttInc ||            // post increment
		tokenType == ttDec ||            // post decrement
		tokenType == ttDot ||            // member access
		tokenType == ttOpenBracket ||    // index operator
		tokenType == ttOpenParanthesis ) // argument list for call on function pointer
		return true;
	return false;
}

bool asCParser::IsConstant(int tokenType)
{
	if( tokenType == ttIntConstant ||
		tokenType == ttFloatConstant ||
		tokenType == ttDoubleConstant ||
		tokenType == ttStringConstant ||
		tokenType == ttMultilineStringConstant ||
		tokenType == ttHeredocStringConstant ||
		tokenType == ttTrue ||
		tokenType == ttFalse ||
		tokenType == ttBitsConstant ||
		tokenType == ttNull )
		return true;

	return false;
}

int asCParser::ParseScript(asCScriptCode *in_script)
{
	Reset();

	this->script = in_script;

	scriptNode = ParseScript(false);

	if( errorWhileParsing )
		return -1;

	// TODO: Should allow application to request this warning to be generated.
	//       It should be off by default, since pre-processor may remove all
	//       code from a section while still being meant as valid code
/*
	// Warn in case there isn't anything in the script
	if( scriptNode->firstChild == 0 )
	{
		if( builder )
			builder->WriteWarning(script->name, TXT_SECTION_IS_EMPTY, 1, 1);
	}
*/
	return 0;
}

int asCParser::ParseExpression(asCScriptCode *in_script)
{
	Reset();

	this->script = in_script;

	checkValidTypes = true;

	scriptNode = ParseExpression();
	if( errorWhileParsing )
		return -1;

	return 0;
}

// BNF:1: IMPORT        ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
asCScriptNode *asCParser::ParseImport()
{
	asCScriptNode *node = CreateNode(snImport);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttImport )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttImport)), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseFunctionDefinition());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttIdentifier )
	{
		Error(ExpectedToken(FROM_TOKEN), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	tempString.Assign(&script->code[t.pos], t.length);
	if( tempString != FROM_TOKEN )
	{
		Error(ExpectedToken(FROM_TOKEN), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttStringConstant )
	{
		Error(TXT_EXPECTED_STRING, &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	asCScriptNode *mod = CreateNode(snConstant);
	if( mod == 0 ) return 0;

	node->AddChildLast(mod);

	mod->SetToken(&t);
	mod->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttEndStatement)), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:0: SCRIPT        ::= {IMPORT | ENUM | TYPEDEF | CLASS | MIXIN | INTERFACE | FUNCDEF | VIRTPROP | VAR | FUNC | NAMESPACE | ';'}
asCScriptNode *asCParser::ParseScript(bool inBlock)
{
	asCScriptNode *node = CreateNode(snScript);
	if( node == 0 ) return 0;

	// Determine type of node
	for(;;)
	{
		while( !isSyntaxError )
		{
			sToken tStart;
			GetToken(&tStart);

			// Optimize by skipping tokens 'shared', 'external', 'final', 'abstract' so they don't have to be checked in every condition
			sToken t1 = tStart;
			while (IdentifierIs(t1, SHARED_TOKEN) ||
				IdentifierIs(t1, EXTERNAL_TOKEN) ||
				IdentifierIs(t1, FINAL_TOKEN) ||
				IdentifierIs(t1, ABSTRACT_TOKEN))
				GetToken(&t1);
			RewindTo(&tStart);

			if( t1.type == ttImport )
				node->AddChildLast(ParseImport());
			else if( t1.type == ttEnum )
				node->AddChildLast(ParseEnumeration());	// Handle enumerations
			else if( t1.type == ttTypedef )
				node->AddChildLast(ParseTypedef());		// Handle primitive typedefs
			else if( t1.type == ttClass )
				node->AddChildLast(ParseClass());
			else if( t1.type == ttMixin )
				node->AddChildLast(ParseMixin());
			else if( t1.type == ttInterface )
				node->AddChildLast(ParseInterface());
			else if( t1.type == ttFuncDef )
				node->AddChildLast(ParseFuncDef());
			else if( t1.type == ttConst || t1.type == ttScope || t1.type == ttAuto || IsDataType(t1) )
			{
				if( IsVirtualPropertyDecl() )
					node->AddChildLast(ParseVirtualPropertyDecl(false, false));
				else if( IsVarDecl() )
					node->AddChildLast(ParseDeclaration(false, true));
				else
					node->AddChildLast(ParseFunction());
			}
			else if( t1.type == ttEndStatement )
			{
				// Ignore a semicolon by itself
				GetToken(&t1);
			}
			else if( t1.type == ttNamespace )
				node->AddChildLast(ParseNamespace());
			else if( t1.type == ttEnd )
				return node;
			else if( inBlock && t1.type == ttEndStatementBlock )
				return node;
			else
			{
				asCString str;
				const char *t = asCTokenizer::GetDefinition(t1.type);
				if( t == 0 ) t = "<unknown token>";

				str.Format(TXT_UNEXPECTED_TOKEN_s, t);

				Error(str, &t1);
			}
		}

		if( isSyntaxError )
		{
			// Search for either ';' or '{' or end
			sToken t1;
			GetToken(&t1);
			while( t1.type != ttEndStatement && t1.type != ttEnd &&
				   t1.type != ttStartStatementBlock )
				GetToken(&t1);

			if( t1.type == ttStartStatementBlock )
			{
				// Find the end of the block and skip nested blocks
				int level = 1;
				while( level > 0 )
				{
					GetToken(&t1);
					if( t1.type == ttStartStatementBlock ) level++;
					if( t1.type == ttEndStatementBlock ) level--;
					if( t1.type == ttEnd ) break;
				}
			}

			isSyntaxError = false;
		}
	}
	UNREACHABLE_RETURN;
}

// BNF:1: NAMESPACE     ::= 'namespace' IDENTIFIER {'::' IDENTIFIER} '{' SCRIPT '}'
asCScriptNode *asCParser::ParseNamespace()
{
	asCScriptNode *node = CreateNode(snNamespace);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( t1.type == ttNamespace )
		node->UpdateSourcePos(t1.pos, t1.length);
	else
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttNamespace)), &t1);
		Error(InsteadFound(t1), &t1);
	}

	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	asCScriptNode *lowestNode = node;
	GetToken(&t1);
	while (t1.type == ttScope)
	{
		lowestNode->UpdateSourcePos(t1.pos, t1.length);

		asCScriptNode *scopeNode = CreateNode(snScript);
		if (scopeNode == 0) 
			return 0;
		lowestNode->AddChildLast(scopeNode);

		lowestNode = CreateNode(snNamespace);
		if (lowestNode == 0) 
			return 0;

		scopeNode->AddChildLast(lowestNode);
		lowestNode->AddChildLast(ParseIdentifier());
		if (isSyntaxError)
			return node;

		GetToken(&t1);
	}
	
	if( t1.type == ttStartStatementBlock )
		node->UpdateSourcePos(t1.pos, t1.length);
	else
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttStartStatementBlock)), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	sToken start = t1;

	lowestNode->AddChildLast(ParseScript(true));

	if( !isSyntaxError )
	{
		GetToken(&t1);
		if( t1.type == ttEndStatementBlock )
			node->UpdateSourcePos(t1.pos, t1.length);
		else
		{
			if( t1.type == ttEnd )
				Error(TXT_UNEXPECTED_END_OF_FILE, &t1);
			else
			{
				Error(ExpectedToken(asCTokenizer::GetDefinition(ttEndStatementBlock)), &t1);
				Error(InsteadFound(t1), &t1);
			}
			Info(TXT_WHILE_PARSING_NAMESPACE, &start);
			return node;
		}
	}

	return node;
}

int asCParser::ParseStatementBlock(asCScriptCode *in_script, asCScriptNode *in_block)
{
	TimeIt("asCParser::ParseStatementBlock");

	Reset();

	// Tell the parser to validate the identifiers as valid types
	checkValidTypes = true;

	this->script = in_script;
	sourcePos = in_block->tokenPos;

	scriptNode = ParseStatementBlock();

	if( isSyntaxError || errorWhileParsing )
		return -1;

	return 0;
}

// BNF:1: ENUM          ::= {'shared' | 'external'} 'enum' IDENTIFIER (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
asCScriptNode *asCParser::ParseEnumeration()
{
	asCScriptNode *ident;
	asCScriptNode *dataType;

	asCScriptNode *node = CreateNode(snEnum);
	if( node == 0 ) return 0;

	sToken token;

	// Optional 'shared' and 'external' token
	GetToken(&token);
	while( IdentifierIs(token, SHARED_TOKEN) ||
		   IdentifierIs(token, EXTERNAL_TOKEN) )
	{
		RewindTo(&token);
		node->AddChildLast(ParseIdentifier());
		if( isSyntaxError ) return node;

		GetToken(&token);
	}

	// Check for enum
	if( token.type != ttEnum )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttEnum)), &token);
		Error(InsteadFound(token), &token);
		return node;
	}

	node->SetToken(&token);
	node->UpdateSourcePos(token.pos, token.length);

	// Get the identifier
	GetToken(&token);
	if(ttIdentifier != token.type)
	{
		Error(TXT_EXPECTED_IDENTIFIER, &token);
		Error(InsteadFound(token), &token);
		return node;
	}

	dataType = CreateNode(snDataType);
	if( dataType == 0 ) return node;

	node->AddChildLast(dataType);

	ident = CreateNode(snIdentifier);
	if( ident == 0 ) return node;

	ident->SetToken(&token);
	ident->UpdateSourcePos(token.pos, token.length);
	dataType->AddChildLast(ident);

	// External shared declarations are ended with ';'
	GetToken(&token);
	if (token.type == ttEndStatement)
	{
		RewindTo(&token);
		node->AddChildLast(ParseToken(ttEndStatement));
		return node;
	}

	// check for the start of the declaration block
	if( token.type != ttStartStatementBlock )
	{
		RewindTo(&token);
		int tokens[] = { ttStartStatementBlock, ttEndStatement };
		Error(ExpectedOneOf(tokens, 2), &token);
		Error(InsteadFound(token), &token);
		return node;
	}

	while(ttEnd != token.type)
	{
		GetToken(&token);

		if( ttEndStatementBlock == token.type )
		{
			RewindTo(&token);
			break;
		}

		if(ttIdentifier != token.type)
		{
			Error(TXT_EXPECTED_IDENTIFIER, &token);
			Error(InsteadFound(token), &token);
			return node;
		}

		// Add the enum element
		ident = CreateNode(snIdentifier);
		if( ident == 0 ) return node;

		ident->SetToken(&token);
		ident->UpdateSourcePos(token.pos, token.length);
		node->AddChildLast(ident);

		GetToken(&token);

		if( token.type == ttAssignment )
		{
			asCScriptNode	*tmp;

			RewindTo(&token);

			tmp = SuperficiallyParseVarInit();

			node->AddChildLast(tmp);
			if( isSyntaxError ) return node;
			GetToken(&token);
		}

		if(ttListSeparator != token.type)
		{
			RewindTo(&token);
			break;
		}
	}

	// check for the end of the declaration block
	GetToken(&token);
	if( token.type != ttEndStatementBlock )
	{
		RewindTo(&token);
		Error(ExpectedToken("}"), &token);
		Error(InsteadFound(token), &token);
		return node;
	}

	return node;
}

bool asCParser::IsVarDecl()
{
	// Set start point so that we can rewind
	sToken t;
	GetToken(&t);
	RewindTo(&t);

	// A class property decl can be preceded by 'private' or 'protected'
	sToken t1;
	GetToken(&t1);
	if( t1.type != ttPrivate && t1.type != ttProtected )
		RewindTo(&t1);

	// A variable decl starts with the type
	if (!IsType(t1))
	{
		RewindTo(&t);
		return false;
	}

	// Jump to the token after the type
	RewindTo(&t1);
	GetToken(&t1);
	
	// The declaration needs to have a name
	if( t1.type != ttIdentifier )
	{
		RewindTo(&t);
		return false;
	}

	// It can be followed by an initialization
	GetToken(&t1);
	if( t1.type == ttEndStatement || t1.type == ttAssignment || t1.type == ttListSeparator )
	{
		RewindTo(&t);
		return true;
	}
	if( t1.type == ttOpenParanthesis )
	{
		// If the closing parenthesis is followed by a statement block, 
		// function decorator, or end-of-file, then treat it as a function. 
		// A function decl may have nested parenthesis so we need to check 
		// for this too.
		int nest = 0;
		while( t1.type != ttEnd )
		{
			if( t1.type == ttOpenParanthesis )
				nest++;
			else if( t1.type == ttCloseParanthesis )
			{
				nest--;
				if( nest == 0 )
					break;
			}
			GetToken(&t1);
		}

		if (t1.type == ttEnd)
		{
			RewindTo(&t);
			return false;
		}
		else
		{
			GetToken(&t1);
			RewindTo(&t);
			if( t1.type == ttStartStatementBlock || 
				t1.type == ttIdentifier || // function decorator
				t1.type == ttEnd )
				return false;
		}

		RewindTo(&t);
		return true;
	}

	RewindTo(&t);
	return false;
}

bool asCParser::IsVirtualPropertyDecl()
{
	// Set start point so that we can rewind
	sToken t;
	GetToken(&t);
	RewindTo(&t);

	// A class property decl can be preceded by 'private' or 'protected'
	sToken t1;
	GetToken(&t1);
	if( t1.type != ttPrivate && t1.type != ttProtected )
		RewindTo(&t1);

	// A variable decl starts with the type
	if (!IsType(t1))
	{
		RewindTo(&t);
		return false;
	}

	// Move to the token after the type
	RewindTo(&t1);
	GetToken(&t1);

	// The decl must have an identifier
	if( t1.type != ttIdentifier )
	{
		RewindTo(&t);
		return false;
	}

	// To be a virtual property it must also have a block for the get/set functions
	GetToken(&t1);
	if( t1.type == ttStartStatementBlock )
	{
		RewindTo(&t);
		return true;
	}

	RewindTo(&t);
	return false;
}

bool asCParser::IsFuncDecl(bool isMethod)
{
	// Set start point so that we can rewind
	sToken t;
	GetToken(&t);
	RewindTo(&t);

	if( isMethod )
	{
		// A class method decl can be preceded by 'private' or 'protected'
		sToken t1, t2;
		GetToken(&t1);
		if( t1.type != ttPrivate && t1.type != ttProtected )
			RewindTo(&t1);

		// A class constructor starts with identifier followed by parenthesis
		// A class destructor starts with the ~ token
		GetToken(&t1);
		GetToken(&t2);
		RewindTo(&t1);
		if( (t1.type == ttIdentifier && t2.type == ttOpenParanthesis) || t1.type == ttBitNot )
		{
			RewindTo(&t);
			return true;
		}
	}

	// A function decl starts with a type
	sToken t1;
	if (!IsType(t1))
	{
		RewindTo(&t);
		return false;
	}

	// Move to the token after the type
	RewindTo(&t1);
	GetToken(&t1);

	// There can be an ampersand if the function returns a reference
	if( t1.type == ttAmp )
	{
		RewindTo(&t);
		return true;
	}

	if( t1.type != ttIdentifier )
	{
		RewindTo(&t);
		return false;
	}

	GetToken(&t1);
	if( t1.type == ttOpenParanthesis )
	{
		// If the closing parenthesis is not followed by a
		// statement block then it is not a function.
		// It's possible that there are nested parenthesis due to default
		// arguments so this should be checked for.
		int nest = 0;
		GetToken(&t1);
		while( (nest || t1.type != ttCloseParanthesis) && t1.type != ttEnd )
		{
			if( t1.type == ttOpenParanthesis )
				nest++;
			if( t1.type == ttCloseParanthesis )
				nest--;

			GetToken(&t1);
		}

		if( t1.type == ttEnd )
			return false;
		else
		{
			if( isMethod )
			{
				// A class method can have a 'const' token after the parameter list
				GetToken(&t1);
				if( t1.type != ttConst )
					RewindTo(&t1);
			}

			// A function may also have any number of additional attributes
			for( ; ; )
			{
				GetToken(&t1);
				if( !IdentifierIs(t1, FINAL_TOKEN) && 
					!IdentifierIs(t1, OVERRIDE_TOKEN) &&
					!IdentifierIs(t1, EXPLICIT_TOKEN) &&
					!IdentifierIs(t1, PROPERTY_TOKEN) )
				{
					RewindTo(&t1);
					break;
				}
			}
			
			GetToken(&t1);
			RewindTo(&t);
			if( t1.type == ttStartStatementBlock )
				return true;
		}

		RewindTo(&t);
		return false;
	}

	RewindTo(&t);
	return false;
}

// BNF:1: FUNCDEF       ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
asCScriptNode *asCParser::ParseFuncDef()
{
	asCScriptNode *node = CreateNode(snFuncDef);
	if( node == 0 ) return 0;

	// Allow keywords 'external' and 'shared' before 'interface'
	sToken t1;
	GetToken(&t1);
	while (IdentifierIs(t1, SHARED_TOKEN) ||
		   IdentifierIs(t1, EXTERNAL_TOKEN))
	{
		RewindTo(&t1);
		node->AddChildLast(ParseIdentifier());
		if (isSyntaxError) return node;

		GetToken(&t1);
	}

	if( t1.type != ttFuncDef )
	{
		Error(asCTokenizer::GetDefinition(ttFuncDef), &t1);
		return node;
	}

	node->SetToken(&t1);

	node->AddChildLast(ParseType(true));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseTypeMod(false));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseParameterList());
	if( isSyntaxError ) return node;

	GetToken(&t1);
	if( t1.type != ttEndStatement )
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttEndStatement)), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:1: FUNC          ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER PARAMLIST ['const'] FUNCATTR (';' | STATBLOCK)
asCScriptNode *asCParser::ParseFunction(bool isMethod)
{
	asCScriptNode *node = CreateNode(snFunction);
	if( node == 0 ) return 0;

	sToken t1;
	GetToken(&t1);
	if (!isMethod)
	{
		// A global function can be marked as shared and external
		while (t1.type == ttIdentifier)
		{
			if (IdentifierIs(t1, SHARED_TOKEN) ||
				IdentifierIs(t1, EXTERNAL_TOKEN))
			{
				RewindTo(&t1);
				node->AddChildLast(ParseIdentifier());
				if (isSyntaxError) return node;
			}
			else
				break;

			GetToken(&t1);
		}
	}

	// A class method can start with 'private' or 'protected'
	if (isMethod && t1.type == ttPrivate)
	{
		RewindTo(&t1);
		node->AddChildLast(ParseToken(ttPrivate));
		GetToken(&t1);
	}
	else if (isMethod && t1.type == ttProtected)
	{
		RewindTo(&t1);
		node->AddChildLast(ParseToken(ttProtected));
		GetToken(&t1);
	}
	if( isSyntaxError ) return node;

	// If it is a global function, or a method, except constructor and destructor, then the return type is parsed
	sToken t2;
	GetToken(&t2);
	RewindTo(&t1);
	if( !isMethod || (t1.type != ttBitNot && t2.type != ttOpenParanthesis) )
	{
		node->AddChildLast(ParseType(true));
		if( isSyntaxError ) return node;

		node->AddChildLast(ParseTypeMod(false));
		if( isSyntaxError ) return node;
	}

	// If this is a class destructor then it starts with ~, and no return type is declared
	if( isMethod && t1.type == ttBitNot )
	{
		node->AddChildLast(ParseToken(ttBitNot));
		if( isSyntaxError ) return node;
	}

	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseParameterList());
	if( isSyntaxError ) return node;

	if( isMethod )
	{
		GetToken(&t1);
		RewindTo(&t1);

		// Is the method a const?
		if( t1.type == ttConst )
			node->AddChildLast(ParseToken(ttConst));
	}

	// TODO: Should support abstract methods, in which case no statement block should be provided
	ParseMethodAttributes(node);
	if( isSyntaxError ) return node;

	// External shared functions must be ended with ';'
	GetToken(&t1);
	RewindTo(&t1);
	if (t1.type == ttEndStatement)
	{
		node->AddChildLast(ParseToken(ttEndStatement));
		return node;
	}

	// We should just find the end of the statement block here. The statements
	// will be parsed on request by the compiler once it starts the compilation.
	node->AddChildLast(SuperficiallyParseStatementBlock());

	return node;
}

// BNF:2: INTFMTHD      ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] ';'
asCScriptNode *asCParser::ParseInterfaceMethod()
{
	asCScriptNode *node = CreateNode(snFunction);
	if( node == 0 ) return 0;

	node->AddChildLast(ParseType(true));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseTypeMod(false));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseParameterList());
	if( isSyntaxError ) return node;

	// Parse an optional const after the method definition
	sToken t1;
	GetToken(&t1);
	RewindTo(&t1);
	if( t1.type == ttConst )
		node->AddChildLast(ParseToken(ttConst));

	GetToken(&t1);
	if( t1.type != ttEndStatement )
	{
		Error(ExpectedToken(";"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:1: VIRTPROP      ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
asCScriptNode *asCParser::ParseVirtualPropertyDecl(bool isMethod, bool isInterface)
{
	asCScriptNode *node = CreateNode(snVirtualProperty);
	if( node == 0 ) return 0;

	sToken t1,t2;
	GetToken(&t1);
	GetToken(&t2);
	RewindTo(&t1);

	// A class method can start with 'private' or 'protected'
	if( isMethod && t1.type == ttPrivate )
		node->AddChildLast(ParseToken(ttPrivate));
	else if( isMethod && t1.type == ttProtected )
		node->AddChildLast(ParseToken(ttProtected));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseType(true));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseTypeMod(false));
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseIdentifier());
	if( isSyntaxError ) return node;

	GetToken(&t1);
	if( t1.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	for(;;)
	{
		GetToken(&t1);
		asCScriptNode *accessorNode = 0;

		if( IdentifierIs(t1, GET_TOKEN) || IdentifierIs(t1, SET_TOKEN) )
		{
			accessorNode = CreateNode(snVirtualProperty);
			if( accessorNode == 0 ) return 0;

			node->AddChildLast(accessorNode);

			RewindTo(&t1);
			accessorNode->AddChildLast(ParseIdentifier());

			if( isMethod )
			{
				GetToken(&t1);
				RewindTo(&t1);
				if( t1.type == ttConst )
					accessorNode->AddChildLast(ParseToken(ttConst));

				if( !isInterface )
				{
					ParseMethodAttributes(accessorNode);
					if( isSyntaxError ) return node;
				}
			}

			if( !isInterface )
			{
				GetToken(&t1);
				if( t1.type == ttStartStatementBlock )
				{
					RewindTo(&t1);
					accessorNode->AddChildLast(SuperficiallyParseStatementBlock());
					if( isSyntaxError ) return node;
				}
				else if( t1.type != ttEndStatement )
				{
					Error(ExpectedTokens(";", "{"), &t1);
					Error(InsteadFound(t1), &t1);
					return node;
				}
			}
			else
			{
				GetToken(&t1);
				if( t1.type != ttEndStatement )
				{
					Error(ExpectedToken(";"), &t1);
					Error(InsteadFound(t1), &t1);
					return node;
				}
			}
		}
		else if( t1.type == ttEndStatementBlock )
			break;
		else
		{
			const char *tokens[] = { GET_TOKEN, SET_TOKEN, asCTokenizer::GetDefinition(ttEndStatementBlock) };
			Error(ExpectedOneOf(tokens, 3), &t1);
			Error(InsteadFound(t1), &t1);
			return node;
		}
	}

	return node;
}

// BNF:1: INTERFACE     ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' IDENTIFIER {',' IDENTIFIER}] '{' {VIRTPROP | INTFMTHD} '}'))
asCScriptNode *asCParser::ParseInterface()
{
	asCScriptNode *node = CreateNode(snInterface);
	if( node == 0 ) return 0;

	sToken t;

	// Allow keywords 'external' and 'shared' before 'interface'
	GetToken(&t);
	while( IdentifierIs(t, SHARED_TOKEN) ||
		   IdentifierIs(t, EXTERNAL_TOKEN) )
	{
		RewindTo(&t);
		node->AddChildLast(ParseIdentifier());
		if (isSyntaxError) return node;

		GetToken(&t);
	}

	if( t.type != ttInterface )
	{
		Error(ExpectedToken("interface"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);
	node->AddChildLast(ParseIdentifier());

	// External shared declarations are ended with ';'
	GetToken(&t);
	if (t.type == ttEndStatement)
	{
		RewindTo(&t);
		node->AddChildLast(ParseToken(ttEndStatement));
		return node;
	}

	// Can optionally have a list of interfaces that are inherited
	if( t.type == ttColon )
	{
		asCScriptNode *inherit = CreateNode(snIdentifier);
		node->AddChildLast(inherit);

		ParseOptionalScope(inherit);
		inherit->AddChildLast(ParseIdentifier());
		GetToken(&t);
		while( t.type == ttListSeparator )
		{
			inherit = CreateNode(snIdentifier);
			node->AddChildLast(inherit);

			ParseOptionalScope(inherit);
			inherit->AddChildLast(ParseIdentifier());
			GetToken(&t);
		}
	}

	if( t.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	// Parse interface methods
	GetToken(&t);
	RewindTo(&t);
	while( t.type != ttEndStatementBlock && t.type != ttEnd )
	{
		if( IsVirtualPropertyDecl() )
			node->AddChildLast(ParseVirtualPropertyDecl(true, true));
		else if( t.type == ttEndStatement )
			// Skip empty declarations
			GetToken(&t);
		else
			// Parse the method signature
			node->AddChildLast(ParseInterfaceMethod());

		if( isSyntaxError ) return node;

		GetToken(&t);
		RewindTo(&t);
	}

	GetToken(&t);
	if( t.type != ttEndStatementBlock )
	{
		Error(ExpectedToken("}"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:1: MIXIN         ::= 'mixin' CLASS
asCScriptNode *asCParser::ParseMixin()
{
	asCScriptNode *node = CreateNode(snMixin);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);

	if( t.type != ttMixin )
	{
		Error(ExpectedToken("mixin"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);

	// A mixin token must be followed by a class declaration
	node->AddChildLast(ParseClass());

	return node;
}

// BNF:1: CLASS         ::= {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' IDENTIFIER {',' IDENTIFIER}] '{' {VIRTPROP | FUNC | VAR | FUNCDEF} '}'))
asCScriptNode *asCParser::ParseClass()
{
	asCScriptNode *node = CreateNode(snClass);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);

	// Allow the keywords 'shared', 'abstract', 'final', and 'external' before 'class'
	while( IdentifierIs(t, SHARED_TOKEN) ||
		   IdentifierIs(t, ABSTRACT_TOKEN) ||
		   IdentifierIs(t, FINAL_TOKEN) ||
		   IdentifierIs(t, EXTERNAL_TOKEN) )
	{
		RewindTo(&t);
		node->AddChildLast(ParseIdentifier());
		GetToken(&t);
	}

	if( t.type != ttClass )
	{
		Error(ExpectedToken("class"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->SetToken(&t);

	if( engine->ep.allowImplicitHandleTypes )
	{
		// Parse 'implicit handle class' construct
		GetToken(&t);

		if ( t.type == ttHandle )
			node->SetToken(&t);
		else
			RewindTo(&t);
	}

	node->AddChildLast(ParseIdentifier());

	// External shared declarations are ended with ';'
	GetToken(&t);
	if (t.type == ttEndStatement)
	{
		RewindTo(&t);
		node->AddChildLast(ParseToken(ttEndStatement));
		return node;
	}

	// Optional list of interfaces that are being implemented and classes that are being inherited
	if( t.type == ttColon )
	{
		asCScriptNode *inherit = CreateNode(snIdentifier);
		node->AddChildLast(inherit);

		ParseOptionalScope(inherit);
		inherit->AddChildLast(ParseIdentifier());
		GetToken(&t);
		while( t.type == ttListSeparator )
		{
			inherit = CreateNode(snIdentifier);
			node->AddChildLast(inherit);

			ParseOptionalScope(inherit);
			inherit->AddChildLast(ParseIdentifier());
			GetToken(&t);
		}
	}

	if( t.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	// Parse properties
	GetToken(&t);
	RewindTo(&t);
	while( t.type != ttEndStatementBlock && t.type != ttEnd )
	{
		// Is it a property or a method?
		if (t.type == ttFuncDef)
			node->AddChildLast(ParseFuncDef());
		else if( IsFuncDecl(true) )
			node->AddChildLast(ParseFunction(true));
		else if( IsVirtualPropertyDecl() )
			node->AddChildLast(ParseVirtualPropertyDecl(true, false));
		else if( IsVarDecl() )
			node->AddChildLast(ParseDeclaration(true));
		else if( t.type == ttEndStatement )
			// Skip empty declarations
			GetToken(&t);
		else
		{
			Error(TXT_EXPECTED_METHOD_OR_PROPERTY, &t);
			Error(InsteadFound(t), &t);
			return node;
		}

		if( isSyntaxError )
			return node;

		GetToken(&t);
		RewindTo(&t);
	}

	GetToken(&t);
	if( t.type != ttEndStatementBlock )
	{
		Error(ExpectedToken("}"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

int asCParser::ParseVarInit(asCScriptCode *in_script, asCScriptNode *in_init)
{
	Reset();

	// Tell the parser to validate the identifiers as valid types
	checkValidTypes = true;

	this->script = in_script;
	sourcePos = in_init->tokenPos;

	// If next token is assignment, parse expression
	sToken t;
	GetToken(&t);
	if( t.type == ttAssignment )
	{
		GetToken(&t);
		RewindTo(&t);
		if( t.type == ttStartStatementBlock )
			scriptNode = ParseInitList();
		else
			scriptNode = ParseAssignment();
	}
	else if( t.type == ttOpenParanthesis )
	{
		RewindTo(&t);
		scriptNode = ParseArgList();
	}
	else
	{
		int tokens[] = {ttAssignment, ttOpenParanthesis};
		Error(ExpectedOneOf(tokens, 2), &t);
		Error(InsteadFound(t), &t);
	}

	// Don't allow any more tokens after the expression
	GetToken(&t);
	if( t.type != ttEnd && t.type != ttEndStatement && t.type != ttListSeparator && t.type != ttEndStatementBlock )
	{
		asCString msg;
		msg.Format(TXT_UNEXPECTED_TOKEN_s, asCTokenizer::GetDefinition(t.type));
		Error(msg, &t);
	}

	if( isSyntaxError || errorWhileParsing )
		return -1;

	return 0;
}

asCScriptNode *asCParser::SuperficiallyParseVarInit()
{
	asCScriptNode *node = CreateNode(snAssignment);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if( t.type == ttAssignment )
	{
		GetToken(&t);
		sToken start = t;

		// Find the end of the expression
		int indentParan = 0;
		int indentBrace = 0;
		while( indentParan || indentBrace || (t.type != ttListSeparator && t.type != ttEndStatement && t.type != ttEndStatementBlock) )
		{
			if( t.type == ttOpenParanthesis )
				indentParan++;
			else if( t.type == ttCloseParanthesis )
				indentParan--;
			else if( t.type == ttStartStatementBlock )
				indentBrace++;
			else if( t.type == ttEndStatementBlock )
				indentBrace--;
			else if( t.type == ttNonTerminatedStringConstant )
			{
				Error(TXT_NONTERMINATED_STRING, &t);
				break;
			}
			else if( t.type == ttEnd )
			{
				Error(TXT_UNEXPECTED_END_OF_FILE, &t);
				Info(TXT_WHILE_PARSING_EXPRESSION, &start);
				break;
			}
			GetToken(&t);
		}

		// Rewind so that the next token read is the list separator, end statement, or end statement block
		RewindTo(&t);
	}
	else if( t.type == ttOpenParanthesis )
	{
		sToken start = t;

		// Find the end of the argument list
		int indent = 1;
		while( indent )
		{
			GetToken(&t);
			if( t.type == ttOpenParanthesis )
				indent++;
			else if( t.type == ttCloseParanthesis )
				indent--;
			else if( t.type == ttNonTerminatedStringConstant )
			{
				Error(TXT_NONTERMINATED_STRING, &t);
				break;
			}
			else if( t.type == ttEnd )
			{
				Error(TXT_UNEXPECTED_END_OF_FILE, &t);
				Info(TXT_WHILE_PARSING_ARG_LIST, &start);
				break;
			}
		}
	}
	else
	{
		int tokens[] = {ttAssignment, ttOpenParanthesis};
		Error(ExpectedOneOf(tokens, 2), &t);
		Error(InsteadFound(t), &t);
	}

	return node;
}

asCScriptNode *asCParser::SuperficiallyParseStatementBlock()
{
	asCScriptNode *node = CreateNode(snStatementBlock);
	if( node == 0 ) return 0;

	// This function will only superficially parse the statement block in order to find the end of it
	sToken t1;

	GetToken(&t1);
	if( t1.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	sToken start = t1;

	int level = 1;
	while( level > 0 && !isSyntaxError )
	{
		GetToken(&t1);
		if( t1.type == ttEndStatementBlock )
			level--;
		else if( t1.type == ttStartStatementBlock )
			level++;
		else if( t1.type == ttNonTerminatedStringConstant )
		{
			Error(TXT_NONTERMINATED_STRING, &t1);
			break;
		}
		else if( t1.type == ttEnd )
		{
			Error(TXT_UNEXPECTED_END_OF_FILE, &t1);
			Info(TXT_WHILE_PARSING_STATEMENT_BLOCK, &start);
			break;
		}
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	return node;
}

// BNF:2: STATBLOCK     ::= '{' {VAR | STATEMENT} '}'
asCScriptNode *asCParser::ParseStatementBlock()
{
	asCScriptNode *node = CreateNode(snStatementBlock);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( t1.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	sToken start = t1;

	node->UpdateSourcePos(t1.pos, t1.length);

	for(;;)
	{
		while( !isSyntaxError )
		{
			GetToken(&t1);
			if( t1.type == ttEndStatementBlock )
			{
				node->UpdateSourcePos(t1.pos, t1.length);

				// Statement block is finished
				return node;
			}
			else
			{
				RewindTo(&t1);

				if( IsVarDecl() )
					node->AddChildLast(ParseDeclaration());
				else
					node->AddChildLast(ParseStatement());
			}
		}

		if( isSyntaxError )
		{
			// Search for either ';', '{', '}', or end
			GetToken(&t1);
			while( t1.type != ttEndStatement && t1.type != ttEnd &&
				   t1.type != ttStartStatementBlock && t1.type != ttEndStatementBlock )
			{
				GetToken(&t1);
			}

			// Skip this statement block
			if( t1.type == ttStartStatementBlock )
			{
				// Find the end of the block and skip nested blocks
				int level = 1;
				while( level > 0 )
				{
					GetToken(&t1);
					if( t1.type == ttStartStatementBlock ) level++;
					if( t1.type == ttEndStatementBlock ) level--;
					if( t1.type == ttEnd ) break;
				}
			}
			else if( t1.type == ttEndStatementBlock )
			{
				RewindTo(&t1);
			}
			else if( t1.type == ttEnd )
			{
				Error(TXT_UNEXPECTED_END_OF_FILE, &t1);
				Info(TXT_WHILE_PARSING_STATEMENT_BLOCK, &start);
				return node;
			}

			isSyntaxError = false;
		}
	}
	UNREACHABLE_RETURN;
}

// BNF:4: INITLIST      ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
asCScriptNode *asCParser::ParseInitList()
{
	asCScriptNode *node = CreateNode(snInitList);
	if( node == 0 ) return 0;

	sToken t1;

	GetToken(&t1);
	if( t1.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t1);
		Error(InsteadFound(t1), &t1);
		return node;
	}

	node->UpdateSourcePos(t1.pos, t1.length);

	GetToken(&t1);
	if( t1.type == ttEndStatementBlock )
	{
		node->UpdateSourcePos(t1.pos, t1.length);

		// Statement block is finished
		return node;
	}
	else
	{
		RewindTo(&t1);
		for(;;)
		{
			GetToken(&t1);
			if( t1.type == ttListSeparator )
			{
				// No expression
				node->AddChildLast(CreateNode(snUndefined));
				node->lastChild->UpdateSourcePos(t1.pos, 1);

				GetToken(&t1);
				if( t1.type == ttEndStatementBlock )
				{
					// No expression
					node->AddChildLast(CreateNode(snUndefined));
					node->lastChild->UpdateSourcePos(t1.pos, 1);
					node->UpdateSourcePos(t1.pos, t1.length);
					return node;
				}
				RewindTo(&t1);
			}
			else if( t1.type == ttEndStatementBlock )
			{
				// No expression
				node->AddChildLast(CreateNode(snUndefined));
				node->lastChild->UpdateSourcePos(t1.pos, 1);
				node->UpdateSourcePos(t1.pos, t1.length);

				// Statement block is finished
				return node;
			}
			else if( t1.type == ttStartStatementBlock )
			{
				RewindTo(&t1);
				node->AddChildLast(ParseInitList());
				if( isSyntaxError ) return node;

				GetToken(&t1);
				if( t1.type == ttListSeparator )
					continue;
				else if( t1.type == ttEndStatementBlock )
				{
					node->UpdateSourcePos(t1.pos, t1.length);

					// Statement block is finished
					return node;
				}
				else
				{
					Error(ExpectedTokens("}", ","), &t1);
					Error(InsteadFound(t1), &t1);
					return node;
				}
			}
			else
			{
				RewindTo(&t1);
				node->AddChildLast(ParseAssignment());
				if( isSyntaxError ) return node;


				GetToken(&t1);
				if( t1.type == ttListSeparator )
					continue;
				else if( t1.type == ttEndStatementBlock )
				{
					node->UpdateSourcePos(t1.pos, t1.length);

					// Statement block is finished
					return node;
				}
				else
				{
					Error(ExpectedTokens("}", ","), &t1);
					Error(InsteadFound(t1), &t1);
					return node;
				}
			}
		}
	}
	UNREACHABLE_RETURN;
}

// BNF:1: VAR           ::= ['private'|'protected'] TYPE IDENTIFIER [( '=' (INITLIST | EXPR)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | EXPR)) | ARGLIST]} ';'
asCScriptNode *asCParser::ParseDeclaration(bool isClassProp, bool isGlobalVar)
{
	asCScriptNode *node = CreateNode(snDeclaration);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	RewindTo(&t);

	// A class property can be preceeded by private
	if( t.type == ttPrivate && isClassProp )
		node->AddChildLast(ParseToken(ttPrivate));
	else if( t.type == ttProtected && isClassProp )
		node->AddChildLast(ParseToken(ttProtected));

	// Parse data type
	node->AddChildLast(ParseType(true, false, !isClassProp));
	if( isSyntaxError ) return node;

	for(;;)
	{
		// Parse identifier
		node->AddChildLast(ParseIdentifier());
		if( isSyntaxError ) return node;

		if( isClassProp || isGlobalVar )
		{
			// Only superficially parse the initialization info for the class property
			GetToken(&t);
			RewindTo(&t);
			if( t.type == ttAssignment || t.type == ttOpenParanthesis )
			{
				node->AddChildLast(SuperficiallyParseVarInit());
				if( isSyntaxError ) return node;
			}
		}
		else
		{
			// If next token is assignment, parse expression
			GetToken(&t);
			if( t.type == ttOpenParanthesis )
			{
				RewindTo(&t);
				node->AddChildLast(ParseArgList());
				if( isSyntaxError ) return node;
			}
			else if( t.type == ttAssignment )
			{
				GetToken(&t);
				RewindTo(&t);
				if( t.type == ttStartStatementBlock )
				{
					node->AddChildLast(ParseInitList());
					if( isSyntaxError ) return node;
				}
				else
				{
					node->AddChildLast(ParseAssignment());
					if( isSyntaxError ) return node;
				}
			}
			else
				RewindTo(&t);
		}

		// continue if list separator, else terminate with end statement
		GetToken(&t);
		if( t.type == ttListSeparator )
			continue;
		else if( t.type == ttEndStatement )
		{
			node->UpdateSourcePos(t.pos, t.length);

			return node;
		}
		else
		{
			Error(ExpectedTokens(",", ";"), &t);
			Error(InsteadFound(t), &t);
			return node;
		}
	}
	UNREACHABLE_RETURN;
}

// BNF:7: STATEMENT     ::= (IF | FOR | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
asCScriptNode *asCParser::ParseStatement()
{
	sToken t1;

	GetToken(&t1);
	RewindTo(&t1);

	if (t1.type == ttIf)
		return ParseIf();
	else if (t1.type == ttFor)
		return ParseFor();
	else if (t1.type == ttWhile)
		return ParseWhile();
	else if (t1.type == ttReturn)
		return ParseReturn();
	else if (t1.type == ttStartStatementBlock)
		return ParseStatementBlock();
	else if (t1.type == ttBreak)
		return ParseBreak();
	else if (t1.type == ttContinue)
		return ParseContinue();
	else if (t1.type == ttDo)
		return ParseDoWhile();
	else if (t1.type == ttSwitch)
		return ParseSwitch();
	else if (t1.type == ttTry)
		return ParseTryCatch();
	else
	{
		if( IsVarDecl() )
		{
			Error(TXT_UNEXPECTED_VAR_DECL, &t1);
			return 0;
		}
		return ParseExpressionStatement();
	}
}

// BNF:8: EXPRSTAT      ::= [ASSIGN] ';'
asCScriptNode *asCParser::ParseExpressionStatement()
{
	asCScriptNode *node = CreateNode(snExpressionStatement);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type == ttEndStatement )
	{
		node->UpdateSourcePos(t.pos, t.length);

		return node;
	}

	RewindTo(&t);

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(";"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:8: SWITCH        ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
asCScriptNode *asCParser::ParseSwitch()
{
	asCScriptNode *node = CreateNode(snSwitch);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttSwitch )
	{
		Error(ExpectedToken("switch"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttCloseParanthesis )
	{
		Error(ExpectedToken(")"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	GetToken(&t);
	if( t.type != ttStartStatementBlock )
	{
		Error(ExpectedToken("{"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	while( !isSyntaxError )
	{
		GetToken(&t);

		if( t.type == ttEndStatementBlock )
			break;

		RewindTo(&t);

		if( t.type != ttCase && t.type != ttDefault )
		{
			const char *tokens[] = {"case", "default"};
			Error(ExpectedOneOf(tokens, 2), &t);
			Error(InsteadFound(t), &t);
			return node;
		}

		node->AddChildLast(ParseCase());
		if( isSyntaxError ) return node;
	}

	if( t.type != ttEndStatementBlock )
	{
		Error(ExpectedToken("}"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	return node;
}

// BNF:9: CASE          ::= (('case' EXPR) | 'default') ':' {STATEMENT}
asCScriptNode *asCParser::ParseCase()
{
	asCScriptNode *node = CreateNode(snCase);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttCase && t.type != ttDefault )
	{
		Error(ExpectedTokens("case", "default"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	if(t.type == ttCase)
	{
		node->AddChildLast(ParseExpression());
	}

	GetToken(&t);
	if( t.type != ttColon )
	{
		Error(ExpectedToken(":"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	// Parse statements until we find either of }, case, default, and break
	GetToken(&t);
	RewindTo(&t);
	while( t.type != ttCase &&
		   t.type != ttDefault &&
		   t.type != ttEndStatementBlock &&
		   t.type != ttBreak )
	{
		if( IsVarDecl() )
			// Variable declarations are not allowed, but we parse it anyway to give a good error message
			node->AddChildLast(ParseDeclaration());
		else
			node->AddChildLast(ParseStatement());
		if( isSyntaxError ) return node;

		GetToken(&t);
		RewindTo(&t);
	}

	// If the case was ended with a break statement, add it to the node
	if( t.type == ttBreak )
		node->AddChildLast(ParseBreak());

	return node;
}

// BNF:8: IF            ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
asCScriptNode *asCParser::ParseIf()
{
	asCScriptNode *node = CreateNode(snIf);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttIf )
	{
		Error(ExpectedToken("if"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttCloseParanthesis )
	{
		Error(ExpectedToken(")"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseStatement());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttElse )
	{
		// No else statement return already
		RewindTo(&t);
		return node;
	}

	node->AddChildLast(ParseStatement());

	return node;
}

// BNF:8: TRY           ::= 'try' STATBLOCK 'catch' STATBLOCK
asCScriptNode *asCParser::ParseTryCatch()
{
	asCScriptNode *node = CreateNode(snTryCatch);
	if (node == 0) return 0;

	sToken t;
	GetToken(&t);
	if (t.type != ttTry)
	{
		Error(ExpectedToken("try"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseStatementBlock());
	if (isSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttCatch)
	{
		Error(ExpectedToken("catch"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseStatementBlock());
	if (isSyntaxError) return node;

	return node;
}

// BNF:8: FOR           ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
asCScriptNode *asCParser::ParseFor()
{
	asCScriptNode *node = CreateNode(snFor);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttFor )
	{
		Error(ExpectedToken("for"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	if( IsVarDecl() )
		node->AddChildLast(ParseDeclaration());
	else
		node->AddChildLast(ParseExpressionStatement());
	if( isSyntaxError ) return node;

	node->AddChildLast(ParseExpressionStatement());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttCloseParanthesis )
	{
		RewindTo(&t);

		// Parse N increment statements separated by ,
		for(;;)
		{
			asCScriptNode *n = CreateNode(snExpressionStatement);
			if( n == 0 ) return 0;
			node->AddChildLast(n);
			n->AddChildLast(ParseAssignment());
			if( isSyntaxError ) return node;

			GetToken(&t);
			if( t.type == ttListSeparator )
				continue;
			else if( t.type == ttCloseParanthesis )
				break;
			else
			{
				const char *tokens[] = {",", ")"};
				Error(ExpectedOneOf(tokens, 2), &t);
				Error(InsteadFound(t), &t);
				return node;
			}
		}
	}

	node->AddChildLast(ParseStatement());

	return node;
}

// BNF:8: WHILE         ::= 'while' '(' ASSIGN ')' STATEMENT
asCScriptNode *asCParser::ParseWhile()
{
	asCScriptNode *node = CreateNode(snWhile);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttWhile )
	{
		Error(ExpectedToken("while"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttCloseParanthesis )
	{
		Error(ExpectedToken(")"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseStatement());

	return node;
}

// BNF:8: DOWHILE       ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
asCScriptNode *asCParser::ParseDoWhile()
{
	asCScriptNode *node = CreateNode(snDoWhile);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttDo )
	{
		Error(ExpectedToken("do"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseStatement());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttWhile )
	{
		Error(ExpectedToken("while"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	GetToken(&t);
	if( t.type != ttOpenParanthesis )
	{
		Error(ExpectedToken("("), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttCloseParanthesis )
	{
		Error(ExpectedToken(")"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(";"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:8: RETURN        ::= 'return' [ASSIGN] ';'
asCScriptNode *asCParser::ParseReturn()
{
	asCScriptNode *node = CreateNode(snReturn);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttReturn )
	{
		Error(ExpectedToken("return"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type == ttEndStatement )
	{
		node->UpdateSourcePos(t.pos, t.length);
		return node;
	}

	RewindTo(&t);

	node->AddChildLast(ParseAssignment());
	if( isSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(";"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:8: BREAK         ::= 'break' ';'
asCScriptNode *asCParser::ParseBreak()
{
	asCScriptNode *node = CreateNode(snBreak);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttBreak )
	{
		Error(ExpectedToken("break"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(";"), &t);
		Error(InsteadFound(t), &t);
	}

	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// BNF:8: CONTINUE      ::= 'continue' ';'
asCScriptNode *asCParser::ParseContinue()
{
	asCScriptNode *node = CreateNode(snContinue);
	if( node == 0 ) return 0;

	sToken t;
	GetToken(&t);
	if( t.type != ttContinue )
	{
		Error(ExpectedToken("continue"), &t);
		Error(InsteadFound(t), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(";"), &t);
		Error(InsteadFound(t), &t);
	}

	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

// TODO: typedef: Typedefs should accept complex types as well
// BNF:1: TYPEDEF       ::= 'typedef' PRIMTYPE IDENTIFIER ';'
asCScriptNode *asCParser::ParseTypedef()
{
	// Create the typedef node
	asCScriptNode *node = CreateNode(snTypedef);
	if( node == 0 ) return 0;

	sToken	token;

	GetToken(&token);
	if( token.type != ttTypedef)
	{
		Error(ExpectedToken(asCTokenizer::GetDefinition(ttTypedef)), &token);
		Error(InsteadFound(token), &token);
		return node;
	}

	node->SetToken(&token);
	node->UpdateSourcePos(token.pos, token.length);

	// Parse the base type
	GetToken(&token);
	RewindTo(&token);

	// Make sure it is a primitive type (except ttVoid)
	if( !IsRealType(token.type) || token.type == ttVoid )
	{
		asCString str;
		str.Format(TXT_UNEXPECTED_TOKEN_s, asCTokenizer::GetDefinition(token.type));
		Error(str, &token);
		return node;
	}

	node->AddChildLast(ParseRealType());
	node->AddChildLast(ParseIdentifier());

	// Check for the end of the typedef
	GetToken(&token);
	if( token.type != ttEndStatement )
	{
		RewindTo(&token);
		Error(ExpectedToken(asCTokenizer::GetDefinition(token.type)), &token);
		Error(InsteadFound(token), &token);
	}

	return node;
}

#endif

END_AS_NAMESPACE


