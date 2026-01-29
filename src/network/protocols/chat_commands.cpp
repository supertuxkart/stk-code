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
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

namespace ChatCommands
{
    // ----------------------------------------------------------------------------------------
    /* This function answers a player's chat command. It supports 2 types of answers:
    * - Sending a command ID with arguments, for standard commands. This allows
    *   to support localization of answer messages without client-side string-analysis.
    * - Sending the answer as a normal chat message. This is used with compatibilities
    *   for older clients and for custom commands (i.e. for servers using custom commands) */
    void answerCommand(CommandAnswers command_id, std::shared_ptr<STKPeer> peer, std::string args)
    {
        NetworkString* answer = ProtocolUtils::getNetworkString(ProtocolType::PROTOCOL_LOBBY_ROOM);

        if (command_id != CA_CHAT &&
            peer->getClientCapabilities().find("command_messages") !=
            peer->getClientCapabilities().end())
        {
            answer->addUInt8(LobbyProtocol::LE_COMMAND_ANSWER);
            answer->addUInt16((uint16_t) command_id);
            answer->setSynchronous(true);
            answer->encodeString16(StringUtils::utf8ToWide(args));
        }
        else
        {
            answer->addUInt8(LobbyProtocol::LE_CHAT);
            answer->setSynchronous(true);
            answer->encodeString16(getAnswerString(command_id, args));
        }
        peer->sendPacket(answer, true/*reliable*/);
        delete answer;
    }   // answerCommand

