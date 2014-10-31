//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013  Joerg Henrichs, Marianne Gagnon
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
#include "graphics/shaders.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/helpers.hpp"
#include "graphics/gpuparticles.hpp"

#include <SParticle.h>
#include <IParticleAffector.h>
#include <ICameraSceneNode.h>
#include <IParticleSystemSceneNode.h>
#include <IParticleBoxEmitter.h>
#include <ISceneManager.h>
#include <algorithm>

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

class WindAffector : public scene::IParticleAffector
{
    /** (Squared) distance from camera at which a particle is completely faded out */
    float m_speed;
    float m_seed;

public:
    WindAffector(float speed): m_speed(speed)
    {
        m_seed = (float)((rand() % 1000) - 500);
    }

    // ------------------------------------------------------------------------

    virtual void affect(u32 now, scene::SParticle* particlearray, u32 count)
    {
        const float time = irr_driver->getDevice()->getTimer()->getTime() / 10000.0f;
        core::vector3df dir = irr_driver->getWind();
        dir *= m_speed * std::min(noise2d(time, m_seed), -0.2f);

        for (u32 n = 0; n < count; n++)
        {
            scene::SParticle& cur = particlearray[n];

            cur.pos += dir;
        }   // for n<count
    }   // affect

    // ------------------------------------------------------------------------

    virtual scene::E_PARTICLE_AFFECTOR_TYPE getType() const
    {
        // FIXME: this method seems to make sense only for built-in affectors
        return scene::EPAT_FADE_OUT;
    }

};   // WindAffector

// ============================================================================

class ScaleAffector : public scene::IParticleAffector
{
public:
    ScaleAffector(const core::vector2df& scaleFactor = core::vector2df(1.0f, 1.0f)) : ScaleFactor(scaleFactor)
    {
    }

    virtual void affect(u32 now, scene::SParticle *particlearray, u32 count)
    {
        for (u32 i = 0; i<count; i++)
        {
            const u32 maxdiff = particlearray[i].endTime - particlearray[i].startTime;
            const u32 curdiff = now - particlearray[i].startTime;
            const f32 timefraction = (f32)curdiff / maxdiff;
            core::dimension2df destsize = particlearray[i].startSize * ScaleFactor;
            particlearray[i].size = particlearray[i].startSize + (destsize - particlearray[i].startSize) * timefraction;
        }
    }

    virtual scene::E_PARTICLE_AFFECTOR_TYPE getType() const
    {
        return scene::EPAT_SCALE;
    }

protected:
    core::vector2df ScaleFactor;
};

// ============================================================================

class ColorAffector : public scene::IParticleAffector
{
protected:
    core::vector3df m_color_from;
    core::vector3df m_color_to;

public:
    ColorAffector(const core::vector3df& colorFrom, const core::vector3df& colorTo) :
        m_color_from(colorFrom), m_color_to(colorTo)
    {
    }

    virtual void affect(u32 now, scene::SParticle *particlearray, u32 count)
    {
        for (u32 i = 0; i<count; i++)
        {
            const u32 maxdiff = particlearray[i].endTime - particlearray[i].startTime;
            const u32 curdiff = now - particlearray[i].startTime;
            const f32 timefraction = (f32)curdiff / maxdiff;
            core::vector3df curr_color = m_color_from + (m_color_to - m_color_from)* timefraction;
            particlearray[i].color = video::SColor(255, (int)curr_color.X, (int)curr_color.Y, (int)curr_color.Z);
        }
    }

    virtual scene::E_PARTICLE_AFFECTOR_TYPE getType() const
    {
        return scene::EPAT_SCALE;
    }

};



// ============================================================================

ParticleEmitter::ParticleEmitter(const ParticleKind* type,
                                 const Vec3 &position,
                                 scene::ISceneNode* parent,
                                 bool randomize_initial_y)
               : m_position(position)
{
    assert(type != NULL);
    m_magic_number        = 0x58781325;
    m_node                = NULL;
    m_emitter             = NULL;
    m_particle_type       = NULL;
    m_parent              = parent;
    m_emission_decay_rate = 0;
    m_is_glsl = irr_driver->isGLSL();
    m_randomize_initial_y = randomize_initial_y;


    setParticleType(type);
    assert(m_node != NULL);

}   // KartParticleSystem

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

#if 0
    // FIXME: to work around irrlicht bug, when an emitter is paused by setting the rate
    //        to 0 results in a massive emission when enabling it back. In irrlicht 1.8
    //        the node has a method called "clearParticles" that should be cleaner than this

    if (f <= 0.0f && m_node->getEmitter())
    {
        m_node->clearParticles();
    }
    else if (m_node->getEmitter() == NULL)
    {
        m_node->setEmitter(m_emitter);
    }
#endif
/*    if (f <= 0.0f)
    {
        m_node->setVisible(false);
    }
    else
    {
        m_node->setVisible(true);
    }*/
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

