//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "guiengine/engine.hpp"
#include "guiengine/widgets/kart_stats_widget.hpp"
#include "utils/string_utils.hpp"
#include <string.h>
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"

#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include "config/user_config.hpp"

#include <IGUIEnvironment.h>
#include <IGUIElement.h>
#include <IGUIButton.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------

KartStatsWidget::KartStatsWidget(core::recti area, const int player_id,
                                 std::string kart_group) : Widget(WTYPE_DIV)
{

    x_speed = 1.0f;
    y_speed = 1.0f;
    w_speed = 1.0f;
    h_speed = 1.0f;

    m_player_id = player_id;

    setSize(area.UpperLeftCorner.X, area.UpperLeftCorner.Y,
            area.getWidth(), area.getHeight()               );
    target_x = m_x;
    target_y = m_y;
    target_w = m_w;
    target_h = m_h;

    // ---- Mass skill level widget
    m_mass_bar = NULL;

    m_mass_bar = new ProgressBarWidget(false);
    m_mass_bar->m_x = m_mass_bar_x;
    m_mass_bar->m_y = m_mass_bar_y;
    m_mass_bar->m_w = m_mass_bar_w;
    m_mass_bar->m_h = m_mass_bar_h;
    m_mass_bar->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_mass", m_player_id);

    const std::string default_kart = UserConfigParams::m_default_kart;
    const KartProperties* props =
        kart_properties_manager->getKart(default_kart);
    m_mass_bar->setValue((int)props->getMass()/10);
    Log::verbose("Value", StringUtils::toString(m_mass_bar->getValue()).c_str());
    m_children.push_back(m_mass_bar);
    if(!props)
    {
        // If the default kart can't be found (e.g. previously a addon
        // kart was used, but the addon package was removed), use the
        // first kart as a default. This way we don't have to hardcode
        // any kart names.
        int id = kart_properties_manager->getKartByGroup(kart_group, 0);
        if (id == -1)
        {
            props = kart_properties_manager->getKartById(0);
        }
        else
        {
            props = kart_properties_manager->getKartById(id);
        }

        if(!props)
        {
            fprintf(stderr,
                    "[KartSelectionScreen] WARNING: Can't find default "
                    "kart '%s' nor any other kart.\n",
                    default_kart.c_str());
            exit(-1);
        }
    }
}   // KartStatsWidget

// -----------------------------------------------------------------------------

void KartStatsWidget::add()
{
    Log::verbose("Widget", "ADD");
    //TODO add others, and add them properly
    m_mass_bar->add();
}

// -----------------------------------------------------------------------------

/** Updates the animation (moving/shrinking/etc.) */
void KartStatsWidget::onUpdate(float delta)
{
    m_mass_bar->setValue(m_mass_value);
    if (target_x == m_x && target_y == m_y &&
            target_w == m_w && target_h == m_h) return;

    int move_step = (int)(delta*1000.0f);

    // move x towards target
    if (m_x < target_x)
    {
        m_x += (int)(move_step*x_speed);
        // don't move to the other side of the target
        if (m_x > target_x) m_x = target_x;
    }
    else if (m_x > target_x)
    {
        m_x -= (int)(move_step*x_speed);
        // don't move to the other side of the target
        if (m_x < target_x) m_x = target_x;
    }

    // move y towards target
    if (m_y < target_y)
    {
        m_y += (int)(move_step*y_speed);
        // don't move to the other side of the target
        if (m_y > target_y) m_y = target_y;
    }
    else if (m_y > target_y)
    {
        m_y -= (int)(move_step*y_speed);
        // don't move to the other side of the target
        if (m_y < target_y) m_y = target_y;
    }

    // move w towards target
    if (m_w < target_w)
    {
        m_w += (int)(move_step*w_speed);
        // don't move to the other side of the target
        if (m_w > target_w) m_w = target_w;
    }
    else if (m_w > target_w)
    {
        m_w -= (int)(move_step*w_speed);
        // don't move to the other side of the target
        if (m_w < target_w) m_w = target_w;
    }
    // move h towards target
    if (m_h < target_h)
    {
        m_h += (int)(move_step*h_speed);
        // don't move to the other side of the target
        if (m_h > target_h) m_h = target_h;
    }
    else if (m_h > target_h)
    {
        m_h -= (int)(move_step*h_speed);
        // don't move to the other side of the target
        if (m_h < target_h) m_h = target_h;
    }

    setSize(m_x, m_y, m_w, m_h);

    if (m_mass_bar != NULL)
    {
        m_mass_bar->move(m_mass_bar_x,
                         m_mass_bar_y,
                         m_mass_bar_w,
                         m_mass_bar_h );
    }
}   // onUpdate

// -------------------------------------------------------------------------

void KartStatsWidget::setSize(const int x, const int y, const int w, const int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    // -- sizes
    m_mass_bar_w = 2*w/3;
    m_mass_bar_h = 100;

    // for shrinking effect
    if (h < 175)
    {
        const float factor = h / 175.0f;
        m_mass_bar_h   = (int)(m_mass_bar_h*factor);
    }

    m_mass_bar_x = x + w/2 - m_mass_bar_w;
    m_mass_bar_y = y + h/2 - m_mass_bar_h;
}   // setSize

// -----------------------------------------------------------------------------

void KartStatsWidget::setMass(int value)
{
    m_mass_value = value;
}

// -----------------------------------------------------------------------------

void KartStatsWidget::setAcceleration(int value)
{
    m_accel_value = value;
}

// -----------------------------------------------------------------------------

void KartStatsWidget::setSpeed(int value)
{
    m_speed_value = value;
}
// -----------------------------------------------------------------------------

