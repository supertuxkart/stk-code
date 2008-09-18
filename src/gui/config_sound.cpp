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

#include "config_sound.hpp"
#include "widget_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "audio/sound_manager.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_MUSIC,
    WTOK_SFX,

    WTOK_QUIT,
};

ConfigSound::ConfigSound()
{
    widget_manager->switchOrder();
    widget_manager->addTitleWgt(WTOK_TITLE, 50, 7, _("Sound Settings"));

    widget_manager->setInitialActivationState(true);
    if( user_config->doMusic() )
    {
        widget_manager->addTextButtonWgt( WTOK_MUSIC, 50, 7, _("Turn off music"));
    }
    else
    {
        widget_manager->addTextButtonWgt( WTOK_MUSIC, 50, 7, _("Turn on music"));
    }

    if( user_config->doSFX() )
    {
        widget_manager->addTextButtonWgt( WTOK_SFX, 50, 7, _("Turn off sound effects"));
    }
    else
    {
        widget_manager->addTextButtonWgt( WTOK_SFX, 50, 7, _("Turn on sound effects"));
    }

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 5);

    widget_manager->addTextButtonWgt(WTOK_QUIT, 50, 7,_("Press <ESC> to go back"));
    widget_manager->setWgtTextSize(WTOK_QUIT, WGT_FNT_SML);

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
ConfigSound::~ConfigSound()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void ConfigSound::select()
{
    switch ( widget_manager->getSelectedWgt())
    {
    case WTOK_MUSIC:
        if(user_config->doMusic())
        {
            user_config->setMusic(UserConfig::UC_DISABLE);
            widget_manager->setWgtText(WTOK_MUSIC, _("Turn on music"));
            sound_manager->stopMusic();
        }
        else
        {
            user_config->setMusic(UserConfig::UC_ENABLE);
            widget_manager->setWgtText(WTOK_MUSIC, _("Turn off music"));
            sound_manager->startMusic(sound_manager->getCurrentMusic());
        }
        break;
    case WTOK_SFX:
        if(user_config->doSFX())
        {
            user_config->setSFX(UserConfig::UC_DISABLE);
            widget_manager->setWgtText(WTOK_SFX, _("Turn on sound effects"));
        }
        else
        {
            user_config->setSFX(UserConfig::UC_ENABLE);
            widget_manager->setWgtText(WTOK_SFX, _("Turn off sound effects"));
        }
        break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }
}



