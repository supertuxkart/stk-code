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

#ifndef HEADER_MATERIAL_HPP
#define HEADER_MATERIAL_HPP

#include "utils/no_copy.hpp"

#include <string>

#include "irrlicht.h"
using namespace irr;

class XMLNode;
class SFXBase;
class ParticleKind;

/**
  * \ingroup graphics
  */
class Material : public NoCopy
{
public:
    enum GraphicalEffect {GE_NONE, GE_WATER};

    enum ParticleConditions
    {
        EMIT_ON_DRIVE = 0,
        EMIT_ON_SKID,
        
        EMIT_KINDS_COUNT
    };
    
private:
    video::ITexture *m_texture;
    unsigned int     m_index;
    std::string      m_texname;
    /** Name of a special sfx to play when a kart is on this terrain, or
     *  "" if no special sfx exists. */
    std::string      m_sfx_name;
    GraphicalEffect  m_graphical_effect;
    /** Set if being on this surface means being under some other mesh.
     *  This is used to simulate that a kart is in water: the ground under
     *  the water is marked as 'm_below_surface', which will then trigger a raycast
     *  up to find the position of the actual water surface. */
    bool             m_below_surface;
    /** A material that is a surface only, i.e. the karts can fall through
     *  but the information is still needed (for GFX mostly). An example is
     *  a water surface: karts can drive while partly in water (so the water
     *  surface is not a physical object), but the location of the water
     *  effect is on the surface. */
    bool             m_surface;
    /** If the material is a zipper, i.e. gives a speed boost. */
    bool             m_zipper;
    /** If a kart is rescued when touching this surface. */
    bool             m_resetter;
    /** If the property should be ignored in the physics. Example would be
     *  plants that a kart can just drive through. */
    bool             m_ignore;
    bool             m_add;
    
    ParticleKind*    m_particles_effects[EMIT_KINDS_COUNT];
    
    /** Texture clamp bitmask */
    unsigned int     m_clamp_tex;
    bool             m_lighting;
    bool             m_sphere_map;
    bool             m_alpha_testing;
    bool             m_alpha_blending;

    /** True if backface culliing should be enabled. */
    bool             m_backface_culling;
     
    /** Whether to use anisotropic filtering for this texture */
    bool             m_anisotropic;
    
    /** Set to true to disable writing to the Z buffer. Usually to be used with alpha blending */
    bool             m_disable_z_write;
    
    /** True if lightmapping is enabled for this material. */
    bool             m_lightmap;
    float            m_friction;
    /** How much the top speed is reduced per second. */
    float            m_slowdown_time;
    /** Maximum speed at which no more slow down occurs. */
    float            m_max_speed_fraction;
    /** The minimum speed at which a special sfx is started to be played. */
    float            m_sfx_min_speed;
    /** The speed at which the maximum pitch is used. */
    float            m_sfx_max_speed;
    /** The minimum pitch to be used (at minimum speed). */
    float            m_sfx_min_pitch;
    /** The maximum pitch to be used (at maximum speed). */
    float            m_sfx_max_pitch;
    /** (max_pitch-min_pitch) / (max_speed - min_speed). Used to adjust
     *  the pitch of a sfx depending on speed of the kart.
     */
    float            m_sfx_pitch_per_speed;
    /** Additional speed allowed on top of the kart-specific maximum kart speed
     *  if a zipper is used. If this value is <0 the kart specific value will
     *  be used. */
    float            m_zipper_max_speed_increase;
    /** Time a zipper stays activated. If this value is <0 the kart specific 
     *  value will be used. */
    float            m_zipper_duration;
    /** A one time additional speed gain - the kart will instantly add this 
     *  amount of speed to its current speed. If this value is <0 the kart
     *  specific value will be used. */
    float            m_zipper_speed_gain;
    /**  Time it takes for the zipper advantage to fade out. If this value 
     *  is <0 the kart specific value will be used. */
    float            m_zipper_fade_out_time;

    void  init    (unsigned int index);
    void  install (bool is_full_path=false);
    void  initCustomSFX(const XMLNode *sfx);
    void  initParticlesEffect(const XMLNode *node);
    
public:
          Material(const XMLNode *node, int index);
          Material(const std::string& fname, int index, 
                   bool is_full_path=false);
         ~Material ();

    void  setSFXSpeed(SFXBase *sfx, float speed) const;
    void  setMaterialProperties(video::SMaterial *m) const;
    /** Returns the ITexture associated with this material. */
    video::ITexture *getTexture() const   { return m_texture;        }
    bool  isIgnore           () const { return m_ignore;             }
    /** Returns true if this material is a zipper. */
    bool  isZipper           () const { return m_zipper;             }
    bool  isSphereMap        () const { return m_sphere_map;         }
    bool  isReset            () const { return m_resetter;           }
    float getFriction        () const { return m_friction;           }
    const std::string& 
          getTexFname        () const { return m_texname;            }
    int   getIndex           () const { return m_index;              }
    
    bool  isTransparent      () const { return m_alpha_testing || m_alpha_blending || m_add; }
    
    // ------------------------------------------------------------------------
    /** Returns the fraction of maximum speed on this material. */
    float getMaxSpeedFraction() const { return m_max_speed_fraction; }
    // ------------------------------------------------------------------------
    /** Returns how long it will take for a slowdown to take effect.
     *  It is the time it takes till the full slowdown applies to
     *  karts. So a short time will slowdown a kart much faster. */
    float getSlowDownTime() const { return m_slowdown_time;          }
    // ------------------------------------------------------------------------
    /** Returns true if this material should have smoke effect. */
    //bool  hasSmoke           () const { return m_graphical_effect==GE_SMOKE;}
    // ------------------------------------------------------------------------
    /** Returns true if this material is under some other mesh and therefore
     *  requires another raycast to find the surface it is under (used for
     *  gfx, e.g. driving under water to find where the water splash should
     *  be shown at. */
    bool isBelowSurface      () const { return m_below_surface; }
    // ------------------------------------------------------------------------
    /** Returns true if this material is a surface, i.e. it is going to be
     *  ignored for the physics, but the information is needed e.g. for 
     *  gfx. See m_below_surface for more details. */
    bool isSurface          () const { return m_surface; }
    // ------------------------------------------------------------------------
    /** Returns true if this material should have water splashes. */
    bool hasWaterSplash      () const { return m_graphical_effect==GE_WATER;}
    // ------------------------------------------------------------------------
    /** Returns the name of a special sfx to play while a kart is on this
     *  terrain. The string will be "" if no special sfx exists. */
    const std::string &
         getSFXName          () const { return m_sfx_name; }
    
    /**
      * \brief Get the kind of particles that are to be used on this material, in the given conditions
      * \return The particles to use, or NULL if none
      */
    const ParticleKind* getParticlesWhen(ParticleConditions cond) const { return m_particles_effects[cond]; }
    
    // ------------------------------------------------------------------------
    /** Returns the zipper parametersfor the current material. */
    void getZipperParameter(float *zipper_max_speed_increase,
                             float *zipper_duration,
                             float *zipper_speed_gain,
                             float *zipper_fade_out_time) const
    {
        *zipper_max_speed_increase = m_zipper_max_speed_increase;
        *zipper_duration           = m_zipper_duration;
        *zipper_speed_gain         = m_zipper_speed_gain;
        *zipper_fade_out_time      = m_zipper_fade_out_time;
    }   // getZipperParameter

} ;


#endif

/* EOF */

