//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2010-2015 Steve Baker, Joerg Henrichs
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
#include "utils/random_generator.hpp"

#include <array>
#include <assert.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace irr
{
    namespace video { class ITexture; class IImage; class SMaterial; }
    namespace scene { class ISceneNode; class IMeshBuffer; }
}
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
        PUSH_BACK,
        PUSH_SOCCER_BALL
    };

private:

    /** Pointer to the texture. */
    video::ITexture *m_texture;

    std::array<video::ITexture*, 4> m_vk_textures;

    /** Name of the texture. */
    std::string      m_texname;

    std::string      m_full_path;

    /** Name of a special sfx to play when a kart is on this terrain, or
     *  "" if no special sfx exists. */
    std::string      m_sfx_name;

    /** Either ' ' (no mirroring), 'U' or 'V' if a texture needs to be
     *  mirrored when driving in reverse. Typically used for arrows indicating
     *  the direction. */
    char             m_mirror_axis_when_reverse;

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

    /** True if this is a texture that will start the jump animation when
     *  leaving it and being in the air. */
    bool             m_is_jump_texture;

    /** True if driving on this texture should adjust the gravity of the kart
     *  to be along the normal of the triangle. This allows karts to drive e.g
     *  upside down. */
    bool             m_has_gravity;

    /** If the property should be ignored in the physics. Example would be
     *  plants that a kart can just drive through. */
    bool             m_ignore;

    /** True if the material shouldn't be "slippy" at an angle */
    bool             m_high_tire_adhesion;

    bool  m_complain_if_not_found;

    bool  m_deprecated;

    bool  m_installed;

    /** True if this material can be colorized (like red/blue in team game). */
    bool             m_colorizable;

    /** True if this material should use texture compression. */
    bool             m_tex_compression;

    /** Minimum resulting saturation when colorized (from 0 to 1) */
    float            m_colorization_factor;

    /** If a kart is rescued when crashing into this surface. */
    CollisionReaction m_collision_reaction;

    /** Particles to show on touch */
    std::string      m_collision_particles;

    /** 
    * Associated with m_mirror_axis_when_reverse, to avoid mirroring the same material twice
    * (setAllMaterialFlags can be called multiple times on the same mesh buffer)
    */
    std::map<void*, bool> m_mirrorred_mesh_buffers;

    ParticleKind*    m_particles_effects[EMIT_KINDS_COUNT];

    /** Texture clamp bitmask */
    unsigned int     m_clamp_tex;

    /** List of hue pre-defined for colorization (from 0 to 1) */
    std::vector<float> m_hue_settings;

    /** Random generator for getting pre-defined hue */
    RandomGenerator m_random_hue;

    /** How much the top speed is reduced per second. */
    int              m_slowdown_ticks;

    /** Maximum speed at which no more slow down occurs. */
    float            m_max_speed_fraction;

    /** Minimum speed on this terrain. This is used for zippers on a ramp to
     *  guarantee the right jump distance. A negative value indicates no
     *  minimum speed. */
    float            m_zipper_min_speed;
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
    /** Additional engine force. */
    float            m_zipper_engine_force;
    
    std::string      m_mask;

    std::string      m_colorization_mask;

    void  init    ();
    void  install (std::function<void(video::IImage*)> image_mani = nullptr,
                   video::SMaterial* m = NULL);
    void  initCustomSFX(const XMLNode *sfx);
    void  initParticlesEffect(const XMLNode *node);

    // SP usage
    std::string      m_shader_name;
    std::string      m_uv_two_tex;
    // Full path for textures in sp shader
    std::array<std::string, 6> m_sampler_path;
    std::string      m_container_id;
    void loadContainerId();

