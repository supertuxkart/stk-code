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
// as_gc.cpp
//
// The implementation of the garbage collector
//


#include <stdlib.h>

#include "as_gc.h"
#include "as_scriptengine.h"
#include "as_scriptobject.h"
#include "as_texts.h"

BEGIN_AS_NAMESPACE

asCGarbageCollector::asCGarbageCollector()
{
	engine          = 0;
	detectState     = clearCounters_init;
	destroyNewState = destroyGarbage_init;
	destroyOldState = destroyGarbage_init;
	numDestroyed    = 0;
	numNewDestroyed = 0;
	numDetected     = 0;
	numAdded        = 0;
	isProcessing    = false;

	seqAtSweepStart[0] = 0;
	seqAtSweepStart[1] = 0;
	seqAtSweepStart[2] = 0;
}

asCGarbageCollector::~asCGarbageCollector()
{
	// This local typedef is done to workaround a compiler error on
	// MSVC6 when using the typedef declared in the class definition
	typedef asSMapNode_t node_t;
	for( asUINT n = 0; n < freeNodes.GetLength(); n++ )
		asDELETE(freeNodes[n], node_t);
	freeNodes.SetLength(0);
}

int asCGarbageCollector::AddScriptObjectToGC(void *obj, asCObjectType *objType)
{
	if( obj == 0 || objType == 0 )
	{
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, TXT_GC_RECEIVED_NULL_PTR);
		return asINVALID_ARG;
	}

	engine->CallObjectMethod(obj, objType->beh.addref);
	asSObjTypePair ot = {obj, objType, 0};

	// Invoke the garbage collector to destroy a little garbage as new comes in
	// This will maintain the number of objects in the GC at a maintainable level without
	// halting the application, and without burdening the application with manually invoking the 
	// garbage collector.
	if( engine->ep.autoGarbageCollect && gcNewObjects.GetLength() )
	{
		// If the GC is already processing in another thread, then don't try this again
		if( TRYENTERCRITICALSECTION(gcCollecting) )
		{
			// Skip this if the GC is already running in this thread
			if( !isProcessing )
			{
				isProcessing = true;

				// TODO: The number of iterations should be dynamic, and increase 
				//       if the number of objects in the garbage collector grows high

				// Run one step of DetectGarbage
				if( gcOldObjects.GetLength() )
				{
					IdentifyGarbageWithCyclicRefs();
					DestroyOldGarbage();
				}

				// Run a few steps of DestroyGarbage
				int iter = (int)gcNewObjects.GetLength();
				if( iter > 10 ) iter = 10;
				while( iter-- > 0 )
					DestroyNewGarbage();

				isProcessing = false;
			}

			LEAVECRITICALSECTION(gcCollecting);
		}
	}

	// Add the data to the gcObjects array in a critical section as
	// another thread might be calling this method at the same time
	ENTERCRITICALSECTION(gcCritical);
	ot.seqNbr = numAdded++;
	gcNewObjects.PushLast(ot);
	LEAVECRITICALSECTION(gcCritical);

	return ot.seqNbr;
}

int asCGarbageCollector::GetObjectInGC(asUINT idx, asUINT *seqNbr, void **obj, asIObjectType **type)
{
	if( seqNbr ) *seqNbr = 0;
	if( obj )    *obj    = 0;
	if( type )   *type   = 0;

	ENTERCRITICALSECTION(gcCritical);
	asSObjTypePair *o = 0;
	asUINT newObjs = asUINT(gcNewObjects.GetLength());
	if( idx < newObjs )
		o = &gcNewObjects[idx];
	else if( idx < gcOldObjects.GetLength() + newObjs )
		o = &gcOldObjects[idx-newObjs];
	else
	{
		LEAVECRITICALSECTION(gcCritical);
		return asINVALID_ARG;
	}
	if( seqNbr ) *seqNbr = o->seqNbr;
	if( obj )    *obj    = o->obj;
	if( type )   *type   = o->type;
	LEAVECRITICALSECTION(gcCritical);

	return asSUCCESS;
}

