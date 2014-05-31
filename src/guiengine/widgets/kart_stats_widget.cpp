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
    m_player_id = player_id;

    setSize(area.UpperLeftCorner.X, area.UpperLeftCorner.Y,
            area.getWidth(), area.getHeight()               );

    const std::string default_kart = UserConfigParams::m_default_kart;
    const KartProperties* props =
        kart_properties_manager->getKart(default_kart);
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



    // ---- Mass skill level widget
    irr::core::recti massArea(m_skill_bar_x, m_skill_bar_y,
                              m_skill_bar_x + m_skill_bar_w,
                              m_skill_bar_y + m_skill_bar_h);
    SkillLevelWidget* skill_bar = NULL;

    skill_bar = new SkillLevelWidget(massArea, m_player_id,
                                      (int) props->getMass()/10, "Weight");
    skill_bar->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_mass", m_player_id);

    m_skills.push_back(skill_bar);
    m_children.push_back(skill_bar);

    // ---- Speed skill level widget
    irr::core::recti speedArea(m_skill_bar_x, m_skill_bar_y - m_skill_bar_h - 10,
                               m_skill_bar_x + m_skill_bar_w,
                               m_skill_bar_y + 10);

    skill_bar = NULL;

    skill_bar = new SkillLevelWidget(speedArea, m_player_id,
                                       (int) props->getMaxSpeed()/10, "Speed");
    skill_bar->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_speed", m_player_id);

    m_skills.push_back(skill_bar);
    m_children.push_back(skill_bar);

    // ---- Acceleration skill level widget
    irr::core::recti accelArea(m_skill_bar_x, m_skill_bar_y + m_skill_bar_h + 10,
                               m_skill_bar_x + m_skill_bar_w,
                               m_skill_bar_y + 2*m_skill_bar_y + 10);

    skill_bar = NULL;

    skill_bar = new SkillLevelWidget(accelArea, m_player_id,
                                       (int) props->getTrackConnectionAccel()/10, "Accel");
    skill_bar->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_accel", m_player_id);

    m_skills.push_back(skill_bar);
    m_children.push_back(skill_bar);

}   // KartStatsWidget

// -----------------------------------------------------------------------------

void KartStatsWidget::add()
{
    for (int i = 0; i < SKILL_COUNT; ++i) {
        m_skills[i]->add();
    }
}

void KartStatsWidget::move(int x, int y, int w, int h)
{
    Widget::move(x,y,w,h);
    setSize(m_x, m_y, m_w, m_h);
    int offset = (m_h - (SKILL_COUNT*m_skill_bar_h)) / 2;
    for (int i = 0; i < SKILL_COUNT; ++i)
    {
        m_skills[i]->move(m_skill_bar_x,
                          m_y + offset + m_skill_bar_h*i,
                          m_skill_bar_w,
                          m_skill_bar_h);
    }
} //move

// -----------------------------------------------------------------------------

// ---- set value for given type
void KartStatsWidget::setValue(Stats type, int value)
{
    m_skills[type]->setValue(value);
} //setValue

// -----------------------------------------------------------------------------

// ---- get value for given type
int KartStatsWidget::getValue(Stats type)
{
    return m_skills[type]->getValue();
}   // getVAlue

// ---- set size for widgets inside KartStatsWidget
void KartStatsWidget::setSize(const int x, const int y, const int w, const int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    // -- sizes
    m_skill_bar_w = w;
    m_skill_bar_h = 100;

    // for shrinking effect
    if (h < 175)
    {
        const float factor = h / 175.0f;
        m_skill_bar_h   = (int)(m_skill_bar_h*factor);
    }

    m_skill_bar_x = x;
    m_skill_bar_y = y + h/2 - m_skill_bar_h/2;
}   // setSize

// -----------------------------------------------------------------------------
