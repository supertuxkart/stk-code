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

class Material
{
public:
    enum GraphicalEffect {GE_NONE, GE_SMOKE, GE_WATER};

private:
    video::ITexture *m_texture;
    unsigned int     m_index;
    std::string      m_texname;
    GraphicalEffect  m_graphical_effect;
    bool             m_collideable;
    bool             m_zipper;
    bool             m_resetter;
    bool             m_ignore;
    int              m_clamp_tex;
    bool             m_lighting;
    bool             m_sphere_map;
    bool             m_transparency;
    bool             m_alpha_blending;
    float            m_friction;
    /** How much the top speed is reduced per second. */
    float            m_slowdown;
    /** Maximum speed at which no more slow down occurs. */
    float            m_max_speed_fraction;
    void init    (unsigned int index);
    void install (bool is_full_path=false);

public:
          Material(const XMLNode *node, int index);
          Material(const std::string& fname, int index, 
                   bool is_full_path=false);
         ~Material ();

    void  setMaterialProperties(video::SMaterial *m) const;
    /** Returns the ITexture associated with this material. */
    video::ITexture *getTexture() const   { return m_texture;        }
    bool  isIgnore           () const { return m_ignore;             }
    bool  isZipper           () const { return m_zipper;             }
    bool  isSphereMap        () const { return m_sphere_map;         }
    bool  isCrashable        () const { return m_collideable;        }
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
} ;


#endif

/* EOF */

