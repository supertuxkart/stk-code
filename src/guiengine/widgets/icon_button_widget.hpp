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



#ifndef HEADER_IBTN_HPP
#define HEADER_IBTN_HPP

#include <irrlicht.h>

#include "guiengine/widget.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /** A button widget with an icon and optionnaly a label beneath (from its properties in base class Widget)
        See guiengine/engine.hpp for a detailed overview */
    class IconButtonWidget : public Widget
    {
        bool clickable;
        irr::gui::IGUIStaticText* label;
    public:
        IconButtonWidget(const bool clickable=true);
        virtual ~IconButtonWidget() {}
        
        void add();
        
        /** Change the text label if there is a label (label won't be added if there initially wasn't one) */
        void setLabel(std::string new_label);
    };
}

#endif
