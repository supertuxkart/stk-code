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



#ifndef HEADER_CHECKBOX_HPP
#define HEADER_CHECKBOX_HPP

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /**
      * \brief A checkbox widget.
      * \ingroup widgetsgroup
      */
    class CheckBoxWidget : public Widget
    {
        bool m_state;
        EventPropagation transmitEvent(Widget* w,
                                       const std::string& originator,
                                       const int playerID);
        
    public:
        
        LEAK_CHECK()
        
        CheckBoxWidget();
        virtual ~CheckBoxWidget() {}
        
        /** \brief Implement callback from parent class Widget */
        void add();
        
        /** Get whether the checkbox is checked */
        bool getState() const { return m_state; }
        
        /** Set whether the checkbox is checked */
        void setState(const bool checked)  { m_state = checked; }
        
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 10; }
    };
}

#endif
