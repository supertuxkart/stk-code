//  $Id: network_gui.cpp 2148 2008-07-03 03:14:27Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008  Joerg Henrichs

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

#include <sstream>

#include <SDL/SDL.h>

#include "widget_manager.hpp"
#include "network_gui.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "sdldrv.hpp"
#include "user_config.hpp"
#include "network/network_manager.hpp"
#include "utils/string_utils.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_CONNECT,
    WTOK_SERVER,
    WTOK_SERVER_ADDRESS,
    WTOK_CONNECTED,
    WTOK_MESSAGE,
    WTOK_QUIT
};

/** Limits the maximum length of the player name. */
const int NetworkGUI::SERVER_NAME_MAX = 50;


NetworkGUI::NetworkGUI()
{
    m_num_clients = 0;
    Widget *w=widget_manager->addTitleWgt( WTOK_TITLE, 60, 7, _("Select network mode"));
    widget_manager->hideWgtRect(WTOK_TITLE);
    w->setPosition(WGT_DIR_CENTER, 0.0, WGT_DIR_FROM_TOP, 0.15f);

    Widget *w_prev=widget_manager->addTextButtonWgt( WTOK_CONNECT, 30, 7, _("Connect to server") );
    widget_manager->resizeWgtToText(WTOK_CONNECT);
    w_prev->setPosition(WGT_DIR_FROM_LEFT, 0.05f, WGT_DIR_FROM_TOP, 0.25f);

    w=widget_manager->addTextButtonWgt( WTOK_SERVER, 30, 7, _("Become server") );
    widget_manager->resizeWgtToText(WTOK_SERVER);
    w->setPosition(WGT_DIR_FROM_LEFT, 0.05f, NULL, 
                   WGT_DIR_UNDER_WIDGET, 0, w_prev);
    widget_manager->sameWidth(WTOK_CONNECT, WTOK_SERVER);

    std::ostringstream s;
    s<<user_config->m_server_address<<":"<<user_config->m_server_port;
    m_server_address=s.str();
    w=widget_manager->addTextButtonWgt(WTOK_SERVER_ADDRESS, 50, 7, m_server_address);
    w->setPosition(WGT_DIR_RIGHT_WIDGET, 0.05f, w_prev,
                   WGT_DIR_FROM_TOP, 0.25f, NULL);

    w=widget_manager->addTextButtonWgt(WTOK_CONNECTED, 50, 20, " ");
    w->setPosition(WGT_DIR_RIGHT_WIDGET, 0.05f, w_prev,
                   WGT_DIR_FROM_TOP, 0.25f, NULL);
    widget_manager->hideWgt(WTOK_CONNECTED);
    widget_manager->deactivateWgt(WTOK_CONNECTED);

    w=widget_manager->addTextButtonWgt( WTOK_QUIT, 60, 7, _("Press <ESC> to go back") );
    w->setPosition(WGT_DIR_CENTER, 0, WGT_DIR_FROM_BOTTOM, 0);

    w=widget_manager->addTextWgt(WTOK_MESSAGE, 30, 7, "");
    w->setPosition(WGT_DIR_CENTER, 0, WGT_DIR_CENTER, 0);
    widget_manager->hideWgt(WTOK_MESSAGE);

    // This can happen either when going back here, or when a command line
    // option was specified causing the connection to already have happened
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
        switchToWaitForConnectionMode();
    widget_manager->layout(WGT_AREA_ALL);
    m_state=NGS_NONE;

}   // NetworkGUI

//-----------------------------------------------------------------------------
NetworkGUI::~NetworkGUI()
{
    widget_manager->reset();
}   // ~NetworkGUI

//-----------------------------------------------------------------------------
void NetworkGUI::select()
{
    const int selected = widget_manager->getSelectedWgt();
    switch (selected)
    {
        case WTOK_SERVER_ADDRESS:
            // Switch to typing in the address of the server
            widget_manager->setWgtText(WTOK_SERVER_ADDRESS, (m_server_address + "<").c_str());
            inputDriver->setMode(SDLDriver::LOWLEVEL);
            break;
        case WTOK_CONNECT:
            // If we could connect here, no message could be displayed since
            // glflush isn't called. So we only set a message for the network
            // manager to display, and set the state so that the actual
            // connection is done later when updating.
            m_state=NGS_CONNECT_DISPLAY;
            widget_manager->setWgtText(WTOK_MESSAGE, _("Waiting for server"));
            widget_manager->resizeWgtToText(WTOK_MESSAGE);
            widget_manager->showWgt(WTOK_MESSAGE);
            widget_manager->hideWgt(WTOK_CONNECT, WTOK_SERVER_ADDRESS);
            widget_manager->hideWgt(WTOK_QUIT);
            break;
        case WTOK_SERVER:
            network_manager->becomeServer();
            widget_manager->hideWgt(WTOK_MESSAGE);
            widget_manager->resizeWgtToText(WTOK_MESSAGE);
            widget_manager->showWgt(WTOK_MESSAGE);
            // Initialising the server does not block, so we don't have to
            // do this in the update loop (to enable updates of the display).
            if(!network_manager->initialiseConnections())
            {
                fprintf(stderr, "Problems initialising network connections,\n"
                    "Running in non-network mode.\n");
            }
            switchToWaitForConnectionMode();
            break;
        case WTOK_QUIT:
            // Disable accepting of clients
            if(network_manager->getMode()==NetworkManager::NW_SERVER)
                network_manager->setState(NetworkManager::NS_MAIN_MENU);
            // Don't do networking if no clients are connected
            if(network_manager->getNumClients()==0)
                network_manager->disableNetworking();
            // Leave menu.
            menu_manager->popMenu();

            break;
    }
}   // select

