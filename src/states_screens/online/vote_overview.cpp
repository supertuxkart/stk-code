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

#include "states_screens/online/vote_overview.hpp"

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
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "network/game_setup.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
//##include "states_screens/track_info_screen.hpp"
#include "states_screens/online/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"
#include "utils/translation.hpp"

#include <iostream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

// -----------------------------------------------------------------------------
VoteOverview::VoteOverview() : Screen("online/vote_overview.stkgui")
{
    m_quit_server       = false;
    m_bottom_box_height = -1;
}   // VoteOverview

// -----------------------------------------------------------------------------
void VoteOverview::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------
void VoteOverview::beforeAddingWidget()
{
    Screen::init();

    m_timer = getWidget<GUIEngine::ProgressBarWidget>("timer");
    m_timer->showLabel(false);

    Widget* rect_box = getWidget("rect-box");

    if (m_bottom_box_height == -1)
        m_bottom_box_height = rect_box->m_h;

    rect_box->setVisible(true);
    rect_box->m_properties[GUIEngine::PROP_HEIGHT] = StringUtils::toString(m_bottom_box_height);

    calculateLayout();
    
    RaceManager::MinorRaceModeType minor_mode = race_manager->getMinorMode();
    bool is_soccer = minor_mode == RaceManager::MINOR_MODE_SOCCER;
    bool is_arena = is_soccer || minor_mode == RaceManager::MINOR_MODE_BATTLE;
 

}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void VoteOverview::init()
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

    for (unsigned int i = 0; i < 8; i++)
    {
        std::string s  = StringUtils::insertValues("track-%d", i);
        IconButtonWidget *track_widget = getWidget<IconButtonWidget>(s.c_str());
        track_widget->setVisible(false);

    }
    for(auto host_id: m_index_to_hostid)
        showVote(host_id);
}   // init

// -----------------------------------------------------------------------------
void VoteOverview::tearDown()
{
    m_quit_server = false;
}   // tearDown

// -----------------------------------------------------------------------------
/** Selects in which part of the grid the new host is being shown and stores
 *  this information in the m_index_to_hostid mapping. If the host_id is
 *  already mapped, this is ignored (this can happen in case one host changes
 *  its vote.
 *  \param host_id Index of the host that is voting.
 */
void VoteOverview::addVote(int host_id)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);

    Log::verbose("VO", "hostid %d new %d",
                 host_id, it==m_index_to_hostid.end());

    // Add a new index if this is the first vote for the host/
    if (it == m_index_to_hostid.end())
    {
        m_index_to_hostid.push_back(host_id);
    }
 
    if(GUIEngine::getCurrentScreen() == this)
        showVote(host_id);

}   // addVote

// ----------------------------------------------------------------------------
/** Populates the one box in the voting screen with the vote from the
 *  corresponding host. A mapping of host_id to index MUST exist for this
 *  host.
 *  \param host_id Host id from hich a new vote was received.
 */
void VoteOverview::showVote(int host_id)
{
    auto it = std::find(m_index_to_hostid.begin(), m_index_to_hostid.end(),
                        host_id);
    assert(it!=m_index_to_hostid.end());

    int index = *it;

    auto lp = LobbyProtocol::get<LobbyProtocol>();
    const LobbyProtocol::PeerVote *vote = lp->getVote(host_id);
    assert(vote);

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
    reverse_widget->setText(_("Reverse: %s", vote->m_reverse ? _("yes")
                                                             : _("no")), true);

}   // addVote

// -----------------------------------------------------------------------------
void VoteOverview::onUpdate(float dt)
{
    // Show the remaining time:
    auto lp = LobbyProtocol::get<LobbyProtocol>();
    float new_value = lp->getRemainingVotingTime() / lp->getMaxVotingTime();
    if (new_value < 0) new_value = 0;
    m_timer->moveValue(int(new_value * 100));

    if(m_index_to_hostid.size()==0) return;

    static float xx = 0.0f;
    xx += dt;
    int index = int(xx) % m_index_to_hostid.size();
    std::string box_name = StringUtils::insertValues("rect-box%d", index);    
    Widget *box = getWidget(box_name.c_str());
    box->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    std::string s = StringUtils::insertValues("name-%d", index);
    LabelWidget *name_widget = getWidget<LabelWidget>(s.c_str());

    name_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    name_widget->setErrorColor();

    s = StringUtils::insertValues("name-%d", 1-index);
    name_widget = getWidget<LabelWidget>(s.c_str());
    name_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    name_widget->setDefaultColor();

#ifdef XX


    std::string s = StringUtils::insertValues("box-%d", index);
    LabelWidget *name_widget = getWidget<>(s.c_str());

        //assert(box);
        std::string num_laps = StringUtils::insertValues("numlaps-%d", i);
        LabelWidget *laps_widget = getWidget<LabelWidget>(num_laps.c_str());
        core::stringw laps = _("Laps: %d", vote->m_num_laps);
        laps_widget->setText(laps, true);
        std::string track_widget_name = StringUtils::insertValues("track-%d", i);
        IconButtonWidget *track_widget =
            getWidget<IconButtonWidget>(track_widget_name.c_str());

        Track *track = track_manager->getTrack(vote->m_track_name);

        track_widget->setImage(track->getScreenshotFile());

    }
    calculateLayout();
#endif
}   // onUpdate

// -----------------------------------------------------------------------------
/** Called on any event, e.g. user input.
 */
void VoteOverview::eventCallback(Widget* widget, const std::string& name,
                                 const int playerID)
{
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (!w2) return;

        std::string selection = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (UserConfigParams::logGUI())
        {
            Log::info("VoteOverview", "Clicked on track '%s'.",
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

    }   // name=="tracks"
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}   // eventCallback

// -----------------------------------------------------------------------------
bool VoteOverview::onEscapePressed()
{
    if (m_quit_server)
    {
        // Remove this screen
        StateManager::get()->popMenu();
        STKHost::get()->shutdown();
    }
    else
    {
        auto ts = TracksScreen::getInstance();
        if (ts)
            ts->setNetworkTracks();
    }
    // remove the screen
    return true;
}   // onEscapePressed