int asCGarbageCollector::GarbageCollect(asDWORD flags)
{
	// If the GC is already processing in another thread, then don't enter here again
	if( TRYENTERCRITICALSECTION(gcCollecting) )
	{
		// If the GC is already processing in this thread, then don't enter here again
		if( isProcessing ) 
		{	
			LEAVECRITICALSECTION(gcCollecting);
			return 1;
		}

		isProcessing = true;

		bool doDetect  = (flags & asGC_DETECT_GARBAGE)  || !(flags & asGC_DESTROY_GARBAGE);
		bool doDestroy = (flags & asGC_DESTROY_GARBAGE) || !(flags & asGC_DETECT_GARBAGE);

		if( flags & asGC_FULL_CYCLE )
		{
			// Reset the state
			if( doDetect )
			{
				// Move all objects to the old list, so we guarantee that all is detected
				for( asUINT n = (asUINT)gcNewObjects.GetLength(); n-- > 0; )
					MoveObjectToOldList(n);
				detectState  = clearCounters_init;
			}
			if( doDestroy )
			{
				destroyNewState = destroyGarbage_init;
				destroyOldState = destroyGarbage_init;
			}

			unsigned int count = (unsigned int)(gcNewObjects.GetLength() + gcOldObjects.GetLength());
			for(;;)
			{
				// Detect all garbage with cyclic references
				if( doDetect )
					while( IdentifyGarbageWithCyclicRefs() == 1 ) {}

				// Now destroy all known garbage
				if( doDestroy )
				{
					while( DestroyNewGarbage() == 1 ) {}
					while( DestroyOldGarbage() == 1 ) {}
				}

				// Run another iteration if any garbage was destroyed
				if( count != (unsigned int)(gcNewObjects.GetLength() + gcOldObjects.GetLength()) )
					count = (unsigned int)(gcNewObjects.GetLength() + gcOldObjects.GetLength());
				else
				{
					// Let the engine destroy the types that reached refCount 0
					// If none were destroyed, then leave the GC
					// TODO: The asCObjectType should destroy its content when refCount reaches 0
					//       since no-one is using them. The registered types should have their
					//       refcount increased by the config groups. Doing it like that will allow 
					//       me to remove this call to ClearUnusedTypes() that the GC really 
					//       shouldn't be calling.
					if( engine->ClearUnusedTypes() == 0 )
						break;
				}
			}

			isProcessing = false;
			LEAVECRITICALSECTION(gcCollecting);
			return 0;
		}
		else
		{
			// Destroy the garbage that we know of
			if( doDestroy )
			{
				DestroyNewGarbage();
				DestroyOldGarbage();
			}

			// Run another incremental step of the identification of cyclic references
			if( doDetect )
				IdentifyGarbageWithCyclicRefs();
		}

		isProcessing = false;
		LEAVECRITICALSECTION(gcCollecting);
	}
	
	// Return 1 to indicate that the cycle wasn't finished
	return 1;
}

void asCGarbageCollector::GetStatistics(asUINT *currentSize, asUINT *totalDestroyed, asUINT *totalDetected, asUINT *newObjects, asUINT *totalNewDestroyed) const
{
	// It's not necessary to protect this access, as
	// it doesn't matter if another thread is currently
	// appending a new object.
	if( currentSize )
		*currentSize = (asUINT)(gcNewObjects.GetLength() + gcOldObjects.GetLength());

	if( totalDestroyed )
		*totalDestroyed = numDestroyed;

	asASSERT( numAdded == gcNewObjects.GetLength() + gcOldObjects.GetLength() + numDestroyed );

	if( totalDetected )
		*totalDetected = numDetected;

	if( newObjects )
		*newObjects = (asUINT)gcNewObjects.GetLength();

	if( totalNewDestroyed )
		*totalNewDestroyed = numNewDestroyed;
}

asCGarbageCollector::asSObjTypePair asCGarbageCollector::GetNewObjectAtIdx(int idx)
{
	// We need to protect this access with a critical section as
	// another thread might be appending an object at the same time
	ENTERCRITICALSECTION(gcCritical);
	asSObjTypePair gcObj = gcNewObjects[idx];
	LEAVECRITICALSECTION(gcCritical);

	return gcObj;
}

