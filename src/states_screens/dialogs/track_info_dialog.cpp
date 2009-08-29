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
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
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

    // ======== High Scores
    std::string game_mode_ident = RaceManager::getIdentOf( race_manager->getMinorMode() );
    const HighscoreEntry::HighscoreType type = "HST_" + game_mode_ident;
    
    HighscoreEntry* highscores = highscore_manager->getHighscoreEntry(type,
                                                                      race_manager->getNumKarts(),
                                                                      race_manager->getDifficulty(),
                                                                      trackIdent,
                                                                      race_manager->getNumLaps()); 
    
    // TODO: update highscores display when number of laps changes
    const int amount = highscores->getNumberEntries();
    stringw highscores_string = "= Highscores =\n";
    std::cout << "====== Highscores =====\n";
    std::cout << "Checking for highscores of type " << type.c_str() << " with nkarts=" << race_manager->getNumKarts()
              << ", difficulty=" << race_manager->getDifficulty() << ", track=" << trackIdent.c_str()
              << ", nlaps=" << race_manager->getNumLaps() << std::endl;
    std::cout << "Got " << amount << " entries\n";
    
    std::string kart_name;
    std::string name;
    float time;
    
    char buffer[128];
    for (int n=0; n<3; n++)
    {
        if (n < amount)
        {
            highscores->getEntry(n, kart_name, name, &time);
            
            sprintf(buffer, "%s (%s) : %.2f\n", kart_name.c_str(), name.c_str(), time);
            
            std::cout << buffer << std::endl;
            highscores_string += buffer;
        }
        else
        {
            //I18N : for empty highscores entries
            highscores_string += _("(Empty)");
            highscores_string += "\n";
        }
    }
    std::cout << "======================\n";

    
    Track* track = track_manager->getTrack(trackIdent);
    highscores_string += "\n"; /*+ track->getDescription() + "\n" */
    
    //I18N : when showing who is the author of track '%s'
    sprintf(buffer, _("By %s"), track->getDesigner().c_str());
    highscores_string += buffer;
    
    core::rect< s32 > area_left(0, y1, m_area.getWidth()/2, y2);
    IGUIStaticText* b = GUIEngine::getGUIEnv()->addStaticText( highscores_string.c_str(),
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
