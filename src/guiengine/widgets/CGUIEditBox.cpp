// Copyright (C) 2002-2015 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "guiengine/widgets/CGUIEditBox.hpp"

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IVideoDriver.h"
#include "rect.h"
//#include "os.h"
#include "Keycodes.h"

#include "config/user_config.hpp"
#include "font/font_manager.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"
#include "utils/utf8.h"

#include "../../../lib/irrlicht/include/IrrCompileConfig.h"
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceLinux.h"
#include <IrrlichtDevice.h>

#include <algorithm>
#include <cstdlib>

#ifdef ANDROID
#include <SDL_system.h>
extern bool Android_isHardwareKeyboardConnected();
extern void Android_toggleOnScreenKeyboard(bool show, int type, int y);
extern void Android_fromSTKEditBox(int widget_id, const core::stringw& text, int selection_start, int selection_end, int type);
#endif

#if !defined(SERVER_ONLY) && defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
#include "SDL.h"
#endif

/*
    todo:
    optional scrollbars
    ctrl+left/right to select word
    double click/ctrl click: word select + drag to select whole words, triple click to select line
    optional? dragging selected text
    numerical
*/

#ifndef SERVER_ONLY
#include <harfbuzz/hb.h>

// Copied from libraqm
namespace Grahem
{
typedef enum
{
  RAQM_GRAPHEM_CR,
  RAQM_GRAPHEM_LF,
  RAQM_GRAPHEM_CONTROL,
  RAQM_GRAPHEM_EXTEND,
  RAQM_GRAPHEM_REGIONAL_INDICATOR,
  RAQM_GRAPHEM_PREPEND,
  RAQM_GRAPHEM_SPACING_MARK,
  RAQM_GRAPHEM_HANGUL_SYLLABLE,
  RAQM_GRAPHEM_OTHER
} _raqm_grapheme_t;

static _raqm_grapheme_t
_raqm_get_grapheme_break (hb_codepoint_t ch,
                          hb_unicode_general_category_t category);

static bool
_raqm_allowed_grapheme_boundary (hb_codepoint_t l_char,
                                 hb_codepoint_t r_char)
{
  hb_unicode_general_category_t l_category;
  hb_unicode_general_category_t r_category;
  _raqm_grapheme_t l_grapheme, r_grapheme;
  hb_unicode_funcs_t* unicode_funcs = hb_unicode_funcs_get_default ();

  l_category = hb_unicode_general_category (unicode_funcs, l_char);
  r_category = hb_unicode_general_category (unicode_funcs, r_char);
  l_grapheme = _raqm_get_grapheme_break (l_char, l_category);
  r_grapheme = _raqm_get_grapheme_break (r_char, r_category);

  if (l_grapheme == RAQM_GRAPHEM_CR && r_grapheme == RAQM_GRAPHEM_LF)
    return false; /*Do not break between a CR and LF GB3*/
  if (l_grapheme == RAQM_GRAPHEM_CONTROL || l_grapheme == RAQM_GRAPHEM_CR ||
      l_grapheme == RAQM_GRAPHEM_LF || r_grapheme == RAQM_GRAPHEM_CONTROL ||
      r_grapheme == RAQM_GRAPHEM_CR || r_grapheme == RAQM_GRAPHEM_LF)
    return true; /*Break before and after CONTROL GB4, GB5*/
  if (r_grapheme == RAQM_GRAPHEM_HANGUL_SYLLABLE)
    return false; /*Do not break Hangul syllable sequences. GB6, GB7, GB8*/
  if (l_grapheme == RAQM_GRAPHEM_REGIONAL_INDICATOR &&
      r_grapheme == RAQM_GRAPHEM_REGIONAL_INDICATOR)
    return false; /*Do not break between regional indicator symbols. GB8a*/
  if (r_grapheme == RAQM_GRAPHEM_EXTEND)
    return false; /*Do not break before extending characters. GB9*/
  /*Do not break before SpacingMarks, or after Prepend characters.GB9a, GB9b*/
  if (l_grapheme == RAQM_GRAPHEM_PREPEND)
    return false;
  if (r_grapheme == RAQM_GRAPHEM_SPACING_MARK)
    return false;
  return true; /*Otherwise, break everywhere. GB1, GB2, GB10*/
}

static _raqm_grapheme_t
_raqm_get_grapheme_break (hb_codepoint_t ch,
                          hb_unicode_general_category_t category)
{
  _raqm_grapheme_t gb_type;

  gb_type = RAQM_GRAPHEM_OTHER;
  switch ((int)category)
  {
    case HB_UNICODE_GENERAL_CATEGORY_FORMAT:
      if (ch == 0x200C || ch == 0x200D)
        gb_type = RAQM_GRAPHEM_EXTEND;
      else
        gb_type = RAQM_GRAPHEM_CONTROL;
      break;

    case HB_UNICODE_GENERAL_CATEGORY_CONTROL:
      if (ch == 0x000D)
        gb_type = RAQM_GRAPHEM_CR;
      else if (ch == 0x000A)
        gb_type = RAQM_GRAPHEM_LF;
      else
        gb_type = RAQM_GRAPHEM_CONTROL;
      break;

    case HB_UNICODE_GENERAL_CATEGORY_SURROGATE:
    case HB_UNICODE_GENERAL_CATEGORY_LINE_SEPARATOR:
    case HB_UNICODE_GENERAL_CATEGORY_PARAGRAPH_SEPARATOR:
    case HB_UNICODE_GENERAL_CATEGORY_UNASSIGNED:
      if ((ch >= 0xFFF0 && ch <= 0xFFF8) ||
          (ch >= 0xE0000 && ch <= 0xE0FFF))
        gb_type = RAQM_GRAPHEM_CONTROL;
      break;

    case HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK:
    case HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK:
    case HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK:
      if (ch != 0x102B && ch != 0x102C && ch != 0x1038 &&
          (ch < 0x1062 || ch > 0x1064) && (ch < 0x1067 || ch > 0x106D) &&
          ch != 0x1083 && (ch < 0x1087 || ch > 0x108C) && ch != 0x108F &&
          (ch < 0x109A || ch > 0x109C) && ch != 0x1A61 && ch != 0x1A63 &&
          ch != 0x1A64 && ch != 0xAA7B && ch != 0xAA70 && ch != 0x11720 &&
          ch != 0x11721) /**/
        gb_type = RAQM_GRAPHEM_SPACING_MARK;

      else if (ch == 0x09BE || ch == 0x09D7 ||
          ch == 0x0B3E || ch == 0x0B57 || ch == 0x0BBE || ch == 0x0BD7 ||
          ch == 0x0CC2 || ch == 0x0CD5 || ch == 0x0CD6 ||
          ch == 0x0D3E || ch == 0x0D57 || ch == 0x0DCF || ch == 0x0DDF ||
          ch == 0x1D165 || (ch >= 0x1D16E && ch <= 0x1D172))
        gb_type = RAQM_GRAPHEM_EXTEND;
      break;

    case HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER:
      if (ch == 0x0E33 || ch == 0x0EB3)
        gb_type = RAQM_GRAPHEM_SPACING_MARK;
      break;

    case HB_UNICODE_GENERAL_CATEGORY_OTHER_SYMBOL:
      if (ch >= 0x1F1E6 && ch <= 0x1F1FF)
        gb_type = RAQM_GRAPHEM_REGIONAL_INDICATOR;
      break;

    default:
      gb_type = RAQM_GRAPHEM_OTHER;
      break;
  }

  return gb_type;
}

};
#endif

