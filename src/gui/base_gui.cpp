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

#include "base_gui.hpp"
#include "widget_set.hpp"
#include "world.hpp"
#include "menu_manager.hpp"

void BaseGUI::input(InputType type, int id0, int  id1, int id2, int value)
{
    switch (type)
    {
    case IT_KEYBOARD:
        inputKeyboard(id0, value);
        break;
    case IT_MOUSEBUTTON:
      if (!value) // Act on button release only.
        switch (id0)
        {
            case SDL_BUTTON_LEFT:
                select();
                break;
            case SDL_BUTTON_RIGHT:
                inputKeyboard(SDLK_ESCAPE, 0);
            break;
        }
        break;

    case IT_STICKMOTION:
        if (id1 == 0)
        {
          // X-Axis
          inputKeyboard((id2 == AD_NEGATIVE) ? SDLK_LEFT : SDLK_RIGHT, !value);
        }
        else if (id1 == 1)
        {
          // Y-Axis
          inputKeyboard((id2 == AD_NEGATIVE) ? SDLK_UP : SDLK_DOWN, !value);
        }
        break;

    case IT_STICKBUTTON:
        if( !value) // act on button release only
            switch (id1) // Button no
            {
            case 0:
                inputKeyboard(SDLK_RETURN, 0);
                break;
            case 1:
                inputKeyboard(SDLK_ESCAPE, 0);
                break;
            }
        break;
    default:
        break;
    }

}

//-----------------------------------------------------------------------------
/**
 * Important note: One day the STK engine code will have no notion of SDL
 * key, button, axes and so on. It will only know actions like menu up, menu
 * down, enter menu, leave menu, ...
 *
 * However this requires some major reworking. Until this is done SDL's keys
 * take the role of the actions. That is why joystick axes & buttons and mouse
 * buttons are converted to key input (see BaseGUI::input).
 *
 * When the game actions are implemented not dealing with the input mechanisms
 * gives more flexibility:
 *  - issue no game actions when input sensing is active
 *  - what issues a certain game action can be conveniently selected
 *    (at compile or runtime, depending on the need)
 *
 * Please keep this goal in mind when you work on the input stuff.
 */
void BaseGUI::inputKeyboard(int key, int pressed)
{
    // Skip on keypress, act on keyrelease only.
    if (pressed)
        return;

    switch ( key )
    {
    case SDLK_LEFT:
    case SDLK_RIGHT:
    case SDLK_UP:
    case SDLK_DOWN:
        widgetSet->pulse(widgetSet->cursor(m_menu_id, key), 1.2f);
        break;

    case SDLK_SPACE:
    case SDLK_RETURN:
        select();
        break;

    case SDLK_ESCAPE:
        if (menu_manager->getMenuStackSize() > 1)
        {
           if(menu_manager->isCurrentMenu(MENUID_RACEMENU))
             world->unpause();
    
           menu_manager->popMenu();
        }
        break;

    default:
        break;
    }   // switch
}   // inputKeyboard
//-----------------------------------------------------------------------------
void
BaseGUI::inputPointer(int x, int y)
{
  widgetSet->pulse(widgetSet->point(m_menu_id, x, y), 1.2f);
}

//-----------------------------------------------------------------------------
void BaseGUI::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
    widgetSet -> paint(m_menu_id) ;
}   // update

//-----------------------------------------------------------------------------
void BaseGUI::TimeToString(const float TIME, char *s)
{
    int min     = (int) floor ( TIME / 60.0 ) ;
    int sec     = (int) floor ( TIME - (double) ( 60 * min ) ) ;
    int tenths  = (int) floor ( 10.0f * (TIME - (double)(sec + 60* min)));
    sprintf ( s, "%d:%02d:%d", min,  sec,  tenths ) ;
}   // TimeToString

//-----------------------------------------------------------------------------
/* EOF */
