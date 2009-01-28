//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include "gui/player_controls.hpp"

#include <SDL/SDL.h>

#include "sdldrv.hpp"
#include "user_config.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_PLYR_NAME0,
    WTOK_PLYR_NAME1,

    WTOK_LEFT,
    WTOK_RIGHT,
    WTOK_ACCEL,
    WTOK_BRAKE,
    WTOK_NITRO,
    WTOK_DRIFT,
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


const char *sKartAction2String[KA_LAST+1] =
{
     //I18N: name of controls
     _("Left"),
     //I18N: name of controls (here, 'right' is the opposite of 'left' not the opposite of 'wrong')
     _("Right"),
     //I18N: name of controls
     _("Accelerate"), _("Brake"),  _("Nitro"), _("Sharp Turn"), _("Rescue"),
    //I18N: name of controls, like in "fire missile"
     _("Fire"),
    //I18N: name of controls
     _("Look back")
};


PlayerControls::PlayerControls(int whichPlayer):
    m_player_index(whichPlayer),
    m_grab_input(false)
{   
    std::string s = StringUtils::insert_values( _("Choose your controls, %s"),
                            user_config->m_player[m_player_index].getName());

    widget_manager->addTitleWgt( WTOK_TITLE, 60, 7, s);
    widget_manager->hideWgtRect(WTOK_TITLE);
    widget_manager->breakLine();

    widget_manager->addTextWgt( WTOK_PLYR_NAME0, 30, 7, _("Player name") );

    m_name = user_config->m_player[m_player_index].getName();
    widget_manager->addTextButtonWgt( WTOK_PLYR_NAME1, 30, 7, m_name );
    widget_manager->breakLine();

    widget_manager->switchOrder();
    for(int i = KA_FIRST; i <= KA_LAST; i++)
    {
        // Note: even though that all strings in sKartAction2Strings above
        // are in _(), they are not translated (since gettext is actually 
        // called at startup (just after loading) of the program, when
        // gettext is not yet initialised - so it returns the untranslated
        // strings). So we add an additional _() here (and in help_page_one).
        widget_manager->addTextWgt( WTOK_KEY0 + i, 30, 7, _(sKartAction2String[i]) );
    }
    widget_manager->breakLine();


    KartAction control;
    widget_manager->switchOrder();
    for(int i = KA_FIRST; i <= KA_LAST; i++)
    {
        control = (KartAction)i;
        m_key_names[control] = user_config->getMappingAsString(m_player_index, control);

        widget_manager->addTextButtonWgt( WTOK_LEFT + i, 30, 7,
            m_key_names[control].c_str());
    }
    widget_manager->breakLine();
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt( WTOK_QUIT, 60, 7, _("Press <ESC> to go back") );
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML);

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
		
            inputDriver->setMode(SDLDriver::LOWLEVEL);
		
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
            // Prefer axis (in case of gamepads that deliver two different
            // events for buttons, e.g. a digital event and an anlogue one)
            // for left/right, in all other cases prefer the button.
            inputDriver->setMode( (selected==WTOK_RIGHT||selected==WTOK_LEFT)
                                  ? SDLDriver::INPUT_SENSE_PREFER_AXIS
                                  : SDLDriver::INPUT_SENSE_PREFER_BUTTON     );
			
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
            {
                // Updates the configuration with the newly sensed input.
                const Input &new_input = inputDriver->getSensedInput();
                // Don't use reference for old_input, otherwise old_input
                // is changed when setInput is called.
                const Input old_input = user_config->getInput(m_player_index, 
                                                               m_edit_action);
                user_config->setInput(m_player_index,
                                      m_edit_action,
                                      new_input);
                // If left/right is set to a stick motion, and right/left is
                // currently set to be something else
                if(new_input.type == Input::IT_STICKMOTION &&
                   new_input.id2 != Input::AD_NEUTRAL     &&
                   old_input.type != Input::IT_STICKMOTION)
                {
                    Input inp(new_input);
                    inp.id2 = (inp.id2==Input::AD_NEGATIVE) ? Input::AD_POSITIVE
                                                            : Input::AD_NEGATIVE;
                    user_config->setInput(m_player_index,
                                          m_edit_action==KA_LEFT ? KA_RIGHT 
                                                                 : KA_LEFT,
                                          inp);
                }

                // Fall through to recover the widget labels.
            }
		case GA_SENSE_CANCEL:
            inputDriver->setMode(SDLDriver::MENU);
		
			// Refresh all key labels since they mave changed because of
			// conflicting bindings.
			updateAllKeyLabels();
			break;
		case GA_ENTER:
			// If the user is typing her name this will be finished at this
			// point.
            if (inputDriver->isInMode(SDLDriver::LOWLEVEL))
			{
				// Prevents zero-length names.
				if (m_name.length() == 0)
                    //I18N: as in 'Player 2'
					m_name = _("Player ") + m_player_index;
				user_config->m_player[m_player_index].setName(m_name);
				widget_manager->setWgtText(WTOK_PLYR_NAME1, m_name.c_str());

                inputDriver->setMode(SDLDriver::MENU);
			}
			else
				select();
			break;
		case GA_LEAVE:
			// If the user is typing her name this will be cancelled at this
			// point.
            if (inputDriver->isInMode(SDLDriver::LOWLEVEL))
			{
				m_name = user_config->m_player[m_player_index].getName();
				widget_manager->setWgtText(WTOK_PLYR_NAME1, m_name.c_str());

                inputDriver->setMode(SDLDriver::MENU);
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
