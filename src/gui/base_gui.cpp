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

    case IT_MOUSEMOTION:
        widgetSet->pulse(widgetSet->point(m_menu_id, id0, id1), 1.2f);

#ifdef  ALT_MOUSE_HANDLING
        if (id0 == 1 && value)
            if (id1 == AD_NEGATIVE)
                inputKeyboard(SDLK_UP, 1);
            else
                inputKeyboard(SDLK_DOWN, 1);
#endif
        break;

    case IT_MOUSEBUTTON:
      if (!value) // Act on button release only.
        switch (id0)
        {
            case SDL_BUTTON_LEFT:
                select();
                break;
            case SDL_BUTTON_RIGHT:
                if (menu_manager->getMenuStackSize() > 1)
                {
                    if(menu_manager->isCurrentMenu(MENUID_RACEMENU))
                    {
                        world->unpause();
                    }

                    menu_manager->popMenu();
                }
            break;
        }
        break;

    case IT_STICKMOTION:
        widgetSet -> pulse(widgetSet -> stick(m_menu_id, id1, id2, value), 1.2f);
        break;

    case IT_STICKBUTTON:
        if( !value) // act on button release only
            switch (id1) // Button no
            {
            case 0:
                select();
                break;
            case 1:
                if (menu_manager->getMenuStackSize() > 1)
                {
                    if(menu_manager->isCurrentMenu(MENUID_RACEMENU))
                    {
                        world->unpause();
                    }

                    menu_manager->popMenu();
                }
                break;
            }
        break;
    default:
        break;
    }

}

//-----------------------------------------------------------------------------
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
            //We don't need to handle the race gui pause with the keyboard
            //(but we have to with the joystick & mouse) because it has it's
            //own keyboard handling function.

            menu_manager->popMenu();
        }
        break;

    default:
        break;
    }   // switch
}   // inputKeyboard

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
