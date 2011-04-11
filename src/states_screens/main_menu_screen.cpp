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

#include "states_screens/main_menu_screen.hpp"

#include <iostream>
#include <string>

#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/challenges.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tutorial_screen.hpp"

//#include "states_screens/feature_unlocked.hpp"
//#include "states_screens/grand_prix_lose.hpp"
//#include "states_screens/grand_prix_win.hpp"

#include "addons/network_http.hpp"

#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( MainMenuScreen );

// ------------------------------------------------------------------------------------------------------

MainMenuScreen::MainMenuScreen() : Screen("main.stkgui")
{
}

// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::loadedFromFile()
{
    m_lang_popup = NULL;
    
#ifndef ADDONS_MANAGER
    RibbonWidget* r = this->getWidget<RibbonWidget>("menu_toprow");
    if (r != NULL) r->deleteChild("addons");
#endif
    
    LabelWidget* w = this->getWidget<LabelWidget>("info_addons");
    w->setScrollSpeed(15);
}

// ------------------------------------------------------------------------------------------------------
//
void MainMenuScreen::init()
{
    Screen::init();
    
    // reset in case we're coming back from a race
    StateManager::get()->resetActivePlayers();
    input_manager->getDeviceList()->setAssignMode(NO_ASSIGN);
    input_manager->getDeviceList()->setSinglePlayer( NULL );
    input_manager->setMasterPlayerOnly(false);

    // Avoid incorrect behaviour in certain race circumstances:
    // If a multi-player game is played with two keyboards, the 2nd
    // player selects his kart last, and only the keyboard is used
    // to select all other settings - then if the next time the kart
    // selection screen comes up, the default device will still be
    // the 2nd player. So if the first player presses 'select', it
    // will instead add a second player (so basically the key 
    // binding for the second player become the default, so pressing
    // select will add a new player). See bug 3090931
    // To avoid this, we will clean the last used device, making
    // the key bindings for the first player the default again.
    input_manager->getDeviceList()->clearLatestUsedDevice();

#ifdef ADDONS_MANAGER
    if(!addons_manager->onlineReady())
    {
        IconButtonWidget* w = this->getWidget<IconButtonWidget>("addons");
        w->setDeactivated();
    }
#endif
    
    LabelWidget* w = this->getWidget<LabelWidget>("info_addons");
    const core::stringw &news_text = network_http->getNextNewsMessage();
    w->setText(news_text, true);
    w->update(0.01f);
}

// ------------------------------------------------------------------------------------------------------
void MainMenuScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
#ifdef ADDONS_MANAGER
    IconButtonWidget* addons_icon = this->getWidget<IconButtonWidget>("addons");
    if (addons_icon != NULL)
    {
        if(!addons_manager->onlineReady())
            addons_icon->setDeactivated();
        else
            addons_icon->setActivated();
    }

