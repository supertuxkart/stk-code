//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/online/tracks_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "network/game_setup.hpp"
#include "network/peer_vote.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/track_info_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

#include <iostream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

static const char ALL_TRACK_GROUPS_ID[] = "all";

// -----------------------------------------------------------------------------

void TracksScreen::eventCallback(Widget* widget, const std::string& name,
                                 const int playerID)
{
    if ((name == "lap-spinner" || name == "reverse") &&
         STKHost::existHost() && m_selected_track != NULL)
    {
        voteForPlayer();
    }

    else if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if(!w2) return;

        std::string selection = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (UserConfigParams::logGUI())
        {
            Log::info("TracksScreen", "Clicked on track '%s'.",
                       selection.c_str());
        }

        UserConfigParams::m_last_track = selection;
        if (selection == "locked" && race_manager->getNumLocalPlayers() == 1)
        {
            unlock_manager->playLockSound();
            return;
        }
        else if (selection == RibbonWidget::NO_ITEM_ID)
        {
            return;
        }

        if (selection == "random_track")
        {
            if (m_random_track_list.empty()) return;

            selection = m_random_track_list.front();
            m_random_track_list.pop_front();
            m_random_track_list.push_back(selection);
        }   // selection=="random_track"

        m_selected_track = track_manager->getTrack(selection);

        if (m_selected_track)
        {
            if (STKHost::existHost())
            {
                w2->setBadge(selection, OK_BADGE);
                voteForPlayer();
            }
            else
            {
                TrackInfoScreen::getInstance()->setTrack(m_selected_track);
                TrackInfoScreen::getInstance()->push();
            }
        }   // if clicked_track

    }   // name=="tracks"
    else if (name == "trackgroups")
    {
        RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
        UserConfigParams::m_last_used_track_group = tabs->getSelectionIDString(0);
        buildTrackList();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// -----------------------------------------------------------------------------
bool TracksScreen::onEscapePressed()
{
    if (m_quit_server)
    {
        // Remove this screen
        StateManager::get()->popMenu();
        STKHost::get()->shutdown();
    }
    else
    {
        NetworkConfig::get()->clearActivePlayersForClient();
    }
    // remove the screen
    return true;
}   // onEscapePressed

// -----------------------------------------------------------------------------
void TracksScreen::tearDown()
{
    m_network_tracks = false;
    m_selected_track = NULL;
    m_quit_server = false;
}   // tearDown

// -----------------------------------------------------------------------------
void TracksScreen::loadedFromFile()
{
    m_reversed = NULL;
    m_laps     = NULL;
}   // loadedFromFile

// -----------------------------------------------------------------------------
void TracksScreen::beforeAddingWidget()
{
    Screen::init();

    m_selected_track = NULL;
    m_timer = getWidget<GUIEngine::ProgressBarWidget>("timer");
    m_timer->showLabel(false);

    Widget* rect_box = getWidget("rect-box");

    if (m_bottom_box_height == -1)
        m_bottom_box_height = rect_box->m_h;

    if (m_network_tracks)
    {
        rect_box->setVisible(true);
        rect_box->m_properties[GUIEngine::PROP_HEIGHT] = StringUtils::toString(m_bottom_box_height);
        getWidget("lap-text")->setVisible(true);
        m_laps = getWidget<SpinnerWidget>("lap-spinner");
        assert(m_laps != NULL);
        m_laps->setVisible(true);
        getWidget("reverse-text")->setVisible(true);
        m_reversed = getWidget<CheckBoxWidget>("reverse");
        assert(m_reversed != NULL);
        m_reversed->m_properties[GUIEngine::PROP_ALIGN] = "center";
        m_reversed->setVisible(true);
        calculateLayout();
    }
    else
    {
        rect_box->setVisible(false);
        rect_box->m_properties[GUIEngine::PROP_HEIGHT] = "0";
        m_laps = NULL;
        m_reversed = NULL;
        getWidget("lap-text")->setVisible(false);
        getWidget("lap-spinner")->setVisible(false);
        getWidget("reverse-text")->setVisible(false);
        getWidget("reverse")->setVisible(false);
        calculateLayout();
    }

    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    tabs->clearAllChildren();
    
    RaceManager::MinorRaceModeType minor_mode = race_manager->getMinorMode();
    bool is_soccer = minor_mode == RaceManager::MINOR_MODE_SOCCER;
    bool is_arena = is_soccer || race_manager->isBattleMode();
    
    const std::vector<std::string>& groups = 
                        is_arena ? track_manager->getAllArenaGroups(is_soccer)
                                 : track_manager->getAllTrackGroups();
    const int group_amount = (int)groups.size();

    if (group_amount > 1)
    {
        //I18N: name of the tab that will show tracks from all groups
        tabs->addTextChild( _("All"), ALL_TRACK_GROUPS_ID );
    }

    // add behind the other categories
    for (int n=0; n<group_amount; n++)
        tabs->addTextChild( _(groups[n].c_str()), groups[n] );

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    tracks_widget->setItemCountHint( (int)track_manager->getNumberOfTracks()+1 );

}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void TracksScreen::init()
{
    // change the back button image (because it makes the game quit)
    if (m_quit_server)
    {
        IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
        back_button->setImage("gui/icons/main_quit.png");
    }
    else
    {
        IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
        back_button->setImage("gui/icons/back.png");
    }

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);

    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    tabs->select(UserConfigParams::m_last_used_track_group, PLAYER_ID_GAME_MASTER);

    buildTrackList();

    // select old track for the game master (if found)
    STKTexManager::getInstance()->setTextureErrorMessage(
              "While loading screenshot in track screen for last track '%s':",
              UserConfigParams::m_last_track);
    if (!tracks_widget->setSelection(UserConfigParams::m_last_track,
                                     PLAYER_ID_GAME_MASTER, true))
    {
        tracks_widget->setSelection(0, PLAYER_ID_GAME_MASTER, true);
    }
    STKTexManager::getInstance()->unsetTextureErrorMessage();
    if (m_network_tracks)
    {
        // Notice: for arena (battle / soccer) lap and reverse will be mapped to
        // goals / time limit and random item location
        auto cl = LobbyProtocol::get<ClientLobby>();
        assert(cl);
        const PeerVote* vote = cl->getVote(STKHost::get()->getMyHostId());
        if (vote)
        {
            DynamicRibbonWidget* w2 = getWidget<DynamicRibbonWidget>("tracks");
            m_selected_track = track_manager->getTrack(vote->m_track_name);
            w2->setBadge(vote->m_track_name, OK_BADGE);
        }

        if (UserConfigParams::m_num_laps == 0 ||
            UserConfigParams::m_num_laps > 20)
            UserConfigParams::m_num_laps = 1;
        if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
        {
            getWidget("lap-text")->setVisible(false);
            m_laps->setVisible(false);
            getWidget("reverse-text")->setVisible(true);
            //I18N: In track screen
            getWidget<LabelWidget>("reverse-text")->setText(_("Random item location"), false);
            m_reversed->setVisible(true);
            m_reversed->setState(UserConfigParams::m_random_arena_item);
            if (vote)
                m_reversed->setState(vote->m_reverse);
        }
        else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
        {
            getWidget("lap-text")->setVisible(false);
            m_laps->setVisible(false);
            getWidget("reverse-text")->setVisible(false);
            m_reversed->setVisible(false);
        }
        else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
        {
            if (cl->isServerAutoGameTime())
            {
                getWidget("lap-text")->setVisible(false);
                m_laps->setVisible(false);
                m_laps->setValue(0);
            }
            else
            {
                m_laps->setVisible(true);
                getWidget("lap-text")->setVisible(true);
                if (cl->getGameSetup()->isSoccerGoalTarget())
                {
                    //I18N: In track screen
                    getWidget<LabelWidget>("lap-text")->setText(_("Number of goals to win"), false);
                    m_laps->setValue(UserConfigParams::m_num_goals);
                    m_laps->setMin(1);
                    m_laps->setMax(10);
                }
                else
                {
                    //I18N: In track screen
                    getWidget<LabelWidget>("lap-text")->setText(_("Maximum time (min.)"), false);
                    m_laps->setValue(UserConfigParams::m_soccer_time_limit);
                    m_laps->setMin(1);
                    m_laps->setMax(15);
                }
                if (vote)
                    m_laps->setValue(vote->m_num_laps);
            }
            getWidget("reverse-text")->setVisible(true);
            //I18N: In track screen
            getWidget<LabelWidget>("reverse-text")->setText(_("Random item location"), false);
            m_reversed->setVisible(true);
            m_reversed->setState(UserConfigParams::m_random_arena_item);
            if (vote)
                m_reversed->setState(vote->m_reverse);
        }
        else
        {
            if (cl->isServerAutoGameTime())
            {
                getWidget("lap-text")->setVisible(false);
                m_laps->setVisible(false);
                m_laps->setValue(0);
            }
            else
            {
                getWidget("lap-text")->setVisible(true);
                //I18N: In track screen
                getWidget<LabelWidget>("lap-text")
                         ->setText(_("Number of laps"), false);
                m_laps->setVisible(true);
                m_laps->setMin(1);
                m_laps->setMax(20);
                m_laps->setValue(UserConfigParams::m_num_laps);
                if (vote)
                    m_laps->setValue(vote->m_num_laps);
            }
            getWidget("reverse-text")->setVisible(true);
            //I18N: In track screen
            getWidget<LabelWidget>("reverse-text")
                     ->setText(_("Drive in reverse"), false);
            m_reversed->setVisible(true);
            if (vote)
                m_reversed->setState(vote->m_reverse);
        }
    }
    if (NetworkConfig::get()->isAutoConnect() && m_network_tracks)
    {
        assert(!m_random_track_list.empty());
        NetworkString vote(PROTOCOL_LOBBY_ROOM);
        vote.addUInt8(LobbyProtocol::LE_VOTE);
        vote.encodeString(m_random_track_list[0]).addUInt8(1).addUInt8(0);
        STKHost::get()->sendToServer(&vote, true);
    }
    updatePlayerVotes();
}   // init

