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

#include "states_screens/tracks_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
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
    // -- track selection screen
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
    if (m_network_tracks)
    {
        // Remove this screen
        StateManager::get()->popMenu();
        STKHost::get()->shutdown();
    }
    return true; // remove the screen
}   // onEscapePressed

// -----------------------------------------------------------------------------
void TracksScreen::tearDown()
{
    m_network_tracks = false;
    m_vote_timeout = -1.0f;
    m_selected_track = NULL;
}   // tearDown

// -----------------------------------------------------------------------------
void TracksScreen::loadedFromFile()
{
    m_reversed = NULL;
    m_laps = NULL;
    m_votes = NULL;
}   // loadedFromFile

// -----------------------------------------------------------------------------
void TracksScreen::beforeAddingWidget()
{
    Screen::init();

    Widget* rect_box = getWidget("rect-box");

    if (m_bottom_box_height == -1)
        m_bottom_box_height = rect_box->m_h;

    if (m_network_tracks)
    {
        rect_box->setVisible(true);
        rect_box->m_properties[GUIEngine::PROP_HEIGHT] = StringUtils::toString(m_bottom_box_height);
        getWidget("lap-text")->setVisible(true);
        //I18N: In track screen
        getWidget<LabelWidget>("lap-text")->setText(_("Number of laps"), false);
        m_laps = getWidget<SpinnerWidget>("lap-spinner");
        assert(m_laps != NULL);
        m_laps->setVisible(true);
        getWidget("reverse-text")->setVisible(true);
        //I18N: In track screen
        getWidget<LabelWidget>("reverse-text")->setText(_("Drive in reverse"), false);
        m_reversed = getWidget<CheckBoxWidget>("reverse");
        assert(m_reversed != NULL);
        m_reversed->m_properties[GUIEngine::PROP_ALIGN] = "center";
        getWidget("all-track")->m_properties[GUIEngine::PROP_WIDTH] = "60%";
        getWidget("vote")->setVisible(true);
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
        getWidget("all-track")->m_properties[GUIEngine::PROP_WIDTH] = "98%";
        getWidget("vote")->setVisible(false);
        calculateLayout();
    }
    m_votes = getWidget<LabelWidget>("vote-text");
    m_votes->setVisible(false);

    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    tabs->clearAllChildren();

    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
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
    if (m_network_tracks)
    {
        IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
        back_button->setImage("gui/main_quit.png");
    }
    else
    {
        IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
        back_button->setImage("gui/back.png");
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
        if (UserConfigParams::m_num_laps == 0 ||
            UserConfigParams::m_num_laps > 20)
            UserConfigParams::m_num_laps = 1;
        m_laps->setValue(UserConfigParams::m_num_laps);
        m_reversed->setState(m_reverse_checked);
    }

    if (NetworkConfig::get()->isAutoConnect() && m_network_tracks)
    {
        assert(!m_random_track_list.empty());
        NetworkString vote(PROTOCOL_LOBBY_ROOM);
        vote.addUInt8(LobbyProtocol::LE_VOTE);
        vote.encodeString(m_random_track_list[0]).addUInt8(1).addUInt8(0);
        STKHost::get()->sendToServer(&vote, true);
    }
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
        if (curr->isArena() || curr->isSoccer()||curr->isInternal()) continue;
        if (!curr->isInGroup(DEFAULT_GROUP_NAME)) continue;
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
                           "/gui/track_random.png", 0 /* no badge */,
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
void TracksScreen::setVoteTimeout(float timeout)
{
    if (m_vote_timeout != -1.0f)
        return;
    m_vote_timeout = (float)StkTime::getRealTime() + timeout;
}   // setVoteTimeout

// -----------------------------------------------------------------------------
void TracksScreen::voteForPlayer()
{
    assert(STKHost::existHost());
    assert(m_selected_track);
    assert(m_laps);
    assert(m_reversed);
    // Remember reverse globally for each stk instance
    m_reverse_checked = m_reversed->getState();
    UserConfigParams::m_num_laps = m_laps->getValue();
    NetworkString vote(PROTOCOL_LOBBY_ROOM);
    vote.addUInt8(LobbyProtocol::LE_VOTE);
    vote.encodeString(m_selected_track->getIdent())
        .addUInt8(m_laps->getValue()).addUInt8(m_reversed->getState());
    STKHost::get()->sendToServer(&vote, true);
}   // voteForPlayer

// -----------------------------------------------------------------------------
void TracksScreen::onUpdate(float dt)
{
    assert(m_votes);
    if (m_vote_timeout == -1.0f)
    {
        m_votes->setText(L"", false);
        return;
    }

    m_votes->setVisible(true);
    int remaining_time = (int)(m_vote_timeout - StkTime::getRealTime());
    if (remaining_time < 0)
        remaining_time = 0;
    //I18N: In tracks screen, about voting of tracks in network
    core::stringw message = _("Remaining time: %d\n", remaining_time);
    unsigned height = GUIEngine::getFont()->getDimension(L"X").Height;
    const unsigned total_height = m_votes->getDimension().Height;
    m_vote_messages.lock();
    for (auto& p : m_vote_messages.getData())
    {
        height += GUIEngine::getFont()->getDimension(L"X").Height * 2;
        if (height > total_height)
            break;
        message += p.second;
        message += L"\n";
    }
    m_vote_messages.unlock();
    m_votes->setText(message, true);

}   // onUpdate
