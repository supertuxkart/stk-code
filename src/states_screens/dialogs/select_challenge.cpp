//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013 Marianne Gagnon
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

#include "states_screens/dialogs/select_challenge.hpp"

#include "challenges/challenge_status.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------

core::stringw getLabel(RaceManager::Difficulty difficulty, const ChallengeData* c)
{
    core::stringw label;

    if (c->getPosition(difficulty) != -1)
    {
        int r = c->getPosition(difficulty);
        if (c->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER) r--;

        if (label.size() > 0) label.append(L"\n");
        label.append( _("Required Rank : %i", r) );
    }
    if (c->getTime(difficulty) > 0)
    {
        if (label.size() > 0) label.append(L"\n");
        label.append( _("Required Time : %i",
                        StringUtils::timeToString(c->getTime(difficulty)).c_str()) );
    }
    if (c->getEnergy(difficulty) > 0)
    {
        if (label.size() > 0) label.append(L"\n");
        label.append( _("Required Nitro Points : %i", c->getEnergy(difficulty)) );
    }

    if (label.size() > 0) label.append(L"\n");
    label.append(_("Number of AI Karts : %i", c->getNumKarts(difficulty) - 1));

    return label;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

SelectChallengeDialog::SelectChallengeDialog(const float percentWidth,
                                             const float percentHeight,
                                             std::string challenge_id) :
    ModalDialog(percentWidth, percentHeight)
{
    loadFromFile("select_challenge.stkgui");
    m_challenge_id = challenge_id;
    World::getWorld()->schedulePause(WorldStatus::IN_GAME_MENU_PHASE);

    switch (UserConfigParams::m_difficulty)
    {
        case 0:
            getWidget("novice")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            break;
        case 1:
            getWidget("intermediate")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            break;
        case 2:
            getWidget("expert")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            break;
    }

    const ChallengeStatus* c = PlayerManager::getCurrentPlayer()
                             ->getChallengeStatus(challenge_id);

    if (c->isSolved(RaceManager::DIFFICULTY_EASY))
    {
        IconButtonWidget* btn = getWidget<IconButtonWidget>("novice");
        btn->setImage(file_manager->getAsset(FileManager::GUI, "cup_bronze.png"),
                     IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }

    if (c->isSolved(RaceManager::DIFFICULTY_MEDIUM))
    {
        IconButtonWidget* btn = getWidget<IconButtonWidget>("intermediate");
        btn->setImage(file_manager->getAsset(FileManager::GUI,"cup_silver.png"),
                     IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }

    if (c->isSolved(RaceManager::DIFFICULTY_HARD))
    {
        IconButtonWidget* btn = getWidget<IconButtonWidget>("expert");
        btn->setImage(file_manager->getAsset(FileManager::GUI,"cup_gold.png"),
                     IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }


    LabelWidget* novice_label = getWidget<LabelWidget>("novice_label");
    LabelWidget* medium_label = getWidget<LabelWidget>("intermediate_label");
    LabelWidget* expert_label = getWidget<LabelWidget>("difficult_label");

    novice_label->setText( getLabel(RaceManager::DIFFICULTY_EASY,   c->getData()), false );
    medium_label->setText( getLabel(RaceManager::DIFFICULTY_MEDIUM, c->getData()), false );
    expert_label->setText( getLabel(RaceManager::DIFFICULTY_HARD,   c->getData()), false );

    if (c->getData()->isGrandPrix())
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(c->getData()->getGPId());
        getWidget<LabelWidget>("title")->setText( gp->getName(), true );
    }
    else
    {
        const core::stringw track_name =
            track_manager->getTrack(c->getData()->getTrackId())->getName();
        getWidget<LabelWidget>("title")->setText( track_name, true );
    }

    LabelWidget* typeLbl = getWidget<LabelWidget>("race_type_val");
    if (c->getData()->isGrandPrix())
        typeLbl->setText(_("Grand Prix"), false );
    else if (c->getData()->getEnergy(RaceManager::DIFFICULTY_EASY) > 0)
        typeLbl->setText(_("Nitro challenge"), false );
    else
        typeLbl->setText( RaceManager::getNameOf(c->getData()->getMinorMode()), false );

}

// ----------------------------------------------------------------------------

SelectChallengeDialog::~SelectChallengeDialog()
{
    World::getWorld()->scheduleUnpause();
}

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation SelectChallengeDialog::processEvent(const std::string& eventSourceParam)
{
    std::string eventSource = eventSourceParam;
    if (eventSource == "novice" || eventSource == "intermediate" ||
        eventSource == "expert")
    {
        const ChallengeData* challenge = unlock_manager->getChallengeData(m_challenge_id);

        if (challenge == NULL)
        {
            Log::error("SelectChallenge", "Cannot find challenge <%s>\n",
                       m_challenge_id.c_str());
            return GUIEngine::EVENT_LET;
        }

        PlayerManager::getCurrentPlayer()->setCurrentChallenge(m_challenge_id);

        ModalDialog::dismiss();

        core::rect<s32> pos(15,
                            10,
                            15 + UserConfigParams::m_width/2,
                            10 + GUIEngine::getTitleFontHeight());

        race_manager->exitRace();
        //StateManager::get()->resetActivePlayers();

        // Use latest used device
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        assert(device != NULL);

        // Set up race manager appropriately
        race_manager->setNumLocalPlayers(1);
        race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);
        race_manager->setReverseTrack(false);

        //int id = StateManager::get()->createActivePlayer( unlock_manager->getCurrentPlayer(), device );
        input_manager->getDeviceManager()->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

        // ASSIGN should make sure that only input from assigned devices is read.
        input_manager->getDeviceManager()->setAssignMode(ASSIGN);

        // Go straight to the race
        StateManager::get()->enterGameState();

        // Initialise global data - necessary even in local games to avoid
        // many if tests in other places (e.g. if network_game call
        // network_manager else call race_manager).
//        network_manager->initCharacterDataStructures();

        // Launch challenge
        if (eventSource == "novice")
        {
            challenge->setRace(RaceManager::DIFFICULTY_EASY);
            UserConfigParams::m_difficulty = 0;
        }
        else if (eventSource == "intermediate")
        {
            challenge->setRace(RaceManager::DIFFICULTY_MEDIUM);
            UserConfigParams::m_difficulty = 1;
        }
        else if (eventSource == "expert")
        {
            challenge->setRace(RaceManager::DIFFICULTY_HARD);
            UserConfigParams::m_difficulty = 2;
        }
        else
        {
            Log::error("SelectChallenge", "Unknown widget <%s>\n",
                        eventSource.c_str());
            //assert(false);
            return GUIEngine::EVENT_LET;
        }

        // Sets up kart info, including random list of kart for AI
        race_manager->setupPlayerKartInfo();
        race_manager->startNew(true);

        irr_driver->hidePointer();

        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}

// ----------------------------------------------------------------------------

