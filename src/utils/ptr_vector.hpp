//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2009-2013 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


/*
 *  I made this class to work like a regular vector, except that
 *  m_contents_vector are placed* one the heap so third-party
 *  m_contents_vector can keep pointers to them.
 */

#ifndef HEADER_PtrVector_HPP
#define HEADER_PtrVector_HPP

#include <vector>
#include <assert.h>

#include "utils/aligned_array.hpp"

enum VECTOR_TYPE
{
    REF,
    HOLD
};


template<typename TYPE, VECTOR_TYPE type=HOLD>
class PtrVector
{

public:
    AlignedArray<TYPE*> m_contents_vector;

    PtrVector()
    {
    }   // PtrVector

    // ------------------------------------------------------------------------

    ~PtrVector()
    {
        if(type == HOLD) clearAndDeleteAll();
    }   // ~PtrVector

    // ------------------------------------------------------------------------

    void push_back(TYPE* t)
    {
        m_contents_vector.push_back(t);
    }   // push_back

    // ------------------------------------------------------------------------
    void swap(int ID1, int ID2)
    {
        assert(ID1 > -1);
        assert((unsigned int)ID1 < m_contents_vector.size());
        assert(ID2 > -1);
        assert((unsigned int)ID2 < m_contents_vector.size());


        TYPE* temp = m_contents_vector[ID2];

        m_contents_vector[ID2] = m_contents_vector[ID1];
        m_contents_vector[ID1] = temp;
    }   // swap

    // ------------------------------------------------------------------------
    TYPE* get(const int ID)
    {
        assert(ID > -1);
        assert((unsigned int)ID < (unsigned int)m_contents_vector.size());

        return m_contents_vector[ID];
    }   // get

    // ------------------------------------------------------------------------
    const TYPE* get(const int ID) const
    {
        assert(ID > -1);
        assert((unsigned int)ID < (unsigned int)m_contents_vector.size());

        return m_contents_vector[ID];
    }   // get

    // ------------------------------------------------------------------------
    unsigned int size() const
    {
        return (int) m_contents_vector.size();
    }   // size

    // ------------------------------------------------------------------------
    void erase(const int ID)
    {
        assert(ID > -1);
        assert((unsigned int)ID < (unsigned int)m_contents_vector.size());

        delete ( TYPE *) m_contents_vector[ID];
#ifdef USE_ALIGNED
        const unsigned int amount = (unsigned int)m_contents_vector.size();
        for(unsigned int i=ID; i<amount-1; i++)
        {
            m_contents_vector[i]=m_contents_vector[i+1];
        }
        m_contents_vector.pop_back();
#else
        m_contents_vector.erase(m_contents_vector.begin()+ID);
#endif
    }   // erase

    // ------------------------------------------------------------------------
    TYPE* remove(const int ID)
    {
        assert(ID > -1);
        assert((unsigned int)ID < (unsigned int)m_contents_vector.size());

        TYPE* out = m_contents_vector[ID];
#ifdef USE_ALIGNED
        const unsigned int amount = (unsigned int)m_contents_vector.size();
        for(unsigned int i=ID; i<amount-1; i++)
        {
            m_contents_vector[i]=m_contents_vector[i+1];
        }
        m_contents_vector.pop_back();
#else
        m_contents_vector.erase(m_contents_vector.begin()+ID);
#endif
        return out;
    }   // remove

    // ------------------------------------------------------------------------
    bool contains( const TYPE* instance ) const
    {
        const unsigned int amount = (unsigned int)m_contents_vector.size();
        for (unsigned int n=0; n<amount; n++)
        {
            const TYPE * pointer = m_contents_vector[n];
            if (pointer == instance) return true;
        }

        return false;
    }   // contains

    // ------------------------------------------------------------------------
    void clearAndDeleteAll()
    {
        for (unsigned int n=0; n<(unsigned int)m_contents_vector.size(); n++)
        {
            TYPE * pointer = m_contents_vector[n];
            delete pointer;
            m_contents_vector[n] = (TYPE*)0xDEADBEEF;

            // When deleting, it's important that the same pointer cannot be
            // twice in the vector, resulting in a double delete
            assert( !contains(pointer) );
        }
        m_contents_vector.clear();
    }  // clearAndDeleteAll

