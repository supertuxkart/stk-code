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

enum VECTOR_TYPE
{
    REF,
    HOLD
};

template<typename TYPE, VECTOR_TYPE type=HOLD>
class ptr_vector
{

public:
    std::vector<TYPE*> contentsVector;

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

// mark is a way to delete an object without changing the order of the element sin the vector
// it can be useful in a 'for' loop when the loop relies on object IDs and vector size not changing

void markToBeDeleted(const int ID) // object is removed from vector and deleted
{
    assert(ID > -1);
    assert((unsigned int)ID < contentsVector.size());

    delete ( TYPE *) contentsVector[ID];

    contentsVector[ID] = 0;

}
void markToBeRemoved(const int ID) // object is removed from vector but not deleted
{
    assert(ID > -1);
    assert((unsigned int)ID < contentsVector.size());

    contentsVector[ID] = 0;

}

bool isMarked(const int ID) const
{
    assert(ID > -1);
    assert((unsigned int)ID < contentsVector.size());

    return (contentsVector[ID] == 0);
}

void removeMarked()
{
    int size = contentsVector.size();
    for(int n=0; n<size; n++)
    {

        if( contentsVector[n] == 0 )
        {
            contentsVector.erase(contentsVector.begin()+n);
            size = contentsVector.size();
            n -= 2;
            if(n < -1) n=-1;
        }
    }//next

}

TYPE* get(const int ID)
{

    assert(ID > -1);
    assert((unsigned int)ID < contentsVector.size());

    return contentsVector[ID];
}

int size() const
{
    return contentsVector.size();
}

void erase(const int ID)
{
    assert(ID > -1);
    assert((unsigned int)ID < contentsVector.size());

    delete ( TYPE *) contentsVector[ID];

    contentsVector.erase(contentsVector.begin()+ID);
}

void remove(const int ID)
{
    assert(ID > -1);
    assert((unsigned int)ID < contentsVector.size());

    contentsVector.erase(contentsVector.begin()+ID);
}

bool contains( TYPE* instance ) const
{
    const unsigned int amount = contentsVector.size();
    for(unsigned int n=0; n<amount; n++)
    {
        TYPE * pointer = contentsVector[n];
        if(pointer == instance) return true;
    }
    
    return false;
}

void clearAndDeleteAll()
{
    for(unsigned int n=0; n<contentsVector.size(); n++)
    {
        TYPE * pointer = contentsVector[n];
        delete pointer;
    }
    contentsVector.clear();
}

TYPE& operator[](const unsigned int ID)
{
    assert((unsigned int)ID < contentsVector.size());

    return *(contentsVector[ID]);
}
const TYPE& operator[](const unsigned int ID) const
{
    assert((unsigned int)ID < contentsVector.size());

    return *(contentsVector[ID]);
}

void clearWithoutDeleting()
{
    contentsVector.clear();
}

void remove(TYPE* obj)
{
    for(unsigned int n=0; n<contentsVector.size(); n++)
    {

        TYPE * pointer = contentsVector[n];
        if(pointer == obj)
        {
            contentsVector.erase(contentsVector.begin()+n);
            return;
        }
    }


}

};


#endif