asCGarbageCollector::asSObjTypePair asCGarbageCollector::GetOldObjectAtIdx(int idx)
{
	// We need to protect this access with a critical section as
	// another thread might be appending an object at the same time
	ENTERCRITICALSECTION(gcCritical);
	asSObjTypePair gcObj = gcOldObjects[idx];
	LEAVECRITICALSECTION(gcCritical);

	return gcObj;
}

void asCGarbageCollector::RemoveNewObjectAtIdx(int idx)
{
	// We need to protect this update with a critical section as
	// another thread might be appending an object at the same time
	ENTERCRITICALSECTION(gcCritical);
	if( idx == (int)gcNewObjects.GetLength() - 1)
		gcNewObjects.PopLast();
	else
		gcNewObjects[idx] = gcNewObjects.PopLast();
	LEAVECRITICALSECTION(gcCritical);
}

void asCGarbageCollector::RemoveOldObjectAtIdx(int idx)
{
	// We need to protect this update with a critical section as
	// another thread might be appending an object at the same time
	ENTERCRITICALSECTION(gcCritical);
	if( idx == (int)gcOldObjects.GetLength() - 1)
		gcOldObjects.PopLast();
	else
		gcOldObjects[idx] = gcOldObjects.PopLast();
	LEAVECRITICALSECTION(gcCritical);
}

void asCGarbageCollector::MoveObjectToOldList(int idx)
{
	// We need to protect this update with a critical section as
	// another thread might be appending an object at the same time
	ENTERCRITICALSECTION(gcCritical);
	gcOldObjects.PushLast(gcNewObjects[idx]);
	if( idx == (int)gcNewObjects.GetLength() - 1)
		gcNewObjects.PopLast();
	else
		gcNewObjects[idx] = gcNewObjects.PopLast();
	LEAVECRITICALSECTION(gcCritical);
}

int asCGarbageCollector::DestroyNewGarbage()
{
	// This function will only be called within the critical section gcCollecting
	asASSERT(isProcessing);

	for(;;)
	{
		switch( destroyNewState )
		{
		case destroyGarbage_init:
		{
			// If there are no objects to be freed then don't start
			if( gcNewObjects.GetLength() == 0 )
				return 0;

			// Update the seqAtSweepStart which is used to determine when 
			// to move an object from the new set to the old set
			seqAtSweepStart[0] = seqAtSweepStart[1];
			seqAtSweepStart[1] = seqAtSweepStart[2];
			seqAtSweepStart[2] = numAdded;

			destroyNewIdx = (asUINT)-1;
			destroyNewState = destroyGarbage_loop;
		}
		break;

		case destroyGarbage_loop:
		case destroyGarbage_haveMore:
		{
			// If the refCount has reached 1, then only the GC still holds a
			// reference to the object, thus we don't need to worry about the
			// application touching the objects during collection.

			// Destroy all objects that have refCount == 1. If any objects are
			// destroyed, go over the list again, because it may have made more
			// objects reach refCount == 1.
			if( ++destroyNewIdx < gcNewObjects.GetLength() )
			{
				asSObjTypePair gcObj = GetNewObjectAtIdx(destroyNewIdx);
				if( engine->CallObjectMethodRetInt(gcObj.obj, gcObj.type->beh.gcGetRefCount) == 1 )
				{
					// Release the object immediately

					// Make sure the refCount is really 0, because the
					// destructor may have increased the refCount again.
					bool addRef = false;
					if( gcObj.type->flags & asOBJ_SCRIPT_OBJECT )
					{
						// Script objects may actually be resurrected in the destructor
						int refCount = ((asCScriptObject*)gcObj.obj)->Release();
						if( refCount > 0 ) addRef = true;
					}
					else
						engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.release);

					// Was the object really destroyed?
					if( !addRef )
					{
						numDestroyed++;
						numNewDestroyed++;
						RemoveNewObjectAtIdx(destroyNewIdx);
						destroyNewIdx--;
					}
					else
					{
						// Since the object was resurrected in the
						// destructor, we must add our reference again
						engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.addref);
					}

					destroyNewState = destroyGarbage_haveMore;
				}
				// Check if this object has been inspected 3 times already, and if so move it to the 
				// set of old objects that are less likely to become garbage in a short time
				// TODO: Is 3 really a good value? Should the number of times be dynamic? 
				else if( gcObj.seqNbr < seqAtSweepStart[0] )
				{
					// We've already verified this object multiple times. It is likely
					// to live for quite a long time so we'll move it to the list if old objects
					MoveObjectToOldList(destroyNewIdx);
					destroyNewIdx--;
				}

				// Allow the application to work a little
				return 1;
			}
			else
			{
				if( destroyNewState == destroyGarbage_haveMore )
				{
					// Restart the cycle
					destroyNewState = destroyGarbage_init;
				}
				else
				{
					// Restart the cycle
					destroyNewState = destroyGarbage_init;

					// Return 0 to tell the application that there 
					// is no more garbage to destroy at the moment
					return 0;
				}
			}
		}
		break;
		}
	}

	// Shouldn't reach this point
	UNREACHABLE_RETURN;
}

