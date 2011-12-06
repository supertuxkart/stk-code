//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Maik Semder <ikework@gmx.de>
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

#ifndef HEADER_REPLAYBUFFERTPL_HPP
#define HEADER_REPLAYBUFFERTPL_HPP

#ifdef HAVE_GHOST_REPLAY


#include <new>

// needed for MSVC-memory-leak-checks
#if defined( REPLAY_UNIT_TEST ) && defined( _MSC_VER )
#  ifdef _DEBUG
#    ifndef _DBG_NEW
#      include <crtdbg.h>
        inline void* __operator_new(size_t __n) { return ::operator 
            new(__n,_NORMAL_BLOCK,__FILE__,__LINE__); }
        inline void* _cdecl operator new(size_t __n,const char* __fname,
            int __line) { 
                return ::operator new(__n,_NORMAL_BLOCK,__fname,__line); }
        inline void _cdecl operator delete(void* __p,const char*,int) 
        { ::operator delete(__p);}
#       define _DBG_NEW new(__FILE__,__LINE__)
#       define new _DBG_NEW
#    endif // _DBG_NEW
#  else
#    define __operator_new(__n) operator new(__n)
#  endif
#endif


template<typename T> class ReplayBufferArray;

template<typename T>
class ReplayBuffer
{
    friend class ReplayBufferArray<T>;

public:
    ReplayBuffer() : m_pp_blocks(NULL),m_number_blocks(0),m_block_size(0),
                     m_number_objects_used(0),m_healthy(true) {}
    ~ReplayBuffer() { destroy(); }

private:
    ReplayBuffer( ReplayBuffer<T> const &c );
    ReplayBuffer<T> const &operator=( ReplayBuffer<T> const &c );

public:
    bool            init( size_t number_preallocated_objects );
    void            destroy();
    // this is false, if a reallocation failed
    bool            isHealthy() const                   { return m_healthy; }
    // returns a new *free* object, allocated memory if necessary
    T*              getNewObject();
    // returs object at given position, like usual array access,
    // does not allocate memory, index must be < getNumberObjectsUsed()
    T const*        getObjectAt( size_t index ) const;
    T*              getObjectAt( size_t index );
    size_t          getNumberObjectsUsed() const 
                    { return m_number_objects_used; }
    size_t          getNumberBlocks() const { return m_number_blocks; }

private:
    // adds a new block of objects to m_pp_blocks with a size of m_block_size
    bool            addNewBlock();
    // helper to make sure healthy bit is set, if new fails
    template<typename TT>
    TT*              buffer_new_array( size_t n )
    {
#if defined( REPLAY_UNIT_TEST ) && defined( _MSC_VER )
        // the msvc-debug-new macros didnt like nothrow
        TT *p = new TT[ n ];
#else
        TT *p = new(std::nothrow) TT[ n ];
#endif
        m_healthy = (NULL != p );
        return p;
    }

private:
    T       **m_pp_blocks;
    // number of allocated blocks. we start with 1 for recording
    // for showing a replay-file it is 1
    size_t  m_number_blocks;
    // size of all blocks
    size_t  m_block_size;
    // number of used objects
    size_t  m_number_objects_used;
    // this flag indicates, that, if any reallocation happedened, it failed
    // then recordings is blocked
    bool    m_healthy;
};


// does the same as ReplayBuffer<T>, but it returns an array of objects, 
// rather than just one object .. 
template<typename T>
class ReplayBufferArray
{
public:
    ReplayBufferArray() : m_Buffer(), m_array_size(0) {}
    ~ReplayBufferArray() { destroy(); }

    void            destroy();
    bool            init( size_t number_preallocated_arrays, 
                          size_t array_size );
    // returns a new *free* array of objects with size of 2nd param in init
    T*              getNewArray();
    // returs objects at given position, like usual array access,
    // does not allocate memory
    T const*        getArrayAt( size_t index ) const
                    { assert( m_array_size ); 
                      return m_Buffer.getObjectAt( m_array_size * index ); }
    T*              getArrayAt( size_t index )
                    { assert( m_array_size ); 
                      return m_Buffer.getObjectAt( m_array_size * index ); }
    size_t          getNumberArraysUsed() const
                    { return m_Buffer.getNumberObjectsUsed() / m_array_size; }
    size_t          getNumberBlocks() const
                    { return m_Buffer.getNumberBlocks(); }
    bool            isHealthy() const
                    { return m_Buffer.isHealthy(); }

private:
    ReplayBuffer<T> m_Buffer;
    size_t          m_array_size;
};




template<typename T>
bool ReplayBuffer<T>::init( size_t number_preallocated_objects )
{
    // make sure *clean* usage
    assert( !m_pp_blocks );
    assert( number_preallocated_objects );
    m_block_size = number_preallocated_objects;

    if( !addNewBlock() ) return false;

    return true;
}

