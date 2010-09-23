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

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/network_manager.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include "irrlicht.h"

using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;
using namespace GUIEngine;

// ------------------------------------------------------------------------------------------------------

TrackInfoDialog::TrackInfoDialog(const std::string& trackIdent, const irr::core::stringw& trackName,
                                 ITexture* screenshot, const float w, const float h) : ModalDialog(w, h)
{
    const bool has_laps       = race_manager->modeHasLaps();
    const bool has_highscores = race_manager->modeHasHighscores();
    
    const int y1 = m_area.getHeight()/7;
    const int y2 = m_area.getHeight()*5/7;
    const int y3 = m_area.getHeight()*6/7;
    
    m_track_ident = trackIdent;

    // ---- Track title
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( trackName.c_str(),
                                                               area_top, false, true, // border, word warp
                                                               m_irrlicht_window);
    a->setTabStop(false);

    const int hscores_y_from = y1;
    const int hscores_y_to = y1 + (y2 - y1)*2/3;
    
    // ---- Track credits
    Track* track = track_manager->getTrack(trackIdent);
    
    core::rect< s32 > creator_info_area(0, hscores_y_to, m_area.getWidth()/2, y2);
    
    //I18N: when showing who is the author of track '%s' (place %s where the name of the author should appear)
    stringw text = StringUtils::insertValues(_("Track by %s"), track->getDesigner().c_str());

    IGUIStaticText* b = GUIEngine::getGUIEnv()->addStaticText( text.c_str(),
                                               creator_info_area, false , true , // border, word warp
                                               m_irrlicht_window);
    b->setTabStop(false);

    
    
    // ---- Track screenshot
    IconButtonWidget* screenshotWidget = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                                                              false /* tab stop */, false /* focusable */);
    // images are saved squared, but must be stretched to 4:
    screenshotWidget->setCustomAspectRatio(4.0f / 3.0f);
    core::rect< s32 > area_right(m_area.getWidth()/2, y1, m_area.getWidth(), y2-10);
    
    screenshotWidget->m_x = area_right.UpperLeftCorner.X;
    screenshotWidget->m_y = area_right.UpperLeftCorner.Y;
    screenshotWidget->m_w = area_right.getWidth();
    screenshotWidget->m_h = area_right.getHeight();
    
    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    screenshotWidget->m_properties[PROP_ICON] = "gui/main_help.png"; 
    screenshotWidget->setParent(m_irrlicht_window);
    screenshotWidget->add();
    
    if (screenshot != NULL)
    {
        screenshotWidget->setImage(screenshot);
    }
    m_widgets.push_back(screenshotWidget);
    
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    b->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    
    // ---- Lap count m_spinner
    if (has_laps)
    {
        m_spinner = new SpinnerWidget();
        m_spinner->m_x = m_area.getWidth()/2 - 200;
        m_spinner->m_y = y2;
        m_spinner->m_w = 400;
        m_spinner->m_h = y3 - y2 - 15;
        m_spinner->setParent(m_irrlicht_window);
        
        m_spinner->m_properties[PROP_ID] = "lapcountspinner";
        m_spinner->m_properties[PROP_MIN_VALUE] = "1";
        m_spinner->m_properties[PROP_MAX_VALUE] = "99";
        
        //I18N: In the track setup screen (number of laps choice, where %i is the number)
        m_spinner->setText( _("%i laps") );
        
        m_widgets.push_back(m_spinner);
        m_spinner->add();
        m_spinner->setValue(3);
        m_spinner->getIrrlichtElement()->setTabStop(true);
        m_spinner->getIrrlichtElement()->setTabGroup(false);
    }
    else
    {
        m_spinner = NULL;
    }
    
    // ---- Start button
    ButtonWidget* okBtn = new ButtonWidget();
    okBtn->m_properties[PROP_ID] = "start";
    okBtn->setText( _("Start Race") );
    okBtn->m_x = m_area.getWidth()/2 - 200;
    okBtn->m_y = y3;
    okBtn->m_w = 400;
    okBtn->m_h = m_area.getHeight() - y3 - 15;
    okBtn->setParent(m_irrlicht_window);
    m_widgets.push_back(okBtn);
    okBtn->add();
    okBtn->getIrrlichtElement()->setTabStop(true);
    okBtn->getIrrlichtElement()->setTabGroup(false);
    
    // ---- High Scores
    if (has_highscores)
    {
        addHighScoreWidgets(hscores_y_from, hscores_y_to);
        const int num_laps = m_spinner->getValue();
        race_manager->setNumLaps(num_laps);
        updateHighScores();
    }
    
    
    okBtn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
    
}