public:
          Material(const XMLNode *node, bool deprecated);
          Material(const std::string& fname,
                   bool is_full_path=false,
                   bool complain_if_not_found=true,
                   bool load_texture = true,
                   const std::string& shader_name = "solid");
         ~Material ();

    void unloadTexture();

    void  setSFXSpeed(SFXBase *sfx, float speed, bool should_be_paused) const;
    void  setMaterialProperties(video::SMaterial *m, scene::IMeshBuffer* mb);

    /** Returns the ITexture associated with this material. */
    video::ITexture *getTexture(bool srgb = true, bool premul_alpha = false);
    // ------------------------------------------------------------------------
    bool  isIgnore           () const { return m_ignore;             }
    // ------------------------------------------------------------------------
    /** Returns true if this material is a zipper. */
    bool  isZipper           () const { return m_zipper;             }
    // ------------------------------------------------------------------------
    /** Returns if this material should trigger a rescue if a kart
     *  is driving on it. */
    bool  isDriveReset       () const { return m_drive_reset;        }
    // ------------------------------------------------------------------------
    /** Returns if this material can be colorized.
     */
    bool  isColorizable      () const { return m_colorizable;        }
    // ------------------------------------------------------------------------
    /** Returns the minimum resulting saturation when colorized.
     */
    float getColorizationFactor () const { return m_colorization_factor;   }
    // ------------------------------------------------------------------------
    bool hasRandomHue() const            { return !m_hue_settings.empty(); }
    // ------------------------------------------------------------------------
    /** Returns a random hue when colorized.
     */
    float getRandomHue()
    {
        if (m_hue_settings.empty())
            return 0.0f;
        const unsigned int hue = m_random_hue.get((int)m_hue_settings.size());
        assert(hue < m_hue_settings.size());
        return m_hue_settings[hue];
    }
    // ------------------------------------------------------------------------
    /** Returns if this material should trigger a rescue if a kart
     *  crashes against it. */
    CollisionReaction  getCollisionReaction() const { return m_collision_reaction; }

    // ------------------------------------------------------------------------
    std::string getCrashResetParticles() const { return m_collision_particles; }

    // ------------------------------------------------------------------------
    bool  highTireAdhesion   () const { return m_high_tire_adhesion; }
    // ------------------------------------------------------------------------
    const std::string&
          getTexFname        () const { return m_texname;            }
    // ------------------------------------------------------------------------
    const std::string&
          getTexFullPath     () const { return m_full_path;          }

    // ------------------------------------------------------------------------
    bool  isTransparent      () const
    {
        return m_shader_name == "additive" || m_shader_name == "alphablend" ||
               m_shader_name == "displace";
    }

    // ------------------------------------------------------------------------
    bool  useAlphaChannel    () const
    {
        return isTransparent() || m_shader_name == "alphatest" ||
               m_shader_name == "unlit" || m_shader_name == "grass";
    }

    // ------------------------------------------------------------------------
    /** Returns the fraction of maximum speed on this material. */
    float getMaxSpeedFraction() const { return m_max_speed_fraction; }
    // ------------------------------------------------------------------------
    /** Returns how long it will take for a slowdown to take effect.
     *  It is the time it takes till the full slowdown applies to
     *  karts. So a short time will slowdown a kart much faster. */
    int getSlowDownTicks() const { return m_slowdown_ticks;          }
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
    const std::string &getSFXName() const { return m_sfx_name; }
    // ------------------------------------------------------------------------
    /** \brief Get the kind of particles that are to be used on this material,
     *  in the given conditions.
     * \return The particles to use, or NULL if none. */
    const ParticleKind* getParticlesWhen(ParticleConditions cond) const
    {
        return m_particles_effects[cond]; 
    }   // getParticlesWhen
    // ------------------------------------------------------------------------
    /** Returns true if a kart falling over this kind of material triggers
     *  the special falling camera. */
    bool hasFallingEffect() const {return m_falling_effect; }
    // ------------------------------------------------------------------------
    /** Returns if being in the air after this texture should start the
     *  jump animation. */
    bool isJumpTexture() const { return m_is_jump_texture; }
    // ------------------------------------------------------------------------
    /** Returns true if this texture adjusts the gravity vector of the kart
     *  to be parallel to the normal of the triangle - which allows karts to
     *  e.g. drive upside down. */
    bool hasGravity() const { return m_has_gravity; }
    // ------------------------------------------------------------------------
    /** Returns the zipper parametersfor the current material. */
    void getZipperParameter(float *zipper_max_speed_increase,
                             float *zipper_duration,
                             float *zipper_speed_gain,
                             float *zipper_fade_out_time,
                             float *zipper_engine_force) const
    {
        *zipper_max_speed_increase = m_zipper_max_speed_increase;
        *zipper_duration           = m_zipper_duration;
        *zipper_speed_gain         = m_zipper_speed_gain;
        *zipper_fade_out_time      = m_zipper_fade_out_time;
        *zipper_engine_force       = m_zipper_engine_force;
    }   // getZipperParameter
    // ------------------------------------------------------------------------
    /** Returns the minimum speed of a kart on this material. This is used
     *  for zippers on a ramp to guarantee the right jump distance even
     *  on lower speeds. A negative value indicates no minimum speed. */
    float getZipperMinSpeed() const { return m_zipper_min_speed; }
    // ------------------------------------------------------------------------
    /** True if this texture should have the U coordinates mirrored. */
    char getMirrorAxisInReverse() const { return m_mirror_axis_when_reverse; }
    // ------------------------------------------------------------------------
    const std::string& getAlphaMask() const                 { return m_mask; }
    // ------------------------------------------------------------------------
    const std::string& getColorizationMask() const
                                               { return m_colorization_mask; }
    // ------------------------------------------------------------------------
    void setShaderName(const std::string& name)      { m_shader_name = name; }
    // ------------------------------------------------------------------------
    const std::string& getShaderName() const         { return m_shader_name; }
    // ------------------------------------------------------------------------
    /* This is used for finding correct material for spm*/
    const std::string& getUVTwoTexture() const
                                                      { return m_uv_two_tex; }
    // ------------------------------------------------------------------------
    bool use2UV() const                      { return !m_uv_two_tex.empty(); }
    // ------------------------------------------------------------------------
    const std::string& getSamplerPath(unsigned layer) const
    {
        assert(layer < 6);
        return m_sampler_path[layer];
    }
    // ------------------------------------------------------------------------
    /* Return the container id used by texture compression to cache texture in
     * cache folder with unique name, if no texture compression is used for
     * this material, then it will always return empty. */
    const std::string& getContainerId() const
    {
        static std::string empty;
        if (!m_tex_compression)
            return empty;
        return m_container_id;
    }
    // ------------------------------------------------------------------------
    std::function<void(irr::video::IImage*)> getMaskImageMani() const;
};


#endif

/* EOF */

