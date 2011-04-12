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

namespace irr { namespace gui { class STKModifiedSpriteBank; } }

namespace GUIEngine
{
    /** \brief A vertical list widget with text entries
      * \ingroup widgets
      * \note items you add to a list are not kept after the the list is in was removed
      *       (i.e. you need to add items everytime the screen is shown)
      */
    class ListWidget : public Widget
    {
        /** \brief whether this list has icons */
        bool m_use_icons;
        
        /** \brief if m_use_icons is true, this will contain the icon bank */
        irr::gui::STKModifiedSpriteBank* m_icons;
                
        struct ListItem
        {
            std::string m_internal_name;
            irr::core::stringw m_label;
            int m_current_id;
        };
        std::vector< ListItem > m_items;

    public:
        ListWidget();
        
        SkinWidgetContainer m_selection_skin_info;
                
        /** \brief implement add method from base class GUIEngine::Widget */
        virtual void add();
        
        /** \brief implement callback from base class GUIEngine::Widget */
        virtual void unfocused(const int playerID);
        
        /** \brief implement callback from base class GUIEngine::Widget */
        virtual void elementRemoved();
        
        /** \brief set the icon bank to use for list entries.
          *
          * The height of list entries will be ajusted to the size of the highest icon.
          * Icons must therefore be at least as high as text.
          *
          * \note  the list widget does NOT take ownership of the bank, dso you must delete it when
          *        you're done with it (but do not delete it when the list widget is still active)
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        void setIcons(irr::gui::STKModifiedSpriteBank* icons);
        
        
        // ---- contents management
        
        /**
         * \brief add an item to the list
         * \param name   user-visible, potentially translated, name of the item
         * \param icon   ID of the icon within the icon bank. Only used if an icon bank was passed.
         * \precondition may only be called after the widget has been added to the screen with add()
         */
        void addItem(const std::string& internal_name, 
                     const irr::core::stringw &name, const int icon=-1);
        
        /**
          * \brief erases all items in the list
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        void clear();
        
        /**
          * \return the number of items in the list
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        int getItemCount() const;
        
        /**
          * \return the index of the selected element within the list, or -1 if none
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        int getSelectionID() const;
        
        /**
          * \return the text of the selected item
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        std::string getSelectionInternalName();
        
        irr::core::stringw getSelectionLabel() const;
        
        /**
          * \brief Finds the ID of the item that has a given internal name
          */
        int getItemID(const std::string internalName) const;
        
        /**
          * \brief change the selected item
          * \param index the index of the element to select within the list, or -1 to select nothing
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        void setSelectionID(const int index);
        
        /**
          * \brief rename an item and/or change its icon based on its ID
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        void renameItem(const int itemID, const irr::core::stringw newName, const int icon=-1);
        
        /**
          * \brief rename an item and/or change its icon based on its internal name
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        void renameItem(const std::string internalName, const irr::core::stringw newName,
                        const int icon=-1)
        {
            const int id = getItemID(internalName);
            assert(id != -1);
            renameItem( id, newName, icon );
        }
        
        /**
          * \brief Make an item red to mark an error, for instance
          * \precondition may only be called after the widget has been added to the screen with add()
          */
        void markItemRed(const int id);
        
        /**
          * \brief Make an item red to mark an error, for instance
          * \precondition may only be called after the widget has been added to the screen with add()
          */        
        void markItemRed(const std::string internalName)
        {
            const int id = getItemID(internalName);
            assert(id != -1);
            markItemRed( id );
        }
    };
}

#endif
