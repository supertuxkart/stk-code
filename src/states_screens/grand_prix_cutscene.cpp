//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 konstin
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

#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "modes/cutscene_world.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/grand_prix_cutscene.hpp"
#include "states_screens/grand_prix_editor_screen.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <string>
#include <vector>

typedef GUIEngine::ButtonWidget Button;

/** A Button to save the GP if it was a random GP */
void GrandPrixCutscene::saveGPButton()
{
    if (RaceManager::get()->getGrandPrix().getId() != GrandPrixData::getRandomGPID())
        getWidget<Button>("save")->setVisible(false);
}   // saveGPButton

// ----------------------------------------------------------------------------

/** \brief Creates a new GP with the same content as the current and saves it
 *  The GP that the race_manager provides can't be used because we need some
 *  functions and settings that the GP manager only gives us through
 *  createNewGP(). */
void GrandPrixCutscene::setNewGPWithName(const irr::core::stringw& name)
{
    // create a new GP with the correct filename and a unique id
    GrandPrixData* gp = grand_prix_manager->createNewGP(name);
    const GrandPrixData current_gp = RaceManager::get()->getGrandPrix();
    std::vector<std::string> tracks  = current_gp.getTrackNames();
    std::vector<int>         laps    = current_gp.getLaps();
    std::vector<bool>        reverse = current_gp.getReverse();
    for (unsigned int i = 0; i < laps.size(); i++)
        gp->addTrack(track_manager->getTrack(tracks[i]), laps[i], reverse[i]);
    gp->writeToFile();

    // Avoid double-save which can have bad side-effects
    getWidget<Button>("save")->setVisible(false);
}   // setNewGPWithName

// ----------------------------------------------------------------------------

void GrandPrixCutscene::eventCallback(GUIEngine::Widget* widget,
                                      const std::string& name,
                                      const int playerID)
{
    if (name == "continue")
    {
        ((CutsceneWorld*)World::getWorld())->abortCutscene();
    }
    else if (name == "save")
    {
        new GeneralTextFieldDialog(_("Please enter the name of the grand prix"),
            std::bind(&GrandPrixCutscene::setNewGPWithName, this,
            std::placeholders::_1), GrandPrixEditorScreen::validateName);
    }
}   // eventCallback

// ----------------------------------------------------------------------------

bool GrandPrixCutscene::onEscapePressed()
{
    ((CutsceneWorld*)World::getWorld())->abortCutscene();
    return false;
}   // onEscapePressed

// ----------------------------------------------------------------------------

void GrandPrixCutscene::tearDown()
{
    Screen::tearDown();
}   // tearDown

