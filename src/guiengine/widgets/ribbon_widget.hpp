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



#ifndef HEADER_RIBBON_HPP
#define HEADER_RIBBON_HPP

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "utils/cpp2011.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

#include <IGUIStaticText.h>

namespace GUIEngine
{
    /** Types of ribbons */
    enum RibbonType
    {
        RIBBON_COMBO,   //!< select one item out of many, like in a combo box
        RIBBON_TOOLBAR, //!< a row of individual buttons
        RIBBON_TABS,    //!< a tab bar
        RIBBON_VERTICAL_TABS //!< a vertical tab bar
    };
    
    /** Filp directions of ribbons */
    enum RibbonFlip
    {
        FLIP_NO, // For non-tab ribbons
        FLIP_UP_LEFT, // For horizontal tabs it goes up vertical ones it goes left
        FLIP_DOWN_RIGHT // For horizontal tabs it goes down vertical ones it goes right
    };

    /** \brief A static text/icons/tabs bar widget.
      * The contents of this ribbon are static.
      * \ingroup widgetsgroup
      * \note items you add to a list are kept after the the ribbon was in
      *       is removed (i.e. you don't need to add items everytime the
      *       screen is shown, only upon loading)
      */
    class RibbonWidget : public Widget
    {
    public:
        class IRibbonListener
        {
        public:
            virtual ~IRibbonListener(){}
            virtual void onRibbonWidgetScroll(const int delta_x) = 0;
            virtual void onRibbonWidgetFocus(RibbonWidget* emitter,
                                             const int playerID) = 0;
            virtual void onSelectionChange() = 0;
        };
        
    private:
        friend class DynamicRibbonWidget;
        friend class EventHandler;
        
        int m_selection[MAX_PLAYER_COUNT];
        
        /** The type of this ribbon (toolbar, combo, tabs, vertical tabs) */
        RibbonType m_ribbon_type;
        
        /** The flip direction of this ribbon */
        RibbonFlip m_ribbon_flip;
                
        /** Each item within the ribbon holds a flag saying whether it is
         *  selected or not. This method updates the flag in all of this
         *  ribbon's children. Called everytime selection changes.*/
        void updateSelection();

        /** Callbacks */
        virtual EventPropagation rightPressed(const int playerID=0) OVERRIDE;
        virtual EventPropagation leftPressed (const int playerID=0) OVERRIDE;
        virtual EventPropagation upPressed   (const int playerID=0) OVERRIDE;
        virtual EventPropagation downPressed (const int playerID=0) OVERRIDE;
        EventPropagation moveToNextItem(const bool horizontally, const bool reverse, const int playerID);
        EventPropagation propagationType(const bool horizontally);
        void selectNextActiveWidget(const bool horizontally, const bool reverse,
                                    const int playerID, const int old_selection);
        virtual EventPropagation mouseHovered(Widget* child,
                                              const int playerID) OVERRIDE;
        virtual EventPropagation transmitEvent(Widget* w,
                                               const std::string& originator,
                                               const int playerID=0) OVERRIDE;
        virtual EventPropagation focused(const int playerID) OVERRIDE;
        virtual void unfocused(const int playerID, Widget* new_focus) OVERRIDE;
        
        virtual EventPropagation onClick() OVERRIDE;
        
        PtrVector<irr::gui::IGUIStaticText, REF> m_labels;
        
        IRibbonListener* m_listener;
        PtrVector<Widget> m_active_children;
        
    public:
        
        LEAK_CHECK()
        
        /** Internal identifier of filler items that are added in a ribbon
         *  widget to filllines when the number of items cannot be divided
         *  by the number of rows in the grid (mostly used by dynamic ribbon
         *  widgets, but the base ribbon needs to know about filler items)
         */
        static const char NO_ITEM_ID[];
        
        /** Contains which element within the ribbon is currently focused by
         *  player 0 (used by the skin to show mouse hovers over items that
         *  are not selected). Only used for COMBO and TAB ribbons. */
        Widget* m_mouse_focus;
        
