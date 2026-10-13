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

#ifndef AS_ARRAY_H
#define AS_ARRAY_H

#if !defined(AS_NO_MEMORY_H)
#include <memory.h>
#endif
#include <string.h> // some compilers declare memcpy() here

#ifdef _MSC_VER
#pragma warning(disable:4345) // warning about a change in how the code is handled in this version
#endif

BEGIN_AS_NAMESPACE

template <class T> class asCArray
{
public:
	asCArray();
	asCArray(const asCArray<T> &);
	explicit asCArray(asUINT reserve);
	~asCArray();

	void   Allocate(asUINT numElements, bool keepData);
	void   AllocateNoConstruct(asUINT numElements, bool keepData);
	inline asUINT GetCapacity() const;

	void PushLast(const T &element);
	inline T    PopLast();

	bool   SetLength(asUINT numElements);
	bool   SetLengthNoConstruct(asUINT numElements);
	inline void   SetLengthNoAllocate(asUINT numElements);
	inline asUINT GetLength() const;

	void         Copy(const T*, asUINT count);
	asCArray<T> &operator =(const asCArray<T> &);
	void         SwapWith(asCArray<T> &other);

	inline const T &operator [](asUINT index) const;
	inline T       &operator [](asUINT index);
	inline T       *AddressOf();
	inline const T *AddressOf() const;

	bool Concatenate(const asCArray<T> &);
	void Concatenate(T*, unsigned int count);

	bool Exists(const T &element) const;
	int  IndexOf(const T &element) const;
	void RemoveIndex(asUINT index);          // Removes the entry without reordering the array
	void RemoveValue(const T &element);      // Removes the value without reordering the array
	void RemoveIndexUnordered(asUINT index); // Removes the entry without keeping the order

	bool operator==(const asCArray<T> &) const;
	bool operator!=(const asCArray<T> &) const;

public:
	// These are public to allow external code (e.g. JIT compiler) to do asOFFSET to 
	// access the members directly without having to modify the code to add friend
	T      *array;
	asUINT  length;                  // 32bits is enough for all uses of this array
	asUINT  maxLength;

protected:
	char    buf[2*4*AS_PTR_SIZE];    // Avoid dynamically allocated memory for tiny arrays
};

// Implementation

template <class T>
inline T *asCArray<T>::AddressOf()
{
	return array;
}

template <class T>
inline const T *asCArray<T>::AddressOf() const
{
	return array;
}

template <class T>
asCArray<T>::asCArray(void) : buf()
{
	array     = 0;
	length    = 0;
	maxLength = 0;
}

template <class T>
asCArray<T>::asCArray(const asCArray<T> &copy) : buf()
{
	array     = 0;
	length    = 0;
	maxLength = 0;

	*this = copy;
}

template <class T>
asCArray<T>::asCArray(asUINT reserve) : buf()
{
	array     = 0;
	length    = 0;
	maxLength = 0;

	Allocate(reserve, false);
}

template <class T>
asCArray<T>::~asCArray(void)
{
	// Allocating a zero length array will free all memory
	Allocate(0,0);
}

template <class T>
inline asUINT asCArray<T>::GetLength() const
{
	return length;
}

template <class T>
inline const T &asCArray<T>::operator [](asUINT index) const
{
	asASSERT(index < length);

	return array[index];
}

template <class T>
inline T &asCArray<T>::operator [](asUINT index)
{
	asASSERT(index < length);

	return array[index];
}

template <class T>
void asCArray<T>::PushLast(const T &element)
{
	if( length == maxLength )
	{
		if( maxLength == 0 )
			Allocate(1, false);
		else
			Allocate(2*maxLength, true);

		if( length == maxLength )
		{
			// Out of memory. Return without doing anything
			return;
		}
	}

	array[length++] = element;
}

template <class T>
inline T asCArray<T>::PopLast()
{
	asASSERT(length > 0);

	return array[--length];
}

