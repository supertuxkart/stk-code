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

#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "guiengine/widgets/CGUIEditBox.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/translation.hpp"
#include "utils/utf8/core.h"
#include "utils/utf8.h"

#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIButton.h>
#include <IrrlichtDevice.h>

using namespace irr;

class MyCGUIEditBox : public CGUIEditBox
{
    PtrVector<GUIEngine::ITextBoxWidgetListener, REF> m_listeners;

public:

    MyCGUIEditBox(const wchar_t* text, bool border, gui::IGUIEnvironment* environment,
                 gui:: IGUIElement* parent, s32 id, const core::rect<s32>& rectangle) :
        CGUIEditBox(text, border, environment, parent, id, rectangle)
    {
    }

    void addListener(GUIEngine::ITextBoxWidgetListener* listener)
    {
        m_listeners.push_back(listener);
    }

    void clearListeners()
    {
        m_listeners.clearWithoutDeleting();
    }

    void handleTextUpdated()
    {
        for (unsigned n = 0; n < m_listeners.size(); n++)
            m_listeners[n].onTextUpdated();
    }

    bool handleEnterPressed()
    {
        bool handled = false;
        for (unsigned n = 0; n < m_listeners.size(); n++)
        {
            if (m_listeners[n].onEnterPressed(Text))
            {
                handled = true;
                setText(L"");
            }
        }
        return handled;
    }

    virtual bool OnEvent(const SEvent& event)
    {
        bool out = CGUIEditBox::OnEvent(event);

        if (event.EventType == EET_KEY_INPUT_EVENT &&
            event.KeyInput.PressedDown)
            handleTextUpdated();

        if (event.EventType == EET_KEY_INPUT_EVENT &&
            event.KeyInput.Key == IRR_KEY_RETURN)
            handleEnterPressed();

        return out;
    }
};

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace irr;

// -----------------------------------------------------------------------------

TextBoxWidget::TextBoxWidget() : Widget(WTYPE_TEXTBOX)
{
}

// -----------------------------------------------------------------------------

void TextBoxWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);

    // Don't call TextBoxWidget::getText(), which assumes that the irrlicht
    // widget already exists.
    const stringw& text = Widget::getText();

    m_element = new MyCGUIEditBox(text.c_str(), true /* border */, GUIEngine::getGUIEnv(),
                                  (m_parent ? m_parent : GUIEngine::getGUIEnv()->getRootGUIElement()),
                                  getNewID(), widget_size);

    if (m_deactivated)
        m_element->setEnabled(false);

    //m_element = GUIEngine::getGUIEnv()->addEditBox(text.c_str(), widget_size,
    //                                               true /* border */, m_parent, getNewID());
    m_id = m_element->getID();
    m_element->setTabOrder(m_id);
    m_element->setTabGroup(false);
    m_element->setTabStop(true);
}

// -----------------------------------------------------------------------------

void TextBoxWidget::addListener(ITextBoxWidgetListener* listener)
{
    ((MyCGUIEditBox*)m_element)->addListener(listener);
}

// -----------------------------------------------------------------------------

void TextBoxWidget::clearListeners()
{
    ((MyCGUIEditBox*)m_element)->clearListeners();
}

// -----------------------------------------------------------------------------

stringw TextBoxWidget::getText() const
{
    const IGUIEditBox* textCtrl =  Widget::getIrrlichtElement<IGUIEditBox>();
    assert(textCtrl != NULL);

    return stringw(textCtrl->getText());
}

// -----------------------------------------------------------------------------

void TextBoxWidget::setPasswordBox(bool passwordBox, wchar_t passwordChar)
{
    IGUIEditBox* textCtrl =  Widget::getIrrlichtElement<IGUIEditBox>();
    assert(textCtrl != NULL);
    textCtrl->setPasswordBox(passwordBox, passwordChar);
    setTextBoxType(TBT_PASSWORD);
}