#endif    

    LabelWidget* w = this->getWidget<LabelWidget>("info_addons");
    w->update(delta);
    if(w->scrolledOff())
    {
        const core::stringw &news_text = network_http->getNextNewsMessage();
        w->setText(news_text, true);
    }
    
    IconButtonWidget* lang_combo = this->getWidget<IconButtonWidget>("lang_combo");
    if (lang_combo != NULL)
    {
        irr::gui::ScalableFont* font = GUIEngine::getFont();
        
        // I18N: Enter the name of YOUR language here, do not literally translate the word "English"
        core::stringw language_name = _("English");
        
        const int LEFT_MARGIN = 5;
        const int arrow_width = (int)(lang_combo->m_h*0.6f); // the arrow is about half wide as the combo is high

        // Below is a not-too-pretty hack. When language name is too long to fit the space allocated by the STK
        // widget, resize the irrlicht element (but don't resize the STK widget on top of it, because we don't
        // want the change of size to be permanent - if we switch back to another language the combo needs
        // to shrink back)
        int element_width = lang_combo->getIrrlichtElement()->getRelativePosition().getWidth();
        const int text_w = (int)font->getDimension(language_name.c_str()).Width;
        const int needed_additional_space = text_w - (element_width - arrow_width - LEFT_MARGIN);
        if (needed_additional_space > 0)
        {
            // language name too long to fit
            gui::IGUIElement* el = lang_combo->getIrrlichtElement();
            core::recti pos = el->getRelativePosition();
            pos.UpperLeftCorner.X -= needed_additional_space;
            el->setRelativePosition( pos );
        }
        
        element_width = lang_combo->getIrrlichtElement()->getRelativePosition().getWidth();
        
        const int elem_x = lang_combo->getIrrlichtElement()->getRelativePosition().UpperLeftCorner.X;
        font->draw(language_name,
                   core::rect<s32>(elem_x + LEFT_MARGIN,
                                   lang_combo->m_y,
                                   // don't go over combo arrow
                                   (int)(elem_x + LEFT_MARGIN + (element_width - arrow_width)),
                                   lang_combo->m_y + lang_combo->m_h),
                   // center horizontally only if there is enough room, irrlicht's centering algorithm
                   // seems to give weird results when space is too tight
                   video::SColor(255,0,0,0), (element_width - text_w > 5) /* hcenter */, true /* vcenter */);
        
        // Close popup when focus lost
        if (m_lang_popup != NULL)
        {
            Widget* focus = GUIEngine::getFocusForPlayer(PLAYER_ID_GAME_MASTER);
            const core::position2d<s32> mouse = irr_driver->getDevice()->getCursorControl()->getPosition();
            if (mouse.X < m_lang_popup->m_x - 50 || mouse.X > m_lang_popup->m_x + m_lang_popup->m_w + 50 ||
                mouse.Y < m_lang_popup->m_y - 50  || // we don't check if mouse Y is too large because the mouse will come from the bottom
                (focus != NULL && focus->getType() == WTYPE_RIBBON))
            {
                closeLangPopup();
            }
        }
    }
}
// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    // most interesting stuff is in the ribbons, so start there
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL)
    {
        // Language selection combo
        if (name == "lang_combo")
        {
            if (m_lang_popup == NULL)
            {
                // When the combo is clicked, show a pop-up list with the choices
                IconButtonWidget* lang_combo = this->getWidget<IconButtonWidget>("lang_combo");
                
                m_lang_popup = new ListWidget();
                m_lang_popup->m_properties[PROP_ID] = "lang_popup";
                const core::dimension2d<u32> frame_size = irr_driver->getFrameSize();
                
                const int width = (int)(frame_size.Width*0.4f);
                
                const int MARGIN_ABOVE_POPUP = 50;
                const int CLEAR_BOTTOM = 15;

                const int height = lang_combo->m_y - MARGIN_ABOVE_POPUP - CLEAR_BOTTOM;
                
                m_lang_popup->m_x = lang_combo->m_x;
                m_lang_popup->m_y = lang_combo->m_y - height - CLEAR_BOTTOM;
                m_lang_popup->m_w = width;
                m_lang_popup->m_h = height;
                
                m_lang_popup->add();

                m_lang_popup->m_properties[PROP_ID] = "language_popup";
                
                // I18N: in the language choice, to select the same language as the OS
                m_lang_popup->addItem("system", _("System Language"));

                const std::vector<std::string>* lang_list = translations->getLanguageList();
                const int amount = lang_list->size();
                for (int n=0; n<amount; n++)
                {
                    std::string code_name = (*lang_list)[n];
                    std::string nice_name = tinygettext::Language::from_name(code_name.c_str()).get_name();
                    m_lang_popup->addItem(code_name, core::stringw(code_name.c_str()) + " (" +
                                                     nice_name.c_str() + ")");
                }

                manualAddWidget(m_lang_popup);
                
                m_lang_popup->setSelectionID( m_lang_popup->getItemID(UserConfigParams::m_language) );
                m_lang_popup->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            }
            else
            {
                closeLangPopup();
            }
        }
        else if (name == "language_popup")
        {
            std::string selection = m_lang_popup->getSelectionInternalName();

            closeLangPopup();
            
            delete translations;
            
            if (selection == "system")
            {
#ifdef WIN32
                _putenv("LANGUAGE=");
#else
                setenv( "LANGUAGE", "", 1);
#endif
            }
            else
            {
#ifdef WIN32
                std::string s=std::string("LANGUAGE=")+selection.c_str();
                _putenv(s.c_str());
#else
                setenv("LANGUAGE", selection.c_str(), 1);
#endif
            }
            
            translations = new Translations();
            GUIEngine::getStateManager()->hardResetAndGoToScreen<MainMenuScreen>();
            
            GUIEngine::getFont()->updateRTL();
            GUIEngine::getTitleFont()->updateRTL();
            GUIEngine::getSmallFont()->updateRTL();
            
            UserConfigParams::m_language = selection.c_str();
            user_config->saveConfig();
        }
        
        return;
    }
    
    // When the lang popup is shown, ignore all other widgets
    // FIXME: for some reasons, irrlicht widgets appear to be click-through, this is why
    //        this hack is needed
    if (m_lang_popup != NULL)
    {
        return;
    }
    
    // ---- A ribbon icon was clicked
    
    std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    
