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

#include "states_screens/options/options_common.hpp"

#include "guiengine/screen.hpp"

namespace OptionsCommon
{
	void switchTab(std::string selected_tab)
	{
		GUIEngine::Screen *screen = NULL;
    	if (selected_tab == "tab_audio")
        	screen = OptionsScreenAudio::getInstance();
    	else if (selected_tab == "tab_display")
        	screen = OptionsScreenDisplay::getInstance();
    	else if (selected_tab == "tab_video")
        	screen = OptionsScreenVideo::getInstance();
    	else if (selected_tab == "tab_players")
        	screen = TabbedUserScreen::getInstance();
    	else if (selected_tab == "tab_controls")
        	screen = OptionsScreenInput::getInstance();
    	else if (selected_tab == "tab_ui")
        	screen = OptionsScreenUI::getInstance();
    	else if (selected_tab == "tab_general")
        	screen = OptionsScreenGeneral::getInstance();
    	else if (selected_tab == "tab_language")
        	screen = OptionsScreenLanguage::getInstance();
    	if(screen)
        	StateManager::get()->replaceTopMostScreen(screen);
	}

	// In the in-game pause options, disable the players and language tabs
	void setTabStatus()
	{
		if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
		{
	    	GUIEngine::getWidget("tab_players")->setActive(false);
	    	GUIEngine::getWidget("tab_language")->setActive(false);			
		}
		else
		{
			GUIEngine::getWidget("tab_players")->setActive(true);
	    	GUIEngine::getWidget("tab_language")->setActive(true);
	    }
	} // setTabStatus
}