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

#include "states_screens/waiting_for_others.hpp"

#include "config/user_config.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/game_setup.hpp"
#include "network/network_player_profile.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( WaitingForOthersScreen );

// -----------------------------------------------------------------------------

WaitingForOthersScreen::WaitingForOthersScreen() : Screen("online/waiting_for_others.stkgui")
{
}   // WaitingForOthersScreen

// -----------------------------------------------------------------------------

void WaitingForOthersScreen::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void WaitingForOthersScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
}   // eventCallback

// -----------------------------------------------------------------------------

void WaitingForOthersScreen::init()
{
    Screen::init();
}   //init

// -----------------------------------------------------------------------------

void WaitingForOthersScreen::onUpdate(float dt)
{
    const GameSetup *setup = STKHost::get()->getGameSetup();
    const std::vector<NetworkPlayerProfile*> &all_profiles = setup->getPlayers();

    RaceConfig* config = STKHost::get()->getGameSetup()->getRaceConfig();
    core::stringw w;
    for (unsigned int i = 0; i < all_profiles.size(); i++)
    {
        const NetworkPlayerProfile* profile = all_profiles[i];
        if (profile == NULL)
            continue;
        core::stringw name = profile->getName();


        int playerId = profile->getGlobalPlayerId();
        const std::string &kart = profile->getKartName();
        if (!kart.empty())
            name += StringUtils::insertValues(L" (%s)", core::stringw(kart.c_str()));

        w += name + L" : ";
        const RaceVote& vote = config->getRaceVote(playerId);
        if (vote.hasVotedTrack())
        {
            w += vote.getTrackVote().c_str();
        }
        else
        {
            w += L"...";
        }

        w += "\n";
    }

    GUIEngine::LabelWidget* lbl = getWidget<GUIEngine::LabelWidget>("lblDetails");
    lbl->setText(w.c_str(), true);
}

// -----------------------------------------------------------------------------

