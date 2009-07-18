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

    class RibbonWidget : public Widget
    {
        friend class RibbonGridWidget;
        friend class EventHandler;
        
        int m_selection;
        RibbonType m_ribbon_type;
        
        void add();
        
        bool rightPressed();
        bool leftPressed();
        bool mouseHovered(Widget* child);
        
        void updateSelection();
        bool transmitEvent(Widget* w, std::string& originator);
        void focused();
        
        ptr_vector<IGUIStaticText, REF> m_labels;
    public:
        Widget* m_focus;
        
        RibbonWidget(const RibbonType type=RIBBON_COMBO);
        virtual ~RibbonWidget() {}
        
        int getSelection() const { return m_selection; }
        void setSelection(const int i) { m_selection = i; updateSelection(); }
        void select(std::string item);
        
        RibbonType getRibbonType() const { return m_ribbon_type; }
        const std::string& getSelectionIDString() { return m_children[m_selection].m_properties[PROP_ID]; }
        const std::string& getSelectionText() { return m_children[m_selection].m_properties[PROP_TEXT]; }
        void setLabel(const int id, std::string new_name);
        
    };

}

#endif
