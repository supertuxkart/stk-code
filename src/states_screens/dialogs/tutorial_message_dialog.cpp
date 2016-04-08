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

#include "states_screens/dialogs/tutorial_message_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ------------------------------------------------------------------------------------------------------

TutorialMessageDialog::TutorialMessageDialog(irr::core::stringw msg, bool stopGame) :
    ModalDialog(0.85f, 0.25f, MODAL_DIALOG_LOCATION_BOTTOM)
{
    m_stop_game = stopGame;

    if (stopGame && StateManager::get()->getGameState() == GUIEngine::GAME)
    {
        World::getWorld()->schedulePause(World::IN_GAME_MENU_PHASE);
    }


    loadFromFile("tutorial_message_dialog.stkgui");


    LabelWidget* message = getWidget<LabelWidget>("title");
    message->setText( msg.c_str(), false );

    ButtonWidget* cancelbtn = getWidget<ButtonWidget>("continue");
    cancelbtn->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    
    World::getWorld()->getKart(0)->getControls().reset();
}

// ------------------------------------------------------------------------------------------------------

TutorialMessageDialog::~TutorialMessageDialog()
{
    if (m_stop_game && StateManager::get()->getGameState() == GUIEngine::GAME)
    {
        World::getWorld()->scheduleUnpause();
    }
}

// ------------------------------------------------------------------------------------------------------

void TutorialMessageDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation TutorialMessageDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "continue")
    {
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------

void TutorialMessageDialog::onUpdate(float dt)
{
}
