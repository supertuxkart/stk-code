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

#include <SDL/SDL.h>

#include "network_info.hpp"
#include "widget_manager.hpp"
#include "translation.hpp"
#include "network/network_manager.hpp"
#include "stk_config.hpp"
#include "gui/menu_manager.hpp"

enum WidgetTokens
{
    WTOK_MSG,
    WTOK_CLIENT,
    WTOK_OK
};

NetworkInfo::NetworkInfo()
{
    //Add some feedback so people know they are going to start the race
    widget_manager->reset();
    Widget *w_prev=widget_manager->addTitleWgt( WTOK_MSG, 60, 7, _("Waiting for clients ...") );
    w_prev->setPosition(WGT_DIR_CENTER,   0.0,   NULL, 
                        WGT_DIR_FROM_TOP, 0.1f, NULL);

    // Display a single space to avoid warnings from the widget_manager
    Widget *w=widget_manager->addTextWgt(WTOK_CLIENT, 60, 10, " ");
    w->setPosition(WGT_DIR_CENTER,       0.0, NULL,
                   WGT_DIR_UNDER_WIDGET, 0.0, w_prev);
    w_prev = w;
    w=widget_manager->addTextButtonWgt(WTOK_OK, 60, 7, _("OK"));
    w->setPosition(WGT_DIR_CENTER,       0.0,  NULL,
                   WGT_DIR_UNDER_WIDGET, 0.1f, w_prev);
    widget_manager->layout(WGT_AREA_TOP);
    m_num_clients = 0;
}   // NetworkInfo

//-----------------------------------------------------------------------------
NetworkInfo::~NetworkInfo()
{
    widget_manager->reset();
}


//-----------------------------------------------------------------------------
void NetworkInfo::update(float DELTA)
{
    widget_manager->update(0.0f);
    if(m_num_clients==network_manager->getNumClients()) return;

    // At least one new client has connected:
    std::string s="";
    m_num_clients = network_manager->getNumClients();
    for(unsigned int i=1; i<=m_num_clients; i++)
    {
        s+=network_manager->getClientName(i)+"\n";
    }
    widget_manager->setWgtText(WTOK_CLIENT, s);

}   // update

//-----------------------------------------------------------------------------
void NetworkInfo::select()
{
    switch( widget_manager->getSelectedWgt() )
    {
        case WTOK_OK:
            network_manager->switchToCharacterSelection();
            menu_manager->popMenu();
            break;
    }
}   // select
