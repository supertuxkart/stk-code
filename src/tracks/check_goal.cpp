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

#include "tracks/check_goal.hpp"

#include "config/user_config.hpp"
#include "io/xml_node.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "modes/soccer_world.hpp"

#include <stdio.h>

/** Constructor for a check goal line.
 *  \param node XML node containing the parameters for this goal line.
 *  \param index Index of this check structure in the check manager.
 */
CheckGoal::CheckGoal(const XMLNode &node,  unsigned int index)
           : CheckStructure(node, index)
{
    // Determine the team for this goal
    m_first_goal = false;
    node.get("first_goal", &m_first_goal);

    node.get("p1", &m_p1);
    node.get("p2", &m_p3);

    m_line.setLine( core::vector2df(m_p1.getX(), m_p1.getZ()),
                    core::vector2df(m_p3.getX(), m_p3.getZ()) );
    m_p2 = (m_p1 + m_p3) / 2;
}   // CheckGoal

// ----------------------------------------------------------------------------
/**
 * Checks the soccer balls to see if they crossed the line and trigger the goal accordingly.
 */
void CheckGoal::update(float dt)
{
    SoccerWorld* world = dynamic_cast<SoccerWorld*>(World::getWorld());

    if (world)
    {
        if (isTriggered(m_previous_ball_position, world->getBallPosition(), 
                        /*kart index - ignore*/-1)                         )
        {
            if (UserConfigParams::m_check_debug)
            {
                Log::info("CheckGoal", "Goal check structure"
                          "%d triggered for ball.", m_index);
            }
            trigger(0);
        }
        m_previous_ball_position = world->getBallPosition();
    }
}   // update

// ----------------------------------------------------------------------------
/** Called when the goal line is triggered. Input any integer for i to use
 */
void CheckGoal::trigger(unsigned int i)
{
    SoccerWorld* world = dynamic_cast<SoccerWorld*>(World::getWorld());
    if(!world)
    {
        Log::warn("CheckGoal",
                  "No soccer world found, cannot count the points.");
        return;
    }

    world->onCheckGoalTriggered(m_first_goal);
}   // trigger

// ----------------------------------------------------------------------------
bool CheckGoal::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                            int kart_index)
{
    core::vector2df cross_point;

    // Check if the finite line was actually crossed:
    return m_line.intersectWith(core::line2df(old_pos.toIrrVector2d(),
                                              new_pos.toIrrVector2d()),
                                cross_point);

}   // isTriggered

// ----------------------------------------------------------------------------
void CheckGoal::reset(const Track &track)
{
    CheckStructure::reset(track);
    m_previous_ball_position = Vec3(0, 0, 0);

    SoccerWorld* world = dynamic_cast<SoccerWorld*>(World::getWorld());

    if (world)
    {
        m_previous_ball_position = world->getBallPosition();
    }

}   // reset
