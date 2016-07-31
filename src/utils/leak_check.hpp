//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Marianne Gagnon, Joerg Henrichs
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


#ifndef HEADER_LEAK_CHECK_HPP
#define HEADER_LEAK_CHECK_HPP


#ifdef DEBUG

#include "utils/log.hpp"

#ifdef WIN32
#  include <string>
#  include <vector>
#endif

namespace MemoryLeaks
{

    class AllocatedObject
    {
#if defined(__APPLE__)
        /** Keep stack information if available (OSX only). */
        char **m_stack;
        /** Keeps stacksize information if available (OSX only). */
        int    m_stack_size;
#elif defined(WIN32) || ENABLE_LIBBFD
        /** Keep the stack information the way it is returned by windows or
         *  libbfd, a flat string, which will be split when printing it. */
        std::string m_stack;
#endif

    public:
        AllocatedObject();
        virtual ~AllocatedObject();
        virtual void print() const;
    };   // AllocatedObjects

    // ========================================================================
    void checkForLeaks();

    void addObject(AllocatedObject* obj);
    void removeObject(AllocatedObject* obj);


}   // namespace MemoryLeaks

#define LEAK_CHECK()                                                 \
class LeakCheck : public MemoryLeaks::AllocatedObject                \
{                                                                    \
public:                                                              \
    virtual void print() const                                       \
    {                                                                \
        Log::error("LeakCheck", "Undeleted object at %s : %i",       \
                    __FILE__, __LINE__);                             \
        AllocatedObject::print();                                    \
    }                                                                \
    virtual ~LeakCheck() {}                                          \
};                                                                   \
                                                                     \
LeakCheck m_leack_check_instance;


#else
// No debugging
#define LEAK_CHECK()
#endif

#endif
