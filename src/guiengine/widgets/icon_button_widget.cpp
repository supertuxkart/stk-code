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

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::video;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------
IconButtonWidget::IconButtonWidget(ScaleMode scale_mode, const bool tab_stop,
                                   const bool focusable, IconPathType pathType) : Widget(WTYPE_ICON_BUTTON)
{
    m_label = NULL;
    m_texture = NULL;
    m_custom_aspect_ratio = 1.0f;

    m_tab_stop = tab_stop;
    m_focusable = focusable;
    m_scale_mode = scale_mode;
    
    m_icon_path_type = pathType;
}
// -----------------------------------------------------------------------------
void IconButtonWidget::add()
{
    // ---- Icon
    if (m_icon_path_type == ICON_PATH_TYPE_ABSOLUTE)
    {
        m_texture = irr_driver->getTexture(m_properties[PROP_ICON].c_str());
    }
    else if (m_icon_path_type == ICON_PATH_TYPE_RELATIVE)
    {
        m_texture = irr_driver->getTexture((file_manager->getDataDir() + "/" +
                                            m_properties[PROP_ICON]).c_str());
    }
    
    if (m_texture == NULL)
    {
        std::cerr << "IconButtonWidget::add() : error, cannot find texture "
                  << m_properties[PROP_ICON].c_str() << std::endl;
        m_texture = irr_driver->getTexture((file_manager->getDataDir() + "/gui/main_help.png").c_str());
        return;
    }
    m_texture_w = m_texture->getSize().Width;
    m_texture_h = m_texture->getSize().Height;

    // irrlicht widgets don't support scaling while keeping aspect ratio
    // so, happily, let's implement it ourselves
    float useAspectRatio = -1.0f;
    
    if (m_scale_mode == SCALE_MODE_KEEP_TEXTURE_ASPECT_RATIO)
    {
        useAspectRatio = (float)m_texture_w / (float)m_texture_h;
        //std::cout << "m_texture_h=" << m_texture_h << "; m_texture_w="<< m_texture_w
        //          << "; useAspectRatio=" << useAspectRatio << std::endl;
    }
    else if (m_scale_mode == SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO)
    {
        useAspectRatio = m_custom_aspect_ratio;
    }
    
    int suggested_h = m_h;
    int suggested_w = (int)((useAspectRatio < 0 ? m_w : useAspectRatio * suggested_h));
    
    if (suggested_w > m_w)
    {
        const float needed_scale_factor = (float)m_w / (float)suggested_w;
        suggested_w = (int)(suggested_w*needed_scale_factor);
        suggested_h = (int)(suggested_h*needed_scale_factor);
    }
    const int x_from = m_x + (m_w - suggested_w)/2; // center horizontally
    const int y_from = m_y + (m_h - suggested_h)/2; // center vertically
    
    rect<s32> widget_size = rect<s32>(x_from,
                                      y_from,
                                      x_from + suggested_w,
                                      y_from + suggested_h);
    
    IGUIButton* btn = GUIEngine::getGUIEnv()->addButton(widget_size,
                                                        m_parent,
                                                        (m_tab_stop ? getNewID() : getNewNoFocusID()),
                                                        L"");

    btn->setTabStop(m_tab_stop);
    m_element = btn;
    m_id = m_element->getID();
    
    // ---- label if any
    const stringw& message = getText();
    if (message.size() > 0)
    {
        //std::cout << "Adding label of icon widget, m_properties[PROP_EXTEND_LABEL] = "
        //          << m_properties[PROP_EXTEND_LABEL] << std::endl;
        const int label_extra_size = ( m_properties[PROP_EXTEND_LABEL].size() == 0 ?
                                       0 : atoi(m_properties[PROP_EXTEND_LABEL].c_str()) );
        
        const bool word_wrap = (m_properties[PROP_WORD_WRAP] == "true");
        
        // leave enough room for two lines of text if word wrap is enabled, otherwise a single line
        widget_size = rect<s32>(m_x - label_extra_size/2,
                                m_y + m_h,
                                m_x + m_w + label_extra_size/2,
                                m_y + m_h + (word_wrap ? GUIEngine::getFontHeight()*2 :
                                                         GUIEngine::getFontHeight()));

        m_label = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size,
                                                        false, word_wrap, m_parent);
        m_label->setTextAlignment(EGUIA_CENTER, EGUIA_UPPERLEFT);
        m_label->setTabStop(false);
        m_label->setNotClipped(true);
        
        const int max_w = m_label->getAbsolutePosition().getWidth();
        if (!word_wrap &&
            (int)GUIEngine::getFont()->getDimension(message.c_str()).Width > max_w + 4) // arbitrarily allow for 4 pixels
        {
            m_label->setOverrideFont( GUIEngine::getSmallFont() );
        }
        
#if IRRLICHT_VERSION_MAJOR > 1 || (IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR >= 8)
        m_label->setRightToLeft( translations->isRTLLanguage() );
        m_label->setTextRestrainedInside(false);
#endif
    }
    
    // ---- IDs
    m_id = m_element->getID();
    if (m_tab_stop) m_element->setTabOrder(m_id);
    m_element->setTabGroup(false);
}
// -----------------------------------------------------------------------------
/** \precondition At the moment, the new texture must have the same aspct ratio
  *               as the previous one since the object will not be modified
  *               modified to fit a different aspect ratio
  */
