//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License: or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/layout_manager.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/CGUIEditBox.hpp"
#include "utils/log.hpp"

#include <algorithm>
#include <string>

using namespace GUIEngine;

#define KEYBOARD_COLS_NUM 11
#define KEYBOARD_ROWS_NUM 3
typedef std::string KeyboardLayout[KEYBOARD_ROWS_NUM][KEYBOARD_COLS_NUM];
    
KeyboardLayout layout_lower = 
            {{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p",     "123"  },
             {"a", "s", "d", "f", "g", "h", "j", "k", "l", "Back",  "Shift"},
             {"z", "x", "c", "v", "b", "n", "m", ",", ".", "Space", "Enter"}};
                                         
KeyboardLayout layout_upper = 
            {{"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",     "123"  },
             {"A", "S", "D", "F", "G", "H", "J", "K", "L", "Back",  "Shift"},
             {"Z", "X", "C", "V", "B", "N", "M", ",", ".", "Space", "Enter"}};
                                   
KeyboardLayout layout_digits = 
            {{"1", "2", "3", "!", "@", "#", "$", "%", "^", "&",     "Text" },
             {"4", "5", "6", "*", "(", ")", "-", "_", "=", "Back",  "Shift"},
             {"7", "8", "9", "0", "+", "/", "?", ",", ".", "Space", "Enter"}};
                                    
KeyboardLayout layout_digits2 = 
            {{"1", "2", "3", "[", "]",  "{",  "}", ";", ":", "\\",    "Text" },
             {"4", "5", "6", "|", "\'", "\"", "<", ">", "`", "Back",  "Shift"},
             {"7", "8", "9", "0", "~",  "/",  "?", ",", ".", "Space", "Enter"}};

ScreenKeyboard* ScreenKeyboard::m_screen_keyboard = NULL;

// ----------------------------------------------------------------------------
/** The screen keyboard constructor
 *  \param percent_width A relative value in range of 0.0 to 1.0 that 
 *         determines width of the screen that will be used by the keyboard.
 *  \param percent_height A relative value in range of 0.0 to 1.0 that 
 *         determines height of the screen that will be used by the keyboard.
 *  \param edit_box The edit box that is assigned to the keyboard.
 */
ScreenKeyboard::ScreenKeyboard(float percent_width, float percent_height, 
                               CGUIEditBox* edit_box)
{
    if (m_screen_keyboard != NULL)
    {
        delete m_screen_keyboard;
        Log::warn("GUIEngine", "Showing a screen keyboard while the previous "
                  "one is still open. Destroying the previous keyboard.");
    }
    
    m_screen_keyboard = this;
    m_buttons_type    = BUTTONS_NONE;
    m_percent_width   = std::min(std::max(percent_width, 0.0f), 1.0f);
    m_percent_height  = std::min(std::max(percent_height, 0.0f), 1.0f);
    m_irrlicht_window = NULL;
    m_edit_box        = edit_box;
    
    init();
}   // ScreenKeyboard

// ----------------------------------------------------------------------------
/** The screen keyboard destructor
 */
ScreenKeyboard::~ScreenKeyboard()
{
    m_screen_keyboard = NULL;
    
    GUIEngine::getGUIEnv()->removeFocus(m_irrlicht_window);
    m_irrlicht_window->remove();

    input_manager->setMode(m_previous_mode);

    elementsWereDeleted();
}   // ~ScreenKeyboard

// ----------------------------------------------------------------------------
/** Internal function that makes whole screen keyboard initialization
 */
void ScreenKeyboard::init()
{
    const core::dimension2d<u32>& frame_size = irr_driver->getFrameSize();

    int margin = 15;
    int w = int(frame_size.Width * m_percent_width);
    int h = int(frame_size.Height * m_percent_height);
    int x = frame_size.Width/2 - w/2;
    int y = frame_size.Height - h - margin;

    if (m_edit_box != NULL)
    {
        core::rect<s32> pos = m_edit_box->getAbsolutePosition();
        
        if (pos.LowerRightCorner.Y + 5 > y)
        {
            y = margin;
        }
    }
    
    m_area = core::rect<s32>(x, y, x + w, y + h);

    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow(m_area, true);
    m_irrlicht_window->setDrawTitlebar(false);
    m_irrlicht_window->getCloseButton()->setVisible(false);
    m_irrlicht_window->setDraggable(UserConfigParams::m_artist_debug_mode);

    m_previous_mode=input_manager->getMode();
    input_manager->setMode(InputManager::MENU);
    
    createButtons();
    assignButtons(BUTTONS_LOWER);
}   // init

// ----------------------------------------------------------------------------
/** Creates all button widgets
 */
void ScreenKeyboard::createButtons()
{
    int pos_x = 1;
    int pos_y = 10;
    int width = (m_area.getWidth() - 2 * pos_x) / KEYBOARD_COLS_NUM;
    int height = (m_area.getHeight() - 2 * pos_y) / KEYBOARD_ROWS_NUM;

    char width_str[100];
    sprintf(width_str, "%i", width);
    char height_str[100];
    sprintf(height_str, "%i", height);
    
    for (int i = 0; i < KEYBOARD_ROWS_NUM; i++)
    {
        char tmp[100];
        sprintf(tmp, "%i", pos_y + height * i);
        std::string pos_y_str = tmp;
        
        for (int j = 0; j < KEYBOARD_COLS_NUM; j++)
        {
            char tmp[100];
            sprintf(tmp, "%i", pos_x + width * j);
            std::string pos_x_str = tmp;
            
            ButtonWidget* button = new ButtonWidget();
            button->setParent(m_irrlicht_window);
            button->m_properties[PROP_WIDTH] = width_str;
            button->m_properties[PROP_HEIGHT] = height_str;
            button->m_properties[PROP_X] = pos_x_str;
            button->m_properties[PROP_Y] = pos_y_str;
            m_widgets.push_back(button);
            m_buttons.push_back(button);
        }
    }

    LayoutManager::calculateLayout(m_widgets, this);
    addWidgetsRecursively(m_widgets);
}   // createButtons

// ----------------------------------------------------------------------------
/** A function that allows to select one of the available buttons layout
 *  \param buttons_type One of the available buttons type
 */
void ScreenKeyboard::assignButtons(ButtonsType buttons_type)
{
    m_buttons_type = buttons_type;
    
    KeyboardLayout* keys = NULL;
    
    switch (buttons_type)
    {
    case BUTTONS_LOWER:
        keys = &layout_lower;
        break;
    case BUTTONS_UPPER:
        keys = &layout_upper;
        break;
    case BUTTONS_DIGITS:
        keys = &layout_digits;
        break;
    case BUTTONS_DIGITS2:
        keys = &layout_digits2;
        break;
    default:
        keys = NULL;
        break;
    };
    
    for (int i = 0; i < KEYBOARD_ROWS_NUM; i++)
    {
        for (int j = 0; j < KEYBOARD_COLS_NUM; j++)
        {
            std::string key = keys != NULL ? (*keys)[i][j] : "?";

            ButtonWidget* button = m_buttons[i * KEYBOARD_COLS_NUM + j];
            button->setText(key.c_str());
            button->m_properties[PROP_ID] = key;
        }
    }
}   // assignButtons

// ----------------------------------------------------------------------------
/** A function that handles buttons events
 *  \param eventSource Button ID
 *  \return Block event if edit box is assigned
 */
EventPropagation ScreenKeyboard::processEvent(const std::string& eventSource)
{
    if (m_edit_box == NULL)
        return EVENT_LET;
        
    SEvent event;
    bool send_event = false;
        
    if (eventSource == "Shift")
    {
        switch (m_buttons_type)
        {
        case BUTTONS_UPPER:
            assignButtons(BUTTONS_LOWER);
            break;
        case BUTTONS_LOWER:
            assignButtons(BUTTONS_UPPER);
            break;
        case BUTTONS_DIGITS:
            assignButtons(BUTTONS_DIGITS2);
            break;
        case BUTTONS_DIGITS2:
            assignButtons(BUTTONS_DIGITS);
            break;
        default:
            break;
        }
    }
    else if (eventSource == "123")
    {
        assignButtons(BUTTONS_DIGITS);
    }
    else if (eventSource == "Text")
    {
        assignButtons(BUTTONS_LOWER);
    }
    else if (eventSource == "Enter")
    {
        dismiss();
        return EVENT_BLOCK;
    }
    else if (eventSource == "Back")
    {
        event.KeyInput.Key = IRR_KEY_BACK;
        event.KeyInput.Char = 0;
        send_event = true;
    }
    else if (eventSource == "Space")
    {
        event.KeyInput.Key = IRR_KEY_UNKNOWN;
        event.KeyInput.Char = ' ';
        send_event = true;
    }
    else if (eventSource.size() > 0)
    {
        event.KeyInput.Key = IRR_KEY_UNKNOWN;
        event.KeyInput.Char = eventSource.at(0);
        send_event = true;
    }

    if (send_event)
    {
        event.EventType = EET_KEY_INPUT_EVENT;
        event.KeyInput.PressedDown = true;
        event.KeyInput.Control = false;
        event.KeyInput.Shift = false;

        m_edit_box->OnEvent(event);
    }

    return EVENT_BLOCK;
}   // processEvent

// ----------------------------------------------------------------------------
/** A function that closes the keyboard
 */
void ScreenKeyboard::dismiss()
{
    delete m_screen_keyboard;
    m_screen_keyboard = NULL;
}   // dismiss

// ----------------------------------------------------------------------------
/** A function that handles escape pressed event
 */
bool ScreenKeyboard::onEscapePressed()
{
    dismiss();
    return true;
}   // onEscapePressed

// ----------------------------------------------------------------------------