int asCGarbageCollector::ReportAndReleaseUndestroyedObjects()
{
	// This function will only be called as the engine is shutting down

	int items = 0;
	for( asUINT n = 0; n < gcOldObjects.GetLength(); n++ )
	{
		asSObjTypePair gcObj = GetOldObjectAtIdx(n);

		int refCount = 0;
		if( gcObj.type->beh.gcGetRefCount && engine->scriptFunctions[gcObj.type->beh.gcGetRefCount] )
			refCount = engine->CallObjectMethodRetInt(gcObj.obj, gcObj.type->beh.gcGetRefCount);

		// Report the object as not being properly destroyed
		asCString msg;
		msg.Format(TXT_d_GC_CANNOT_FREE_OBJ_OF_TYPE_s_REF_COUNT_d, gcObj.seqNbr, gcObj.type->name.AddressOf(), refCount - 1);
		engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());

		// Add additional info for builtin types
		if( gcObj.type->name == "_builtin_function_" )
		{
			// Unfortunately we can't show the function declaration here, because the engine may have released the parameter list already so the declaration would only be misleading
			// We need to show the function type too as for example delegates do not have a name
			msg.Format(TXT_PREV_FUNC_IS_NAMED_s_TYPE_IS_d, reinterpret_cast<asCScriptFunction*>(gcObj.obj)->GetName(), reinterpret_cast<asCScriptFunction*>(gcObj.obj)->GetFuncType());
			engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
		}
		else if( gcObj.type->name == "_builtin_objecttype_" )
		{
			msg.Format(TXT_PREV_TYPE_IS_NAMED_s, reinterpret_cast<asCObjectType*>(gcObj.obj)->GetName());
			engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
		}
		else if( gcObj.type->name == "_builtin_globalprop_" )
		{
			msg.Format(TXT_PREV_TYPE_IS_NAMED_s, reinterpret_cast<asCGlobalProperty*>(gcObj.obj)->name.AddressOf());
			engine->WriteMessage("", 0, 0, asMSGTYPE_INFORMATION, msg.AddressOf());
		}

		// Release the reference that the GC holds if the release functions is still available
		if( gcObj.type->beh.release && engine->scriptFunctions[gcObj.type->beh.release] )
			engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.release);

		items++;
	}
	return items;
}

