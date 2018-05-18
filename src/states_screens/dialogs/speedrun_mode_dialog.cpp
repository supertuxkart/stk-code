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

#include "states_screens/dialogs/speedrun_mode_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;
using namespace irr::core;

// ------------------------------------------------------------------------------------------------------

SpeedrunModeDialog::SpeedrunModeDialog() : ModalDialog(0.7f, 0.7f)
{
    loadFromFile("speedrun_mode_dialog.stkgui");
}

// ------------------------------------------------------------------------------------------------------

void SpeedrunModeDialog::onEnterPressedInternal()
{
    onEscapePressed();
    //FIXME : the dismiss avoids the dialog being repeated
    //        but it flashes once before disappearing
    ModalDialog::dismiss();
}

// ----------------------------------------------------------------------------

bool SpeedrunModeDialog::onEscapePressed()
{
    ModalDialog::dismiss();
    return true;
}   // escapePressed

// ""

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation SpeedrunModeDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "accept")
    {
        ModalDialog::dismiss();

        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}
