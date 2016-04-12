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



#ifndef HEADER_BUTTON_HPP
#define HEADER_BUTTON_HPP

/**
  * \defgroup widgetsgroup Guiengine/Widgets
  * Contains the various types of widgets supported by the GUI engine.
  */

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /**
      * \brief A text button widget.
      * \ingroup widgetsgroup
      */
    class ButtonWidget : public Widget
    {
    public:
        
        LEAK_CHECK()
        
        ButtonWidget();
        virtual ~ButtonWidget() {}
        
        /** \brief Implement callback from base class Widget */
        void add();
        
        /**
          * \brief Change the label on the button
          * \pre This should only be called after a widget has been add()ed (changing the label
          *               before the widget is added can be done by editing the 'text' property of Widget).
          */
        void setLabel(const irr::core::stringw &label);
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getWidthNeededAroundLabel()  const { return 35; }
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 4; }
    };
}

#endif