// -----------------------------------------------------------------------------
/** Rebuild the list of tracks. This need to be recomputed e.g. to
 *  take unlocked tracks into account.
 */
void TracksScreen::buildTrackList()
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");

    // Reset track list everytime (accounts for locking changes, etc.)
    tracks_widget->clearItems();
    m_random_track_list.clear();

    const std::string& curr_group_name = tabs->getSelectionIDString(0);
    const int track_amount = (int)track_manager->getNumberOfTracks();

    // First build a list of all tracks to be displayed
    // (e.g. exclude arenas, ...)
	bool is_network = (STKHost::existHost());
    std::shared_ptr<ClientLobby> clrp;
    if (is_network)
    {
        clrp = LobbyProtocol::get<ClientLobby>();
        assert(clrp);
    }
    PtrVector<Track, REF> tracks;
    for (int n = 0; n < track_amount; n++)
    {
        Track* curr = track_manager->getTrack(n);
        if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_EASTER_EGG
            && !curr->hasEasterEggs())
            continue;
        if (!is_network &&
            (curr->isArena() || curr->isSoccer() || curr->isInternal()))
            continue;
        if (curr_group_name != ALL_TRACK_GROUPS_ID &&
            !curr->isInGroup(curr_group_name)) continue;
        if (is_network &&
            clrp->getAvailableTracks().find(curr->getIdent()) ==
            clrp->getAvailableTracks().end())
        {
            continue;
        }
        tracks.push_back(curr);
    }   // for n<track_amount

    tracks.insertionSort();
    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        Track *curr = tracks.get(i);
        if (PlayerManager::getCurrentPlayer()->isLocked(curr->getIdent()) &&
            race_manager->getNumLocalPlayers() == 1 && !is_network)
        {
            tracks_widget->addItem(
                _("Locked: solve active challenges to gain access to more!"),
                "locked", curr->getScreenshotFile(), LOCKED_BADGE,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            tracks_widget->addItem(translations->fribidize(curr->getName()),
                curr->getIdent(),
                curr->getScreenshotFile(), 0,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            m_random_track_list.push_back(curr->getIdent());
        }
    }

    tracks_widget->addItem(_("Random Track"), "random_track",
                           "/gui/icons/track_random.png", 0 /* no badge */,
                           IconButtonWidget::ICON_PATH_TYPE_RELATIVE);

    tracks_widget->updateItemDisplay();
    std::random_shuffle( m_random_track_list.begin(), m_random_track_list.end() );
}   // buildTrackList

