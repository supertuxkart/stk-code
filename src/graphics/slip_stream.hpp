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
    namespace scene { class ISceneNode; class IAnimatedMesh; }
}
using namespace irr;

namespace SP
{
    class SPDynamicDrawCall;
    class SPMesh;
}

#include "graphics/moving_texture.hpp"
#include "utils/no_copy.hpp"
#include <memory>

class AbstractKart;
class Quad;
class Material;

/**
  * \ingroup graphics
  */
class SlipStream
{
private:
    /** The kart to which this smoke belongs. */
    AbstractKart *m_kart;

    /** The moving texture for the normal node */

    MovingTexture *m_moving;

    /** The moving texture for the fast node */

    MovingTexture *m_moving_fast;

    /** The moving texture for the fast node */

    MovingTexture *m_moving_bonus;

    /** The scene node. */
    scene::ISceneNode *m_node;

    /** The fast scene node. */
    scene::ISceneNode *m_node_fast;

    /** The node used when the bonus is active. */
    scene::ISceneNode *m_bonus_node;

    /** For debugging: a simple quad to display where slipstream works. */
    std::shared_ptr<SP::SPDynamicDrawCall> m_debug_dc;

    /** For debugging: a simple quad to display where inner slipstream works. */
    std::shared_ptr<SP::SPDynamicDrawCall> m_debug_dc2;

    /** The length of the slipstream cylinder. This is used to scale
     *  the actual scene node correctly. Shared between node and node_fast */
    float         m_length;

    /** The time a kart was in slipstream. */
    float         m_slipstream_time;

    /** The remaining active time bonus */
    float         m_bonus_time;

    /** This bool is used to know the first time we're going out of the slipstreaming area */
    bool          m_bonus_active;

    /** Used to trigger automatically the slipstreaming bonus */

    int          m_current_target_id;
    int          m_previous_target_id;
    int          m_speed_increase_ticks;
    int          m_speed_increase_duration;

    /** Slipstream mode: either nothing happening, or the kart is collecting
     *  'slipstream credits'. Credits can be accumulated while the bonus is used */
    enum         {SS_NONE, SS_COLLECT} m_slipstream_mode;

    /** This is slipstream area if the kart is at 0,0,0 without rotation. */
    Quad         *m_slipstream_quad;

    /** This is the inner slipstream area if the kart is at 0,0,0 without rotation. */
    Quad         *m_slipstream_inner_quad;

    /** This is the outer slipstream area if the kart is at 0,0,0 without rotation. 
        No slipstream time is accumulated there, but it's lost slower*/
    Quad         *m_slipstream_outer_quad;

    /** The kart from which this kart gets slipstream. Used by the AI to
     ** overtake the right kart. */
    AbstractKart* m_target_kart;

    SP::SPMesh*  createMeshSP(unsigned material_id, bool bonus_mesh);
    scene::IAnimatedMesh* createMesh(unsigned material_id, bool bonus_mesh);

    void         setDebugColor(const video::SColor &color, bool inner);
    void         updateQuad();
    void         updateSlipstreamingTextures(float f, const AbstractKart* kart);
    void         updateBonusTexture();
    void         hideAllNodes();
public:
                 SlipStream  (AbstractKart* kart);
                 ~SlipStream  ();
    void         reset();
    void         update(int ticks);
    bool         isSlipstreamReady() const;
    void         updateSpeedIncrease();
    // ------------------------------------------------------------------------
    /** Returns the quad in which slipstreaming is effective for
     *  this kart. */
    const Quad& getSlipstreamQuad() const { return *m_slipstream_quad; }
    // ------------------------------------------------------------------------
    /** Returns the kart from which slipstream is 'collected'. */
    const AbstractKart* getSlipstreamTarget() const {return m_target_kart;}
    // ------------------------------------------------------------------------
    /** Returns if slipstream is being used. */
    bool        inUse() const {return m_bonus_time>0.0f; }
};   // SlipStream
#endif
