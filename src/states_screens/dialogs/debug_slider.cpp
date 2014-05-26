//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 Marianne Gagnon
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

#include "states_screens/dialogs/debug_slider.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "graphics/irr_driver.hpp"

using namespace GUIEngine;

// ------------------------------------------------------------------------------------------------------

DebugSliderDialog::DebugSliderDialog(std::string id, irr::core::stringw msg) :
    ModalDialog(0.85f, 0.25f, MODAL_DIALOG_LOCATION_BOTTOM)
{
    //if (StateManager::get()->getGameState() == GUIEngine::GAME)
    //{
    //    World::getWorld()->schedulePause(World::IN_GAME_MENU_PHASE);
    //}

    m_id = id;
    m_fade_background = false;

    loadFromFile("debug_slider.stkgui");


    LabelWidget* message = getWidget<LabelWidget>("title");
    message->setText( msg.c_str(), false );

    float val;
    if (m_id == "lwhite")
      val = irr_driver->getLwhite() * 10.f;
    if (m_id == "exposure")
      val = irr_driver->getExposure() * 100.f;

    getWidget<SpinnerWidget>("value_slider")->setValue(int(val));
}

// ------------------------------------------------------------------------------------------------------

DebugSliderDialog::~DebugSliderDialog()
{
    //if (StateManager::get()->getGameState() == GUIEngine::GAME)
    //{
    //    World::getWorld()->scheduleUnpause();
    //}
}

// ------------------------------------------------------------------------------------------------------

void DebugSliderDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation DebugSliderDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "value_slider")
    {
        int value = getWidget<SpinnerWidget>("value_slider")->getValue();
        Log::info("DebugSlider", "Value for <%s> : %i", m_id.c_str(), value);
        if (m_id == "lwhite")
            irr_driver->setLwhite(value / 10.f);
        if (m_id == "exposure")
            irr_driver->setExposure(value / 100.f);
        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------

void DebugSliderDialog::onUpdate(float dt)
{
}
