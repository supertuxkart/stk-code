//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_EXPLOSION_H
#define HEADER_EXPLOSION_H

class ssgSelector;
class Projectile;


class Explosion : public ssgTransform
{
public:
    int m_step ;
    ssgSelector  *m_seq ;
public:

    Explosion(Projectile* p);
    int  inUse    ()              {return (m_step >= 0); }
    void init     (Projectile *p);
    void update   (float delta_t);
    bool hasEnded () {return m_step >= m_seq->getNumKids(); }

} ;

#endif

/* EOF */
