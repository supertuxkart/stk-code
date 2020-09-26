// Copyright (C) 2002-2015 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_EDIT_BOX_H_INCLUDED__
#define __C_GUI_EDIT_BOX_H_INCLUDED__

#include "IrrCompileConfig.h"
#include "IGUIEditBox.h"
#include "irrArray.h"
#include "IOSOperator.h"
#include "utils/leak_check.hpp"

#include "GlyphLayout.h"

#include <string>
#include <vector>

#if !defined(SERVER_ONLY) && defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
#include "SDL_events.h"
#endif

using namespace irr;
using namespace gui;

namespace GUIEngine
{
    enum TextBoxType: int;
}

    class CGUIEditBox : public IGUIEditBox
    {
    public:

        LEAK_CHECK()

        //! constructor
        CGUIEditBox(const wchar_t* text, bool border, IGUIEnvironment* environment,
            IGUIElement* parent, s32 id, const core::rect<s32>& rectangle);

        //! destructor
        virtual ~CGUIEditBox();

        //! Sets another skin independent font.
        virtual void setOverrideFont(IGUIFont* font=0);

        //! Sets another color for the text.
        virtual void setOverrideColor(video::SColor color);

        //! Gets the override color
        virtual video::SColor getOverrideColor() const;

        //! Sets if the text should use the overide color or the
        //! color in the gui skin.
        virtual void enableOverrideColor(bool enable);

        //! Checks if an override color is enabled
        /** \return true if the override color is enabled, false otherwise */
        virtual bool isOverrideColorEnabled(void) const;

        //! Turns the border on or off
        virtual void setDrawBorder(bool border);

        //! Enables or disables word wrap for using the edit box as multiline text editor.
        virtual void setWordWrap(bool enable) {}

        //! Checks if word wrap is enabled
        //! \return true if word wrap is enabled, false otherwise
        virtual bool isWordWrapEnabled() const { return false; }

        //! Enables or disables newlines.
        /** \param enable: If set to true, the EGET_EDITBOX_ENTER event will not be fired,
        instead a newline character will be inserted. */
        virtual void setMultiLine(bool enable) {}

        //! Checks if multi line editing is enabled
        //! \return true if mult-line is enabled, false otherwise
        virtual bool isMultiLineEnabled() const { return false; }

        //! Enables or disables automatic scrolling with cursor position
        //! \param enable: If set to true, the text will move around with the cursor position
        virtual void setAutoScroll(bool enable);

        //! Checks to see if automatic scrolling is enabled
        //! \return true if automatic scrolling is enabled, false if not
        virtual bool isAutoScrollEnabled() const;

        //! Gets the size area of the text in the edit box
        //! \return Returns the size in pixels of the text
        virtual core::dimension2du getTextDimension();

        //! Sets text justification
        virtual void setTextAlignment(EGUI_ALIGNMENT horizontal, EGUI_ALIGNMENT vertical);

        //! called if an event happened.
        virtual bool OnEvent(const SEvent& event);

        //! draws the element and its children
        virtual void draw();

        //! Sets the new caption of this element.
        virtual void setText(const core::stringw& text);

        //! Sets the maximum amount of characters which may be entered in the box.
        //! \param max: Maximum amount of characters. If 0, the character amount is
        //! infinity.
        virtual void setMax(u32 max);

        //! Returns maximum amount of characters, previously set by setMax();
        virtual u32 getMax() const;

        //! Sets whether the edit box is a password box. Setting this to true will
        /** disable MultiLine, WordWrap and the ability to copy with ctrl+c or ctrl+x
        \param passwordBox: true to enable password, false to disable
        \param passwordChar: the character that is displayed instead of letters */
        virtual void setPasswordBox(bool passwordBox, wchar_t passwordChar = L'*');

        //! Returns true if the edit box is currently a password box.
        virtual bool isPasswordBox() const;

        //! Updates the absolute position, splits text if required
        virtual void updateAbsolutePosition();

        //! Writes attributes of the element.
        virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

        //! Reads attributes of the element
        virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

        virtual irr::gui::IGUIFont* getOverrideFont() const { return NULL; }
        virtual irr::gui::IGUIFont* getActiveFont() const { return NULL; }
        virtual void setDrawBackground(bool) { }

        void fromAndroidEditText(const std::u32string& text, int start, int end,
                                 int composing_start, int composing_end);
        void openScreenKeyboard();
        s32 getCursorPosInBox() const { return m_cursor_pos; }
        s32 getTextCount() const { return (s32)m_edit_text.size(); }
        void setTextBoxType(GUIEngine::TextBoxType t) { m_type = t; }
        virtual void setComposingText(const std::u32string& ct) { m_composing_text = ct; }
        virtual void clearComposingText() { m_composing_text.clear(); }
        virtual const core::position2di& getICPos() const { return m_ic_pos; }
        void sendGuiEvent(EGUI_EVENT_TYPE type);
    protected:
        //! sets the area of the given line
        void setTextRect(s32 line);
        //! adds a letter to the edit box
        virtual void inputChar(char32_t c);
        //! calculates the current scroll position
        void calculateScrollPos();
        //! send some gui event to parent
        //! set text markers
        void setTextMarkers(s32 begin, s32 end);
        void updateCursorDistance();
        bool processKey(const SEvent& event);
        bool processMouse(const SEvent& event);
        s32 getCursorPos(s32 x, s32 y);
        void updateGlyphLayouts();

        bool MouseMarking;
        bool Border;
        bool OverrideColorEnabled;
        s32 m_mark_begin;
        s32 m_mark_end;

        GUIEngine::TextBoxType m_type;

        video::SColor OverrideColor;
        gui::IGUIFont *OverrideFont, *LastBreakFont;
        IOSOperator* Operator;

        uint64_t m_force_show_cursor_time;
        s32 m_cursor_pos;
        s32 m_scroll_pos;
        s32 m_cursor_distance;
        u32 m_max_chars;

        bool AutoScroll, PasswordBox;
        char32_t PasswordChar;
        EGUI_ALIGNMENT HAlign, VAlign;

        core::rect<s32> CurrentTextRect, FrameRect; // temporary values

        s32 m_composing_start;
        s32 m_composing_end;

        /* UTF32 string for shaping and editing to avoid wchar_t issue in
         * windows */
        std::u32string m_edit_text;
        /* Used in windows api for native composing text handling. */
        std::u32string m_composing_text;
        std::vector<GlyphLayout> m_glyph_layouts;
        // Pre-edit surrogate chars
        std::vector<wchar_t> m_surrogate_chars;
        // Position to show composition box in native window (linux / windows)
        core::position2di m_ic_pos;
        void correctCursor(s32& cursor_pos, bool left);
        void updateSurrogatePairText();
    };



#endif // __C_GUI_EDIT_BOX_H_INCLUDED__