int asCGarbageCollector::DestroyOldGarbage()
{
	// This function will only be called within the critical section gcCollecting
	asASSERT(isProcessing);

	for(;;)
	{
		switch( destroyOldState )
		{
		case destroyGarbage_init:
		{
			// If there are no objects to be freed then don't start
			if( gcOldObjects.GetLength() == 0 )
				return 0;

			destroyOldIdx = (asUINT)-1;
			destroyOldState = destroyGarbage_loop;
		}
		break;

		case destroyGarbage_loop:
		case destroyGarbage_haveMore:
		{
			// If the refCount has reached 1, then only the GC still holds a
			// reference to the object, thus we don't need to worry about the
			// application touching the objects during collection.

			// Destroy all objects that have refCount == 1. If any objects are
			// destroyed, go over the list again, because it may have made more
			// objects reach refCount == 1.
			if( ++destroyOldIdx < gcOldObjects.GetLength() )
			{
				asSObjTypePair gcObj = GetOldObjectAtIdx(destroyOldIdx);

				if( gcObj.type->beh.gcGetRefCount == 0 )
				{
					// If circular references are formed with registered types that hasn't 
					// registered the GC behaviours, then the engine may be forced to free
					// the object type before the actual object instance. In this case we
					// will be forced to skip the destruction of the objects, so as not to 
					// crash the application.
					asCString msg;
					msg.Format(TXT_d_GC_CANNOT_FREE_OBJ_OF_TYPE_s, gcObj.seqNbr, gcObj.type->name.AddressOf());
					engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, msg.AddressOf());

					// Just remove the object, as we will not bother to destroy it
					numDestroyed++;
					RemoveOldObjectAtIdx(destroyOldIdx);
					destroyOldIdx--;
				}
				else if( engine->CallObjectMethodRetInt(gcObj.obj, gcObj.type->beh.gcGetRefCount) == 1 )
				{
					// Release the object immediately

					// Make sure the refCount is really 0, because the
					// destructor may have increased the refCount again.
					bool addRef = false;
					if( gcObj.type->flags & asOBJ_SCRIPT_OBJECT )
					{
						// Script objects may actually be resurrected in the destructor
						int refCount = ((asCScriptObject*)gcObj.obj)->Release();
						if( refCount > 0 ) addRef = true;
					}
					else
						engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.release);

					// Was the object really destroyed?
					if( !addRef )
					{
						numDestroyed++;
						RemoveOldObjectAtIdx(destroyOldIdx);
						destroyOldIdx--;
					}
					else
					{
						// Since the object was resurrected in the
						// destructor, we must add our reference again
						engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.addref);
					}

					destroyOldState = destroyGarbage_haveMore;
				}

				// Allow the application to work a little
				return 1;
			}
			else
			{
				if( destroyOldState == destroyGarbage_haveMore )
				{
					// Restart the cycle
					destroyOldState = destroyGarbage_init;
				}
				else
				{
					// Restart the cycle
					destroyOldState = destroyGarbage_init;

					// Return 0 to tell the application that there 
					// is no more garbage to destroy at the moment
					return 0;
				}
			}
		}
		break;
		}
	}

	// Shouldn't reach this point
	UNREACHABLE_RETURN;
}

