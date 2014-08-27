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

#include "states_screens/gp_info_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/saved_grand_prix.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
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

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( GPInfoScreen );

GPInfoScreen::GPInfoScreen() : Screen("gp_info.stkgui")
{
    m_gp        = NULL;
    m_curr_time = 0.0f;
}   // GPInfoScreen

// ----------------------------------------------------------------------------

GPInfoScreen::~GPInfoScreen()
{
}   // ~GPInfoScreen

// ----------------------------------------------------------------------------
/** Sets the GP to be displayed.
 */
void GPInfoScreen::setGP(const std::string &gp_ident)
{
    m_gp = grand_prix_manager->getGrandPrix(gp_ident);
}   // setGP

// ----------------------------------------------------------------------------
void GPInfoScreen::init()
{
    Screen::init();
    m_curr_time = 0.0f;

    getWidget<LabelWidget>("name")->setText(m_gp->getName(), false);

    m_gp->checkConsistency();

    m_over_body   = UserConfigParams::m_height/7;
    m_lower_bound = UserConfigParams::m_height*6/7;

    addTracks();
    addScreenshot();

    CheckBoxWidget *reverse = getWidget<CheckBoxWidget>("reverse");
    reverse->setState(false);

    // Check if there is a saved GP:
    SavedGrandPrix* saved_gp = 
        SavedGrandPrix::getSavedGP(StateManager::get()
                                   ->getActivePlayerProfile(0)->getUniqueID(),
                                   m_gp->getId(),
                                   race_manager->getDifficulty(),
                                   race_manager->getNumberOfKarts(),
                                   race_manager->getNumLocalPlayers());
    
    getWidget<IconButtonWidget>("continue")->setVisible(saved_gp!=NULL);
}   // init

// ----------------------------------------------------------------------------
void GPInfoScreen::loadedFromFile()
{
}   // loadedFromFile

// ----------------------------------------------------------------------------

void GPInfoScreen::addTracks()
{
    const std::vector<std::string> tracks = m_gp->getTrackNames();

    ListWidget *list = getWidget<ListWidget>("tracks");
    list->clear();
    for(unsigned int i=0; i<tracks.size(); i++)
    {
        const Track *track = track_manager->getTrack(tracks[i]);
        std::string s = StringUtils::toString(i);
        list->addItem(s, translations->fribidize(track->getName()) );
    }
}   // addTracks

// ----------------------------------------------------------------------------

void GPInfoScreen::addScreenshot()
{
    Widget* screenshot_div = getWidget("screenshot_div");

    m_screenshot_widget = new GUIEngine::IconButtonWidget(
        GUIEngine::IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
        false, false, GUIEngine::IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    // images are saved squared, but must be stretched to 4:3
    m_screenshot_widget->setCustomAspectRatio(4.0f / 3.0f);
    m_screenshot_widget->m_x = screenshot_div->m_x;
    m_screenshot_widget->m_y = screenshot_div->m_y;
    m_screenshot_widget->m_w = screenshot_div->m_w;
    m_screenshot_widget->m_h = screenshot_div->m_h;


    // Temporary icon, will replace it just after 
    // (but it will be shown if the given icon is not found)
    m_screenshot_widget->m_properties[PROP_ICON] = "gui/main_help.png";
    m_screenshot_widget->add();

    const Track *track = track_manager->getTrack(m_gp->getTrackId(0));
    video::ITexture* screenshot = irr_driver->getTexture(track->getScreenshotFile(),
                                    "While loading screenshot for track '%s':",
                                           track->getFilename()            );
    if (screenshot != NULL)
        m_screenshot_widget->setImage(screenshot);
    m_widgets.push_back(m_screenshot_widget);
}   // addScreenShot

// ----------------------------------------------------------------------------

void GPInfoScreen::onEnterPressedInternal()
{
    // Save the GP id because dismiss() will destroy this instance
    std::string gp_id = m_gp->getId();
//FIXME    ModalDialog::dismiss();
    // Disable accidentally unlocking of a challenge
    PlayerManager::getCurrentPlayer()->setCurrentChallenge("");
    race_manager->startGP(grand_prix_manager->getGrandPrix(gp_id), false, false);
}

// ----------------------------------------------------------------------------

void GPInfoScreen::eventCallback(GUIEngine::Widget *, const std::string &name,
                                 const int player_id)
{
    if(name=="buttons")
    {
        const std::string &button = getWidget<GUIEngine::RibbonWidget>("buttons")
                                  ->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (button == "start")
        {
            // Save GP identifier, since dismiss will delete this object.
            std::string gp_id = m_gp->getId();
            // Also create a copy of the string: it is a reference to data
            // in a widget in the dialog - so if we call dismiss, this reference
            // becomes invalid!
            std::string save_source = name;
            // FIXME ModalDialog::dismiss();
            race_manager->startGP(grand_prix_manager->getGrandPrix(gp_id), false,
                (save_source == "continue"));
        }
    }   // name=="buttons"

}   // eventCallback

// ----------------------------------------------------------------------------

void GPInfoScreen::onUpdate(float dt)
{
    if (dt == 0)
        return; // if nothing changed, return right now

    m_curr_time += dt;
    int frameAfter = (int)(m_curr_time / 1.5f);

    const std::vector<std::string> tracks = m_gp->getTrackNames();
    if (frameAfter >= (int)tracks.size())
    {
        frameAfter = 0;
        m_curr_time = 0;
    }

    Track* track = track_manager->getTrack(tracks[frameAfter]);
    std::string file = track->getScreenshotFile();
    m_screenshot_widget->setImage(file, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    m_screenshot_widget->m_properties[GUIEngine::PROP_ICON] = file;
}   // onUpdate