    // ----------------------------------------------------------------------------------------
    irr::core::stringw getAnswerString(CommandAnswers command_id, std::string args)
    {
        irr::core::stringw msg = "";
        auto argv = StringUtils::split(args, ' ');
        switch(command_id)
        {
            // In the case of CA_CHAT, the string argument contains the full message
            case CA_CHAT:
                return StringUtils::utf8ToWide(args);
            case CA_UNKNOWN:
                return _("Unknown command: %s. Use /help to list the available commands",
                        StringUtils::utf8ToWide(args));

            // Start of the help commands
            case CA_HELP_HELP:
                return _("To see the usage format of a command, "
                "use /help [command], for example '/help mute' gives "
                "information about the /mute command.");
            case CA_HELP_HELP_EXTRA:
                return _("This server supports the following commands: /help "
                "/spectate /listserveraddon /playerhasaddon /serverhasaddon "
                "/playeraddonscore /kick /mute /unmute /listmute");
            case CA_HELP_SPECTATE_EXTRA:
                return _("This command allows to enable or disable spectator mode.");
            case CA_HELP_SPECTATE:
                return _("Usage: /spectate [0 or 1], before the game starts");
            case CA_HELP_LISTADDONS_EXTRA:
                return _("This command allows to find all addons available on this server "
                        "that contain the searched string in their name.");
            case CA_HELP_LISTADDONS:
                return _("Usage: /listserveraddon [option] [addon string to find "
                        "(at least 3 characters)]. Available options: "
                        "-track, -arena, -kart, -soccer.");
            case CA_HELP_PLAYER_HAS_ADDON_EXTRA:
                return _("This command allows to check if a player has a specific addon.");
            case CA_HELP_PLAYER_HAS_ADDON:
                return _("Usage: /playerhasaddon [addon_identity] [player name]");
            case CA_HELP_SERVER_HAS_ADDON_EXTRA:
                return _("This command allows to check if the server has a specific addon.");
            case CA_HELP_SERVER_HAS_ADDON:
                return _("Usage: /serverhasaddon [addon_identity]");
            case CA_HELP_ADDON_SCORE_EXTRA:
                return _("This command tells what share of the addons available "
                        "in this server a player has installed.");
            case CA_HELP_ADDON_SCORE:
                return _("Usage: /playeraddonscore [player name]");
            case CA_HELP_KICK_EXTRA:
                return _("This command kicks the named player out of the server, "
                        "if you have the rights to do so.");
            case CA_HELP_KICK:
                return _("Usage: /kick [player name]");
            case CA_HELP_MUTE_EXTRA:
                return _("This command prevents you from seeing chat messages from the selected player.");
            case CA_HELP_MUTE:
                return _("Usage: /mute player_name (not including yourself)");
            case CA_HELP_UNMUTE_EXTRA:
                return _("This command enables reading chat messages from a previously muted player.");
            case CA_HELP_UNMUTE:
                return _("Usage: /unmute player_name");
            case CA_HELP_LISTMUTE_EXTRA:
                return _("This command gives you the list of all players you have muted.");
            case CA_HELP_LISTMUTE:
                return _("Usage: /listmute");
            // End of the help commands

            case CA_SPECTATE_UNAVAILABLE:
                return _("Spectating is not possible on this server.");
            case CA_SPECTATE_GUI_HOST:
                return _("Spectating is not possible for the host of a GUI server.");
            case CA_SPECTATE_ACTIVATED:
                return _("You have activated spectating mode.");
            case CA_SPECTATE_DISABLED:
                return _("You have disabled spectating mode.");
            case CA_LISTADDONS_NOT_FOUND:
                return _("No addon with an internal name containing '%s' has been found on this server.",
                        StringUtils::utf8ToWide(args));
            case CA_LISTADDONS_FOUND:
                return _P("This server has the following addon matching your query: %s",
                        "This server has the following addons matching your query: %s",
                        /* to pick the plural form */ argv.size(), StringUtils::utf8ToWide(args));
            case CA_PLAYER_HAS_ADDON_NOT_FOUND:
                if (argv.size() < 2)
                {
                    Log::error("ChatCommands","Missing arguments for CA_PLAYER_HAS_ADDON_NOT_FOUND");
                    return msg;
                }
                return _("%s doesn't have the addon '%s'", StringUtils::utf8ToWide(argv[0]),
                        StringUtils::utf8ToWide(argv[1]));
            case CA_PLAYER_HAS_ADDON_FOUND:
                if (argv.size() < 2)
                {
                    Log::error("ChatCommands","Missing arguments for CA_PLAYER_HAS_ADDON_FOUND");
                    return msg;
                }
                return _("%s has the addon '%s'", StringUtils::utf8ToWide(argv[0]),
                        StringUtils::utf8ToWide(argv[1]));
            case CA_SERVER_HAS_ADDON_NOT_FOUND:
                return _("This server doesn't have the addon '%s'", StringUtils::utf8ToWide(args));
            case CA_SERVER_HAS_ADDON_FOUND:
                return _("This server has the addon '%s'", StringUtils::utf8ToWide(args));
            case CA_ADDON_SCORE:
                if (argv.size() < 5)
                {
                    Log::error("ChatCommands","Missing arguments for CA_ADDON_SCORE");
                    return msg;
                }
                return _("Of the addons available in this server, %s has installed %s of the karts, "
                        "%s of the tracks, %s of the arenas and %s of the soccer fields.",
                        StringUtils::utf8ToWide(argv[0]), StringUtils::utf8ToWide(argv[1]),
                        StringUtils::utf8ToWide(argv[2]), StringUtils::utf8ToWide(argv[3]),
                        StringUtils::utf8ToWide(argv[4]));
            case CA_KICK_NO_RIGHTS:
                return _("You don't have the rights to kick other players out of this server.");
            case CA_KICK_SUCCESS:
                return _("You have kicked %s out of this server.", StringUtils::utf8ToWide(args));
            case CA_MUTE_ALREADY_MUTED:
                return _("Player %s is already muted.", StringUtils::utf8ToWide(args));
            case CA_MUTE_SUCCESS:
                return _("Player %s has been muted.", StringUtils::utf8ToWide(args));
            case CA_UNMUTE_NOT_MUTED:
                return _("Player %s is already unmuted.", StringUtils::utf8ToWide(args));
            case CA_UNMUTE_SUCCESS:
                return _("Player %s has been unmuted.", StringUtils::utf8ToWide(args));
            case CA_LISTMUTE_NO_MUTE:
                return _("You have not muted any player.");
            case CA_LISTMUTE_FOUND:
                return _P("You have muted %s",
                        "You have muted the following players: %s",
                        /* to pick the plural form */ argv.size(), StringUtils::utf8ToWide(args));
            default:
                return _("Unknown command answer! The server is probably misconfigured.");
        }
        return msg;
    }   // getAnswerString

