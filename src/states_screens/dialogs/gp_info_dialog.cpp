//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "states_screens/dialogs/gp_info_dialog.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/saved_grand_prix.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

using irr::gui::IGUIStaticText;
using GUIEngine::PROP_ID;

typedef GUIEngine::LabelWidget Label;

const float GPInfoDialog::PERCENT_WIDTH  = 0.8f;
const float GPInfoDialog::PERCENT_HEIGHT = 0.7f;

GPInfoDialog::GPInfoDialog(const std::string& gp_ident)
            : ModalDialog(PERCENT_WIDTH, PERCENT_HEIGHT)
{
    doInit();
    m_curr_time = 0.0f;

    m_gp = *grand_prix_manager->getGrandPrix(gp_ident);
    m_gp.checkConsistency();

    m_under_title = m_area.getHeight()/7;
    m_over_body = m_area.getHeight()/7;
    m_lower_bound = m_area.getHeight()*6/7;

    addTitle();
    addTracks();
    addScreenshot();
    addButtons();
}

// ----------------------------------------------------------------------------

GPInfoDialog::~GPInfoDialog()
{
    GUIEngine::Screen* curr_screen = GUIEngine::getCurrentScreen();
    if (curr_screen->getName() == "tracks.stkgui")
        static_cast<TracksScreen*>(curr_screen)->setFocusOnGP(m_gp.getId());
}

// ----------------------------------------------------------------------------

void GPInfoDialog::addTitle()
{
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), m_under_title);
    IGUIStaticText* title = GUIEngine::getGUIEnv()->addStaticText(
        translations->fribidize(m_gp.getName()),
        area_top, false, true, // border, word wrap
        m_irrlicht_window);
    title->setTabStop(false);
    title->setTextAlignment(irr::gui::EGUIA_CENTER, irr::gui::EGUIA_CENTER);
}

// ----------------------------------------------------------------------------

void GPInfoDialog::addTracks()
{
    const std::vector<std::string> tracks = m_gp.getTrackNames();
    const unsigned int track_amount = tracks.size();

    int height_of_one_line = std::min((m_lower_bound - m_over_body)/(track_amount+1),
                                      (unsigned int)(GUIEngine::getFontHeight()*1.5f));

    // Count the number of label already existing labels representing a track
    unsigned int existing = 0;
    for (unsigned int i = 0; i < m_widgets.size(); i++)
    {
        if (m_widgets.get(i)->m_properties[PROP_ID] == "Track label")
            existing++;
    }

    unsigned int reuse = std::min(existing, track_amount);
    // m_widgets has the type PtrVector<Widget, HOLD>
    unsigned int widgets_iter = 0;
    for (unsigned int i = 0; i < reuse; i++)
    {
        Track* track = track_manager->getTrack(tracks[i]);

        // Find the next widget that is a track label
        while (m_widgets.get(widgets_iter)->m_properties[PROP_ID] != "Track label")
            widgets_iter++;

        Label* widget = dynamic_cast<Label*>(m_widgets.get(widgets_iter));
        widget->setText(translations->fribidize(track->getName()), false);
        widget->move(20, m_over_body + height_of_one_line*i,
                     m_area.getWidth()/2 - 20, height_of_one_line);

        widgets_iter++;
    }

    if (existing < track_amount)
    {
        // There are not enough labels for all the track names, so we have to
        // add some more
        for (unsigned int i = reuse; i < track_amount; i++)
        {
            Track* track = track_manager->getTrack(tracks[i]);
            assert(track != NULL);

            Label* widget = new Label();
            widget->m_properties[PROP_ID] = "Track label";
            widget->setText(translations->fribidize(track->getName()), false);
            widget->setParent(m_irrlicht_window);
            m_widgets.push_back(widget);
            widget->add();

            widget->move(20, m_over_body + height_of_one_line*i,
                         m_area.getWidth()/2 - 20, height_of_one_line);
        }
    }
    else if (existing > track_amount)
    {
        // There are label which are not necessary anymore so they're deleted
        for (unsigned int i = widgets_iter; i < m_widgets.size(); i++)
        {
            if (m_widgets.get(i)->m_properties[PROP_ID] == "Track label")
            {
                m_irrlicht_window->removeChild(m_widgets.get(i)->getIrrlichtElement());
                m_widgets.remove(i);
                i--;
            }
        }
    }
}

// ----------------------------------------------------------------------------

