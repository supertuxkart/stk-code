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
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------

core::stringw getLabel(RaceManager::Difficulty difficulty, const ChallengeData* c_data)
{
    core::stringw label, description;
    
    if (c_data->isGrandPrix())
    {
        description += _("Grand Prix");
        description += L" - ";
        description += RaceManager::getNameOf(c_data->getMinorMode());
    } // if isGrandPrix
    else
    {
        if (c_data->getEnergy(RaceManager::DIFFICULTY_EASY) > 0)
        {
            //I18N: In the Select challenge dialog
            description += _("Nitro challenge");
        }
        else if (c_data->isGhostReplay())
        {
            //I18N: In the Select challenge dialog
            description += _("Ghost replay race");
        }
        else
            description += RaceManager::getNameOf(c_data->getMinorMode());

        description += L" - ";
        //I18N: In the Select challenge dialog
        description += _("Laps: %i", c_data->getNumLaps());

        if (c_data->getReverse())
        {
            description += L" - ";
            //I18N: In the Select challenge dialog, tell user this challenge has reversed laps
            description += _("Mode: Reverse");
        }
    } // if !isGrandPrix
    //I18N: In the Select challenge dialog, type of this challenge
    label = _("Type: %s", description);
    
    if (c_data->getMaxPosition(difficulty) != -1)
    {
        int r = c_data->getMaxPosition(difficulty);
        if (c_data->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER) r--;
        label += L"\n";
        //I18N: In the Select challenge dialog
        label += _("Required Rank: %i", r);
    }
    if (c_data->getTimeRequirement(difficulty) > 0)
    {
        label += L"\n";
        //I18N: In the Select challenge dialog
        label += _("Required Time: %i",
                        StringUtils::timeToString(c_data->getTimeRequirement(difficulty)).c_str());
    }
    if (c_data->getEnergy(difficulty) > 0)
    {
        label += L"\n";
        //I18N: In the Select challenge dialog
        label += _("Required Nitro Points: %i", c_data->getEnergy(difficulty));
    }
    if (!c_data->isGhostReplay())
    {
        label += L"\n";
        //I18N: In the Select challenge dialog
        label += _("Number of AI Karts: %i", c_data->getNumKarts(difficulty) - 1);
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
    loadFromFile("select_challenge.stkgui");
    
    m_challenge_id = challenge_id;
    World::getWorld()->schedulePause(WorldStatus::IN_GAME_MENU_PHASE);
    
    GUIEngine::RibbonWidget* difficulty =
        getWidget<GUIEngine::RibbonWidget>("difficulty");
    
    if (UserConfigParams::m_difficulty == RaceManager::DIFFICULTY_BEST &&
        PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
    {
        difficulty->setSelection(RaceManager::DIFFICULTY_HARD, PLAYER_ID_GAME_MASTER);
    }
    else
    {
        difficulty->setSelection( UserConfigParams::m_difficulty, PLAYER_ID_GAME_MASTER );
    }

    const ChallengeStatus* c = PlayerManager::getCurrentPlayer()
                             ->getChallengeStatus(challenge_id);
    LabelWidget* challenge_info = getWidget<LabelWidget>("challenge_info");
    
    switch (UserConfigParams::m_difficulty)
    {
        case 0:
            challenge_info->setText(getLabel(RaceManager::DIFFICULTY_EASY,   c->getData()), false );
            break;
        case 1:
            challenge_info->setText(getLabel(RaceManager::DIFFICULTY_MEDIUM, c->getData()), false );
            break;
        case 2:
            challenge_info->setText(getLabel(RaceManager::DIFFICULTY_HARD,   c->getData()), false );
            break;
        case 3:
            if (UserConfigParams::m_difficulty == RaceManager::DIFFICULTY_BEST &&
                PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
            {
                challenge_info->setText(getLabel(RaceManager::DIFFICULTY_HARD,   c->getData()), false );
            }
            else
            {
                challenge_info->setText(getLabel(RaceManager::DIFFICULTY_BEST,   c->getData()), false );
            }
            break;
    }
    
    updateSolvedIcon(c, RaceManager::DIFFICULTY_EASY,   "novice",       "cup_bronze.png");
    updateSolvedIcon(c, RaceManager::DIFFICULTY_MEDIUM, "intermediate", "cup_silver.png");
    updateSolvedIcon(c, RaceManager::DIFFICULTY_HARD,   "expert",       "cup_gold.png");
    updateSolvedIcon(c, RaceManager::DIFFICULTY_BEST,   "supertux",     "cup_platinum.png");
    
    if (c->getData()->isGrandPrix())
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(c->getData()->getGPId());
        getWidget<LabelWidget>("title")->setText(gp->getName(), true);
    }
    else
    {
        const core::stringw track_name =
            track_manager->getTrack(c->getData()->getTrackId())->getName();
        getWidget<LabelWidget>("title")->setText(track_name, true);
    }

    
    if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
    {
        getWidget<IconButtonWidget>("supertux")->setBadge(LOCKED_BADGE);
        getWidget<IconButtonWidget>("supertux")->setActive(false);
    }
    else
    {
        getWidget<IconButtonWidget>("supertux")->unsetBadge(LOCKED_BADGE);
        getWidget<IconButtonWidget>("supertux")->setActive(true);
    }

    GUIEngine::RibbonWidget* actions =
            getWidget<GUIEngine::RibbonWidget>("actions");
     actions->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
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
    
    GUIEngine::RibbonWidget* actions =
            getWidget<GUIEngine::RibbonWidget>("actions");
    GUIEngine::RibbonWidget* difficulty =
            getWidget<GUIEngine::RibbonWidget>("difficulty");
    
    LabelWidget* challenge_info = getWidget<LabelWidget>("challenge_info");
    
    const ChallengeData* c_data = unlock_manager->getChallengeData(m_challenge_id);
    
    const ChallengeStatus* c_stat = PlayerManager::getCurrentPlayer()
                             ->getChallengeStatus(m_challenge_id);
    
    if (eventSource == "actions")
    {
        const std::string& action =
            actions->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        if(action == "back")
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(action == "start")
        {
            if (c_data == NULL)
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

            RaceManager::get()->exitRace();
            //StateManager::get()->resetActivePlayers();

            // Use latest used device
#ifdef DEBUG
            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
            assert(device != NULL);
#endif
            // Set up race manager appropriately
            RaceManager::get()->setNumPlayers(1);
            RaceManager::get()->setPlayerKart(0, UserConfigParams::m_default_kart);

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
            switch (UserConfigParams::m_difficulty)
            {
                case 0:
                    c_data->setRace(RaceManager::DIFFICULTY_EASY);
                    break;
                case 1:
                    c_data->setRace(RaceManager::DIFFICULTY_MEDIUM);
                    break;
                case 2:
                    c_data->setRace(RaceManager::DIFFICULTY_HARD);
                    break;
                case 3:
                    if (UserConfigParams::m_difficulty == RaceManager::DIFFICULTY_BEST &&
                        PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
                    {
                        c_data->setRace(RaceManager::DIFFICULTY_HARD);
                    }
                    else
                    {
                        c_data->setRace(RaceManager::DIFFICULTY_BEST);
                    }
                    break;
            }
            RaceManager::get()->setupPlayerKartInfo();
            RaceManager::get()->startNew(true);

            irr_driver->hidePointer();

            return GUIEngine::EVENT_BLOCK;
        }
        else
        {
            Log::error("SelectChallenge", "Unknown widget <%s>\n",
                        action.c_str());
            //assert(false);
            return GUIEngine::EVENT_LET;
        }
    }

    if (eventSource == "difficulty")
    {
        const std::string& selected =
            difficulty->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        core::stringw description;

        // Select difficulty
        if (selected == "novice")
        {
            description = getLabel(RaceManager::DIFFICULTY_EASY,   c_stat->getData());
            UserConfigParams::m_difficulty = 0;
        }
        else if (selected == "intermediate")
        {
            description = getLabel(RaceManager::DIFFICULTY_MEDIUM, c_stat->getData());
            UserConfigParams::m_difficulty = 1;
        }
        else if (selected == "expert")
        {
            description = getLabel(RaceManager::DIFFICULTY_HARD,   c_stat->getData());
            UserConfigParams::m_difficulty = 2;
        }
        else if (selected == "supertux")
        {
            description = getLabel(RaceManager::DIFFICULTY_BEST,   c_stat->getData());
            UserConfigParams::m_difficulty = 3;
        }
        else
        {
            Log::error("SelectChallenge", "Unknown widget <%s>\n",
                        selected.c_str());
            //assert(false);
            return GUIEngine::EVENT_LET;
        }

        challenge_info->setText(description, false );
        // Sets up kart info, including random list of kart for AI
    }

    return GUIEngine::EVENT_LET;
}

// ----------------------------------------------------------------------------
