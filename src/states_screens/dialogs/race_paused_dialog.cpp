//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "states_screens/dialogs/race_paused_dialog.hpp"

#include <string>

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "modes/overworld.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

// ----------------------------------------------------------------------------

RacePausedDialog::RacePausedDialog(const float percentWidth,
                                   const float percentHeight) :
    ModalDialog(percentWidth, percentHeight)
{
    if (dynamic_cast<OverWorld*>(World::getWorld()) != NULL)
    {
        loadFromFile("overworld_dialog.stkgui");
    }
    else
    {
        loadFromFile("race_paused_dialog.stkgui");
    }

    World::getWorld()->schedulePause(WorldStatus::IN_GAME_MENU_PHASE);

    IconButtonWidget* back_btn = getWidget<IconButtonWidget>("backbtn");
    back_btn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
}   // RacePausedDialog

// ----------------------------------------------------------------------------
RacePausedDialog::~RacePausedDialog()
{
    World::getWorld()->scheduleUnpause();
}   // ~RacePausedDialog

// ----------------------------------------------------------------------------

void RacePausedDialog::loadedFromFile()
{
    // disable the "restart" button in GPs
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        GUIEngine::RibbonWidget* choice_ribbon =
            getWidget<GUIEngine::RibbonWidget>("choiceribbon");
        const bool success = choice_ribbon->deleteChild("restart");
        assert(success);
    }
    // Remove "endrace" button for types not (yet?) implemented
    // Also don't show it unless the race has started. Prevents finishing in
    // a time of 0:00:00.
    if ((race_manager->getMinorMode() != RaceManager::MINOR_MODE_NORMAL_RACE  &&
         race_manager->getMinorMode() != RaceManager::MINOR_MODE_TIME_TRIAL ) ||
         World::getWorld()->isStartPhase())
    {
        GUIEngine::RibbonWidget* choice_ribbon =
            getWidget<GUIEngine::RibbonWidget>("choiceribbon");
        choice_ribbon->deleteChild("endrace");
    }
}

// ----------------------------------------------------------------------------

void RacePausedDialog::onEnterPressedInternal()
{
}   // onEnterPressedInternal

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation
           RacePausedDialog::processEvent(const std::string& eventSource)
{
    GUIEngine::RibbonWidget* choice_ribbon =
            getWidget<GUIEngine::RibbonWidget>("choiceribbon");

    if (eventSource == "backbtn")
    {
        // unpausing is done in the destructor so nothing more to do here
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "choiceribbon")
    {
        const std::string& selection =
            choice_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "exit")
        {
            ModalDialog::dismiss();
            race_manager->exitRace();
            race_manager->setAIKartOverride("");
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());

            if (race_manager->raceWasStartedFromOverworld())
            {
                OverWorld::enterOverWorld();
            }

            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "help")
        {
            dismiss();
            HelpScreen1::getInstance()->push();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "options")
        {
            dismiss();
            OptionsScreenVideo::getInstance()->push();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "restart")
        {
            ModalDialog::dismiss();
//            network_manager->setState(NetworkManager::NS_MAIN_MENU);
            World::getWorld()->scheduleUnpause();
            race_manager->rerunRace();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "newrace")
        {
            ModalDialog::dismiss();
            World::getWorld()->scheduleUnpause();
            race_manager->exitRace();
            Screen* newStack[] = {MainMenuScreen::getInstance(),
                                  RaceSetupScreen::getInstance(), NULL};
            StateManager::get()->resetAndSetStack( newStack );
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "endrace")
        {
            ModalDialog::dismiss();
            World::getWorld()->endRaceEarly();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "selectkart")
        {
            dynamic_cast<OverWorld*>(World::getWorld())->scheduleSelectKart();
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------

void RacePausedDialog::beforeAddingWidgets()
{
    GUIEngine::RibbonWidget* choice_ribbon =
        getWidget<GUIEngine::RibbonWidget>("choiceribbon");

    bool showSetupNewRace = race_manager->raceWasStartedFromOverworld();
    int index = choice_ribbon->findItemNamed("newrace");
    if (index != -1)
        choice_ribbon->setItemVisible(index, !showSetupNewRace);
}

// ----------------------------------------------------------------------------
