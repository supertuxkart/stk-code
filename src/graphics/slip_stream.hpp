//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

#ifndef HEADER_SLIP_STREAM_HPP
#define HEADER_SLIP_STREAM_HPP

#include <matrix4.h>
namespace irr
{
    namespace video { class SMaterial; class SColor; }
    namespace scene { class IMeshSceneNode; class IMesh; class IMesh; }
}
using namespace irr;

#include "graphics/moving_texture.hpp"
#include "utils/no_copy.hpp"

class Kart;
class Quad;

/**
  * \ingroup graphics
  */
class SlipStream : public MovingTexture
{
private:
    /** The kart to which this smoke belongs. */
    Kart              *m_kart;

    /** The scene node. */
    scene::IMeshSceneNode *m_node;

    /** The actual mesh. */
    scene::IMesh      *m_mesh;

    /** For debugging: display where slipstream works. */
    scene::IMeshSceneNode *m_debug_node;

    /** For debugging: a simple quad to display where slipstream works. */
    scene::IMesh      *m_debug_mesh;

    /** The texture matrix for the slipstream effect. */
    core::matrix4     *m_matrix;

    /** The length of the slipstream cylinder. This is used to scale
     *  the actual scene node correctly. */
    float              m_length;

    /** The time a kart was in slipstream. */
    float         m_slipstream_time;
    
    /** Slipstream mode: either nothing happening, or the kart is collecting
     *  'slipstream credits', or the kart is using accumulated credits. */
    enum         {SS_NONE, SS_COLLECT, SS_USE} m_slipstream_mode;

    /** The quad inside which another kart is considered to be slipstreaming.
     *  This value is current area, i.e. takes the kart position into account. */
    Quad         *m_slipstream_quad;

    /** This is slipstream area if the kart is at 0,0,0 without rotation. From 
     *  this value m_slipstream_area is computed by applying the kart transform. */
    Quad         *m_slipstream_original_quad;

    /** The kart from which this kart gets slipstream. Used by the AI to
     ** overtake the right kart. */
    Kart         *m_target_kart;

    void         createMesh(const video::SMaterial &m);
    void         setDebugColor(const video::SColor &color);
public:
                 SlipStream  (Kart* kart);
    virtual     ~SlipStream  ();
    void         reset();
    virtual void update(float dt);
    void         setIntensity(float f, const Kart* kart);
    float        getSlipstreamPower();
    bool         isSlipstreamReady() const;

    // ------------------------------------------------------------------------
    /** Returns the quad in which slipstreaming is effective for
     *  this kart. */
    const Quad& getSlipstreamQuad() const { return *m_slipstream_quad; }
    // ------------------------------------------------------------------------
    /** Returns the kart from which slipstream is 'collected'. */
    const Kart* getSlipstreamTarget() const {return m_target_kart;}
    // ------------------------------------------------------------------------
    /** Returns if slipstream is being used. */
    bool        inUse() const {return m_slipstream_mode==SS_USE; }
};   // SlipStream
#endif