    // ----------------------------------------------------------------------------------------
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
            answerCommand(CA_UNKNOWN, peer, argv[0]);
    }   // handleServerCommand

    // ----------------------------------------------------------------------------------------
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

    // ----------------------------------------------------------------------------------------
    void helpMessage(std::string cmd_name, std::shared_ptr<STKPeer> peer, bool extra_info)
    {
        if (cmd_name == "help")
        {
            answerCommand(CA_HELP_HELP, peer);
            answerCommand(CA_HELP_HELP_EXTRA, peer);
            return;
        }

        // These messages explain the purpose of the command
        if (extra_info)
        {
            if (cmd_name == "spectate")
                answerCommand(CA_HELP_SPECTATE_EXTRA, peer);
            else if (cmd_name == "listserveraddon")
                answerCommand(CA_HELP_LISTADDONS_EXTRA, peer);
            else if (cmd_name == "playerhasaddon")
                answerCommand(CA_HELP_PLAYER_HAS_ADDON_EXTRA, peer);
            else if (cmd_name == "serverhasaddon")
                answerCommand(CA_HELP_SERVER_HAS_ADDON_EXTRA, peer);
            else if (cmd_name == "playeraddonscore")
                answerCommand(CA_HELP_ADDON_SCORE_EXTRA, peer);
            else if (cmd_name == "kick")
                answerCommand(CA_HELP_KICK_EXTRA, peer);
            else if (cmd_name == "mute")
                answerCommand(CA_HELP_MUTE_EXTRA, peer);
            else if (cmd_name == "unmute")
                answerCommand(CA_HELP_UNMUTE_EXTRA, peer);
            else if (cmd_name == "listmute")
                answerCommand(CA_HELP_LISTMUTE_EXTRA, peer);
        }

        // These messages explain the format to use the command
        if (cmd_name == "spectate")
            answerCommand(CA_HELP_SPECTATE, peer);
        else if (cmd_name == "listserveraddon")
            answerCommand(CA_HELP_LISTADDONS, peer);
        else if (cmd_name == "playerhasaddon")
            answerCommand(CA_HELP_PLAYER_HAS_ADDON, peer);
        else if (cmd_name == "serverhasaddon")
            answerCommand(CA_HELP_SERVER_HAS_ADDON, peer);
        else if (cmd_name == "playeraddonscore")
            answerCommand(CA_HELP_ADDON_SCORE, peer);
        else if (cmd_name == "kick")
            answerCommand(CA_HELP_KICK, peer);
        else if (cmd_name == "mute")
            answerCommand(CA_HELP_MUTE, peer);
        else if (cmd_name == "unmute")
            answerCommand(CA_HELP_UNMUTE, peer);
        else if (cmd_name == "listmute")
            answerCommand(CA_HELP_LISTMUTE, peer);
        else
            answerCommand(CA_UNKNOWN, peer, cmd_name);
    } // helpMessage

    // ----------------------------------------------------------------------------------------
    void spectate(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');
        if (lobby->getGameSetup()->isGrandPrix() || !ServerConfig::m_live_players)
        {
            answerCommand(CA_SPECTATE_UNAVAILABLE, peer);
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
                answerCommand(CA_SPECTATE_GUI_HOST, peer);
                return;
            }
            peer->setAlwaysSpectate(ASM_COMMAND);
            answerCommand(CA_SPECTATE_ACTIVATED, peer);
        }
        else
        {
            peer->setAlwaysSpectate(ASM_NONE);
            answerCommand(CA_SPECTATE_DISABLED, peer);
        }
        lobby->updatePlayerList();
    } // spectate

    // ----------------------------------------------------------------------------------------
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
                answerCommand(CA_LISTADDONS_NOT_FOUND, peer, text);
            }
            else
            {
                msg = msg.substr(0, msg.size() - 2);
                answerCommand(CA_LISTADDONS_FOUND, peer, msg);
            }
        }
    } // listServerAddons

    // ----------------------------------------------------------------------------------------
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
            std::string args = player_name + " " + addon_id;
            if (found)
                answerCommand(CA_PLAYER_HAS_ADDON_FOUND, peer, args);
            else
                answerCommand(CA_PLAYER_HAS_ADDON_NOT_FOUND, peer, args);
        }
    } // playerHasAddon

    // ----------------------------------------------------------------------------------------
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
            answerCommand(CA_SERVER_HAS_ADDON_FOUND, peer, argv[1]);
        else
            answerCommand(CA_SERVER_HAS_ADDON_NOT_FOUND, peer, argv[1]);
    } // serverHasAddon

    // ----------------------------------------------------------------------------------------
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

        std::string msg = player_name;
        msg += " " + StringUtils::toString((scores[AS_KART] == - 1)   ? 0 : scores[AS_KART])   + "%";
        msg += " " + StringUtils::toString((scores[AS_TRACK] == - 1)  ? 0 : scores[AS_TRACK])  + "%";
        msg += " " + StringUtils::toString((scores[AS_ARENA] == - 1)  ? 0 : scores[AS_ARENA])  + "%";
        msg += " " + StringUtils::toString((scores[AS_SOCCER] == - 1) ? 0 : scores[AS_SOCCER]) + "%";
        answerCommand(CA_ADDON_SCORE, peer, msg);
    } // playerAddonScore

    // ----------------------------------------------------------------------------------------
    void kick(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        if (lobby->m_server_owner.lock() != peer)
        {
            answerCommand(CA_KICK_NO_RIGHTS, peer);
            return;
        }
        std::string player_name;
        if (cmd.length() > 5)
            player_name = cmd.substr(5);
        std::shared_ptr<STKPeer> player_peer = STKHost::get()->findPeerByName(
            StringUtils::utf8ToWide(player_name));

        if (player_name.empty() || !player_peer || player_peer->isAIPeer())
        {
            helpMessage(argv[0], peer);
        }
        else
        {
            answerCommand(CA_KICK_SUCCESS, peer, player_name);
            player_peer->kick();
        }
    } // kick

    // ----------------------------------------------------------------------------------------
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

        // We check that the player is not already muted
        for (auto it = lobby->m_peers_muted_players[peer].begin();
            it != lobby->m_peers_muted_players[peer].end(); it++)
        {
            if (*it == player_name)
            {
                it = lobby->m_peers_muted_players[peer].erase(it);
                answerCommand(CA_MUTE_ALREADY_MUTED, peer, argv[1]);
                return;
            }
        }

        lobby->m_peers_muted_players[peer].insert(player_name);
        answerCommand(CA_MUTE_SUCCESS, peer, argv[1]);
        return;

mute_error:
        helpMessage(argv[0], peer);
    } // mute

    // ----------------------------------------------------------------------------------------
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
                answerCommand(CA_UNMUTE_SUCCESS, peer, argv[1]);
                return;
            }
        }

        player_peer = STKHost::get()->findPeerByName(player_name);
        if (player_peer)
        {
            answerCommand(CA_UNMUTE_NOT_MUTED, peer, argv[1]);
            return;
        }

unmute_error:
        helpMessage(argv[0], peer);
    } // unmute

    // ----------------------------------------------------------------------------------------
    void listMute(std::string cmd, ServerLobby* lobby, std::shared_ptr<STKPeer> peer)
    {
        auto argv = StringUtils::split(cmd, ' ');

        core::stringw total;
        for (auto& name : lobby->m_peers_muted_players[peer])
        {
            total += name;
            total += ", ";
        }

        if (total.empty())
        {
            answerCommand(CA_LISTMUTE_NO_MUTE, peer);
        }
        else
        {
            std::string msg = StringUtils::wideToUtf8(total);
            msg = msg.substr(0, msg.size() - 2); // Remove the extra comma
            answerCommand(CA_LISTMUTE_FOUND, peer, msg);
        }
    } // listMute
} // namespace
