//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Joerg Henrichs
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

class AbstractKart;
class Quad;
class Material;

/**
  * \ingroup graphics
  */
class SlipStream : public MovingTexture
{
private:
    /** The kart to which this smoke belongs. */
    AbstractKart *m_kart;

    /** The scene node. */
    scene::IMeshSceneNode *m_node;

    /** The actual mesh. */
    scene::IMesh      *m_mesh;

    /** For debugging: display where slipstream works. */
    scene::IMeshSceneNode *m_debug_node;

    /** For debugging: a simple quad to display where slipstream works. */
    scene::IMesh      *m_debug_mesh;

    /** The length of the slipstream cylinder. This is used to scale
     *  the actual scene node correctly. */
    float              m_length;

    /** The time a kart was in slipstream. */
    float         m_slipstream_time;

    /** Slipstream mode: either nothing happening, or the kart is collecting
     *  'slipstream credits', or the kart is using accumulated credits. */
    enum         {SS_NONE, SS_COLLECT, SS_USE} m_slipstream_mode;

    /** This is slipstream area if the kart is at 0,0,0 without rotation. */
    Quad         *m_slipstream_quad;

    /** The kart from which this kart gets slipstream. Used by the AI to
     ** overtake the right kart. */
    AbstractKart* m_target_kart;

    void         createMesh(Material* material);
    void         setDebugColor(const video::SColor &color);
public:
                 SlipStream  (AbstractKart* kart);
    virtual     ~SlipStream  ();
    void         reset();
    virtual void update(float dt);
    void         setIntensity(float f, const AbstractKart* kart);
    void         updateSlipstreamPower();
    bool         isSlipstreamReady() const;

    // ------------------------------------------------------------------------
    /** Returns the quad in which slipstreaming is effective for
     *  this kart. */
    const Quad& getSlipstreamQuad() const { return *m_slipstream_quad; }
    // ------------------------------------------------------------------------
    /** Returns the kart from which slipstream is 'collected'. */
    const AbstractKart* getSlipstreamTarget() const {return m_target_kart;}
    // ------------------------------------------------------------------------
    /** Returns if slipstream is being used. */
    bool        inUse() const {return m_slipstream_mode==SS_USE; }
};   // SlipStream
#endif

