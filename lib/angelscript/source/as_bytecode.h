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
// as_bytecode.h
//
// A class for constructing the final byte code
//



#ifndef AS_BYTECODE_H
#define AS_BYTECODE_H

#include "as_config.h"

#ifndef AS_NO_COMPILER

#include "as_array.h"

BEGIN_AS_NAMESPACE

#define BYTECODE_SIZE  4
#define MAX_DATA_SIZE  8
#define MAX_INSTR_SIZE (BYTECODE_SIZE+MAX_DATA_SIZE)

class asCScriptEngine;
class asCScriptFunction;
class asCByteInstruction;

class asCByteCode
{
public:
	asCByteCode(asCScriptEngine *engine);
	~asCByteCode();

	void ClearAll();

	int GetSize();

	void Finalize(const asCArray<int> &tempVariableOffsets);

	void Optimize();
	void OptimizeLocally(const asCArray<int> &tempVariableOffsets);
	void ExtractLineNumbers();
	void ExtractObjectVariableInfo(asCScriptFunction *outFunc);
	int  ResolveJumpAddresses();
	int  FindLabel(int label, asCByteInstruction *from, asCByteInstruction **dest, int *positionDelta);

	void AddPath(asCArray<asCByteInstruction *> &paths, asCByteInstruction *instr, int stackSize);

	void Output(asDWORD *array);
	void AddCode(asCByteCode *bc);

	void PostProcess();

#ifdef AS_DEBUG
	void DebugOutput(const char *name, asCScriptEngine *engine, asCScriptFunction *func);
#endif

	int  GetLastInstr();
	int  RemoveLastInstr();
	asDWORD GetLastInstrValueDW();

	void InsertIfNotExists(asCArray<int> &vars, int var);
	void GetVarsUsed(asCArray<int> &vars);
	bool IsVarUsed(int offset);
	void ExchangeVar(int oldOffset, int newOffset);
	bool IsSimpleExpression();

	void Label(short label);
	void Line(int line, int column, int scriptIdx);
	void ObjInfo(int offset, int info);
	void Block(bool start);
	void VarDecl(int varDeclIdx);
	void Call(asEBCInstr bc, int funcID, int pop);
	void CallPtr(asEBCInstr bc, int funcPtrVar, int pop);
	void Alloc(asEBCInstr bc, void *objID, int funcID, int pop);
	void Ret(int pop);
	void JmpP(int var, asDWORD max);

	int InsertFirstInstrDWORD(asEBCInstr bc, asDWORD param);
	int InsertFirstInstrQWORD(asEBCInstr bc, asQWORD param);
	int Instr(asEBCInstr bc);
	int InstrQWORD(asEBCInstr bc, asQWORD param);
	int InstrDOUBLE(asEBCInstr bc, double param);
	int InstrPTR(asEBCInstr bc, void *param);
	int InstrDWORD(asEBCInstr bc, asDWORD param);
	int InstrWORD(asEBCInstr bc, asWORD param);
	int InstrSHORT(asEBCInstr bc, short param);
	int InstrFLOAT(asEBCInstr bc, float param);
	int InstrINT(asEBCInstr bc, int param);
	int InstrW_W_W(asEBCInstr bc, int a, int b, int c);
	int InstrSHORT_B(asEBCInstr bc, short a, asBYTE b);
	int InstrSHORT_W(asEBCInstr bc, short a, asWORD b);
	int InstrSHORT_DW(asEBCInstr bc, short a, asDWORD b);
	int InstrSHORT_QW(asEBCInstr bc, short a, asQWORD b);
	int InstrW_DW(asEBCInstr bc, asWORD a, asDWORD b);
	int InstrW_QW(asEBCInstr bc, asWORD a, asQWORD b);
	int InstrW_PTR(asEBCInstr bc, short a, void *param);
	int InstrW_FLOAT(asEBCInstr bc, asWORD a, float b);
	int InstrW_W(asEBCInstr bc, int w, int b);
	int InstrSHORT_DW_DW(asEBCInstr bc, short a, asDWORD b, asDWORD c);

	asCScriptEngine *GetEngine() const { return engine; };

	asCArray<int> lineNumbers;
	asCArray<int> sectionIdxs;
	int largestStackUsed;

protected:
	// Assignments are not allowed
	void operator=(const asCByteCode &) {}

	// Helpers for Optimize
	bool CanBeSwapped(asCByteInstruction *curr);
	asCByteInstruction *ChangeFirstDeleteNext(asCByteInstruction *curr, asEBCInstr bc);
	asCByteInstruction *DeleteFirstChangeNext(asCByteInstruction *curr, asEBCInstr bc);
	asCByteInstruction *DeleteInstruction(asCByteInstruction *instr);
	void RemoveInstruction(asCByteInstruction *instr);
	asCByteInstruction *GoBack(asCByteInstruction *curr);
	asCByteInstruction *GoForward(asCByteInstruction *curr);
	void InsertBefore(asCByteInstruction *before, asCByteInstruction *instr);
	bool RemoveUnusedValue(asCByteInstruction *curr, asCByteInstruction **next);
	bool IsTemporary(int offset);
	bool IsTempRegUsed(asCByteInstruction *curr);
	bool IsTempVarRead(asCByteInstruction *curr, int offset);
	bool PostponeInitOfTemp(asCByteInstruction *curr, asCByteInstruction **next);
	bool IsTempVarReadByInstr(asCByteInstruction *curr, int var);
	bool IsTempVarOverwrittenByInstr(asCByteInstruction *curr, int var);
	bool IsInstrJmpOrLabel(asCByteInstruction *curr);

	int AddInstruction();
	int AddInstructionFirst();

	asCByteInstruction *first;
	asCByteInstruction *last;

	const asCArray<int> *temporaryVariables;

	asCScriptEngine *engine;
};

class asCByteInstruction
{
public:
	asCByteInstruction();

	void AddAfter(asCByteInstruction *nextCode);
	void AddBefore(asCByteInstruction *nextCode);
	void Remove();

	int  GetSize();
	int  GetStackIncrease();

	asCByteInstruction *next;
	asCByteInstruction *prev;

	asEBCInstr op;
	asQWORD arg;
	short wArg[3];
	int size;
	int stackInc;

	// Testing
	bool marked;
	int  stackSize;
};

END_AS_NAMESPACE

#endif // AS_NO_COMPILER

#endif
