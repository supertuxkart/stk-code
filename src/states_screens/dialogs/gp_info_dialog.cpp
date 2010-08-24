//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/gp_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"


#include "irrlicht.h"

using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;
using namespace GUIEngine;

// ------------------------------------------------------------------------------------------------------

GPInfoDialog::GPInfoDialog(const std::string& gpIdent, const float w, const float h) : ModalDialog(w, h)
{
    m_curr_time = 0.0f;
    
    const int y1 = m_area.getHeight()/7;
    const int y2 = m_area.getHeight()*6/7;
    
    m_gp_ident = gpIdent;

    const GrandPrixData* gp = grand_prix_manager->getGrandPrix(gpIdent);
    if (gp == NULL)
    {
        assert(false);
        std::cerr << "ERROR at " << __FILE__ << " : " << __LINE__ << "; trying to continue\n";
        ModalDialog::dismiss();
        return;
    }
    
    // ---- GP Name
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* title = GUIEngine::getGUIEnv()->addStaticText( gp->getName().c_str(),
                                                               area_top, false, true, // border, word warp
                                                               m_irrlicht_window);
    title->setTabStop(false);
    title->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);


    // ---- Track listings
    const std::vector<std::string>& tracks = gp->getTracks();
    const int trackAmount = tracks.size();

    int height_of_one_line = (y2 - y1)/(trackAmount+1);
    const int textHeight = GUIEngine::getFontHeight();
    if (height_of_one_line > (int)(textHeight*1.5f)) height_of_one_line = (int)(textHeight*1.5f);
    
    bool gp_ok = true;
    
    for (int t=0; t<trackAmount; t++)
    {
        const int from_y      = y1 + height_of_one_line*(t+1);
        
        Track* track = track_manager->getTrack(tracks[t]);
        stringw lineText;
        if (track == NULL)
        {
            lineText = L"MISSING : ";
            lineText.append( stringw(tracks[t].c_str()) );
            gp_ok = false;
        }
        else
        {
            lineText = track->getName();
        }
                
        LabelWidget* widget = new LabelWidget();
        widget->setText(lineText);
        widget->m_x = 20;
        widget->m_y = from_y;
        widget->m_w = m_area.getWidth()/2 - 20;
        widget->m_h = height_of_one_line;
        widget->setParent(m_irrlicht_window);
        
        m_widgets.push_back(widget);
        widget->add();
        
        // IGUIStaticText* line = GUIEngine::getGUIEnv()->addStaticText( lineText.c_str(),
        //                                       entry_area, false , true , // border, word warp
        //                                       m_irrlicht_window);
    }
  
    // ---- Track screenshot
    
    m_screenshot_widget = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                                               false /* tab stop */, false /* focusable */,
                                               IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE /* Track gives us absolute paths */);
    // images are saved squared, but must be stretched to 4:3
    m_screenshot_widget->setCustomAspectRatio(4.0f / 3.0f);
    
    m_screenshot_widget->m_x = m_area.getWidth()/2;
    m_screenshot_widget->m_y = y1;
    m_screenshot_widget->m_w = m_area.getWidth()/2;
    m_screenshot_widget->m_h = y2 - y1 - 10;
    
    Track* track = track_manager->getTrack(tracks[0]);
    
    m_screenshot_widget->m_properties[PROP_ICON] = (track  != NULL ?
                                                    track->getScreenshotFile().c_str() :
                                                    file_manager->getDataDir() + "gui/main_help.png");
    m_screenshot_widget->setParent(m_irrlicht_window);
    m_screenshot_widget->add();
    m_widgets.push_back(m_screenshot_widget);
    
    
    // ---- Start button
    ButtonWidget* okBtn = new ButtonWidget();
    
    if (gp_ok)
    {
        okBtn->m_properties[PROP_ID] = "start";
        okBtn->setText(_("Start Grand Prix"));
    }
    else
    {
        okBtn->m_properties[PROP_ID] = "cannot_start";
        okBtn->setText(_("This Grand Prix is broken!"));
        okBtn->setBadge(BAD_BADGE);
    }
    
    okBtn->m_x = m_area.getWidth()/2 - 200;
    okBtn->m_y = y2;
    okBtn->m_w = 400;
    okBtn->m_h = m_area.getHeight() - y2 - 15;
    okBtn->setParent(m_irrlicht_window);
    m_widgets.push_back(okBtn);
    okBtn->add();
    okBtn->getIrrlichtElement()->setTabStop(true);
    okBtn->getIrrlichtElement()->setTabGroup(false);
    
    okBtn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
    
}

// ------------------------------------------------------------------------------------------------------

GPInfoDialog::~GPInfoDialog()
{
    // Place focus back on selected GP, in case the dialog was cancelled and we're back to
    // the track selection screen after
    Screen* curr_screen = GUIEngine::getCurrentScreen();
    if (curr_screen->getName() == "tracks.stkgui")
    {
        ((TracksScreen*)curr_screen)->setFocusOnGP(m_gp_ident);
    }
    
}


// ------------------------------------------------------------------------------------------------------

// FIXME : this probably doesn't belong here
void startGPGame(const GrandPrixData* gp)
{
    assert(gp != NULL);
    ModalDialog::dismiss();
    
    //FIXME: simplify and centralize race start sequence!!
    
    StateManager::get()->enterGameState();
    //race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setGrandPrix(*gp);
    race_manager->setCoinTarget( 0 ); // Might still be set from a previous challenge
    //race_manager->setNumKarts( 1 );
    network_manager->setupPlayerKartInfo();
    //race_manager->getKartType(1) = KT_PLAYER;
    
    race_manager->setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
    race_manager->startNew();
}

// ------------------------------------------------------------------------------------------------------

void GPInfoDialog::onEnterPressedInternal()
{
    startGPGame(grand_prix_manager->getGrandPrix(m_gp_ident));
}

// ------------------------------------------------------------------------------------------------------   

GUIEngine::EventPropagation GPInfoDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "start")
    {
        startGPGame(grand_prix_manager->getGrandPrix(m_gp_ident));
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "cannot_start")
    {
        sfx_manager->quickSound( "use_anvil" );
    }
    
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------

void GPInfoDialog::onUpdate(float dt)
{
    const int frameBefore = (int)(m_curr_time / 1.5f);
    m_curr_time += dt;
    int frameAfter = (int)(m_curr_time / 1.5f);
    
    if (frameAfter == frameBefore) return; // if nothing changed, return right now
    
    const GrandPrixData* gp = grand_prix_manager->getGrandPrix(m_gp_ident);
    assert(gp != NULL);
    const std::vector<std::string>& tracks = gp->getTracks();
    if (frameAfter >= (int)tracks.size())
    {
        frameAfter = 0;
        m_curr_time = 0;
    }
    
    Track* track = track_manager->getTrack(tracks[frameAfter]);
    m_screenshot_widget->setImage((track  != NULL ? track->getScreenshotFile().c_str() :
                                                    (file_manager->getDataDir()+"gui/main_help.png").c_str()),
                                  IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
}

// ------------------------------------------------------------------------------------------------------
