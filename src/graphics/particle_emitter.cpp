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

#include "graphics/particle_emitter.hpp"

#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

class FadeAwayAffector : public scene::IParticleAffector
{
    /** (Squared) distance from camera at which a particle started being faded out */
    float m_start_fading;
    
    /** (Squared) distance from camera at which a particle is completely faded out */
    float m_end_fading;
    
public:
    FadeAwayAffector(float start, float end)
    {
        m_start_fading = start;
        m_end_fading = end;
        assert(m_end_fading >= m_start_fading);
    }   // FadeAwayAffector
    
    // ------------------------------------------------------------------------
    
    virtual void affect(u32 now, scene::SParticle* particlearray, u32 count)
    {
        scene::ICameraSceneNode* curr_cam = 
            irr_driver->getSceneManager()->getActiveCamera();
        const core::vector3df& cam_pos = curr_cam->getPosition();
        
        // printf("Affect called with now=%u, camera=%s\n", now, curr_cam->getName());
        
        for (unsigned int n=0; n<count; n++)
        {
            scene::SParticle& curr = particlearray[n];
            core::vector3df diff = curr.pos - cam_pos;
            const float x = diff.X;
            const float y = diff.Y;
            const float z = diff.Z;
            const float distance_squared = x*x + y*y + z*z;
            
            if (distance_squared < m_start_fading)
            {
                curr.color.setAlpha(255);
            }
            else if (distance_squared > m_end_fading)
            {
                curr.color.setAlpha(0);
            }
            else
            {
                curr.color.setAlpha((int)((distance_squared - m_start_fading) 
                                        / (m_end_fading - m_start_fading)));
            }
        }   // for n<count
    }   // affect

    // ------------------------------------------------------------------------
    
    virtual scene::E_PARTICLE_AFFECTOR_TYPE getType() const
    {
        // FIXME: this method seems to make sense only for built-in affectors
        return scene::EPAT_FADE_OUT;
    }
    
};   // FadeAwayAffector


// ============================================================================

class HeightMapCollisionAffector : public scene::IParticleAffector
{
    std::vector< std::vector<float> > m_height_map;
    Track* m_track;
    bool m_first_time;
    
public:
    HeightMapCollisionAffector(Track* t) : m_height_map(t->buildHeightMap())
    {
        m_track = t;
        m_first_time = true;
    }
    
    virtual void affect(u32 now, scene::SParticle* particlearray, u32 count)
    {
        const Vec3* aabb_min;
        const Vec3* aabb_max;
        m_track->getAABB(&aabb_min, &aabb_max);
        float track_x = aabb_min->getX();
        float track_z = aabb_min->getZ();
        const float track_x_len = aabb_max->getX() - aabb_min->getX();
        const float track_z_len = aabb_max->getZ() - aabb_min->getZ();
        
        for (unsigned int n=0; n<count; n++)
        {
            scene::SParticle& curr = particlearray[n];
            const int i = (int)( (curr.pos.X - track_x)
                                 /track_x_len*(HEIGHT_MAP_RESOLUTION) );
            const int j = (int)( (curr.pos.Z - track_z)
                                 /track_z_len*(HEIGHT_MAP_RESOLUTION) );
            if (i >= HEIGHT_MAP_RESOLUTION || j >= HEIGHT_MAP_RESOLUTION) continue;
            if (i < 0 || j < 0) continue;

            /*
            // debug draw
            core::vector3df lp = curr.pos;
            core::vector3df lp2 = curr.pos;
            lp2.Y = m_height_map[i][j] + 0.02f;
            
            irr_driver->getVideoDriver()->draw3DLine(lp, lp2, video::SColor(255,255,0,0));
            core::vector3df lp3 = lp2;
            lp3.X += 0.1f;
            lp3.Y += 0.02f;
            lp3.Z += 0.1f;
            lp2.X -= 0.1f;
            lp2.Y -= 0.02f;
            lp2.Z -= 0.1f;
            irr_driver->getVideoDriver()->draw3DBox(core::aabbox3d< f32 >(lp2, lp3), video::SColor(255,255,0,0));
            */
            
            if (m_first_time)
            {
                curr.pos.Y = m_height_map[i][j] 
                           + (curr.pos.Y - m_height_map[i][j])
                                *((rand()%500)/500.0f);
            }
            else
            {
                if (curr.pos.Y < m_height_map[i][j])
                {
                    //curr.color = video::SColor(255,255,0,0);
                    curr.endTime = curr.startTime; // destroy particle
                }
            }
        }
        
        if (m_first_time) m_first_time = false;
    }
    
