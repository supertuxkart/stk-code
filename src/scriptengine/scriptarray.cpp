#include <new>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h> // sprintf

#include "scriptarray.hpp"

using namespace std;

BEGIN_AS_NAMESPACE

static void RegisterScriptArray_Native(asIScriptEngine *engine);
static void RegisterScriptArray_Generic(asIScriptEngine *engine);

struct SArrayBuffer
{
	asDWORD maxElements;
	asDWORD numElements;
	asBYTE  data[1];
};

struct SArrayCache
{
	asIScriptFunction *cmpFunc;
	asIScriptFunction *eqFunc;
	int cmpFuncReturnCode; // To allow better error message in case of multiple matches
    int eqFuncReturnCode;
};

// We just define a number here that we assume nobody else is using for
// object type user data. The add-ons have reserved the numbers 1000
// through 1999 for this purpose, so we should be fine.
const asPWORD ARRAY_CACHE = 1000;

static void CleanupObjectTypeArrayCache(asIObjectType *type)
{
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(type->GetUserData(ARRAY_CACHE));
	if( cache )
		delete cache;
}

static CScriptArray* ScriptArrayFactory2(asIObjectType *ot, asUINT length)
{
	CScriptArray *a = new CScriptArray(length, ot);

	// It's possible the constructor raised a script exception, in which case we
	// need to free the memory and return null instead, else we get a memory leak.
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
	{
		a->Release();
		return 0;
	}

	return a;
}

static CScriptArray* ScriptArrayListFactory(asIObjectType *ot, void *initList)
{
	CScriptArray *a = new CScriptArray(ot, initList);

	// It's possible the constructor raised a script exception, in which case we
	// need to free the memory and return null instead, else we get a memory leak.
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
	{
		a->Release();
		return 0;
	}

	return a;
}

static CScriptArray* ScriptArrayFactoryDefVal(asIObjectType *ot, asUINT length, void *defVal)
{
	CScriptArray *a = new CScriptArray(length, defVal, ot);

	// It's possible the constructor raised a script exception, in which case we
	// need to free the memory and return null instead, else we get a memory leak.
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
	{
		a->Release();
		return 0;
	}

	return a;
}

static CScriptArray* ScriptArrayFactory(asIObjectType *ot)
{
	return ScriptArrayFactory2(ot, asUINT(0));
}

// This optional callback is called when the template type is first used by the compiler.
// It allows the application to validate if the template can be instanciated for the requested
// subtype at compile time, instead of at runtime. The output argument dontGarbageCollect
// allow the callback to tell the engine if the template instance type shouldn't be garbage collected,
// i.e. no asOBJ_GC flag.
static bool ScriptArrayTemplateCallback(asIObjectType *ot, bool &dontGarbageCollect)
{
	// Make sure the subtype can be instanciated with a default factory/constructor,
	// otherwise we won't be able to instanciate the elements.
	int typeId = ot->GetSubTypeId();
	if( typeId == asTYPEID_VOID )
		return false;
	if( (typeId & asTYPEID_MASK_OBJECT) && !(typeId & asTYPEID_OBJHANDLE) )
	{
		asIObjectType *subtype = ot->GetEngine()->GetObjectTypeById(typeId);
		asDWORD flags = subtype->GetFlags();
		if( (flags & asOBJ_VALUE) && !(flags & asOBJ_POD) )
		{
			// Verify that there is a default constructor
			bool found = false;
			for( asUINT n = 0; n < subtype->GetBehaviourCount(); n++ )
			{
				asEBehaviours beh;
				asIScriptFunction *func = subtype->GetBehaviourByIndex(n, &beh);
				if( beh != asBEHAVE_CONSTRUCT ) continue;

				if( func->GetParamCount() == 0 )
				{
					// Found the default constructor
					found = true;
					break;
				}
			}

			if( !found )
			{
				// There is no default constructor
				return false;
			}
		}
		else if( (flags & asOBJ_REF) )
		{
			bool found = false;

			// If value assignment for ref type has been disabled then the array
			// can be created if the type has a default factory function
			if( !ot->GetEngine()->GetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE) )
			{
				// Verify that there is a default factory
				for( asUINT n = 0; n < subtype->GetFactoryCount(); n++ )
				{
					asIScriptFunction *func = subtype->GetFactoryByIndex(n);
					if( func->GetParamCount() == 0 )
					{
						// Found the default factory
						found = true;
						break;
					}
				}
			}

			if( !found )
			{
				// No default factory
				return false;
			}
		}

		// If the object type is not garbage collected then the array also doesn't need to be
		if( !(flags & asOBJ_GC) )
			dontGarbageCollect = true;
	}
	else if( !(typeId & asTYPEID_OBJHANDLE) )
	{
		// Arrays with primitives cannot form circular references,
		// thus there is no need to garbage collect them
		dontGarbageCollect = true;
	}

	// The type is ok
	return true;
}

// Registers the template array type
void RegisterScriptArray(asIScriptEngine *engine, bool defaultArray)
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") == 0 )
		RegisterScriptArray_Native(engine);
	else
		RegisterScriptArray_Generic(engine);

	if( defaultArray )
	{
		int r = engine->RegisterDefaultArrayType("array<T>"); assert( r >= 0 );
	}
}

