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



#ifndef HEADER_RIBBON_HPP
#define HEADER_RIBBON_HPP

#include <irrlicht.h>

#include "guiengine/widget.hpp"
#include "utils/ptr_vector.hpp"

using namespace irr;
using namespace gui;

namespace GUIEngine
{

    enum RibbonType
    {
        RIBBON_COMBO, /* select one item out of many, like in a combo box */
        RIBBON_TOOLBAR, /* a row of individual buttons */
        RIBBON_TABS /* a tab bar */
    };

    /** A static text/icons/tabs bar widget. The contents of this ribbon are static.
        See guiengine/engine.hpp for a detailed overview */
    class RibbonWidget : public Widget
    {
        friend class RibbonGridWidget;
        friend class EventHandler;
        
        int m_selection;
        
        /** The type of this ribbon (toolbar, combo, tabs) */
        RibbonType m_ribbon_type;
        
        void add();
        
        /** Each item within the ribbon holds a flag saying whether it is selected or not.
            This method updates the flag in all of this ribbon's children. Called everytime
            selection changes.*/
        void updateSelection();
        
        /** Callbacks */
        bool rightPressed();
        bool leftPressed();
        bool mouseHovered(Widget* child);
        bool transmitEvent(Widget* w, std::string& originator);
        void focused();
        
        ptr_vector<IGUIStaticText, REF> m_labels;
        
    public:
        
        /** Contains which element within the ribbon is currently focused (used by the skin) */
        Widget* m_focus;
        
        RibbonWidget(const RibbonType type=RIBBON_COMBO);
        virtual ~RibbonWidget() {}
        
        /** Returns the numerical ID of the selected item within the ribbon */
        int getSelection() const { return m_selection; }
        
        /** Returns the string ID (internal name) of the selection */
        const std::string& getSelectionIDString() { return m_children[m_selection].m_properties[PROP_ID]; }
        
        /** Returns the user-visible text of the selection */
        const std::string& getSelectionText() { return m_children[m_selection].m_properties[PROP_TEXT]; }
        
        /** Returns the type of this ribbon (see guiengine/engine.hpp for detaield descriptions) */
        RibbonType getRibbonType() const { return m_ribbon_type; }
        
        /** Sets the ID of the selected item within the ribbon */
        void setSelection(const int i) { m_selection = i; updateSelection(); }
        
        /** Select an item in the ribbon by its internal name */
        void select(std::string item);
        
        /** When each item has a label, this method can be used to rename an item 
            (especially used in scrolling ribbons, when scrolling occurs by renaming
            items - note that this statis ribbon doesn't support scrolling, only
            superclasses/wrappers of this do.) */
        void setLabel(const int id, std::string new_name);
        
    };

}

#endif