//! constructor
CGUIEditBox::CGUIEditBox(const wchar_t* text, bool border,
        IGUIEnvironment* environment, IGUIElement* parent, s32 id,
        const core::rect<s32>& rectangle)
    : IGUIEditBox(environment, parent, id, rectangle), MouseMarking(false),
    Border(border), OverrideColorEnabled(false), m_mark_begin(0), m_mark_end(0),
    OverrideColor(video::SColor(101,255,255,255)), OverrideFont(0), LastBreakFont(0),
    Operator(0), m_force_show_cursor_time(0), m_cursor_pos(0), m_scroll_pos(0), m_cursor_distance(0),
    m_max_chars(0), AutoScroll(true), PasswordBox(false),
    PasswordChar(U'*'), HAlign(EGUIA_UPPERLEFT), VAlign(EGUIA_CENTER),
    CurrentTextRect(0,0,1,1), FrameRect(rectangle)
{
    m_composing_start = 0;
    m_composing_end = 0;
    m_type = (GUIEngine::TextBoxType)0;

    #ifdef _DEBUG
    setDebugName("CGUIEditBox");
    #endif

    Text = text;
    m_edit_text = StringUtils::wideToUtf32(text);
    if (!m_edit_text.empty())
        updateGlyphLayouts();

#ifndef SERVER_ONLY
    if (Environment)
        Operator = Environment->getOSOperator();

    if (Operator)
        Operator->grab();
#endif
    // this element can be tabbed to
    setTabStop(true);
    setTabOrder(-1);

    IGUISkin *skin = 0;
    if (Environment)
        skin = Environment->getSkin();
    if (Border && skin)
    {
        FrameRect.UpperLeftCorner.X += skin->getSize(EGDS_TEXT_DISTANCE_X)+1;
        FrameRect.UpperLeftCorner.Y += skin->getSize(EGDS_TEXT_DISTANCE_Y)+1;
        FrameRect.LowerRightCorner.X -= skin->getSize(EGDS_TEXT_DISTANCE_X)+1;
        FrameRect.LowerRightCorner.Y -= skin->getSize(EGDS_TEXT_DISTANCE_Y)+1;
    }

    m_scroll_pos = m_cursor_distance = 0;
}


//! destructor
CGUIEditBox::~CGUIEditBox()
{
    if (OverrideFont)
        OverrideFont->drop();
#ifndef SERVER_ONLY
    if (Operator)
        Operator->drop();
#ifdef ANDROID
    if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard() &&
        GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
        Android_toggleOnScreenKeyboard(false, 0, 0);
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
    if (SDL_IsTextInputActive())
        SDL_StopTextInput();
#endif

#endif
}


//! Sets another skin independent font.
void CGUIEditBox::setOverrideFont(IGUIFont* font)
{
    if (OverrideFont == font)
        return;

    if (OverrideFont)
        OverrideFont->drop();

    OverrideFont = font;

    if (OverrideFont)
        OverrideFont->grab();

}


//! Sets another color for the text.
void CGUIEditBox::setOverrideColor(video::SColor color)
{
    OverrideColor = color;
    OverrideColorEnabled = true;
}


video::SColor CGUIEditBox::getOverrideColor() const
{
    return OverrideColor;
}


//! Turns the border on or off
void CGUIEditBox::setDrawBorder(bool border)
{
    Border = border;
}


//! Sets if the text should use the overide color or the color in the gui skin.
void CGUIEditBox::enableOverrideColor(bool enable)
{
    OverrideColorEnabled = enable;
}

bool CGUIEditBox::isOverrideColorEnabled() const
{
    _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
    return OverrideColorEnabled;
}


void CGUIEditBox::updateAbsolutePosition()
{
    IGUIElement::updateAbsolutePosition();
}


void CGUIEditBox::setPasswordBox(bool passwordBox, wchar_t passwordChar)
{
    PasswordBox = passwordBox;
    if (PasswordBox)
        PasswordChar = (char32_t)passwordChar;
    if (!m_edit_text.empty())
        updateGlyphLayouts();
}


bool CGUIEditBox::isPasswordBox() const
{
    _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
    return PasswordBox;
}


//! Sets text justification
void CGUIEditBox::setTextAlignment(EGUI_ALIGNMENT horizontal, EGUI_ALIGNMENT vertical)
{
    HAlign = horizontal;
    VAlign = vertical;
}


