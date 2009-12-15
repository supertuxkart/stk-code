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



#ifndef HEADER_SPINNER_HPP
#define HEADER_SPINNER_HPP

#include <irrlicht.h>

#include "guiengine/widget.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /** A spinner or gauge widget (to select numbers / percentages). See guiengine/engine.hpp for a detailed overview */
    class SpinnerWidget : public Widget
    {
        int m_value, m_min, m_max;
        std::vector<irr::core::stringw> m_labels;
        bool m_graphical;
        bool m_gauge;
        
        EventPropagation transmitEvent(Widget* w, std::string& originator, const int playerID);
        EventPropagation rightPressed(const int playerID);
        EventPropagation leftPressed(const int playerID);
    public:
        
        SpinnerWidget(const bool gauge=false);
        virtual ~SpinnerWidget() {}
        virtual void move(const int x, const int y, const int w, const int h);
                
        void addLabel(irr::core::stringw label);
        void clearLabels();
        irr::core::stringw getStringValue() const;

        void add();
        void setValue(const int new_value);

        bool isGauge()  const { return m_gauge; }
        int  getValue() const { return m_value; }
        int  getMax()   const { return m_max;   }
        int  getMin()   const { return m_min;   }
    };
    
}

#endif
