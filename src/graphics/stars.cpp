//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2012-2015  SuperTuxKart-Team
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

#include "graphics/stars.hpp"

#include "graphics/irr_driver.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "utils/constants.hpp"

#include <ISceneNode.h>
#include <IBillboardSceneNode.h>

#include <cmath>

const int FREQUENCY = 2;
const int STAR_AMOUNT = 7;
const float RADIUS = 0.7f;
const float STAR_SIZE = 0.4f;

Stars::Stars(AbstractKart *kart)
{
    m_parent_kart_node = kart->getNode();
    m_enabled = false;
    m_center = core::vector3df(0.0f,
                               kart->getKartModel()->getModel()
                                   ->getBoundingBox().MaxEdge.Y,
                               0.0f                             );

    for (int n=0; n<STAR_AMOUNT; n++)
    {
        scene::ISceneNode* billboard =
            irr_driver->addBillboard(core::dimension2df(STAR_SIZE, STAR_SIZE),
                                     "starparticle.png", kart->getNode());
#ifdef DEBUG
        billboard->setName("star");
#endif

        billboard->setVisible(false);

        m_nodes.push_back(billboard);
    }
}   // Stars

// ----------------------------------------------------------------------------

Stars::~Stars()
{
    const int nodeAmount = (int) m_nodes.size();
    for (int n=0; n<nodeAmount; n++)
    {
        m_parent_kart_node->removeChild(m_nodes[n]);
    }
}   // ~Stars

// ----------------------------------------------------------------------------

void Stars::showFor(float time)
{
    m_enabled        = true;
    m_remaining_time = time;
    m_period         = time;

    const int node_amount = (int)m_nodes.size();
    for (int n=0; n<node_amount; n++)
    {
        m_nodes[n]->setVisible(true);
        ((scene::IBillboardSceneNode*)m_nodes[n])
               ->setSize( core::dimension2df(0.01f, 0.01f) );
    }

    // set stars initial position
    update(0);
}   // showFor

// ----------------------------------------------------------------------------
/** Resets the stars, esp. disabling them at a restart.
 */
void Stars::reset()
{
    // This will make the stars invisible and disable them
    m_remaining_time = -1;
}   // reset

// ----------------------------------------------------------------------------

void Stars::update(float delta_t)
{
    if (!m_enabled) return;

    m_remaining_time -= delta_t;
    if (m_remaining_time < 0)
    {
        m_enabled = false;

        const int node_amount = (int)m_nodes.size();
        for (int n=0; n<node_amount; n++)
        {
            m_nodes[n]->setVisible(false);
        }
        return;
    }

    // This is a basic Fourier series for a square wave, with edits to allow
    // adjusting the period and controlling the frequency based on the
    // period.
    float factor = 0.0f;
    for (int n = 1; n <= FREQUENCY * static_cast<int>(m_period); n++)
    {
        factor += 4.0f / M_PI * (sin(M_PI / m_period * (2.0f * n - 1.0f) *
                    (m_period - m_remaining_time)) / (2.0f * n - 1.0f));
    }
    factor = pow(factor, 2.f);

    // When the factor is very small, the stars are rendered incorrectly
    if (factor <= 0.001f)
        return;

    float radius = RADIUS * factor;

    const int node_amount = (int)m_nodes.size();
    for (int n=0; n<node_amount; n++)
    {
        // do one full rotation every 4 seconds (this "ranges" ranges
        // from 0 to 1)
        float angle = (m_remaining_time / 4.0f) - (int)(m_remaining_time / 4);

        // each star must be at a different angle
        angle += n * (1.0f / STAR_AMOUNT);

        // keep angle in range [0, 1[
        angle -= (int)angle;

        ((scene::IBillboardSceneNode*)m_nodes[n])
                ->setSize( core::dimension2df(factor*STAR_SIZE,
                                              factor*STAR_SIZE) );

        // Set position: X and Z are the position in the cirlce,
        // the Y components shakes the stars up and down like falling coin
        core::vector3df offset(std::cos(angle*M_PI*2.0f)*radius,
                               std::cos(angle*M_PI*2.0f+m_remaining_time*4.0f)
                               *radius*0.25f,
                               std::sin(angle*M_PI*2.0f)*radius              );
        m_nodes[n]->setPosition(m_center + offset);
    } // end for
}   // update

