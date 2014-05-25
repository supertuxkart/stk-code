//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 konstin
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

#include "guiengine/engine.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/dialogs/random_gp_dialog.hpp"
#include "tracks/track_manager.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;
using namespace GUIEngine;

randomGPInfoDialog::randomGPInfoDialog()
{
    // Defaults - loading selection from last time frrom a file would be better
    m_number_of_tracks = 2;
    m_trackgroup = "all";
    m_use_reverse = true;

    doInit();
    m_curr_time = 0.0f;

    int y1 = m_area.getHeight()/7;

    m_gp_ident = "random";
    m_gp = new GrandPrixData(m_number_of_tracks, m_trackgroup, m_use_reverse);

    // ---- GP Name
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* title = GUIEngine::getGUIEnv()->addStaticText(translations->fribidize("Random Grand Prix"),
                                                               area_top, false, true, // border, word wrap
                                                               m_irrlicht_window);
    title->setTabStop(false);
    title->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

    // ---- Spinners
    // Trackgroup chooser
    GUIEngine::SpinnerWidget* spinner = new GUIEngine::SpinnerWidget(false);
    spinner->m_properties[PROP_ID] = "Trackgroup";
    spinner->setParent(m_irrlicht_window);
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(10, m_area.getHeight()/7, 300, 40);
    // Fill it with with all the track group names
    spinner->addLabel("all");
    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
    for (unsigned int i = 1; i < groups.size() + 1; i++)
        spinner->addLabel(stringw(groups[i].c_str()));

    // Number of laps chooser
    spinner = new GUIEngine::SpinnerWidget(false);
    spinner->setValue(m_number_of_tracks);
    spinner->setMin(1);
    spinner->setMax(track_manager->getNumberOfTracks()); // default is "all"
    spinner->setParent(m_irrlicht_window);
    spinner->m_properties[PROP_ID] = "Number of tracks";
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(310, m_area.getHeight()/7, 200, 40);

    displayTracks(m_area.getHeight()/7 + 50, m_area.getHeight()*6/7);
    InitAfterDrawingTheHeader(m_area.getHeight()/7 + 50,
                              m_area.getHeight()*6/7,
                              "random");
}

GUIEngine::EventPropagation randomGPInfoDialog::processEvent(
                                                 const std::string& eventSource)
{
    if (eventSource == "Number of tracks")
    {
        m_number_of_tracks = getWidget<GUIEngine::SpinnerWidget>("Number of tracks")->getValue();
        updateGP();
    }
    else if (eventSource == "Trackgroup")
    {
        GUIEngine::SpinnerWidget* t = getWidget<GUIEngine::SpinnerWidget>("Trackgroup");
        GUIEngine::SpinnerWidget* s = getWidget<GUIEngine::SpinnerWidget>("Number of tracks");

        m_trackgroup = stringc(t->getStringValue()).c_str();

        // Update the maximum for the number of tracks since it's depending on
        // the current track. The current value in the Number-of-tracks-spinner
        // has to be updated, since otherwise the displayed (and used) value
        // can be bigger than the maximum. (Might be a TODO to fix this)
        unsigned int max = (m_trackgroup == "all") ?
                           track_manager->getNumberOfTracks() :
                           track_manager->getTracksInGroup(m_trackgroup).size();
        m_number_of_tracks = std::min(max, m_number_of_tracks);
        s->setMax(max);
        if (s->getValue() > (signed)max)
            s->setValue(max);

        updateGP();
    }

    return GPInfoDialog::processEvent(eventSource);
}

void randomGPInfoDialog::updateGP()
{
    if (m_gp != NULL)
        delete m_gp;

    m_gp = new GrandPrixData(m_number_of_tracks, m_trackgroup, m_use_reverse);
    displayTracks(m_area.getHeight()/7, m_area.getHeight()*6/7);
}

