//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2025 SuperTuxKart-Team
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

#ifndef CHAT_COMMANDS_HPP
#define CHAT_COMMANDS_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "network/protocols/server_lobby.hpp"
#include "utils/cpp2011.hpp"
#include "utils/time.hpp"

#include "irrString.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>

class BareNetworkString;
class DatabaseConnector;
class NetworkItemManager;
class NetworkString;
class NetworkPlayerProfile;
class STKPeer;
class SocketAddress;
class Ranking;

namespace ChatCommands
{
    void answerCommand(std::string msg, std::shared_ptr<STKPeer> peer);
    void handleServerCommand(ServerLobby* lobby, Event* event, std::shared_ptr<STKPeer> peer);
}

#endif // CHAT_COMMANDS_HPP