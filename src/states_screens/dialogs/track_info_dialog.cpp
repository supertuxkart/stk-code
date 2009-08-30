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
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/network_manager.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

#include "irrlicht.h"

using namespace irr::gui;
using namespace GUIEngine;


TrackInfoDialog::TrackInfoDialog(const std::string& trackIdent, const char* trackName, ITexture* screenshot, const float w, const float h) : ModalDialog(w, h)
{
    const int y1 = m_area.getHeight()/7;
    const int y2 = m_area.getHeight()*5/7;
    const int y3 = m_area.getHeight()*6/7;
    
    m_track_ident = trackIdent;

    // ---- Lap count spinner
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

    // ---- Start button
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
    
    // ---- Track title
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( stringw(trackName).c_str(),
                                                                  area_top, false, true, // border, word warp
                                                                  m_irrlicht_window);
    a->setTabStop(false);

    // ---- High Scores & track info
    // TODO: update highscores display when number of laps changes

    const int hscores_y_from = y1;
    const int hscores_y_to = y1 + (y2 - y1)*2/3;

    core::rect< s32 > hiscores_title_area(5, hscores_y_from, m_area.getWidth()/2, hscores_y_from + 30);
    stringw text = _("= Highscores =");
    IGUIStaticText* hscores_header = GUIEngine::getGUIEnv()->addStaticText( text.c_str(), hiscores_title_area,
                                                                           false , true , // border, word warp
                                                                            m_irrlicht_window);
    hscores_header->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

    
    ITexture* texture = irr_driver->getTexture( (file_manager->getGUIDir() + "/track_random.png").c_str() ) ;

    std::string game_mode_ident = RaceManager::getIdentOf( race_manager->getMinorMode() );
    const HighscoreEntry::HighscoreType type = "HST_" + game_mode_ident;
    
    HighscoreEntry* highscores = highscore_manager->getHighscoreEntry(type,
                                                                      race_manager->getNumKarts(),
                                                                      race_manager->getDifficulty(),
                                                                      trackIdent,
                                                                      race_manager->getNumLaps()); 
    const int amount = highscores->getNumberEntries();
    
    std::string kart_name;
    std::string name;
    float time;
    
    char buffer[128];
    
    // fill highscore entries
    for (int n=0; n<HIGHSCORE_COUNT; n++)
    {
        const int from_y = hscores_y_from + (hscores_y_to - hscores_y_from)*(n+1)/(HIGHSCORE_COUNT+1);
        const int next_from_y = hscores_y_from + (hscores_y_to - hscores_y_from)*(n+2)/(HIGHSCORE_COUNT+1);

        const int gap = 3;
        const int icon_size = next_from_y - from_y - gap*2;
        
        core::rect< s32 > icon_area(5, from_y + gap, 5 + icon_size, from_y + icon_size);
        
        m_kart_icons[n] = GUIEngine::getGUIEnv()->addImage( icon_area, m_irrlicht_window );
        m_kart_icons[n]->setImage(texture);
        m_kart_icons[n]->setScaleImage(true);
        m_kart_icons[n]->setTabStop(false);
        
        core::rect< s32 > entry_area(icon_size + 10, from_y, m_area.getWidth()/2, next_from_y);

        // Check if this entry is filled or still empty
        if (n < amount)
        {
            highscores->getEntry(n, kart_name, name, &time);
            sprintf(buffer, "%s : %.2f s\n", name.c_str(), time);
                        
            const KartProperties* prop = kart_properties_manager->getKart(kart_name);
            if (prop != NULL)
            {
                std::string icon_path = file_manager->getDataDir() ;
                icon_path += "/karts/" + prop->getIdent() + "/" + prop->getIconFile();
                ITexture* kart_icon_texture = irr_driver->getTexture( icon_path );
                m_kart_icons[n]->setImage(kart_icon_texture);
            }
        }
        else
        {
            //I18N: for empty highscores entries
            sprintf(buffer, "%s\n", _("(Empty)"));
        }
        
        text = buffer;
        m_highscore_entries[n] = GUIEngine::getGUIEnv()->addStaticText( text.c_str(), entry_area,
                                                                       false , true , // border, word warp
                                                                        m_irrlicht_window);
        
    }
    
    Track* track = track_manager->getTrack(trackIdent);
    
    core::rect< s32 > creator_info_area(0, hscores_y_to, m_area.getWidth()/2, y2);
    
    //I18N: when showing who is the author of track '%s' (place %s where the name of the author should appear)
    sprintf(buffer, _("Track by %s"), track->getDesigner().c_str());
    text = buffer;

    IGUIStaticText* b = GUIEngine::getGUIEnv()->addStaticText( text.c_str(),
                                                                  creator_info_area, false , true , // border, word warp
                                                                  m_irrlicht_window);
    b->setTabStop(false);

    
    // ---- Track screenshot
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
