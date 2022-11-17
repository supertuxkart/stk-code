//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs, Marianne Gagnon
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

#ifndef SERVER_ONLY

#include "graphics/particle_emitter.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/stk_particle.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

#include <algorithm>

#include <ITexture.h>

//-----------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(const ParticleKind* type,
                                 const Vec3 &position,
                                 scene::ISceneNode* parent,
                                 bool randomize_initial_y,
                                 bool important)
               : m_position(position)
{
    assert(type != NULL);
    m_magic_number        = 0x58781325;
    m_node                = NULL;
    m_emitter             = NULL;
    m_particle_type       = NULL;
    m_parent              = parent;
    m_emission_decay_rate = 0;
    m_randomize_initial_y = randomize_initial_y;
    m_important = important;

    setParticleType(type);
    assert(m_node != NULL);

}   // ParticleEmitter

//-----------------------------------------------------------------------------
/** Destructor, removes
 */
ParticleEmitter::~ParticleEmitter()
{
    assert(m_magic_number == 0x58781325);
    if (m_node != NULL)
        irr_driver->removeNode(m_node);
    m_emitter->drop();

    m_magic_number = 0xDEADBEEF;
}   // ~ParticleEmitter

//-----------------------------------------------------------------------------
void ParticleEmitter::update(float dt)
{
    assert(m_magic_number == 0x58781325);

    // No particles to emit, nothing to do
    if (m_emitter->getMinParticlesPerSecond() == 0) return;

    // the emission direction does not automatically follow the orientation of
    // the node so fix that manually...
    core::matrix4   transform = m_node->getAbsoluteTransformation();
    core::vector3df velocity(m_particle_type->getVelocityX(),
                             m_particle_type->getVelocityY(),
                             m_particle_type->getVelocityZ());

    transform.rotateVect(velocity);
    m_emitter->setDirection(velocity);

    if (m_emission_decay_rate > 0)
    {
        m_max_rate = m_min_rate = std::max(0.0f, (m_min_rate - m_emission_decay_rate*dt));
        setCreationRateAbsolute(m_min_rate);
    }

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
/** Sets the creation rate as a relative fraction between minimum (f=0) and
 *  maximum (f=1) of the creation rates defined in the particle kind.
 *  \param fraction Fraction to use.
 */
void ParticleEmitter::setCreationRateRelative(float fraction)
{
    assert(fraction >= 0.0f);
    assert(fraction <= 1.0f);
    const float min_rate = (float)(m_particle_type->getMinRate());
    const float max_rate = (float)(m_particle_type->getMaxRate());
    setCreationRateAbsolute(min_rate + fraction*(max_rate - min_rate));
}   // setCreationRateRelative

//-----------------------------------------------------------------------------
/** Sets the absolute creation rate (in particles per second).
 *  \param f The creation rate (in particles per second).
 */
void ParticleEmitter::setCreationRateAbsolute(float f)
{
    m_emitter->setMinParticlesPerSecond(int(f));
    m_emitter->setMaxParticlesPerSecond(int(f));

    m_min_rate = f;
    m_max_rate = f;

}   // setCreationRateAbsolute

//-----------------------------------------------------------------------------
int ParticleEmitter::getCreationRate()
{
    if (m_node->getEmitter() == NULL) return 0;
    return m_emitter->getMinParticlesPerSecond();
}

//-----------------------------------------------------------------------------
/** Sets the position of the particle emitter.
 *  \param pos The position for the particle emitter.
 */
void ParticleEmitter::setPosition(const Vec3 &pos)
{
    m_node->setPosition(pos.toIrrVector());
}   // setPosition

//-----------------------------------------------------------------------------
/** Sets the rotation of the particle emitter.
 *  \param pos The rotation for the particle emitter.
 */
void ParticleEmitter::setRotation(const Vec3 &rot)
{
    m_node->setRotation(rot.toIrrVector());
}   // setRotation

//-----------------------------------------------------------------------------
void ParticleEmitter::setParticleType(const ParticleKind* type)
{
    assert(m_magic_number == 0x58781325);
    bool is_new_type = (m_particle_type != type);
    if (is_new_type)
    {
        if (m_node != NULL)
        {
            m_node->removeAll();
            m_node->removeAllAffectors();
            m_emitter->drop();
        }
        else
        {
            m_node = new STKParticle(type->randomizeInitialY());
        }

        if (m_parent != NULL)
        {
            m_node->setParent(m_parent);
        }

        m_particle_type = type;
    }

    m_emission_decay_rate = type->getEmissionDecayRate();

    Material* material    = type->getMaterial();
    const float minSize   = type->getMinSize();
    const float maxSize   = type->getMaxSize();
    const int lifeTimeMin = type->getMinLifetime();
    const int lifeTimeMax = type->getMaxLifetime();

    assert(maxSize >= minSize);
    assert(lifeTimeMax >= lifeTimeMin);

#ifdef DEBUG
    if (material != NULL)
    {
        video::ITexture* tex = material->getTexture();
        assert(tex != NULL);
        const io::SNamedPath& name = tex->getName();
        const io::path& tpath = name.getPath();

        std::string debug_name = std::string("particles(") + tpath.c_str() + ")";
        m_node->setName(debug_name.c_str());
    }
#endif
    m_min_rate = (float)type->getMinRate();
    m_max_rate = (float)type->getMaxRate();

    if (is_new_type)
    {
        video::SMaterial& mat0 = m_node->getMaterial(0);

        m_node->setPosition(m_position.toIrrVector());

        if (material != NULL)
        {
            assert(material->getTexture() != NULL);
            material->setMaterialProperties(&mat0, NULL);
            m_node->setMaterialTexture(0, material->getTexture());

            mat0.ZWriteEnable = !material->isTransparent(); // disable z-buffer writes if material is transparent
        }
        else
        {
            std::string help = file_manager->getAsset(FileManager::GUI_ICON, "main_help.png");
            m_node->setMaterialTexture(0, irr_driver->getTexture(help));
        }

        // velocity in m/ms
        core::vector3df velocity(m_particle_type->getVelocityX(),
                                 m_particle_type->getVelocityY(),
                                 m_particle_type->getVelocityZ());

        switch (type->getShape())
        {
            case EMITTER_POINT:
            {
                m_emitter = m_node->createPointEmitter(velocity,
                                                       type->getMinRate(),  type->getMaxRate(),
                                                       type->getMinColor(), type->getMinColor(),
                                                       lifeTimeMin, lifeTimeMax,
                                                       m_particle_type->getAngleSpread() /* angle */
                                                       );
                break;
            }

            case EMITTER_BOX:
            {
                const float box_size_x = type->getBoxSizeX()/2.0f;
                const float box_size_y = type->getBoxSizeY()/2.0f;

                m_emitter = m_node->createBoxEmitter(core::aabbox3df(-box_size_x, -box_size_y, -0.6f,
                                                                     box_size_x,  box_size_y,  -0.6f - type->getBoxSizeZ()),
                                                     velocity,
                                                     type->getMinRate(),  type->getMaxRate(),
                                                     type->getMinColor(), type->getMinColor(),
                                                     lifeTimeMin, lifeTimeMax,
                                                     m_particle_type->getAngleSpread()
                                                     );
                break;
            }
            case EMITTER_SPHERE:
            {
                m_emitter = m_node->createSphereEmitter(core::vector3df(0.0f,0.0f,0.0f) /* center */,
                                                        m_particle_type->getSphereRadius(),
                                                        velocity,
                                                        type->getMinRate(),  type->getMaxRate(),
                                                        type->getMinColor(), type->getMinColor(),
                                                        lifeTimeMin, lifeTimeMax,
                                                        m_particle_type->getAngleSpread()
                                                 );
                break;
            }
            default:
            {
                Log::error("ParticleEmitter", "Unknown shape");
                return;
            }
        }
    }
    else
    {
        m_emitter->setMinParticlesPerSecond(int(m_min_rate));
        m_emitter->setMaxParticlesPerSecond(int(m_max_rate));
    }

    m_emitter->setMinStartSize(core::dimension2df(minSize, minSize));
    m_emitter->setMaxStartSize(core::dimension2df(maxSize, maxSize));

    if (is_new_type)
    {
        m_node->setEmitter(m_emitter); // this grabs the emitter

        if (type->hasScaleAffector())
        {
            m_node->setIncreaseFactor(type->getScaleAffectorFactorX());
        }

        if (type->getMinColor() != type->getMaxColor())
        {

            video::SColor color_from = type->getMinColor();
            m_node->setColorFrom(color_from.getRed() / 255.0f,
                color_from.getGreen() / 255.0f,
                color_from.getBlue() / 255.0f);

            video::SColor color_to = type->getMaxColor();
            m_node->setColorTo(color_to.getRed() / 255.0f,
                color_to.getGreen() / 255.0f,
                color_to.getBlue() / 255.0f);
        }

        const bool flips = type->getFlips();
        if (flips)
        {
            m_node->setFlips();
        }
    }
}   // setParticleType

//-----------------------------------------------------------------------------
void ParticleEmitter::addHeightMapAffector(Track* t)
{
    const Vec3* aabb_min;
    const Vec3* aabb_max;
    t->getAABB(&aabb_min, &aabb_max);
    float track_x = aabb_min->getX();
    float track_z = aabb_min->getZ();
    const float track_x_len = aabb_max->getX() - aabb_min->getX();
    const float track_z_len = aabb_max->getZ() - aabb_min->getZ();
    std::vector<std::vector<float> > array = t->buildHeightMap();
    m_node->setHeightmap(array, track_x, track_z, track_x_len, track_z_len);
}

//-----------------------------------------------------------------------------
void ParticleEmitter::resizeBox(float size)
{
    scene::IParticleBoxEmitter* emitter = (scene::IParticleBoxEmitter*)m_emitter;

    const float box_size_x = m_particle_type->getBoxSizeX()/2.0f;
    const float box_size_y = m_particle_type->getBoxSizeY()/2.0f;

    emitter->setBox( core::aabbox3df(-box_size_x, -box_size_y, -0.6f,
                                     box_size_x,  box_size_y,  -0.6f - size) );

}

#endif   // !SERVER_ONLY