static void RegisterScriptArray_Native(asIScriptEngine *engine)
{
	int r;

	// Register the object type user data clean up
	engine->SetObjectTypeUserDataCleanupCallback(CleanupObjectTypeArrayCache, ARRAY_CACHE);

	// Register the array type as a template
	r = engine->RegisterObjectType("array<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE); assert( r >= 0 );

	// Register a callback for validating the subtype before it is used
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(ScriptArrayTemplateCallback), asCALL_CDECL); assert( r >= 0 );

	// Templates receive the object type as the first parameter. To the script writer this is hidden
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in)", asFUNCTIONPR(ScriptArrayFactory, (asIObjectType*), CScriptArray*), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint)", asFUNCTIONPR(ScriptArrayFactory2, (asIObjectType*, asUINT), CScriptArray*), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint, const T &in)", asFUNCTIONPR(ScriptArrayFactoryDefVal, (asIObjectType*, asUINT, void *), CScriptArray*), asCALL_CDECL); assert( r >= 0 );

	// Register the factory that will be used for initialization lists
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_LIST_FACTORY, "array<T>@ f(int&in type, int&in list) {repeat T}", asFUNCTIONPR(ScriptArrayListFactory, (asIObjectType*, void*), CScriptArray*), asCALL_CDECL); assert( r >= 0 );

	// The memory management methods
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_ADDREF, "void f()", asMETHOD(CScriptArray,AddRef), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASE, "void f()", asMETHOD(CScriptArray,Release), asCALL_THISCALL); assert( r >= 0 );

	// The index operator returns the template subtype
	r = engine->RegisterObjectMethod("array<T>", "T &opIndex(uint)", asMETHODPR(CScriptArray, At, (asUINT), void*), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "const T &opIndex(uint) const", asMETHODPR(CScriptArray, At, (asUINT) const, const void*), asCALL_THISCALL); assert( r >= 0 );

	// The assignment operator
	r = engine->RegisterObjectMethod("array<T>", "array<T> &opAssign(const array<T>&in)", asMETHOD(CScriptArray, operator=), asCALL_THISCALL); assert( r >= 0 );

	// Other methods
	r = engine->RegisterObjectMethod("array<T>", "void insertAt(uint, const T&in)", asMETHOD(CScriptArray, InsertAt), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void removeAt(uint)", asMETHOD(CScriptArray, RemoveAt), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void insertLast(const T&in)", asMETHOD(CScriptArray, InsertLast), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void removeLast()", asMETHOD(CScriptArray, RemoveLast), asCALL_THISCALL); assert( r >= 0 );
	// TODO: Should length() and resize() be deprecated as the property accessors do the same thing?
	r = engine->RegisterObjectMethod("array<T>", "uint length() const", asMETHOD(CScriptArray, GetSize), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void reserve(uint)", asMETHOD(CScriptArray, Reserve), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void resize(uint)", asMETHODPR(CScriptArray, Resize, (asUINT), void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortAsc()", asMETHODPR(CScriptArray, SortAsc, (), void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortAsc(uint, uint)", asMETHODPR(CScriptArray, SortAsc, (asUINT, asUINT), void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortDesc()", asMETHODPR(CScriptArray, SortDesc, (), void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortDesc(uint, uint)", asMETHODPR(CScriptArray, SortDesc, (asUINT, asUINT), void), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void reverse()", asMETHOD(CScriptArray, Reverse), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int find(const T&in) const", asMETHODPR(CScriptArray, Find, (void*) const, int), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int find(uint, const T&in) const", asMETHODPR(CScriptArray, Find, (asUINT, void*) const, int), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int findByRef(const T&in) const", asMETHODPR(CScriptArray, FindByRef, (void*) const, int), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int findByRef(uint, const T&in) const", asMETHODPR(CScriptArray, FindByRef, (asUINT, void*) const, int), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "bool opEquals(const array<T>&in) const", asMETHOD(CScriptArray, operator==), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "bool isEmpty() const", asMETHOD(CScriptArray, IsEmpty), asCALL_THISCALL); assert( r >= 0 );

	// Register virtual properties
	r = engine->RegisterObjectMethod("array<T>", "uint get_length() const", asMETHOD(CScriptArray, GetSize), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void set_length(uint)", asMETHODPR(CScriptArray, Resize, (asUINT), void), asCALL_THISCALL); assert( r >= 0 );

	// Register GC behaviours in case the array needs to be garbage collected
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(CScriptArray, GetRefCount), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(CScriptArray, SetFlag), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(CScriptArray, GetFlag), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(CScriptArray, EnumReferences), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(CScriptArray, ReleaseAllHandles), asCALL_THISCALL); assert( r >= 0 );

#if AS_USE_STLNAMES == 1
	// Same as length
	r = engine->RegisterObjectMethod("array<T>", "uint size() const", asMETHOD(CScriptArray, GetSize), asCALL_THISCALL); assert( r >= 0 );
	// Same as isEmpty
	r = engine->RegisterObjectMethod("array<T>", "bool empty() const", asMETHOD(CScriptArray, IsEmpty), asCALL_THISCALL); assert( r >= 0 );
	// Same as insertLast
	r = engine->RegisterObjectMethod("array<T>", "void push_back(const T&in)", asMETHOD(CScriptArray, InsertLast), asCALL_THISCALL); assert( r >= 0 );
	// Same as removeLast
	r = engine->RegisterObjectMethod("array<T>", "void pop_back()", asMETHOD(CScriptArray, RemoveLast), asCALL_THISCALL); assert( r >= 0 );
	// Same as insertAt
	r = engine->RegisterObjectMethod("array<T>", "void insert(uint, const T&in)", asMETHOD(CScriptArray, InsertAt), asCALL_THISCALL); assert( r >= 0 );
	// Same as removeAt
	r = engine->RegisterObjectMethod("array<T>", "void erase(uint)", asMETHOD(CScriptArray, RemoveAt), asCALL_THISCALL); assert( r >= 0 );
#endif
}

CScriptArray &CScriptArray::operator=(const CScriptArray &other)
{
	// Only perform the copy if the array types are the same
	if( &other != this &&
		other.GetArrayObjectType() == GetArrayObjectType() )
	{
		// Make sure the arrays are of the same size
		Resize(other.buffer->numElements);

		// Copy the value of each element
		CopyBuffer(buffer, other.buffer);
	}

	return *this;
}

CScriptArray::CScriptArray(asIObjectType *ot, void *buf)
{
	refCount = 1;
	gcFlag = false;
	objType = ot;
	objType->AddRef();
	buffer = 0;

	Precache();

	asIScriptEngine *engine = ot->GetEngine();

	// Determine element size
	if( subTypeId & asTYPEID_MASK_OBJECT )
		elementSize = sizeof(asPWORD);
	else
		elementSize = engine->GetSizeOfPrimitiveType(subTypeId);

	// Determine the initial size from the buffer
	asUINT length = *(asUINT*)buf;

	// Make sure the array size isn't too large for us to handle
	if( !CheckMaxSize(length) )
	{
		// Don't continue with the initialization
		return;
	}

	// Copy the values of the array elements from the buffer
	if( (ot->GetSubTypeId() & asTYPEID_MASK_OBJECT) == 0 )
	{
		CreateBuffer(&buffer, length);

		// Copy the values of the primitive type into the internal buffer
		memcpy(At(0), (((asUINT*)buf)+1), length * elementSize);
	}
	else if( ot->GetSubTypeId() & asTYPEID_OBJHANDLE )
	{
		CreateBuffer(&buffer, length);

		// Copy the handles into the internal buffer
		memcpy(At(0), (((asUINT*)buf)+1), length * elementSize);

		// With object handles it is safe to clear the memory in the received buffer
		// instead of increasing the ref count. It will save time both by avoiding the
		// call the increase ref, and also relieve the engine from having to release
		// its references too
		memset((((asUINT*)buf)+1), 0, length * elementSize);
	}
	else if( ot->GetSubType()->GetFlags() & asOBJ_REF )
	{
		// Only allocate the buffer, but not the objects
		subTypeId |= asTYPEID_OBJHANDLE;
		CreateBuffer(&buffer, length);
		subTypeId &= ~asTYPEID_OBJHANDLE;

		// Copy the handles into the internal buffer
		memcpy(buffer->data, (((asUINT*)buf)+1), length * elementSize);

		// For ref types we can do the same as for handles, as they are
		// implicitly stored as handles.
		memset((((asUINT*)buf)+1), 0, length * elementSize);
	}
	else
	{
		// TODO: Optimize by calling the copy constructor of the object instead of
		//       constructing with the default constructor and then assigning the value
		// TODO: With C++11 ideally we should be calling the move constructor, instead
		//       of the copy constructor as the engine will just discard the objects in the
		//       buffer afterwards.
		CreateBuffer(&buffer, length);

		// For value types we need to call the opAssign for each individual object
		for( asUINT n = 0; n < length; n++ )
		{
			void *obj = At(n);
			asBYTE *srcObj = (asBYTE*)buf;
			srcObj += 4 + n*ot->GetSubType()->GetSize();
			engine->AssignScriptObject(obj, srcObj, ot->GetSubType());
		}
	}

	// Notify the GC of the successful creation
	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);
}

CScriptArray::CScriptArray(asUINT length, asIObjectType *ot)
{
	refCount = 1;
	gcFlag = false;
	objType = ot;
	objType->AddRef();
	buffer = 0;

	Precache();

	// Determine element size
	if( subTypeId & asTYPEID_MASK_OBJECT )
		elementSize = sizeof(asPWORD);
	else
		elementSize = objType->GetEngine()->GetSizeOfPrimitiveType(subTypeId);

	// Make sure the array size isn't too large for us to handle
	if( !CheckMaxSize(length) )
	{
		// Don't continue with the initialization
		return;
	}

	CreateBuffer(&buffer, length);

	// Notify the GC of the successful creation
	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);
}

CScriptArray::CScriptArray(const CScriptArray &other)
{
	refCount = 1;
	gcFlag = false;
	objType = other.objType;
	objType->AddRef();
	buffer = 0;

	Precache();

	elementSize = other.elementSize;

	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);

	CreateBuffer(&buffer, 0);

	// Copy the content
	*this = other;
}