template <class T>
void asCArray<T>::Allocate(asUINT numElements, bool keepData)
{
	// We have 4 situations
	// 1. The previous array is 8 bytes or smaller and the new array is also 8 bytes or smaller
	// 2. The previous array is 8 bytes or smaller and the new array is larger than 8 bytes
	// 3. The previous array is larger than 8 bytes and the new array is 8 bytes or smaller
	// 4. The previous array is larger than 8 bytes and the new array is also larger than 8 bytes

	T *tmp = 0;
	if( numElements )
	{
		if( sizeof(T)*numElements <= sizeof(buf) )
			// Use the internal buffer
			tmp = reinterpret_cast<T*>(buf);
		else
		{
			// Allocate the array and construct each of the elements
			tmp = asNEWARRAY(T,numElements);
			if( tmp == 0 )
			{
				// Out of memory. Return without doing anything
				return;
			}
		}

		if( array == tmp )
		{
			// Construct only the newly allocated elements
			for( asUINT n = length; n < numElements; n++ )
				new (&tmp[n]) T();
		}
		else
		{
			// Construct all elements
			for( asUINT n = 0; n < numElements; n++ )
				new (&tmp[n]) T();
		}
	}

	if( array )
	{	
		asUINT oldLength = length;

		if( array == tmp )
		{
			if( keepData )
			{
				if( length > numElements )
					length = numElements;
			}
			else
				length = 0;

			// Call the destructor for elements that are no longer used
			for( asUINT n = length; n < oldLength; n++ )
				array[n].~T();
		}
		else
		{
			if( keepData )
			{
				if( length > numElements )
					length = numElements;

				for( asUINT n = 0; n < length; n++ )
					tmp[n] = array[n];
			}
			else
				length = 0;

			// Call the destructor for all elements
			for( asUINT n = 0; n < oldLength; n++ )
				array[n].~T();

			if( array != reinterpret_cast<T*>(buf) )
				asDELETEARRAY(array);
		}
	}

	array = tmp;
	maxLength = numElements;
}

template <class T>
void asCArray<T>::AllocateNoConstruct(asUINT numElements, bool keepData)
{
	// We have 4 situations
	// 1. The previous array is 8 bytes or smaller and the new array is also 8 bytes or smaller
	// 2. The previous array is 8 bytes or smaller and the new array is larger than 8 bytes
	// 3. The previous array is larger than 8 bytes and the new array is 8 bytes or smaller
	// 4. The previous array is larger than 8 bytes and the new array is also larger than 8 bytes

	T *tmp = 0;
	if( numElements )
	{
		if( sizeof(T)*numElements <= sizeof(buf) )
			// Use the internal buffer
			tmp = reinterpret_cast<T*>(buf);
		else
		{
			// Allocate the array and construct each of the elements
			tmp = asNEWARRAY(T,numElements);
			if( tmp == 0 )
			{
				// Out of memory. Return without doing anything
				return;
			}
		}
	}

	if( array )
	{
		if( array == tmp )
		{
			if( keepData )
			{
				if( length > numElements )
					length = numElements;
			}
			else
				length = 0;
		}
		else
		{
			if( keepData )
			{
				if( length > numElements )
					length = numElements;

				memcpy(tmp, array, sizeof(T)*length);
			}
			else
				length = 0;

			if( array != reinterpret_cast<T*>(buf) )
				asDELETEARRAY(array);
		}
	}

	array = tmp;
	maxLength = numElements;
}

template <class T>
inline asUINT asCArray<T>::GetCapacity() const
{
	return maxLength;
}

template <class T>
inline void asCArray<T>::SetLengthNoAllocate(asUINT numElements)
{
	asASSERT(numElements <= maxLength);
	length = numElements;
}

template <class T>
bool asCArray<T>::SetLength(asUINT numElements)
{
	if( numElements > maxLength )
	{
		Allocate(numElements, true);
		if( numElements > maxLength )
		{
			// Out of memory. Return without doing anything
			return false;
		}
	}

	length = numElements;
	return true;
}

