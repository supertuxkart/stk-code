//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_WORLDSCREEN_H
#define HEADER_WORLDSCREEN_H

#include "screen.hpp"


class Camera;
class World;
class RaceSetup;

class WorldScreen : public Screen
{
private:

    ulClock m_fclock ;
    ulClock m_frame_clock;
    int     m_frame_count;

    typedef std::vector<Camera*> Cameras;
    Cameras m_cameras;

    static WorldScreen* m_current_;
public:
    static WorldScreen* current() { return m_current_; }

    WorldScreen(const RaceSetup& racesetup);
    virtual ~WorldScreen();

    Camera* getCamera(int i) const;

    void draw();
    void update();
};

#endif

/* EOF */
