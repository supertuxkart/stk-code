//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include <plib/ssg.h>
#include "explosion.hpp"
#include "projectile_manager.hpp"
#include "sound_manager.hpp"
#include "scene.hpp"


Explosion::Explosion(const Vec3& coord) : ssgTransform()
{
    this->ref();
    ssgCutout *cut = new ssgCutout();
    addKid(cut);  // derefing the explosion will free the cutout
    m_seq   = projectile_manager->getExplosionModel();
    cut->addKid(m_seq);
    init(coord);
}   // Explosion

//-----------------------------------------------------------------------------
void Explosion::init(const Vec3& coord)
{
    sound_manager->playSfx( SOUND_EXPLOSION );

    sgCoord c;
    c.xyz[0]=coord[0];c.xyz[1]=coord[1];c.xyz[2]=coord[2];
    c.hpr[0]=0; c.hpr[1]=0; c.hpr[2]=0;
    setTransform(&c);
    m_step = -1;
    scene->add(this);
}

//-----------------------------------------------------------------------------
void Explosion::update (float dt)
{

    if ( ++m_step >= m_seq->getNumKids() )
    {
        scene->remove((ssgTransform*)this);
        projectile_manager->FinishedExplosion();
        return ;
    }

    m_seq -> selectStep ( m_step ) ;

}