//! called if an event happened.
bool CGUIEditBox::OnEvent(const SEvent& event)
{
#ifndef SERVER_ONLY
    if (isEnabled())
    {
        switch(event.EventType)
        {
        case EET_SDL_TEXT_EVENT:
            if (event.SDLTextEvent.Type == SDL_TEXTINPUT)
            {
                m_composing_text.clear();
                m_composing_start = 0;
                m_composing_end = 0;
                std::u32string text = StringUtils::utf8ToUtf32(event.SDLTextEvent.Text);
                for (char32_t t : text)
                    inputChar(t);
            }
            else
            {
                m_composing_text = StringUtils::utf8ToUtf32(event.SDLTextEvent.Text);
                m_composing_start = m_cursor_pos;
                m_composing_end = m_cursor_pos + m_composing_text.size();
                // In linux these values don't seem to provide any more useful info
                // It's always 0, event.edit.text.size()
                //printf("Debug: %d, %d\n", event.SDLTextEvent.Start, event.SDLTextEvent.Length);
            }
            return true;
        case EET_GUI_EVENT:
            if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
            {
                if (event.GUIEvent.Caller == this)
                {
                    MouseMarking = false;
                    setTextMarkers(0,0);
                }
#if !defined(ANDROID) && defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
                if (SDL_IsTextInputActive())
                    SDL_StopTextInput();
#endif
#ifdef ANDROID
            // If using non touchscreen input in android dismiss text input
            // if out focus because it cannot use emoji keyboard at the same
            // time
            if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard() &&
                GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard() &&
                (Android_isHardwareKeyboardConnected() || SDL_IsAndroidTV()))
                Android_toggleOnScreenKeyboard(false, 0, 0);
#endif
                m_composing_start = 0;
                m_composing_end = 0;
                m_composing_text.clear();
            }
            else if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUSED)
            {
                // Required for correct screen keyboard position in the beginning
                FrameRect = AbsoluteRect;
                m_mark_begin = m_mark_end = m_cursor_pos = getTextCount();
#ifdef ANDROID
                calculateScrollPos();
                if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard() &&
                    GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
                {
                    // If user toggle with hacker keyboard with arrows, keep
                    // using only text from STKEditText
                    Android_fromSTKEditBox(getID(), Text, m_mark_begin, m_mark_end, m_type);
                    // Enable auto focus which allows hardware keyboard unicode characters
                    Android_toggleOnScreenKeyboard(true, m_type, CurrentTextRect.LowerRightCorner.Y + 5);
                }
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
#ifdef WIN32
                // In windows we use caret to determine candidate box, which
                // needs to calculate it first
                calculateScrollPos();
#endif
                SDL_StartTextInput();
#endif
#if !defined(ANDROID) && !defined(WIN32)
                calculateScrollPos();
#endif

                m_composing_text.clear();
            }
            break;
        case EET_KEY_INPUT_EVENT:
            if (processKey(event))
                return true;
            break;
        case EET_MOUSE_INPUT_EVENT:
            if (processMouse(event))
                return true;
            break;
        default:
            break;
        }
    }
#endif
    return IGUIElement::OnEvent(event);
}


void CGUIEditBox::correctCursor(s32& cursor_pos, bool left)
{
#ifndef SERVER_ONLY
    if (left)
    {
        if (cursor_pos >= (s32)m_edit_text.size())
            return;
        while (cursor_pos != 0 &&
            !Grahem::_raqm_allowed_grapheme_boundary(m_edit_text[cursor_pos - 1],
            m_edit_text[cursor_pos]))
            cursor_pos--;
    }
    else
    {
        while (cursor_pos != 0 && cursor_pos != (s32)m_edit_text.size() &&
            !Grahem::_raqm_allowed_grapheme_boundary(m_edit_text[cursor_pos - 1],
            m_edit_text[cursor_pos]))
            cursor_pos++;
    }
#endif
}


