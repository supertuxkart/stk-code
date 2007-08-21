//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#ifndef HEADER_PROJECTILE_H
#define HEADER_PROJECTILE_H

#include "moveable.hpp"

class Kart;

class Projectile : public Moveable
{
    sgCoord     m_last_pos;
    const Kart* m_owner;
    float       m_speed;    // Speed of the projectile
    int         m_type ;
    bool        m_has_hit_something;
    int         m_last_radar_beep;
public:

    Projectile               (Kart* kart_, int type);
    virtual ~Projectile();
    void init                (Kart* kart_, int type);
    void update              (float);
    void doCollisionAnalysis (float dt, float hot);
    void doObjectInteractions();
    void explode             (Kart* kart);
    bool hasHit              ()            {return m_has_hit_something;}
    void reset               ()
    {
        Moveable::reset();
        sgCopyCoord(&m_last_pos,&m_reset_pos );
    }
    void OutsideTrack        (int isReset) {explode(NULL);}

} ;



#endif
