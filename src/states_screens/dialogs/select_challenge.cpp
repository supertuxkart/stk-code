//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Marianne Gagnon
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

    if (c->getMaxPosition(difficulty) != -1)
    {
        int r = c->getMaxPosition(difficulty);
        if (c->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER) r--;

        if (label.size() > 0) label.append(L"\n");
        label.append( _("Required Rank: %i", r) );
    }
    if (c->getTimeRequirement(difficulty) > 0)
    {
        if (label.size() > 0) label.append(L"\n");
        label.append( _("Required Time: %i",
                        StringUtils::timeToString(c->getTimeRequirement(difficulty)).c_str()) );
    }
    if (c->getEnergy(difficulty) > 0)
    {
        if (label.size() > 0) label.append(L"\n");
        label.append( _("Required Nitro Points: %i", c->getEnergy(difficulty)) );
    }

    if (!c->isGhostReplay())
    {
        if (label.size() > 0) label.append(L"\n");
        label.append(_("Number of AI Karts: %i", c->getNumKarts(difficulty) - 1));
    }

    return label;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

SelectChallengeDialog::SelectChallengeDialog(const float percentWidth,
                                             const float percentHeight,
                                             std::string challenge_id) :
    ModalDialog(percentWidth, percentHeight)
{
    if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
        loadFromFile("select_challenge_nobest.stkgui");
    else
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
        case 3:
        {
            if(PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
                getWidget("expert")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            else
                getWidget("supertux")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            break;
        }
    }

    const ChallengeStatus* c = PlayerManager::getCurrentPlayer()
                             ->getChallengeStatus(challenge_id);

    updateSolvedIcon(c, RaceManager::DIFFICULTY_EASY,   "novice",       "cup_bronze.png");
    updateSolvedIcon(c, RaceManager::DIFFICULTY_MEDIUM, "intermediate", "cup_silver.png");
    updateSolvedIcon(c, RaceManager::DIFFICULTY_HARD,   "expert",       "cup_gold.png");
    updateSolvedIcon(c, RaceManager::DIFFICULTY_BEST,   "supertux",     "cup_platinum.png");

    LabelWidget* novice_label = getWidget<LabelWidget>("novice_label");
    LabelWidget* medium_label = getWidget<LabelWidget>("intermediate_label");
    LabelWidget* expert_label = getWidget<LabelWidget>("difficult_label");

    novice_label->setText( getLabel(RaceManager::DIFFICULTY_EASY,   c->getData()), false );
    medium_label->setText( getLabel(RaceManager::DIFFICULTY_MEDIUM, c->getData()), false );
    expert_label->setText( getLabel(RaceManager::DIFFICULTY_HARD,   c->getData()), false );

    if (!PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
    {
        LabelWidget* supertux_label = getWidget<LabelWidget>("supertux_label");
        supertux_label->setText( getLabel(RaceManager::DIFFICULTY_BEST,   c->getData()), false );
    }

    if (c->getData()->isGrandPrix())
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(c->getData()->getGPId());
        getWidget<LabelWidget>("title")->setText(translations->fribidize(gp->getName()), true);
    }
    else
    {
        const core::stringw track_name =
            track_manager->getTrack(c->getData()->getTrackId())->getName();
        getWidget<LabelWidget>("title")->setText(translations->fribidize(track_name), true);
    }

    LabelWidget* typeLbl = getWidget<LabelWidget>("race_type_val");
    core::stringw description;

    if (c->getData()->isGrandPrix())
    {
        // Doesn't work for RTL
        description = _("Grand Prix");
        description += L" - ";
        description += RaceManager::getNameOf(c->getData()->getMinorMode());
    } // if isGrandPrix
    else
    {
        if (c->getData()->getEnergy(RaceManager::DIFFICULTY_EASY) > 0)
            description = _("Nitro challenge");
        else if (c->getData()->isGhostReplay())
            description = _("Ghost replay race");
        else
            description = RaceManager::getNameOf(c->getData()->getMinorMode());

        description += L" - ";
        description += _("Laps: %i", c->getData()->getNumLaps());

        if (c->getData()->getReverse())
        {
            description += L" - ";
            description += _("Mode: Reverse");
        }
    } // if !isGrandPrix
    typeLbl->setText(description, false );
}

// ----------------------------------------------------------------------------

SelectChallengeDialog::~SelectChallengeDialog()
{
    World::getWorld()->scheduleUnpause();
}

// ----------------------------------------------------------------------------

void SelectChallengeDialog::updateSolvedIcon(const ChallengeStatus* c, RaceManager::Difficulty diff,
                                             const char* widget_name, const char* path)
{
    if (c->isSolved(diff))
    {
        IconButtonWidget* btn = getWidget<IconButtonWidget>(widget_name);
        btn->setImage(file_manager->getAsset(FileManager::GUI_ICON, path),
                     IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }
} //updateSolvedIcon

// -----------------------------------------------------------------------------
void SelectChallengeDialog::onUpdate(float dt)
{
    if (m_self_destroy)
    {
        ModalDialog::clearWindow();
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation SelectChallengeDialog::processEvent(const std::string& eventSourceParam)
{
    std::string eventSource = eventSourceParam;
    if (eventSource == "back")
    {
        m_self_destroy = true;
        return GUIEngine::EVENT_BLOCK;
    }

    if (eventSource == "icon_novice" || eventSource == "icon_intermediate" ||
        eventSource == "icon_expert" || eventSource == "icon_supertux")
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
#ifdef DEBUG
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        assert(device != NULL);
#endif
        // Set up race manager appropriately
        race_manager->setNumPlayers(1);
        race_manager->setPlayerKart(0, UserConfigParams::m_default_kart);

        //int id = StateManager::get()->createActivePlayer( unlock_manager->getCurrentPlayer(), device );
        input_manager->getDeviceManager()->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

        // ASSIGN should make sure that only input from assigned devices is read.
        input_manager->getDeviceManager()->setAssignMode(ASSIGN);

        // Go straight to the race
        StateManager::get()->enterGameState();

        // Initialise global data - necessary even in local games to avoid
        // many if tests in other places (e.g. if network_game call
        // network_manager else call race_manager).
        // network_manager->initCharacterDataStructures();

        // Launch challenge
        if (eventSource == "icon_novice")
        {
            challenge->setRace(RaceManager::DIFFICULTY_EASY);
            UserConfigParams::m_difficulty = 0;
        }
        else if (eventSource == "icon_intermediate")
        {
            challenge->setRace(RaceManager::DIFFICULTY_MEDIUM);
            UserConfigParams::m_difficulty = 1;
        }
        else if (eventSource == "icon_expert")
        {
            challenge->setRace(RaceManager::DIFFICULTY_HARD);
            UserConfigParams::m_difficulty = 2;
        }
        else if (eventSource == "icon_supertux")
        {
            challenge->setRace(RaceManager::DIFFICULTY_BEST);
            UserConfigParams::m_difficulty = 3;
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
