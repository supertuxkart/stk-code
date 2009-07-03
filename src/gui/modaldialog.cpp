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

#include "config/player.hpp"
#include "gui/engine.hpp"
#include "gui/modaldialog.hpp"
#include "gui/options_screen.hpp"
#include "gui/state_manager.hpp"
#include "gui/widget.hpp"
#include "input/input_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/translation.hpp"

using namespace irr;

namespace GUIEngine
{
// global instance of the current dialog if any
static ModalDialog* modalWindow = NULL;

ModalDialog::ModalDialog(const float percentWidth, const float percentHeight)
{
    const core::dimension2d<s32>& frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
    const int w = (int)(frame_size.Width*percentWidth);
    const int h = (int)(frame_size.Height*percentHeight);
    m_area = core::rect< s32 >( position2d< s32 >(frame_size.Width/2 - w/2, frame_size.Height/2 - h/2),
                               dimension2d< s32 >(w, h) );
    
    if(modalWindow != NULL) delete modalWindow;
    modalWindow = this;
    
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow  	( m_area, true /* modal */ );
}


ModalDialog::~ModalDialog()
{
    // irrLicht is to stupid to remove focus from deleted widgets
    // so do it by hand
    GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
    
    m_irrlicht_window->remove();
    
    if(modalWindow == this) modalWindow = NULL;
}

void ModalDialog::clearWindow()
{
    const int children_amount = m_children.size();
    for(int i=0; i<children_amount; i++)
    {
        m_irrlicht_window->removeChild( m_children[i].m_element );
    }
    m_children.clearAndDeleteAll();   
    
    m_irrlicht_window->remove();
    m_irrlicht_window = GUIEngine::getGUIEnv()->addWindow  	( m_area, true /* modal */ );
    
    /*
    const core::list<IGUIElement*>& remainingChildren = m_irrlicht_window->getChildren();
    const int amount = remainingChildren.getSize();
    for(core::list<IGUIElement*>::Iterator it=remainingChildren.begin(); it != remainingChildren.end(); it++)
    {
        it->remove();
    }
     */
}
    
void ModalDialog::dismiss()
{
    if(modalWindow != NULL) delete modalWindow;
    modalWindow = NULL;
}

void ModalDialog::onEnterPressed()
{
    std::cout << "onEnterPressed()\n";
    if(modalWindow != NULL) modalWindow->onEnterPressedInternal();
}

bool ModalDialog::isADialogActive()
{
    return modalWindow != NULL;
}
ModalDialog* ModalDialog::getCurrent()
{
    return modalWindow;
}

void ModalDialog::onEnterPressedInternal()
{
}

#if 0
#pragma mark -
#pragma mark PressAKeyDialog
#endif
    
// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

PressAKeyDialog::PressAKeyDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    //core::rect< s32 > area2(0, 0, m_area.getWidth(), m_area.getHeight());
    //IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText( stringw(_("Press a key")).c_str(),
    //                                      area2, false /* border */, true /* word wrap */,
    //                                      m_irrlicht_window);
    //label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    
    LabelWidget* widget = new LabelWidget();
    widget->m_type = WTYPE_LABEL;
    widget->m_properties[PROP_TEXT] = _("Press a key");
    widget->m_properties[PROP_TEXT_ALIGN] = "center";
    widget->x = 0;
    widget->y = 0;
    widget->w = m_area.getWidth();
    widget->h = m_area.getHeight()/2;
    widget->setParent(m_irrlicht_window);
    
    m_children.push_back(widget);
    widget->add();
    
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = font->getDimension(L"X").Height;
        
    ButtonWidget* widget2 = new ButtonWidget();
    widget2->m_type = WTYPE_BUTTON; // FIXME : shouldn't constructor set type?
    widget2->m_properties[PROP_ID] = "cancel";
    widget2->m_properties[PROP_TEXT] = _("Press ESC to cancel");
    widget2->x = 15;
    widget2->y = m_area.getHeight() - textHeight - 12;
    widget2->w = m_area.getWidth() - 30;
    widget2->h = textHeight + 6;
    widget2->setParent(m_irrlicht_window);
    
    m_children.push_back(widget2);
    widget2->add();
}
void PressAKeyDialog::processEvent(std::string& eventSource)
{
    if(eventSource == "cancel")
    {
        input_manager->setMode(InputManager::MENU);
        dismiss();
    }
}
    
#if 0
#pragma mark -
#pragma mark EnterPlayerNameDialog
#endif
    
// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------