CScriptArray::CScriptArray(asUINT length, void *defVal, asIObjectType *ot)
{
	refCount = 1;
	gcFlag = false;
	objType = ot;
	objType->AddRef();
	buffer = 0;

	Precache();

	// Determine element size
	if( subTypeId & asTYPEID_MASK_OBJECT )
		elementSize = sizeof(asPWORD);
	else
		elementSize = objType->GetEngine()->GetSizeOfPrimitiveType(subTypeId);

	// Make sure the array size isn't too large for us to handle
	if( !CheckMaxSize(length) )
	{
		// Don't continue with the initialization
		return;
	}

	CreateBuffer(&buffer, length);

	// Notify the GC of the successful creation
	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);

	// Initialize the elements with the default value
	for( asUINT n = 0; n < GetSize(); n++ )
		SetValue(n, defVal);
}

void CScriptArray::SetValue(asUINT index, void *value)
{
	// At() will take care of the out-of-bounds checking, though
	// if called from the application then nothing will be done
	void *ptr = At(index);
	if( ptr == 0 ) return;

	if( (subTypeId & ~asTYPEID_MASK_SEQNBR) && !(subTypeId & asTYPEID_OBJHANDLE) )
		objType->GetEngine()->AssignScriptObject(ptr, value, objType->GetSubType());
	else if( subTypeId & asTYPEID_OBJHANDLE )
	{
		void *tmp = *(void**)ptr;
		*(void**)ptr = *(void**)value;
		objType->GetEngine()->AddRefScriptObject(*(void**)value, objType->GetSubType());
		if( tmp )
			objType->GetEngine()->ReleaseScriptObject(tmp, objType->GetSubType());
	}
	else if( subTypeId == asTYPEID_BOOL ||
			 subTypeId == asTYPEID_INT8 ||
			 subTypeId == asTYPEID_UINT8 )
		*(char*)ptr = *(char*)value;
	else if( subTypeId == asTYPEID_INT16 ||
			 subTypeId == asTYPEID_UINT16 )
		*(short*)ptr = *(short*)value;
	else if( subTypeId == asTYPEID_INT32 ||
			 subTypeId == asTYPEID_UINT32 ||
			 subTypeId == asTYPEID_FLOAT ||
			 subTypeId > asTYPEID_DOUBLE ) // enums have a type id larger than doubles
		*(int*)ptr = *(int*)value;
	else if( subTypeId == asTYPEID_INT64 ||
			 subTypeId == asTYPEID_UINT64 ||
			 subTypeId == asTYPEID_DOUBLE )
		*(double*)ptr = *(double*)value;
}

CScriptArray::~CScriptArray()
{
	if( buffer )
	{
		DeleteBuffer(buffer);
		buffer = 0;
	}
	if( objType ) objType->Release();
}

asUINT CScriptArray::GetSize() const
{
	return buffer->numElements;
}

bool CScriptArray::IsEmpty() const
{
	return buffer->numElements == 0;
}

void CScriptArray::Reserve(asUINT maxElements)
{
	if( maxElements <= buffer->maxElements )
		return;

	if( !CheckMaxSize(maxElements) )
		return;

	// Allocate memory for the buffer
	SArrayBuffer *newBuffer;
	#if defined(__S3E__) // Marmalade doesn't understand (nothrow)
	newBuffer = (SArrayBuffer*)new asBYTE[sizeof(SArrayBuffer)-1 + elementSize*maxElements];
	#else
	newBuffer = (SArrayBuffer*)new (nothrow) asBYTE[sizeof(SArrayBuffer)-1 + elementSize*maxElements];
	#endif
	if( newBuffer )
	{
		newBuffer->numElements = buffer->numElements;
		newBuffer->maxElements = maxElements;
	}
	else
	{
		// Out of memory
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Out of memory");
		return;
	}

	memcpy(newBuffer->data, buffer->data, buffer->numElements*elementSize);

	// Release the old buffer
	delete[] (asBYTE*)buffer;

	buffer = newBuffer;
}

void CScriptArray::Resize(asUINT numElements)
{
	if( !CheckMaxSize(numElements) )
		return;

	Resize((int)numElements - (int)buffer->numElements, (asUINT)-1);
}

