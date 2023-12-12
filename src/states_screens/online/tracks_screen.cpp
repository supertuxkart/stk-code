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

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
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
#include "utils/string_utils.hpp"
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

    else if (name == "vote-list")
    {
        auto cl = LobbyProtocol::get<ClientLobby>();
        ListWidget* list = dynamic_cast<ListWidget*>(widget);
        DynamicRibbonWidget* tracks_widget =
            getWidget<DynamicRibbonWidget>("tracks");

        if (!list || !cl || !tracks_widget || !m_laps || !m_reversed)
            return;
        // Vote to agree with selection of host id
        uint32_t host_id = -1;
        if (StringUtils::fromString(list->getSelectionInternalName(),
            host_id) && host_id != STKHost::get()->getMyHostId())
        {
            const PeerVote* host_vote = cl->getVote(host_id);
            if (host_vote)
            {
                m_selected_track = track_manager->getTrack(
                    host_vote->m_track_name);
                if (!m_selected_track)
                    return;
                tracks_widget->setBadge(host_vote->m_track_name, OK_BADGE);
                m_laps->setValue(host_vote->m_num_laps);
                m_reversed->setState(host_vote->m_reverse);
                voteForPlayer();
            }
        }
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
        if (selection == "locked" && RaceManager::get()->getNumLocalPlayers() == 1)
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
        
        if (m_network_tracks)
        {
            auto cl = LobbyProtocol::get<ClientLobby>();
    
            const PeerVote* vote = cl->getVote(STKHost::get()->getMyHostId());
            if (vote)
            {
                DynamicRibbonWidget* w2 = getWidget<DynamicRibbonWidget>("tracks");
                w2->setBadge(vote->m_track_name, OK_BADGE);
            }
        }
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
    else if (NetworkConfig::get()->isNetworking())
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
    m_laps = NULL;
    m_reversed = NULL;
    m_quit_server = false;
}   // tearDown

// -----------------------------------------------------------------------------
void TracksScreen::loadedFromFile()
{
    m_reversed = NULL;
    m_laps     = NULL;
    m_track_icons = new gui::STKModifiedSpriteBank(GUIEngine::getGUIEnv());
}   // loadedFromFile

// ----------------------------------------------------------------------------
void TracksScreen::unloaded()
{
    delete m_track_icons;
    m_track_icons = NULL;
    m_timer = NULL;
    m_vote_list = NULL;
    m_selected_track = NULL;
    m_laps = NULL;
    m_reversed = NULL;
}   // unloaded