// -----------------------------------------------------------------------------
void TracksScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");

    // only the game master can select tracks,
    // so it's safe to use 'PLAYER_ID_GAME_MASTER'
    tracks_widget->setSelection(trackName, PLAYER_ID_GAME_MASTER, true);
}   // setFocusOnTrack

// -----------------------------------------------------------------------------
void TracksScreen::voteForPlayer()
{
    assert(STKHost::existHost());

    assert(m_laps);
    assert(m_reversed);
    // Remember reverse globally for each stk instance if not arena
    if (!race_manager->isBattleMode() &&
        race_manager->getMinorMode() != RaceManager::MINOR_MODE_SOCCER)
    {
        UserConfigParams::m_num_laps = m_laps->getValue();
    }
    else
        UserConfigParams::m_random_arena_item = m_reversed->getState();

    NetworkString vote(PROTOCOL_LOBBY_ROOM);
    vote.addUInt8(LobbyProtocol::LE_VOTE);
    const core::stringw &player_name =
        PlayerManager::getCurrentPlayer()->getName();
    vote.encodeString(player_name);
    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
    {
        vote.encodeString(m_selected_track->getIdent())
            .addUInt8(0).addUInt8(m_reversed->getState() ? 1 : 0);
    }
    else if (race_manager->getMinorMode() ==
        RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
    {
        vote.encodeString(m_selected_track->getIdent())
            .addUInt8(0).addUInt8(0);
    }
    else
    {
        vote.encodeString(m_selected_track->getIdent())
            .addUInt8(m_laps->getValue())
            .addUInt8(m_reversed->getState() ? 1 : 0);
    }
    if (auto lp = LobbyProtocol::get<LobbyProtocol>())
    {
        vote.reset();
        vote.skip(2);
        PeerVote pvote(vote);
        lp->addVote(STKHost::get()->getMyHostId(), pvote);
    }
    STKHost::get()->sendToServer(&vote, true);
}   // voteForPlayer

// -----------------------------------------------------------------------------
void TracksScreen::onUpdate(float dt)
{
    // The following code
    if(!m_network_tracks) return;

    auto lp = LobbyProtocol::get<LobbyProtocol>();
    float new_value = lp->getRemainingVotingTime() / lp->getMaxVotingTime();
    if (new_value < 0) new_value = 0;
    m_timer->setValue(new_value * 100.0f);

}   // onUpdate

// ----------------------------------------------------------------------------
/** Called when the final 'random picking' animation is finished so that only
 *  the result is shown : all votes except the winner is set to be invisible.
 */
void TracksScreen::showVoteResult()
{
    Log::info("TracksScreen", "showVoteResult: winning index %d",
              m_winning_index);
    // TODO: Make all listed votes except the winner invisible: something
    // like this:
    //for (unsigned int i = 0; i < 8; i++)
    //{
    //   std::string box_name = StringUtils::insertValues("rect-box%d", i);
    //    Widget *box = getWidget(box_name.c_str());
    //    if (i != m_winning_index)
    //        box->setVisible(false);
    //    else
    //        box->setSelected(PLAYER_ID_GAME_MASTER, true);
    //}
}   // showVoteResult

// ----------------------------------------------------------------------------
/** Stores the number of players. This can be used to determine how many
 *  slotes for votes are required. This function is called from ClientLobby
 *  upon an update from the server.
 *  \param n New number of players that can vote.
 */
void TracksScreen::updateNumPlayers(int n)
{
    m_max_num_votes = n;
}   //updateNumPlayers

// -----------------------------------------------------------------------------
/** Selects in which part of the vote list the new host is being shown and
 *  stores this information in the m_index_to_hostid mapping. If the host_id is
 *  already mapped, this is ignored (this can happen in case one host changes
 *  its vote.
 *  \param host_id Index of the host that is voting.
 */
void TracksScreen::addVote(int host_id)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);

    Log::verbose("TracksScreen", "addVote: hostid %d is new %d",
                 host_id, it == m_index_to_hostid.end());

    // Add a new index if this is the first vote for the host/
    if (it == m_index_to_hostid.end())
    {
        m_index_to_hostid.push_back(host_id);
    }

    // If the screen is already shown, update the voting display
    if (GUIEngine::getCurrentScreen() == this)
        showVote(host_id);
}   // addVote