// Internal
void CScriptArray::Resize(int delta, asUINT at)
{
	if( delta < 0 )
	{
		if( -delta > (int)buffer->numElements )
			delta = -(int)buffer->numElements;
		if( at > buffer->numElements + delta )
			at = buffer->numElements + delta;
	}
	else if( delta > 0 )
	{
		// Make sure the array size isn't too large for us to handle
		if( delta > 0 && !CheckMaxSize(buffer->numElements + delta) )
			return;

		if( at > buffer->numElements )
			at = buffer->numElements;
	}

	if( delta == 0 ) return;

	if( buffer->maxElements < buffer->numElements + delta )
	{
		// Allocate memory for the buffer
		SArrayBuffer *newBuffer;
		#if defined(__S3E__) // Marmalade doesn't understand (nothrow)
		newBuffer = (SArrayBuffer*)new asBYTE[sizeof(SArrayBuffer)-1 + elementSize*(buffer->numElements + delta)];
		#else
		newBuffer = (SArrayBuffer*)new (nothrow) asBYTE[sizeof(SArrayBuffer)-1 + elementSize*(buffer->numElements + delta)];
		#endif
		if( newBuffer )
		{
			newBuffer->numElements = buffer->numElements + delta;
			newBuffer->maxElements = newBuffer->numElements;
		}
		else
		{
			// Out of memory
			asIScriptContext *ctx = asGetActiveContext();
			if( ctx )
				ctx->SetException("Out of memory");
			return;
		}

		// TODO: memcpy assumes the objects in the array doesn't hold pointers to themselves
		//       This should really be using the objects copy constructor to copy each object
		//       to the new location. It would most likely be a hit on the performance though.
		memcpy(newBuffer->data, buffer->data, at*elementSize);
		if( at < buffer->numElements )
			memcpy(newBuffer->data + (at+delta)*elementSize, buffer->data + at*elementSize, (buffer->numElements-at)*elementSize);

		if( subTypeId & asTYPEID_MASK_OBJECT )
			Construct(newBuffer, at, at+delta);

		// Release the old buffer
		delete[] (asBYTE*)buffer;

		buffer = newBuffer;
	}
	else if( delta < 0 )
	{
		Destruct(buffer, at, at-delta);
		// TODO: memmove assumes the objects in the array doesn't hold pointers to themselves
		//       This should really be using the objects copy constructor to copy each object
		//       to the new location. It would most likely be a hit on the performance though.
		memmove(buffer->data + at*elementSize, buffer->data + (at-delta)*elementSize, (buffer->numElements - (at-delta))*elementSize);
		buffer->numElements += delta;
	}
	else
	{
		// TODO: memmove assumes the objects in the array doesn't hold pointers to themselves
		//       This should really be using the objects copy constructor to copy each object
		//       to the new location. It would most likely be a hit on the performance though.
		memmove(buffer->data + (at+delta)*elementSize, buffer->data + at*elementSize, (buffer->numElements - at)*elementSize);
		Construct(buffer, at, at+delta);
		buffer->numElements += delta;
	}
}

// internal
bool CScriptArray::CheckMaxSize(asUINT numElements)
{
	// This code makes sure the size of the buffer that is allocated
	// for the array doesn't overflow and becomes smaller than requested

	asUINT maxSize = 0xFFFFFFFFul - sizeof(SArrayBuffer) + 1;
	if( subTypeId & asTYPEID_MASK_OBJECT )
		maxSize /= sizeof(void*);
	else if( elementSize > 0 )
		maxSize /= elementSize;

	if( numElements > maxSize )
	{
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
		{
			// Set a script exception
			ctx->SetException("Too large array size");
		}

		return false;
	}

	// OK
	return true;
}

asIObjectType *CScriptArray::GetArrayObjectType() const
{
	return objType;
}

int CScriptArray::GetArrayTypeId() const
{
	return objType->GetTypeId();
}

int CScriptArray::GetElementTypeId() const
{
	return subTypeId;
}

void CScriptArray::InsertAt(asUINT index, void *value)
{
	if( index > buffer->numElements )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Index out of bounds");
		return;
	}

	// Make room for the new element
	Resize(1, index);

	// Set the value of the new element
	SetValue(index, value);
}

void CScriptArray::InsertLast(void *value)
{
	InsertAt(buffer->numElements, value);
}

void CScriptArray::RemoveAt(asUINT index)
{
	if( index >= buffer->numElements )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Index out of bounds");
		return;
	}

	// Remove the element
	Resize(-1, index);
}

void CScriptArray::RemoveLast()
{
	RemoveAt(buffer->numElements-1);
}

// Return a pointer to the array element. Returns 0 if the index is out of bounds
const void *CScriptArray::At(asUINT index) const
{
	if( buffer == 0 || index >= buffer->numElements )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Index out of bounds");
		return 0;
	}

	if( (subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) )
		return (void*)((size_t*)buffer->data)[index];
	else
		return buffer->data + elementSize*index;
}
void *CScriptArray::At(asUINT index)
{
	return const_cast<void*>(const_cast<const CScriptArray *>(this)->At(index));
}


// internal
void CScriptArray::CreateBuffer(SArrayBuffer **buf, asUINT numElements)
{
	if( subTypeId & asTYPEID_MASK_OBJECT )
	{
		#if defined(__S3E__) // Marmalade doesn't understand (nothrow)
		*buf = (SArrayBuffer*)new asBYTE[sizeof(SArrayBuffer)-1+sizeof(void*)*numElements];
		#else
		*buf = (SArrayBuffer*)new (nothrow) asBYTE[sizeof(SArrayBuffer)-1+sizeof(void*)*numElements];
		#endif
	}
	else
	{
		#if defined(__S3E__)
		*buf = (SArrayBuffer*)new asBYTE[sizeof(SArrayBuffer)-1+elementSize*numElements];
		#else
		*buf = (SArrayBuffer*)new (nothrow) asBYTE[sizeof(SArrayBuffer)-1+elementSize*numElements];
		#endif
	}

	if( *buf )
	{
		(*buf)->numElements = numElements;
		(*buf)->maxElements = numElements;
		Construct(*buf, 0, numElements);
	}
	else
	{
		// Oops, out of memory
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Out of memory");
	}
}

// internal
void CScriptArray::DeleteBuffer(SArrayBuffer *buf)
{
	Destruct(buf, 0, buf->numElements);

	// Free the buffer
	delete[] (asBYTE*)buf;
}

// internal
void CScriptArray::Construct(SArrayBuffer *buf, asUINT start, asUINT end)
{
	if( subTypeId & asTYPEID_OBJHANDLE )
	{
		// Set all object handles to null
		void *d = (void*)(buf->data + start * sizeof(void*));
		memset(d, 0, (end-start)*sizeof(void*));
	}
	else if( subTypeId & asTYPEID_MASK_OBJECT )
	{
		void **max = (void**)(buf->data + end * sizeof(void*));
		void **d = (void**)(buf->data + start * sizeof(void*));

		asIScriptEngine *engine = objType->GetEngine();
		asIObjectType *subType = objType->GetSubType();

		for( ; d < max; d++ )
			*d = (void*)engine->CreateScriptObject(subType);
	}
}