// -----------------------------------------------------------------------------
void TracksScreen::beforeAddingWidget()
{
    Screen::init();

    m_selected_track = NULL;
    m_search_track = NULL;
    m_timer = getWidget<GUIEngine::ProgressBarWidget>("timer");
    m_timer->showLabel(false);

    Widget* rect_box = getWidget("rect-box");

    if (m_bottom_box_height == -1)
        m_bottom_box_height = rect_box->m_h;
    m_vote_list = getWidget<ListWidget>("vote-list");

    m_track_icons->clear();
    if (m_network_tracks)
    {
        rect_box->setCollapsed(false, m_bottom_box_height);
        getWidget("lap-text")->setVisible(true);
        m_laps = getWidget<SpinnerWidget>("lap-spinner");
        assert(m_laps != NULL);
        m_laps->setVisible(true);
        getWidget("reverse-text")->setVisible(true);
        m_reversed = getWidget<CheckBoxWidget>("reverse");
        assert(m_reversed != NULL);
        m_reversed->m_properties[GUIEngine::PROP_ALIGN] = "center";
        m_reversed->setVisible(true);
        m_timer->setVisible(true);
        getWidget("all-track")->m_properties[GUIEngine::PROP_WIDTH] = "60%";
        getWidget("vote")->setVisible(true);
        m_vote_list->setVisible(true);
        m_vote_list->clearColumns();
        auto cl = LobbyProtocol::get<ClientLobby>();
        assert(cl);
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
        {
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "difficulty_medium.png")), 5);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "track_random.png")), 2);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "swap-icon.png")), 1);
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
        {
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "difficulty_medium.png")), 6);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "track_random.png")), 2);
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
        {
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "difficulty_medium.png")), 4);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "track_random.png")), 2);
            if (cl->getGameSetup()->isSoccerGoalTarget())
            {
                m_vote_list->addColumn(irr_driver->getTexture
                    (file_manager->getAsset(FileManager::GUI_ICON,
                    "soccer_ball_normal.png")), 1);
            }
            else
            {
                m_vote_list->addColumn(irr_driver->getTexture
                    (file_manager->getAsset(FileManager::GUI_ICON,
                    "loading.png")), 1);
            }
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "swap-icon.png")), 1);
        }
        else
        {
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "difficulty_medium.png")), 4);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "track_random.png")), 2);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "lap_flag.png")), 1);
            m_vote_list->addColumn(irr_driver->getTexture
                (file_manager->getAsset(FileManager::GUI_ICON,
                "restart.png")), 1);
        }
        calculateLayout();
        static bool shown_msg = false;
        if (!shown_msg)
        {
            shown_msg = true;
            //I18N: In track screen for networking, clarify voting phase
            core::stringw msg = _("If a majority of players all select the"
                " same track and race settings, voting will end early.");
            MessageQueue::add(MessageQueue::MT_GENERIC, msg);
        }
    }
    else
    {
        m_timer->setVisible(false);
        rect_box->setCollapsed(true);
        m_laps = NULL;
        m_reversed = NULL;
        getWidget("lap-text")->setVisible(false);
        getWidget("lap-spinner")->setVisible(false);
        getWidget("reverse-text")->setVisible(false);
        getWidget("reverse")->setVisible(false);
        getWidget("all-track")->m_properties[GUIEngine::PROP_WIDTH] = "98%";
        getWidget("vote")->setVisible(false);
        m_vote_list->setVisible(false);
        m_vote_list->clearColumns();
        calculateLayout();
    }

    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    tabs->clearAllChildren();
    
    RaceManager::MinorRaceModeType minor_mode = RaceManager::get()->getMinorMode();
    bool is_soccer = minor_mode == RaceManager::MINOR_MODE_SOCCER;
    bool is_arena = is_soccer || RaceManager::get()->isBattleMode();
    
    const std::vector<std::string>& groups = 
                        is_arena ? track_manager->getAllArenaGroups(is_soccer)
                                 : track_manager->getAllTrackGroups();
    const int group_amount = (int)groups.size();

    if (group_amount > 1)
    {
        //I18N: name of the tab that will show tracks from all groups
        tabs->addTextChild( _("All"), ALL_TRACK_GROUPS_ID );
    }
    
    // Make group names being picked up by gettext
