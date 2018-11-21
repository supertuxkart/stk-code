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
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "network/game_setup.hpp"
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
/** Called on any event, e.g. user input.
 */
void VoteOverview::eventCallback(Widget* widget, const std::string& name,
                                 const int playerID)
{
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if(!w2) return;

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

        if (selection == "random_track")
        {
            if (m_random_track_list.empty()) return;

            selection = m_random_track_list.front();
            m_random_track_list.pop_front();
            m_random_track_list.push_back(selection);

        }   // selection=="random_track"

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
        auto cl = LobbyProtocol::get<ClientLobby>();
        if (cl)
            cl->clearPlayers();
    }
    // remove the screen
    return true;
}   // onEscapePressed

// -----------------------------------------------------------------------------
void VoteOverview::tearDown()
{
    m_quit_server = false;
}   // tearDown

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
    

    //DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    //tracks_widget->setItemCountHint( (int)track_manager->getNumberOfTracks()+1 );

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

}   // init

// -----------------------------------------------------------------------------
void VoteOverview::onUpdate(float dt)
{
    auto lp = LobbyProtocol::get<LobbyProtocol>();
    float new_value = lp->getRemainingVotingTime() / lp->getMaxVotingTime();
    if (new_value < 0) new_value = 0;
    m_timer->moveValue(int(new_value * 100));

}   // onUpdate
