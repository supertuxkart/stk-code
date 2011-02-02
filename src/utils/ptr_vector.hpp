/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * I made this class to work like a regular vector, except that contentsVector are placed
 * one the heap so third-party contentsVector can keep pointers to them.
 */

#ifndef HEADER_PTR_VECTOR_HPP
#define HEADER_PTR_VECTOR_HPP

#include <vector>
#include <iostream>
#include <assert.h>

#include "utils/aligned_array.hpp"

enum VECTOR_TYPE
{
    REF,
    HOLD
};

template<typename TYPE, VECTOR_TYPE type=HOLD>
class ptr_vector
{

public:
    AlignedArray<TYPE*> contentsVector;

ptr_vector()
{
}

~ptr_vector()
{
    if(type == HOLD) clearAndDeleteAll();
}

void push_back(TYPE* t)
{
    contentsVector.push_back(t);
}

void add(TYPE* t, int index)
{
    contentsVector.insert(contentsVector.begin()+index, t);
}

void swap(int ID1, int ID2)
{
    assert(ID1 > -1);
    assert((unsigned int)ID1 < contentsVector.size());
    assert(ID2 > -1);
    assert((unsigned int)ID2 < contentsVector.size());


    TYPE* temp = contentsVector[ID2];

    contentsVector[ID2] = contentsVector[ID1];
    contentsVector[ID1] = temp;
}

TYPE* get(const int ID)
{

    assert(ID > -1);
    assert((unsigned int)ID < (unsigned int)contentsVector.size());

    return contentsVector[ID];
}

const TYPE* getConst(const int ID) const
{
    
    assert(ID > -1);
    assert((unsigned int)ID < (unsigned int)contentsVector.size());
    
    return contentsVector[ID];
}

int size() const
{
    return contentsVector.size();
}

void erase(const int ID)
{
    assert(ID > -1);
    assert((unsigned int)ID < (unsigned int)contentsVector.size());

    delete ( TYPE *) contentsVector[ID];
#ifdef USE_ALIGNED
    const unsigned int amount = (unsigned int)contentsVector.size();
    for(unsigned int i=ID; i<amount-1; i++)
    {
        contentsVector[i]=contentsVector[i+1];
    }
    contentsVector.pop_back();
#else
    contentsVector.erase(contentsVector.begin()+ID);
#endif
}

TYPE* remove(const int ID)
{
    assert(ID > -1);
    assert((unsigned int)ID < (unsigned int)contentsVector.size());

    TYPE* out = contentsVector[ID];
#ifdef USE_ALIGNED
    const unsigned int amount = (unsigned int)contentsVector.size();
    for(unsigned int i=ID; i<amount-1; i++)
    {
        contentsVector[i]=contentsVector[i+1];
    }
    contentsVector.pop_back();
#else
    contentsVector.erase(contentsVector.begin()+ID);
#endif
    return out;
}

bool contains( const TYPE* instance ) const
{
    const unsigned int amount = (unsigned int)contentsVector.size();
    for (unsigned int n=0; n<amount; n++)
    {
        const TYPE * pointer = contentsVector[n];
        if (pointer == instance) return true;
    }
    
    return false;
}

void clearAndDeleteAll()
{
    for (unsigned int n=0; n<(unsigned int)contentsVector.size(); n++)
    {
        TYPE * pointer = contentsVector[n];
        delete pointer;
        contentsVector[n] = (TYPE*)0xDEADBEEF;
        
        // When deleting, it's important that the same pointer cannot be
        // twice in the vector, resulting in a double delete
        assert( !contains(pointer) );
    }
    contentsVector.clear();
}

TYPE& operator[](const unsigned int ID)
{
    assert((unsigned int)ID < (unsigned int)contentsVector.size());

    return *(contentsVector[ID]);
}
const TYPE& operator[](const unsigned int ID) const
{
    assert((unsigned int)ID < (unsigned int)contentsVector.size());

    return *(contentsVector[ID]);
}

void clearWithoutDeleting()
{
    contentsVector.clear();
}

/**
  * Removes without deleting
  */
void remove(TYPE* obj)
{
    for(unsigned int n=0; n<(unsigned int)contentsVector.size(); n++)
    {

        TYPE * pointer = contentsVector[n];
        if(pointer == obj)
        {
#ifdef USE_ALIGNED
            const unsigned int amount = (unsigned int)contentsVector.size();
            for(unsigned int i=n; i<amount-1; i++)
            {
                contentsVector[i]=contentsVector[i+1];
            }
            contentsVector.pop_back();
#else
            contentsVector.erase(contentsVector.begin()+n);
#endif
            return;
        }
    }

}

/**
  * \brief Removes and deletes the given object.
  * \return whether this object was found in the vector and deleted
  */
bool erase(void* obj)
{
    for(unsigned int n=0; n<(unsigned int)contentsVector.size(); n++)
    {
        TYPE * pointer = contentsVector[n];
        if((void*)pointer == obj)
        {
#ifdef USE_ALIGNED
             const unsigned int amount = (unsigned int)contentsVector.size();
             for(unsigned int i=n; i<amount-1; i++)
             {
                 contentsVector[i]=contentsVector[i+1];
             }
             contentsVector.pop_back();
#else
            contentsVector.erase(contentsVector.begin()+n);
#endif
            delete pointer;
            return true;
        }
    }
    return false;
}

    void insertionSort(unsigned int start=0)
    {
        for(unsigned int j=start; j<(unsigned)contentsVector.size()-1; j++)
        {
            if(*(contentsVector[j])<*(contentsVector[j+1])) continue;
            // Now search the proper place for contentsVector[j+1] 
            // in the sorted section contentsVectot[start:j]
            TYPE* t=contentsVector[j+1];
            unsigned int i = j+1;
            do
            {
                contentsVector[i] = contentsVector[i-1];
                i--;
            } while (i>0 && *t<*(contentsVector[i-1]));
            contentsVector[i]=t;
        }
    }   // insertionSort


};


#endif