EnterPlayerNameDialog::EnterPlayerNameDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    //core::rect< s32 > area_top(0, 0, m_area.getWidth(), m_area.getHeight()/2);
    //IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText( stringw(_("Enter the new player's name")).c_str(),
    //                                                              area_top, false /* border */, true /* word wrap */,
    //                                                              m_irrlicht_window);
   // label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
  
    LabelWidget* widget = new LabelWidget();
    widget->m_type = WTYPE_LABEL;
    widget->m_properties[PROP_TEXT] = _("Enter the new player's name");
    widget->m_properties[PROP_TEXT_ALIGN] = "center";
    widget->x = 0;
    widget->y = 0;
    widget->w = m_area.getWidth();
    widget->h = m_area.getHeight()/3;
    widget->setParent(m_irrlicht_window);
    
    m_children.push_back(widget);
    widget->add();
    
    // ----
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = font->getDimension(L"X").Height;
    
    const int textAreaYFrom = m_area.getHeight()/2 - textHeight/2;
    
    textCtrl = new TextBoxWidget();
    textCtrl->m_type = WTYPE_BUTTON;
    textCtrl->m_properties[PROP_TEXT] = "";
    textCtrl->x = 50;
    textCtrl->y = textAreaYFrom - 10;
    textCtrl->w = m_area.getWidth()-100;
    textCtrl->h = textHeight + 5;
    textCtrl->setParent(m_irrlicht_window);
    m_children.push_back(textCtrl);
    textCtrl->add();
    GUIEngine::getGUIEnv()->setFocus( textCtrl->m_element );
    
    // TODO : add Ok button

    cancelButton = new ButtonWidget();
    cancelButton->m_type = WTYPE_BUTTON; // FIXME : shouldn't constructor set type?
    cancelButton->m_properties[PROP_ID] = "cancel";
    cancelButton->m_properties[PROP_TEXT] = _("Cancel");
    cancelButton->x = 15;
    cancelButton->y = m_area.getHeight() - textHeight - 12;
    cancelButton->w = m_area.getWidth() - 30;
    cancelButton->h = textHeight + 6;
    cancelButton->setParent(m_irrlicht_window);
    
    m_children.push_back(cancelButton);
    cancelButton->add();

}
EnterPlayerNameDialog::~EnterPlayerNameDialog()
{
    textCtrl->m_element->remove();
}
void EnterPlayerNameDialog::processEvent(std::string& eventSource)
{
    if(eventSource == "cancel")
    {
        dismiss();
        return;
    }
}
void EnterPlayerNameDialog::onEnterPressedInternal()
{
    // ---- Cancel button pressed
    if( GUIEngine::getGUIEnv()->hasFocus(cancelButton->m_element) )
    {
        std::string fakeEvent = "cancel";
        processEvent(fakeEvent);
        return;
    }
        
    // ---- Otherwise, accept entered name
    stringw playerName = textCtrl->getText();
    if(playerName.size() > 0)
        StateManager::gotNewPlayerName( playerName );
    
    // irrLicht is too stupid to remove focus from deleted widgets
    // so do it by hand
    GUIEngine::getGUIEnv()->removeFocus( textCtrl->m_element );
    GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );

    ModalDialog::dismiss();
}

#if 0
#pragma mark -
#pragma mark TrackInfoDialog
#endif
    
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

    
    SpinnerWidget* widget = new SpinnerWidget();
    widget->m_type = WTYPE_SPINNER;
    widget->x = 0;
    widget->y = y2;
    widget->w = m_area.getWidth();
    widget->h = m_area.getHeight() - y2;
    widget->setParent(m_irrlicht_window);
    
    widget->m_properties[PROP_MIN_VALUE] = "1";
    widget->m_properties[PROP_MAX_VALUE] = "99";
    
    m_children.push_back(widget);
    widget->add();
    widget->setValue(3);
    
    //IGUIStaticText* d = GUIEngine::getGUIEnv()->addStaticText( stringw(_("Number of laps")).c_str(),
    //                                                          area_bottom, false /* border */, true /* word wrap */,
    //                                                          m_irrlicht_window);

    
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    b->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    //d->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
}

// ------------------------------------------------------------------------------------------------------

