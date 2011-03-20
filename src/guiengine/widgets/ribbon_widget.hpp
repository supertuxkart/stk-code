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
#include "guiengine/widgets/icon_button_widget.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{

    enum RibbonType
    {
        RIBBON_COMBO, /* select one item out of many, like in a combo box */
        RIBBON_TOOLBAR, /* a row of individual buttons */
        RIBBON_TABS /* a tab bar */
    };

    /** \brief A static text/icons/tabs bar widget.
      * The contents of this ribbon are static.
      * \ingroup widgets
      * \note items you add to a list are kept after the the ribbon was in is removed
      *       (i.e. you don't need to add items everytime the screen is shown, only upon loading)
      */
    class RibbonWidget : public Widget
    {
    public:
        class IRibbonListener
        {
        public:
            virtual ~IRibbonListener(){}
            virtual void onRibbonWidgetScroll(const int delta_x) = 0;
            virtual void onRibbonWidgetFocus(RibbonWidget* emitter, const int playerID) = 0;
            virtual void onSelectionChange() = 0;
        };
        
    private:
        friend class DynamicRibbonWidget;
        friend class EventHandler;
        
        int m_selection[MAX_PLAYER_COUNT];
        
        /** The type of this ribbon (toolbar, combo, tabs) */
        RibbonType m_ribbon_type;
                
        /** Each item within the ribbon holds a flag saying whether it is selected or not.
            This method updates the flag in all of this ribbon's children. Called everytime
            selection changes.*/
        void updateSelection();
        
        /** Callbacks */
        virtual EventPropagation rightPressed(const int playerID=0);
        virtual EventPropagation leftPressed(const int playerID=0);
        virtual EventPropagation mouseHovered(Widget* child, const int playerID);
        virtual EventPropagation transmitEvent(Widget* w, std::string& originator, const int playerID=0);
        virtual EventPropagation focused(const int playerID);
        virtual void unfocused(const int playerID);
        
        PtrVector<irr::gui::IGUIStaticText, REF> m_labels;
        
        IRibbonListener* m_listener;
        
    public:
        
        /** 
         * Internal identifier of filler items that are added in a ribbon widget to fill
         * lines when the number of items cannot be divided by the number of rows in the grid
         * (mostly used by dynamic ribbon widgets, but the base ribbon needs to know about filler items)
         */
        static const char* NO_ITEM_ID;
        
        /** Contains which element within the ribbon is currently focused by player 0 (used by the skin to
            show mouse hovers over items that are not selected). Only used for COMBO and TAB ribbons. */
        Widget* m_mouse_focus;
        
        RibbonWidget(const RibbonType type=RIBBON_COMBO);
        virtual ~RibbonWidget() {}
        
        void add();

        /** Sets a listener that will be notified of changes on this ribbon.
          * Does _not_ take ownership of the listener, i.e. will not delete it.
          * You may call this with the listener parameter set to NULL to remove the listener. */
        void setListener(IRibbonListener* listener) { m_listener = listener; }
        
        /** Returns the type of this ribbon (see guiengine/engine.hpp for detailed descriptions) */
        RibbonType getRibbonType() const { return m_ribbon_type; }
        
        /** Returns the numerical ID of the selected item within the ribbon */
        int getSelection(const int playerID) const { return m_selection[playerID]; }
        
        /** Returns the string ID (internal name) of the selection */
        const std::string& getSelectionIDString(const int playerID);
        
        /** Returns the user-visible text of the selection */
        const irr::core::stringw& getSelectionText(const int playerID) { return m_children[m_selection[playerID]].m_text; }
        
        /** Sets the ID of the selected item within the ribbon */
        void setSelection(const int i, const int playerID) { m_selection[playerID] = i; updateSelection(); }
        
        /** Select an item in the ribbon by its internal name */
        void select(std::string item, const int playerID);
        
        /** When each item has a label, this method can be used to rename an item 
            (especially used in scrolling ribbons, when scrolling occurs by renaming
            items - note that this statis ribbon doesn't support scrolling, only
            superclasses/wrappers of this do.) */
        void setLabel(const int id, irr::core::stringw new_name);
        
        /** Returns the ID of the item, or -1 if not found */
        int findItemNamed(const char* internalName);
        
        /** \brief        dynamically (at runtime) add a text item to this ribbon
          * \precondition this must be called before RibbonWidget::add, while the widget is not yet displayed
          * \precondition only valid for ribbons that take text-only contents (e.g. tab bars)
          */
        void addTextChild(const wchar_t* text, const std::string id);
        
        
        /** \brief        dynamically (at runtime) add an icon item to this ribbon
         *  \precondition this must be called before RibbonWidget::add, while the widget is not yet displayed
         *  \precondition only valid for ribbons that take icon contents
         */
        void addIconChild(const wchar_t* text, const std::string id,
                          const int w, const int h, const std::string icon,
                          const IconButtonWidget::IconPathType iconPathType=IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    
        /**
          * \brief clear all children of this ribbon (likely because new ones will be added soon after)
          */
        void clearAllChildren();
        
        PtrVector<Widget>& getRibbonChildren() { return m_children; }
    };

}

#endif
