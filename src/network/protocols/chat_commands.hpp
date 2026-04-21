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
    // ----------------------------------------------------------------------------------------
    /** The list of standard command answers.
     * If new commands are added and backward compatibility is desired,
     * the new commands should be inserted at the end to not change the value of the others.
     * A client-capability check should also be used. */
    enum CommandAnswers
    {
        CA_CHAT,                          // Use the chat protocol for this answer
        CA_UNKNOWN,                       // The requested command is unknown
        // Start of the help command answers
        CA_HELP_HELP,
        CA_HELP_HELP_EXTRA,
        CA_HELP_SPECTATE,
        CA_HELP_SPECTATE_EXTRA,
        CA_HELP_LISTADDONS,
        CA_HELP_LISTADDONS_EXTRA,
        CA_HELP_PLAYER_HAS_ADDON,
        CA_HELP_PLAYER_HAS_ADDON_EXTRA,
        CA_HELP_SERVER_HAS_ADDON,
        CA_HELP_SERVER_HAS_ADDON_EXTRA,
        CA_HELP_ADDON_SCORE,
        CA_HELP_ADDON_SCORE_EXTRA,
        CA_HELP_KICK,
        CA_HELP_KICK_EXTRA,
        CA_HELP_MUTE,
        CA_HELP_MUTE_EXTRA,
        CA_HELP_UNMUTE,
        CA_HELP_UNMUTE_EXTRA,
        CA_HELP_LISTMUTE,
        CA_HELP_LISTMUTE_EXTRA,
        // End of the help command answers
        CA_SPECTATE_UNAVAILABLE,
        CA_SPECTATE_GUI_HOST,
        CA_SPECTATE_ACTIVATED,
        CA_SPECTATE_DISABLED,
        CA_LISTADDONS_NOT_FOUND,
        CA_LISTADDONS_FOUND,
        CA_PLAYER_HAS_ADDON_NOT_FOUND,
        CA_PLAYER_HAS_ADDON_FOUND,
        CA_SERVER_HAS_ADDON_NOT_FOUND,
        CA_SERVER_HAS_ADDON_FOUND,
        CA_ADDON_SCORE,
        CA_KICK_NO_RIGHTS,
        CA_KICK_SUCCESS,
        CA_MUTE_ALREADY_MUTED,
        CA_MUTE_SUCCESS,
        CA_UNMUTE_NOT_MUTED,
        CA_UNMUTE_SUCCESS,
        CA_LISTMUTE_NO_MUTE,
        CA_LISTMUTE_FOUND
    };


    void answerCommand(CommandAnswers command_id, std::shared_ptr<STKPeer> peer, std::string args = "");
    irr::core::stringw getAnswerString(CommandAnswers command_id, std::string args);
    void handleServerCommand(ServerLobby* lobby, Event* event, std::shared_ptr<STKPeer> peer);

    void help(std::string cmd, std::shared_ptr<STKPeer> peer);
    void helpMessage(std::string cmd_name, std::shared_ptr<STKPeer> peer, bool extra_info = false);
    void spectate(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void listServerAddons(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void playerHasAddon(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void serverHasAddon(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void playerAddonScore(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void kick(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void mute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void unmute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
    void listMute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer);
}

#endif // CHAT_COMMANDS_HPP
