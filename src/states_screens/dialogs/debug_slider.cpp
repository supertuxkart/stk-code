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

DebugSliderDialog::DebugSliderDialog() : ModalDialog(0.85f, 0.45f, MODAL_DIALOG_LOCATION_BOTTOM)
{
    m_fade_background = false;

    loadFromFile("debug_slider.stkgui");
}

void DebugSliderDialog::setSliderHook(std::string id, unsigned min, unsigned max, std::function<int()> G, std::function<void(int)> S)
{
    getWidget<SpinnerWidget>(id.c_str())->setValue(G());
    getWidget<SpinnerWidget>(id.c_str())->setMin(min);
    getWidget<SpinnerWidget>(id.c_str())->setMax(max);
    Setters[id] = S;
}

// ------------------------------------------------------------------------------------------------------

void DebugSliderDialog::changeLabel(std::string id, std::string new_label)
{
    getWidget<LabelWidget>(id.c_str())->setText(stringw(new_label.c_str()), true);
}

// ------------------------------------------------------------------------------------------------------

void DebugSliderDialog::toggleSlider(std::string id, bool option)
{
    getWidget<SpinnerWidget>(id.c_str())->setActive(option);
}

// ------------------------------------------------------------------------------------------------------

void DebugSliderDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation DebugSliderDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "close")
    {
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }

    if (Setters.find(eventSource) == Setters.end())
        return GUIEngine::EVENT_LET;

    int value = getWidget<SpinnerWidget>(eventSource.c_str())->getValue();
    Log::info("DebugSlider", "Value for <%s> : %i", eventSource.c_str(), value);
    Setters[eventSource](value);
    return GUIEngine::EVENT_BLOCK;
}

// ------------------------------------------------------------------------------------------------------

void DebugSliderDialog::onUpdate(float dt)
{
}
