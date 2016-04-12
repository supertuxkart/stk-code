//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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
                                   bool multiplayer, bool display_text,
                                   const int value, const stringw& label)
                                  : Widget(WTYPE_DIV)
{
    m_player_id = player_id;
    m_display_text = display_text;

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

    m_label = new LabelWidget(!multiplayer, true);
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
    m_label->setVisible(m_display_text);
}

// -----------------------------------------------------------------------------


void SkillLevelWidget::move(const int x, const int y, const int w, const int h)
{
    Widget::move(x,y,w,h);
    setSize(m_x, m_y, m_w, m_h);

    if (m_bar != NULL)
    {
        m_bar->move(m_bar_x,
                    m_bar_y,
                    m_bar_w,
                    m_bar_h );
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
    if (m_display_text)
        m_bar_w = (w / 2) * 3 / 4;
    else
        m_bar_w = w * 2 / 3;
    m_bar_h = h;
    m_label_w = w/2;
    m_label_h = h;

    // for shrinking effect
    if (h < 175)
    {
        const float factor = h / 175.0f;
        m_bar_h   = (int)(m_bar_h*factor);
        m_label_h = (int)(m_label_h*factor);
    }

    if (m_display_text)
        m_bar_x = x + w / 2;
    else
        m_bar_x = x + w / 6;
    m_bar_y = y + m_h/2 - m_bar_h/2;

    m_label_x = x;
    m_label_y = y + m_h/2 - m_label_h/2;
}   // setSize

// -----------------------------------------------------------------------------

void SkillLevelWidget::setValue(const int value)
{
    m_bar->moveValue(value);
}

// -----------------------------------------------------------------------------

void SkillLevelWidget::setLabel(const irr::core::stringw& label)
{
    m_label->setText(label, false);
}

void SkillLevelWidget::setDisplayText(bool display_text)
{
    if(m_display_text != display_text)
    {
        m_display_text = display_text;
        m_label->setVisible(display_text);
        setSize(m_x, m_y, m_w, m_h);
    }
}

