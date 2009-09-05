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
#include "guiengine/my_button.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
using namespace GUIEngine;

// -----------------------------------------------------------------------------
IconButtonWidget::IconButtonWidget(const bool clickable)
{
    IconButtonWidget::clickable = clickable;
    label = NULL;
    m_type = WTYPE_ICON_BUTTON;
}
// -----------------------------------------------------------------------------
void IconButtonWidget::add()
{
    // ---- Icon
    ITexture* texture = GUIEngine::getDriver()->getTexture((file_manager->getDataDir() + "/" +m_properties[PROP_ICON]).c_str());
    assert(texture != NULL);
    const int texture_w = texture->getSize().Width, texture_h = texture->getSize().Height;

    // irrlicht widgets don't support scaling while keeping aspect ratio
    // so, happily, let's implement it ourselves
    const int x_gap = (int)((float)w - (float)texture_w * (float)h / texture_h);
    
    rect<s32> widget_size;
    if (clickable)
    {
        widget_size = rect<s32>(x + x_gap/2, y, x + w - x_gap/2, y + h);

        MyGUIButton* btn = new MyGUIButton(GUIEngine::getGUIEnv(), m_parent,  getNewID(), widget_size, true);
        //IGUIButton* btn = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, getNewID(), L"");
        btn->setUseAlphaChannel(true);
        btn->setImage(texture);
        btn->setDrawBorder(false);
        btn->setTabStop(true);
        m_element = btn;
    }
    else
    {
        widget_size = rect<s32>(x + x_gap/2, y, x + w - x_gap/2, y + h);
        
        IGUIImage* btn = GUIEngine::getGUIEnv()->addImage(widget_size, m_parent, getNewNoFocusID());
        m_element = btn;
        btn->setUseAlphaChannel(true);
        btn->setImage(texture);
        btn->setTabStop(false);
        btn->setScaleImage(true);
    }
    
    // ---- label if any
    stringw& message = m_text;
    if (message.size() > 0)
    {
        widget_size = rect<s32>(x, y + h, x + w, y + h*2);

        label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size, false, false /* word wrap */, m_parent);
        label->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);
        label->setTabStop(false);
    }
    
    // ---- IDs
    id = m_element->getID();
    if(clickable) m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    
    /*
     IGUISpriteBank* sprite_bank = GUIEngine::getGUIEnv()->getSkin()->getSpriteBank();
     // GUIEngine::getDriver()->makeColorKeyTexture(GUIEngine::getDriver()->getTexture("irrlichtlogo2.png"), position2di(0,0));
     sprite_bank->addTexture( GUIEngine::getDriver()->getTexture("irrlichtlogo2.png") );
     
     SGUISprite sprite;
     sprite.frameTime = 3000;
     SGUISpriteFrame frame;
     core::array<core::rect<s32> >& rectangles = sprite_bank->getPositions();
     rectangles.push_back(rect<s32>(0,0,128,128));
     frame.rectNumber = rectangles.size()-1;
     frame.textureNumber = sprite_bank->getTextureCount() - 1;
     sprite.Frames.push_back(frame);
     sprite_bank->getSprites().push_back(sprite);
     
     button->setSpriteBank(sprite_bank);
     button->setSprite(EGBS_BUTTON_UP, sprite_bank->getSprites().size()-1);
     button->setSprite(EGBS_BUTTON_DOWN, sprite_bank->getSprites().size()-1);
     */
}
// -----------------------------------------------------------------------------
void IconButtonWidget::setLabel(std::string new_label)
{
    if (label == NULL) return;
    
    label->setText( stringw(new_label.c_str()).c_str() );
}