    // ------------------------------------------------------------------------
    TYPE& operator[](const unsigned int ID)
    {
        assert((unsigned int)ID < (unsigned int)m_contents_vector.size());

        return *(m_contents_vector[ID]);
    }   // operator[]

    // ------------------------------------------------------------------------
    const TYPE& operator[](const unsigned int ID) const
    {
        assert((unsigned int)ID < (unsigned int)m_contents_vector.size());

        return *(m_contents_vector[ID]);
    }   // operator[]

    // ------------------------------------------------------------------------
    void clearWithoutDeleting()
    {
        m_contents_vector.clear();
    }   // clearWithoutDeleting

    // ------------------------------------------------------------------------
    /**
    * Removes without deleting
    */
    void remove(TYPE* obj)
    {
        for(unsigned int n=0; n<(unsigned int)m_contents_vector.size(); n++)
        {

            TYPE * pointer = m_contents_vector[n];
            if(pointer == obj)
            {
#ifdef USE_ALIGNED
                const unsigned int amount =
                    (unsigned int)m_contents_vector.size();
                for(unsigned int i=n; i<amount-1; i++)
                {
                    m_contents_vector[i]=m_contents_vector[i+1];
                }
                m_contents_vector.pop_back();
#else
                m_contents_vector.erase(m_contents_vector.begin()+n);
#endif
                return;
            }
        }   // for n < size()

    }   // remove

    // ------------------------------------------------------------------------
    /**
    * \brief Removes and deletes the given object.
    * \return whether this object was found in the vector and deleted
    */
    bool erase(void* obj)
    {
        for(unsigned int n=0; n<(unsigned int)m_contents_vector.size(); n++)
        {
            TYPE * pointer = m_contents_vector[n];
            if((void*)pointer == obj)
            {
#ifdef USE_ALIGNED
                const unsigned int amount =
                    (unsigned int)m_contents_vector.size();
                for(unsigned int i=n; i<amount-1; i++)
                {
                    m_contents_vector[i]=m_contents_vector[i+1];
                }
                m_contents_vector.pop_back();
#else
                m_contents_vector.erase(m_contents_vector.begin()+n);
#endif
                delete pointer;
                return true;
            }
        }   // for n < size()
        return false;
    }   // erase

    // ------------------------------------------------------------------------
    void insertionSort(unsigned int start=0, bool desc = false)
    {
        if (!desc)
        {
            // We should not used unsigned ints here, because if the vector is
            // empty j needs to be compared against -1
            for(int j=(int)start; j<(int)m_contents_vector.size()-1; j++)
            {
                if(*(m_contents_vector[j])<*(m_contents_vector[j+1])) continue;
                // Now search the proper place for m_contents_vector[j+1]
                // in the sorted section contentsVectot[start:j]
                TYPE* t=m_contents_vector[j+1];
                unsigned int i = j+1;
                do
                {
                    m_contents_vector[i] = m_contents_vector[i-1];
                    i--;
                } while (i>start && *t<*(m_contents_vector[i-1]));
                m_contents_vector[i]=t;
            }
        }
        else
        {
            for(int j=(int)start; j<(int)m_contents_vector.size()-1; j++)
             {
                 if(*(m_contents_vector[j+1])<*(m_contents_vector[j])) continue;
                 // Now search the proper place for m_contents_vector[j+1]
                 // in the sorted section contentsVectot[start:j]
                 TYPE* t=m_contents_vector[j+1];
                 unsigned int i = j+1;
                 do
                 {
                     m_contents_vector[i] = m_contents_vector[i-1];
                     i--;
                 } while (i>start && *(m_contents_vector[i-1]) <*t);
                 m_contents_vector[i]=t;
             }
         }
    }   // insertionSort

    bool empty() const
    {
        return m_contents_vector.empty();
    }

};   // class ptrVector


#define for_in( VAR, VECTOR ) for (unsigned int _foreach_i = 0; \
           VAR = (_foreach_i < VECTOR.size() ? VECTOR.get(_foreach_i) : NULL),\
           _foreach_i < VECTOR.size(); _foreach_i++)
#define for_var_in( TYPE, VAR, VECTOR ) TYPE VAR; for (unsigned int _foreach_i = 0; \
    VAR = (_foreach_i < VECTOR.size() ? VECTOR.get(_foreach_i) : NULL), \
    _foreach_i < VECTOR.size(); _foreach_i++)

#endif