int asCGarbageCollector::IdentifyGarbageWithCyclicRefs()
{
	// This function will only be called within the critical section gcCollecting
	asASSERT(isProcessing);

	for(;;)
	{
		switch( detectState )
		{
		case clearCounters_init:
			detectState = clearCounters_loop;
		break;

		case clearCounters_loop:
		{
			// Decrease reference counter for all objects removed from the map
			asSMapNode<void*, asSIntTypePair> *cursor = 0;
			gcMap.MoveFirst(&cursor);
			if( cursor )
			{
				void *obj = gcMap.GetKey(cursor);
				asSIntTypePair it = gcMap.GetValue(cursor);

				engine->CallObjectMethod(obj, it.type->beh.release);

				ReturnNode(gcMap.Remove(cursor));

				return 1;
			}

			detectState = buildMap_init;
		}
		break;

		case buildMap_init:
			detectIdx = 0;
			detectState = buildMap_loop;
		break;

		case buildMap_loop:
		{
			// Build a map of objects that will be checked, the map will
			// hold the object pointer as key, and the gcCount and the
			// object's type as value. As objects are added to the map the
			// gcFlag must be set in the objects, so we can be verify if
			// the object is accessed during the GC cycle.

			// If an object is removed from the gcObjects list during the
			// iteration of this step, it is possible that an object won't
			// be used during the analyzing for cyclic references. This
			// isn't a problem, as the next time the GC cycle starts the
			// object will be verified.
			if( detectIdx < gcOldObjects.GetLength() )
			{
				// Add the gc count for this object
				asSObjTypePair gcObj = GetOldObjectAtIdx(detectIdx);
	
				int refCount = 0;
				if( gcObj.type->beh.gcGetRefCount )
					refCount = engine->CallObjectMethodRetInt(gcObj.obj, gcObj.type->beh.gcGetRefCount);

				if( refCount > 1 )
				{
					asSIntTypePair it = {refCount-1, gcObj.type};

					gcMap.Insert(GetNode(gcObj.obj, it));

					// Increment the object's reference counter when putting it in the map
					engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.addref);

					// Mark the object so that we can
					// see if it has changed since read
					engine->CallObjectMethod(gcObj.obj, gcObj.type->beh.gcSetFlag);
				}

				detectIdx++; 

				// Let the application work a little
				return 1;
			}
			else
				detectState = countReferences_init;
		}
		break;

		case countReferences_init:
		{
			gcMap.MoveFirst(&gcMapCursor);
			detectState = countReferences_loop;
		}
		break;

		case countReferences_loop:
		{
			// Call EnumReferences on all objects in the map to count the number
			// of references reachable from between objects in the map. If all
			// references for an object in the map is reachable from other objects
			// in the map, then we know that no outside references are held for
			// this object, thus it is a potential dead object in a circular reference.

			// If the gcFlag is cleared for an object we consider the object alive
			// and referenced from outside the GC, thus we don't enumerate its references.

			// Any new objects created after this step in the GC cycle won't be
			// in the map, and is thus automatically considered alive.
			if( gcMapCursor )
			{
				void *obj = gcMap.GetKey(gcMapCursor);
				asCObjectType *type = gcMap.GetValue(gcMapCursor).type;
				gcMap.MoveNext(&gcMapCursor, gcMapCursor);

				if( engine->CallObjectMethodRetBool(obj, type->beh.gcGetFlag) )
				{
					engine->CallObjectMethod(obj, engine, type->beh.gcEnumReferences);
				}

				// Allow the application to work a little
				return 1;
			}
			else
				detectState = detectGarbage_init;
		}
		break;

		case detectGarbage_init:
		{
			gcMap.MoveFirst(&gcMapCursor);
			liveObjects.SetLength(0);
			detectState = detectGarbage_loop1;
		}
		break;

		case detectGarbage_loop1:
		{
			// All objects that are known not to be dead must be removed from the map,
			// along with all objects they reference. What remains in the map after
			// this pass is sure to be dead objects in circular references.

			// An object is considered alive if its gcFlag is cleared, or all the
			// references were not found in the map.

			// Add all alive objects from the map to the liveObjects array
			if( gcMapCursor )
			{
				asSMapNode<void*, asSIntTypePair> *cursor = gcMapCursor;
				gcMap.MoveNext(&gcMapCursor, gcMapCursor);

				void *obj = gcMap.GetKey(cursor);
				asSIntTypePair it = gcMap.GetValue(cursor);

				bool gcFlag = engine->CallObjectMethodRetBool(obj, it.type->beh.gcGetFlag);
				if( !gcFlag || it.i > 0 )
				{
					liveObjects.PushLast(obj);
				}

				// Allow the application to work a little
				return 1;
			}
			else
				detectState = detectGarbage_loop2;
		}
		break;

		case detectGarbage_loop2:
		{
			// In this step we are actually removing the alive objects from the map.
			// As the object is removed, all the objects it references are added to the
			// liveObjects list, by calling EnumReferences. Only objects still in the map
			// will be added to the liveObjects list.
			if( liveObjects.GetLength() )
			{
				void *gcObj = liveObjects.PopLast();
				asCObjectType *type = 0;

				// Remove the object from the map to mark it as alive
				asSMapNode<void*, asSIntTypePair> *cursor = 0;
				if( gcMap.MoveTo(&cursor, gcObj) )
				{
					type = gcMap.GetValue(cursor).type;
					ReturnNode(gcMap.Remove(cursor));

					// We need to decrease the reference count again as we remove the object from the map
					engine->CallObjectMethod(gcObj, type->beh.release);

					// Enumerate all the object's references so that they too can be marked as alive
					engine->CallObjectMethod(gcObj, engine, type->beh.gcEnumReferences);
				}

				// Allow the application to work a little
				return 1;
			}
			else
				detectState = verifyUnmarked_init;
		}
		break;

		case verifyUnmarked_init:
			gcMap.MoveFirst(&gcMapCursor);
			detectState = verifyUnmarked_loop;
			break;

		case verifyUnmarked_loop:
		{
			// In this step we must make sure that none of the objects still in the map
			// has been touched by the application. If they have then we must run the
			// detectGarbage loop once more.
			if( gcMapCursor )
			{
				void *gcObj = gcMap.GetKey(gcMapCursor);
				asCObjectType *type = gcMap.GetValue(gcMapCursor).type;

				bool gcFlag = engine->CallObjectMethodRetBool(gcObj, type->beh.gcGetFlag);
				if( !gcFlag )
				{
					// The unmarked object was touched, rerun the detectGarbage loop
					detectState = detectGarbage_init;
				}
				else
					gcMap.MoveNext(&gcMapCursor, gcMapCursor);

				// Allow the application to work a little
				return 1;
			}
			else
			{
				// No unmarked object was touched, we can now be sure
				// that objects that have gcCount == 0 really is garbage
				detectState = breakCircles_init;
			}
		}
		break;

		case breakCircles_init:
		{
			gcMap.MoveFirst(&gcMapCursor);
			detectState = breakCircles_loop;
		}
		break;

		case breakCircles_loop:
		case breakCircles_haveGarbage:
		{
			// All objects in the map are now known to be dead objects
			// kept alive through circular references. To be able to free
			// these objects we need to force the breaking of the circle
			// by having the objects release their references.
			if( gcMapCursor )
			{
				numDetected++;
				void *gcObj = gcMap.GetKey(gcMapCursor);
				asCObjectType *type = gcMap.GetValue(gcMapCursor).type;
				if( type->flags & asOBJ_SCRIPT_OBJECT )
				{
					// For script objects we must call the class destructor before
					// releasing the references, otherwise the destructor may not
					// be able to perform the necessary clean-up as the handles will 
					// be null.
					reinterpret_cast<asCScriptObject*>(gcObj)->CallDestructor();
				}
				engine->CallObjectMethod(gcObj, engine, type->beh.gcReleaseAllReferences);

				gcMap.MoveNext(&gcMapCursor, gcMapCursor);

				detectState = breakCircles_haveGarbage;

				// Allow the application to work a little
				return 1;
			}
			else
			{
				// If no garbage was detected we can finish now
				if( detectState != breakCircles_haveGarbage )
				{
					// Restart the GC
					detectState = clearCounters_init;
					return 0;
				}
				else
				{
					// Restart the GC
					detectState = clearCounters_init;
					return 1;
				}
			}
		}
		} // switch
	}

	// Shouldn't reach this point
	UNREACHABLE_RETURN;
}