bool CGUIEditBox::processKey(const SEvent& event)
{
#ifdef SERVER_ONLY
    return false;
#else

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
    // Prevent double key events when the user is using IME
    if (!m_composing_text.empty())
        return false;
#endif

    if (!event.KeyInput.PressedDown)
        return false;

    bool text_changed = false;
    s32 new_mark_begin = m_mark_begin;
    s32 new_mark_end = m_mark_end;
    s32 new_cursor_pos = m_cursor_pos;

    // control shortcut handling
    if (event.KeyInput.Control)
    {
        // german backlash '\' entered with control + '?'
        if ( event.KeyInput.Char == '\\' )
        {
            inputChar(event.KeyInput.Char);
            return true;
        }

        switch(event.KeyInput.Key)
        {
        case IRR_KEY_A:
            // select all
            new_mark_begin = 0;
            new_mark_end = (s32)m_edit_text.size();
            new_cursor_pos = new_mark_end;
            break;
        case IRR_KEY_C:
            // copy to clipboard
            if (!PasswordBox && Operator && m_mark_begin != m_mark_end)
            {
                const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
                const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;

                std::u32string s = m_edit_text.substr(realmbgn, realmend - realmbgn);
                Operator->copyToClipboard(StringUtils::utf32ToUtf8(s).c_str());
            }
            break;
        case IRR_KEY_X:
            // cut to the clipboard
            if (!PasswordBox && Operator && m_mark_begin != m_mark_end)
            {
                const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
                const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;

                // copy
                std::u32string s = m_edit_text.substr(realmbgn, realmend - realmbgn);
                Operator->copyToClipboard(StringUtils::utf32ToUtf8(s).c_str());

                if (isEnabled())
                {
                    // delete
                    std::u32string sub_str = m_edit_text.substr(0, realmbgn);
                    sub_str += m_edit_text.substr(realmend, m_edit_text.size() - realmend);
                    m_edit_text = sub_str;

                    new_mark_begin = 0;
                    new_mark_end = 0;
                    new_cursor_pos = realmbgn;
                    text_changed = true;
                }
            }
            break;

        case IRR_KEY_V:
            if (!isEnabled())
                break;

            // paste from the clipboard
            if (Operator)
            {
                const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
                const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;

                // add new character
                char* clipboard_u8 = (char*)Operator->getTextFromClipboard();
                std::u32string clipboard;
                if (clipboard_u8)
                    clipboard = StringUtils::utf8ToUtf32(clipboard_u8);
                if (!clipboard.empty())
                {
                    if (m_mark_begin == m_mark_end)
                    {
                        // insert text
                        std::u32string sub_str = m_edit_text.substr(0, m_cursor_pos);
                        sub_str += clipboard;
                        sub_str += m_edit_text.substr(m_cursor_pos, m_edit_text.size() - m_cursor_pos);

                        if (m_max_chars == 0 || sub_str.size() <= m_max_chars) // thx to Fish FH for fix
                        {
                            m_edit_text = sub_str;
                            new_cursor_pos = m_cursor_pos + (s32)clipboard.size();
                        }
                    }
                    else
                    {
                        // replace text
                        std::u32string sub_str = m_edit_text.substr(0, realmbgn);
                        sub_str += clipboard;
                        sub_str += m_edit_text.substr(realmend, m_edit_text.size() - realmend);

                        if (m_max_chars == 0 || sub_str.size() <= m_max_chars) // thx to Fish FH for fix
                        {
                            m_edit_text = sub_str;
                            new_cursor_pos = realmbgn + (s32)sub_str.size();
                        }
                    }
                    new_mark_begin = 0;
                    new_mark_end = 0;
                    text_changed = true;
                }
            }
            break;
        case IRR_KEY_HOME:
            {
                // move/highlight to start of text
                if (event.KeyInput.Shift)
                {
                    new_mark_end = m_cursor_pos;
                    new_mark_begin = 0;
                    new_cursor_pos = 0;
                }
                else
                {
                    new_cursor_pos = 0;
                    new_mark_begin = 0;
                    new_mark_end = 0;
                }
            }
            break;
        case IRR_KEY_END:
            {
                // move/highlight to end of text
                if (event.KeyInput.Shift)
                {
                    new_mark_begin = m_cursor_pos;
                    new_mark_end = (s32)m_edit_text.size();
                    new_cursor_pos = 0;
                }
                else
                {
                    new_mark_begin = 0;
                    new_mark_end = 0;
                    new_cursor_pos = (s32)m_edit_text.size();
                }
            }
            break;
        default:
            return false;
        }
    }
    // default keyboard handling
    else
    switch(event.KeyInput.Key)
    {
            /*
        case IRR_KEY_Q:
            inputChar(L'\u05DC');
            text_changed = true;
            return true;
        case IRR_KEY_W:
            inputChar(L'\u05DB');
            text_changed = true;
            return true;
        case IRR_KEY_E:
            inputChar(L'\u05DA');
            text_changed = true;
            return true;
        case IRR_KEY_R:
            inputChar(L'\u05D9');
            text_changed = true;
            return true;
        case IRR_KEY_T:
            inputChar(L'\u05D8');
            text_changed = true;
            return true;
        case IRR_KEY_Y:
            inputChar(L'\u05D7');
            text_changed = true;
            return true;
            */

    case IRR_KEY_END:
        {
            s32 p = getTextCount();
            if (event.KeyInput.Shift)
            {
                if (m_mark_begin == m_mark_end)
                    new_mark_begin = m_cursor_pos;
                new_mark_end = p;
            }
            else
            {
                new_mark_begin = 0;
                new_mark_end = 0;
            }
            new_cursor_pos = p;
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
        }
        break;
    case IRR_KEY_HOME:
        {
            s32 p = 0;
            if (event.KeyInput.Shift)
            {
                if (m_mark_begin == m_mark_end)
                    new_mark_begin = m_cursor_pos;
                new_mark_end = p;
            }
            else
            {
                new_mark_begin = 0;
                new_mark_end = 0;
            }
            new_cursor_pos = p;
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
        }
        break;
    case IRR_KEY_RETURN:
        {
#ifdef ANDROID
            if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard() &&
                GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
                Android_toggleOnScreenKeyboard(false, 0, 0);
#endif
            sendGuiEvent( EGET_EDITBOX_ENTER );
        }
        break;
    case IRR_KEY_LEFT:
        {
            if (event.KeyInput.Shift)
            {
                if (m_cursor_pos > 0)
                {
                    if (m_mark_begin == m_mark_end)
                        new_mark_begin = m_cursor_pos;

                    new_mark_end = m_cursor_pos-1;
                    correctCursor(new_mark_end, true/*left*/);
                }
            }
            else
            {
                new_mark_begin = 0;
                new_mark_end = 0;
            }

            if (m_cursor_pos > 0)
            {
                new_cursor_pos = m_cursor_pos - 1;
                correctCursor(new_cursor_pos, true/*left*/);
            }
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
        }
        break;

    case IRR_KEY_RIGHT:
        {
            if (event.KeyInput.Shift)
            {
                if (m_edit_text.size() > (u32)m_cursor_pos)
                {
                    if (m_mark_begin == m_mark_end)
                        new_mark_begin = m_cursor_pos;

                    new_mark_end = m_cursor_pos + 1;
                    correctCursor(new_mark_end, false/*left*/);
                }
            }
            else
            {
                new_mark_begin = 0;
                new_mark_end = 0;
            }

            if (m_edit_text.size() > (u32)m_cursor_pos)
            {
                new_cursor_pos = m_cursor_pos + 1;
                correctCursor(new_cursor_pos, false/*left*/);
            }
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
        }
        break;
    case IRR_KEY_UP:
    case IRR_KEY_DOWN:
        {
            return false;
        }
        break;

    case IRR_KEY_BACK:
        if (!isEnabled())
            break;

        if (!m_edit_text.empty())
        {
            std::u32string sub_str;
            if (m_mark_begin != m_mark_end)
            {
                // delete marked text
                const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
                const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;

                std::u32string sub_str = m_edit_text.substr(0, realmbgn);
                sub_str += m_edit_text.substr(realmend, m_edit_text.size() - realmend);
                m_edit_text = sub_str;
                new_cursor_pos = realmbgn;
            }
            else
            {
                // delete text behind cursor
                if (m_cursor_pos > 0)
                    sub_str = m_edit_text.substr(0, m_cursor_pos - 1);
                else
                    sub_str.clear();
                sub_str += m_edit_text.substr(m_cursor_pos, m_edit_text.size() - m_cursor_pos);
                m_edit_text = sub_str;
                new_cursor_pos = m_cursor_pos - 1;
            }
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
            new_mark_begin = 0;
            new_mark_end = 0;
            text_changed = true;
        }
        break;
    case IRR_KEY_DELETE:
        if (!isEnabled())
            break;

        if (!m_edit_text.empty())
        {
            std::u32string sub_str;
            if (m_mark_begin != m_mark_end)
            {
                // delete marked text
                const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
                const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;

                sub_str = m_edit_text.substr(0, realmbgn);
                sub_str += m_edit_text.substr(realmend, m_edit_text.size() - realmend);
                m_edit_text = sub_str;
                new_cursor_pos = realmbgn;
                text_changed = true;
            }
            else if (m_cursor_pos != getTextCount())
            {
                // delete text before cursor
                sub_str = m_edit_text.substr(0, m_cursor_pos);
                sub_str += m_edit_text.substr(m_cursor_pos + 1, m_edit_text.size() - m_cursor_pos - 1);
                m_edit_text = sub_str;
                text_changed = true;
            }
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
            new_mark_begin = 0;
            new_mark_end = 0;
        }
        break;

    case IRR_KEY_ESCAPE:
    case IRR_KEY_TAB:
    case IRR_KEY_SHIFT:
    case IRR_KEY_F1:
    case IRR_KEY_F2:
    case IRR_KEY_F3:
    case IRR_KEY_F4:
    case IRR_KEY_F5:
    case IRR_KEY_F6:
    case IRR_KEY_F7:
    case IRR_KEY_F8:
    case IRR_KEY_F9:
    case IRR_KEY_F10:
    case IRR_KEY_F11:
    case IRR_KEY_F12:
    case IRR_KEY_F13:
    case IRR_KEY_F14:
    case IRR_KEY_F15:
    case IRR_KEY_F16:
    case IRR_KEY_F17:
    case IRR_KEY_F18:
    case IRR_KEY_F19:
    case IRR_KEY_F20:
    case IRR_KEY_F21:
    case IRR_KEY_F22:
    case IRR_KEY_F23:
    case IRR_KEY_F24:
        // ignore these keys
        return false;

    default:
        inputChar(event.KeyInput.Char);
        return true;
    }

    // Update glyph layouts, the next setTextMarks will update text to android
    if (text_changed)
    {
        updateGlyphLayouts();
    }

    // Set new text markers
    setTextMarkers(new_mark_begin, new_mark_end);

    if (new_cursor_pos > getTextCount())
        new_cursor_pos = getTextCount();
    m_cursor_pos = new_cursor_pos;
    if (m_cursor_pos < 0)
        m_cursor_pos = 0;

    calculateScrollPos();

    return true;
#endif
}