// internal
void CScriptArray::Destruct(SArrayBuffer *buf, asUINT start, asUINT end)
{
	if( subTypeId & asTYPEID_MASK_OBJECT )
	{
		asIScriptEngine *engine = objType->GetEngine();

		void **max = (void**)(buf->data + end * sizeof(void*));
		void **d   = (void**)(buf->data + start * sizeof(void*));

		for( ; d < max; d++ )
		{
			if( *d )
				engine->ReleaseScriptObject(*d, objType->GetSubType());
		}
	}
}


// internal
bool CScriptArray::Less(const void *a, const void *b, bool asc, asIScriptContext *ctx, SArrayCache *cache)
{
	if( !asc )
	{
		// Swap items
		const void *TEMP = a;
		a = b;
		b = TEMP;
	}

	if( !(subTypeId & ~asTYPEID_MASK_SEQNBR) )
	{
		// Simple compare of values
		switch( subTypeId )
		{
			#define COMPARE(T) *((T*)a) < *((T*)b)
			case asTYPEID_BOOL: return COMPARE(bool);
			case asTYPEID_INT8: return COMPARE(signed char);
			case asTYPEID_UINT8: return COMPARE(unsigned char);
			case asTYPEID_INT16: return COMPARE(signed short);
			case asTYPEID_UINT16: return COMPARE(unsigned short);
			case asTYPEID_INT32: return COMPARE(signed int);
			case asTYPEID_UINT32: return COMPARE(unsigned int);
			case asTYPEID_FLOAT: return COMPARE(float);
			case asTYPEID_DOUBLE: return COMPARE(double);
			default: return COMPARE(signed int); // All enums fall in this case
			#undef COMPARE
		}
	}
	else
	{
		int r = 0;

		if( subTypeId & asTYPEID_OBJHANDLE )
		{
			// Allow sort to work even if the array contains null handles
			if( *(void**)a == 0 ) return true;
			if( *(void**)b == 0 ) return false;
		}

		// Execute object opCmp
		if( cache && cache->cmpFunc )
		{
			// TODO: Add proper error handling
			r = ctx->Prepare(cache->cmpFunc); assert(r >= 0);

			if( subTypeId & asTYPEID_OBJHANDLE )
			{
				r = ctx->SetObject(*((void**)a)); assert(r >= 0);
				r = ctx->SetArgObject(0, *((void**)b)); assert(r >= 0);
			}
			else
			{
				r = ctx->SetObject((void*)a); assert(r >= 0);
				r = ctx->SetArgObject(0, (void*)b); assert(r >= 0);
			}

			r = ctx->Execute();

			if( r == asEXECUTION_FINISHED )
			{
				return (int)ctx->GetReturnDWord() < 0;
			}
		}
	}

	return false;
}

void CScriptArray::Reverse()
{
	asUINT size = GetSize();

	if( size >= 2 )
	{
		asBYTE TEMP[16];

		for( asUINT i = 0; i < size / 2; i++ )
		{
			Copy(TEMP, GetArrayItemPointer(i));
			Copy(GetArrayItemPointer(i), GetArrayItemPointer(size - i - 1));
			Copy(GetArrayItemPointer(size - i - 1), TEMP);
		}
	}
}

bool CScriptArray::operator==(const CScriptArray &other) const
{
	if( objType != other.objType )
		return false;

	if( GetSize() != other.GetSize() )
		return false;

	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = objType->GetEngine()->CreateContext();
		}
	}

	// Check if all elements are equal
	bool isEqual = true;
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	for( asUINT n = 0; n < GetSize(); n++ )
		if( !Equals(At(n), other.At(n), cmpContext, cache) )
		{
			isEqual = false;
			break;
		}

	if( cmpContext )
    {
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
    }

	return isEqual;
}

// internal
bool CScriptArray::Equals(const void *a, const void *b, asIScriptContext *ctx, SArrayCache *cache) const
{
	if( !(subTypeId & ~asTYPEID_MASK_SEQNBR) )
	{
		// Simple compare of values
		switch( subTypeId )
		{
			#define COMPARE(T) *((T*)a) == *((T*)b)
			case asTYPEID_BOOL: return COMPARE(bool);
			case asTYPEID_INT8: return COMPARE(signed char);
			case asTYPEID_UINT8: return COMPARE(unsigned char);
			case asTYPEID_INT16: return COMPARE(signed short);
			case asTYPEID_UINT16: return COMPARE(unsigned short);
			case asTYPEID_INT32: return COMPARE(signed int);
			case asTYPEID_UINT32: return COMPARE(unsigned int);
			case asTYPEID_FLOAT: return COMPARE(float);
			case asTYPEID_DOUBLE: return COMPARE(double);
			default: return COMPARE(signed int); // All enums fall here
			#undef COMPARE
		}
	}
	else
	{
		int r = 0;

		if( subTypeId & asTYPEID_OBJHANDLE )
		{
			// Allow the find to work even if the array contains null handles
			if( *(void**)a == *(void**)b ) return true;
		}

		// Execute object opEquals if available
		if( cache && cache->eqFunc )
		{
			// TODO: Add proper error handling
			r = ctx->Prepare(cache->eqFunc); assert(r >= 0);

			if( subTypeId & asTYPEID_OBJHANDLE )
			{
				r = ctx->SetObject(*((void**)a)); assert(r >= 0);
				r = ctx->SetArgObject(0, *((void**)b)); assert(r >= 0);
			}
			else
			{
				r = ctx->SetObject((void*)a); assert(r >= 0);
				r = ctx->SetArgObject(0, (void*)b); assert(r >= 0);
			}

			r = ctx->Execute();

			if( r == asEXECUTION_FINISHED )
				return ctx->GetReturnByte() != 0;

			return false;
		}

		// Execute object opCmp if available
		if( cache && cache->cmpFunc )
		{
			// TODO: Add proper error handling
			r = ctx->Prepare(cache->cmpFunc); assert(r >= 0);

			if( subTypeId & asTYPEID_OBJHANDLE )
			{
				r = ctx->SetObject(*((void**)a)); assert(r >= 0);
				r = ctx->SetArgObject(0, *((void**)b)); assert(r >= 0);
			}
			else
			{
				r = ctx->SetObject((void*)a); assert(r >= 0);
				r = ctx->SetArgObject(0, (void*)b); assert(r >= 0);
			}

			r = ctx->Execute();

			if( r == asEXECUTION_FINISHED )
				return (int)ctx->GetReturnDWord() == 0;

			return false;
		}
	}

	return false;
}

