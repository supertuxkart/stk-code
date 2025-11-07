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
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/string_utils.hpp"

namespace ChatCommands
{
    //-----------------------------------------------------------------------------
    void answerCommand(std::string msg, std::shared_ptr<STKPeer> peer)
    {
        NetworkString* chat = ProtocolUtils::getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);
        chat->addUInt8(LobbyProtocol::LE_CHAT);
        chat->setSynchronous(true);
        chat->encodeString16(StringUtils::utf8ToWide(msg));
        peer->sendPacket(chat, true/*reliable*/);
        delete chat;
    }

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
        if (argv[0] == "help")
            help(cmd, peer);
        else if (argv[0] == "spectate")
            spectate(cmd, lobby, peer);
        else if (argv[0] == "listserveraddon")
            listServerAddons(cmd, lobby, peer);
        else if (argv[0] == "playerhasaddon")
            playerHasAddon(cmd, lobby, peer);
        else if (argv[0] == "serverhasaddon")
            serverHasAddon(cmd, lobby, peer);
        else if (argv[0] == "playeraddonscore")
            playerAddonScore(cmd, lobby, peer);
        else if (argv[0] == "kick")
            kick(cmd, lobby, peer);
        else if (argv[0] == "mute")
            mute(cmd, lobby, peer);
        else if (argv[0] == "unmute")
            unmute(cmd, lobby, peer);
        else if (argv[0] == "listmute")
            listMute(cmd, lobby, peer);
        else
            unknownCommand(cmd, peer);
    }   // handleServerCommand

    void help(std::string cmd, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');
        // A command name is supplied as an argument, give that command's help info
        if (argv.size() == 2)
            helpMessage(argv[1], peer, /* extra_info */ true);
        // Generic help message
        else
            helpMessage("help", peer);
    } // help

    void helpMessage(std::string cmd_name, std::shared_ptr<STKPeer> peer, bool extra_info)
    {
        if (cmd_name == "help")
        {
            std::string msg = "To see the usage format of a command, "
                "use /help [command], for example '/help mute' gives "
                "information about the /mute command.";
            answerCommand(msg, peer);
            msg = "This server supports the following commands: /help "
                "/spectate /listserveraddon /playerhasaddon /serverhasaddon "
                "/playeraddonscore /kick /mute /unmute /listmute";
            answerCommand(msg, peer);
            return;
        }

        if (cmd_name == "spectate")
            answerCommand("Usage: /spectate [0 or 1], before the game starts", peer);
        else if (cmd_name == "listserveraddon")
            answerCommand("Usage: /listserveraddon [option][addon string to find "
                "(at least 3 characters)]. Available options: "
                "-track, -arena, -kart, -soccer.", peer);
        else if (cmd_name == "playerhasaddon")
            answerCommand("Usage: /playerhasaddon [addon_identity] [player name]", peer);
        else if (cmd_name == "serverhasaddon")
            answerCommand("Usage: /serverhasaddon [addon_identity]", peer);
        else if (cmd_name == "playeraddonscore")
            answerCommand("Usage: /playeraddonscore [player name]", peer);
        else if (cmd_name == "kick")
            answerCommand("Usage: /kick [player name]", peer);
        else if (cmd_name == "mute")
            answerCommand("Usage: /mute player_name (not including yourself)", peer);
        else if (cmd_name == "unmute")
            answerCommand("Usage: /unmute player_name", peer);
        else if (cmd_name == "listmute")
            answerCommand("Usage: /listmute", peer);
        else
            unknownCommand(cmd_name, peer);

        if (extra_info)
        {
            if (cmd_name == "spectate")
                answerCommand("This command allows to enable or disable spectator mode.", peer);
            else if (cmd_name == "listserveraddon")
                answerCommand("This command allows to find all addons available on this server "
                    "that contain the searched string in their name.", peer);
            else if (cmd_name == "playerhasaddon")
                answerCommand("This command allows to check if a player has a specific addon.", peer);
            else if (cmd_name == "serverhasaddon")
                answerCommand("This command allows to check if the server has a specific addon.)", peer);
            else if (cmd_name == "playeraddonscore")
                answerCommand("This command returns a value between 0% and 100%.)", peer);
            else if (cmd_name == "kick")
                answerCommand("This command kicks the named player out of the server, "
                    "if you have the rights to do so.)", peer);
            else if (cmd_name == "mute")
                answerCommand("This command prevents you from seeing chat messages from the selected player.", peer);
            else if (cmd_name == "unmute")
                answerCommand("This command enables reading chat messages from a previously muted player.", peer);
            else if (cmd_name == "listmute")
                answerCommand("This command gives you the list of all players you have muted.", peer);
        }
    } // helpMessage

    void spectate(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');
        if (lobby->getGameSetup()->isGrandPrix() || !ServerConfig::m_live_players)
        {
            answerCommand("Spectating is not possible on this server.", peer);
            return;
        }
    
        if (argv.size() != 2 || (argv[1] != "0" && argv[1] != "1") ||
            lobby->getCurrentState() != ServerLobby::ServerState::WAITING_FOR_START_GAME)
        {
            helpMessage(argv[0], peer);
            return;
        }
    
        if (argv[1] == "1")
        {
            if (lobby->getProcessType() == PT_CHILD &&
                peer->getHostId() == lobby->getClientServerHostId())
            {
                answerCommand("Spectating is not possible for the host of a GUI server.", peer);
                return;
            }
            peer->setAlwaysSpectate(ASM_COMMAND);
        }
        else
            peer->setAlwaysSpectate(ASM_NONE);
        lobby->updatePlayerList();
    } // spectate

    void listServerAddons(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');
        bool has_options = argv.size() > 1 &&
            (argv[1].compare("-track") == 0 ||
            argv[1].compare("-arena") == 0 ||
            argv[1].compare("-kart") == 0 ||
            argv[1].compare("-soccer") == 0);
        if (argv.size() == 1 || argv.size() > 3 || argv[1].size() < 3 ||
            (argv.size() == 2 && (argv[1].size() < 3 || has_options)) ||
            (argv.size() == 3 && (!has_options || argv[2].size() < 3)))
        {
            helpMessage(argv[0], peer);
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
            {
                msg = "Addon not found";
            }
            else
            {
                msg = msg.substr(0, msg.size() - 2);
                msg = "Server addon: " + msg;
            }
            answerCommand(msg, peer);
        }
    } // listServerAddons

    void playerHasAddon(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');
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
            helpMessage(argv[0], peer);
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
            std::string msg = player_name;
            if (found)
                msg += " has addon " + addon_id;
            else
                msg += " has no addon " + addon_id;

            answerCommand(msg, peer);
        }
    } // playerHasAddon

    void serverHasAddon(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        if (argv.size() != 2)
        {
            helpMessage(argv[0], peer);
            return;
        }

        std::set<std::string> total_addons;
        total_addons.insert(lobby->m_addon_kts.first.begin(), lobby->m_addon_kts.first.end());
        total_addons.insert(lobby->m_addon_kts.second.begin(), lobby->m_addon_kts.second.end());
        total_addons.insert(lobby->m_addon_arenas.begin(), lobby->m_addon_arenas.end());
        total_addons.insert(lobby->m_addon_soccers.begin(), lobby->m_addon_soccers.end());
        std::string addon_id_test = Addon::createAddonId(argv[1]);
        bool found = total_addons.find(addon_id_test) != total_addons.end();

        if (found)
            answerCommand("Server has addon " + argv[1], peer);
        else
            answerCommand("Server has no addon " + argv[1], peer);
    } // serverHasAddon

    void playerAddonScore(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        std::string player_name;
        if (cmd.length() > 17)
            player_name = cmd.substr(17);
        std::shared_ptr<STKPeer> player_peer = STKHost::get()->findPeerByName(
            StringUtils::utf8ToWide(player_name));
        if (player_name.empty() || !player_peer)
        {
            helpMessage(argv[0], peer);
            return;
        }

        auto& scores = player_peer->getAddonsScores();
        if (scores[AS_KART] == -1 && scores[AS_TRACK] == -1 &&
            scores[AS_ARENA] == -1 && scores[AS_SOCCER] == -1)
        {
            answerCommand(player_name + " has no addon", peer);
            return;
        }

        std::string msg = player_name;
        msg += " addon:";
        if (scores[AS_KART] != -1)
            msg += " kart: " + StringUtils::toString(scores[AS_KART]) + "%,";
        if (scores[AS_TRACK] != -1)
            msg += " track: " + StringUtils::toString(scores[AS_TRACK]) + "%,";
        if (scores[AS_ARENA] != -1)
            msg += " arena: " + StringUtils::toString(scores[AS_ARENA]) + "%,";
        if (scores[AS_SOCCER] != -1)
            msg += " soccer: " + StringUtils::toString(scores[AS_SOCCER]) + "%,";
        msg = msg.substr(0, msg.size() - 1);
        answerCommand(msg, peer);
    } // playerAddonScore

    void kick(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        if (lobby->m_server_owner.lock() != peer)
        {
            answerCommand("You are not server owner.", peer);
            return;
        }
        std::string player_name;
        if (cmd.length() > 5)
            player_name = cmd.substr(5);
        std::shared_ptr<STKPeer> player_peer = STKHost::get()->findPeerByName(
            StringUtils::utf8ToWide(player_name));

        if (player_name.empty() || !player_peer || player_peer->isAIPeer())
            helpMessage(argv[0], peer);
        else
            player_peer->kick();
    } // kick

    void mute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        std::shared_ptr<STKPeer> player_peer;
        std::string result_msg;
        core::stringw player_name;

        if (argv.size() != 2 || argv[1].empty())
            goto mute_error;

        player_name = StringUtils::utf8ToWide(argv[1]);
        player_peer = STKHost::get()->findPeerByName(player_name);

        if (!player_peer || player_peer == peer)
            goto mute_error;

        lobby->m_peers_muted_players[peer].insert(player_name);
        result_msg = "Player " + argv[1] + " has been muted.";
        answerCommand(result_msg, peer);
        return;

mute_error:
        helpMessage(argv[0], peer);
    } // mute

    void unmute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        std::shared_ptr<STKPeer> player_peer;
        std::string result_msg;
        core::stringw player_name;

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
        result_msg = "Player " + argv[1] + " has been unmuted.";
        answerCommand(result_msg, peer);
        return;

unmute_error:
        helpMessage(argv[0], peer);
    } // unmute

    void listMute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        core::stringw total;
        for (auto& name : lobby->m_peers_muted_players[peer])
        {
            total += name;
            total += " ";
        }

        if (total.empty())
            total = "You have not muted any player.";
        else
            total += "muted";

        answerCommand(StringUtils::wideToUtf8(total), peer);
    } // listMute

    void unknownCommand(std::string cmd, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        std::string msg = "Unknown command: ";
        msg += cmd;
        answerCommand(msg, peer);
    } // unknownCommand
} // namespace