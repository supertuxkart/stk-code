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
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
using namespace GUIEngine;
using namespace irr::video;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------
IconButtonWidget::IconButtonWidget(const bool tab_stop, const bool focusable)
{
    m_tab_stop = tab_stop;
    m_label = NULL;
    m_type = WTYPE_ICON_BUTTON;
    m_texture = NULL;
    m_focusable = focusable;
}
// -----------------------------------------------------------------------------
void IconButtonWidget::add()
{
    // ---- Icon
    m_texture = GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" +m_properties[PROP_ICON]).c_str());
    assert(m_texture != NULL);
    m_texture_w = m_texture->getSize().Width;
    m_texture_h = m_texture->getSize().Height;

    // irrlicht widgets don't support scaling while keeping aspect ratio
    // so, happily, let's implement it ourselves
    const int x_gap = (int)((float)w - (float)m_texture_w * (float)h / m_texture_h);
    
    rect<s32> widget_size = rect<s32>(x + x_gap/2, y, x + w - x_gap/2, y + h);
    //std::cout << "Creating a IGUIButton " << widget_size.UpperLeftCorner.X << ", " << widget_size.UpperLeftCorner.Y <<
    //" : " << widget_size.getWidth() << "x" << widget_size.getHeight() << std::endl;
    
    IGUIButton* btn = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, (m_tab_stop ? getNewID() : getNewNoFocusID()), L"");

    btn->setTabStop(m_tab_stop);
    m_element = btn;
    id = m_element->getID();
    
    // ---- label if any
    stringw& message = m_text;
    if (message.size() > 0)
    {
        widget_size = rect<s32>(x, y + h, x + w, y + h*2);

        m_label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size, false, false /* word wrap */, m_parent);
        m_label->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);
        m_label->setTabStop(false);
    }
    
    // ---- IDs
    id = m_element->getID();
    if (m_tab_stop) m_element->setTabOrder(id);
    m_element->setTabGroup(false);
}
// -----------------------------------------------------------------------------
/** \precondition At the moment, the new texture must have the same aspct ratio as the previous one since the object will not
  *               be modified to fit a different aspect ratio
  */
void IconButtonWidget::setImage(const char* path_to_texture)
{
    m_properties[PROP_ICON] = path_to_texture;
    m_texture = GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" + m_properties[PROP_ICON]).c_str());

    if (m_texture == NULL)
    {
        // texture not found, try with absolute path
        m_texture = GUIEngine::getDriver()->getTexture(m_properties[PROP_ICON].c_str());
    }
    assert(m_texture != NULL);

    m_texture_w = m_texture->getSize().Width;
    m_texture_h = m_texture->getSize().Height;
}
// -----------------------------------------------------------------------------
/** \precondition At the moment, the new texture must have the same aspct ratio as the previous one since the object will not
 *                be modified to fit a different aspect ratio
 */
void IconButtonWidget::setImage(ITexture* texture)
{
    m_texture = texture;
    assert(m_texture != NULL);
    
    m_texture_w = m_texture->getSize().Width;
    m_texture_h = m_texture->getSize().Height;
}
// -----------------------------------------------------------------------------
void IconButtonWidget::setLabel(stringw new_label)
{
    // FIXME: does not update m_text. Is this a behaviour we want?
    if (m_label == NULL) return;
    
    m_label->setText( new_label.c_str() );
}