asCGarbageCollector::asSMapNode_t *asCGarbageCollector::GetNode(void *obj, asSIntTypePair it)
{
	// This function will only be called within the critical section gcCollecting
	asASSERT(isProcessing);

	asSMapNode_t *node;
	if( freeNodes.GetLength() )
		node = freeNodes.PopLast();
	else
		node = asNEW(asSMapNode_t);

	node->Init(obj, it);
	return node;
}

void asCGarbageCollector::ReturnNode(asSMapNode_t *node)
{
	// This function will only be called within the critical section gcCollecting
	asASSERT(isProcessing);

	if( node )
		freeNodes.PushLast(node);
}

void asCGarbageCollector::GCEnumCallback(void *reference)
{
	// This function will only be called within the critical section gcCollecting
	asASSERT(isProcessing);

	if( detectState == countReferences_loop )
	{
		// Find the reference in the map
		asSMapNode<void*, asSIntTypePair> *cursor = 0;
		if( gcMap.MoveTo(&cursor, reference) )
		{
			// Decrease the counter in the map for the reference
			gcMap.GetValue(cursor).i--;
		}
	}
	else if( detectState == detectGarbage_loop2 )
	{
		// Find the reference in the map
		asSMapNode<void*, asSIntTypePair> *cursor = 0;
		if( gcMap.MoveTo(&cursor, reference) )
		{
			// Add the object to the list of objects to mark as alive
			liveObjects.PushLast(reference);
		}
	}
}

END_AS_NAMESPACE