//-----------------------------------------------------------------------------
void NetworkGUI::switchToWaitForConnectionMode()
{
    widget_manager->setWgtText(WTOK_CONNECT, _("Connected:"));
    widget_manager->hideWgtRect(WTOK_CONNECT);
    widget_manager->deactivateWgt(WTOK_CONNECT); //make it non-selectable
    widget_manager->setWgtText(WTOK_QUIT, _("OK"));
    widget_manager->hideWgt(WTOK_SERVER_ADDRESS);
    widget_manager->showWgt(WTOK_CONNECTED);
    widget_manager->hideWgt(WTOK_SERVER);
    widget_manager->setWgtText(WTOK_TITLE,_("Waiting for clients"));
    widget_manager->hideWgt(WTOK_MESSAGE);
    m_num_clients = 0;
}   // switchToWaitForConnectionMode

//-----------------------------------------------------------------------------
void NetworkGUI::update(float dt)
{
    // We need one 'in between' frame (finite state machine goes from 
    // NGS_CONNECT_DISPLAY to NGS_CONNECT_DOIT) since otherwise the text
    // set for the message widget is not displayed (since glFlush isn't
    // called before the blocking initialiseConnection call).
    if(m_state==NGS_CONNECT_DOIT)
    {
        network_manager->becomeClient();
        if(!network_manager->initialiseConnections())
        {
            widget_manager->setWgtText(WTOK_MESSAGE, _("Can't connect to server"));
            widget_manager->resizeWgtToText(WTOK_MESSAGE);
            widget_manager->showWgt(WTOK_QUIT);
            network_manager->disableNetworking();
        }
        else
        {
            network_manager->sendConnectMessage();
            menu_manager->popMenu();
        }
        m_state=NGS_NONE;
    }
    else if(m_state==NGS_CONNECT_DISPLAY)
        m_state=NGS_CONNECT_DOIT;

    widget_manager->update(0.0f);
    if(m_num_clients==network_manager->getNumClients()) return;

    // At least one new client has connected:
    std::string s="";
    m_num_clients = network_manager->getNumClients();
    for(unsigned int i=1; i<=m_num_clients; i++)
    {
        s+=network_manager->getClientName(i)+"\n";
    }
    widget_manager->setWgtText(WTOK_CONNECTED, s);
}   // update

//-----------------------------------------------------------------------------
void NetworkGUI::inputKeyboard(SDLKey key, int unicode)
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
        if (m_server_address.size() >=1)
            m_server_address.erase(m_server_address.size()-1, 1);
        
        widget_manager->setWgtText(WTOK_SERVER_ADDRESS, (m_server_address + "<").c_str());
        break;
        break;
    default:
        // Adds the character to the name.
        // For this menu only unicode translation is enabled.
        // So we use the unicode character here, since this will
        // take care of upper/lower case etc.
      if (unicode && (int)m_server_address.size() <= SERVER_NAME_MAX)
            m_server_address += (char) unicode;
        widget_manager->setWgtText(WTOK_SERVER_ADDRESS, (m_server_address + "<").c_str());
        break;
    }

}
//-----------------------------------------------------------------------------
void NetworkGUI::handle(GameAction ga, int value)
{
    if (value)
        return;
    
    switch (ga)
    {
        case GA_ENTER:
            // If the user is typing her name this will be finished at this
            // point.
            if (inputDriver->isInMode(SDLDriver::LOWLEVEL))
            {
                // Prevents zero-length names.
                if (m_server_address.length() == 0)
                    m_server_address = "localhost:2305";
                std::vector<std::string> sl=StringUtils::split(m_server_address,':');
                if(sl.size()>1)
                {
                    user_config->m_server_address = sl[0];
                    user_config->m_server_port    = atoi(sl[1].c_str());
                }
                else
                {
                    user_config->m_server_address = m_server_address;
                    std::ostringstream s;
                    s<<m_server_address<<":"<<user_config->m_server_port;
                    m_server_address=s.str();
                }
                widget_manager->setWgtText(WTOK_SERVER_ADDRESS, m_server_address.c_str());

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
                std::ostringstream s;
                s<<user_config->m_server_address<<":"<<user_config->m_server_port;
                m_server_address=s.str();
                widget_manager->setWgtText(WTOK_SERVER_ADDRESS, m_server_address.c_str());

                inputDriver->setMode(SDLDriver::MENU);
                break;
            }
            // Fall through to reach the usual GA_LEAVE code (leave menu).
        default:
            BaseGUI::handle(ga, value);
    }
    
}
