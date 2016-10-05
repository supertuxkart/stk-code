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
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "physics/physics.hpp"
#include "utils/random_generator.hpp"

SpareTireAI::SpareTireAI(AbstractKart *kart)
           : BattleAI(kart)
{
    reset();
    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kart.
    Controller::setControllerName("SpareTireAI");

}   // SpareTireAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void SpareTireAI::reset()
{
    BattleAI::reset();
    m_fixed_target_nodes.clear();
    m_idx = 0;
    m_timer = 0.0f;
}   // reset

//-----------------------------------------------------------------------------
void SpareTireAI::update(float dt)
{
    assert(!m_fixed_target_nodes.empty());

    m_kart->setSlowdown(MaxSpeed::MS_DECREASE_AI, 0.5f, /*fade_in_time*/0.0f);

    BattleAI::update(dt);
    m_timer -= dt;
    if (m_timer < 0.0f)
        unspawn();
}   // update

//-----------------------------------------------------------------------------
void SpareTireAI::findDefaultPath()
{
    // Randomly find 3 nodes for spare tire kart to move
    assert(m_fixed_target_nodes.empty());
    const int nodes = m_graph->getNumNodes();
    const float min_dist = sqrtf(nodes);
    RandomGenerator random;
    while (m_fixed_target_nodes.size() < 3)
    {
        int node = random.get(nodes);
        if (m_fixed_target_nodes.empty())
        {
            m_fixed_target_nodes.push_back(node);
            continue;
        }
        bool succeed = true;
        for (const int& all_node : m_fixed_target_nodes)
        {
            float dist = m_graph->getDistance(all_node, node);
            if (dist < min_dist)
                succeed = false;
        }
        if (succeed)
            m_fixed_target_nodes.push_back(node);
    }
    m_idx = 0;
    m_target_node = m_fixed_target_nodes[m_idx];

}   // findDefaultPath

//-----------------------------------------------------------------------------
void SpareTireAI::findTarget()
{
    if (getCurrentNode() == m_fixed_target_nodes[m_idx])
        m_idx = m_idx == 2 ? 0 : m_idx + 1;

    const int chosen_node = m_fixed_target_nodes[m_idx];
    m_target_node = chosen_node;
    m_target_point = m_graph->getNode(chosen_node)->getCenter();
}   // findTarget

//-----------------------------------------------------------------------------
void SpareTireAI::spawn(float time_to_last)
{
    findDefaultPath();
    m_timer = time_to_last;

    World::getWorld()->getPhysics()->addKart(m_kart);
    m_kart->getKartGFX()->reset();
    m_kart->getNode()->setVisible(true);

}   // spawn

//-----------------------------------------------------------------------------
void SpareTireAI::unspawn()
{
    reset();
    m_kart->eliminate();
}   // unspawn

//-----------------------------------------------------------------------------
void SpareTireAI::crashed(const AbstractKart *k)
{
    // Nothing happen when two spare tire karts crash each other
    if (dynamic_cast<const SpareTireAI*>(k->getController()) != NULL) return;

    // Max 3 lives only
    if (m_world->getKartLife(k->getWorldKartId()) == 3) return;

    // Otherwise increase one life for that kart and unspawn
    m_world->addKartLife(k->getWorldKartId());
    unspawn();

}   // crashed
