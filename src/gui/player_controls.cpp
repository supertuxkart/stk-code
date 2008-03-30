//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <SDL/SDL.h>

#include "widget_manager.hpp"
#include "player_controls.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "sdldrv.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_PLYR_NAME0,
    WTOK_PLYR_NAME1,

    WTOK_LEFT,
    WTOK_RIGHT,
    WTOK_ACCEL,
    WTOK_BRAKE,
    WTOK_WHEELIE,
    WTOK_JUMP,
    WTOK_RESCUE,
    WTOK_FIRE,
    WTOK_LOOK_BACK,

    WTOK_KEY0,
    WTOK_KEY1,
    WTOK_KEY2,
    WTOK_KEY3,
    WTOK_KEY4,
    WTOK_KEY5,
    WTOK_KEY6,
    WTOK_KEY7,
    WTOK_KEY8,

    WTOK_QUIT
};

/** Limits the maximum length of the player name. */
const size_t PlayerControls::PLAYER_NAME_MAX = 10;

const char *sKartAction2String[KA_LAST+1] = {_("Left"), _("Right"), _("Accelerate"),
                                             _("Brake"),  _("Wheelie"), _("Jump"),
                                             _("Rescue"), _("Fire"), _("Look back") };


PlayerControls::PlayerControls(int whichPlayer):
    m_player_index(whichPlayer),
    m_grab_input(false)
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI );

    widget_manager->addWgt( WTOK_TITLE, 60, 7);
    sprintf(m_heading, _("Choose your controls, %s"),
            user_config->m_player[m_player_index].getName().c_str());
    widget_manager->setWgtText( WTOK_TITLE, m_heading);
    widget_manager->breakLine();

    widget_manager->addWgt( WTOK_PLYR_NAME0, 30, 7);
    widget_manager->setWgtText( WTOK_PLYR_NAME0, _("Player name"));

    widget_manager->addWgt( WTOK_PLYR_NAME1, 30, 7);
    m_name = user_config->m_player[m_player_index].getName();
    widget_manager->setWgtText( WTOK_PLYR_NAME1, m_name);
    widget_manager->activateWgt( WTOK_PLYR_NAME1);
    widget_manager->breakLine();

    KartAction control;
    for(int i = KA_FIRST; i <= KA_LAST; i++)
    {
        widget_manager->addWgt( WTOK_KEY0 + i, 30, 7);
        widget_manager->setWgtText( WTOK_KEY0 + i, sKartAction2String[i]);

        control = (KartAction)i;
        m_key_names[control] = user_config->getMappingAsString(m_player_index, control);
        widget_manager->addWgt( WTOK_LEFT + i, 30, 7);
        widget_manager->setWgtText( WTOK_LEFT + i, m_key_names[control].c_str());
        widget_manager->activateWgt( WTOK_LEFT + i);

        widget_manager->breakLine();
    }

    widget_manager->addWgt( WTOK_QUIT, 60, 7);
    widget_manager->setWgtText( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML);
    widget_manager->activateWgt( WTOK_QUIT);

    widget_manager->layout(WGT_AREA_ALL);
}   // PlayerControls

//-----------------------------------------------------------------------------
PlayerControls::~PlayerControls()
{
    widget_manager->reset();
    // The unicode translation is not generally needed, so disable it again.
}   // ~PlayerControls

