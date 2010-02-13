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
#include "guiengine/widget.hpp"
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
    
    for (int t=0; t<trackAmount; t++)
    {
        const int from_y      = y1 + height_of_one_line*(t+1);
        const int next_from_y = y1 + height_of_one_line*(t+2);
        
        Track* track = track_manager->getTrack(tracks[t]);
        stringw lineText;
        if (track == NULL)
        {
            //FIXME: what to do if this happens?
            lineText = L"MISSING : ";
            lineText.append( stringw(tracks[t].c_str()) );
        }
        else
        {
            lineText = track->getName();
        }
        
        core::rect< s32 > entry_area(20, from_y, m_area.getWidth()/2, next_from_y);
        IGUIStaticText* line = GUIEngine::getGUIEnv()->addStaticText( lineText.c_str(),
                                               entry_area, false , true , // border, word warp
                                               m_irrlicht_window);
    }

   /*    
    // ---- Track screenshot
    IconButtonWidget* screenshotWidget = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                                                              false, false);
    // images are saved squared, but must be stretched to 4:
    screenshotWidget->setCustomAspectRatio(4.0f / 3.0f); 3 
    core::rect< s32 > area_right(m_area.getWidth()/2, y1, m_area.getWidth(), y1-10);
    
    screenshotWidget->x = area_right.UpperLeftCorner.X;
    screenshotWidget->y = area_right.UpperLeftCorner.Y;
    screenshotWidget->w = area_right.getWidth();
    screenshotWidget->h = area_right.getHeight();
    
    // temporary icon, will replace it just after
    screenshotWidget->m_properties[PROP_ICON] = "gui/main_help.png"; 
    screenshotWidget->setParent(m_irrlicht_window);
    screenshotWidget->add();
    screenshotWidget->setImage(screenshot);
    m_children.push_back(screenshotWidget);
    
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    b->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    */
    
    // ---- Start button
    ButtonWidget* okBtn = new ButtonWidget();
    okBtn->m_properties[PROP_ID] = "start";
    okBtn->m_text = _("Start Grand Prix");
    okBtn->x = m_area.getWidth()/2 - 200;
    okBtn->y = y2;
    okBtn->w = 400;
    okBtn->h = m_area.getHeight() - y2 - 15;
    okBtn->setParent(m_irrlicht_window);
    m_children.push_back(okBtn);
    okBtn->add();
    okBtn->getIrrlichtElement()->setTabStop(true);
    okBtn->getIrrlichtElement()->setTabGroup(false);
    
    okBtn->setFocusForPlayer( GUI_PLAYER_ID );
    
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
    
    IVideoDriver* driver = GUIEngine::getDriver();
    
    //TODO?: draw a loading screen
    driver->endScene();
    driver->beginScene(true, false);
    driver->endScene();
    
    
    StateManager::get()->enterGameState();
    //race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setGrandPrix(*gp);
    race_manager->setCoinTarget( 0 ); // Might still be set from a previous challenge
    //race_manager->setNumKarts( 1 );
    //network_manager->setupPlayerKartInfo();
    //race_manager->getKartType(1) = KT_PLAYER;
    
    race_manager->startNew();
}

// ------------------------------------------------------------------------------------------------------

void GPInfoDialog::onEnterPressedInternal()
{
    startGPGame(grand_prix_manager->getGrandPrix(m_gp_ident));
}

// ------------------------------------------------------------------------------------------------------   

GUIEngine::EventPropagation GPInfoDialog::processEvent(std::string& eventSource)
{
    if (eventSource == "start" )
    {
        startGPGame(grand_prix_manager->getGrandPrix(m_gp_ident));
        return GUIEngine::EVENT_BLOCK;
    }
    
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------