void ParticleEmitter::clearParticles()
{
    m_node->clearParticles();
}

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
            if (m_is_glsl)
                m_node = ParticleSystemProxy::addParticleNode(m_is_glsl, type->randomizeInitialY());
            else
                m_node = irr_driver->addParticleNode();
            
            if (m_is_glsl)
            {
                bool additive = (type->getMaterial()->getShaderType() == Material::SHADERTYPE_ADDITIVE);
                static_cast<ParticleSystemProxy *>(m_node)->setAlphaAdditive(additive);
            }
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
            std::string help = file_manager->getAsset(FileManager::GUI, "main_help.png");
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

    #if VISUALIZE_BOX_EMITTER
                if (m_parent != NULL)
                {
                    for (int x=0; x<2; x++)
                    {
                        for (int y=0; y<2; y++)
                        {
                            for (int z=0; z<2; z++)
                            {
                                m_visualisation.push_back(
                                irr_driver->getSceneManager()->addSphereSceneNode(0.05f, 16, m_parent, -1,
                                                                                   core::vector3df((x ? box_size_x : -box_size_x),
                                                                                                   (y ? box_size_y : -box_size_y),
                                                                                                   -0.6 - (z ? 0 : type->getBoxSizeZ())))
                                                          );
                            }
                        }
                    }
                }
    #endif
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

        if (type->hasScaleAffector())
        {
            if (m_is_glsl)
            {
                static_cast<ParticleSystemProxy *>(m_node)->setIncreaseFactor(type->getScaleAffectorFactorX());
            }
            else
            {
                core::vector2df factor = core::vector2df(type->getScaleAffectorFactorX(),
                    type->getScaleAffectorFactorY());
                scene::IParticleAffector* scale_affector = new ScaleAffector(factor);
                m_node->addAffector(scale_affector);
                scale_affector->drop();
            }
        }

        if (type->getMinColor() != type->getMaxColor())
        {
            if (m_is_glsl)
            {
                video::SColor color_from = type->getMinColor();
                static_cast<ParticleSystemProxy *>(m_node)->setColorFrom(color_from.getRed() / 255.0f,
                    color_from.getGreen() / 255.0f,
                    color_from.getBlue() / 255.0f);

                video::SColor color_to = type->getMaxColor();
                static_cast<ParticleSystemProxy *>(m_node)->setColorTo(color_to.getRed() / 255.0f,
                    color_to.getGreen() / 255.0f,
                    color_to.getBlue() / 255.0f);
            }
            else
            {
                video::SColor color_from = type->getMinColor();
                core::vector3df color_from_v =
                    core::vector3df(float(color_from.getRed()),
                                    float(color_from.getGreen()),
                                    float(color_from.getBlue()));

                video::SColor color_to = type->getMaxColor();
                core::vector3df color_to_v = core::vector3df(float(color_to.getRed()),
                                                             float(color_to.getGreen()),
                                                             float(color_to.getBlue()));

                ColorAffector* affector = new ColorAffector(color_from_v, color_to_v);
                m_node->addAffector(affector);
                affector->drop();
            }
        }

        const float windspeed = type->getWindSpeed();
        if (windspeed > 0.01f)
        {
            WindAffector *waf = new WindAffector(windspeed);
            m_node->addAffector(waf);
            waf->drop();

            // TODO: wind affector for GLSL particles
        }

        const bool flips = type->getFlips();
        if (flips)
        {
            if (m_is_glsl)
                static_cast<ParticleSystemProxy *>(m_node)->setFlip();
        }
    }
}   // setParticleType

//-----------------------------------------------------------------------------

void ParticleEmitter::addHeightMapAffector(Track* t)
{
    
    if (m_is_glsl)
    {
        const Vec3* aabb_min;
        const Vec3* aabb_max;
        t->getAABB(&aabb_min, &aabb_max);
        float track_x = aabb_min->getX();
        float track_z = aabb_min->getZ();
        const float track_x_len = aabb_max->getX() - aabb_min->getX();
        const float track_z_len = aabb_max->getZ() - aabb_min->getZ();
        static_cast<ParticleSystemProxy *>(m_node)->setHeightmap(t->buildHeightMap(),
            track_x, track_z, track_x_len, track_z_len);
    }
    else
    {
        HeightMapCollisionAffector* hmca = new HeightMapCollisionAffector(t);
        m_node->addAffector(hmca);
        hmca->drop();
    }
}

//-----------------------------------------------------------------------------

void ParticleEmitter::resizeBox(float size)
{
    scene::IParticleBoxEmitter* emitter = (scene::IParticleBoxEmitter*)m_emitter;

    const float box_size_x = m_particle_type->getBoxSizeX()/2.0f;
    const float box_size_y = m_particle_type->getBoxSizeY()/2.0f;


    emitter->setBox( core::aabbox3df(-box_size_x, -box_size_y, -0.6f,
                                     box_size_x,  box_size_y,  -0.6f - size) );

#if VISUALIZE_BOX_EMITTER
    if (m_parent != NULL)
    {
        int n = 0;
        for (int x=0; x<2; x++)
        {
            for (int y=0; y<2; y++)
            {
                for (int z=0; z<2; z++)
                {
                    m_visualisation[n]->setPosition(
                                                    core::vector3df((x ? box_size_x : -box_size_x),
                                                                    (y ? box_size_y : -box_size_y),
                                                                    -0.6 - (z ? 0 : size))
                                                    );
                    n++;
                }
            }
        }
    }
#endif
}