// -----------------------------------------------------------------------------
void TextBoxWidget::setTextBoxType(TextBoxType t)
{
    ((MyCGUIEditBox*)m_element)->setTextBoxType(t);
}

// -----------------------------------------------------------------------------

EventPropagation TextBoxWidget::focused(const int playerID)
{
    assert(playerID == 0); // No support for multiple players in text areas!

    // special case : to work, the text box must receive "irrLicht focus", STK focus is not enough
    GUIEngine::getGUIEnv()->setFocus(m_element);
    setWithinATextBox(true);
    return EVENT_LET;
}

// -----------------------------------------------------------------------------

void TextBoxWidget::unfocused(const int playerID, Widget* new_focus)
{
    assert(playerID == 0); // No support for multiple players in text areas!

    setWithinATextBox(false);
    
    GUIEngine::getGUIEnv()->removeFocus(m_element);
}

// -----------------------------------------------------------------------------

void TextBoxWidget::elementRemoved()
{
    // normally at this point normal widgets have been deleted by irrlicht already.
    // but this is a custom widget and the gui env does not appear to want to
    // manage it. so we free it manually
    m_element->drop();
    Widget::elementRemoved();
}

// -----------------------------------------------------------------------------

void TextBoxWidget::setActive(bool active)
{
    Widget::setActive(active);

    if (m_element != NULL)
    {
        IGUIEditBox* textCtrl = Widget::getIrrlichtElement<IGUIEditBox>();
        assert(textCtrl != NULL);
        textCtrl->setEnabled(active);
    }
}   // setActive

// -----------------------------------------------------------------------------

EventPropagation TextBoxWidget::onActivationInput(const int playerID)
{
    if (GUIEngine::ScreenKeyboard::shouldUseScreenKeyboard())
    {
        ((MyCGUIEditBox*)m_element)->openScreenKeyboard();
    }

    // The onWidgetActivated() wasn't used at all before, so always block
    // event to avoid breaking something
    return EVENT_BLOCK;
}

// -----------------------------------------------------------------------------
EventPropagation TextBoxWidget::rightPressed(const int playerID)
{
    if (((MyCGUIEditBox*)m_element)->getTextCount() ==
        ((MyCGUIEditBox*)m_element)->getCursorPosInBox())
        return EVENT_BLOCK;

    return EVENT_LET;
}   // rightPressed

// -----------------------------------------------------------------------------
EventPropagation TextBoxWidget::leftPressed (const int playerID)
{
    if (((MyCGUIEditBox*)m_element)->getCursorPosInBox() == 0)
        return EVENT_BLOCK;

    return EVENT_LET;
}   // leftPressed

// ============================================================================
/* This callback will allow copying android edittext data directly to editbox,
 * which will allow composing text to be auto updated. */
#ifdef ANDROID
#include "jni.h"

#if !defined(ANDROID_PACKAGE_CALLBACK_NAME)
    #error
#endif

#define MAKE_EDITTEXT_CALLBACK(x) JNIEXPORT void JNICALL Java_ ## x##_STKEditText_editText2STKEditbox(JNIEnv* env, jobject this_obj, jint widget_id, jstring text, jint start, jint end, jint composing_start, jint composing_end)
#define ANDROID_EDITTEXT_CALLBACK(PKG_NAME) MAKE_EDITTEXT_CALLBACK(PKG_NAME)

