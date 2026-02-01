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
#include "icon_button_widget.hpp"

#include <IGUIEnvironment.h>
#include <IGUIElement.h>
#include <IGUIButton.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------

SkillLevelWidget::SkillLevelWidget(core::recti area, const int player_id,
                                   bool multiplayer, bool display_icon,
                                   const float value)
                                  : Widget(WTYPE_DIV)
{
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

    m_iconbutton = NULL;

    m_iconbutton = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_TEXTURE_ASPECT_RATIO,
                                        false, false, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);

    //m_iconbutton_* properties are calculated in setSize method
    m_iconbutton->m_x = m_iconbutton_x;
    m_iconbutton->m_y = m_iconbutton_y;
    m_iconbutton->m_w = m_iconbutton_w;
    m_iconbutton->m_h = m_iconbutton_h;
    m_iconbutton->m_properties[PROP_ID] = StringUtils::insertValues("@p%i_skill_label", m_player_id);

    m_children.push_back(m_bar);
    m_children.push_back(m_iconbutton);
    
    m_display_icon = display_icon;
}   // KartStatsWidget

// -----------------------------------------------------------------------------

void SkillLevelWidget::add()
{
    m_bar->add();
    m_iconbutton->add();
    m_iconbutton->setVisible(m_display_icon);
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
    if (m_iconbutton != NULL)
    {
        m_iconbutton->move( m_iconbutton_x,
                            m_iconbutton_y,
                            m_iconbutton_w,
                            m_iconbutton_h);
    }
}

// -------------------------------------------------------------------------

void SkillLevelWidget::setSize(const int x, const int y, const int w, const int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;
    
    int iconbox_h = h * 5 / 3; 
    int iconbox_w = h * 5 / 3; //assuming square icon
    
    m_iconbutton_h = iconbox_h; 
    m_iconbutton_w = iconbox_w; 

    // -- sizes
    m_bar_w = m_w - iconbox_w - 25;  //leaving just enough space for icon + its margin  
    m_bar_h = h;
    
    m_bar_x = x + iconbox_w + m_w / 32;    
    m_bar_y = y + h/2 - m_bar_h/2; //align to midpoint in y direction

    m_iconbutton_x = x; //make sure icon has enough space on the right
    m_iconbutton_y = y + h/2 - m_iconbutton_h/2; //align to midpoint in y direction
}   // setSize

// -----------------------------------------------------------------------------

void SkillLevelWidget::setValue(const float value)
{
    m_bar->moveValue(value);
}

// -----------------------------------------------------------------------------

void SkillLevelWidget::setIcon(const irr::core::stringc& filepath)
{
    m_iconbutton->setImage(filepath.c_str());
}

// -----------------------------------------------------------------------------

void SkillLevelWidget::setDisplayIcon(bool display_icon)
{
    if(m_display_icon != display_icon)
    {
        m_display_icon = display_icon;
        m_iconbutton->setVisible(display_icon);
        setSize(m_x, m_y, m_w, m_h);
    }
}
