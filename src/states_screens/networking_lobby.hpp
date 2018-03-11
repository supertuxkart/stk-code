//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_NETWORKING_LOBBY_HPP
#define HEADER_NETWORKING_LOBBY_HPP

#include "guiengine/screen.hpp"
#include <memory>
#include <tuple>

class Server;

namespace GUIEngine
{ 
    class Widget;
    class LabelWidget;
    class ListWidget;
    class IconButtonWidget;
    class TextBoxWidget;
}

/**
  * \brief Handles the networking lobby
  * \ingroup states_screens
  */
class NetworkingLobby : public GUIEngine::Screen,
                        public GUIEngine::ScreenSingleton<NetworkingLobby>
{
private:
    friend class GUIEngine::ScreenSingleton<NetworkingLobby>;

    NetworkingLobby();

    std::shared_ptr<Server> m_joined_server;
    std::vector<core::stringw> m_server_info;

    GUIEngine::IconButtonWidget * m_back_widget;
    GUIEngine::LabelWidget * m_text_bubble;
    GUIEngine::IconButtonWidget * m_exit_widget;
    GUIEngine::IconButtonWidget *m_start_button;
    GUIEngine::ListWidget *m_player_list;
    GUIEngine::TextBoxWidget* m_chat_box;

public:

    virtual void onUpdate(float delta) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual bool onEscapePressed() OVERRIDE;

    /** Used to insert each client chat message (reserved). */
    void addMoreServerInfo(const core::stringw& info);
    void setJoinedServer(std::shared_ptr<Server> server);
    void addPlayer(const std::tuple<uint32_t/*host id*/, uint32_t/*online id*/,
                   core::stringw/*player name*/, bool/*is server owner*/>& p);
    void cleanPlayers();
};   // class NetworkingLobby

#endif