template <class T>
bool asCArray<T>::SetLengthNoConstruct(asUINT numElements)
{
	if( numElements > maxLength )
	{
		AllocateNoConstruct(numElements, true);
		if( numElements > maxLength )
		{
			// Out of memory. Return without doing anything
			return false;
		}
	}

	length = numElements;
	return true;
}

template <class T>
void asCArray<T>::Copy(const T *data, asUINT count)
{
	if( maxLength < count )
	{
		Allocate(count, false);
		if( maxLength < count )
		{
			// Out of memory. Return without doing anything
			return;
		}
	}

	for( asUINT n = 0; n < count; n++ )
		array[n] = data[n];

	length = count;
}

template <class T>
asCArray<T> &asCArray<T>::operator =(const asCArray<T> &copy)
{
	Copy(copy.array, copy.length);

	return *this;
}

template <class T>
void asCArray<T>::SwapWith(asCArray<T> &other)
{
	T      *tmpArray = array;
	asUINT  tmpLength = length;
	asUINT  tmpMaxLength = maxLength;
	char    tmpBuf[sizeof(buf)];
	memcpy(tmpBuf, buf, sizeof(buf));

	array = other.array;
	length = other.length;
	maxLength = other.maxLength;
	memcpy(buf, other.buf, sizeof(buf));

	other.array = tmpArray;
	other.length = tmpLength;
	other.maxLength = tmpMaxLength;
	memcpy(other.buf, tmpBuf, sizeof(buf));

	// If the data is in the internal buffer, then the array pointer must refer to it
	if( array == reinterpret_cast<T*>(other.buf) )
		array = reinterpret_cast<T*>(buf);
	if( other.array == reinterpret_cast<T*>(buf) )
		other.array = reinterpret_cast<T*>(other.buf);
}

template <class T>
bool asCArray<T>::operator ==(const asCArray<T> &other) const
{
	if( length != other.length ) return false;

	for( asUINT n = 0; n < length; n++ )
		if( array[n] != other.array[n] )
			return false;

	return true;
}

template <class T>
bool asCArray<T>::operator !=(const asCArray<T> &other) const
{
	return !(*this == other);
}


// Returns false if the concatenation wasn't successful due to out of memory
template <class T>
bool asCArray<T>::Concatenate(const asCArray<T> &other)
{
	if( maxLength < length + other.length )
	{
		Allocate(length + other.length, true);
		if( maxLength < length + other.length )
		{
			// Out of memory
			return false;
		}
	}

	for( asUINT n = 0; n < other.length; n++ )
		array[length+n] = other.array[n];

	length += other.length;

	// Success
	return true;
}

template <class T>
void asCArray<T>::Concatenate(T* other, unsigned int count)
{
	for( unsigned int c = 0; c < count; c++ )
		PushLast(other[c]);
}

template <class T>
bool asCArray<T>::Exists(const T &e) const
{
	return IndexOf(e) == -1 ? false : true;
}

template <class T>
int asCArray<T>::IndexOf(const T &e) const
{
	for( asUINT n = 0; n < length; n++ )
		if( array[n] == e ) return static_cast<int>(n);

	return -1;
}

template <class T>
void asCArray<T>::RemoveIndex(asUINT index)
{
	if( index < length )
	{
		for( asUINT n = index; n < length-1; n++ )
			array[n] = array[n+1];

		PopLast();
	}
}

template <class T>
void asCArray<T>::RemoveValue(const T &e)
{
	for( asUINT n = 0; n < length; n++ )
	{
		if( array[n] == e )
		{
			RemoveIndex(n);
			break;
		}
	}
}

template <class T>
void asCArray<T>::RemoveIndexUnordered(asUINT index)
{
	if( index == length - 1 )
		PopLast();
	else if( index < length )
		array[index] = PopLast();
}

END_AS_NAMESPACE

#endif
