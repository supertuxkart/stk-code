//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Marc Coll
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

#include "states_screens/dialogs/scripting_console.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "modes/world.hpp"
#include "scriptengine/script_engine.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------

ScriptingConsole::ScriptingConsole() :
    ModalDialog(0.95f, 0.2f, GUIEngine::MODAL_DIALOG_LOCATION_BOTTOM)
{
    m_fade_background = false;
    loadFromFile("scripting_console.stkgui");

    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    assert(textCtrl != NULL);
    textCtrl->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}

// -----------------------------------------------------------------------------

ScriptingConsole::~ScriptingConsole()
{
    // FIXME: what is this code for?
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    textCtrl->getIrrlichtElement()->remove();
    textCtrl->clearListeners();
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation ScriptingConsole::processEvent(const std::string& eventSource)
{
    if (eventSource == "close")
    {
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "run")
    {
        runScript();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void ScriptingConsole::runScript()
{
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    core::stringw script = textCtrl->getText();
    textCtrl->setText(L"");

    World::getWorld()->getScriptEngine()->evalScript(core::stringc(script.c_str()).c_str());
}

// -----------------------------------------------------------------------------
