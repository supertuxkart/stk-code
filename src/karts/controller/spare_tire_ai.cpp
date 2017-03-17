//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "karts/controller/spare_tire_ai.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/kart_gfx.hpp"
#include "karts/max_speed.hpp"
#include "modes/three_strikes_battle.hpp"
#include "states_screens/race_gui.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "physics/physics.hpp"
#include "utils/random_generator.hpp"

#include <algorithm>

SpareTireAI::SpareTireAI(AbstractKart *kart)
           : BattleAI(kart)
{
    reset();
    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kart.
    Controller::setControllerName("SpareTireAI");

    // Pre-load the 4 nodes of bounding box defined by battle world
    memcpy(m_fixed_target_nodes, m_graph->getBBNodes(), 4 * sizeof(int));

    // Reverse the order depends on world ID, so not all spare tire karts go
    // the same way
    if (m_kart->getWorldKartId() % 2 != 0)
    {
        std::reverse(std::begin(m_fixed_target_nodes),
            std::end(m_fixed_target_nodes));
    }
}   // SpareTireAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void SpareTireAI::reset()
{
    BattleAI::reset();
    m_idx = 0;
    m_timer = 0.0f;
}   // reset

//-----------------------------------------------------------------------------
/** Besides calling update from parent class, it will auto \ref unspawn if
 *  \ref m_timer reaches zero which it will be decreased here.
 *  \param dt Time step size.
 */
void SpareTireAI::update(float dt)
{
    BattleAI::update(dt);
    m_kart->setSlowdown(MaxSpeed::MS_DECREASE_AI, 0.5f, /*fade_in_time*/0.0f);
    m_timer -= dt;
    if (m_timer < 0.0f)
        unspawn();
}   // update

//-----------------------------------------------------------------------------
/** Randomly find a start node for spare tire kart to move, called after \ref
 *  spawn.
 */
void SpareTireAI::findDefaultPath()
{
    assert(m_idx == -1);

    RandomGenerator random;
    m_idx = random.get(4);
    m_target_node = m_fixed_target_nodes[m_idx];

}   // findDefaultPath

//-----------------------------------------------------------------------------
/** For SpareTireAI, it will pick next node in \ref m_fixed_target_nodes after
 *  reach the one in \ref m_idx, or the first one if it's the last.
 */
void SpareTireAI::findTarget()
{
    assert(m_idx != -1 && m_idx < 4);
    if (getCurrentNode() == m_fixed_target_nodes[m_idx])
        m_idx = m_idx == 3 ? 0 : m_idx + 1;

    const int chosen_node = m_fixed_target_nodes[m_idx];
    m_target_node = chosen_node;
    m_target_point = m_graph->getNode(chosen_node)->getCenter();
}   // findTarget

//-----------------------------------------------------------------------------
/** Spawn the SpareTireAI, it will start appearing in the battle mode and
 *  moving around.
 *  \param time_to_last Time before calling \ref unspawn.
 */
void SpareTireAI::spawn(float time_to_last)
{
    findDefaultPath();
    m_timer = time_to_last;

    Physics::getInstance()->addKart(m_kart);
    m_kart->startEngineSFX();
    m_kart->getKartGFX()->reset();
    m_kart->getNode()->setVisible(true);

}   // spawn

//-----------------------------------------------------------------------------
/** Unspawn the SpareTireAI, it will be hidden in the battle mode but not
 *  deleted, so it can be called with \ref spawn again later.
 */
void SpareTireAI::unspawn()
{
    m_idx = -1;
    m_kart->eliminate();
}   // unspawn

//-----------------------------------------------------------------------------
/** Callback function when a kart crashes into the SpareTireAI, the kart will
 *  increase one life if its life is not equal 3. A message will be shown too.
 *  \param k \ref AbstractKart this SpareTireAI crashed.
 */
void SpareTireAI::crashed(const AbstractKart *k)
{
    // Nothing happen when two spare tire karts crash each other
    if (dynamic_cast<const SpareTireAI*>(k->getController()) != NULL) return;

    // Tell player that they have max 3 lives only
    if (m_world->getKartLife(k->getWorldKartId()) == 3)
    {
        World::getWorld()->getRaceGUI()->addMessage
            (_("You can have at most 3 lives!"), k, 2.0f);
        return;
    }

    // Otherwise increase one life for that kart and unspawn
    m_world->addKartLife(k->getWorldKartId());
    World::getWorld()->getRaceGUI()->addMessage(_("+1 life."), k, 2.0f);
    unspawn();

}   // crashed
