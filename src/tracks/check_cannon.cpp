//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#include "tracks/check_cannon.hpp"

#include "animations/animation_base.hpp"
#include "animations/ipo.hpp"
#include "config/user_config.hpp"
#include "graphics/show_curve.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "io/xml_node.hpp"
#include "items/flyable.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/cannon_animation.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"


/** Constructor for a check cannon.
 *  \param node XML node containing the parameters for this checkline.
 *  \param index Index of this check structure in the check manager.
 */
CheckCannon::CheckCannon(const XMLNode &node,  unsigned int index)
           : CheckLine(node, index)
{
    std::string p1("target-p1");
    std::string p2("target-p2");

    if (race_manager->getReverseTrack())
    {
        p1 = "p1";
        p2 = "p2";
    }

    if( !node.get(p1, &m_target_left ) || 
        !node.get(p2, &m_target_right)    )
        Log::fatal("CheckCannon", "No target line specified.");

    m_curve = new Ipo(*(node.getNode("curve")),
                      /*fps*/25,
                      /*reverse*/race_manager->getReverseTrack());

#if defined(DEBUG) && !defined(SERVER_ONLY)
    if(UserConfigParams::m_track_debug)
    {
        m_show_curve = new ShowCurve(0.5f, 0.5f);
        const std::vector<Vec3> &p = m_curve->getPoints();
        for(unsigned int i=0; i<p.size(); i++)
            m_show_curve->addPoint(p[i]);
    }
    if (UserConfigParams::m_check_debug)
    {
        m_debug_target_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP, SP::getSPShader("additive"),
            material_manager->getSPMaterial("additive"));
        SP::addDynamicDrawCall(m_debug_target_dy_dc);
        m_debug_target_dy_dc->getVerticesVector().resize(4);
        auto& vertices = m_debug_target_dy_dc->getVerticesVector();
        Vec3 height(0, 3, 0);
        vertices[0].m_position = m_target_left.toIrrVector();
        vertices[1].m_position = m_target_right.toIrrVector();
        vertices[2].m_position = Vec3(m_target_left  + height).toIrrVector();
        vertices[3].m_position = Vec3(m_target_right + height).toIrrVector();
        for (unsigned int i = 0; i < 4; i++)
        {
            vertices[i].m_color = m_active_at_reset
                ? video::SColor(128, 255, 0, 0)
                : video::SColor(128, 128, 128, 128);
        }
        m_debug_target_dy_dc->recalculateBoundingBox();
    }
#endif   // DEBUG AND !SERVER_ONLY

}   // CheckCannon

// ----------------------------------------------------------------------------
/** Destructor, frees the curve data (which the cannon animation objects only
 *  have a read-only copy of).
 */
CheckCannon::~CheckCannon()
{
    delete m_curve;
#if defined(DEBUG) && !defined(SERVER_ONLY)
    if(UserConfigParams::m_track_debug)
        delete m_show_curve;
    if (m_debug_target_dy_dc)
        m_debug_target_dy_dc->removeFromSP();
#endif
}   // ~CheckCannon

// ----------------------------------------------------------------------------
/** Changes the colour of a check cannon depending on state.
 */
void CheckCannon::changeDebugColor(bool is_active)
{
#if defined(DEBUG) && !defined(SERVER_ONLY)
    CheckLine::changeDebugColor(is_active);

    video::SColor color = is_active ? video::SColor(192, 255, 0, 0)
        : video::SColor(192, 128, 128, 128);
    for (unsigned int i = 0; i < 4; i++)
    {
        m_debug_target_dy_dc->getVerticesVector()[i].m_color = color;
    }
    m_debug_target_dy_dc->setUpdateOffset(0);
#endif
}   // changeDebugColor

// ----------------------------------------------------------------------------
/** Adds a flyable to be tested for crossing a cannon checkline.
 *  \param flyable The flyable to be tested.
 */
void CheckCannon::addFlyable(Flyable *flyable)
{
    m_all_flyables.push_back(flyable);
    m_flyable_previous_position.push_back(flyable->getXYZ());
}   // addFlyable

// ----------------------------------------------------------------------------
/** Removes a flyable from the tests if it crosses a checkline. Used when
 *  the flyable is removed (e.g. explodes).
 */
void CheckCannon::removeFlyable(Flyable *flyable)
{
    std::vector<Flyable*>::iterator i = std::find(m_all_flyables.begin(),
                                                  m_all_flyables.end(),
                                                  flyable);
    assert(i != m_all_flyables.end());
    size_t index = i - m_all_flyables.begin();   // get the index
    m_all_flyables.erase(i);
    m_flyable_previous_position.erase(m_flyable_previous_position.begin() + index);
}   // removeFlyable

// ----------------------------------------------------------------------------
/** Overriden to also check all flyables registered with the cannon.
 */
void CheckCannon::update(float dt)
{
    CheckLine::update(dt);
    for (unsigned int i = 0; i < m_all_flyables.size(); i++)
    {
        setIgnoreHeight(true);
        bool triggered = isTriggered(m_flyable_previous_position[i],
                                     m_all_flyables[i]->getXYZ(),
                                     /*kart index - ignore*/ -1     );
        setIgnoreHeight(false);
        m_flyable_previous_position[i] = m_all_flyables[i]->getXYZ();
        if(!triggered) continue;

        // Cross the checkline - add the cannon animation
        CannonAnimation *animation =
            new CannonAnimation(m_all_flyables[i], m_curve->clone(),
                                getLeftPoint(), getRightPoint(),
                                m_target_left, m_target_right);
        m_all_flyables[i]->setAnimation(animation);
    }   // for i in all flyables
}   // update
// ----------------------------------------------------------------------------
/** Called when the check line is triggered. This function  creates a cannon
 *  animation object and attaches it to the kart.
 *  \param kart_index The index of the kart that triggered the check line.
 */
void CheckCannon::trigger(unsigned int kart_index)
{
    AbstractKart *kart = World::getWorld()->getKart(kart_index);
    if (kart->getKartAnimation() || kart->isGhostKart())
    {
        return;
    }

    // The constructor AbstractKartAnimation resets the skidding to 0. So in
    // order to smooth rotate the kart, we need to keep the current visual
    // rotation and pass it to the CannonAnimation.
    float skid_rot = kart->getSkidding()->getVisualSkidRotation();
    new CannonAnimation(kart, m_curve->clone(), getLeftPoint(), getRightPoint(),
                        m_target_left, m_target_right, skid_rot);
}   // CheckCannon
