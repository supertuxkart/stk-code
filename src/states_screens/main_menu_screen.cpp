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

#include "guiengine/scalable_font.hpp"
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

//FIXME : remove, temporary tutorial test
#include "states_screens/tutorial_screen.hpp"

// FIXME : remove, temporary test
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#include "addons/network_http.hpp"

#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( MainMenuScreen );

// ------------------------------------------------------------------------------------------------------
#ifdef ADDONS_MANAGER
MainMenuScreen::MainMenuScreen() : Screen("mainaddons.stkgui")
{
}
#else
MainMenuScreen::MainMenuScreen() : Screen("main.stkgui")
{
}
#endif

// ------------------------------------------------------------------------------------------------------

void MainMenuScreen::loadedFromFile()
{
    m_lang_popup = NULL;
}

// ------------------------------------------------------------------------------------------------------
//
void MainMenuScreen::init()
{
    Screen::init();
    
    // reset in case we're coming back from a race
    StateManager::get()->resetActivePlayers();
    input_manager->getDeviceList()->setAssignMode(NO_ASSIGN);
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

}

#ifdef ADDONS_MANAGER
// ------------------------------------------------------------------------------------------------------
void MainMenuScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
    IconButtonWidget* icon = this->getWidget<IconButtonWidget>("addons");
    if(!addons_manager->onlineReady())
        icon->setDeactivated();
    else
        icon->setActivated();


    LabelWidget* w = this->getWidget<LabelWidget>("info_addons");
    const std::string &news_text = network_http->getNewsMessage();
    w->setText(news_text.c_str());
    
    IconButtonWidget* lang_combo = this->getWidget<IconButtonWidget>("lang_combo");
    if (lang_combo != NULL)
    {
        irr::gui::ScalableFont* font = GUIEngine::getFont();
        
        // I18N: Enter the name of YOUR language here, do not literally translate the word "English"
        font->draw(_("English"),
                   core::rect<s32>(lang_combo->m_x, lang_combo->m_y,
                                   (int)(lang_combo->m_x 
                                         + lang_combo->m_w*0.9f), // multiply to not go over combo arrow
                                   lang_combo->m_y + lang_combo->m_h),
                   video::SColor(255,0,0,0), true /* hcenter */, true /* vcenter */);
        
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
#endif
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
                    // TODO: retrieve a nice name for each language instead of displaying the language code
                    m_lang_popup->addItem((*lang_list)[n], core::stringw((*lang_list)[n].c_str()));
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
                putenv( "LANGUAGE=" );
            }
            else
            {
                char buffer[1024];
                snprintf(buffer, 1024, "LANGUAGE=%s", selection.c_str());
                putenv( buffer );
            }
            
            translations = new Translations();
            GUIEngine::getStateManager()->hardResetAndGoToScreen<MainMenuScreen>();
            
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
    else if (selection == "new")
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