//! draws the element and its children
void CGUIEditBox::draw()
{
#ifndef SERVER_ONLY
    if (!IsVisible)
        return;

    if (Environment->hasFocus(this))
        updateSurrogatePairText();
    GUIEngine::ScreenKeyboard* screen_kbd = GUIEngine::ScreenKeyboard::getCurrent();
    bool has_screen_kbd = (screen_kbd && screen_kbd->getEditBox() == this);
    
    const bool focus = Environment->hasFocus(this) || has_screen_kbd;
    
    IGUISkin* skin = Environment->getSkin();
    if (!skin)
        return;

    FrameRect = AbsoluteRect;

    // draw the border
    if (Border)
    {
        EGUI_DEFAULT_COLOR col = EGDC_GRAY_EDITABLE;
        if (isEnabled())
            col = focus ? EGDC_FOCUSED_EDITABLE : EGDC_EDITABLE;
        skin->draw3DSunkenPane(this, skin->getColor(col),
            false, true, FrameRect, &AbsoluteClippingRect);

        FrameRect.UpperLeftCorner.X += skin->getSize(EGDS_TEXT_DISTANCE_X)+1;
        FrameRect.UpperLeftCorner.Y += skin->getSize(EGDS_TEXT_DISTANCE_Y)+1;
        FrameRect.LowerRightCorner.X -= skin->getSize(EGDS_TEXT_DISTANCE_X)+1;
        FrameRect.LowerRightCorner.Y -= skin->getSize(EGDS_TEXT_DISTANCE_Y)+1;
    }
    core::rect<s32> localClipRect = FrameRect;
    localClipRect.clipAgainst(AbsoluteClippingRect);

    IGUIFont* font = OverrideFont;
    if (!OverrideFont)
        font = skin->getFont();

    if (!font)
        return;

    setTextRect(0);
    // Save the override color information.
    // Then, alter it if the edit box is disabled.
    const bool prevOver = OverrideColorEnabled;
    const video::SColor prevColor = OverrideColor;

    if (!isEnabled() && !OverrideColorEnabled)
    {
        OverrideColorEnabled = true;
        OverrideColor = skin->getColor(EGDC_GRAY_TEXT);
    }

    const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
    const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;
    const s32 realcbgn = m_composing_start < m_composing_end ? m_composing_start : m_composing_end;
    const s32 realcend = m_composing_start < m_composing_end ? m_composing_end : m_composing_start;

    // draw the text layout
    if (!m_composing_text.empty())
    {
        std::vector<gui::GlyphLayout> ct;
        std::u32string total = m_edit_text;
        const s32 realcbgn = m_cursor_pos;
        const s32 realcend = m_cursor_pos + (s32)m_composing_text.size();
        total.insert(m_cursor_pos, m_composing_text);
        font_manager->shape(total, ct);
        gui::removeHighlightedURL(ct);
        for (unsigned i = 0; i < ct.size(); i++)
        {
            GlyphLayout& glyph = ct[i];
            auto& cluster = glyph.cluster;
            for (unsigned c = 0; c < glyph.cluster.size(); c++)
            {
                if (realcbgn != realcend)
                {
                    if (cluster[c] >= realcbgn && cluster[c] < realcend)
                        glyph.draw_flags.at(c) = GLD_COMPOSING;
                }
                else
                    glyph.draw_flags.at(c) = GLD_NONE;
            }
        }
        font->draw(ct, CurrentTextRect,
            OverrideColorEnabled ? OverrideColor : skin->getColor(EGDC_BUTTON_TEXT),
            false, true, &localClipRect);
    }
    else
    {
        for (unsigned i = 0; i < m_glyph_layouts.size(); i++)
        {
            GlyphLayout& glyph = m_glyph_layouts[i];
            auto& cluster = glyph.cluster;
            for (unsigned c = 0; c < glyph.cluster.size(); c++)
            {
                if (realmbgn != realmend)
                {
                    if (cluster[c] >= realmbgn && cluster[c] < realmend)
                        glyph.draw_flags.at(c) = GLD_MARKED;
                }
                else if (!PasswordBox && realcbgn != realcend)
                {
                    if (cluster[c] >= realcbgn && cluster[c] < realcend)
                        glyph.draw_flags.at(c) = GLD_COMPOSING;
                }
                else
                    glyph.draw_flags.at(c) = GLD_NONE;
            }
        }
        font->draw(m_glyph_layouts, CurrentTextRect,
            OverrideColorEnabled ? OverrideColor : skin->getColor(EGDC_BUTTON_TEXT),
            false, true, &localClipRect);
    }

    // Reset draw flags
    for (unsigned i = 0; i < m_glyph_layouts.size(); i++)
    {
        GlyphLayout& glyph = m_glyph_layouts[i];
        std::fill(glyph.draw_flags.begin(), glyph.draw_flags.end(), GLD_NONE);
    }

    // draw cursor
    uint64_t time_ms = StkTime::getMonoTimeMs();
    if (focus &&
        ((time_ms / 600) % 2 == 0 || m_force_show_cursor_time > time_ms))
    {
        core::rect< s32 > caret_rect = CurrentTextRect;
        caret_rect.UpperLeftCorner.X += m_cursor_distance - 1;
        caret_rect.LowerRightCorner.X = caret_rect.UpperLeftCorner.X + 2;
        GL32_draw2DRectangle(skin->getColor(EGDC_BUTTON_TEXT), caret_rect);
    }

    // Return the override color information to its previous settings.
    OverrideColorEnabled = prevOver;
    OverrideColor = prevColor;

    // draw children
    IGUIElement::draw();
#endif
}


