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

namespace GUIEngine
{
    /**
     * Even if you have a ribbon that only acts on click/enter, you may wish to know which
     * item is currently highlighted. In this case, create a listener and pass it to the ribbon.
     */
    class DynamicRibbonHoverListener
    {
    public:
        virtual ~DynamicRibbonHoverListener() {}
        virtual void onSelectionChanged(DynamicRibbonWidget* theWidget, const std::string& selectionID, 
                                        const irr::core::stringw& selectionText, const int playerID) = 0;
    };
    
    /** The description of an item added to a DynamicRibbonWidget */
    struct ItemDescription
    {
        irr::core::stringw m_user_name;
        std::string m_code_name;
        std::string m_sshot_file;
    };
    
    /** A dynamic ribbon (builds upon RibbonWidget, adding dynamic contents creation and sizing, scrolling, multiple-row
        layouts). See guiengine/engine.hpp for a detailed overview */
    class DynamicRibbonWidget : public Widget
    {
        friend class RibbonWidget;
        
        /** A list of all listeners that registered to be notified on hover/selection */
        ptr_vector<DynamicRibbonHoverListener> m_hover_listeners;
        
        virtual ~DynamicRibbonWidget() {}
        
        /** Used for ribbon grids that have a label at the bottom */
        bool m_has_label;
        irr::gui::IGUIStaticText* m_label;
        int m_label_height;
        
        /** Used to keep track of item count changes */
        int m_previous_item_count;
        
        /** List of items in the ribbon */
        std::vector<ItemDescription> m_items;
        
        /** Width of the scrolling arrows on each side */
        int m_arrows_w;
        
        /** Current scroll offset within items */
        int m_scroll_offset;
        
        /** Width and height of children as declared in the GUI file */
        int m_child_width, m_child_height;
        
        /** Number of rows and columns. Number of columns can dynamically change, number of row is
            determined at creation */
        int m_row_amount;
        int m_col_amount;
        
        /** The total number of columns given item count and row count (even counting not visible with current scrolling) */
        int m_needed_cols;
        
        /** The maximum number of rows, as passed to the constructor */
        int m_max_rows;
        
        /** irrlicht relies on consecutive IDs to perform keyboard navigation between widgets. However, since this
            widget is dynamic, irrlicht widgets are not created as early as all others, so by the time we're ready
            to create the full contents of this widget, the ID generator is already incremented, thus messing up
            keyboard navigation. To work around this, at the same time all other widgets are created, I gather a
            number of IDs (the number of rows) and store them here. Then, when we're finally ready to create the
            contents dynamically, we can re-use these IDs and get correct navigation order. */
        std::vector<int> m_ids;
        
        /** Whether this is a "combo" style ribbon grid widget */
        bool m_combo;
        
        /* reference pointers only, the actual instances are owned by m_children */
        Widget* m_left_widget;
        Widget* m_right_widget;
        
        /** Returns the currently selected row */
        RibbonWidget* getSelectedRibbon(const int playerID) const;
        
        /** Returns the row */
        RibbonWidget* getRowContaining(Widget* w) const;
        
        /** Updates the visible label to match the currently selected item */
        void updateLabel(RibbonWidget* from_this_ribbon=NULL);
        
        /** Even though the ribbon grid widget looks like a grid, it is really a vertical stack of
            independant ribbons. When moving selection horizontally, this method is used to notify
            other rows above and below of which column is selected, so that moving vertically to
            another row keeps the same selected column. */
        void propagateSelection();
        
        /** Callback called widget is focused */
        EventPropagation focused(const int playerID);
                
        /** Removes all previously added contents icons, and re-adds them (calculating the new amount) */
        void setSubElements();

        /** Call this to scroll within a scrollable ribbon */
        void scroll(const int x_delta);
        
        /** Used  for combo ribbons, to contain the ID of the currently selected item for each player */
        int m_selected_item[MAX_PLAYER_COUNT];
        
        /** Callbacks */
        void onRowChange(RibbonWidget* row, const int playerID);
        void add();
        EventPropagation mouseHovered(Widget* child);
        EventPropagation transmitEvent(Widget* w, std::string& originator, const int playerID);
        

    public:
        DynamicRibbonWidget(const bool combo=false, const int max_rows=4);
        
        /** Reference pointers only, the actual instances are owned by m_children. Used to create mtultiple-row
         ribbons (what appears to be a grid of icons is actually a vector of stacked basic ribbons) */
        ptr_vector<RibbonWidget, REF> m_rows;
        
        /** Dynamically add an item to the ribbon's list of items (will not be visible until you
         call 'updateItemDisplay' or 'add') */
        void addItem( const irr::core::stringw& user_name, const std::string& code_name, const std::string& image_file );
        
        /** Clears all items added through 'addItem'. You can then add new items with 'addItem' and call
            'updateItemDisplay' to update the display. */
        void clearItems();
        
        /** Register a listener to be notified of selection changes within the ribbon */
        void registerHoverListener(DynamicRibbonHoverListener* listener);
        
        /** Called when right key is pressed */
        EventPropagation rightPressed(const int playerID);
        
        /** Called when left key is pressed */
        EventPropagation leftPressed(const int playerID);
        
        /** Updates icons/labels given current items and scrolling offset, taking care of resizing
            the dynamic ribbon if the number of items changed */
        void updateItemDisplay();
        
        /** Get the internal name (ID) of the selected item */
        const std::string& getSelectionIDString(const int playerID);
        
        /** Get the user-visible text of the selected item */
        const irr::core::stringw& getSelectionText(const int playerID);
        
        /** Returns a read-only list of items added to this ribbon */
        const std::vector<ItemDescription>& getItems() const { return m_items; }
        
        /** Select an item from its numerical ID. Only for [1-row] combo ribbons.
         ID ranges from {0} to {number of items added through 'addItem' - 1}
         \return Whether setting the selection was successful (whether the item exists) */
        bool setSelection(int item_id, const int playerID);
        
        /** Select an item from its codename.
            \return Whether setting the selection was successful (whether the item exists) */
        bool setSelection(const std::string item_codename, const int playerID);
        
        /**
         * Called when irrLicht widgets cleared. Forget all references to them, they're no more valid.
         */
        virtual void elementRemoved();
    };
    
}

#endif
