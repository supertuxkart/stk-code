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

#include "gui/engine.hpp"
#include "gui/modaldialog.hpp"
#include "gui/options_screen.hpp"
#include "gui/state_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/translation.hpp"

using namespace irr;

// global instance of the current dialog if any
static ModalDialog* modalWindow = NULL;

ModalDialog::~ModalDialog()
{
    m_irrlicht_window->remove();
    
    if(modalWindow == this) modalWindow = NULL;
}

void ModalDialog::dismiss()
{
    if(modalWindow != NULL) delete modalWindow;
    modalWindow = NULL;
}

void ModalDialog::onEnterPressed()
{
    if(modalWindow != NULL) modalWindow->onEnterPressedInternal();
}

bool ModalDialog::isADialogActive()
{
    return modalWindow != NULL;
}

void ModalDialog::onEnterPressedInternal()
{
}

ModalDialog::ModalDialog(const float percentWidth, const float percentHeight)
{
    const core::dimension2d<s32>& frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
    const int w = (int)(frame_size.Width*percentWidth);
    const int h = (int)(frame_size.Height*percentHeight);
    m_area = core::rect< s32 >( position2d< s32 >(frame_size.Width/2 - w/2, frame_size.Height/2 - h/2),
                           dimension2d< s32 >(w, h) );
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow  	( m_area, true /* modal */ );

    if(modalWindow != NULL) delete modalWindow;
    modalWindow = this;
}

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

PressAKeyDialog::PressAKeyDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    core::rect< s32 > area2(0, 0, m_area.getWidth(), m_area.getHeight());
    IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText( stringw(_("Press a key")).c_str(),
                                          area2, false /* border */, true /* word wrap */,
                                          m_irrlicht_window);
    label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
}

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

EnterPlayerNameDialog::EnterPlayerNameDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), m_area.getHeight()/2);
    IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText( stringw(_("Enter the new player's name")).c_str(),
                                                                  area_top, false /* border */, true /* word wrap */,
                                                                  m_irrlicht_window);
    label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
  
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = font->getDimension(L"X").Height;
    
    const int bottomYFrom = m_area.getHeight()/2;
    const int bottomYTo = m_area.getHeight();
    const int bottomHeight = bottomYTo - bottomYFrom;
    const int textAreaYFrom = bottomYFrom + bottomHeight/2 - textHeight/2;
    
    core::rect< s32 > area_bottom(50, textAreaYFrom - 10, m_area.getWidth()-50, textAreaYFrom + textHeight + 10);
    textCtrl = GUIEngine::getGUIEnv()->addEditBox (L"", area_bottom, true /* border */, m_irrlicht_window);
    GUIEngine::getGUIEnv()->setFocus(textCtrl);
}

// ------------------------------------------------------------------------------------------------------

void EnterPlayerNameDialog::onEnterPressedInternal()
{
    stringw playerName = textCtrl->getText();
    StateManager::gotNewPlayerName( playerName );
    ModalDialog::dismiss();
}

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------


TrackInfoDialog::TrackInfoDialog(const char* trackName, ITexture* screenshot, const float w, const float h) : ModalDialog(w, h)
{
    const int y1 = m_area.getHeight()/3;
    const int y2 = m_area.getHeight() - 50;
    
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( stringw(trackName).c_str(),
                                                                  area_top, false /* border */, true /* word wrap */,
                                                                  m_irrlicht_window);
    
    
    core::rect< s32 > area_left(0, y1, m_area.getWidth()/2, y2);
    IGUIStaticText* b = GUIEngine::getGUIEnv()->addStaticText( stringw(_("High Scores & Track Info")).c_str(),
                                                                  area_left, false /* border */, true /* word wrap */,
                                                                  m_irrlicht_window);
 
    // TODO : preserve aspect ratio
    core::rect< s32 > area_right(m_area.getWidth()/2, y1, m_area.getWidth(), y2);
    IGUIImage* screenshotWidget = GUIEngine::getGUIEnv()->addImage( area_right, m_irrlicht_window );
    screenshotWidget->setImage(screenshot);
    screenshotWidget->setScaleImage(true);

    core::rect< s32 > area_bottom(0, y2, m_area.getWidth(), m_area.getHeight());
    IGUIStaticText* d = GUIEngine::getGUIEnv()->addStaticText( stringw(_("Number of laps")).c_str(),
                                                              area_bottom, false /* border */, true /* word wrap */,
                                                              m_irrlicht_window);

    
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    b->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    d->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
}

// ------------------------------------------------------------------------------------------------------

void TrackInfoDialog::onEnterPressedInternal()
{
    IVideoDriver* driver = GUIEngine::getDriver();
    IGUIFont* font = GUIEngine::getFont();
    
    // TODO : draw a loading screen
    driver->endScene();
    driver->beginScene(true, false);
    driver->endScene();

    
    StateManager::enterGameState();
    //race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setTrack("beach");
    race_manager->setNumLaps( 3 );
    race_manager->setCoinTarget( 0 ); // Might still be set from a previous challenge
    //race_manager->setNumKarts( 1 );
    race_manager->setNumPlayers( 1 );
    race_manager->setNumLocalPlayers( 1 );
    network_manager->setupPlayerKartInfo();
    //race_manager->getKartType(1) = KT_PLAYER;

    race_manager->startNew();
}

