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

#ifndef __HEADER_OPTIONS_COMMON_HPP__
#define __HEADER_OPTIONS_COMMON_HPP__

// This file contains include headers that are used by all or most options screens.
// It also contains a standalone function to switch between the options screens.
// This simplifies maintenance.

// Config to read and save settings
#include "config/user_config.hpp"

// Frequent widgets used by multiple option screens
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

// Other option screens, for navigation between them
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_display.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/options/user_screen.hpp"

// GUI management
#include "states_screens/state_manager.hpp"

// Utils for translation of tooltips
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

namespace OptionsCommon
{
	void switchTab(std::string selected_tab);
	void setTabStatus();
}

#endif