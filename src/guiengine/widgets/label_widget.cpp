//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "guiengine/widgets/label_widget.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

// ----------------------------------------------------------------------------
/** Constructs the label widget. Parameter:
 *  \param title True if the special title font should be used.
 */
LabelWidget::LabelWidget(bool title) : Widget(WTYPE_LABEL)
{
    m_title_font = title;
    m_has_color  = false;
}   // LabelWidget

// ----------------------------------------------------------------------------
/** Adds the stk widget to the irrlicht widget set.
 */
void LabelWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    const bool word_wrap = m_properties[PROP_WORD_WRAP] == "true";
    const stringw& message = getText();
    
    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if      (m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if (m_properties[PROP_TEXT_ALIGN] == "right")  align = EGUIA_LOWERRIGHT;
    EGUI_ALIGNMENT valign = EGUIA_CENTER ; //TODO: make label v-align configurable through XML file?
    
    IGUIStaticText* irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size, false, word_wrap, m_parent, -1);
    m_element = irrwidget;
    irrwidget->setTextAlignment( align, valign );
    
    if (m_has_color)
    {
        irrwidget->setOverrideColor(m_color);
    }
    
    if (m_title_font)
    {
        irrwidget->setOverrideColor( video::SColor(255,255,255,255) );
        irrwidget->setOverrideFont( GUIEngine::getTitleFont() );
    }
    //irrwidget->setBackgroundColor( video::SColor(255,255,0,0) );
    //irrwidget->setDrawBackground(true);
    
    m_id = m_element->getID();
    //m_element->setTabOrder(id);
    m_element->setTabStop(false);
    m_element->setTabGroup(false);
}   // add

// ----------------------------------------------------------------------------
