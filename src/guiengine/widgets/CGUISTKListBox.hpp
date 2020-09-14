// Copyright (C) 2002-2015 Nikolaus Gebhardt
//               2013 Glenn De Jonghe
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef HEADER_CGUISTKListBox_HPP
#define HEADER_CGUISTKListBox_HPP

#include "IrrCompileConfig.h"

#include "IGUIListBox.h"
#include "IGUIElement.h"
#include "irrArray.h"
#include <string>
#include <vector>

#include "GlyphLayout.h"

namespace irr
{
    namespace gui
    {
    class IGUIFont;
    class IGUIScrollBar;

    class CGUISTKListBox : public IGUIElement
    {
    public:

            struct ListItem
            {

                struct ListCell
                {
                    irr::core::stringw m_text;
                    int m_proportion;
                    s32 m_icon;
                    std::vector<GlyphLayout> m_glyph_layouts;
                    bool m_center;

                    ListCell(irr::core::stringw text, s32 icon = -1, int proportion = 1, bool center = false)
                    {
                        m_text = text;
                        m_proportion = proportion;
                        m_icon = icon;
                        m_center = center;
                    }
                };

                core::array< ListCell > m_contents;

                // Actually only used in list_widget -- still refactoring FIXME
                std::string m_internal_name;
                int m_current_id;

                bool m_word_wrap = false;
                float m_line_height_scale = 0.0f;

                // A multicolor extension
                struct ListItemOverrideColor
                {
                        ListItemOverrideColor() : Use(false) {}
                        bool Use;
                        video::SColor Color;
                };

                ListItemOverrideColor OverrideColors[EGUI_LBC_COUNT];
            };

            //! constructor
            CGUISTKListBox(IGUIEnvironment* environment, IGUIElement* parent,
                    s32 id, core::rect<s32> rectangle, bool clip=true,
                    bool drawBack=false, bool moveOverSelect=false);

            //! destructor
            virtual ~CGUISTKListBox();

            //! returns amount of list items
            virtual u32 getItemCount() const;

            virtual const wchar_t* getCellText(u32 row_num, u32 col_num) const;

            virtual ListItem getItem(u32 id) const;

            //! clears the list
            virtual void clear();

            //! returns id of selected item. returns -1 if no item is selected.
            virtual s32 getSelected() const;

            //! sets the selected item. Set this to -1 if no item should be selected
            virtual void setSelected(s32 id);

            virtual s32 getRowByCellText(const wchar_t * text);

            //! sets the selected item. Set this to -1 if no item should be selected
            virtual void setSelectedByCellText(const wchar_t * text);

            virtual s32 getRowByInternalName(const std::string & text) const;

            //! called if an event happened.
            virtual bool OnEvent(const SEvent& event);

            //! draws the element and its children
            virtual void draw();

            //! adds an list item with an icon
            //! \param text Text of list entry
            //! \param icon Sprite index of the Icon within the current sprite bank. Set it to -1 if you want no icon
            //! \return
            //! returns the id of the new created item
            //virtual u32 addItem(const wchar_t* text, s32 icon);

            virtual u32 addItem(const ListItem & item);

            //! Returns the icon of an item
            virtual s32 getIcon(u32 row_num, u32 col_num) const;

            //! removes an item from the list
            virtual void removeItem(u32 id);

            //! get the the id of the item at the given absolute coordinates
            virtual s32 getItemAt(s32 xpos, s32 ypos) const;

            //! Sets the sprite bank which should be used to draw list icons. This font is set to the sprite bank of
            //! the built-in-font by default. A sprite can be displayed in front of every list item.
            //! An icon is an index within the icon sprite bank. Several default icons are available in the
            //! skin through getIcon
            virtual void setSpriteBank(IGUISpriteBank* bank);

            //! set whether the listbox should scroll to newly selected items
            virtual void setAutoScrollEnabled(bool scroll);

            //! returns true if automatic scrolling is enabled, false if not.
            virtual bool isAutoScrollEnabled() const;

            //! Update the position and size of the listbox, and update the scrollbar
            virtual void updateAbsolutePosition();

            //! set all item colors at given index to color
            virtual void setItemOverrideColor(u32 index, video::SColor color);

            //! set all item colors of specified type at given index to color
            virtual void setItemOverrideColor(u32 index, EGUI_LISTBOX_COLOR colorType, video::SColor color);

            //! clear all item colors at index
            virtual void clearItemOverrideColor(u32 index);

            //! clear item color at index for given colortype
            virtual void clearItemOverrideColor(u32 index, EGUI_LISTBOX_COLOR colorType);

            //! has the item at index its color overwritten?
            virtual bool hasItemOverrideColor(u32 index, EGUI_LISTBOX_COLOR colorType) const;

            //! return the overwrite color at given item index.
            virtual video::SColor getItemOverrideColor(u32 index, EGUI_LISTBOX_COLOR colorType) const;

            //! return the default color which is used for the given colorType
            virtual video::SColor getItemDefaultColor(EGUI_LISTBOX_COLOR colorType) const;

            //! set the item at the given index
            virtual void setCell(u32 row_num, u32 col_num, const wchar_t* text, s32 icon);

            //! Swap the items at the given indices
            virtual void swapItems(u32 index1, u32 index2);

            //! set global itemHeight
            virtual void setItemHeight( s32 height );

            //! Sets whether to draw the background
            virtual void setDrawBackground(bool draw);

            void setAlternatingDarkness(bool val) { m_alternating_darkness = val; }
            gui::IGUIScrollBar* getScrollBar() const { return ScrollBar; }
            void setDisactivated(bool val)
            {
                m_deactivated = val;
                if (m_deactivated)
                    Selected = -1;
            }
    private:

            void recalculateItemHeight();
            void selectNew(s32 ypos, bool onlyHover=false);
            void recalculateScrollPos();

            // extracted that function to avoid copy&paste code
            void recalculateIconWidth();

            core::array< ListItem > Items;
            s32 Selected;
            s32 ItemHeight;
            s32 ItemHeightOverride;
            s32 TotalItemHeight;
            s32 ItemsIconWidth;
            s32 MousePosY;
            gui::IGUIFont* Font;
            gui::IGUISpriteBank* IconBank;
            gui::IGUIScrollBar* ScrollBar;
            u32 selectTime;
            core::stringw KeyBuffer;
            bool Selecting;
            bool Moving;
            bool DrawBack;
            bool MoveOverSelect;
            bool AutoScroll;
            bool HighlightWhenNotFocused;
            bool m_alternating_darkness;
            bool m_deactivated;
    };


    } // end namespace gui
} // end namespace irr

#endif
