//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2012-2013  SuperTuxKart-Team
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

#include <cmath>

#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "utils/constants.hpp"

#include <ISceneNode.h>
#include <IBillboardSceneNode.h>

const int STAR_AMOUNT = 7;
const float RADIUS = 0.7f;
const float STAR_SIZE = 0.4f;

Stars::Stars(scene::ISceneNode* parentKart, core::vector3df center)
{
    m_parent_kart_node = parentKart;
    m_enabled = false;

    video::ITexture* texture = irr_driver->getTexture("starparticle.png");
    Material* star_material =
        material_manager->getMaterial("starparticle.png");

    m_center = center;

    for (int n=0; n<STAR_AMOUNT; n++)
    {
        scene::ISceneNode* billboard =
            irr_driver->addBillboard(core::dimension2df(STAR_SIZE, STAR_SIZE),
                                     texture, parentKart);
#ifdef DEBUG
        billboard->setName("star");
#endif
        star_material->setMaterialProperties(&(billboard->getMaterial(0)), NULL);
        billboard->setMaterialTexture(0, star_material->getTexture());

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
    m_fade_in_time   = 1.0f;

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

        float radius = RADIUS;


        // manage "fade-in"
        if (m_fade_in_time > 0.0f)
        {
            float fade = (1.0f - m_fade_in_time);

            ((scene::IBillboardSceneNode*)m_nodes[n])->setSize(
                    core::dimension2d< f32 >(fade*STAR_SIZE, fade*STAR_SIZE) );

            radius *= fade;
        }
        // manage "fade-out"
        else if (m_remaining_time < 1.0f)
        {
            radius *= m_remaining_time;

            ((scene::IBillboardSceneNode*)m_nodes[n])
                ->setSize( core::dimension2df(m_remaining_time*STAR_SIZE,
                                              m_remaining_time*STAR_SIZE) );
        }

        // Set position: X and Z are the position in the cirlce,
        // the Y components shakes the stars up and down like falling coin
        core::vector3df offset(std::cos(angle*M_PI*2.0f)*radius,
                               std::cos(angle*M_PI*2.0f+m_remaining_time*4.0f)
                               *radius*0.25f,
                               std::sin(angle*M_PI*2.0f)*radius              );
        m_nodes[n]->setPosition(m_center + offset);
    } // end for

    if (m_fade_in_time > 0.0f) m_fade_in_time -= delta_t;
}   // update