#define FOR_GETTEXT_ONLY(x)
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("All") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Standard") )
    //I18N: track group name
    FOR_GETTEXT_ONLY( _("Add-Ons") )

    // Add other groups after
    for (int n=0; n<group_amount; n++)
    {
        if (groups[n] == "standard") // Fix capitalization (#4622)
            tabs->addTextChild( _("Standard") , groups[n]);
        else // Try to translate group names
            tabs->addTextChild( _(groups[n].c_str()) , groups[n]);
    } // for n<group_amount

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");

    // Avoid too many items shown at the same time
    tracks_widget->setItemCountHint(std::min((int)track_manager->getNumberOfTracks() + 1, 15));

}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void TracksScreen::init()
{
    if (m_network_tracks)
    {
        m_search_track = getWidget<TextBoxWidget>("search_track");
        m_search_track->setVisible(true);
        m_search_track->setText(L"");
        // Add listener for incremental update when search text is changed
        m_search_track->clearListeners();
        m_search_track->addListener(this);
        updateProgressBarText();
    }
    else
        getWidget("search_track")->setVisible(false);

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
        for (const std::string& track : cl->getAvailableTracks())
        {
            Track* t = track_manager->getTrack(track);
            if (!t)
            {
                Log::fatal("TracksScreen", "Missing network track %s",
                    track.c_str());
            }
            video::ITexture* tex =
                irr_driver->getTexture(t->getScreenshotFile());
            if (!tex)
            {
                tex = irr_driver->getTexture(file_manager
                    ->getAsset(FileManager::GUI_ICON, "main_help.png"));
            }
            assert(tex);
            m_track_icons->addTextureAsSprite(tex);
        }

        int icon_height = getHeight() / 13;
        m_track_icons->setScale(icon_height / 256.0f);
        m_track_icons->setTargetIconSize(256, 256);
        m_vote_list->setIcons(m_track_icons, (int)icon_height);

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
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
        {
            getWidget("lap-text")->setVisible(false);
            m_laps->setValue(0);
            m_laps->setVisible(false);
            getWidget("reverse-text")->setVisible(true);
            //I18N: In track screen
            getWidget<LabelWidget>("reverse-text")->setText(_("Random item location"), false);
            m_reversed->setVisible(true);
            m_reversed->setState(UserConfigParams::m_random_arena_item);
            if (vote)
                m_reversed->setState(vote->m_reverse);
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
        {
            getWidget("lap-text")->setVisible(false);
            m_laps->setValue(0);
            m_laps->setVisible(false);
            getWidget("reverse-text")->setVisible(false);
            m_reversed->setState(false);
            m_reversed->setVisible(false);
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
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
            m_reversed->setState(false);
            if (vote)
                m_reversed->setState(vote->m_reverse);
        }
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
        core::stringw search_text;
        if (m_search_track)
        {
            search_text = m_search_track->getText();
            search_text.make_lower();
        }
        if (!search_text.empty() &&
            curr->getName().make_lower().find(search_text.c_str()) == -1)
            continue;
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_EASTER_EGG
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
        if (PlayerManager::getCurrentPlayer() &&
            PlayerManager::getCurrentPlayer()->isLocked(curr->getIdent()) &&
            RaceManager::get()->getNumLocalPlayers() == 1 && !is_network)
        {
            tracks_widget->addItem(
                _("Locked: solve active challenges to gain access to more!"),
                "locked", curr->getScreenshotFile(), LOCKED_BADGE,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            tracks_widget->addItem(curr->getName(),
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
    if (!RaceManager::get()->isBattleMode() &&
        RaceManager::get()->getMinorMode() != RaceManager::MINOR_MODE_SOCCER)
    {
        UserConfigParams::m_num_laps = m_laps->getValue();
    }
    else
        UserConfigParams::m_random_arena_item = m_reversed->getState();

    NetworkString vote(PROTOCOL_LOBBY_ROOM);
    vote.addUInt8(LobbyProtocol::LE_VOTE);
    core::stringw player_name;
    if (PlayerManager::getCurrentPlayer())
        player_name = PlayerManager::getCurrentPlayer()->getName();
    vote.encodeString(player_name);
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
    {
        vote.encodeString(m_selected_track->getIdent())
            .addUInt8(0).addUInt8(m_reversed->getState() ? 1 : 0);
    }
    else if (RaceManager::get()->getMinorMode() ==
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
    if (!m_network_tracks)
        return;

    if (m_winning_index != std::numeric_limits<uint32_t>::max() && m_vote_list)
    {
        int list_id =
            m_vote_list->getItemID(StringUtils::toString(m_winning_index));
        //if (StkTime::getMonoTimeMs() / 1000 % 2 == 0)
            m_vote_list->setSelectionID(list_id);
        //else
        //    m_vote_list->unfocused(PLAYER_ID_GAME_MASTER, NULL);
        return;
    }
    updateProgressBarText();

}   // onUpdate

// -----------------------------------------------------------------------------
/** Selects in which part of the vote list the new host is being shown and
 *  stores this information in the m_index_to_hostid mapping. If the host_id is
 *  already mapped, this is ignored (this can happen in case one host changes
 *  its vote.
 *  \param host_id Index of the host that is voting.
 *  \param vote Vote information.
 */
void TracksScreen::addVote(uint32_t host_id, const PeerVote& vote)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);

    Log::debug("TracksScreen", "addVote: hostid %d is new %d",
                 host_id, it == m_index_to_hostid.end());

    // Add a new index if this is the first vote for the host
    if (it == m_index_to_hostid.end())
    {
        // Sound effect like lobby chat
        if (GUIEngine::getCurrentScreen() == this)
            SFXManager::get()->quickSound("plopp");
        m_index_to_hostid.push_back(host_id);
    }
    if (host_id == STKHost::get()->getMyHostId() && m_laps && m_reversed)
    {
        m_laps->setValue(vote.m_num_laps);
        m_reversed->setState(vote.m_reverse);
    }
}   // addVote

// ----------------------------------------------------------------------------
/** Removes a vote, which is triggered when a client disconnects.
 *  \param host_id Host id of the disconnected client.
 */
void TracksScreen::removeVote(uint32_t host_id)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);

    Log::debug("TracksScreen", "removeVote: hostid %d found %d",
                 host_id, it != m_index_to_hostid.end());

    // Add a new index if this is the first vote for the host
    if (it != m_index_to_hostid.end())
    {
        m_index_to_hostid.erase(it);
    }
}   //removeVote

// -----------------------------------------------------------------------------
/** Received the winning vote. i.e. the data about the track to play (including
 *  #laps etc).
 */
void TracksScreen::setResult(uint32_t winner_host,
                             const PeerVote& winner_vote)
{
    // If the GUI is forced from the server lobby, m_timer is not defined
    if (!m_timer || winner_host == std::numeric_limits<uint32_t>::max() ||
        !m_vote_list)
        return;
    if (m_timer) m_timer->setVisible(false);

    m_winning_index = winner_host;
    if (auto lp = LobbyProtocol::get<LobbyProtocol>())
    {
        lp->addVote(winner_host, winner_vote);
    }
    updatePlayerVotes();
}   // setResult

// -----------------------------------------------------------------------------
/* Update player votes whenever vote is recieved from any players or
 * player disconnected, or when this screen is pushed.
 */
void TracksScreen::updatePlayerVotes()
{
    auto cl = LobbyProtocol::get<ClientLobby>();
    if (GUIEngine::getCurrentScreen() != this || !cl || !m_vote_list)
        return;
    
    std::string selected_name = m_vote_list->getSelectionInternalName();
    
    m_vote_list->clear();
    for (unsigned i = 0; i < m_index_to_hostid.size(); i++)
    {
        const PeerVote* p = cl->getVote(m_index_to_hostid[i]);
        assert(p);
        std::vector<GUIEngine::ListWidget::ListCell> row;
        core::stringw y = L"\u2714";
        core::stringw n = L"\u2716";
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
        {
            row.push_back(GUIEngine::ListWidget::ListCell
                (p->m_player_name , -1, 5));
            int icon = -1;
            const auto& tracks = cl->getAvailableTracks();
            auto it = tracks.find(p->m_track_name);
            if (it != tracks.end())
            {
                icon = (int)std::distance(tracks.begin(), it);
            }
            row.push_back(GUIEngine::ListWidget::ListCell
                ("" , icon, 2, true/*center*/));
            row.push_back(GUIEngine::ListWidget::ListCell
                (p->m_reverse ? y : n , -1, 1, true/*center*/));
            m_vote_list->addItem(
                StringUtils::toString(m_index_to_hostid[i]), row);
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
        {
            row.push_back(GUIEngine::ListWidget::ListCell
                (p->m_player_name , -1, 6));
            int icon = -1;
            const auto& tracks = cl->getAvailableTracks();
            auto it = tracks.find(p->m_track_name);
            if (it != tracks.end())
            {
                icon = (int)std::distance(tracks.begin(), it);
            }
            row.push_back(GUIEngine::ListWidget::ListCell
                ("" , icon, 2, true/*center*/));
            m_vote_list->addItem(
                StringUtils::toString(m_index_to_hostid[i]), row);
        }
        else
        {
            row.push_back(GUIEngine::ListWidget::ListCell
                (p->m_player_name , -1, 4, 0));
            int icon = -1;
            const auto& tracks = cl->getAvailableTracks();
            auto it = tracks.find(p->m_track_name);
            if (it != tracks.end())
            {
                icon = (int)std::distance(tracks.begin(), it);
            }
            row.push_back(GUIEngine::ListWidget::ListCell
                ("" , icon, 2, true/*center*/));
            int laps = p->m_num_laps;
            row.push_back(GUIEngine::ListWidget::ListCell
                (StringUtils::toWString(laps) , -1, 1, true/*center*/));
            row.push_back(GUIEngine::ListWidget::ListCell
                (p->m_reverse ? y : n , -1, 1, true/*center*/));
            m_vote_list->addItem(
                StringUtils::toString(m_index_to_hostid[i]), row);
        }
    }
    
    if (!selected_name.empty())
    {
        int id = m_vote_list->getItemID(selected_name);
        m_vote_list->setSelectionID(id);
    }
}   // updatePlayerVotes

// ----------------------------------------------------------------------------
void TracksScreen::updateProgressBarText()
{
    if (auto lp = LobbyProtocol::get<LobbyProtocol>())
    {
        float new_value =
            lp->getRemainingVotingTime() / lp->getMaxVotingTime();
        if (new_value < 0.0f)
            new_value = 0.0f;
        m_timer->setValue(new_value * 100.0f);
        int remaining_time = (int)(lp->getRemainingVotingTime());
        if (remaining_time < 0)
            remaining_time = 0;
        //I18N: In kart screen, show before the voting period in network ends.
        core::stringw message = _("Remaining time: %d", remaining_time);
        m_timer->setText(message);
    }
}   // updateProgressBarText
