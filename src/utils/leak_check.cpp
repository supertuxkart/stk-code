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
#include "utils/leak_check.hpp"

#include "utils/ptr_vector.hpp"

#ifdef DEBUG

/** Switch this to 1 to get the backtrace of the leaks (slows down execution a little)
    Atm only implemented for OSX */
#define GET_STACK_TRACE 0


#if (GET_STACK_TRACE == 1) && defined(MAC_OS_X_VERSION_10_5)
#include <Availability.h>
#include <execinfo.h>
#endif

#include <iostream>
#include <set>
#include <stdio.h>

namespace MemoryLeaks
{
    
    std::set<MyObject*> g_all_objs;
    
    void addObj(MyObject* obj)
    {
        g_all_objs.insert(obj);
    }
    
    void removeObj(MyObject* obj)
    {
        g_all_objs.erase(obj);
        delete obj;
    }
    
    MyObject::MyObject(AbstractLeakCheck* objArg)
    {
        obj = objArg;
        
#if (GET_STACK_TRACE == 1) && defined(MAC_OS_X_VERSION_10_5)
        
        const int maxsize = 32;
        void* callstack[maxsize];
        stackSize = backtrace(callstack, maxsize);
        
        stack = backtrace_symbols(callstack, stackSize);
#else
        stack = NULL;
#endif
        
    }
    
    void MyObject::print()
    {
        obj->print();
        
        if (stack == NULL)
        {
            printf("    (No stack information available)\n");
        }
        else
        {
            for (int i = 0; i < stackSize; ++i)
            {
                printf("    %s\n", stack[i]);
            }
        }
        
    }
    
    void checkForLeaks()
    {
        
        std::cout << "checking for leaks... " << std::endl;
        
        if (g_all_objs.size()>0)
        {
            std::cout << "leaks detected!!" << std::endl;
            std::cout << "\n\n* * * * WARNING * * * * WARNING * * * * MEMORY LEAK! * * * *\n" << std::endl;
        }
        else
        {
            std::cout << "ok (no watched class left leaking)" << std::endl;
            return;
        }
        std::cout << "LEAK CHECK: " << g_all_objs.size() << " watched objects leaking" << std::endl;
        
        std::set<MyObject*>::iterator it;
        for (it = g_all_objs.begin(); it != g_all_objs.end(); ++it)
        {
            (*it)->print();
        }
        /*
        for (int n=0; n<g_all_objs.size(); n++)
        {
            g_all_objs[n].print();
        }*/
    }
    
}

#endif
