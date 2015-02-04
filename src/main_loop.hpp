//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#ifndef HEADER_MAIN_LOOP_HPP
#define HEADER_MAIN_LOOP_HPP

typedef unsigned long Uint32;


/** Management class for the whole gameflow, this is where the
    main-loop is */
class MainLoop
{
private:
    bool m_abort;
    bool m_throttle_fps;

    Uint32   m_curr_time;
    Uint32   m_prev_time;
    float    getLimitedDt();
    void     updateRace(float dt);
public:
         MainLoop();
        ~MainLoop();
    void run();
    void abort();
    void setThrottleFPS(bool throttle) { m_throttle_fps = throttle; }
    // ------------------------------------------------------------------------
    /** Returns true if STK is to be stoppe. */
    bool isAborted() const { return m_abort; }
};   // MainLoop

extern MainLoop* main_loop;

#endif

/* EOF */