//! Sets the new caption of this element.
void CGUIEditBox::setText(const core::stringw& text)
{
    m_edit_text = StringUtils::wideToUtf32(text);
    updateGlyphLayouts();
    m_mark_begin = m_mark_end = m_cursor_pos = getTextCount();
    m_scroll_pos = 0;
    calculateScrollPos();
#ifdef ANDROID
        if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard() &&
            GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
        {
            Android_fromSTKEditBox(getID(), Text, m_mark_begin, m_mark_end, m_type);
        }
#endif
}


//! Enables or disables automatic scrolling with cursor position
//! \param enable: If set to true, the text will move around with the cursor position
void CGUIEditBox::setAutoScroll(bool enable)
{
    AutoScroll = enable;
}


//! Checks to see if automatic scrolling is enabled
//! \return true if automatic scrolling is enabled, false if not
bool CGUIEditBox::isAutoScrollEnabled() const
{
    _IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
    return AutoScroll;
}


//! Gets the area of the text in the edit box
//! \return Returns the size in pixels of the text
core::dimension2du CGUIEditBox::getTextDimension()
{
    IGUISkin* skin = Environment->getSkin();
    if (!skin)
        return core::dimension2du(0, 0);

    IGUIFont* font = OverrideFont;
    if (!OverrideFont)
        font = skin->getFont();

    if (!font)
        return core::dimension2du(0, 0);

    return gui::getGlyphLayoutsDimension(m_glyph_layouts,
        font->getHeightPerLine(), font->getInverseShaping(), font->getScale());
}


//! Sets the maximum amount of characters which may be entered in the box.
//! \param max: Maximum amount of characters. If 0, the character amount is
//! infinity.
void CGUIEditBox::setMax(u32 max)
{
    m_max_chars = max;
    if (m_max_chars != 0 && m_edit_text.size() > m_max_chars)
        m_edit_text.substr(0, m_max_chars);
}


//! Returns maximum amount of characters, previously set by setMax();
u32 CGUIEditBox::getMax() const
{
    return m_max_chars;
}


