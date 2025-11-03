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

#include "network/protocols/chat_commands.hpp"

#include "addons/addon.hpp"
#include "config/user_config.hpp"
#include "items/network_item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/official_karts.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/linear_world.hpp"
#include "network/crypto.hpp"
#include "network/database_connector.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/peer_vote.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/chat_commands.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/protocols/ranking.hpp"
#include "network/race_event_manager.hpp"
#include "network/server_config.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "network/stk_ipv6.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "online/xml_request.hpp"
#include "race/race_manager.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

using namespace ProtocolUtils;

namespace ChatCommands
{
    //-----------------------------------------------------------------------------
    void handleServerCommand(ServerLobby* lobby, Event* event, std::shared_ptr<STKPeer> peer)
    {
        NetworkString& data = event->data();
        std::string language;
        data.decodeString(&language);
        std::string cmd;
        data.decodeString(&cmd);
        auto argv = StringUtils::split(cmd, ' ');
        if (argv.size() == 0)
            return;
        if (argv[0] == "spectate")
        {
            if (lobby->getGameSetup()->isGrandPrix() || !ServerConfig::m_live_players)
            {
                NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
                chat->addUInt8(LobbyProtocol::LE_CHAT);
                chat->setSynchronous(true);
                std::string msg = "Server doesn't support spectate";
                chat->encodeString16(StringUtils::utf8ToWide(msg));
                peer->sendPacket(chat, true/*reliable*/);
                delete chat;
                return;
            }
    
            if (argv.size() != 2 || (argv[1] != "0" && argv[1] != "1") ||
                lobby->getCurrentState() != ServerLobby::ServerState::WAITING_FOR_START_GAME)
            {
                NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
                chat->addUInt8(LobbyProtocol::LE_CHAT);
                chat->setSynchronous(true);
                std::string msg = "Usage: spectate [0 or 1], before game started";
                chat->encodeString16(StringUtils::utf8ToWide(msg));
                peer->sendPacket(chat, true/*reliable*/);
                delete chat;
                return;
            }
    
            if (argv[1] == "1")
            {
                if (lobby->getProcessType() == PT_CHILD &&
                    peer->getHostId() == lobby->getClientServerHostId())
                {
                    NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
                    chat->addUInt8(LobbyProtocol::LE_CHAT);
                    chat->setSynchronous(true);
                    std::string msg = "Graphical client server cannot spectate";
                    chat->encodeString16(StringUtils::utf8ToWide(msg));
                    peer->sendPacket(chat, true/*reliable*/);
                    delete chat;
                    return;
                }
                peer->setAlwaysSpectate(ASM_COMMAND);
            }
            else
                peer->setAlwaysSpectate(ASM_NONE);
            lobby->updatePlayerList();
        }
        else if (argv[0] == "listserveraddon")
        {
            NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            chat->addUInt8(LobbyProtocol::LE_CHAT);
            chat->setSynchronous(true);
            bool has_options = argv.size() > 1 &&
                (argv[1].compare("-track") == 0 ||
                argv[1].compare("-arena") == 0 ||
                argv[1].compare("-kart") == 0 ||
                argv[1].compare("-soccer") == 0);
            if (argv.size() == 1 || argv.size() > 3 || argv[1].size() < 3 ||
                (argv.size() == 2 && (argv[1].size() < 3 || has_options)) ||
                (argv.size() == 3 && (!has_options || argv[2].size() < 3)))
            {
                chat->encodeString16(
                    L"Usage: /listserveraddon [option][addon string to find "
                    "(at least 3 characters)]. Available options: "
                    "-track, -arena, -kart, -soccer.");
            }
            else
            {
                std::string type = "";
                std::string text = "";
                if(argv.size() > 1)
                {
                    if(argv[1].compare("-track") == 0 ||
                       argv[1].compare("-arena") == 0 ||
                       argv[1].compare("-kart" ) == 0 ||
                       argv[1].compare("-soccer" ) == 0)
                        type = argv[1].substr(1);
                    if((argv.size() == 2 && type.empty()) || argv.size() == 3)
                        text = argv[argv.size()-1];
                }
    
                std::set<std::string> total_addons;
                if(type.empty() || // addon type not specified
                   (!type.empty() && type.compare("kart") == 0)) // list kart addon
                {
                    total_addons.insert(lobby->m_addon_kts.first.begin(), lobby->m_addon_kts.first.end());
                }
                if(type.empty() || // addon type not specified
                   (!type.empty() && type.compare("track") == 0))
                {
                    total_addons.insert(lobby->m_addon_kts.second.begin(), lobby->m_addon_kts.second.end());
                }
                if(type.empty() || // addon type not specified
                   (!type.empty() && type.compare("arena") == 0))
                {
                    total_addons.insert(lobby->m_addon_arenas.begin(), lobby->m_addon_arenas.end());
                }
                if(type.empty() || // addon type not specified
                   (!type.empty() && type.compare("soccer") == 0))
                {
                    total_addons.insert(lobby->m_addon_soccers.begin(), lobby->m_addon_soccers.end());
                }
                std::string msg = "";
                for (auto& addon : total_addons)
                {
                    // addon_ (6 letters)
                    if (!text.empty() && addon.find(text, 6) == std::string::npos)
                        continue;
    
                    msg += addon.substr(6);
                    msg += ", ";
                }
                if (msg.empty())
                    chat->encodeString16(L"Addon not found");
                else
                {
                    msg = msg.substr(0, msg.size() - 2);
                    chat->encodeString16(StringUtils::utf8ToWide(
                        std::string("Server addon: ") + msg));
                }
            }
            peer->sendPacket(chat, true/*reliable*/);
            delete chat;
        }
        else if (StringUtils::startsWith(cmd, "playerhasaddon"))
        {
            NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            chat->addUInt8(LobbyProtocol::LE_CHAT);
            chat->setSynchronous(true);
            std::string part;
            if (cmd.length() > 15)
                part = cmd.substr(15);
            std::string addon_id = part.substr(0, part.find(' '));
            std::string player_name;
            if (part.length() > addon_id.length() + 1)
                player_name = part.substr(addon_id.length() + 1);
            std::shared_ptr<STKPeer> player_peer = STKHost::get()->findPeerByName(
                StringUtils::utf8ToWide(player_name));
            if (player_name.empty() || !player_peer || addon_id.empty())
            {
                chat->encodeString16(
                    L"Usage: /playerhasaddon [addon_identity] [player name]");
            }
            else
            {
                std::string addon_id_test = Addon::createAddonId(addon_id);
                bool found = false;
                const auto& kt = player_peer->getClientAssets();
                for (auto& kart : kt.first)
                {
                    if (kart == addon_id_test)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    for (auto& track : kt.second)
                    {
                        if (track == addon_id_test)
                        {
                            found = true;
                            break;
                        }
                    }
                }
                if (found)
                {
                    chat->encodeString16(StringUtils::utf8ToWide
                        (player_name + " has addon " + addon_id));
                }
                else
                {
                    chat->encodeString16(StringUtils::utf8ToWide
                        (player_name + " has no addon " + addon_id));
                }
            }
            peer->sendPacket(chat, true/*reliable*/);
            delete chat;
        }
        else if (StringUtils::startsWith(cmd, "kick"))
        {
            if (lobby->m_server_owner.lock() != peer)
            {
                NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
                chat->addUInt8(LobbyProtocol::LE_CHAT);
                chat->setSynchronous(true);
                chat->encodeString16(L"You are not server owner");
                peer->sendPacket(chat, true/*reliable*/);
                delete chat;
                return;
            }
            std::string player_name;
            if (cmd.length() > 5)
                player_name = cmd.substr(5);
            std::shared_ptr<STKPeer> player_peer = STKHost::get()->findPeerByName(
                StringUtils::utf8ToWide(player_name));
            if (player_name.empty() || !player_peer || player_peer->isAIPeer())
            {
                NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
                chat->addUInt8(LobbyProtocol::LE_CHAT);
                chat->setSynchronous(true);
                chat->encodeString16(
                    L"Usage: /kick [player name]");
                peer->sendPacket(chat, true/*reliable*/);
                delete chat;
            }
            else
            {
                player_peer->kick();
            }
        }
        else if (StringUtils::startsWith(cmd, "playeraddonscore"))
        {
            NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            chat->addUInt8(LobbyProtocol::LE_CHAT);
            chat->setSynchronous(true);
            std::string player_name;
            if (cmd.length() > 17)
                player_name = cmd.substr(17);
            std::shared_ptr<STKPeer> player_peer = STKHost::get()->findPeerByName(
                StringUtils::utf8ToWide(player_name));
            if (player_name.empty() || !player_peer)
            {
                chat->encodeString16(
                    L"Usage: /playeraddonscore [player name] (return 0-100)");
            }
            else
            {
                auto& scores = player_peer->getAddonsScores();
                if (scores[AS_KART] == -1 && scores[AS_TRACK] == -1 &&
                    scores[AS_ARENA] == -1 && scores[AS_SOCCER] == -1)
                {
                    chat->encodeString16(StringUtils::utf8ToWide
                        (player_name + " has no addon"));
                }
                else
                {
                    std::string msg = player_name;
                    msg += " addon:";
                    if (scores[AS_KART] != -1)
                        msg += " kart: " + StringUtils::toString(scores[AS_KART]) + ",";
                    if (scores[AS_TRACK] != -1)
                        msg += " track: " + StringUtils::toString(scores[AS_TRACK]) + ",";
                    if (scores[AS_ARENA] != -1)
                        msg += " arena: " + StringUtils::toString(scores[AS_ARENA]) + ",";
                    if (scores[AS_SOCCER] != -1)
                        msg += " soccer: " + StringUtils::toString(scores[AS_SOCCER]) + ",";
                    msg = msg.substr(0, msg.size() - 1);
                    chat->encodeString16(StringUtils::utf8ToWide(msg));
                }
            }
            peer->sendPacket(chat, true/*reliable*/);
            delete chat;
        }
        else if (argv[0] == "serverhasaddon")
        {
            NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            chat->addUInt8(LobbyProtocol::LE_CHAT);
            chat->setSynchronous(true);
            if (argv.size() != 2)
            {
                chat->encodeString16(
                    L"Usage: /serverhasaddon [addon_identity]");
            }
            else
            {
                std::set<std::string> total_addons;
                total_addons.insert(lobby->m_addon_kts.first.begin(), lobby->m_addon_kts.first.end());
                total_addons.insert(lobby->m_addon_kts.second.begin(), lobby->m_addon_kts.second.end());
                total_addons.insert(lobby->m_addon_arenas.begin(), lobby->m_addon_arenas.end());
                total_addons.insert(lobby->m_addon_soccers.begin(), lobby->m_addon_soccers.end());
                std::string addon_id_test = Addon::createAddonId(argv[1]);
                bool found = total_addons.find(addon_id_test) != total_addons.end();
                if (found)
                {
                    chat->encodeString16(StringUtils::utf8ToWide(std::string
                        ("Server has addon ") + argv[1]));
                }
                else
                {
                    chat->encodeString16(StringUtils::utf8ToWide(std::string
                        ("Server has no addon ") + argv[1]));
                }
            }
            peer->sendPacket(chat, true/*reliable*/);
            delete chat;
        }
        else if (argv[0] == "mute")
        {
            std::shared_ptr<STKPeer> player_peer;
            std::string result_msg;
            core::stringw player_name;
            NetworkString* result = NULL;
    
            if (argv.size() != 2 || argv[1].empty())
                goto mute_error;
    
            player_name = StringUtils::utf8ToWide(argv[1]);
            player_peer = STKHost::get()->findPeerByName(player_name);
    
            if (!player_peer || player_peer == peer)
                goto mute_error;
    
            lobby->m_peers_muted_players[peer].insert(player_name);
            result = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            result->addUInt8(LobbyProtocol::LE_CHAT);
            result->setSynchronous(true);
            result_msg = "Muted player ";
            result_msg += argv[1];
            result->encodeString16(StringUtils::utf8ToWide(result_msg));
            peer->sendPacket(result, true/*reliable*/);
            delete result;
            return;
    
    mute_error:
            NetworkString* error = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            error->addUInt8(LobbyProtocol::LE_CHAT);
            error->setSynchronous(true);
            std::string msg = "Usage: /mute player_name (not including yourself)";
            error->encodeString16(StringUtils::utf8ToWide(msg));
            peer->sendPacket(error, true/*reliable*/);
            delete error;
        }
        else if (argv[0] == "unmute")
        {
            std::shared_ptr<STKPeer> player_peer;
            std::string result_msg;
            core::stringw player_name;
            NetworkString* result = NULL;
    
            if (argv.size() != 2 || argv[1].empty())
                goto unmute_error;
    
            player_name = StringUtils::utf8ToWide(argv[1]);

            // We try to unmute, if possible
            for (auto it = lobby->m_peers_muted_players[peer].begin();
                it != lobby->m_peers_muted_players[peer].end(); it++)
            {
                if (*it == player_name)
                {
                    it = lobby->m_peers_muted_players[peer].erase(it);
                    goto unmute_success;
                }
            }
            goto unmute_error;

    unmute_success:
            result = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            result->addUInt8(LobbyProtocol::LE_CHAT);
            result->setSynchronous(true);
            result_msg = "Unmuted player ";
            result_msg += argv[1];
            result->encodeString16(StringUtils::utf8ToWide(result_msg));
            peer->sendPacket(result, true/*reliable*/);
            delete result;
            return;

    unmute_error:
            NetworkString* error = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            error->addUInt8(LobbyProtocol::LE_CHAT);
            error->setSynchronous(true);
            std::string msg = "Usage: /unmute player_name";
            error->encodeString16(StringUtils::utf8ToWide(msg));
            peer->sendPacket(error, true/*reliable*/);
            delete error;
        }
        else if (argv[0] == "listmute")
        {
            NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            chat->addUInt8(LobbyProtocol::LE_CHAT);
            chat->setSynchronous(true);
            core::stringw total;
            for (auto& name : lobby->m_peers_muted_players[peer])
            {
                total += name;
                total += " ";
            }
            if (total.empty())
                chat->encodeString16("No player has been muted by you");
            else
            {
                total += "muted";
                chat->encodeString16(total);
            }
            peer->sendPacket(chat, true/*reliable*/);
            delete chat;
        }
        else
        {
            NetworkString* chat = getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
            chat->addUInt8(LobbyProtocol::LE_CHAT);
            chat->setSynchronous(true);
            std::string msg = "Unknown command: ";
            msg += cmd;
            chat->encodeString16(StringUtils::utf8ToWide(msg));
            peer->sendPacket(chat, true/*reliable*/);
            delete chat;
        }
    }   // handleServerCommand
} // namespace