// ----------------------------------------------------------------------------
/** Removes a vote, which is triggered when a client disconnects.
 *  \param host_id Host id of the disconnected client.
 */
void TracksScreen::removeVote(int host_id)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);

    Log::verbose("TracksScreen", "removeVote: hostid %d found %d",
                 host_id, it != m_index_to_hostid.end());

    // Add a new index if this is the first vote for the host/
    if (it != m_index_to_hostid.end())
    {
        m_index_to_hostid.erase(it);
    }
}   //removeVote

// ----------------------------------------------------------------------------
/** Populates one entry in the voting list with the vote from the
 *  corresponding host. A mapping of host_id to index MUST exist for this
 *  host when this function is called.
 *  \param host_id Host id from hich a new vote was received.
 */
void TracksScreen::showVote(int host_id)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);
    assert(it != m_index_to_hostid.end());

    int index = it - m_index_to_hostid.begin();

    auto lp = LobbyProtocol::get<LobbyProtocol>();
    const PeerVote *vote = lp->getVote(host_id);
    assert(vote);

    // This is the old code that needs to be updated for the new list display
#ifdef OLD_DISPLAY
    std::string s = StringUtils::insertValues("name-%d", index);
    LabelWidget *name_widget = getWidget<LabelWidget>(s.c_str());
    name_widget->setText(_("Name: %s", vote->m_player_name), true);

    s = StringUtils::insertValues("track-%d", index);
    IconButtonWidget *track_widget = getWidget<IconButtonWidget>(s.c_str());
    Track *track = track_manager->getTrack(vote->m_track_name);
    track_widget->setVisible(true);
    track_widget->setImage(track->getScreenshotFile());

    s = StringUtils::insertValues("numlaps-%d", index);
    LabelWidget *laps_widget = getWidget<LabelWidget>(s.c_str());
    laps_widget->setText(_("Laps: %d", vote->m_num_laps), true);

    s = StringUtils::insertValues("reverse-%d", index);
    LabelWidget *reverse_widget = getWidget<LabelWidget>(s.c_str());
    core::stringw yes = _("yes");
    core::stringw no = _("no");
    reverse_widget->setText(_("Reverse: %s", vote->m_reverse ? yes : no),
                            true);