    virtual scene::E_PARTICLE_AFFECTOR_TYPE getType() const
    {
        // FIXME: this method seems to make sense only for built-in affectors
        return scene::EPAT_FADE_OUT;
    }
};


// ============================================================================

ParticleEmitter::ParticleEmitter(const ParticleKind* type, 
                                 const Vec3 &position,
                                 scene::ISceneNode* parent) 
               : m_position(position)
{
    assert(type != NULL);
    m_magic_number = 0x58781325;
    m_node = NULL;
    m_particle_type = NULL;
    m_parent = parent;
    setParticleType(type);
    assert(m_node != NULL);
    
}   // KartParticleSystem

//-----------------------------------------------------------------------------
/** Destructor, removes
 */
ParticleEmitter::~ParticleEmitter()
{
    assert(m_magic_number == 0x58781325);
    assert(m_node != NULL);
    irr_driver->removeNode(m_node);
    
    m_magic_number = 0xDEADBEEF;
}   // ~ParticleEmitter

//-----------------------------------------------------------------------------
void ParticleEmitter::update()
{
    assert(m_magic_number == 0x58781325);
    
    // No particles to emit, nothing to do
    if (m_emitter->getMinParticlesPerSecond() == 0) return;
    
    // the emission direction does not automatically follow the orientation of
    // the node so fix that manually...
    core::matrix4 	transform = m_node->getAbsoluteTransformation();
    core::vector3df velocity(m_particle_type->getVelocityX(),
                             m_particle_type->getVelocityY(),
                             m_particle_type->getVelocityZ());
    
    transform.rotateVect(velocity);
    m_emitter->setDirection(velocity);
    
    // There seems to be no way to randomise the velocity for particles,
    // so we have to do this manually, by changing the default velocity.
    // Irrlicht expects velocity (called 'direction') in m/ms!!
    /*
    const int x = m_particle_type->getAngleSpreadX();
    const int y = m_particle_type->getAngleSpreadY();
    const int z = m_particle_type->getAngleSpreadZ();
    Vec3 dir(cos(DEGREE_TO_RAD*(rand()%x - x/2))*m_particle_type->getVelocityX(),
             sin(DEGREE_TO_RAD*(rand()%y - x/2))*m_particle_type->getVelocityY(),
             sin(DEGREE_TO_RAD*(rand()%z - x/2))*m_particle_type->getVelocityZ());
    m_emitter->setDirection(dir.toIrrVector());
     */
}   // update

//-----------------------------------------------------------------------------

void ParticleEmitter::setCreationRate(float f)
{
    m_emitter->setMinParticlesPerSecond(int(f));
    m_emitter->setMaxParticlesPerSecond(int(f));
    
    // FIXME: to work around irrlicht bug, when an emitter is paused by setting the rate
    //        to 0 results in a massive emission when enabling it back. In irrlicht 1.8
    //        the node has a method called "clearParticles" that should be cleaner than this
    if (f == 0.0f)
    {
        m_emitter->grab();
        m_node->setEmitter(NULL);
    }
    else if (m_node->getEmitter() == NULL)
    {
        m_node->setEmitter(m_emitter);
        m_emitter->drop();
    }
}   // setCreationRate

//-----------------------------------------------------------------------------
/** Sets the position of the particle emitter.
 *  \param pos The position for the particle emitter.
 */
void ParticleEmitter::setPosition(const Vec3 &pos)
{
    m_node->setPosition(pos.toIrrVector());
}   // setPosition

//-----------------------------------------------------------------------------

