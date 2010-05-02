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



#ifndef HEADER_LISTWIDGET_HPP
#define HEADER_LISTWIDGET_HPP

#include <irrlicht.h>

#include "guiengine/widget.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /** \brief A vertical list widget with text entries
      * \ingroup widgets
      */
    class ListWidget : public Widget
    {
        bool m_use_icons;
        
    public:
        ListWidget();
        
        SkinWidgetContainer m_selection_skin_info;
                
        /** \brief implement add method from base class GUIEngine::Widget */
        virtual void add();
        
        /** \brief implement callback from base class GUIEngine::Widget */
        virtual void unfocused(const int playerID);
        
        // ---- contents management
        
        /** \brief add an item to the list */
        void addItem(const char* item);

        /** \brief erases all items in the list */
        void clear();
        
        /** \return the number of items in the list */
        int getItemCount() const;
        
        /** \return the index of the selected element within the list, or -1 if none */
        int getSelectionID() const;
        
        /** \return the text of the selected item */
        std::string getSelectionName() const;
        
        /**
          * \brief change the selected item
          * \param index the index of the element to select within the list, or -1 to select nothing
          */
        void setSelectionID(const int index);
    };
}

#endif
