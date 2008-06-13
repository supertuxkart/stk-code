//  $Id: moving_texture.hpp 796 2006-09-27 07:06:34Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef MOVING_TEXTURE_H
#define MOVING_TEXTURE_H

#include <string>
#include <plib/sg.h>
#include <plib/ssg.h>
#include "callback.hpp"

class MovingTexture : public Callback
{
private:
    // the ac model files assume MODE_FORWARD=1, so a MODE_NONE is added
    enum allCallbackModesType { MODE_NONE, MODE_FORWARD, MODE_CYCLE, MODE_SHUTTLE,
                                MODE_SINESHUTTLE};

    sgCoord    m_delta;
    sgCoord    m_now;
    float      m_phase;
    float      m_cycle;
    int        m_mode;
    ssgBranch *m_branch;

    void       parseData(char *data);

public:
    MovingTexture(char *data, ssgBranch *branch_);
     ~MovingTexture();
    void update  (float dt);
    void init    ()         {};
}
;   // MovingTexture

#endif