void GPInfoDialog::addScreenshot()
{
    m_screenshot_widget = new GUIEngine::IconButtonWidget(
        GUIEngine::IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
        false, false, GUIEngine::IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    // images are saved squared, but must be stretched to 4:3
    m_screenshot_widget->setCustomAspectRatio(4.0f / 3.0f);

    m_screenshot_widget->m_x = m_area.getWidth()/2-20;
    m_screenshot_widget->m_y = m_over_body + 10;

    // Scale the picture to the biggest possible size without an overflow
    if (m_lower_bound - m_over_body - 20 < m_area.getWidth()/2*3/4)
    {
        m_screenshot_widget->m_w = (m_lower_bound - m_over_body - 30)*4/3;
        m_screenshot_widget->m_h = m_lower_bound - m_over_body - 30;
    }
    else
    {
        m_screenshot_widget->m_w = m_area.getWidth()/2;
        m_screenshot_widget->m_h = m_area.getWidth()*3/8; // *(3/4)*(1/2)
    }

    Track* track = track_manager->getTrack(m_gp.getTrackNames()[0]);
    m_screenshot_widget->m_properties[GUIEngine::PROP_ICON] = (track->getScreenshotFile().c_str());
    m_screenshot_widget->setParent(m_irrlicht_window);
    m_screenshot_widget->add();
    m_widgets.push_back(m_screenshot_widget);
}


// ----------------------------------------------------------------------------
void GPInfoDialog::addButtons()
{
    // ---- Start button
    GUIEngine::ButtonWidget* okBtn = new GUIEngine::ButtonWidget();
    GUIEngine::ButtonWidget* continueBtn = new GUIEngine::ButtonWidget();

    SavedGrandPrix* saved_gp = SavedGrandPrix::getSavedGP( StateManager::get()
                                               ->getActivePlayerProfile(0)
                                               ->getUniqueID(),
                                               m_gp.getId(),
                                               race_manager->getNumLocalPlayers());

    okBtn->m_properties[PROP_ID] = "start";
    okBtn->setText(_("Start Grand Prix"));

    continueBtn->m_properties[PROP_ID] = "continue";
    continueBtn->setText(_("Continue"));

    if (saved_gp)
    {
        continueBtn->m_x = m_area.getWidth()/2 + 110;
        continueBtn->m_y = m_lower_bound;
        continueBtn->m_w = 200;
        continueBtn->m_h = m_area.getHeight() - m_lower_bound - 15;
        continueBtn->setParent(m_irrlicht_window);
        m_widgets.push_back(continueBtn);
        continueBtn->add();
        continueBtn->getIrrlichtElement()->setTabStop(true);
        continueBtn->getIrrlichtElement()->setTabGroup(false);

        okBtn->m_x = m_area.getWidth()/2 - 310;
    }
    else
    {
        okBtn->m_x = m_area.getWidth()/2 - 200;
    }

    okBtn->m_y = m_lower_bound;
    okBtn->m_w = 400;
    okBtn->m_h = m_area.getHeight() - m_lower_bound - 15;
    okBtn->setParent(m_irrlicht_window);
    m_widgets.push_back(okBtn);
    okBtn->add();
    okBtn->getIrrlichtElement()->setTabStop(true);
    okBtn->getIrrlichtElement()->setTabGroup(false);

    okBtn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
}

// ----------------------------------------------------------------------------

void GPInfoDialog::onEnterPressedInternal()
{
    // Save the GP id because dismiss() will destroy this instance
    std::string gp_id = m_gp.getId();
    ModalDialog::dismiss();
    // Disable accidentally unlocking of a challenge
    PlayerManager::getCurrentPlayer()->setCurrentChallenge("");
    race_manager->startGP(*grand_prix_manager->getGrandPrix(gp_id), false, false);
}

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation GPInfoDialog::processEvent(const std::string& event_source)
{
    if (event_source == "start" || event_source == "continue")
    {
        // Save GP identifier, since dismiss will delete this object.
        std::string gp_id = m_gp.getId();
        // Also create a copy of the string: it is a reference to data
        // in a widget in the dialog - so if we call dismiss, this reference
        // becomes invalid!
        std::string save_source = event_source;
        ModalDialog::dismiss();
        //race_manager->startGP(grand_prix_manager->getGrandPrix(gp_id), false,
        //                      (save_source == "continue"));
        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}

// ----------------------------------------------------------------------------

void GPInfoDialog::onUpdate(float dt)
{
    if (dt == 0)
        return; // if nothing changed, return right now

    m_curr_time += dt;
    int frameAfter = (int)(m_curr_time / 1.5f);

    const std::vector<std::string> tracks = m_gp.getTrackNames();
    if (frameAfter >= (int)tracks.size())
    {
        frameAfter = 0;
        m_curr_time = 0;
    }

    Track* track = track_manager->getTrack(tracks[frameAfter]);
    std::string file = track->getScreenshotFile();
    typedef GUIEngine::IconButtonWidget Icon;
    m_screenshot_widget->setImage(file.c_str(), Icon::ICON_PATH_TYPE_ABSOLUTE);
}

