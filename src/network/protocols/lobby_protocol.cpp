//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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


#include "network/protocols/lobby_protocol.hpp"

#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/screen_keyboard.hpp"
#include <ge_render_info.hpp>
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/peer_vote.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"
#include "utils/translation.hpp"

std::weak_ptr<LobbyProtocol> LobbyProtocol::m_lobby[PT_COUNT];

LobbyProtocol::LobbyProtocol()
             : Protocol(PROTOCOL_LOBBY_ROOM),
               m_process_type(STKProcess::getType())
{
    resetGameStartedProgress();
    m_game_setup = new GameSetup();
    m_end_voting_period.store(0);
}   // LobbyProtocol

// ----------------------------------------------------------------------------
LobbyProtocol::~LobbyProtocol()
{
    RaceEventManager::destroy();
    delete m_game_setup;
    joinStartGameThread();
}   // ~LobbyProtocol

//-----------------------------------------------------------------------------
/** Starts the sychronization protocol and the RaceEventManager. It then
 *  sets the player structures up, creates the active player, and loads
 *  the world.
 *  This is called on the client when the server informs them that
 *  the world can be loaded (LE_LOAD_WORLD) and on the server in state
 *  LOAD_WORLD (i.e. just after informing all clients).
 */
void LobbyProtocol::loadWorld()
{
    Log::info("LobbyProtocol", "Ready !");
    // Race startup sequence
    // ---------------------
    // This creates the network world.
    auto gep = std::make_shared<GameEventsProtocol>();
    if (!RaceEventManager::get())
        RaceEventManager::create();
    RaceEventManager::get()->start(gep);

    // Make sure that if there is only a single local player this player can
    // use all input devices.
    StateManager::ActivePlayer *ap = RaceManager::get()->getNumLocalPlayers()>1
                                   ? NULL
                                   : StateManager::get()->getActivePlayer(0);
    // We only need to use input manager in main process
    if (m_process_type == PT_MAIN)
        input_manager->getDeviceManager()->setSinglePlayer(ap);

    // Load the actual world.
    m_game_setup->loadWorld();
    World::getWorld()->setNetworkWorld(true);
    GameProtocol::createInstance()->requestStart();
    gep->requestStart();

}   // loadWorld

// ----------------------------------------------------------------------------
void LobbyProtocol::configRemoteKart(
    const std::vector<std::shared_ptr<NetworkPlayerProfile> >& players,
    int local_player_size) const
{
    // The number of karts includes the AI karts, which are not supported atm
    RaceManager::get()->setNumKarts((int)players.size());

    // Set number of global and local players.
    RaceManager::get()->setNumPlayers((int)players.size(), local_player_size);

    int local_player_count = -1;
    if (NetworkConfig::get()->isClient())
    {
        local_player_count =
            (int)NetworkConfig::get()->getNetworkPlayers().size();
    }
    // Create the kart information for the race manager:
    // -------------------------------------------------
    for (unsigned int i = 0; i < players.size(); i++)
    {
        const std::shared_ptr<NetworkPlayerProfile>& profile = players[i];
        bool is_local = profile->isLocalPlayer();

        // All non-local players are created here. This means all players
        // on the server, and all non-local players on a client (the local
        // karts are created in the ClientLobby).
        int local_player_id = profile->getLocalPlayerId();

        // local_player_id >= local_player_count for fixed AI defined in create
        // server screen
        if (!is_local || local_player_id >= local_player_count)
        {
            // No device or player profile is needed for remote kart.
            local_player_id =
                (int)(StateManager::get()->createActivePlayer(NULL, NULL));
        }

        // Adjust the local player id so that local players have the numbers
        // 0 to num-1; and all other karts start with num. This way the local
        // players get the first ActivePlayers assigned (which have the 
        // corresponding device associated with it).
        RemoteKartInfo rki(local_player_id,
                           profile->getKartName(),
                           profile->getName(),
                           profile->getHostId(),
                          !is_local);
        rki.setGlobalPlayerId(i);
        rki.setDefaultKartColor(profile->getDefaultKartColor());
        rki.setHandicap(profile->getHandicap());
        rki.setOnlineId(profile->getOnlineId());
        if (RaceManager::get()->teamEnabled())
            rki.setKartTeam(profile->getTeam());
        rki.setCountryCode(profile->getCountryCode());
        rki.setKartData(profile->getKartData());
        rki.setNetworkPlayerProfile(profile);
        // Inform the race manager about the data for this kart.
        RaceManager::get()->setPlayerKart(i, rki);
    }   // for i in players
    // Clean all previous AI if exists in offline game
    RaceManager::get()->computeRandomKartList();
    Log::info("LobbyProtocol", "Player configuration ready.");
}   // configRemoteKart

//-----------------------------------------------------------------------------
/** A previous GameSetup is deleted and a new one is created.
 *  \return Newly create GameSetup object.
 */