// ------------------------------------------------------------------------------------------------------

TrackInfoDialog::~TrackInfoDialog()
{
    // Place focus back on selected track, in case the dialog was cancelled and we're back to
    // the track selection screen after
    Screen* curr_screen = GUIEngine::getCurrentScreen();
    if (curr_screen->getName() == "tracks.stkgui")
    {
        ((TracksScreen*)curr_screen)->setFocusOnTrack(m_track_ident);
    }
    
}

// ------------------------------------------------------------------------------------------------------

void TrackInfoDialog::addHighScoreWidgets(const int hscores_y_from, const int hscores_y_to)
{
    ITexture* texture = irr_driver->getTexture( (file_manager->getGUIDir() + "/random_kart.png").c_str() ) ;

    core::rect< s32 > hiscores_title_area(5, hscores_y_from, m_area.getWidth()/2, hscores_y_from + 30);
    stringw text = _("= Highscores =");
    IGUIStaticText* hscores_header = GUIEngine::getGUIEnv()->addStaticText( text.c_str(), hiscores_title_area,
                                                                           false , true , // border, word warp
                                                                           m_irrlicht_window);
    hscores_header->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    
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
        m_kart_icons[n]->setUseAlphaChannel(true);

        core::rect< s32 > entry_area(icon_size + 10, from_y, m_area.getWidth()/2, next_from_y);
        m_highscore_entries[n] = GUIEngine::getGUIEnv()->addStaticText( L"", entry_area,
                                                               false , true , // border, word warp
                                                               m_irrlicht_window);
    }
}

// ------------------------------------------------------------------------------------------------------

void TrackInfoDialog::updateHighScores()
{
    std::string game_mode_ident = RaceManager::getIdentOf( race_manager->getMinorMode() );
    const Highscores::HighscoreType type = "HST_" + game_mode_ident;
    
    Highscores* highscores = 
        highscore_manager->getHighscores(type,
                                         race_manager->getNumberOfKarts(),
                                         race_manager->getDifficulty(),
                                         m_track_ident,
                                         race_manager->getNumLaps()        );
    const int amount = highscores->getNumberEntries();
    
    std::string kart_name;
    std::string name;
    float time;
    
    // fill highscore entries
    for (int n=0; n<HIGHSCORE_COUNT; n++)
    {        
        irr::core::stringw line;
        
        // Check if this entry is filled or still empty
        if (n < amount)
        {
            char buffer[256];
            
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
            line = buffer;
        }
        else
        {
            //I18N: for empty highscores entries
            line = _("(Empty)");
            line += "\n";
            
            ITexture* no_kart_texture = irr_driver->getTexture(
                    (file_manager->getGUIDir() + "/random_kart.png").c_str() ) ;
            m_kart_icons[n]->setImage(no_kart_texture);

        }
        
        m_highscore_entries[n]->setText( line.c_str() );
        
    }    
}

// ------------------------------------------------------------------------------------------------------

// FIXME : this probably doesn't belong here
/**
  * Start a race, using the settings set through previous menus, plus which track
  * plus number of laps if relevant
  *
  * \param trackIdent  Internal name of the track to race on
  * \param num_laps    Number of laps to do, or -1 if not relevant
  */
void startGame(const std::string trackIdent, const int num_laps)
{
    //FIXME: simplify and centralize race start sequence!!

    ModalDialog::dismiss();
    
    StateManager::get()->enterGameState();
    //race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setTrack(trackIdent.c_str());
    
    if (num_laps != -1) race_manager->setNumLaps( num_laps );
    
    race_manager->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

    race_manager->setCoinTarget( 0 ); // Might still be set from a previous challenge
    network_manager->setupPlayerKartInfo();
    
    race_manager->startNew();
}

// ------------------------------------------------------------------------------------------------------

void TrackInfoDialog::onEnterPressedInternal()
{
    const int num_laps = (m_spinner == NULL ? -1 : m_spinner->getValue());
    startGame(m_track_ident, num_laps);
}

// ------------------------------------------------------------------------------------------------------   

GUIEngine::EventPropagation TrackInfoDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "start" )
    {
        const int num_laps = (m_spinner == NULL ? -1 : m_spinner->getValue());
        startGame(m_track_ident, num_laps);
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "lapcountspinner")
    {
        assert(m_spinner != NULL);
        const int num_laps = m_spinner->getValue();
        race_manager->setNumLaps(num_laps);
        updateHighScores();
    }
    
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------
