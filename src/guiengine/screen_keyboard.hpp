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

#ifndef HEADER_SCREEN_KEYBOARD_HPP
#define HEADER_SCREEN_KEYBOARD_HPP

#include <IGUIButton.h>
#include <IGUIWindow.h>

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/skin.hpp"
#include "input/input_manager.hpp"
#include "utils/leak_check.hpp"


class CGUIEditBox;

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
    class ButtonWidget;

    /**
     * \brief Class representing a screen keyboard. Only once instance at a 
     * time (if you create a 2nd the first will be destroyed). You can call 
     * static function 'dismiss' to simply close the keyboard (so you don't
     * need to keep track of instances yourself)
     * \ingroup guiengine
     */
    class ScreenKeyboard : public SkinWidgetContainer, 
                           public AbstractTopLevelContainer
    {
    protected:
        typedef std::vector<std::vector<std::string> > KeyboardLayout;
        typedef std::vector<std::vector<int> > KeyboardLayoutProportions;
        enum ButtonsType
        {
            BUTTONS_NONE,
            BUTTONS_LOWER,
            BUTTONS_UPPER,
            BUTTONS_DIGITS,
            BUTTONS_DIGITS2,
            BUTTONS_EMOJI
        };
    private:
        /** Global instance of the current screen keyboard */
        static ScreenKeyboard* m_screen_keyboard;
        
        /** A value in range of 0.0 to 1.0 that determines width of the screen 
         *  that is used by the keyboard */
        float m_percent_width;
        
        /** A value in range of 0.0 to 1.0 that determines height of the screen 
         *  that is used by the keyboard */
        float m_percent_height;
        
        /** A time for repeat key feature */
        unsigned int m_repeat_time;
        
        /** True if backspace button was pressed */
        bool m_back_button_pressed;
        
        /** True if screen keyboard is going to be closed */
        bool m_schedule_close;
        
        /** The edit box that is assigned to the keyboard */
        CGUIEditBox* m_edit_box;
        
        /** A button that is used as backspace key */
        irr::gui::IGUIButton* m_back_button;
        
        /** Remembers currently selected button type */
        ButtonsType m_buttons_type;
        
        /** Irrlicht window used by the keyboard widget */
        irr::gui::IGUIWindow* m_irrlicht_window;
        
        /** Contains position and dimensions of the keyboard */
        irr::core::rect<irr::s32> m_area;
        
        /** Contans the pointers to all button widgets */
        std::vector<ButtonWidget*> m_buttons;
        
        /** Remembered input mode that was used before keyboard creation */
        InputManager::InputDriverMode m_previous_mode;

        void createButtons();
        void assignButtons(ButtonsType buttons_type);
        core::stringw getKeyName(std::string key_id);

    public:
        LEAK_CHECK()

        ScreenKeyboard(float percent_width, float percent_height, 
                       CGUIEditBox* edit_box);
        ~ScreenKeyboard();

        void init();

        virtual EventPropagation processEvent(const std::string& eventSource);

        static void dismiss();
        static bool onEscapePressed();
        /** Returns pointer to the created keyboard or NULL if keyboard was
         *  not created */
        static ScreenKeyboard* getCurrent() {return m_screen_keyboard;}
        
        /** Returns true if keyboard is created */
        static bool isActive() {return m_screen_keyboard != NULL;}

        static bool shouldUseScreenKeyboard();
        static bool hasSystemScreenKeyboard();

        /** Override to be notified of updates */
        virtual void onUpdate(float dt);
        
        bool onEvent(const SEvent &event);
        
        /** Get irrlicht window used by the keyboard widget */
        irr::gui::IGUIWindow* getIrrlichtElement() {return m_irrlicht_window;}

        /** Checks if the screen keyboard is a parent of the selected item
         *  \param widget A widget that should be checked
         *  \return True if keyboard is the parent
         */
        bool isMyIrrChild(irr::gui::IGUIElement* widget) const 
                                {return m_irrlicht_window->isMyChild(widget);}

        /** Returns width of the screen keyboard */
        int getWidth()  {return m_area.getWidth();}
        
        /** Returns height of the screen keyboard */
        int getHeight() {return m_area.getHeight();}
        
        /** Returns assigned edit box */
        CGUIEditBox* getEditBox() {return m_edit_box;}

        virtual KeyboardLayoutProportions getKeyboardLayoutProportions() const;

        virtual KeyboardLayout* getKeyboardLayout(ButtonsType bt) const;

        virtual ButtonsType getDefaultButtonsType() const
        {
            return BUTTONS_LOWER;
        }
    };
}

#endif
