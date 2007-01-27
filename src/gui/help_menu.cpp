//  $Id: help_menu.cpp 812 2006-10-07 11:43:57Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "help_menu.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "config.hpp"
#include "player.hpp"
#include "collectable_manager.hpp"
#include "material.hpp"

enum WidgetTokens {
    WTOK_FIRST_PAGE,
    WTOK_SECOND_PAGE,
    WTOK_QUIT
};

HelpMenu::HelpMenu()
{
    switch_to_first_screen();
}   // HelpMenu

//-----------------------------------------------------------------------------
void HelpMenu::switch_to_first_screen()
{
    m_menu_id = widgetSet->vstack(0);

    widgetSet->multi(m_menu_id,
//The next line starts at column 0 so no spaces appear in the game's help.
"Finish the race before other drivers, by driving and using\n\
powerups from the blue boxes! Bananas will slow you down,\n\
coins will let you get more powerups, gold coins are better.\n\
At high speeds you can use wheelies to go even faster, but\n\
be careful because you won't be able to steer.\n\
If you get stuck somewhere or fall too far from the road,\n\
use the rescue button to get back on track.\n\
Current keys bindings for the first player:",GUI_SML);

    const int HA        = widgetSet->harray(m_menu_id);
    const int CHANGE_ID = widgetSet->varray(HA);
    const int LABEL_ID  = widgetSet->varray(HA);

    for(int i = KC_LEFT; i <= KC_FIRE; i++)
    {
        //FIXME: this is temporal, just while the jumping is disabled.
        if(i == KC_JUMP) continue;

        // *sigh* widget set stores only pointer to strings, so
        // to make sure that all key-strings are permanent, they
        // are assigned to an array m_all_keys within this object.
        m_all_keys[i]=config->getInputAsString(0, (KartActions)i);
        widgetSet->label(LABEL_ID,  sKartAction2String[i], GUI_SML, GUI_LFT);
        widgetSet->label(CHANGE_ID, m_all_keys[i].c_str(), GUI_SML, GUI_RGT);
    }
    widgetSet->start(m_menu_id,"Next screen", GUI_SML, WTOK_SECOND_PAGE);
    widgetSet->state(m_menu_id,"Go back to the main menu", GUI_SML, WTOK_QUIT);
    widgetSet->layout(m_menu_id, 0, 0);
}

//-----------------------------------------------------------------------------
void HelpMenu::switch_to_second_screen()
{
    m_menu_id = widgetSet->vstack(0);

    widgetSet->label(m_menu_id,
        "To help you win, there are certain collectables you can grab:",
        GUI_SML);

    const int HA        = widgetSet->hstack(m_menu_id);
    const int LABEL_ID  = widgetSet->varray(HA);
    const int IMAGE_ID = widgetSet->vstack(HA);

    const int ICON_SIZE = 64;

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_MISSILE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Missile - fast stopper in a straight line", GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_HOMING_MISSILE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Homing missile - follows rivals, but is slower than the missile", GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_SPARK)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Fuzzy blob/Spark - very slow, but bounces from walls", GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_ZIPPER)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Zipper - speed boost", GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Parachute - slows down all karts in a better position!", GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_ANVIL)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Anvil - slows down greatly the kart in the first position", GUI_SML);

#ifdef USE_MAGNETS
    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE);
    widgetSet->label(LABEL_ID, "Missile - fast stopper in a straight line", GUI_SML);
#endif

/*    for(int i = KC_LEFT; i <= KC_FIRE; i++)
    {
        //FIXME: this is temporal, just while the jumping is disabled.
        if(i == KC_JUMP) continue;

        // *sigh* widget set stores only pointer to strings, so
        // to make sure that all key-strings are permanent, they
        // are assigned to an array m_all_keys within this object.
        m_all_keys[i]=config->getInputAsString(0, (KartActions)i);
        widgetSet->label(IMAGE_ID, m_all_keys[i].c_str(),    GUI_SML);
        widgetSet->label(LABEL_ID,  sKartAction2String[i], GUI_SML);
    }*/
    widgetSet->start(m_menu_id,"Previous screen", GUI_SML, WTOK_FIRST_PAGE);
    widgetSet->state(m_menu_id,"Go back to the main menu", GUI_SML, WTOK_QUIT);
    widgetSet->layout(m_menu_id, 0, 0);
}

//-----------------------------------------------------------------------------
HelpMenu::~HelpMenu()
{
    widgetSet -> delete_widget(m_menu_id) ;
}   // ~HelpMenu

//-----------------------------------------------------------------------------
void HelpMenu::select()
{
    switch( widgetSet->token (widgetSet->click()))
    {
        case WTOK_FIRST_PAGE:
            widgetSet -> delete_widget(m_menu_id) ;
            switch_to_first_screen();
            break;

        case WTOK_SECOND_PAGE:
            widgetSet -> delete_widget(m_menu_id) ;
            switch_to_second_screen();
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