void LobbyProtocol::setup()
{
    std::unique_lock<std::mutex> ul(m_current_track_mutex);
    m_current_track.clear();
    ul.unlock();
    m_last_live_join_util_ticks = 0;
    resetVotingTime();
    m_peers_votes.clear();
    m_game_setup->reset();
}   // setupNewGame

//-----------------------------------------------------------------------------
/** Starts the voting period time with the specified maximum time.
 *  \param max_time Maximum voting time in seconds
 */
void LobbyProtocol::startVotingPeriod(float max_time)
{
    m_max_voting_time = uint64_t(max_time*1000);
    m_end_voting_period.store(StkTime::getMonoTimeMs() + m_max_voting_time);
}   // startVotingPeriod

//-----------------------------------------------------------------------------
/** Returns the remaining voting time in seconds. */
float LobbyProtocol::getRemainingVotingTime()
{
    if (m_end_voting_period.load() == 0 ||
        StkTime::getMonoTimeMs() >= m_end_voting_period.load())
        return 0.0f;
    uint64_t t = m_end_voting_period.load() - StkTime::getMonoTimeMs();
    return t / 1000.0f;
}   // getRemainingVotingTime

//-----------------------------------------------------------------------------
/** Returns if the voting period is over. */
bool LobbyProtocol::isVotingOver()
{
    return m_end_voting_period.load() != 0 &&
        m_end_voting_period.load() < StkTime::getMonoTimeMs();
}   // isVotingOver

//-----------------------------------------------------------------------------
/** Adds a vote.
 *  \param host_id Host id of this vote.
 *  \param vote The vote to add. */
void LobbyProtocol::addVote(uint32_t host_id, const PeerVote &vote)
{
    m_peers_votes[host_id] = vote;
}   // addVote

//-----------------------------------------------------------------------------
/** Returns the voting data for one host. Returns NULL if the vote from
 *  the given host id has not yet arrived (or if it is an invalid host id).
 */
const PeerVote* LobbyProtocol::getVote(uint32_t host_id) const
{
    auto it = m_peers_votes.find(host_id);
    if (it == m_peers_votes.end()) return NULL;
    return &(it->second);
}   // getVote

//-----------------------------------------------------------------------------
void LobbyProtocol::addLiveJoiningKart(int kart_id, const RemoteKartInfo& rki,
                                       int live_join_util_ticks) const
{
    AbstractKart* k = World::getWorld()->getKart(kart_id);
    k->changeKart(rki.getKartName(), rki.getHandicap(),
        rki.getKartTeam() == KART_TEAM_RED ?
        std::make_shared<GE::GERenderInfo>(1.0f) :
        rki.getKartTeam() == KART_TEAM_BLUE ?
        std::make_shared<GE::GERenderInfo>(0.66f) :
        std::make_shared<GE::GERenderInfo>(rki.getDefaultKartColor()),
        rki.getKartData());
    k->setLiveJoinKart(live_join_util_ticks);
    World::getWorld()->initTeamArrows(k);
    if (!k->getController()->isLocalPlayerController())
        k->setOnScreenText(rki.getPlayerName().c_str());
}   // addLiveJoiningKart

//-----------------------------------------------------------------------------
bool LobbyProtocol::hasLiveJoiningRecently() const
{
    World* w = World::getWorld();
    if (!w)
        return false;
    return m_last_live_join_util_ticks != 0 &&
        w->getTicksSinceStart() - m_last_live_join_util_ticks > 0 &&
        w->getTicksSinceStart() - m_last_live_join_util_ticks < 120;
}   // hasLiveJoiningRecently

//-----------------------------------------------------------------------------
Track* LobbyProtocol::getPlayingTrack() const
{
    std::unique_lock<std::mutex> ul(m_current_track_mutex);
    std::string track_ident = m_current_track;
    ul.unlock();
    return track_manager->getTrack(track_ident);
}   // getPlayingTrack

//-----------------------------------------------------------------------------
void LobbyProtocol::exitGameState()
{
    bool create_gp_msg = false;
    if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX &&
        RaceManager::get()->getTrackNumber() == RaceManager::get()->getNumOfTracks() - 1)
    {
        create_gp_msg = true;
    }

    RaceManager::get()->clearNetworkGrandPrixResult();
    RaceManager::get()->exitRace();
    RaceManager::get()->setAIKartOverride("");

    if (GUIEngine::isNoGraphics())
    {
        // No screen is ever created when no graphics is on
        StateManager::get()->enterMenuState();
        return;
    }

    GUIEngine::ModalDialog::dismiss();
    GUIEngine::ScreenKeyboard::dismiss();
    RaceResultGUI::getInstance()->cleanupGPProgress();
    if (create_gp_msg)
    {
        core::stringw msg = _("Network grand prix has been finished.");
        MessageQueue::add(MessageQueue::MT_ACHIEVEMENT, msg);
    }

    if (GUIEngine::getCurrentScreen() != NetworkingLobby::getInstance())
    {
        StateManager::get()->resetAndSetStack(
            NetworkConfig::get()->getResetScreens(true/*lobby*/).data());
    }
}   // exitGameState
