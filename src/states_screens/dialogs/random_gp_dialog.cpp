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
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/random_gp_dialog.hpp"
#include "tracks/track_manager.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

using irr::core::stringc;
using irr::core::stringw;
using irr::gui::IGUIStaticText;

typedef GUIEngine::SpinnerWidget Spinner;

RandomGPInfoDialog::RandomGPInfoDialog()
{
    // Defaults - loading selection from last time frrom a file would be better
    m_number_of_tracks = 2;
    m_trackgroup = "all";
    m_use_reverse = true;

    doInit();
    m_curr_time = 0.0f;

    m_under_title = m_area.getHeight()/7;
    m_over_body = m_area.getHeight()/7 + SPINNER_HEIGHT + 10; // 10px space
    m_lower_bound = m_area.getHeight()*6/7;

    m_gp = new GrandPrixData(m_number_of_tracks, m_trackgroup, m_use_reverse);

    addTitle();
    addSpinners();
    addTracks();
    addScreenshot();
    addButtons();
}

// ----------------------------------------------------------------------------

void RandomGPInfoDialog::addSpinners()
{
    // Trackgroup chooser
    Spinner* spinner = new Spinner(false);
    spinner->m_properties[GUIEngine::PROP_ID] = "Trackgroup";
    spinner->setParent(m_irrlicht_window);
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(10, m_under_title, 300, SPINNER_HEIGHT);
    // Fill it with with all the track group names
    spinner->addLabel("all");
    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
    for (unsigned int i = 1; i < groups.size() + 1; i++)
        spinner->addLabel(stringw(groups[i].c_str()));

    // Number of laps chooser
    spinner = new Spinner(false);
    spinner->setValue(m_number_of_tracks);
    spinner->setMin(1);
    spinner->setMax(track_manager->getNumberOfTracks()); // default is "all"
    spinner->setParent(m_irrlicht_window);
    spinner->m_properties[GUIEngine::PROP_ID] = "Number of tracks";
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(310, m_under_title, 200, SPINNER_HEIGHT);
}

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation RandomGPInfoDialog::processEvent(
                                                 const std::string& eventSource)
{
    if (eventSource == "start")
    {
        // Save the gp since dismiss deletes it otherwise
        GrandPrixData buff = *m_gp;
        ModalDialog::dismiss();
        race_manager->startGP(&buff, false, false);
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "Number of tracks")
    {
        // The old gp can be reused because there's only track deletion/adding
        m_number_of_tracks = getWidget<Spinner>("Number of tracks")->getValue();
        m_gp->changeTrackNumber(m_number_of_tracks, m_trackgroup, m_use_reverse);
        addTracks();
    }
    else if (eventSource == "Trackgroup")
    {
        Spinner* t = getWidget<Spinner>("Trackgroup");
        Spinner* s = getWidget<Spinner>("Number of tracks");

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

    return GUIEngine::EVENT_LET;
}

// ----------------------------------------------------------------------------

void RandomGPInfoDialog::updateGP()
{
    if (m_gp != NULL)
        delete m_gp;

    m_gp = new GrandPrixData(m_number_of_tracks, m_trackgroup, m_use_reverse);
    addTracks();
}

