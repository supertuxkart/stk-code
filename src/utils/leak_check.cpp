//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2012 Marianne Gagnon, Joerg Henrichs
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "utils/leak_check.hpp"

#include "utils/synchronised.hpp"
#include "utils/ptr_vector.hpp"

#ifdef DEBUG

/** Switch this to 1 to get the backtrace of the leaks (slows down execution a little)
    Atm only implemented for OSX */
#define GET_STACK_TRACE 0


#if (GET_STACK_TRACE == 1) && defined(__APPLE__)
#  include <Availability.h>
#  include <execinfo.h>
#endif

#include <iostream>
#include <set>
#include <stdio.h>

namespace MemoryLeaks
{
    /** A set with all currently allocated objects. */ 
    Synchronised< std::set<AllocatedObject*> > g_all_objects;
    
    // ------------------------------------------------------------------------
    AllocatedObject::AllocatedObject()
    {        
#if (GET_STACK_TRACE == 1) && defined(__APPLE__)    
        const int max_size = 32;
        void* callstack[max_size];
        m_stack_size = backtrace(callstack, max_size);
        
        m_stack = backtrace_symbols(callstack, m_stack_size);
#else
        m_stack = NULL;
#endif
        addObject(this);
    }   // AllocatedObject
    
    // ------------------------------------------------------------------------
    AllocatedObject::~AllocatedObject()
    {
        removeObject(this);
    }   // ~AllocatedObject
    // ------------------------------------------------------------------------
    /** Print the data associated with the objects.
     */
    void AllocatedObject::print() const
    {
        //m_object->print();
        
        if (m_stack == NULL)
        {
            printf("    (No stack information available)\n");
        }
        else
        {
            for (int i = 0; i < m_stack_size; ++i)
            {
                printf("    %s\n", m_stack[i]);
            }
        }
        
    }   // print
    
    // ========================================================================
    /** Adds an object to the sets of all allocated objects. */
    void addObject(AllocatedObject* obj)
    {
        g_all_objects.lock();
        g_all_objects.getData().insert(obj);
        g_all_objects.unlock();
    }   // addObject

    // ------------------------------------------------------------------------
    /** Removes an object from the set of all allocated objects. */
    void removeObject(AllocatedObject* obj)
    {
        g_all_objects.lock();
        g_all_objects.getData().erase(obj);
        g_all_objects.unlock();
        //delete obj;
    }   // removeObject
    
    // ------------------------------------------------------------------------
    /** Checks if any objects are still allocated, and if so print information
     *  about those objects. */
    void checkForLeaks()
    {   
        std::cout << "checking for leaks... " << std::endl;
        g_all_objects.lock();
        if (g_all_objects.getData().size()>0)
        {
            std::cout << "leaks detected!!" << std::endl;
            std::cout << "\n\n* * * * WARNING * * * * WARNING * * * * "
                         "MEMORY LEAK! * * * *\n" << std::endl;
            std::cout << "LEAK CHECK: " << g_all_objects.getData().size() 
                      << " watched objects leaking" << std::endl;
        }
        else
        {
            std::cout << "ok (no watched class left leaking)" << std::endl;
        }
        
        std::set<AllocatedObject*>::iterator it;
        for (it  = g_all_objects.getData().begin(); 
             it != g_all_objects.getData().end();   ++it)
        {
            (*it)->print();
        }
        g_all_objects.unlock();
    }   // checkForLeaks
    
}   // namespace MemoryLeaks

#endif
