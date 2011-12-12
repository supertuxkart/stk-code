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

#define DEBUG_MENU_ITEM 0

#include "states_screens/main_menu_screen.hpp"

#include <string>

#include "addons/network_http.hpp"
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

#if DEBUG_MENU_ITEM
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#endif

#include "states_screens/dialogs/message_dialog.hpp"

#include "addons/news_manager.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( MainMenuScreen );

// ----------------------------------------------------------------------------

MainMenuScreen::MainMenuScreen() : Screen("main.stkgui")
{
}   // MainMenuScreen

// ----------------------------------------------------------------------------

void MainMenuScreen::loadedFromFile()
{  
    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    w->setScrollSpeed(15);
}   // loadedFromFile

// ----------------------------------------------------------------------------
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

    if (UserConfigParams::m_internet_status!=NetworkHttp::IPERM_ALLOWED)
    {
        IconButtonWidget* w = getWidget<IconButtonWidget>("addons");
        w->setDeactivated();
        w->resetAllBadges();
        w->setBadge(BAD_BADGE);
    }
    else if (!addons_manager->onlineReady())
    {
        IconButtonWidget* w = getWidget<IconButtonWidget>("addons");
        w->setDeactivated();
        w->resetAllBadges();
        w->setBadge(LOADING_BADGE);
    }

    
    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    const core::stringw &news_text = news_manager->getNextNewsMessage();
    w->setText(news_text, true);
    w->update(0.01f);
    
    RibbonWidget* r = getWidget<RibbonWidget>("menu_bottomrow");
    // FIXME: why do I need to do this manually
    ((IconButtonWidget*)r->getChildren().get(0))->unfocused(PLAYER_ID_GAME_MASTER, NULL);

    r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

// ----------------------------------------------------------------------------
void MainMenuScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
    IconButtonWidget* addons_icon = getWidget<IconButtonWidget>("addons");
    if (addons_icon != NULL)
    {
        if(UserConfigParams::m_internet_status!=NetworkHttp::IPERM_ALLOWED )
        {
            addons_icon->setDeactivated();
            addons_icon->resetAllBadges();
            addons_icon->setBadge(BAD_BADGE);
        }
        else if (addons_manager->wasError())
        {
            addons_icon->setDeactivated();
            addons_icon->resetAllBadges();
            addons_icon->setBadge(BAD_BADGE);
        }
        else if (addons_manager->onlineReady())
        {
            addons_icon->setActivated();
            addons_icon->resetAllBadges();
        }
        else 
        {
            // Addons manager is still initialising/downloading.
            addons_icon->setDeactivated();
            addons_icon->resetAllBadges();
            addons_icon->setBadge(LOADING_BADGE);
        }
    }

    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    w->update(delta);
    if(w->scrolledOff())
    {
        const core::stringw &news_text = news_manager->getNextNewsMessage();
        w->setText(news_text, true);
    }
}   // onUpdate

// ----------------------------------------------------------------------------

void MainMenuScreen::eventCallback(Widget* widget, const std::string& name, 
                                   const int playerID)
{
    // most interesting stuff is in the ribbons, so start there
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    
    if (ribbon == NULL) return; // what's that event??
    
    // ---- A ribbon icon was clicked
    std::string selection = 
        ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    
#if DEBUG_MENU_ITEM
    if (selection == "options")
    {
        // The DEBUG item
        FeatureUnlockedCutScene* scene = 
            FeatureUnlockedCutScene::getInstance();
        
        static int i = 1;
        i++;
        
        if (i % 4 == 0)
        {
            // the passed kart will not be modified, that's why I allow myself
            // to use const_cast
            scene->addUnlockedKart( 
                                   const_cast<KartProperties*>(
                                        kart_properties_manager->getKart("tux")
                                                              ),
                                   L"Unlocked"
                                   );
            StateManager::get()->pushScreen(scene);
        }
        else if (i % 4 == 1)
        {
            std::vector<video::ITexture*> textures;
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("lighthouse")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("crescentcrossing")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("sandtrack")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("snowmountain")
                             ->getScreenshotFile().c_str()));

            scene->addUnlockedPictures(textures, 1.0, 0.75, L"You did it");
            
            /*
            scene->addUnlockedPicture( 
                irr_driver->getTexture(
                    track_manager->getTrack("lighthouse")
                    ->getScreenshotFile().c_str()),
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
        StateManager::get()->popMenu();
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
    else if (selection == "story")
    {
        StateManager::get()->pushScreen(ChallengesScreen::getInstance());
    }
    else if (selection == "tutorial")
    {
        StateManager::get()->pushScreen(TutorialScreen::getInstance());
    }
    else if (selection == "addons")
    {
        StateManager::get()->pushScreen(AddonsScreen::getInstance());
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void MainMenuScreen::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------

void MainMenuScreen::onDisabledItemClicked(const std::string& item)
{
    if (item == "addons")
    {
        if (UserConfigParams::m_internet_status != NetworkHttp::IPERM_ALLOWED)
        {
            new MessageDialog( _("The add-ons module is currently disabled in "
                                 "the Options screen") );
        }
        else if (addons_manager->wasError())
        {
            new MessageDialog( _("Sorry, an error occurred while contacting "
                                 "the add-ons website. Make sure you are "
                                 "connected to the Internet and that "
                                 "SuperTuxKart is not blocked by a firewall"));
        }
        else if (addons_manager->isLoading())
        {
            new MessageDialog( _("Please wait while the add-ons are loading"));
        }
    }
}   // onDisabledItemClicked
