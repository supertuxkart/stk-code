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
#include "guiengine/ribbon_widget.hpp"
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
        
        /* reference pointers only, the actual instances are owned by m_children */
        ptr_vector<RibbonWidget, REF> m_rows;
        
        std::vector<ItemDescription> m_items;
        IGUIStaticText* m_label;
        RibbonWidget* getSelectedRibbon() const;
        RibbonWidget* getRowContaining(Widget* w) const;
        
        void updateLabel(RibbonWidget* from_this_ribbon=NULL);
        
        void propagateSelection();
        void focused();
        
        bool transmitEvent(Widget* w, std::string& originator);
        
        void scroll(const int x_delta);
        
        int m_scroll_offset;
        int m_needed_cols;
        int m_col_amount;
        int m_max_rows;
        bool m_combo;
        
        bool m_has_label;
        
        /* reference pointers only, the actual instances are owned by m_children */
        Widget* m_left_widget;
        Widget* m_right_widget;
    public:
        RibbonGridWidget(const bool combo=false, const int max_rows=4);
        
        void registerHoverListener(RibbonGridHoverListener* listener);
        
        void add();
        bool rightPressed();
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
