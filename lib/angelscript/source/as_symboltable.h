/*
   AngelCode Scripting Library
   Copyright (c) 2012-2021 Andreas Jonsson

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
// as_symboltable.h
//
//  Created on: Jun 19, 2012
//      Author: Markus Lenger, a.k.a. mlengerx
//
// This class is used for fast symbol lookups while parsing or loading bytecode
//

#ifndef AS_SYMBOLTABLE_H
#define AS_SYMBOLTABLE_H

#include "as_config.h"
#include "as_memory.h"
#include "as_string.h"
#include "as_map.h"
#include "as_datatype.h"
#include "as_namespace.h"


BEGIN_AS_NAMESPACE





// Interface to avoid nested templates which is not well supported by older compilers, e.g. MSVC6
struct asIFilter
{
	virtual bool operator()(const void*) const = 0;
	virtual ~asIFilter() {};
};




// forward declaration
template<class T>
class asCSymbolTable;




// Iterator that allows iterating in index order
template<class T, class T2 = T>
class asCSymbolTableIterator
{
public:
	T2* operator*() const;
	T2* operator->() const;
	asCSymbolTableIterator<T, T2>& operator++(int);
	asCSymbolTableIterator<T, T2>& operator--(int);
	operator bool() const;
	int GetIndex() const { return m_idx; }

private:
	friend class asCSymbolTable<T>;
	asCSymbolTableIterator(asCSymbolTable<T> *table);

	void Next();
	void Previous();

	asCSymbolTable<T>* m_table;
	unsigned int       m_idx;
};




// Symbol table mapping namespace + name to symbols
// The structure keeps the entries indexed in an array so the indices will not change
// There is also a map for a quick lookup. The map supports multiple entries with the same name
template<class T>
class asCSymbolTable
{
public:
	typedef asCSymbolTableIterator<T, T> iterator;
	typedef asCSymbolTableIterator<T, const T> const_iterator;

	asCSymbolTable(asUINT initialCapacity = 0);

	int      GetFirstIndex(const asSNameSpace *ns, const asCString &name, const asIFilter &comparator) const;
	int      GetFirstIndex(const asSNameSpace *ns, const asCString &name) const;
	int      GetLastIndex() const;

	int      GetIndex(const T*) const;

	T*       GetFirst(const asSNameSpace *ns, const asCString &name, const asIFilter &comparator) const;
	T*       GetFirst(const asSNameSpace *ns, const asCString &name);
	const T* GetFirst(const asSNameSpace *ns, const asCString &name) const;
	T*       Get(asUINT index);
	const T* Get(asUINT index) const;
	T*       GetLast();
	const T* GetLast() const;

	const asCArray<asUINT> &GetIndexes(const asSNameSpace *ns, const asCString &name) const;

	asUINT   Put(T* entry);

	asUINT   GetSize() const;

	void SwapWith(asCSymbolTable<T> &other);

	void Clear();
	bool Erase(asUINT idx);
	void Allocate(asUINT elem_cnt, bool keep_data);

	iterator List();
	const_iterator List() const;

private:
	// Don't allow assignment
	asCSymbolTable<T>& operator=(const asCSymbolTable<T> &other) { return *this; }

	friend class asCSymbolTableIterator<T, T>;
	friend class asCSymbolTableIterator<T, const T>;

	void GetKey(const T *entry, asSNameSpaceNamePair &key) const;
	bool CheckIdx(asUINT idx) const;

	asCMap<asSNameSpaceNamePair, asCArray<asUINT> > m_map;
	asCArray<T*>                                    m_entries;
	unsigned int                                    m_size;
};




template<class T>
void asCSymbolTable<T>::SwapWith(asCSymbolTable<T> &other)
{
	m_map.SwapWith(other.m_map);
	m_entries.SwapWith(other.m_entries);

	asUINT tmp = m_size;
	m_size = other.m_size;
	other.m_size = tmp;
}




// Constructor
// initialCapacity gives the number of entries to allocate in advance
template<class T>
asCSymbolTable<T>::asCSymbolTable(asUINT initialCapacity) : m_entries(initialCapacity)
{
	m_size = 0;
}



template<class T>
int asCSymbolTable<T>::GetFirstIndex(
        const asSNameSpace *ns,
        const asCString &name,
        const asIFilter &filter) const
{
	asSNameSpaceNamePair key(ns, name);

	asSMapNode<asSNameSpaceNamePair, asCArray<asUINT> > *cursor;
	if( m_map.MoveTo(&cursor, key) )
	{
		const asCArray<asUINT> &arr = m_map.GetValue(cursor);
		for( asUINT n = 0; n < arr.GetLength(); n++ )
		{
			T *entry = m_entries[arr[n]];
			if( entry && filter(entry) )
				return arr[n];
		}
	}

	return -1;
}



template<class T>
const asCArray<asUINT> &asCSymbolTable<T>::GetIndexes(const asSNameSpace *ns, const asCString &name) const
{
	asSNameSpaceNamePair key(ns, name);

	asSMapNode<asSNameSpaceNamePair, asCArray<asUINT> > *cursor;
	if( m_map.MoveTo(&cursor, key) )
		return m_map.GetValue(cursor);

	static asCArray<asUINT> dummy;
	return dummy;
}




template<class T>
T* asCSymbolTable<T>::GetFirst(const asSNameSpace *ns, const asCString &name, const asIFilter &comp) const
{
	int idx = GetFirstIndex(ns, name, comp);
	if (idx != -1) return m_entries[idx];
	return 0;
}




template<class T>
int asCSymbolTable<T>::GetFirstIndex(const asSNameSpace *ns, const asCString &name) const
{
	asSNameSpaceNamePair key(ns, name);

	asSMapNode<asSNameSpaceNamePair, asCArray<asUINT> > *cursor;
	if( m_map.MoveTo(&cursor, key) )
		return m_map.GetValue(cursor)[0];

	return -1;
}




// Find the index of a certain symbol
// ATTENTION: this function has linear runtime complexity O(n)!!
template<class T>
int asCSymbolTable<T>::GetIndex(const T* entry) const
{
	for( asUINT n = 0; n < m_entries.GetLength(); n++ )
		if( m_entries[n] == entry )
			return n;

	return -1;
}






template<class T>
T* asCSymbolTable<T>::Get(asUINT idx)
{
	if( !CheckIdx(idx) )
		return 0;

	return m_entries[idx];
}

template<class T>
const T* asCSymbolTable<T>::Get(asUINT idx) const
{
	return const_cast< asCSymbolTable<T>* >(this)->Get(idx);
}





template<class T>
T* asCSymbolTable<T>::GetFirst(const asSNameSpace *ns, const asCString &name)
{
	int idx = GetFirstIndex(ns, name);
	return Get(idx);
}

template<class T>
const T* asCSymbolTable<T>::GetFirst(const asSNameSpace *ns, const asCString &name) const
{
	return const_cast< asCSymbolTable<T>* >(this)->GetFirst(ns, name);
}





template<class T>
T* asCSymbolTable<T>::GetLast()
{
	return Get(GetLastIndex());
}

template<class T>
const T* asCSymbolTable<T>::GetLast() const
{
	return const_cast< asCSymbolTable<T>* >(this)->GetLast();
}





// Clear the symbol table
// ATTENTION: The contained symbols are not rleased. This is up to the client
template<class T>
void asCSymbolTable<T>::Clear()
{
	m_entries.SetLength(0);
	m_map.EraseAll();
	m_size = 0;
}




// Pre-allocate slots for elemCnt entries
template<class T>
void asCSymbolTable<T>::Allocate(asUINT elemCnt, bool keepData)
{
	asASSERT( elemCnt >= m_entries.GetLength() );
	m_entries.Allocate(elemCnt, keepData);
	if( !keepData )
		m_map.EraseAll();
}



template<class T>
bool asCSymbolTable<T>::Erase(asUINT idx)
{
	if( !CheckIdx(idx) )
	{
		asASSERT(false);
		return false;
	}

	T *entry = m_entries[idx];
	asASSERT(entry);
	if( !entry )
		return false;

	// Remove the symbol from the lookup map
	asSNameSpaceNamePair key;
	GetKey(entry, key);

	asSMapNode<asSNameSpaceNamePair, asCArray<asUINT> > *cursor;
	if( m_map.MoveTo(&cursor, key) )
	{
		asCArray<asUINT> &arr = m_map.GetValue(cursor);
		arr.RemoveValue(idx);
		if( arr.GetLength() == 0 )
			m_map.Erase(cursor);
	}
	else
		asASSERT(false);

	// Remove the symbol from the indexed array
	if( idx == m_entries.GetLength() - 1 )
		m_entries.PopLast();
	else
	{
		// Must keep the array packed
		int prevIdx = int(m_entries.GetLength()-1);
		m_entries[idx] = m_entries.PopLast();
		
		// Update the index in the lookup map
		entry = m_entries[idx];
		GetKey(entry, key);
		if( m_map.MoveTo(&cursor, key) )
		{
			asCArray<asUINT> &arr = m_map.GetValue(cursor);
			arr[arr.IndexOf(prevIdx)] = idx;
		}
		else
			asASSERT(false);
	}
	m_size--;

	return true;
}




template<class T>
asUINT asCSymbolTable<T>::Put(T *entry)
{
	asUINT idx = m_entries.GetLength();
	asSNameSpaceNamePair key;
	GetKey(entry, key);

	asSMapNode<asSNameSpaceNamePair, asCArray<asUINT> > *cursor;
	if( m_map.MoveTo(&cursor, key) )
		m_map.GetValue(cursor).PushLast(idx);
	else
	{
		asCArray<asUINT> arr(1);
		arr.PushLast(idx);
		m_map.Insert(key, arr);
	}

	m_entries.PushLast(entry);
	m_size++;
	return idx;
}




// Return key for specified symbol (namespace and name are used to generate the key)
template<class T>
void asCSymbolTable<T>::GetKey(const T *entry, asSNameSpaceNamePair &key) const
{
	key = asSNameSpaceNamePair(entry->nameSpace, entry->name);
}




template<class T>
asUINT asCSymbolTable<T>::GetSize() const
{
	return m_size;
}




template<class T>
bool asCSymbolTable<T>::CheckIdx(asUINT idx) const
{
	return idx < m_entries.GetLength();
}




template<class T>
int asCSymbolTable<T>::GetLastIndex() const
{
	int idx = int(m_entries.GetLength()) - 1;
	asASSERT( idx == -1 || m_entries[idx] );
	return idx;
}




template<class T>
asCSymbolTableIterator<T, T> asCSymbolTable<T>::List()
{
	return asCSymbolTableIterator<T, T>(this);
}




template<class T>
typename asCSymbolTable<T>::const_iterator asCSymbolTable<T>::List() const
{
	return asCSymbolTableIterator<T, const T>(const_cast< asCSymbolTable<T> *>(this));
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Iterator


template<class T, class T2>
asCSymbolTableIterator<T, T2>::asCSymbolTableIterator(asCSymbolTable<T> *table) : m_table(table), m_idx(0)
{
	asUINT sz = m_table->m_entries.GetLength();
	while( m_idx < sz && m_table->m_entries[m_idx] == 0 )
		m_idx++;
}



template<class T, class T2>
T2* asCSymbolTableIterator<T, T2>::operator*() const
{
	asASSERT(m_table->CheckIdx(m_idx));
	return m_table->m_entries[m_idx];
}



template<class T, class T2>
T2* asCSymbolTableIterator<T, T2>::operator->() const
{
	asASSERT(m_table->CheckIdx(m_idx));
	return m_table->m_entries[m_idx];
}



template<class T, class T2>
asCSymbolTableIterator<T, T2>& asCSymbolTableIterator<T, T2>::operator++(int)
{
	Next();
	return *this;
}



// Return true if more elements are following
// ATTENTION: When deleting the object currently pointed to by this iterator this
// method returns false even though there might be more elements in the list
template<class T, class T2>
asCSymbolTableIterator<T, T2>::operator bool() const
{
	return m_idx < m_table->m_entries.GetLength() && m_table->m_entries[m_idx] != 0;
}



template<class T, class T2>
void asCSymbolTableIterator<T, T2>::Next()
{
	asUINT sz = m_table->m_entries.GetLength();
	m_idx++;
	while( m_idx < sz && m_table->m_entries[m_idx] == 0 )
		m_idx++;
}



template<class T, class T2>
void asCSymbolTableIterator<T, T2>::Previous()
{
	// overflow on stepping over first element
	asUINT sz = m_table->m_entries.GetLength();
	m_idx--;
	while( m_idx < sz && m_table->m_entries[m_idx] == 0 )
		m_idx--;
}



template<class T, class T2>
asCSymbolTableIterator<T, T2>& asCSymbolTableIterator<T, T2>::operator--(int)
{
	Previous();
	return *this;
}


END_AS_NAMESPACE

#endif // AS_SYMBOLTABLE_H