bool CGUIEditBox::processMouse(const SEvent& event)
{
    switch(event.MouseInput.Event)
    {
    case irr::EMIE_LMOUSE_LEFT_UP:
        if (Environment->hasFocus(this))
        {
            m_cursor_pos = getCursorPos(event.MouseInput.X, event.MouseInput.Y);
            s32 old_cursor = m_cursor_pos;
            correctCursor(m_cursor_pos, old_cursor < m_mark_begin);
            correctCursor(m_mark_begin, old_cursor > m_mark_begin);
            if (MouseMarking)
                setTextMarkers(m_mark_begin, m_cursor_pos);

            MouseMarking = false;
            calculateScrollPos();
            return true;
        }
        else
        {
            MouseMarking = false;
        }
        break;
    case irr::EMIE_MOUSE_MOVED:
        {
            if (MouseMarking)
            {
                m_cursor_pos = getCursorPos(event.MouseInput.X, event.MouseInput.Y);
                correctCursor(m_cursor_pos, m_cursor_pos < m_mark_begin);
                setTextMarkers(m_mark_begin, m_cursor_pos);
                calculateScrollPos();
                return true;
            }
        }
        break;
    case EMIE_LMOUSE_PRESSED_DOWN:
        if (!Environment->hasFocus(this))
        {
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
            MouseMarking = true;
            m_cursor_pos = getCursorPos(event.MouseInput.X, event.MouseInput.Y);
            correctCursor(m_cursor_pos, m_cursor_pos < m_mark_begin);
            setTextMarkers(m_cursor_pos, m_cursor_pos);
            calculateScrollPos();

            return true;
        }
        else
        {
            if (!AbsoluteClippingRect.isPointInside(
                core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
            {
                return false;
            }
            
            if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard())
            {
#ifdef ANDROID
                if (GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
                    Android_toggleOnScreenKeyboard(true, m_type, CurrentTextRect.LowerRightCorner.Y + 5);
                else
#endif
                    openScreenKeyboard();
            }

            // move cursor
            m_cursor_pos = getCursorPos(event.MouseInput.X, event.MouseInput.Y);
            s32 new_mark_begin = m_mark_begin;
            if (!MouseMarking)
                new_mark_begin = m_cursor_pos;

            MouseMarking = true;
            setTextMarkers(new_mark_begin, m_cursor_pos);
            calculateScrollPos();

            return true;
        }
    default:
        break;
    }

    return false;
}


s32 CGUIEditBox::getCursorPos(s32 x, s32 y)
{
    IGUIFont* font = OverrideFont;
    IGUISkin* skin = Environment->getSkin();
    if (!skin)
        return 0;
    if (!OverrideFont)
        font = skin->getFont();

    x -= AbsoluteRect.UpperLeftCorner.X;
    x += m_scroll_pos;
    if (x < 0)
        x = 0;
    return getCurosrFromDimension((f32)x, (f32)y, m_glyph_layouts, font->getHeightPerLine(),
        font->getInverseShaping(), font->getScale());
}


void CGUIEditBox::setTextRect(s32 line)
{
    core::dimension2du d;

    IGUISkin* skin = Environment->getSkin();
    if (!skin)
        return;

    IGUIFont* font = OverrideFont ? OverrideFont : skin->getFont();

    if (!font)
        return;

    // get text dimension
    d = gui::getGlyphLayoutsDimension(m_glyph_layouts,
        font->getHeightPerLine(), font->getInverseShaping(), font->getScale());
    // justification
    switch (HAlign)
    {
    case EGUIA_CENTER:
        // align to h centre
        CurrentTextRect.UpperLeftCorner.X = (FrameRect.getWidth()/2) - (d.Width/2);
        CurrentTextRect.LowerRightCorner.X = (FrameRect.getWidth()/2) + (d.Width/2);
        break;
    case EGUIA_LOWERRIGHT:
        // align to right edge
        CurrentTextRect.UpperLeftCorner.X = FrameRect.getWidth() - d.Width;
        CurrentTextRect.LowerRightCorner.X = FrameRect.getWidth();
        break;
    default:
        // align to left edge
        CurrentTextRect.UpperLeftCorner.X = 0;
        CurrentTextRect.LowerRightCorner.X = d.Width;

    }

    switch (VAlign)
    {
    case EGUIA_CENTER:
        // align to v centre
        CurrentTextRect.UpperLeftCorner.Y =
            (FrameRect.getHeight()/2) - (d.Height)/2 + d.Height*line;
        break;
    case EGUIA_LOWERRIGHT:
        // align to bottom edge
        CurrentTextRect.UpperLeftCorner.Y =
            FrameRect.getHeight() - d.Height + d.Height*line;
        break;
    default:
        // align to top edge
        CurrentTextRect.UpperLeftCorner.Y = d.Height*line;
        break;
    }

    CurrentTextRect.UpperLeftCorner.X  -= m_scroll_pos;
    CurrentTextRect.LowerRightCorner.X -= m_scroll_pos;
    CurrentTextRect.LowerRightCorner.Y = CurrentTextRect.UpperLeftCorner.Y + d.Height;

    CurrentTextRect += FrameRect.UpperLeftCorner;

}


void CGUIEditBox::inputChar(char32_t c)
{
    if (!isEnabled())
        return;

    // Ignore unsupported characters
    if (c < 32)
        return;

    if (c < 65536)
    {
        wchar_t wc = c & 65535;
        if (utf8::internal::is_surrogate(wc) || !m_surrogate_chars.empty())
        {
            // Handle utf16 to 32 conversion together later, including any emoji
            // joint character (which is not surrogate)
            m_surrogate_chars.push_back(wc);
            return;
        }
    }

    if ((u32)getTextCount() < m_max_chars || m_max_chars == 0)
    {
        std::u32string sub_str;

        if (m_mark_begin != m_mark_end)
        {
            // replace marked text
            const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
            const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;

            sub_str = m_edit_text.substr(0, realmbgn);
            sub_str += c;
            sub_str += m_edit_text.substr(realmend, m_edit_text.size() - realmend);
            m_edit_text = sub_str;
            m_cursor_pos = realmbgn + 1;
        }
        else
        {
            // add new character
            sub_str = m_edit_text.substr(0, m_cursor_pos);
            sub_str += c;
            sub_str += m_edit_text.substr(m_cursor_pos, m_edit_text.size() - m_cursor_pos);
            m_edit_text = sub_str;
            m_cursor_pos++;
        }

        m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
        updateGlyphLayouts();
        setTextMarkers(0, 0);
        calculateScrollPos();
    }
}

void CGUIEditBox::updateCursorDistance()
{
    m_cursor_distance = 0;
    // Update cursor position
    IGUISkin* skin = Environment->getSkin();
    if (!skin)
        return;
    IGUIFont* font = OverrideFont ? OverrideFont : skin->getFont();
    if (!font)
        return;
    if (m_glyph_layouts.empty())
        return;
    if (m_cursor_pos != 0)
    {
        m_cursor_distance = getGlyphLayoutsDimension(m_glyph_layouts,
            font->getHeightPerLine(), font->getInverseShaping(),
            font->getScale(), m_cursor_pos - 1).Width;
    }
    else if ((m_glyph_layouts[0].flags & GLF_RTL_LINE) != 0)
    {
        // For rtl line the cursor in the begining is total width
        m_cursor_distance = getGlyphLayoutsDimension(m_glyph_layouts,
            font->getHeightPerLine(), font->getInverseShaping(),
            font->getScale()).Width;
    }
}

void CGUIEditBox::calculateScrollPos()
{
#ifndef SERVER_ONLY
    // Update cursor position
    updateCursorDistance();

    IGUISkin* skin = Environment->getSkin();
    if (!skin)
        return;
    IGUIFont* font = OverrideFont ? OverrideFont : skin->getFont();
    if (!font)
        return;

    if (!AutoScroll)
        return;
    setTextRect(0);

    s32 cStart = CurrentTextRect.UpperLeftCorner.X + m_scroll_pos +
        m_cursor_distance;
    // Reserver 2x font height at border to see the clipped text
    s32 cEnd = cStart + GUIEngine::getFontHeight() * 2;

    if (FrameRect.LowerRightCorner.X < cEnd)
        m_scroll_pos = cEnd - FrameRect.LowerRightCorner.X;
    else if (FrameRect.UpperLeftCorner.X > cStart)
        m_scroll_pos = cStart - FrameRect.UpperLeftCorner.X;
    else
        m_scroll_pos = 0;

    // todo: adjust scrollbar
    // calculate the position of input composition window
#if !defined(ANDROID) && defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
    SDL_Rect rect;
    rect.x = CurrentTextRect.UpperLeftCorner.X + m_cursor_distance - 1;
    rect.y = CurrentTextRect.UpperLeftCorner.Y;
    rect.w = 1;
    rect.h =
        CurrentTextRect.LowerRightCorner.Y - CurrentTextRect.UpperLeftCorner.Y;
    float inverse_scale_x = 1.0f / irr_driver->getDevice()->getNativeScaleX();
    float inverse_scale_y = 1.0f / irr_driver->getDevice()->getNativeScaleY();
    rect.x *= inverse_scale_x;
    rect.y *= inverse_scale_y;
    rect.w *= inverse_scale_x;
    rect.h *= inverse_scale_y;
    SDL_SetTextInputRect(&rect);
#endif

#endif   // SERVER_ONLY
}

//! set text markers
void CGUIEditBox::setTextMarkers(s32 begin, s32 end)
{
    if (GUIEngine::ScreenKeyboard::isActive())
    {
        m_mark_begin = m_mark_end = 0;
        return;
    }

    if (begin != m_mark_begin || end != m_mark_end)
    {
        m_mark_begin = begin;
        m_mark_end = end;
        sendGuiEvent(EGET_EDITBOX_MARKING_CHANGED);
#ifdef ANDROID
        if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard() &&
            GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
        {
            Android_fromSTKEditBox(getID(), Text, m_mark_begin, m_mark_end, m_type);
        }
#endif
    }
}

//! send some gui event to parent
void CGUIEditBox::sendGuiEvent(EGUI_EVENT_TYPE type)
{
    if ( Parent )
    {
        SEvent e;
        e.EventType = EET_GUI_EVENT;
        e.GUIEvent.Caller = this;
        e.GUIEvent.Element = 0;
        e.GUIEvent.EventType = type;

        Parent->OnEvent(e);
    }
}

//! Writes attributes of the element.
void CGUIEditBox::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
    // IGUIEditBox::serializeAttributes(out,options);

    out->addBool  ("OverrideColorEnabled",OverrideColorEnabled );
    out->addColor ("OverrideColor",       OverrideColor);
    // out->addFont("OverrideFont",OverrideFont);
    out->addInt   ("MaxChars",            m_max_chars);
    out->addBool  ("AutoScroll",          AutoScroll);
    out->addBool  ("PasswordBox",         PasswordBox);
    core::stringw ch = L" ";
    ch[0] = (wchar_t)PasswordChar;
    out->addString("PasswordChar",        ch.c_str());
    out->addEnum  ("HTextAlign",          HAlign, GUIAlignmentNames);
    out->addEnum  ("VTextAlign",          VAlign, GUIAlignmentNames);

    IGUIEditBox::serializeAttributes(out,options);
}


//! Reads attributes of the element
void CGUIEditBox::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
    IGUIEditBox::deserializeAttributes(in,options);

    setOverrideColor(in->getAttributeAsColor("OverrideColor"));
    enableOverrideColor(in->getAttributeAsBool("OverrideColorEnabled"));
    setMax(in->getAttributeAsInt("MaxChars"));
    setAutoScroll(in->getAttributeAsBool("AutoScroll"));
    core::stringw ch = in->getAttributeAsStringW("PasswordChar");

    if (!ch.size())
        setPasswordBox(in->getAttributeAsBool("PasswordBox"));
    else
        setPasswordBox(in->getAttributeAsBool("PasswordBox"), ch[0]);

    setTextAlignment( (EGUI_ALIGNMENT) in->getAttributeAsEnumeration("HTextAlign", GUIAlignmentNames),
            (EGUI_ALIGNMENT) in->getAttributeAsEnumeration("VTextAlign", GUIAlignmentNames));

    // setOverrideFont(in->getAttributeAsFont("OverrideFont"));
}

