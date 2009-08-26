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



#ifndef HEADER_RIBBONGRID_HPP
#define HEADER_RIBBONGRID_HPP

#include <irrlicht.h>

#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "utils/ptr_vector.hpp"

using namespace irr;
using namespace gui;

namespace GUIEngine
{
    /**
     * Even if you have a ribbon that only acts on click/enter, you may wish to know which
     * item is currently highlighted. In this case, create a listener and pass it to the ribbon.
     */
    class RibbonGridHoverListener
    {
    public:
        virtual ~RibbonGridHoverListener() {}
        virtual void onSelectionChanged(RibbonGridWidget* theWidget, const std::string& selectionID) = 0;
    };
    
    struct ItemDescription
    {
        std::string m_user_name;
        std::string m_code_name;
        std::string m_sshot_file;
    };
    
    class RibbonGridWidget : public Widget
    {
        friend class RibbonWidget;
        
        ptr_vector<RibbonGridHoverListener> m_hover_listeners;
        
        virtual ~RibbonGridWidget() {}
        
        /** reference pointers only, the actual instances are owned by m_children */
        ptr_vector<RibbonWidget, REF> m_rows;
        
        /** Used for ribbon grids that have a label */
        bool m_has_label;
        IGUIStaticText* m_label;
        int m_label_height;
        
        /** Used to keep track of item count changes */
        int m_previous_item_count;
        
        /** List of items in the ribbon */
        std::vector<ItemDescription> m_items;
        
        /** Width of the scrolling arrows on each side */
        int m_arrows_w;
        
        int m_scroll_offset;
        int m_needed_cols;
        
        int m_child_width, m_child_height;
        
        /** Number of rows and columns. Number of columns can dynamically change, number of row is
            determined at creation */
        int m_row_amount;
        int m_col_amount;
        
        int m_max_rows;
        std::vector<int> m_ids;
        
        /** Whether this is a "combo" style ribbon grid widget */
        bool m_combo;
        
        /* reference pointers only, the actual instances are owned by m_children */
        Widget* m_left_widget;
        Widget* m_right_widget;
        
        RibbonWidget* getSelectedRibbon() const;
        RibbonWidget* getRowContaining(Widget* w) const;
        
        /** Updates the visible label to match the currently selected item */
        void updateLabel(RibbonWidget* from_this_ribbon=NULL);
        
        /** Even though the ribbon grid widget looks like a grid, it is really a vertical stack of
            independant ribbons. When moving selection horizontally, this method is used to notify
            other rows above and below of which column is selected, so that moving vertically to
            another row keeps the same selected column. */
        void propagateSelection();
        
        /** Callback called widget is focused */
        void focused();
        
        bool transmitEvent(Widget* w, std::string& originator);
        
        /** Removes all previously added contents icons, and re-adds them (calculating the new amount) */
        void setSubElements();

        void scroll(const int x_delta);
        
        /** Used  for combo ribbons, to contain the ID of the currently selected item */
        int m_selected_item;
        
    public:
        RibbonGridWidget(const bool combo=false, const int max_rows=4);
        
        void registerHoverListener(RibbonGridHoverListener* listener);
        
        void add();
        
        /** Called when right key is pressed */
        bool rightPressed();
        
        /** Called when left key is pressed */
        bool leftPressed();
        
        void addItem( std::string user_name, std::string code_name, std::string image_file );
        
        void updateItemDisplay();
        
        bool mouseHovered(Widget* child);
        void onRowChange(RibbonWidget* row);
        
        const std::string& getSelectionIDString();
        const std::string& getSelectionText();
        
        void setSelection(int item_id);
        void setSelection(const std::string& code_name);
    };
    
}

#endif
