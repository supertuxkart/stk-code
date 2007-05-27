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

#include "config_sound.hpp"
#include "widget_set.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_MUSIC,
    WTOK_SFX,
    WTOK_BACK,
};

ConfigSound::ConfigSound()
{
    m_menu_id = widgetSet -> vstack(0);
    widgetSet -> label(m_menu_id, _("Sound Settings"), GUI_LRG, GUI_ALL, 0, 0);

    const int VA = widgetSet -> varray(m_menu_id);
    // The spaces are important, otherwise the set_label calls below will
    // increase the width of the container, resulting in a warning being
    // printed by widgetSet.
    if( user_config->doMusic() )
        m_music_menu_id = widgetSet->start(VA,_("  Turn off music  "), 
                                           GUI_MED, WTOK_MUSIC, GUI_ON);
    else
        m_music_menu_id = widgetSet->start(VA,_("  Turn on music  "), 
                                           GUI_MED, WTOK_MUSIC, GUI_OFF);

    if( user_config->doSFX() )
        m_sfx_menu_id   = widgetSet->state(VA,_("  Turn off sound effects  "),
                                           GUI_MED, WTOK_SFX, GUI_ON);
    else
        m_sfx_menu_id   = widgetSet->state(VA,_("  Turn on sound effects  "),
                                           GUI_MED, WTOK_SFX, GUI_OFF);

    widgetSet -> space(VA);
    widgetSet -> state(VA, _("Press <ESC> to go back"), GUI_SML, WTOK_BACK);
    widgetSet -> layout(m_menu_id, 0, 0);
}

//-----------------------------------------------------------------------------
ConfigSound::~ConfigSound()
{
    widgetSet -> delete_widget(m_menu_id) ;
}

//-----------------------------------------------------------------------------
void ConfigSound::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
#if 0
    // This menu can be triggered from the game, when it is paused
    // so we have to check it and draw it as in pause
    if(widgetSet -> get_paused()) widgetSet -> blank() ;
#endif

    widgetSet -> paint(m_menu_id) ;
}

//-----------------------------------------------------------------------------
void ConfigSound::select()
{
    switch ( widgetSet -> get_token (widgetSet -> click()) )
    {
    case WTOK_MUSIC:
        if(user_config->doMusic())
        {
            user_config->setMusic(UserConfig::UC_DISABLE);
            widgetSet->set_label(m_music_menu_id, _("Turn on music"));
        }
        else
        {
            user_config->setMusic(UserConfig::UC_ENABLE);
            widgetSet->set_label(m_music_menu_id, _("Turn off music"));
        }
        widgetSet->toggle(m_music_menu_id);
        break;
    case WTOK_SFX:
        if(user_config->doSFX())
        {
            user_config->setSFX(UserConfig::UC_DISABLE);
            widgetSet->set_label(m_sfx_menu_id, _("Turn on sound effects"));
        }
        else
        {
            user_config->setSFX(UserConfig::UC_ENABLE);
            widgetSet->set_label(m_sfx_menu_id, _("Turn off sound effects"));
        }
        widgetSet->toggle(m_sfx_menu_id);
        break;
    case WTOK_BACK:
        menu_manager->popMenu();
        break;
    default: break;
    }
}