void CGUIEditBox::openScreenKeyboard()
{
    // If the device has native on screen keyboard, always use it
    if (GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard())
        return;

    if (GUIEngine::ScreenKeyboard::getCurrent() != NULL)
        return;

    GUIEngine::ScreenKeyboard* k = new GUIEngine::ScreenKeyboard(1.0f, 0.40f, this);
    k->init();
}

// Real copying is happening in text_box_widget.cpp with static function
void CGUIEditBox::fromAndroidEditText(const std::u32string& text, int start,
                                      int end, int composing_start,
                                      int composing_end)
{
    // Prevent invalid start or end
    if ((unsigned)end > text.size())
    {
        end = (int)text.size();
        start = end;
    }
    m_edit_text = text;
    updateGlyphLayouts();
    m_cursor_pos = end;
    m_composing_start = 0;
    m_composing_end = 0;

    m_mark_begin = start;
    m_mark_end = end;

    if (composing_start != composing_end)
    {
        if (composing_start < 0)
            composing_start = 0;
        if (composing_end > end)
            composing_end = end;
        m_composing_start = composing_start;
        m_composing_end = composing_end;
    }
    calculateScrollPos();
}

void CGUIEditBox::updateGlyphLayouts()
{
#ifndef SERVER_ONLY
    // Clear any unsupported characters
    m_edit_text.erase(std::remove(m_edit_text.begin(), m_edit_text.end(),
        U'\r'), m_edit_text.end());
    m_edit_text.erase(std::remove(m_edit_text.begin(), m_edit_text.end(),
        U'\t'), m_edit_text.end());
    m_edit_text.erase(std::remove(m_edit_text.begin(), m_edit_text.end(),
        U'\n'), m_edit_text.end());
    m_glyph_layouts.clear();
    if (PasswordBox)
    {
        font_manager->shape(std::u32string(m_edit_text.size(), PasswordChar),
            m_glyph_layouts);
    }
    else
    {
        font_manager->shape(m_edit_text, m_glyph_layouts);
        gui::removeHighlightedURL(m_glyph_layouts);
    }
    Text = StringUtils::utf32ToWide(m_edit_text);
#endif
}

void CGUIEditBox::updateSurrogatePairText()
{
    if (!m_surrogate_chars.empty())
    {
        wchar_t last_char = m_surrogate_chars.back();
        if (utf8::internal::is_trail_surrogate(last_char) ||
            !utf8::internal::is_surrogate(last_char))
        {
            const s32 realmbgn = m_mark_begin < m_mark_end ? m_mark_begin : m_mark_end;
            const s32 realmend = m_mark_begin < m_mark_end ? m_mark_end : m_mark_begin;
            m_surrogate_chars.push_back(0);
            std::u32string result = StringUtils::wideToUtf32(m_surrogate_chars.data());
            if (m_mark_begin == m_mark_end)
            {
                // insert text
                std::u32string sub_str = m_edit_text.substr(0, m_cursor_pos);
                sub_str += result;
                sub_str += m_edit_text.substr(m_cursor_pos, m_edit_text.size() - m_cursor_pos);

                if (m_max_chars == 0 || sub_str.size() <= m_max_chars) // thx to Fish FH for fix
                {
                    m_edit_text = sub_str;
                    m_cursor_pos = m_cursor_pos + (s32)result.size();
                }
            }
            else
            {
                // replace text
                std::u32string sub_str = m_edit_text.substr(0, realmbgn);
                sub_str += result;
                sub_str += m_edit_text.substr(realmend, m_edit_text.size() - realmend);

                if (m_max_chars == 0 || sub_str.size() <= m_max_chars) // thx to Fish FH for fix
                {
                    m_edit_text = sub_str;
                    m_cursor_pos = realmbgn + (s32)sub_str.size();
                }
            }
            m_force_show_cursor_time = StkTime::getMonoTimeMs() + 200;
            updateGlyphLayouts();
            setTextMarkers(0, 0);
            if (m_cursor_pos > getTextCount())
                m_cursor_pos = getTextCount();
            if (m_cursor_pos < 0)
                m_cursor_pos = 0;
            calculateScrollPos();
            m_surrogate_chars.clear();
        }
    }
}