// FIXME : this probably doesn't belong here
void startGame()
{
    ModalDialog::dismiss();
    
    IVideoDriver* driver = GUIEngine::getDriver();
    
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
    
void TrackInfoDialog::onEnterPressedInternal()
{
    startGame();
}


// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------


#if 0
#pragma mark -
#pragma mark PlayerInfoDialog
#endif
    
PlayerInfoDialog::PlayerInfoDialog(Player* player, const float w, const float h) : ModalDialog(w, h)
{
    m_player = player;
    
    showRegularDialog();
}
void PlayerInfoDialog::showRegularDialog()
{
    clearWindow();
    
    const int y1 = m_area.getHeight()/6;
    const int y2 = m_area.getHeight()*2/6;
    const int y3 = m_area.getHeight()*3/6;
    const int y4 = m_area.getHeight()*5/6;
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = font->getDimension(L"X").Height;
    const int buttonHeight = textHeight + 10;
    
    {
        textCtrl = new TextBoxWidget();
        textCtrl->m_type = WTYPE_BUTTON;
        textCtrl->m_properties[PROP_ID] = "renameplayer";
        textCtrl->m_properties[PROP_TEXT] = m_player->getName();
        textCtrl->x = 50;
        textCtrl->y = y1 - textHeight/2;
        textCtrl->w = m_area.getWidth()-100;
        textCtrl->h = textHeight + 5;
        textCtrl->setParent(m_irrlicht_window);
        m_children.push_back(textCtrl);
        textCtrl->add();
        GUIEngine::getGUIEnv()->setFocus( textCtrl->m_element );
    }
    
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_type = WTYPE_BUTTON;
        widget->m_properties[PROP_ID] = "renameplayer";
        widget->m_properties[PROP_TEXT] = _("Rename");
        
        const int textWidth = font->getDimension( stringw(widget->m_properties[PROP_TEXT].c_str()).c_str() ).Width + 40;
        
        widget->x = m_area.getWidth()/2 - textWidth/2;
        widget->y = y2;
        widget->w = textWidth;
        widget->h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
    }
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_type = WTYPE_BUTTON;
        widget->m_properties[PROP_ID] = "cancel";
        widget->m_properties[PROP_TEXT] = _("Cancel");
        
        const int textWidth = font->getDimension( stringw(widget->m_properties[PROP_TEXT].c_str()).c_str() ).Width + 40;
        
        widget->x = m_area.getWidth()/2 - textWidth/2;
        widget->y = y3;
        widget->w = textWidth;
        widget->h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
    }
    
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_type = WTYPE_BUTTON;
        widget->m_properties[PROP_ID] = "removeplayer";
        widget->m_properties[PROP_TEXT] = _("Remove");
        
        const int textWidth = font->getDimension( stringw(widget->m_properties[PROP_TEXT].c_str()).c_str() ).Width + 40;
        
        widget->x = m_area.getWidth()/2 - textWidth/2;
        widget->y = y4;
        widget->w = textWidth;
        widget->h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
    }
    
}
    
void PlayerInfoDialog::showConfirmDialog()
{
    clearWindow();
        
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = font->getDimension(L"X").Height;
    const int buttonHeight = textHeight + 10;
    
    
    char message[256];
    sprintf(message, _("Do you really want to delete player '%s' ?"), m_player->getName());
    
    core::rect< s32 > area_left(5, 0, m_area.getWidth()-5, m_area.getHeight()/2);
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( stringw(message).c_str(),
                                                              area_left, false /* border */, true /* word wrap */,
                                                              m_irrlicht_window);
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_type = WTYPE_BUTTON;
        widget->m_properties[PROP_ID] = "confirmremove";
        widget->m_properties[PROP_TEXT] = _("Confirm Remove");
        
        const int textWidth = font->getDimension( stringw(widget->m_properties[PROP_TEXT].c_str()).c_str() ).Width + 40;
        
        widget->x = m_area.getWidth()/2 - textWidth/2;
        widget->y = m_area.getHeight()/2;
        widget->w = textWidth;
        widget->h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
    }
    
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_type = WTYPE_BUTTON;
        widget->m_properties[PROP_ID] = "cancelremove";
        widget->m_properties[PROP_TEXT] = _("Cancel Remove");
        
        const int textWidth = font->getDimension( stringw(widget->m_properties[PROP_TEXT].c_str()).c_str() ).Width + 40;
        
        widget->x = m_area.getWidth()/2 - textWidth/2;
        widget->y = m_area.getHeight()*3/4;
        widget->w = textWidth;
        widget->h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
        GUIEngine::getGUIEnv()->setFocus( widget->m_element );

    }
    
}
    
void PlayerInfoDialog::onEnterPressedInternal()
{
}
void PlayerInfoDialog::processEvent(std::string& eventSource)
{
    if(eventSource == "renameplayer")
    {
        // accept entered name
        stringw playerName = textCtrl->getText();
        if(playerName.size() > 0)
        {
            StateManager::gotNewPlayerName( playerName, m_player );
        }
        
        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->m_element );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        ModalDialog::dismiss();
        
        dismiss();
        return;
    }
    else if(eventSource == "removeplayer")
    {
        showConfirmDialog();
    }
    else if(eventSource == "confirmremove")
    {
        StateManager::deletePlayer( m_player );

        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->m_element );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        ModalDialog::dismiss();
        
        return;
    }
    else if(eventSource == "cancelremove")
    {
        showRegularDialog();
    }
    else if(eventSource == "cancel")
    {   
        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->m_element );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        ModalDialog::dismiss();
        
        return;
    }
    
}
    
}