#if 0
    if (selection == "network")
    {
        // The DEBUG item
        FeatureUnlockedCutScene* scene = FeatureUnlockedCutScene::getInstance();
        
        static int i = 1;
        i++;
        
        if (i % 4 == 0)
        {
            // the passed kart will not be modified, that's why I allow myself to use const_cast
            scene->addUnlockedKart( const_cast<KartProperties*>(kart_properties_manager->getKart("tux")),
                                   L"Unlocked");
            StateManager::get()->pushScreen(scene);
        }
        else if (i % 4 == 1)
        {
            std::vector<video::ITexture*> textures;
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("lighthouse")->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("crescentcrossing")->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("sandtrack")->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(track_manager->getTrack("snowmountain")->getScreenshotFile().c_str()));

            scene->addUnlockedPictures(textures, 1.0, 0.75, L"You did it");
            
            /*
            scene->addUnlockedPicture( irr_driver->getTexture(track_manager->getTrack("lighthouse")->getScreenshotFile().c_str()),
                                      1.0, 0.75, L"You did it");
            */
            
            StateManager::get()->pushScreen(scene);
        }
        else if (i % 4 == 2)
        {
            GrandPrixWin* scene = GrandPrixWin::getInstance();
            const std::string winners[] = { "elephpant", "nolok", "pidgin" };
            StateManager::get()->pushScreen(scene);
            scene->setKarts( winners );
        }
        else
        {
            GrandPrixLose* scene = GrandPrixLose::getInstance();
            StateManager::get()->pushScreen(scene);
            std::vector<std::string> losers;
            losers.push_back("nolok");
            losers.push_back("elephpant");
            losers.push_back("wilber");
            scene->setKarts( losers );
        }
    }
    else
#endif
    if (selection == "new")
    {
        KartSelectionScreen* s = KartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        StateManager::get()->pushScreen( s );
    }
    else if (selection == "multiplayer")
    {
        KartSelectionScreen* s = KartSelectionScreen::getInstance();
        s->setMultiplayer(true);
        StateManager::get()->pushScreen( s );
    }
    else if (selection == "options")
    {
        StateManager::get()->pushScreen( OptionsScreenVideo::getInstance() );
    }
    else if (selection == "quit")
    {
        main_loop->abort();
        return;
    }
    else if (selection == "about")
    {
        StateManager::get()->pushScreen(CreditsScreen::getInstance());
    }
    else if (selection == "help")
    {
        StateManager::get()->pushScreen(HelpScreen1::getInstance());
    }
    else if (selection == "challenges")
    {
        StateManager::get()->pushScreen(ChallengesScreen::getInstance());
    }
    else if (selection == "tutorial")
    {
        StateManager::get()->pushScreen(TutorialScreen::getInstance());
    }
#ifdef ADDONS_MANAGER
    else if (selection == "addons")
    {
        std::cout << "Addons" << std::endl;
        StateManager::get()->pushScreen(AddonsScreen::getInstance());
    }
#endif
}

// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::tearDown()
{
    if (m_lang_popup != NULL)
    {
        closeLangPopup();
    }
}

// ------------------------------------------------------------------------------------------------------

bool MainMenuScreen::onEscapePressed()
{
    if (m_lang_popup != NULL)
    {
        closeLangPopup();
        return false;
    }
    
    return true;
}

// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::closeLangPopup()
{
    assert(m_lang_popup != NULL);
    
    m_lang_popup->getIrrlichtElement()->remove();
    m_lang_popup->elementRemoved();
    manualRemoveWidget(m_lang_popup);
    delete m_lang_popup;
    m_lang_popup = NULL;
}
