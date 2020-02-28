//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#ifndef HEADER_STK_PROCESS_HPP
#define HEADER_STK_PROCESS_HPP

#include "utils/tls.hpp"

enum ProcessType : unsigned int
{
    PT_MAIN = 0, // Main process
    PT_CHILD = 1, // Child process inside main (can be server or ai instance)
    PT_COUNT = 2
};

namespace STKProcess
{
    // ========================================================================
    extern thread_local ProcessType g_process_type;
    // ------------------------------------------------------------------------
    /** Return which type (main or child) this thread belongs to. */
    inline ProcessType getType()                     { return g_process_type; }
    // ------------------------------------------------------------------------
    /** Called when any thread in main or child is created. */
    inline void init(ProcessType pt)                   { g_process_type = pt; }
    // ------------------------------------------------------------------------
    /** Reset when stk is started (for android mostly). */
    inline void reset()                           { g_process_type = PT_MAIN; }
} // namespace STKProcess

#endif