#endif
}   // addVote

// -----------------------------------------------------------------------------
/** Received the winning vote. i.e. the data about the track to play (including
 *  #laps etc).
 */
void TracksScreen::setResult(const PeerVote &winner_vote)
{
    // If the GUI is forced from the server lobby, m_timer is not defined
    if (m_timer) m_timer->setVisible(false);

    // Note that the votes on the server might have a different order from
    // the votes here on the client. Potentially there could also be a missing
    // vote(??)
    auto lp = LobbyProtocol::get<LobbyProtocol>();
    m_winning_index = -1;
    for (unsigned int i = 0; i < m_index_to_hostid.size(); i++)
    {
        const PeerVote *vote = lp->getVote(m_index_to_hostid[i]);
        if (!vote) continue;
        if (vote->m_track_name == winner_vote.m_track_name &&
            vote->m_num_laps == winner_vote.m_num_laps &&
            vote->m_reverse == winner_vote.m_reverse)
        {
            m_winning_index = i;
            break;
        }
        // Try to prepare a fallback in case that the right vote is not here.
        if (vote->m_track_name == winner_vote.m_track_name)
        {
            m_winning_index = i;
        }
    }   // for i in m_index_to_hostid

    if (m_winning_index == -1)
    {
        // We don't have the right vote. Assume that a message got lost,
        // In this case, change one non-local vote:
        for (unsigned int i = 0; i < m_index_to_hostid.size(); i++)
        {
            if (m_index_to_hostid[i] != STKHost::get()->getMyHostId())
            {
                lp->addVote(m_index_to_hostid[i], winner_vote);
                m_winning_index = i;
                break;
            }
        }
    }   // wim_winning_index == -1

}   // setResult

// -----------------------------------------------------------------------------
/* Update player votes whenever vote is recieved from any players or
 * player disconnected, or when this screen is pushed.
 */
void TracksScreen::updatePlayerVotes()
{
}   // updatePlayerVotes
