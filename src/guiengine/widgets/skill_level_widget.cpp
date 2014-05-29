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
#include "guiengine/widgets/skill_level_widget.hpp"
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

SkillLevelWidget::SkillLevelWidget(core::recti area, const int player_id,
                                   const int value, const stringw& label) : Widget(WTYPE_DIV)
{
    m_bar_value = value;
    m_player_id = player_id;

    setSize(area.UpperLeftCorner.X, area.UpperLeftCorner.Y,
            area.getWidth(), area.getHeight()               );

    // ---- Mass skill level widget
    m_bar = NULL;

    m_bar = new ProgressBarWidget(false);
    m_bar->setValue(value);

    m_bar->m_x = m_bar_x;
    m_bar->m_y = m_bar_y;
    m_bar->m_w = m_bar_w;
    m_bar->m_h = m_bar_h;
    m_bar->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_skill_bar", m_player_id);

    m_label = NULL;

    m_label = new LabelWidget(true, true);
    m_label->setText(label,false);

    m_label->m_x = m_label_x;
    m_label->m_y = m_label_y;
    m_label->m_w = m_label_w;
    m_label->m_h = m_label_h;
    m_label->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_skill_label", m_player_id);

    m_children.push_back(m_bar);
    m_children.push_back(m_label);
}   // KartStatsWidget

// -----------------------------------------------------------------------------

void SkillLevelWidget::add()
{
    m_bar->add();
    m_label->add();
}

// -----------------------------------------------------------------------------


void SkillLevelWidget::move(int x, int y, int w, int h)
{
    Widget::move(x,y,w,h);
    setSize(m_x, m_y, m_w, m_h);

    if (m_bar != NULL)
    {
        m_bar->move(m_bar_x,
                    m_bar_y,
                    m_bar_w,
                    m_bar_h );
        m_bar->setValue(m_bar_value);
    }
    if (m_label != NULL)
    {
        m_label->move(m_label_x,
                      m_label_y,
                      m_label_w,
                      m_label_h);
    }
}

// -------------------------------------------------------------------------

void SkillLevelWidget::setSize(const int x, const int y, const int w, const int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    // -- sizes
    m_bar_w = w/2;
    m_bar_h = GUIEngine::getFontHeight();
    m_label_w = w/2;
    m_label_h = GUIEngine::getFontHeight();

    // for shrinking effect
    if (h < 175)
    {
        const float factor = h / 175.0f;
        m_bar_h   = (int)(m_bar_h*factor);
        m_label_h = (int)(m_label_h*factor);
    }

    m_bar_x = x + w/2;
    m_bar_y = y;

    m_label_x = x;
    m_label_y = y;
}   // setSize

// -----------------------------------------------------------------------------

void SkillLevelWidget::setValue(int value)
{
    m_bar_value = value;
}

// -----------------------------------------------------------------------------

