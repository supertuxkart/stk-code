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
// as_configgroup.cpp
//
// This class holds configuration groups for the engine
//



#include "as_config.h"
#include "as_configgroup.h"
#include "as_scriptengine.h"
#include "as_texts.h"

BEGIN_AS_NAMESPACE

asCConfigGroup::asCConfigGroup()
{
	refCount = 0;
}

asCConfigGroup::~asCConfigGroup()
{
}

int asCConfigGroup::AddRef()
{
	refCount++;
	return refCount;
}

int asCConfigGroup::Release()
{
	// Don't delete the object here, the engine will delete the object when ready
	refCount--;
	return refCount;
}

asCObjectType *asCConfigGroup::FindType(const char *obj)
{
	for( asUINT n = 0; n < objTypes.GetLength(); n++ )
		if( objTypes[n]->name == obj )
			return objTypes[n];

	return 0;
}

void asCConfigGroup::RefConfigGroup(asCConfigGroup *group)
{
	if( group == this || group == 0 ) return;

	// Verify if the group is already referenced
	for( asUINT n = 0; n < referencedConfigGroups.GetLength(); n++ )
		if( referencedConfigGroups[n] == group )
			return;

	referencedConfigGroups.PushLast(group);
	group->AddRef();
}

bool asCConfigGroup::HasLiveObjects()
{
	for( asUINT n = 0; n < objTypes.GetLength(); n++ )
		if( objTypes[n]->GetRefCount() != 0 )
			return true;

	return false;
}

void asCConfigGroup::RemoveConfiguration(asCScriptEngine *engine, bool notUsed)
{
	asASSERT( refCount == 0 );

	asUINT n;

	// Remove global variables
	for( n = 0; n < globalProps.GetLength(); n++ )
	{
		int index = engine->registeredGlobalProps.GetIndex(globalProps[n]);
		if( index >= 0 )
		{
			globalProps[n]->Release();

			// TODO: global: Should compact the registeredGlobalProps array
			engine->registeredGlobalProps.Erase(index);
		}
	}
	globalProps.SetLength(0);

	// Remove global functions
	for( n = 0; n < scriptFunctions.GetLength(); n++ )
	{
		int index = engine->registeredGlobalFuncs.GetIndex(scriptFunctions[n]);
		if( index >= 0 )
			engine->registeredGlobalFuncs.Erase(index);
		scriptFunctions[n]->Release();
		if( engine->stringFactory == scriptFunctions[n] )
			engine->stringFactory = 0;
	}
	scriptFunctions.SetLength(0);

	// Remove behaviours and members of object types
	for( n = 0; n < objTypes.GetLength(); n++ )
	{
		asCObjectType *obj = objTypes[n];

		obj->ReleaseAllFunctions();
	}

	// Remove function definitions
	for( n = 0; n < funcDefs.GetLength(); n++ )
	{
		engine->registeredFuncDefs.RemoveValue(funcDefs[n]);
		funcDefs[n]->Release();
	}
	funcDefs.SetLength(0);

	engine->ClearUnusedTypes();

	// Remove object types (skip this if it is possible other groups are still using the types)
	if( !notUsed )
	{
		for( n = 0; n < objTypes.GetLength(); n++ )
		{
			asCObjectType *t = objTypes[n];
			asSMapNode<asSNameSpaceNamePair, asCObjectType*> *cursor;
			if( engine->allRegisteredTypes.MoveTo(&cursor, asSNameSpaceNamePair(t->nameSpace, t->name)) &&
				cursor->value == t )
			{
#ifdef AS_DEBUG
				ValidateNoUsage(engine, t);
#endif

				engine->allRegisteredTypes.Erase(cursor);
				if( engine->defaultArrayObjectType == t )
					engine->defaultArrayObjectType = 0;

				if( t->flags & asOBJ_TYPEDEF )
					engine->registeredTypeDefs.RemoveValue(t);
				else if( t->flags & asOBJ_ENUM )
					engine->registeredEnums.RemoveValue(t);
				else if( t->flags & asOBJ_TEMPLATE )
					engine->registeredTemplateTypes.RemoveValue(t);
				else
					engine->registeredObjTypes.RemoveValue(t);

				asDELETE(t, asCObjectType);
			}
			else
			{
				int idx = engine->templateInstanceTypes.IndexOf(t);
				if( idx >= 0 )
				{
#ifdef AS_DEBUG
					ValidateNoUsage(engine, t);
#endif

					engine->templateInstanceTypes.RemoveIndexUnordered(idx);
					t->templateSubTypes.SetLength(0);

					asDELETE(t, asCObjectType);
				}
			}
		}
		objTypes.SetLength(0);
	}

	// Release other config groups
	for( n = 0; n < referencedConfigGroups.GetLength(); n++ )
		referencedConfigGroups[n]->refCount--;
	referencedConfigGroups.SetLength(0);
}

#ifdef AS_DEBUG
void asCConfigGroup::ValidateNoUsage(asCScriptEngine *engine, asCObjectType *type)
{
	for( asUINT n = 0; n < engine->scriptFunctions.GetLength(); n++ )
	{
		asCScriptFunction *func = engine->scriptFunctions[n];
		if( func == 0 ) continue;

		// Ignore factory, list factory, and members
		if( func->name == "_beh_3_" || func->name == "_beh_4_" || func->objectType == type )
			continue;

		// Ignore function definitions too, as they aren't released until the engine is destroyed
		if( func->funcType == asFUNC_FUNCDEF )
			continue;

		// Ignore functions whose object type has already reached refCount 0 as they are to be removed
		if( func->objectType && func->objectType->GetRefCount() == 0 )
			continue;

		if( func->returnType.GetObjectType() == type )
		{
			asCString msg;
			// We can only use the function name here, because the types used by the function may have been deleted already
			msg.Format(TXT_TYPE_s_IS_STILL_USED_BY_FUNC_s, type->name.AddressOf(), func->GetDeclaration());
			engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
		}
		else
		{
			for( asUINT p = 0; p < func->parameterTypes.GetLength(); p++ )
			{
				if( func->parameterTypes[p].GetObjectType() == type )
				{
					asCString msg;
					// We can only use the function name here, because the types used by the function may have been deleted already
					msg.Format(TXT_TYPE_s_IS_STILL_USED_BY_FUNC_s, type->name.AddressOf(), func->GetDeclaration());
					engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());
					break;
				}
			}
		}	
	}

	// TODO: Check also usage of the type in global variables 

	// TODO: Check also usage of the type in local variables in script functions

	// TODO: Check also usage of the type as members of classes

	// TODO: Check also usage of the type as sub types in other types
}
#endif

END_AS_NAMESPACE
