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
#include <map>

#include <IShaderConstantSetCallBack.h>

#define LIGHTMAP_VISUALISATION 0


namespace irr
{
    namespace video { class ITexture; class SMaterial; }
    namespace scene { class ISceneNode; class IMeshBuffer; }
}
using namespace irr;

class XMLNode;
class SFXBase;
class ParticleKind;

class NormalMapProvider;
class SplattingProvider;
class BubbleEffectProvider;

/**
  * \ingroup graphics
  */
class Material : public NoCopy
{
public:
    enum GraphicalEffect {GE_NONE,
                          /** Water splash effect. This is set on the seabed material. */
                          GE_WATER,
                          /** Effect where the UV texture is moved in a wave pattern */
                          GE_BUBBLE};

    enum ParticleConditions
    {
        EMIT_ON_DRIVE = 0,
        EMIT_ON_SKID,
        
        EMIT_KINDS_COUNT
    };
    
    enum CollisionReaction
    {
        NORMAL,
        RESCUE,
        PUSH_BACK
    };
    
private:
    
    enum Shaders
    {
        NORMAL_MAP,
        SPLATTING,
        WATER_SHADER,
        SHADER_COUNT
    };
    
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

    /** If a kart is falling over a material with this flag set, it
     *  will trigger the special camera fall effect. */
    bool             m_falling_effect;
    /** A material that is a surface only, i.e. the karts can fall through
     *  but the information is still needed (for GFX mostly). An example is
     *  a water surface: karts can drive while partly in water (so the water
     *  surface is not a physical object), but the location of the water
     *  effect is on the surface. */
    bool             m_surface;
    /** If the material is a zipper, i.e. gives a speed boost. */
    bool             m_zipper;
    /** If a kart is rescued when driving on this surface. */
    bool             m_drive_reset;
    bool             m_water_shader;
    /** If a kart is rescued when crashing into this surface. */
    CollisionReaction m_collision_reaction;
    
    /** Particles to show on touch */
    std::string      m_collision_particles;
    
    /** If the property should be ignored in the physics. Example would be
     *  plants that a kart can just drive through. */
    bool             m_ignore;
    bool             m_add;
    
    bool             m_fog;
        
    ParticleKind*    m_particles_effects[EMIT_KINDS_COUNT];
    
    /** For normal maps */
    bool             m_normal_map;
    std::string      m_normal_map_tex;
    //bool             m_normal_map_uv2; //!< Whether to use a second UV layer for normal map
    bool             m_is_heightmap;
    bool             m_parallax_map;
    float            m_parallax_height;
    
    /** Texture clamp bitmask */
    unsigned int     m_clamp_tex;
    
    bool             m_lighting;
    bool             m_sphere_map;
    bool             m_alpha_testing;
    bool             m_alpha_blending;
    bool             m_alpha_to_coverage;

    /** True if backface culliing should be enabled. */
    bool             m_backface_culling;
    
    /** Set to true to disable writing to the Z buffer. Usually to be used with alpha blending */
    bool             m_disable_z_write;

    /** Some textures need to be pre-multiplied, some divided to give
     *  the intended effect. */
    enum             {ADJ_NONE, ADJ_PREMUL, ADJ_DIV}
                     m_adjust_image;
    /** True if (blending) lightmapping is enabled for this material. */
    bool             m_lightmap;
    /** True if (additive) lightmapping is enabled for this material. */
    bool             m_additive_lightmap;
    
    bool             m_high_tire_adhesion;
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

    std::string      m_mask;
    
    /** Whether to use splatting */
    bool             m_splatting;
    
    /** If m_splatting is true, indicates the first splatting texture */
    std::string      m_splatting_texture_1;
    
    /** If m_splatting is true, indicates the second splatting texture */
    std::string      m_splatting_texture_2;
    
    /** If m_splatting is true, indicates the third splatting texture */
    std::string      m_splatting_texture_3;   
    
    /** If m_splatting is true, indicates the fourth splatting texture */
    std::string      m_splatting_texture_4;
    
    irr::video::IShaderConstantSetCallBack* m_shaders[SHADER_COUNT];

    /** Only used if bubble effect is enabled */
    std::map<scene::IMeshBuffer*, BubbleEffectProvider*> m_bubble_provider;
    
    void  init    (unsigned int index);
    void  install (bool is_full_path=false, bool complain_if_not_found=true);
    void  initCustomSFX(const XMLNode *sfx);
    void  initParticlesEffect(const XMLNode *node);
    
public:
          Material(const XMLNode *node, int index);
          Material(const std::string& fname, int index, 
                   bool is_full_path=false,
                   bool complain_if_not_found=true);
         ~Material ();

    void  setSFXSpeed(SFXBase *sfx, float speed) const;
    void  setMaterialProperties(video::SMaterial *m, scene::IMeshBuffer* mb);
    void  adjustForFog(scene::ISceneNode* parent, video::SMaterial *m, bool use_fog) const;
    
    /** Returns the ITexture associated with this material. */
    video::ITexture *getTexture() const   { return m_texture;        }
    bool  isIgnore           () const { return m_ignore;             }
    /** Returns true if this material is a zipper. */
    bool  isZipper           () const { return m_zipper;             }
    bool  isSphereMap        () const { return m_sphere_map;         }
    /** Returns if this material should trigger a rescue if a kart
     *  is driving on it. */
    bool  isDriveReset       () const { return m_drive_reset;        }
    /** Returns if this material should trigger a rescue if a kart
     *  crashes against it. */
    CollisionReaction  getCollisionReaction() const { return m_collision_reaction; }
    
    std::string getCrashResetParticles() const { return m_collision_particles; }
    
    bool  highTireAdhesion   () const { return m_high_tire_adhesion; }
    const std::string& 
          getTexFname        () const { return m_texname;            }
    int   getIndex           () const { return m_index;              }
    
    bool  isTransparent      () const { return m_alpha_testing || m_alpha_blending || m_add; }
    
    // ------------------------------------------------------------------------
    /** Returns true if this materials need pre-multiply of alpha. */
    bool isPreMul() const {return m_adjust_image==ADJ_PREMUL; }
    // ------------------------------------------------------------------------
    /** Returns true if this materials need pre-division of alpha. */
    bool isPreDiv() const {return m_adjust_image==ADJ_DIV; }
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
    /** Returns the name of a special sfx to play while a kart is on this
     *  terrain. The string will be "" if no special sfx exists. */
    const std::string &
         getSFXName          () const { return m_sfx_name; }
    
    bool isFogEnabled() const { return m_fog; }
    
    /**
      * \brief Get the kind of particles that are to be used on this material, in the given conditions
      * \return The particles to use, or NULL if none
      */
    const ParticleKind* getParticlesWhen(ParticleConditions cond) const { return m_particles_effects[cond]; }
    
    // ------------------------------------------------------------------------
    /** Returns true if a kart falling over this kind of material triggers
     *  the special falling camera. */
    bool hasFallingEffect() const {return m_falling_effect; }
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

    bool isNormalMap() const { return m_normal_map; }
    
    void onMadeVisible(scene::IMeshBuffer* who);
    void onHidden(scene::IMeshBuffer* who);
    void isInitiallyHidden(scene::IMeshBuffer* who);
} ;


#endif

/* EOF */

