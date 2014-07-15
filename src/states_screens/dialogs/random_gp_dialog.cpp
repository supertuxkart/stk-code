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
#include "guiengine/widgets/icon_button_widget.hpp"
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
    m_number_of_tracks = 2; // We can assume that there are at least 2 standard tracks
    m_trackgroup = "standard";
    m_use_reverse = GrandPrixData::GP_NO_REVERSE;

    doInit();
    m_curr_time = 0.0f;

    m_under_title = m_area.getHeight()/7;
    m_over_body = m_area.getHeight()/7 + SPINNER_HEIGHT + 10; // 10px space
    m_lower_bound = m_area.getHeight()*6/7;

    m_gp.createRandomGP(m_number_of_tracks, m_trackgroup, m_use_reverse);

    addTitle();
    addSpinners();
    addTracks();
    addScreenshot();
    addButtons();
    addRestartButton();
}

// ----------------------------------------------------------------------------

void RandomGPInfoDialog::addSpinners()
{
    const int trackgroup_width = 200, laps_with = 150, reverse_width = 200;
    const int left =  (m_area.getWidth() - trackgroup_width - 150 - 250)/2;

    // Trackgroup chooser
    Spinner* spinner = new Spinner(false);
    spinner->m_properties[GUIEngine::PROP_ID] = "Trackgroup";
    spinner->m_properties[GUIEngine::PROP_WRAP_AROUND] = "true";
    spinner->setParent(m_irrlicht_window);
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(left, m_under_title, trackgroup_width, SPINNER_HEIGHT);
    // Fill it with all the track group names
    spinner->addLabel("all");
    int index_standard;
    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
    for (unsigned int i = 0; i < groups.size(); i++)
    {
        // FIXME: The NULL check is necessary until #1348 on github is fixed
        if (groups[i].c_str() != NULL)
        {
            spinner->addLabel(stringw(groups[i].c_str()));
            if(groups[i] == "standard")
                index_standard  = i+1;
        }
    }
    // The value can only be set here because SpinnerWidget resets the value
    // every time a label is added
    spinner->setValue(index_standard);

    // Number of laps chooser
    spinner = new Spinner(false);
    spinner->setValue(m_number_of_tracks);
    spinner->setMin(1);
    spinner->setMax(track_manager->getTracksInGroup("standard").size());
    spinner->setParent(m_irrlicht_window);
    spinner->m_properties[GUIEngine::PROP_ID] = "Number of tracks";
    spinner->m_properties[GUIEngine::PROP_WRAP_AROUND] = "true";
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(left + trackgroup_width + 10, m_under_title, laps_with, SPINNER_HEIGHT);

    // reverse choose
    spinner = new Spinner(false);
    spinner->setParent(m_irrlicht_window);
    spinner->m_properties[GUIEngine::PROP_ID] = "reverse";
    spinner->m_properties[GUIEngine::PROP_WRAP_AROUND] = "true";
    m_widgets.push_back(spinner);
    spinner->add();
    spinner->move(left + trackgroup_width + laps_with + 10, m_under_title, reverse_width, SPINNER_HEIGHT);
    spinner->addLabel("no reverse");
    spinner->addLabel("all reverse");
    spinner->addLabel("mixed");
}

// ----------------------------------------------------------------------------

void RandomGPInfoDialog::addRestartButton()
{
    GUIEngine::IconButtonWidget* button = new GUIEngine::IconButtonWidget();
    button->setImage("gui/restart.png");
    button->setParent(m_irrlicht_window);
    button->m_properties[GUIEngine::PROP_ID] = "reload";
    m_widgets.push_back(button);
    button->add();
    button->move(m_area.getWidth() - 20 - 32, 20, 32, 32);
}

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation RandomGPInfoDialog::processEvent(
                                                 const std::string& eventSource)
{
    if (eventSource == "start")
    {
        // Save GP data, since dismiss will delete this object.
        GrandPrixData gp = m_gp;
        ModalDialog::dismiss();
        race_manager->startGP(&gp, false, false);
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "Number of tracks")
    {
        // The old gp can be reused because there's only track deletion/adding
        m_number_of_tracks = getWidget<Spinner>("Number of tracks")->getValue();
        m_gp.changeTrackNumber(m_number_of_tracks, m_trackgroup);
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

        // Create a new (i.e. with new tracks) random gp, since the old
        // tracks might not all belong to the newly selected group.
        m_gp.createRandomGP(m_number_of_tracks, m_trackgroup, m_use_reverse,
                            /*new_tracks*/true);
        addTracks();
    }
    else if (eventSource == "reverse")
    {
        Spinner* r = getWidget<Spinner>("reverse");
        m_use_reverse = static_cast<GrandPrixData::GPReverseType>(r->getValue());
        m_gp.changeReverse(m_use_reverse);
    }
    else if (eventSource == "reload")
    {
        m_gp.createRandomGP(m_number_of_tracks, m_trackgroup, m_use_reverse,
                           /*new_tracks*/true);
        addTracks();
    }

    return GUIEngine::EVENT_LET;
}