int CScriptArray::FindByRef(void *ref) const
{
	return FindByRef(0, ref);
}

int CScriptArray::FindByRef(asUINT startAt, void *ref) const
{
	// Find the matching element by its reference
	asUINT size = GetSize();
	if( subTypeId & asTYPEID_OBJHANDLE )
	{
		// Dereference the pointer
		ref = *(void**)ref;
		for( asUINT i = startAt; i < size; i++ )
		{
			if( *(void**)At(i) == ref )
				return i;
		}
	}
	else
	{
		// Compare the reference directly
		for( asUINT i = startAt; i < size; i++ )
		{
			if( At(i) == ref )
				return i;
		}
	}

	return -1;
}

int CScriptArray::Find(void *value) const
{
	return Find(0, value);
}

int CScriptArray::Find(asUINT startAt, void *value) const
{
	// Check if the subtype really supports find()
	// TODO: Can't this be done at compile time too by the template callback
	SArrayCache *cache = 0;
	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
		if( !cache || (cache->cmpFunc == 0 && cache->eqFunc == 0) )
		{
			asIScriptContext *ctx = asGetActiveContext();
			asIObjectType* subType = objType->GetEngine()->GetObjectTypeById(subTypeId);

			// Throw an exception
			if( ctx )
			{
				char tmp[512];

				if( cache && cache->eqFuncReturnCode == asMULTIPLE_FUNCTIONS )
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' has multiple matching opEquals or opCmp methods", subType->GetName());
#else
					sprintf(tmp, "Type '%s' has multiple matching opEquals or opCmp methods", subType->GetName());
#endif
				else
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' does not have a matching opEquals or opCmp method", subType->GetName());
#else
					sprintf(tmp, "Type '%s' does not have a matching opEquals or opCmp method", subType->GetName());
#endif
				ctx->SetException(tmp);
			}

			return -1;
		}
	}

	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = objType->GetEngine()->CreateContext();
		}
	}

	// Find the matching element
	int ret = -1;
	asUINT size = GetSize();

	for( asUINT i = startAt; i < size; i++ )
	{
		// value passed by reference
		if( Equals(At(i), value, cmpContext, cache) )
		{
			ret = (int)i;
			break;
		}
	}

	if( cmpContext )
	{
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
	}

	return ret;
}



// internal
// Copy object handle or primitive value
void CScriptArray::Copy(void *dst, void *src)
{
	memcpy(dst, src, elementSize);
}


// internal
// Return pointer to array item (object handle or primitive value)
void *CScriptArray::GetArrayItemPointer(int index)
{
	return buffer->data + index * elementSize;
}

// internal
// Return pointer to data in buffer (object or primitive)
void *CScriptArray::GetDataPointer(void *buffer)
{
	if ((subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) )
	{
		// Real address of object
		return reinterpret_cast<void*>(*(size_t*)buffer);
	}
	else
	{
		// Primitive is just a raw data
		return buffer;
	}
}


// Sort ascending
void CScriptArray::SortAsc()
{
	Sort(0, GetSize(), true);
}

// Sort ascending
void CScriptArray::SortAsc(asUINT startAt, asUINT count)
{
	Sort(startAt, count, true);
}

// Sort descending
void CScriptArray::SortDesc()
{
	Sort(0, GetSize(), false);
}

// Sort descending
void CScriptArray::SortDesc(asUINT startAt, asUINT count)
{
	Sort(startAt, count, false);
}


// internal
void CScriptArray::Sort(asUINT startAt, asUINT count, bool asc)
{
	// Subtype isn't primitive and doesn't have opCmp
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		if( !cache || cache->cmpFunc == 0 )
		{
			asIScriptContext *ctx = asGetActiveContext();
			asIObjectType* subType = objType->GetEngine()->GetObjectTypeById(subTypeId);

			// Throw an exception
			if( ctx )
			{
				char tmp[512];

				if( cache && cache->cmpFuncReturnCode == asMULTIPLE_FUNCTIONS )
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' has multiple matching opCmp methods", subType->GetName());
#else
					sprintf(tmp, "Type '%s' has multiple matching opCmp methods", subType->GetName());
#endif
				else
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' does not have a matching opCmp method", subType->GetName());
#else
					sprintf(tmp, "Type '%s' does not have a matching opCmp method", subType->GetName());
#endif

				ctx->SetException(tmp);
			}

			return;
		}
	}

	// No need to sort
	if( count < 2 )
	{
		return;
	}

	int start = startAt;
	int end = startAt + count;

	// Check if we could access invalid item while sorting
	if( start >= (int)buffer->numElements || end > (int)buffer->numElements )
	{
		asIScriptContext *ctx = asGetActiveContext();

		// Throw an exception
		if( ctx )
		{
			ctx->SetException("Index out of bounds");
		}

		return;
	}

	asBYTE tmp[16];
	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = objType->GetEngine()->CreateContext();
		}
	}

	// Insertion sort
	for( int i = start + 1; i < end; i++ )
	{
		Copy(tmp, GetArrayItemPointer(i));

		int j = i - 1;

		while( j >= start && Less(GetDataPointer(tmp), At(j), asc, cmpContext, cache) )
		{
			Copy(GetArrayItemPointer(j + 1), GetArrayItemPointer(j));
			j--;
		}

		Copy(GetArrayItemPointer(j + 1), tmp);
	}

	if( cmpContext )
    {
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
    }
}

// internal
void CScriptArray::CopyBuffer(SArrayBuffer *dst, SArrayBuffer *src)
{
	asIScriptEngine *engine = objType->GetEngine();
	if( subTypeId & asTYPEID_OBJHANDLE )
	{
		// Copy the references and increase the reference counters
		if( dst->numElements > 0 && src->numElements > 0 )
		{
			int count = dst->numElements > src->numElements ? src->numElements : dst->numElements;

			void **max = (void**)(dst->data + count * sizeof(void*));
			void **d   = (void**)dst->data;
			void **s   = (void**)src->data;

			for( ; d < max; d++, s++ )
			{
				void *tmp = *d;
				*d = *s;
				if( *d )
					engine->AddRefScriptObject(*d, objType->GetSubType());
				// Release the old ref after incrementing the new to avoid problem incase it is the same ref
				if( tmp )
					engine->ReleaseScriptObject(tmp, objType->GetSubType());
			}
		}
	}
	else
	{
		if( dst->numElements > 0 && src->numElements > 0 )
		{
			int count = dst->numElements > src->numElements ? src->numElements : dst->numElements;
			if( subTypeId & asTYPEID_MASK_OBJECT )
			{
				// Call the assignment operator on all of the objects
				void **max = (void**)(dst->data + count * sizeof(void*));
				void **d   = (void**)dst->data;
				void **s   = (void**)src->data;

				asIObjectType *subType = objType->GetSubType();
				for( ; d < max; d++, s++ )
					engine->AssignScriptObject(*d, *s, subType);
			}
			else
			{
				// Primitives are copied byte for byte
				memcpy(dst->data, src->data, count*elementSize);
			}
		}
	}
}

