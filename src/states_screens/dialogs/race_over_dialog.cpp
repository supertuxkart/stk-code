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

#include "challenges/unlock_manager.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/race_over_dialog.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

RaceOverDialog::RaceOverDialog(const float percentWidth, const float percentHeight) : ModalDialog(percentWidth, percentHeight)
{
    // Switch to barrier mode: server waits for ack from each client
    network_manager->beginRaceResultBarrier();

    //const int w = m_area.getWidth();
    //const int h = m_area.getHeight();
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = font->getDimension(L"X").Height;
    
    
    if (unlock_manager->getUnlockedFeatures().size()>0)
    {
        // TODO: unlocked features
    }
    
    core::rect< s32 > area(0, 0, m_area.getWidth(), textHeight);
    IGUIStaticText* caption = GUIEngine::getGUIEnv()->addStaticText( _("Race Results"),
                                                              area, false, false, // border, word warp
                                                              m_irrlicht_window);
    caption->setTabStop(false);
    caption->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        // TODO: offer to continue GP
        // _("Continue Grand Prix")
    }
    else
    {
        // TODO: offer to go back to menu
        // _("Back to the main menu")
    }
    
    // TODO : _("Race in this track again")
    
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_SINGLE)
    {
        // TODO
        // _("Setup New Race")
    }
    
    // _("Race results")
   //  const unsigned int MAX_STR_LEN = 60;
    const unsigned int num_karts = race_manager->getNumKarts();
    
    int*  order = new int [num_karts];
    RaceManager::getWorld()->raceResultOrder( order );
    
    const bool display_time = (RaceManager::getWorld()->getClockMode() == CHRONO);
    
    const int line_h = textHeight + 15;
    const int lines_from_y = textHeight + 15;
    int kart_id = 0; // 'i' below is not reliable because some karts (e.g. leader) will be skipped
    for (unsigned int i = 0; i < num_karts; ++i)
    {
        if (order[i] == -1) continue;
        
        stringw kart_results_line;
        
        const Kart *current_kart = RaceManager::getKart(order[i]);
        const stringw& kart_name = current_kart->getName();
        
        char sTime[20];
        sTime[0]=0;
        
        const float time      = current_kart->getFinishTime();
        
        if (display_time)
        {
            const int min     = (int) floor ( time / 60.0 ) ;
            const int sec     = (int) floor ( time - (double) ( 60 * min ) ) ;
            const int tenths  = (int) floor ( 10.0f * (time - (double)(sec + 60* min)));
            sprintf ( sTime, "%d:%02d:%d", min,  sec,  tenths ) ;
        }
        
        //This shows position + driver name + time + points earned + total points
        if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            const int prev_score = race_manager->getKartPrevScore(order[i]);
            const int new_score = race_manager->getKartScore(order[i]);
            
            kart_results_line = StringUtils::insertValues( L"#%i. %s (%i + %i = %i)",
                                                          current_kart->getPosition(), kart_name.c_str(),
                                                          prev_score, (new_score - prev_score), new_score);
        }
        else
        {
            std::wcout << kart_name.c_str() << std::endl;

            kart_results_line = StringUtils::insertValues( L"%i. %s %s",
                                                          current_kart->getPosition(), kart_name.c_str(), sTime);
        }
        
        const KartProperties* prop = current_kart->getKartProperties();
        std::string icon_path = file_manager->getDataDir() ;
        icon_path += "/karts/" + prop->getIdent() + "/" + prop->getIconFile();
        ITexture* kart_icon_texture = irr_driver->getTexture( icon_path );
        
        const int icon_size = textHeight;
        core::rect< s32 > entry_area(10 + icon_size, lines_from_y+line_h*i, m_area.getWidth(), lines_from_y+line_h*(i+1));
        core::rect< s32 > icon_area(5, lines_from_y + line_h*i, 5+icon_size, lines_from_y+icon_size);

        std::wcout << kart_results_line.c_str() << std::endl;
        
        GUIEngine::getGUIEnv()->addStaticText( kart_results_line.c_str(), entry_area,
                                                                       false , true , // border, word warp
                                                                       m_irrlicht_window);
        IGUIImage* img = GUIEngine::getGUIEnv()->addImage( icon_area, m_irrlicht_window );
        img->setImage(kart_icon_texture);
        img->setScaleImage(true);
        img->setTabStop(false);
        
        kart_id++;
    }

    delete[] order;
    
    /*
    const HighscoreEntry *hs = RaceManager::getWorld()->getHighscores();
    if (hs != NULL)
    {
        w_prev=widget_manager->addTextWgt( WTOK_HIGHSCORES, 5, 7, _("Highscores") );
        widget_manager->hideWgtRect(WTOK_HIGHSCORES);
        w_prev->setPosition(WGT_DIR_FROM_RIGHT, 0.01f, NULL, WGT_DIR_FROM_TOP, 0.01f, NULL);
        
        unsigned int num_scores = hs->getNumberEntries();
        char *highscores = new char[num_scores * MAX_STR_LEN];
        
        for(unsigned int i=0; i<num_scores; i++)
        {
            std::string kart_name, name;
            float T;
            hs->getEntry(i, kart_name, name, &T);
            const int   MINS   = (int) floor ( T / 60.0 ) ;
            const int   SECS   = (int) floor ( T - (float) ( 60 * MINS ) ) ;
            const int   TENTHS = (int) floor ( 10.0f * (T - (float)(SECS + 60*MINS)));
            sprintf((char*)( highscores + MAX_STR_LEN * i ),
                    "%s: %3d:%02d.%01d", name.c_str(), MINS, SECS, TENTHS);
            
            Widget *w=widget_manager->addTextWgt(WTOK_FIRST_HIGHSCORE + i, 5, 7,
                                                 (char*)( highscores+MAX_STR_LEN*i ) );
            w->setPosition(WGT_DIR_FROM_RIGHT, 0.05f, NULL, WGT_DIR_UNDER_WIDGET, 0, w_prev);
            w_prev=w;
        } // next score
        
        widget_manager->sameWidth(WTOK_HIGHSCORES, WTOK_FIRST_HIGHSCORE+num_scores-1);
        
        bottom_of_list = (num_scores > num_karts) ? w_prev : bottom_of_list;
    } // end if hs != NULL
    */
}

void RaceOverDialog::onEnterPressedInternal()
{
}

void RaceOverDialog::processEvent(std::string& eventSource)
{
}
