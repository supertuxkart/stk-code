//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon, 2024 Alayan
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

#include "states_screens/dialogs/recommend_video_settings.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

RecommendVideoSettingsDialog::RecommendVideoSettingsDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("recommend_video_settings.stkgui");
}

// -----------------------------------------------------------------------------

RecommendVideoSettingsDialog::~RecommendVideoSettingsDialog()
{
}

// -----------------------------------------------------------------------------

void RecommendVideoSettingsDialog::beforeAddingWidgets()
{
#ifndef SERVER_ONLY
    getWidget<CheckBoxWidget>("performance")->setState(false);
    getWidget<CheckBoxWidget>("balance")->setState(true);
    getWidget<CheckBoxWidget>("graphics")->setState(false);
    getWidget<CheckBoxWidget>("sparing")->setState(false);

    SpinnerWidget* blur_priority = getWidget<SpinnerWidget>("blur_priority");
    blur_priority->addLabel(_("No"));
    blur_priority->addLabel(_("Yes"));
    blur_priority->setValue(0);
#endif
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation RecommendVideoSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY

    // Make sure that one and only one of the performance-to-graphics checkboxes is always selected.
    if (eventSource == "performance" || eventSource == "balance" || eventSource == "graphics")
    {
        getWidget<CheckBoxWidget>("performance")->setState(false);
        getWidget<CheckBoxWidget>("balance")->setState(false);
        getWidget<CheckBoxWidget>("graphics")->setState(false);
    }

    if (eventSource == "performance")
    {
        getWidget<CheckBoxWidget>("performance")->setState(true);
    }
    else if (eventSource == "balance")
    {
        getWidget<CheckBoxWidget>("balance")->setState(true);
    }
    else if (eventSource == "graphics")
    {
        getWidget<CheckBoxWidget>("graphics")->setState(true);
    }

    else if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "start_test")
        {
            // TODO
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
#endif
    return GUIEngine::EVENT_LET;
}   // processEvent