// internal
// Precache some info
void CScriptArray::Precache()
{
	subTypeId = objType->GetSubTypeId();

	// Check if it is an array of objects. Only for these do we need to cache anything
	// Type ids for primitives and enums only has the sequence number part
	if( !(subTypeId & ~asTYPEID_MASK_SEQNBR) )
		return;

	// The opCmp and opEquals methods are cached because the searching for the
	// methods is quite time consuming if a lot of array objects are created.

	// First check if a cache already exists for this array type
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	if( cache )	return;

	// We need to make sure the cache is created only once, even
	// if multiple threads reach the same point at the same time
	asAcquireExclusiveLock();

	// Now that we got the lock, we need to check again to make sure the
	// cache wasn't created while we were waiting for the lock
	cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	if( cache )
	{
		asReleaseExclusiveLock();
		return;
	}

	// Create the cache
	cache = new SArrayCache();
	memset(cache, 0, sizeof(SArrayCache));

	// If the sub type is a handle to const, then the methods must be const too
	bool mustBeConst = (subTypeId & asTYPEID_HANDLETOCONST) ? true : false;

	asIObjectType *subType = objType->GetEngine()->GetObjectTypeById(subTypeId);
	if( subType )
	{
		for( asUINT i = 0; i < subType->GetMethodCount(); i++ )
		{
			asIScriptFunction *func = subType->GetMethodByIndex(i);

			if( func->GetParamCount() == 1 && (!mustBeConst || func->IsReadOnly()) )
			{
				asDWORD flags = 0;
				int returnTypeId = func->GetReturnTypeId(&flags);

				// The method must not return a reference
				if( flags != asTM_NONE )
					continue;

				// opCmp returns an int and opEquals returns a bool
				bool isCmp = false, isEq = false;
				if( returnTypeId == asTYPEID_INT32 && strcmp(func->GetName(), "opCmp") == 0 )
					isCmp = true;
				if( returnTypeId == asTYPEID_BOOL && strcmp(func->GetName(), "opEquals") == 0 )
					isEq = true;

				if( !isCmp && !isEq )
					continue;

				// The parameter must either be a reference to the subtype or a handle to the subtype
				int paramTypeId = func->GetParamTypeId(0, &flags);

				if( (paramTypeId & ~(asTYPEID_OBJHANDLE|asTYPEID_HANDLETOCONST)) != (subTypeId &  ~(asTYPEID_OBJHANDLE|asTYPEID_HANDLETOCONST)) )
					continue;

				if( (flags & asTM_INREF) )
				{
					if( (paramTypeId & asTYPEID_OBJHANDLE) || (mustBeConst && !(flags & asTM_CONST)) )
						continue;
				}
				else if( paramTypeId & asTYPEID_OBJHANDLE )
				{
					if( mustBeConst && !(paramTypeId & asTYPEID_HANDLETOCONST) )
						continue;
				}
				else
					continue;

				if( isCmp )
				{
					if( cache->cmpFunc || cache->cmpFuncReturnCode )
					{
						cache->cmpFunc = 0;
						cache->cmpFuncReturnCode = asMULTIPLE_FUNCTIONS;
					}
					else
						cache->cmpFunc = func;
				}
				else if( isEq )
				{
					if( cache->eqFunc || cache->eqFuncReturnCode )
					{
						cache->eqFunc = 0;
						cache->eqFuncReturnCode = asMULTIPLE_FUNCTIONS;
					}
					else
						cache->eqFunc = func;
				}
			}
		}
	}

	if( cache->eqFunc == 0 && cache->eqFuncReturnCode == 0 )
		cache->eqFuncReturnCode = asNO_FUNCTION;
	if( cache->cmpFunc == 0 && cache->cmpFuncReturnCode == 0 )
		cache->cmpFuncReturnCode = asNO_FUNCTION;

	// Set the user data only at the end so others that retrieve it will know it is complete
	objType->SetUserData(cache, ARRAY_CACHE);

	asReleaseExclusiveLock();
}

// GC behaviour
void CScriptArray::EnumReferences(asIScriptEngine *engine)
{
	// If the array is holding handles, then we need to notify the GC of them
	if( subTypeId & asTYPEID_MASK_OBJECT )
	{
		void **d = (void**)buffer->data;
		for( asUINT n = 0; n < buffer->numElements; n++ )
		{
			if( d[n] )
				engine->GCEnumCallback(d[n]);
		}
	}
}

// GC behaviour
void CScriptArray::ReleaseAllHandles(asIScriptEngine *engine)
{
	// Resizing to zero will release everything
	Resize(0);
}

void CScriptArray::AddRef() const
{
	// Clear the GC flag then increase the counter
	gcFlag = false;
	asAtomicInc(refCount);
}

void CScriptArray::Release() const
{
	// Clearing the GC flag then descrease the counter
	gcFlag = false;
	if( asAtomicDec(refCount) == 0 )
	{
		// When reaching 0 no more references to this instance
		// exists and the object should be destroyed
		delete this;
	}
}

// GC behaviour
int CScriptArray::GetRefCount()
{
	return refCount;
}

// GC behaviour
void CScriptArray::SetFlag()
{
	gcFlag = true;
}

// GC behaviour
bool CScriptArray::GetFlag()
{
	return gcFlag;
}

//--------------------------------------------
// Generic calling conventions

static void ScriptArrayFactory_Generic(asIScriptGeneric *gen)
{
	asIObjectType *ot = *(asIObjectType**)gen->GetAddressOfArg(0);

	*(CScriptArray**)gen->GetAddressOfReturnLocation() = ScriptArrayFactory(ot);
}

static void ScriptArrayFactory2_Generic(asIScriptGeneric *gen)
{
	asIObjectType *ot = *(asIObjectType**)gen->GetAddressOfArg(0);
	asUINT length = gen->GetArgDWord(1);

	*(CScriptArray**)gen->GetAddressOfReturnLocation() = ScriptArrayFactory2(ot, length);
}

static void ScriptArrayListFactory_Generic(asIScriptGeneric *gen)
{
	asIObjectType *ot = *(asIObjectType**)gen->GetAddressOfArg(0);
	void *buf = gen->GetArgAddress(1);

	*(CScriptArray**)gen->GetAddressOfReturnLocation() = ScriptArrayListFactory(ot, buf);
}