//-----------------------------------------------------------------------------
void
PlayerControls::select()
{
    const int selected = widget_manager->getSelectedWgt();
	switch (selected)
	{
		case WTOK_PLYR_NAME1:
			// Switch to typing in the player's name.
			widget_manager->setWgtText(WTOK_PLYR_NAME1, (m_name + "<").c_str());
		
			inputDriver->setMode(LOWLEVEL);
		
        	break;
		case WTOK_QUIT:
			// Leave menu.
	        menu_manager->popMenu();
		
			break;
		default:
			// Switch to input sensing.

			// If the only remaining and not yet handled widgets are the ones
			// that deal with the kart controls and the values are still in the
			// correct order the assertion should hold. If not did something
			// bad.
			assert (selected >= WTOK_LEFT
					&& selected <= WTOK_LOOK_BACK);
		
			
		    m_edit_action = static_cast<KartAction>(selected - WTOK_LEFT);
		    widget_manager->setWgtText(selected, _("Press key"));
			
			inputDriver->setMode(INPUT_SENSE);
			
			break;
	}
}   // select
//-----------------------------------------------------------------------------
void
PlayerControls::inputKeyboard(SDLKey key, int unicode)
{
	switch (key)
	{
	case SDLK_RSHIFT:
	case SDLK_LSHIFT:
		// Ignore shift, otherwise shift will disable input
		// (making it impossible to enter upper case characters)
	case SDLK_SPACE:
		// Ignore space to prevent invisible names.
			
		// Note: This will never happen as long as SPACE has a mapping which
		// causes GA_ENTER and therefore finishes the typing. Please leave this
		// because I am not sure whether this is good behavior (that SPACE
		// cannot reach inputKeyboard()) and with some changes to the input
		// driver this code has suddenly a useful effect.
	case SDLK_KP_ENTER:
	case SDLK_RETURN:
	case SDLK_ESCAPE:
		// Ignore some control keys. What they could provide is implemented
		// in the handle() method.
		return;
	case SDLK_BACKSPACE:
		// Handle backspace.
		if (m_name.size() >=1)
			m_name.erase(m_name.size()-1, 1);
		
		widget_manager->setWgtText(WTOK_PLYR_NAME1, (m_name + "<").c_str());
		break;
		break;
	default:
		// Adds the character to the name.
		// For this menu only unicode translation is enabled.
		// So we use the unicode character here, since this will
		// take care of upper/lower case etc.
		if (unicode && m_name.size() <= PLAYER_NAME_MAX)
			m_name += (char) unicode;
		widget_manager->setWgtText(WTOK_PLYR_NAME1, (m_name + "<").c_str());
		break;
	}

}
//-----------------------------------------------------------------------------
void
PlayerControls::clearMapping()
{
	const int selected = widget_manager->getSelectedWgt();
	if (selected >= WTOK_LEFT && selected <= WTOK_LOOK_BACK)
	{
		user_config->clearInput(m_player_index,
							  (KartAction) (selected - WTOK_LEFT));
		updateAllKeyLabels();
	}
}
//-----------------------------------------------------------------------------
void PlayerControls::handle(GameAction ga, int value)
{
	if (value)
		return;
	
	switch (ga)
	{
		case GA_CLEAR_MAPPING:
            clearMapping();

			break;
		case GA_SENSE_COMPLETE:
			// Updates the configuration with the newly sensed input.
            user_config->setInput(m_player_index,
								  m_edit_action,
								  inputDriver->getSensedInput());
			// Fall through to recover the widget labels.
		case GA_SENSE_CANCEL:
			inputDriver->setMode(MENU);
		
			// Refresh all key labels since they mave changed because of
			// conflicting bindings.
			updateAllKeyLabels();
			break;
		case GA_ENTER:
			// If the user is typing her name this will be finished at this
			// point.
			if (inputDriver->isInMode(LOWLEVEL))
			{
				// Prevents zero-length names.
				if (m_name.length() == 0)
					m_name = _("Player ") + m_player_index;
				user_config->m_player[m_player_index].setName(m_name);
				widget_manager->setWgtText(WTOK_PLYR_NAME1, m_name.c_str());

				inputDriver->setMode(MENU);
			}
			else
				select();
			break;
		case GA_LEAVE:
			// If the user is typing her name this will be cancelled at this
			// point.
			if (inputDriver->isInMode(LOWLEVEL))
			{
				m_name = user_config->m_player[m_player_index].getName();
				widget_manager->setWgtText(WTOK_PLYR_NAME1, m_name.c_str());

				inputDriver->setMode(MENU);
				break;
			}
			// Fall through to reach the usual GA_LEAVE code (leave menu).
		default:
			BaseGUI::handle(ga, value);
	}
	
}

//-----------------------------------------------------------------------------
void
PlayerControls::updateAllKeyLabels()
{
	for (int i = KA_FIRST; i <= KA_LAST; i++)
  {
    m_key_names[i] = user_config->getMappingAsString(m_player_index,
													 (KartAction) i);
    widget_manager->setWgtText(WTOK_LEFT + i, m_key_names[i].c_str());
  }
}
