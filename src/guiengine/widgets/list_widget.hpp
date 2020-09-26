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



#ifndef HEADER_LISTWIDGET_HPP
#define HEADER_LISTWIDGET_HPP

#include <irrString.h>

#include "guiengine/widgets/CGUISTKListBox.hpp"
#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"
#include "IGUIElement.h"


namespace irr { namespace gui { class STKModifiedSpriteBank; } }

namespace GUIEngine
{
    class IListWidgetHeaderListener
    {
    public:
        virtual ~IListWidgetHeaderListener(){}
        
        virtual void onColumnClicked(int column_id, bool sort_desc, bool sort_default) = 0;
    };
    
    /** \brief A vertical list widget with text entries
      * \ingroup widgetsgroup
      * \note items you add to a list are not kept after the the list is in was removed
      *       (i.e. you need to add items everytime the screen is shown)
      */
    class ListWidget : public Widget
    {
        friend class Skin;
        
        /** \brief whether this list has icons */
        bool m_use_icons;
        
        /** \brief if m_use_icons is true, this will contain the icon bank */
        irr::gui::STKModifiedSpriteBank* m_icons;
                
        PtrVector< Widget > m_header_elements;
        
        Widget* m_selected_column;
        
        /** \brief whether this list is sorted in descending order */
        bool m_sort_desc;
        
        /** true when deault sorting is enabled */
        bool m_sort_default;
        
        /** index of column*/
        int m_sort_col;

        bool m_choosing_header;

        struct Column
        {
            irr::core::stringw m_text;
            int m_proportion;
            irr::video::ITexture* m_texture;
            Column(irr::core::stringw text, int proportion)
            {
                m_text = text;
                m_proportion = proportion;
                m_texture = NULL;
            }
            Column(irr::video::ITexture* texture, int proportion)
            {
                m_proportion = proportion;
                m_texture = texture;
            }
        };
        
        /** Leave empty for no header */
        std::vector< Column > m_header;
        
        IListWidgetHeaderListener* m_listener;

        bool m_sortable;

        bool m_header_created;

        void repairSortCol()
        {
            // Exclude scrollbar
            int max_size = (int)m_header_elements.size() - 1;
            if (m_sort_col < 0)
                m_sort_col = max_size - 1;
            else if (m_sort_col >= max_size)
                m_sort_col = 0;
        }

    public:
        typedef irr::gui::CGUISTKListBox::ListItem ListItem;
        typedef ListItem::ListCell ListCell;
        
        LEAK_CHECK()
        
        ListWidget();
        
        SkinWidgetContainer m_selection_skin_info;
                
        /** \brief implement add method from base class GUIEngine::Widget */
        virtual void add();
        
        /** \brief implement callback from base class GUIEngine::Widget */
        virtual void unfocused(const int playerID, Widget* new_focus);
        
        /** \brief implement callback from base class GUIEngine::Widget */
        virtual void elementRemoved();
        
        /** \brief set the icon bank to use for list entries.
          *
          * The height of list entries will be ajusted to the size of the highest icon.
          * Icons must therefore be at least as high as text.
          *
          * \note  the list widget does NOT take ownership of the bank, dso you must delete it when
          *        you're done with it (but do not delete it when the list widget is still active)
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void setIcons(irr::gui::STKModifiedSpriteBank* icons, int size=-1);
        
        
        // ---- contents management
        
        /**
         * \brief add an item to the list
         * \param name   user-visible, potentially translated, name of the item
         * \param icon   ID of the icon within the icon bank. Only used if an icon bank was passed.
         * \pre may only be called after the widget has been added to the screen with add()
         */
        void addItem(   const std::string& internal_name,
                        const irr::core::stringw &name,
                        const int icon=-1,
                        bool center = false);

        void addItem(   const std::string& internal_name,
                        const std::vector<ListCell>& contents);