static void ScriptArrayFactoryDefVal_Generic(asIScriptGeneric *gen)
{
	asIObjectType *ot = *(asIObjectType**)gen->GetAddressOfArg(0);
	asUINT length = gen->GetArgDWord(1);
	void *defVal = gen->GetArgAddress(2);

	*(CScriptArray**)gen->GetAddressOfReturnLocation() = ScriptArrayFactoryDefVal(ot, length, defVal);
}

static void ScriptArrayTemplateCallback_Generic(asIScriptGeneric *gen)
{
	asIObjectType *ot = *(asIObjectType**)gen->GetAddressOfArg(0);
	bool *dontGarbageCollect = *(bool**)gen->GetAddressOfArg(1);
	*(bool*)gen->GetAddressOfReturnLocation() = ScriptArrayTemplateCallback(ot, *dontGarbageCollect);
}

static void ScriptArrayAssignment_Generic(asIScriptGeneric *gen)
{
	CScriptArray *other = (CScriptArray*)gen->GetArgObject(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	*self = *other;
	gen->SetReturnObject(self);
}

static void ScriptArrayEquals_Generic(asIScriptGeneric *gen)
{
	CScriptArray *other = (CScriptArray*)gen->GetArgObject(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	gen->SetReturnByte(self->operator==(*other));
}

static void ScriptArrayFind_Generic(asIScriptGeneric *gen)
{
	void *value = gen->GetArgAddress(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	gen->SetReturnDWord(self->Find(value));
}

static void ScriptArrayFind2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	void *value = gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	gen->SetReturnDWord(self->Find(index, value));
}

static void ScriptArrayFindByRef_Generic(asIScriptGeneric *gen)
{
	void *value = gen->GetArgAddress(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	gen->SetReturnDWord(self->FindByRef(value));
}

static void ScriptArrayFindByRef2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	void *value = gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	gen->SetReturnDWord(self->FindByRef(index, value));
}

static void ScriptArrayAt_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();

	gen->SetReturnAddress(self->At(index));
}

static void ScriptArrayInsertAt_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	void *value = gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->InsertAt(index, value);
}

static void ScriptArrayRemoveAt_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->RemoveAt(index);
}

static void ScriptArrayInsertLast_Generic(asIScriptGeneric *gen)
{
	void *value = gen->GetArgAddress(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->InsertLast(value);
}

static void ScriptArrayRemoveLast_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->RemoveLast();
}

static void ScriptArrayLength_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();

	gen->SetReturnDWord(self->GetSize());
}

static void ScriptArrayResize_Generic(asIScriptGeneric *gen)
{
	asUINT size = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();

	self->Resize(size);
}

static void ScriptArrayReserve_Generic(asIScriptGeneric *gen)
{
	asUINT size = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->Reserve(size);
}

static void ScriptArraySortAsc_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->SortAsc();
}

static void ScriptArrayReverse_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->Reverse();
}

static void ScriptArrayIsEmpty_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->IsEmpty();
}

static void ScriptArraySortAsc2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	asUINT count = gen->GetArgDWord(1);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->SortAsc(index, count);
}

static void ScriptArraySortDesc_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->SortDesc();
}

static void ScriptArraySortDesc2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	asUINT count = gen->GetArgDWord(1);
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->SortDesc(index, count);
}

static void ScriptArrayAddRef_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->AddRef();
}

static void ScriptArrayRelease_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->Release();
}

static void ScriptArrayGetRefCount_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	*(int*)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void ScriptArraySetFlag_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	self->SetFlag();
}

static void ScriptArrayGetFlag_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	*(bool*)gen->GetAddressOfReturnLocation() = self->GetFlag();
}

static void ScriptArrayEnumReferences_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void ScriptArrayReleaseAllHandles_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObject();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

static void RegisterScriptArray_Generic(asIScriptEngine *engine)
{
	int r;

	engine->SetObjectTypeUserDataCleanupCallback(CleanupObjectTypeArrayCache, ARRAY_CACHE);

	r = engine->RegisterObjectType("array<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(ScriptArrayTemplateCallback_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in)", asFUNCTION(ScriptArrayFactory_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint)", asFUNCTION(ScriptArrayFactory2_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint, const T &in)", asFUNCTION(ScriptArrayFactoryDefVal_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_LIST_FACTORY, "array<T>@ f(int&in, int&in) {repeat T}", asFUNCTION(ScriptArrayListFactory_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptArrayAddRef_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptArrayRelease_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "T &opIndex(uint)", asFUNCTION(ScriptArrayAt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "const T &opIndex(uint) const", asFUNCTION(ScriptArrayAt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "array<T> &opAssign(const array<T>&in)", asFUNCTION(ScriptArrayAssignment_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("array<T>", "void insertAt(uint, const T&in)", asFUNCTION(ScriptArrayInsertAt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void removeAt(uint)", asFUNCTION(ScriptArrayRemoveAt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void insertLast(const T&in)", asFUNCTION(ScriptArrayInsertLast_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void removeLast()", asFUNCTION(ScriptArrayRemoveLast_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "uint length() const", asFUNCTION(ScriptArrayLength_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void reserve(uint)", asFUNCTION(ScriptArrayReserve_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void resize(uint)", asFUNCTION(ScriptArrayResize_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortAsc()", asFUNCTION(ScriptArraySortAsc_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortAsc(uint, uint)", asFUNCTION(ScriptArraySortAsc2_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortDesc()", asFUNCTION(ScriptArraySortDesc_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void sortDesc(uint, uint)", asFUNCTION(ScriptArraySortDesc2_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void reverse()", asFUNCTION(ScriptArrayReverse_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int find(const T&in) const", asFUNCTION(ScriptArrayFind_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int find(uint, const T&in) const", asFUNCTION(ScriptArrayFind2_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int findByRef(const T&in) const", asFUNCTION(ScriptArrayFindByRef_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "int findByRef(uint, const T&in) const", asFUNCTION(ScriptArrayFindByRef2_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "bool opEquals(const array<T>&in) const", asFUNCTION(ScriptArrayEquals_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "bool isEmpty() const", asFUNCTION(ScriptArrayIsEmpty_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "uint get_length() const", asFUNCTION(ScriptArrayLength_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("array<T>", "void set_length(uint)", asFUNCTION(ScriptArrayResize_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptArrayGetRefCount_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptArraySetFlag_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptArrayGetFlag_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptArrayEnumReferences_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptArrayReleaseAllHandles_Generic), asCALL_GENERIC); assert( r >= 0 );
}

END_AS_NAMESPACE