extern "C"
ANDROID_EDITTEXT_CALLBACK(ANDROID_PACKAGE_CALLBACK_NAME)
{
    if (text == NULL)
        return;

    const uint16_t* utf16_text = (const uint16_t*)env->GetStringChars(text, NULL);
    if (utf16_text == NULL)
        return;
    const size_t len = env->GetStringLength(text);
    // Android use 32bit wchar_t and java use utf16 string
    // We should not use the modified utf8 from java as it fails for emoji
    // because it's larger than 16bit
    std::u32string to_editbox;

    std::vector<int> mappings;
    int pos = 0;
    mappings.push_back(pos++);
    for (unsigned i = 0; i < len; i++)
    {
        if (utf8::internal::is_lead_surrogate(utf16_text[i]))
        {
            int duplicated_pos = pos++;
            mappings.push_back(duplicated_pos);
            mappings.push_back(duplicated_pos);
            i++;
        }
        else
            mappings.push_back(pos++);
    }

    // Correct start / end position for utf16
    if (start < (int)mappings.size())
        start = mappings[start];
    if (end < (int)mappings.size())
        end = mappings[end];
    if (composing_start < (int)mappings.size())
        composing_start = mappings[composing_start];
    if (composing_end < (int)mappings.size())
        composing_end = mappings[composing_end];

    try
    {
        utf8::utf16to32(utf16_text, utf16_text + len,
            back_inserter(to_editbox));
    }
    catch (std::exception& e)
    {
        (void)e;
    }
    env->ReleaseStringChars(text, utf16_text);

    GUIEngine::addGUIFunctionBeforeRendering([widget_id, to_editbox, start,
        end, composing_start, composing_end]()
        {
            TextBoxWidget* tb =
                dynamic_cast<TextBoxWidget*>(getFocusForPlayer(0));
            if (!tb || (int)widget_id != tb->getID())
                return;
            MyCGUIEditBox* eb = tb->getIrrlichtElement<MyCGUIEditBox>();
            if (!eb)
                return;
            core::stringw old_text = eb->getText();
            eb->fromAndroidEditText(to_editbox, start, end, composing_start,
                composing_end);
            if (old_text != eb->getText())
                eb->handleTextUpdated();
        });
}

#define MAKE_HANDLE_ACTION_NEXT_CALLBACK(x) JNIEXPORT void JNICALL Java_ ## x##_STKEditText_handleActionNext(JNIEnv* env, jobject this_obj, jint widget_id)
#define ANDROID_HANDLE_ACTION_NEXT_CALLBACK(PKG_NAME) MAKE_HANDLE_ACTION_NEXT_CALLBACK(PKG_NAME)

extern "C"
ANDROID_HANDLE_ACTION_NEXT_CALLBACK(ANDROID_PACKAGE_CALLBACK_NAME)
{
    GUIEngine::addGUIFunctionBeforeRendering([widget_id]()
        {
            TextBoxWidget* tb =
                dynamic_cast<TextBoxWidget*>(getFocusForPlayer(0));
            if (!tb || (int)widget_id != tb->getID())
                return;
            MyCGUIEditBox* eb = tb->getIrrlichtElement<MyCGUIEditBox>();
            if (!eb)
                return;

            // First test for onEnterPressed, if true then close keyboard
            if (eb->handleEnterPressed())
            {
                // Clear text like onEnterPressed callback
                GUIEngine::getDevice()->toggleOnScreenKeyboard(false,
                    1/*clear_text*/);
                return;
            }

            // As it's action "next", check if below widget is a text box, if
            // so focus it and keep the screen keyboard, so user can keep
            // typing
            int id = GUIEngine::EventHandler::get()->findIDClosestWidget(
                NAV_DOWN, 0, tb, true/*ignore_disabled*/);
            TextBoxWidget* closest_tb = NULL;
            if (id != -1)
            {
                closest_tb =
                    dynamic_cast<TextBoxWidget*>(GUIEngine::getWidget(id));
            }

            if (closest_tb)
            {
                closest_tb->setFocusForPlayer(0);
            }
            else
            {
                // Post an enter event and close the keyboard
                SEvent enter_event;
                enter_event.EventType = EET_KEY_INPUT_EVENT;
                enter_event.KeyInput.Char = 0;
                enter_event.KeyInput.PressedDown = true;
                enter_event.KeyInput.Key = IRR_KEY_RETURN;
                GUIEngine::getDevice()->postEventFromUser(enter_event);
                enter_event.KeyInput.PressedDown = false;
                GUIEngine::getDevice()->postEventFromUser(enter_event);
                GUIEngine::getDevice()->toggleOnScreenKeyboard(false);
            }
        });
}
#endif
