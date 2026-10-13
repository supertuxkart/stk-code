/*
   AngelCode Scripting Library
   Copyright (c) 2003-2024 Andreas Jonsson

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
// as_restore.h
//
// Functions for saving and restoring module bytecode
// asCRestore was originally written by Dennis Bollyn, dennis@gyrbo.be
// It was later split in two classes asCReader and asCWriter by me

#ifndef AS_RESTORE_H
#define AS_RESTORE_H

#include "as_scriptengine.h"
#include "as_context.h"
#include "as_map.h"

BEGIN_AS_NAMESPACE

class asCReader
{
public:
	asCReader(asCModule *module, asIBinaryStream *stream, asCScriptEngine *engine);

	int Read(bool *wasDebugInfoStripped);

protected:
	asCModule       *module;
	asIBinaryStream *stream;
	asCScriptEngine *engine;
	bool             noDebugInfo;
	bool             error;
	asUINT           bytesRead;

	int                Error(const char *msg);

	int                ReadInner();

	int                ReadData(void *data, asUINT size);
	void               ReadString(asCString *str);
	asCScriptFunction *ReadFunction(bool &isNew, bool addToModule = true, bool addToEngine = true, bool addToGC = true, bool *isExternal = 0);
	void               ReadFunctionSignature(asCScriptFunction *func, asCObjectType **parentClass = 0);
	void               ReadGlobalProperty();
	void               ReadObjectProperty(asCObjectType *ot);
	void               ReadDataType(asCDataType *dt);
	asCTypeInfo       *ReadTypeInfo();
	void               ReadTypeDeclaration(asCTypeInfo *ot, int phase, bool *isExternal = 0);
	void               ReadByteCode(asCScriptFunction *func);
	asWORD             ReadEncodedUInt16();
	asUINT             ReadEncodedUInt();
	int                ReadEncodedInt();
	asQWORD            ReadEncodedUInt64();
	asUINT             SanityCheck(asUINT val, asUINT max);
	int                SanityCheck(int val, asUINT max);

	void ReadUsedTypeIds();
	void ReadUsedFunctions();
	void ReadUsedGlobalProps();
	void ReadUsedStringConstants();
	void ReadUsedObjectProps();

	asCTypeInfo *      FindType(int idx);
	int                FindTypeId(int idx);
	short              FindObjectPropOffset(asWORD index);
	asCScriptFunction *FindFunction(int idx);

	// After loading, each function needs to be translated to update pointers, function ids, etc
	void TranslateFunction(asCScriptFunction *func);
	void CalculateAdjustmentByPos(asCScriptFunction *func);
	int  AdjustStackPosition(int pos);
	int  AdjustGetOffset(int offset, asCScriptFunction *func, asDWORD programPos);
	void CalculateStackNeeded(asCScriptFunction *func);

	// Temporary storage for persisting variable data
	asCArray<int>                usedTypeIds;
	asCArray<asCTypeInfo*>       usedTypes;
	asCArray<asCScriptFunction*> usedFunctions;
	asCArray<void*>              usedGlobalProperties;
	asCArray<void*>              usedStringConstants;

	asCArray<asCScriptFunction*>  savedFunctions;
	asCArray<asCDataType>         savedDataTypes;
	asCArray<asCString>           savedStrings;

	asCArray<int>                 adjustByPos;
	asCArray<int>                 adjustNegativeStackByPos;

	struct SObjProp
	{
		asCObjectType     *objType;
		asCObjectProperty *prop;
	};
	asCArray<SObjProp> usedObjectProperties;

	asCMap<void*,bool>              existingShared;
	asCMap<asCScriptFunction*,bool> dontTranslate;

	// Helper class for adjusting offsets within initialization list buffers
	struct SListAdjuster
	{
		SListAdjuster(asCReader *rd, asDWORD *bc, asCObjectType *ot);
		void AdjustAllocMem();
		int  AdjustOffset(int offset);
		void SetRepeatCount(asUINT rc);
		void SetNextType(int typeId);

		struct SInfo
		{
			asUINT              repeatCount;
			asSListPatternNode *startNode;
		};
		asCArray<SInfo> stack;

		asCReader          *reader;
		asDWORD            *allocMemBC;
		asUINT              maxOffset;
		asCObjectType      *patternType;
		asUINT              repeatCount;
		int                 lastOffset;
		int                 nextOffset;
		asUINT              lastAdjustedOffset;
		asSListPatternNode *patternNode;
		int                 nextTypeId;
	};
	asCArray<SListAdjuster*> listAdjusters;

	// Used by FindObjectPropOffset
	asCObjectProperty* lastCompositeProp;
};

#ifndef AS_NO_COMPILER

class asCWriter
{
public:
	asCWriter(asCModule *module, asIBinaryStream *stream, asCScriptEngine *engine, bool stripDebugInfo);

	int Write();

protected:
	asCModule       *module;
	asIBinaryStream *stream;
	asCScriptEngine *engine;
	bool             stripDebugInfo;
	bool             error;
	asUINT           bytesWritten;

	int              Error(const char *msg);

	int  WriteData(const void *data, asUINT size);

	void WriteString(asCString *str);
	void WriteFunction(asCScriptFunction *func);
	void WriteFunctionSignature(asCScriptFunction *func);
	void WriteGlobalProperty(asCGlobalProperty *prop);
	void WriteObjectProperty(asCObjectProperty *prop);
	void WriteDataType(const asCDataType *dt);
	void WriteTypeInfo(asCTypeInfo *ot);
	void WriteTypeDeclaration(asCTypeInfo *ot, int phase);
	void WriteByteCode(asCScriptFunction *func);
	void WriteEncodedInt64(asINT64 i);

	// Helper functions for storing variable data
	int FindTypeInfoIdx(asCTypeInfo *ti);
	int FindTypeIdIdx(int typeId);
	int FindFunctionIndex(asCScriptFunction *func);
	int FindGlobalPropPtrIndex(void *);
	int FindStringConstantIndex(void *str);
	int FindObjectPropIndex(short offset, int typeId, asDWORD *bc);

	void CalculateAdjustmentByPos(asCScriptFunction *func);
	int  AdjustStackPosition(int pos);
	int  AdjustProgramPosition(int pos);
	int  AdjustGetOffset(int offset, asCScriptFunction *func, asDWORD programPos);

	// Intermediate data used for storing that which isn't constant, function id's, pointers, etc
	void WriteUsedTypeIds();
	void WriteUsedFunctions();
	void WriteUsedGlobalProps();
	void WriteUsedStringConstants();
	void WriteUsedObjectProps();

	// Temporary storage for persisting variable data
	asCArray<int>                usedTypeIds;
	asCArray<asCTypeInfo*>       usedTypes;
	asCArray<asCScriptFunction*> usedFunctions;
	asCArray<void*>              usedGlobalProperties;
	asCArray<void*>              usedStringConstants;
	asCMap<void*, int>           stringToIndexMap;

	asCArray<asCScriptFunction*>  savedFunctions;
	asCArray<asCDataType>         savedDataTypes;
	asCArray<asCString>           savedStrings;
	asCMap<asCString, int>        stringToIdMap;
	asCArray<int>                 adjustStackByPos;
	asCArray<int>                 adjustNegativeStackByPos;
	asCArray<int>                 bytecodeNbrByPos;

	struct SObjProp
	{
		asCObjectType     *objType;
		asCObjectProperty *prop;
	};
	asCArray<SObjProp>           usedObjectProperties;

	// Helper class for adjusting offsets within initialization list buffers
	struct SListAdjuster
	{
		SListAdjuster(asCObjectType *ot);
		int  AdjustOffset(int offset, asCObjectType *listPatternType);
		void SetRepeatCount(asUINT rc);
		void SetNextType(int typeId);

		struct SInfo
		{
			asUINT              repeatCount;
			asSListPatternNode *startNode;
		};
		asCArray<SInfo> stack;

		asCObjectType      *patternType;
		asUINT              repeatCount;
		asSListPatternNode *patternNode;
		asUINT              entries;
		int                 lastOffset;  // Last offset adjusted
		int                 nextOffset;  // next expected offset to be adjusted
		int                 nextTypeId;
	};
	asCArray<SListAdjuster*> listAdjusters;

	// Used by FindObjectPropIndex
	bool lastWasComposite;
};

#endif

END_AS_NAMESPACE

#endif // AS_RESTORE_H