void IconButtonWidget::setImage(const char* path_to_texture, IconPathType pathType)
{
    if (pathType != ICON_PATH_TYPE_NO_CHANGE)
    {
        m_icon_path_type = pathType;
    }
    
    m_properties[PROP_ICON] = path_to_texture;
    
    if (m_icon_path_type == ICON_PATH_TYPE_ABSOLUTE)
    {
        m_texture = irr_driver->getTexture(m_properties[PROP_ICON].c_str());
    }
    else if (m_icon_path_type == ICON_PATH_TYPE_RELATIVE)
    {
        m_texture = irr_driver->getTexture((file_manager->getDataDir() + "/" + m_properties[PROP_ICON]).c_str());
    }
    
    if (!m_texture)
    {
        fprintf(stderr, "Texture '%s' not found!\n",  m_properties[PROP_ICON].c_str());
        m_texture = irr_driver->getTexture((file_manager->getDataDir() + "/gui/main_help.png").c_str());
    }

    m_texture_w = m_texture->getSize().Width;
    m_texture_h = m_texture->getSize().Height;
}
// -----------------------------------------------------------------------------
/** \precondition At the moment, the new texture must have the same aspct ratio
 *                as the previous one since the object will not
 *                be modified to fit a different aspect ratio
 */
void IconButtonWidget::setImage(ITexture* texture)
{
    if (texture != NULL)
    {
        m_texture = texture;
        
        m_texture_w = m_texture->getSize().Width;
        m_texture_h = m_texture->getSize().Height;
    }
    else
    {
        fprintf(stderr, "[IconButtonWidget::setImage] invoked with NULL image pointer\n");
        m_texture = irr_driver->getTexture((file_manager->getDataDir() + "/gui/main_help.png").c_str());
    }
}
// -----------------------------------------------------------------------------
void IconButtonWidget::setLabel(stringw new_label)
{
    // FIXME: does not update m_text. Is this a behaviour we want?
    if (m_label == NULL) return;
    
    m_label->setText( new_label.c_str() );
    
    const bool word_wrap = (m_properties[PROP_WORD_WRAP] == "true");
    const int max_w = m_label->getAbsolutePosition().getWidth();
    
    if (!word_wrap &&
        (int)GUIEngine::getFont()->getDimension(new_label.c_str()).Width > max_w + 4) // arbitrarily allow for 4 pixels
    {
        m_label->setOverrideFont( GUIEngine::getSmallFont() );
    }
    else
    {
        m_label->setOverrideFont( NULL );
    }
}