        RibbonWidget(const RibbonType type=RIBBON_COMBO);
        virtual ~RibbonWidget();
        
        virtual void add() OVERRIDE;

        /** Sets a listener that will be notified of changes on this ribbon.
         *  Does _not_ take ownership of the listener, i.e. will not delete it.
         *  You may call this with the listener parameter set to NULL to
         *  remove the listener. */
        void setListener(IRibbonListener* listener) { m_listener = listener; }
        // --------------------------------------------------------------------
        /** Returns the type of this ribbon (see the GUI module overview page
         *  for detailed descriptions) */
        RibbonType getRibbonType() const { return m_ribbon_type; }
        // --------------------------------------------------------------------
        /** Returns the flip direction of thin ribbon */
        RibbonFlip getRibbonFlip() const { return m_ribbon_flip; }
        // --------------------------------------------------------------------
        /** Returns the number of active items within the ribbon */
        int getActiveChildrenNumber(const int playerID) const
                                              { return m_active_children.size(); }
        // --------------------------------------------------------------------
        /** Returns the numerical ID of the selected item within the ribbon */
        int getSelection(const int playerID) const
                                              { return m_selection[playerID]; }
        // --------------------------------------------------------------------
        /** Returns the string ID (internal name) of the selection */
        const std::string& getSelectionIDString(const int playerID);
        // --------------------------------------------------------------------
        /** Returns the user-visible text of the selection */
        irr::core::stringw getSelectionText(const int playerID)
        {
            const int selection = m_selection[playerID];
            if (selection < 0 || selection >= int(m_children.size())) return "";
            return m_children[selection].m_text;
        }
        // --------------------------------------------------------------------

        /** Sets the ID of the selected item within the ribbon */
        void setSelection(const int i, const int playerID)
                             { m_selection[playerID] = i; updateSelection(); }
        
        /** Select an item in the ribbon by its internal name */
        void select(std::string item, const int playerID);
        
        /**
          * \brief This method can be used to rename an item.
          * Has no effect for ribbons without individual labels.
          *
          * \pre Must be called after the ribbon was add()ed
          * \param id The index of the item to rename, in range [0 .. item count - 1]
          */
        void setLabel(const unsigned int id, irr::core::stringw new_name);
        
        void setItemVisible(const unsigned int id, bool visible);
        
        void setFlip(RibbonFlip direction);

        /** Returns the ID of the item, or -1 if not found */
        int findItemNamed(const char* internalName);
        
        /** Returns the the widget, or NULL if not found */
        GUIEngine::Widget * findWidgetNamed(const char* interalName);

        /** \brief Dynamically (at runtime) add a text item to this ribbon
          * \pre This must be called before RibbonWidget::add, while the
          *      widget is not yet displayed
          * \pre only valid for ribbons that take text-only contents
          *       (e.g. tab bars)
          */
        void addTextChild(const core::stringw& text, const std::string &id);
        
        
        /** \brief Dynamically (at runtime) add an icon item to this ribbon.
         *  \pre this must be called before RibbonWidget::add, while the widget
         *       is not yet displayed
         *  \pre only valid for ribbons that take icon contents
         */
        void addIconChild(const core::stringw& text, const std::string &id,
                          const int w, const int h, const std::string &icon,
                          const IconButtonWidget::IconPathType iconPathType=
                                    IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    
        /**
          * \brief clear all children of this ribbon (likely because new ones will be added soon after)
          * \pre this must be called before RibbonWidget::add, while the widget is not yet displayed
          */
        void clearAllChildren();
        
        /**
         * \brief clear one child from this ribbon
         * \pre this must be called before RibbonWidget::add, while the widget is not yet displayed
         */
        void removeChildNamed(const char* name);
        
        PtrVector<Widget>& getRibbonChildren() { return m_children; }

        virtual EventPropagation onActivationInput(const int playerID) OVERRIDE;
    };

}

#endif
