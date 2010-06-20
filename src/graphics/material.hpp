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

#include <string>

#include "irrlicht.h"
using namespace irr;

class XMLNode;
class SFXBase;

/**
  * \ingroup graphics
  */
class Material
{
public:
    enum GraphicalEffect {GE_NONE, GE_SMOKE, GE_WATER};

private:
    video::ITexture *m_texture;
    unsigned int     m_index;
    std::string      m_texname;
    /** Name of a special sfx to play when a kart is on this terrain, or
     *  "" if no special sfx exists. */
    std::string      m_sfx_name;
    GraphicalEffect  m_graphical_effect;
    bool             m_zipper;
    bool             m_resetter;
    bool             m_ignore;
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
    
    /** True if lightmapping is enabled for this material. */
    bool             m_lightmap;
    float            m_friction;
    /** How much the top speed is reduced per second. */
    float            m_slowdown;
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

    void  init    (unsigned int index);
    void  install (bool is_full_path=false);
    void  initCustomSFX(const XMLNode *sfx);
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
    bool  isZipper           () const { return m_zipper;             }
    bool  isSphereMap        () const { return m_sphere_map;         }
    bool  isReset            () const { return m_resetter;           }
    float getFriction        () const { return m_friction;           }
    const std::string& 
          getTexFname        () const { return m_texname;            }
    int   getIndex           () const { return m_index;              }
    /** Returns the fraction of maximum speed on this material. */
    float getMaxSpeedFraction() const { return m_max_speed_fraction; }
    /** Returns the slowdown that happens if a kart is
     *  faster than the maximum speed. */
    float getSlowDown        () const { return m_slowdown;           }
    /** Returns true if this material should have smoke effect. */
    bool  hasSmoke           () const { return m_graphical_effect==GE_SMOKE;}
    /** Returns true if this material should have water splashes. */
    bool hasWaterSplash      () const { return m_graphical_effect==GE_WATER;}
    /** Returns the name of a special sfx to play while a kart is on this
     *  terrain. The string will be "" if no special sfx exists. */
    const std::string &
         getSFXName          () const { return m_sfx_name; }
} ;


#endif

/* EOF */