template<typename T>
void ReplayBuffer<T>::destroy()
{
    size_t tmp;
    if( m_pp_blocks )
    {
        for( tmp = 0; tmp < m_number_blocks; ++tmp ) delete[] m_pp_blocks[tmp];
        delete[] m_pp_blocks; m_pp_blocks = NULL;
        m_number_blocks = 0;
        m_block_size = 0;
        m_number_objects_used = 0;
        m_healthy = true;
    }
}

// returns a new *free* frame to be used to store the current frame-data into 
// it used to *record* the replay
template<typename T>
T* ReplayBuffer<T>::getNewObject()
{
    // make sure initialization was called properly
    assert( m_pp_blocks );
    assert( m_number_blocks );

    if( !m_healthy ) return NULL;

    // check, if we need a new block
    if( m_number_objects_used == (m_block_size*m_number_blocks) )
    {
        // we need a new block
        if( !addNewBlock() ) return NULL;
    }

    // get current frame
    T* block_current = m_pp_blocks[ m_number_blocks-1 ];
    size_t new_in_block_idx = m_number_objects_used % m_block_size;
    T* current = block_current + new_in_block_idx;

    ++m_number_objects_used;

    return current;
}

// returs frame at given position from replay data
// used to *show* the replay
template<typename T>
T const* ReplayBuffer<T>::getObjectAt( size_t index ) const
{
    // make sure initialization was called properly
    assert( m_pp_blocks );
    assert( m_number_blocks );
    assert( index < m_number_objects_used );
    assert( (index / m_block_size) < m_number_blocks );

    T const* block = m_pp_blocks[ index / m_block_size ];
    return block + (index % m_block_size);
}

template<typename T>
T* ReplayBuffer<T>::getObjectAt( size_t index )
{
    // make sure initialization was called properly
    assert( m_pp_blocks );
    assert( m_number_blocks );
    assert( index < m_number_objects_used );
    assert( (index / m_block_size) < m_number_blocks );

    T* block = m_pp_blocks[ index / m_block_size ];
    return block + (index % m_block_size);
}

// adds a new block of objects to m_pp_blocks with a size of m_block_size
template<typename T>
bool ReplayBuffer<T>::addNewBlock()
{
    assert( m_block_size );

    if( !m_healthy ) return false;

    size_t number_blocks_new = m_number_blocks + 1;

    T **pp_blocks_old = m_pp_blocks;
    m_pp_blocks = buffer_new_array<T*>( number_blocks_new );
    if( !m_pp_blocks ) 
    {
        // put back old blocks
        m_pp_blocks = pp_blocks_old;
        return false;
    }

    // copy old block pointers .. note: we dont copy the objects,
    // only the pointers, to blocks of objects, which is supposed 
    // to be a very small number .. 2 is probably never reached
    size_t tmp;
    for( tmp = 0; tmp < m_number_blocks; ++tmp ) 
        m_pp_blocks[tmp] = pp_blocks_old[tmp];

    // create new objects at new block position
    m_pp_blocks[m_number_blocks] = buffer_new_array<T>( m_block_size );
    if( !m_pp_blocks[m_number_blocks] )
    {
        // delete and put back old blocks
        delete[] m_pp_blocks;
        m_pp_blocks = pp_blocks_old;
        return false;
    }

    // everything went fine, we got new arrays of objects
    delete[] pp_blocks_old; pp_blocks_old = NULL;

    ++m_number_blocks;

    return true;
}


template<typename T>
void ReplayBufferArray<T>::destroy()
{
    m_Buffer.destroy();
    m_array_size = 0;
}

template<typename T>
bool ReplayBufferArray<T>::init( size_t number_preallocated_arrays,
                                 size_t array_size )
{
    assert( number_preallocated_arrays );
    assert( array_size );
    m_array_size = array_size;
    return m_Buffer.init( number_preallocated_arrays * array_size );
}

// returns a new *free* array of objects
template<typename T>
T* ReplayBufferArray<T>::getNewArray()
{
    if( !isHealthy() ) return NULL;

    // check, if we need a new block
    if( m_Buffer.m_number_objects_used == 
        (m_Buffer.m_block_size*m_Buffer.m_number_blocks) )
    {
        // we need a new block
        if( !m_Buffer.addNewBlock() ) return NULL;
    }

    // get current frame
    T* block_current = m_Buffer.m_pp_blocks[ m_Buffer.m_number_blocks-1 ];
    size_t new_in_block_idx = 
        m_Buffer.m_number_objects_used % m_Buffer.m_block_size;
    T* current = block_current + new_in_block_idx;

    assert( (current + m_array_size) <= (m_Buffer
            .m_pp_blocks[ m_Buffer.m_number_blocks-1 ] 
           + m_Buffer.m_block_size) );

    m_Buffer.m_number_objects_used += m_array_size;

    return current;
}


#endif // HAVE_GHOST_REPLAY

#endif // HEADER_REPLAYBUFFERTPL_HPP

