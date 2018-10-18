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



#ifndef HEADER_RIBBONGRID_HPP
#define HEADER_RIBBONGRID_HPP

#include <irrString.h>

#include <algorithm>

#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    class IconButtonWidget;

    /**
     * Even if you have a ribbon that only acts on click/enter, you may wish to know which
     * item is currently highlighted. In this case, create a listener and pass it to the ribbon.
     */
    class DynamicRibbonHoverListener
    {
    public:
        virtual ~DynamicRibbonHoverListener() {}
        virtual void onSelectionChanged(DynamicRibbonWidget* theWidget,
                                       const std::string& selectionID,
                                       const irr::core::stringw& selectionText,
                                       const int playerID) = 0;
    };

    /** The description of an item added to a DynamicRibbonWidget */
    struct ItemDescription
    {
        irr::core::stringw m_user_name;
        std::string m_code_name;
        std::string m_sshot_file;
        IconButtonWidget::IconPathType m_image_path_type;

        bool m_animated;
        /** used instead of 'm_sshot_file' if m_animated is true */
        std::vector<std::string> m_all_images;
        float m_curr_time;
        float m_time_per_frame;

        unsigned int m_badges;
    };

    /**
      * \brief An extended version of RibbonWidget, with more capabilities.
      * A dynamic ribbon builds upon RibbonWidget, adding dynamic contents creation and sizing,
      * scrolling, multiple-row layouts.
      * \ingroup widgetsgroup
      * \note items you add to a list are kept after the the ribbon was in is removed
      *       (i.e. you don't need to add items everytime the screen is shown, only upon loading)
      */
    class DynamicRibbonWidget : public Widget, public RibbonWidget::IRibbonListener
    {
        friend class RibbonWidget;

        /** A list of all listeners that registered to be notified on hover/selection */
        PtrVector<DynamicRibbonHoverListener> m_hover_listeners;

        virtual ~DynamicRibbonWidget();

        /** Used for ribbon grids that have a label at the bottom */
        bool m_has_label;
        irr::gui::IGUIStaticText* m_label;

        /** Height of ONE label text line (if label is multiline only one line is measured here).
          * If there is no label, will be 0.
          */
        int m_label_height;

        /** Whether this ribbon contains at least one animated item */
        bool m_animated_contents;

        /** Whether there are more items than can fit in a single screen; arrows will then appear
          * on each side of the ribbon to scroll the contents
          */
        bool m_scrolling_enabled;

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

        /** Whether this ribbon can have multiple rows (i.e. ribbon grid) or is a single line */
        bool m_multi_row;

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
        IconButtonWidget* m_left_widget;
        IconButtonWidget* m_right_widget;

        /** Returns the currently selected row */
        RibbonWidget* getSelectedRibbon(const int playerID);

        /** Returns the row */
        RibbonWidget* getRowContaining(Widget* w);

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
        void buildInternalStructure();

        /** Call this to scroll within a scrollable ribbon */
        void scroll(int x_delta, bool evenIfDeactivated = false);

        /** Used  for combo ribbons, to contain the ID of the currently selected item for each player */
        int m_selected_item[MAX_PLAYER_COUNT];

        /** Callbacks */
        virtual void add();
        virtual EventPropagation mouseHovered(Widget* child, const int playerID);
        virtual EventPropagation transmitEvent(Widget* w, const std::string& originator, const int playerID);

        bool findItemInRows(const char* name, int* p_row, int* p_id);

        int m_item_count_hint;

        float getFontScale(int icon_width) const;
        void setLabelSize(const irr::core::stringw& text);
        irr::core::stringw getUserName(const irr::core::stringw& user_name) const;

        /**
         * Font used to write the labels, can be scaled down depending on the
         * length of the text
         */
        irr::gui::ScalableFont* m_font;

        /** Max width of a label, in pixels */
        int m_max_label_width;

        /** Max length of a label, in characters */
        unsigned int m_max_label_length;

    public:

        LEAK_CHECK()

        /**
          * \param combo     Whether this is a "combo" ribbon, i.e. whether there is always one selected item.
          *                  If set to false, will behave more like a toolbar.
          * \param multi_row Whether this ribbon can have more than one row
          */
        DynamicRibbonWidget(const bool combo, const bool multi_row);

        /** Reference pointers only, the actual instances are owned by m_children. Used to create mtultiple-row
         ribbons (what appears to be a grid of icons is actually a vector of stacked basic ribbons) */
        PtrVector<RibbonWidget, REF> m_rows;

        /** Dynamically add an item to the ribbon's list of items (will not be visible until you
          * call 'updateItemDisplay' or 'add').
          *
          * \param user_name       The name that will shown to the user (may be translated)
          * \param code_name       The non-translated internal name used to uniquely identify this item.
          * \param image_file      A path to a texture that will the icon of this item (path relative to data dir, just like PROP_ICON)
          * \param badge           Whether to add badges to this item (bitmask, see possible values in widget.hpp)
          * \param image_path_type How to interpret the path to the image (absolute or relative)
          */
        void addItem( const irr::core::stringw& user_name, const std::string& code_name,
                      const std::string& image_file, const unsigned int badge=0,
                      IconButtonWidget::IconPathType image_path_type=IconButtonWidget::ICON_PATH_TYPE_RELATIVE);

        /** Dynamically add an animated item to the ribbon's list of items (will not be visible until you
         * call 'updateItemDisplay' or 'add'). Animated means it has many images that will be shown in
         * a slideshown fashion.
         *
         * \param user_name       The name that will shown to the user (may be translated)
         * \param code_name       The non-translated internal name used to uniquely identify this item.
         * \param image_files     A path to a texture that will the icon of this item (path relative to data dir, just like PROP_ICON)
         * \param time_per_frame  Time (in seconds) to spend at each image.
         * \param badge           Whether to add badges to this item (bitmask, see possible values in widget.hpp)
         * \param image_path_type How to interpret the path to the image (absolute or relative)
         */
        void addAnimatedItem( const irr::core::stringw& user_name, const std::string& code_name,
                             const std::vector<std::string>& image_files, const float time_per_frame,
                             const unsigned int badge=0,
                             IconButtonWidget::IconPathType image_path_type=IconButtonWidget::ICON_PATH_TYPE_RELATIVE);

        /** Clears all items added through 'addItem'. You can then add new items with 'addItem' and call
            'updateItemDisplay' to update the display. */
        void clearItems();

        /** Sort the list of items with a given comparator. */
        template<typename Compare>
        void sortItems(Compare comp)
        {
            std::sort(m_items.begin(), m_items.end(), comp);
        }

        /**
          * \brief Register a listener to be notified of selection changes within the ribbon.
          * \note  The ribbon takes ownership of this listener and will delete it.
          * \note  The listener will be deleted upon leaving the screen, so you will likely
          *        want to add a listener in the "init" callback of your screen.
          */
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
        irr::core::stringw getSelectionText(const int playerID);

        /** Returns a read-only list of items added to this ribbon */
        const std::vector<ItemDescription>& getItems() const { return m_items; }

        /**
          * \brief Select an item from its numerical ID. Only for [1-row] combo ribbons.
          *
          * \param  item_id In range [0 .. number of items added through 'addItem' - 1]
          * \return Whether setting the selection was successful (whether the item exists)
          */
        bool setSelection(int item_id, const int playerID, const bool focusIt, bool evenIfDeactivated=false);

        /**
          * \brief Select an item from its codename.
          *
          * \return Whether setting the selection was successful (whether the item exists)
          */
        bool setSelection(const std::string &item_codename,
                          const int playerID, const bool focusIt,
                          bool evenIfDeactivated=false);

        /** \brief Callback from parent class Widget. */
        virtual void elementRemoved();

        /** \brief callback from IRibbonListener */
        virtual void onRibbonWidgetScroll(const int delta_x);

        /** \brief callback from IRibbonListener */
        virtual void onRibbonWidgetFocus(RibbonWidget* emitter, const int playerID);

        /** \brief callback from IRibbonListener */
        virtual void onSelectionChange(){}

        virtual void setText(const wchar_t *text);

        virtual void update(float delta);

        /** Set approximately how many items are expected to be in this ribbon; will help the layout
          * algorithm next time add() is called */
        void setItemCountHint(int hint) { m_item_count_hint = hint; }

        /** Set max length of displayed text. */
        void setMaxLabelLength(int length) { m_max_label_length = length; }
    };

}

#endif
