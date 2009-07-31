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
#include "guiengine/widget.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;


TrackInfoDialog::TrackInfoDialog(const std::string& trackIdent, const char* trackName, ITexture* screenshot, const float w, const float h) : ModalDialog(w, h)
{
    const int y1 = m_area.getHeight()/7;
    const int y2 = m_area.getHeight()*5/7;
    const int y3 = m_area.getHeight()*6/7;
    
    m_track_ident = trackIdent;

    SpinnerWidget* spinner = new SpinnerWidget();
    spinner->x = m_area.getWidth()/2 - 200;
    spinner->y = y2;
    spinner->w = 400;
    spinner->h = y3 - y2 - 15;
    spinner->setParent(m_irrlicht_window);
    
    spinner->m_properties[PROP_MIN_VALUE] = "1";
    spinner->m_properties[PROP_MAX_VALUE] = "99";
    spinner->m_properties[PROP_TEXT] = "%i laps";
    
    m_children.push_back(spinner);
    spinner->add();
    spinner->setValue(3);
    spinner->getIrrlichtElement()->setTabStop(true);
    spinner->getIrrlichtElement()->setTabGroup(false);

    ButtonWidget* okBtn = new ButtonWidget();
    okBtn->m_properties[PROP_ID] = "start";
    okBtn->m_properties[PROP_TEXT] = _("Start Race");
    okBtn->x = m_area.getWidth()/2 - 200;
    okBtn->y = y3;
    okBtn->w = 400;
    okBtn->h = m_area.getHeight() - y3 - 15;
    okBtn->setParent(m_irrlicht_window);
    m_children.push_back(okBtn);
    okBtn->add();
    okBtn->getIrrlichtElement()->setTabStop(true);
    okBtn->getIrrlichtElement()->setTabGroup(false);
    
    GUIEngine::getGUIEnv()->setFocus( okBtn->getIrrlichtElement() );
    
    
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( stringw(trackName).c_str(),
                                                                  area_top, false, true, // border, word warp
                                                                  m_irrlicht_window);
    a->setTabStop(false);

    
    core::rect< s32 > area_left(0, y1, m_area.getWidth()/2, y2);
    IGUIStaticText* b = GUIEngine::getGUIEnv()->addStaticText( stringw(_("High Scores & Track Info")).c_str(),
                                                                  area_left, false , true , // border, word warp
                                                                  m_irrlicht_window);
    b->setTabStop(false);

    
    // TODO : preserve aspect ratio
    core::rect< s32 > area_right(m_area.getWidth()/2, y1, m_area.getWidth(), y2-10);
    IGUIImage* screenshotWidget = GUIEngine::getGUIEnv()->addImage( area_right, m_irrlicht_window );
    screenshotWidget->setImage(screenshot);
    screenshotWidget->setScaleImage(true);
    screenshotWidget->setTabStop(false);


    
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    b->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

}

// ------------------------------------------------------------------------------------------------------

// FIXME : this probably doesn't belong here
void startGame(const std::string trackIdent)
{
    ModalDialog::dismiss();
    
    IVideoDriver* driver = GUIEngine::getDriver();
    
    // TODO : draw a loading screen
    driver->endScene();
    driver->beginScene(true, false);
    driver->endScene();
    
    
    StateManager::get()->enterGameState();
    //race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setTrack(trackIdent.c_str());
    race_manager->setNumLaps( 3 );
    race_manager->setCoinTarget( 0 ); // Might still be set from a previous challenge
    //race_manager->setNumKarts( 1 );
    network_manager->setupPlayerKartInfo();
    //race_manager->getKartType(1) = KT_PLAYER;
    
    race_manager->startNew();
}
    
void TrackInfoDialog::onEnterPressedInternal()
{
    startGame(m_track_ident);
}
    
void TrackInfoDialog::processEvent(std::string& eventSource)
{
    if (eventSource == "start" ) startGame(m_track_ident);
}
