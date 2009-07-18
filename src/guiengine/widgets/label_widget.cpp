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

#include "guiengine/engine.hpp"
#include "guiengine/widgets/label_widget.hpp"
using namespace GUIEngine;

LabelWidget::LabelWidget()
{
    m_type = WTYPE_LABEL;
}

void LabelWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    const bool word_wrap = m_properties[PROP_WORD_WRAP] == "true";
    stringw  message = m_properties[PROP_TEXT].c_str();
    
    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if(m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if(m_properties[PROP_TEXT_ALIGN] == "right") align = EGUIA_LOWERRIGHT;
    EGUI_ALIGNMENT valign = EGUIA_CENTER ; // TODO - make confiurable through XML file?
    
    IGUIStaticText* irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size, false, word_wrap, m_parent, -1);
    m_element = irrwidget;
    irrwidget->setTextAlignment( align, valign );
    
    id = m_element->getID();
    //m_element->setTabOrder(id);
    m_element->setTabStop(false);
    m_element->setTabGroup(false);
}

void LabelWidget::setText(stringw newText)
{
    IGUIStaticText* irrwidget = Widget::getIrrlichtElement<IGUIStaticText>();   
    irrwidget->setText(newText.c_str());
}
