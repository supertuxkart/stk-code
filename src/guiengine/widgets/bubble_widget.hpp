//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#ifndef __BUBBLE_WIDGET_HPP__
#define __BUBBLE_WIDGET_HPP__

#include <irrlicht.h>
#include "guiengine/widget.hpp"

namespace GUIEngine
{
    const int BUBBLE_MARGIN_ON_RIGHT = 15;
    
    class BubbleWidget : public Widget
    {
        friend class Skin;
        
        /** shrinked size of this widget (size allowed in layout; internal text may be bigger than that).
          * If the text all fits in the allowed layout space, m_shrinked_size == m_expanded_size.
          */
        irr::core::rect<irr::s32> m_shrinked_size;
        
        /** Expanded size of this widget (size to see all text inside the bubble).
          * If the text all fits in the allowed layout space, m_shrinked_size == m_expanded_size.
          */
        irr::core::rect<irr::s32> m_expanded_size;
        
        /** Text shrinked to fit into the allowed layout space (will be same as m_text if all text fits) */
        irr::core::stringw m_shrinked_text;
        
        /** For the skin to create the zooming effect */
        float m_zoom;
        
    public:
        
        BubbleWidget();
        
        virtual void add();
        
        virtual EventPropagation focused(const int playerID);
        
        void updateSize();
    };

}

#endif