void ParticleEmitter::setParticleType(const ParticleKind* type)
{    
    assert(m_magic_number == 0x58781325);
    if (m_particle_type == type) return; // already the right type
    
    if (m_node != NULL)
    {
        m_node->removeAll();
        m_node->removeAllAffectors();
    }
    else
    {
        m_node = irr_driver->addParticleNode();
    }
    
    if (m_parent != NULL)
    {
        m_node->setParent(m_parent);
    }
    
    
    m_particle_type = type;
    
    Material* material    = type->getMaterial();
    const float minSize   = type->getMinSize();
    const float maxSize   = type->getMaxSize();
    const int lifeTimeMin = type->getMinLifetime();
    const int lifeTimeMax = type->getMaxLifetime();
    
    assert(material->getTexture() != NULL);
    assert(maxSize >= minSize);
    assert(lifeTimeMax >= lifeTimeMin);
    
#ifdef DEBUG
    video::ITexture* tex = material->getTexture();
    assert(tex != NULL);
    const io::SNamedPath& name = tex->getName();
    const io::path& tpath = name.getPath();
    
    std::string debug_name = std::string("particles(") + tpath.c_str() + ")";
    m_node->setName(debug_name.c_str());
#endif
    
    video::SMaterial& mat0 = m_node->getMaterial(0);
    
    m_node->setPosition(m_position.toIrrVector());
    material->setMaterialProperties(&mat0);
    m_node->setMaterialTexture(0, material->getTexture());
    
    mat0.ZWriteEnable = !material->isTransparent(); // disable z-buffer writes if material is transparent
    
    switch (type->getShape())
    {
        case EMITTER_POINT:
        {
            m_emitter = m_node->createPointEmitter(core::vector3df(m_particle_type->getVelocityX(),
                                                                   m_particle_type->getVelocityY(),
                                                                   m_particle_type->getVelocityZ()),   // velocity in m/ms
                                                   type->getMinRate(), type->getMaxRate(),
                                                   type->getMinColor(), type->getMaxColor(),
                                                   lifeTimeMin, lifeTimeMax,
                                                   m_particle_type->getAngleSpread() /* angle */
                                                   );
            break;
        }
            
        case EMITTER_BOX:
        {
            const float box_size_x = type->getBoxSizeX()/2.0f;
            const float box_size_y = type->getBoxSizeY()/2.0f;
            const float box_size_z = type->getBoxSizeZ()/2.0f;
            m_emitter = m_node->createBoxEmitter(core::aabbox3df(-box_size_x, -box_size_y, -box_size_z,
                                                                 box_size_x,  box_size_y,  box_size_z),
                                                 core::vector3df(m_particle_type->getVelocityX(),
                                                                 m_particle_type->getVelocityY(),
                                                                 m_particle_type->getVelocityZ()),   // velocity in m/ms
                                                 type->getMinRate(), type->getMaxRate(),
                                                 type->getMinColor(), type->getMaxColor(),
                                                 lifeTimeMin, lifeTimeMax,
                                                 m_particle_type->getAngleSpread() /* angle */
                                                 );            
            break;
        }
            
        default:
        {
            fprintf(stderr, "[ParticleEmitter] Unknown shape\n");
            return;
        }
    }
    
    m_emitter->setMinStartSize(core::dimension2df(minSize, minSize));
    m_emitter->setMaxStartSize(core::dimension2df(maxSize, maxSize));
    m_node->setEmitter(m_emitter); // this grabs the emitter
    m_emitter->drop();             // so we can drop our references
    
    // FIXME: this is ridiculous, the fadeout time should be equal to the lifetime, except that the
    //        lifetime is random...
    scene::IParticleFadeOutAffector *af = m_node->createFadeOutParticleAffector(video::SColor(0, 255, 255, 255),
                                                                                type->getFadeoutTime());
    m_node->addAffector(af);
    af->drop();
    
    if (type->getGravityStrength() != 0)
    {
        scene::IParticleGravityAffector *gaf = m_node->createGravityAffector(core::vector3df(00.0f, type->getGravityStrength(), 0.0f),
                                                                             type->getForceLostToGravityTime());
        m_node->addAffector(gaf);
        gaf->drop();
    }
    
    const float fas = type->getFadeAwayStart();
    const float fae = type->getFadeAwayEnd();
    if (fas > 0.0f && fae > 0.0f)
    {
        FadeAwayAffector* faa = new FadeAwayAffector(fas*fas, fae*fae);
        m_node->addAffector(faa);
        faa->drop();
    }
}   // setParticleType

//-----------------------------------------------------------------------------

void ParticleEmitter::addHeightMapAffector(Track* t)
{
    HeightMapCollisionAffector* hmca = new HeightMapCollisionAffector(t);
    m_node->addAffector(hmca);
    hmca->drop();
}
