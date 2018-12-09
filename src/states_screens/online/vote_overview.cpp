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
#include "guiengine/widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

// -----------------------------------------------------------------------------
VoteOverview::VoteOverview() : Screen("online/vote_overview.stkgui")
{
    m_quit_server       = false;
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

    rect_box->setVisible(true);
    rect_box->m_properties[GUIEngine::PROP_HEIGHT] = StringUtils::toString(rect_box->m_h);

    calculateLayout();
    
    RaceManager::MinorRaceModeType minor_mode = race_manager->getMinorMode();
    bool is_soccer = minor_mode == RaceManager::MINOR_MODE_SOCCER;
    bool is_arena = is_soccer || minor_mode == RaceManager::MINOR_MODE_BATTLE;
 

}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void VoteOverview::init()
{
    m_timer->setVisible(true);
    m_random_anim_timer = 0.0f;
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

    int index = it - m_index_to_hostid.begin();

    auto lp = LobbyProtocol::get<LobbyProtocol>();
    const PeerVote *vote = lp->getVote(host_id);
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
    core::stringw yes = _("yes");
    core::stringw no  = _("no");
    reverse_widget->setText(_("Reverse: %s", vote->m_reverse ? yes : no), 
                            true);

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

    // First make sure the old grid is set back to normal:
    int old_index = int(m_random_anim_timer) % m_index_to_hostid.size();
    std::string box_name = StringUtils::insertValues("rect-box%d", old_index);
    Widget *box = getWidget(box_name.c_str());
    box->setSelected(PLAYER_ID_GAME_MASTER, false);

    // Increase timer, and determine next index to show
    m_random_anim_timer += 2*dt;
    int new_index = int(m_random_anim_timer) % m_index_to_hostid.size();
    box_name = StringUtils::insertValues("rect-box%d", new_index);
    box = getWidget(box_name.c_str());
    box->setSelected(PLAYER_ID_GAME_MASTER, true);
}   // onUpdate

// -----------------------------------------------------------------------------
/** Received the winning vote. i.e. the data about the track to play (including
 *  #laps etc).
 */
void VoteOverview::setResult(const PeerVote &winner_vote)
{
    m_timer->setVisible(false);
    // Note that the votes on the server might have a different order from
    // the votes here on the client. Potentially there could also be a missing
    // vote(??)
    auto lp = LobbyProtocol::get<LobbyProtocol>();
    m_winning_index = -1;
    for (unsigned int i=0; i<m_index_to_hostid.size(); i++)
    {
        const PeerVote *vote = lp->getVote(m_index_to_hostid[i]);
        if(!vote) continue;
        if(vote->m_track_name == winner_vote.m_track_name && 
           vote->m_num_laps == winner_vote.m_num_laps &&
           vote->m_reverse == winner_vote.m_reverse      )
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

    if(m_winning_index == -1)
    {
        // We don't have the right vote. Assume that a message got lost,
        // In this case, change one non-local vote:
        for(unsigned int i=0; i<m_index_to_hostid.size(); i++)
        {
            if(m_index_to_hostid[i] != STKHost::get()->getMyHostId())
            {
                lp->addVote(m_index_to_hostid[i], winner_vote);
                m_winning_index = i;
                break;
            }
        }
    }   // winnind==-1
}   // setResult

// -----------------------------------------------------------------------------
/** Called when the final 'random picking' animation is finished so that only
 *  the result is shown: all boxes except the winner is set to be invisible.
 */
void VoteOverview::showVoteResult()
{
    for (unsigned int i = 0; i < 8; i++)
    {
        std::string box_name = StringUtils::insertValues("rect-box%d", i);
        Widget *box = getWidget(box_name.c_str());
        if(i!=m_winning_index)
            box->setVisible(false);
        else
            box->setSelected(PLAYER_ID_GAME_MASTER, true);
    }

}   // showVoteResult

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