        /**
          * \brief create a header based on m_header
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void createHeader();
        
        /**
          * \brief erases all items in the list, don't clear header
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void clear();

        /**
          * \brief clear the header
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void clearColumns();
        
        /**
          * \return the number of items in the list
          * \pre may only be called after the widget has been added to the screen with add()
          */
        int getItemCount() const;
        
        /**
          * \return the index of the selected element within the list, or -1 if none
          * \pre may only be called after the widget has been added to the screen with add()
          */
        int getSelectionID() const;
        
        /**
          * \return the text of the selected item
          * \pre may only be called after the widget has been added to the screen with add()
          */
        std::string getSelectionInternalName();
        
        irr::core::stringw getSelectionLabel(const int cell = 0) const;
        
        void selectItemWithLabel(const irr::core::stringw& name);
        
        /**
          * \brief Finds the ID of the item that has a given internal name
          */
        int getItemID(const std::string &internalName) const;
        
        /**
          * \brief change the selected item
          * \param index the index of the element to select within the list, or -1 to select nothing
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void setSelectionID(const int index);
        
        /**
          * \brief rename an item and/or change its icon based on its ID
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void renameCell(const int row_num, const int col_num, 
                        const irr::core::stringw &newName, const int icon=-1);
        
        /**
         * renames first cell only
         */
        void renameItem(const int row_num, 
                        const irr::core::stringw &newName, const int icon=-1);
        void renameItem(const std::string  & internal_name, 
                        const irr::core::stringw &newName, const int icon=-1);

        /**
          * \brief rename an item and/or change its icon based on its internal name
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void renameCell(const std::string internalName, const int col_num, 
                        const irr::core::stringw &newName, const int icon=-1)
        {
            const int id = getItemID(internalName);
            assert(id != -1);
            renameCell( id, col_num, newName, icon );
        }
        
        /**
          * \brief Make an item red to mark an error, for instance
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void markItemRed(const int id, bool red=true);
        void markItemBlue(const int id, bool blue=true);
        void emphasisItem(const int id, bool enable=true);
        
        /**
          * \brief Make an item red to mark an error, for instance
          * \pre may only be called after the widget has been added to the screen with add()
          */
        void markItemRed(const std::string &internalName, bool red=true)
        {
            const int id = getItemID(internalName);
            assert(id != -1);
            markItemRed( id, red );
        }

        void markItemBlue(const std::string &internalName, bool blue=true)
        {
            const int id = getItemID(internalName);
            assert(id != -1);
            markItemBlue( id, blue );
        }

        void emphasisItem(const std::string &internalName, bool enable=true)
        {
            const int id = getItemID(internalName);
            assert(id != -1);
            emphasisItem(id, enable);
        }

        /** Override callback from Widget */
        virtual EventPropagation transmitEvent(Widget* w,
                                               const std::string& originator,
                                               const int playerID);

        /** \brief implementing method from base class Widget */
        virtual EventPropagation upPressed(const int playerID);
        
        /** \brief implementing method from base class Widget */
        virtual EventPropagation downPressed(const int playerID);

        /** \brief implementing method from base class Widget */
        virtual EventPropagation leftPressed(const int playerID);

        /** \brief implementing method from base class Widget */
        virtual EventPropagation rightPressed(const int playerID);

        /** \brief implement common core parts of upPressed and downPressed */ 
        EventPropagation moveToNextItem(const bool down);
        
        void setColumnListener(IListWidgetHeaderListener* listener)
        {
            if (m_listener) delete m_listener;
            m_listener = listener;
        }
        
        /** Columns are persistent across multiple "clear" add/remove cycles. clearColumns clear them immediately.
          * \param proportion A column with proportion 2 will be twice as large as a column with proportion 1
          */
        void addColumn(irr::core::stringw col, int proportion=1) { m_header.push_back( Column(col, proportion) ); }
        void addColumn(irr::video::ITexture* tex, int proportion=1) { m_header.push_back( Column(tex, proportion) ); }

        void setSortable(bool sortable) { m_sortable = sortable; }
        void focusHeader(const NavigationDirection nav);
        virtual void setActive(bool active=true);
    };
}

#endif
