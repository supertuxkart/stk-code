//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs, Marianne Gagnon
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

#ifndef HEADER_SMOKE_HPP
#define HEADER_SMOKE_HPP

#include "irrlicht.h"
using namespace irr;

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#define VISUALIZE_BOX_EMITTER 0

#if VISUALIZE_BOX_EMITTER
#include <vector>
#endif

class Material;
class ParticleKind;
class Track;

/**
 * \brief manages smoke particle effects
 * \ingroup graphics
 */
class ParticleEmitter : public NoCopy
{
private:
    
    /** Irrlicht's particle systems. */
    scene::IParticleSystemSceneNode *m_node;
    
    Vec3                             m_position;
    
    scene::ISceneNode*               m_parent;
    
    /** The emitters. Access to these is needed to adjust the number of
     *  particles per second. */
    scene::IParticleEmitter         *m_emitter;

#if VISUALIZE_BOX_EMITTER
    std::vector<scene::ISceneNode*> m_visualisation;
#endif
    
    const ParticleKind              *m_particle_type;
    
    unsigned int m_magic_number;
    
public:
    
    ParticleEmitter             (const ParticleKind* type, 
                                 const Vec3 &position,
                                 scene::ISceneNode* parent = NULL);
    virtual     ~ParticleEmitter();
    virtual void update         ();
    void         setCreationRate(float f);
    
    void         setPosition(const Vec3 &pos);
    
    const ParticleKind* getParticlesInfo() const { return m_particle_type; }
    
    void         setParticleType(const ParticleKind* p);
    
    void resizeBox(float size);
    
    scene::IParticleSystemSceneNode* getNode() { return m_node; }
    
    void         addHeightMapAffector(Track* t);
};
#endif


