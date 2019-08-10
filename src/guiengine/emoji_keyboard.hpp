//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#ifndef HEADER_EMOJI_KEYBOARD_HPP
#define HEADER_EMOJI_KEYBOARD_HPP

#include "guiengine/screen_keyboard.hpp"
#include "utils/cpp2011.hpp"

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
    class EmojiKeyboard : public ScreenKeyboard
    {
    public:
        LEAK_CHECK()
        EmojiKeyboard(float percent_width, float percent_height, CGUIEditBox* edit_box)
            : ScreenKeyboard(percent_width, percent_height, edit_box) {}
        virtual KeyboardLayoutProportions getKeyboardLayoutProportions() const OVERRIDE;
        virtual KeyboardLayout* getKeyboardLayout(ButtonsType bt) const OVERRIDE;
        virtual ButtonsType getDefaultButtonsType() const OVERRIDE { return BUTTONS_EMOJI; }
    };
}

#endif
