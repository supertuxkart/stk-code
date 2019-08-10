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

#include "guiengine/emoji_keyboard.hpp"
#include "io/file_manager.hpp"
#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/utf8.h"

#include <fstream>
#include <memory>
#include <string>

using namespace GUIEngine;
// ============================================================================
ScreenKeyboard::KeyboardLayoutProportions
                            EmojiKeyboard::getKeyboardLayoutProportions() const
{
    return
        {
            {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
            {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
            {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
            {2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
        };
}   // getKeyboardLayoutProportions

// ============================================================================
ScreenKeyboard::KeyboardLayout*
                         EmojiKeyboard::getKeyboardLayout(ButtonsType bt) const
{
    static KeyboardLayout emojis =
        {
            {"?", "?", "?", "?", "?", "?", "?", "?", "?", "?"},
            {"?", "?", "?", "?", "?", "?", "?", "?", "?", "?"},
            {"?", "?", "?", "?", "?", "?", "?", "?", "?", "Back"},
            {"?", "?", "?", "?", "?", "?", "?", "?", "?", "Enter"}
        };
    static bool loaded_emojis = false;
    if (!loaded_emojis)
    {
        const std::string file_name = file_manager->getAsset("emoji_used.txt");
        std::vector<std::string> emoji_chars;
        try
        {
            std::ifstream in(FileUtils::getPortableReadingPath(file_name));
            if (!in.is_open())
            {
                Log::error("EmojiKeyboard", "Error: failure opening: '%s'.",
                    file_name.c_str());
            }
            else
            {
                std::string line;
                while (!StringUtils::safeGetline(in, line).eof())
                {
                    // Check for possible bom mark
                    if (line[0] == '#' ||
                        utf8::starts_with_bom(line.begin(), line.end()))
                        continue;
                    emoji_chars.push_back(line);
                }
            }
        }
        catch (std::exception& e)
        {
            Log::error("EmojiKeyboard", "Error: failure emoji used file.");
            Log::error("EmojiKeyboard", "%s", e.what());
        }
        // Default if no such emoji in case errors
        emoji_chars.resize(38, "?");
        unsigned emoji_copied = 0;
        for (unsigned i = 0; i < emojis.size(); i++)
        {
            for (unsigned j = 0; j < emojis[i].size(); j++)
            {
                std::string& emoji = emojis[i][j];
                if (emoji == "Back" || emoji == "Enter")
                    continue;
                emoji = emoji_chars[emoji_copied++];
            }
        }
        loaded_emojis = true;
    }

    KeyboardLayout* keys = NULL;
    switch (bt)
    {
    case BUTTONS_EMOJI:
        keys = &emojis;
        break;
    default:
        break;
    };
    return keys;
}   // getKeyboardLayout
