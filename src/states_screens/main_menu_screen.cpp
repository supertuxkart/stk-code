//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "addons/news_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
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
#include "modes/cutscene_world.hpp"
#include "modes/overworld.hpp"
#include "modes/demo_world.hpp"
#include "online/request_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/grand_prix_editor_screen.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/login_screen.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#if DEBUG_MENU_ITEM
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#endif
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"



#include <string>


using namespace GUIEngine;
using namespace Online;

DEFINE_SCREEN_SINGLETON( MainMenuScreen );

bool MainMenuScreen::m_enable_online = false;

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

    if (addons_manager->isLoading())
    {
        IconButtonWidget* w = getWidget<IconButtonWidget>("addons");
        w->setDeactivated();
        w->resetAllBadges();
        w->setBadge(LOADING_BADGE);
    }

    m_online = getWidget<IconButtonWidget>("online");

    if(!m_enable_online)
        m_online->setDeactivated();

    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    const core::stringw &news_text = NewsManager::get()->getNextNewsMessage();
    w->setText(news_text, true);
    w->update(0.01f);

    RibbonWidget* r = getWidget<RibbonWidget>("menu_bottomrow");
    // FIXME: why do I need to do this manually
    ((IconButtonWidget*)r->getChildren().get(0))->unfocused(PLAYER_ID_GAME_MASTER, NULL);
    ((IconButtonWidget*)r->getChildren().get(1))->unfocused(PLAYER_ID_GAME_MASTER, NULL);
    ((IconButtonWidget*)r->getChildren().get(2))->unfocused(PLAYER_ID_GAME_MASTER, NULL);

    r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    DemoWorld::resetIdleTime();

#if _IRR_MATERIAL_MAX_TEXTURES_ < 8
    getWidget<IconButtonWidget>("logo")->setImage("gui/logo_broken.png",
        IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
#endif

}   // init

// ----------------------------------------------------------------------------
void MainMenuScreen::onUpdate(float delta)

{
    if(PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST  ||
       PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
    {
        m_online->setActivated();
        m_online->setLabel( _("Online"));
    }
    else if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_OUT)
    {
        m_online->setActivated();
        m_online->setLabel( _("Login" ));
    }
    else // now must be either logging in or logging out
        m_online->setDeactivated();

    m_online->setLabel(PlayerManager::getCurrentOnlineId() ? _("Online")
                                                           : _("Login" )  );
    IconButtonWidget* addons_icon = getWidget<IconButtonWidget>("addons");
    if (addons_icon != NULL)
    {
        if (addons_manager->wasError())
        {
            addons_icon->setActivated();
            addons_icon->resetAllBadges();
            addons_icon->setBadge(BAD_BADGE);
        }
        else if (addons_manager->isLoading() && UserConfigParams::m_internet_status
            == Online::RequestManager::IPERM_ALLOWED)
        {
            // Addons manager is still initialising/downloading.
            addons_icon->setDeactivated();
            addons_icon->resetAllBadges();
            addons_icon->setBadge(LOADING_BADGE);
        }
        else
        {
            addons_icon->setActivated();
            addons_icon->resetAllBadges();
        }
        // maybe add a new badge when not allowed to access the net
    }

    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    w->update(delta);
    if(w->scrolledOff())
    {
        const core::stringw &news_text = NewsManager::get()->getNextNewsMessage();
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

    /*
    if (selection == "story")
    {
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts( 0 );
        race_manager->setNumPlayers(0);
        race_manager->setNumLocalPlayers(0);
        race_manager->startSingleRace("endcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("introcutscene");
        parts.push_back("introcutscene2");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        //race_manager->startSingleRace("introcutscene2", 999, false);
        return;
    }
    */

#if DEBUG_MENU_ITEM
    if (selection == "options")
    {
        // The DEBUG item

        // GP WIN
        /*
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumLocalPlayers(0);
        race_manager->startSingleRace("gpwin", 999, false);
        GrandPrixWin* scene = GrandPrixWin::getInstance();
        StateManager::get()->pushScreen(scene);
        const std::string winners[] = { "elephpant", "nolok", "pidgin" };
        scene->setKarts(winners);
        */

        // GP Lose
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumLocalPlayers(0);
        race_manager->startSingleRace("gplose", 999, false);
        GrandPrixLose* scene = GrandPrixLose::getInstance();
        StateManager::get()->pushScreen(scene);
        std::vector<std::string> losers;
        losers.push_back("nolok");
        losers.push_back("elephpant");
        //losers.push_back("wilber");
        //losers.push_back("tux");
        scene->setKarts(losers);


        /*
        // FEATURE UNLOCKED
        FeatureUnlockedCutScene* scene =
        FeatureUnlockedCutScene::getInstance();

        scene->addTrophy(RaceManager::DIFFICULTY_EASY);
        StateManager::get()->pushScreen(scene);

        static int i = 1;
        i++;

        if (i % 2 == 0)
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
        else if (i % 2 == 1)
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

            StateManager::get()->pushScreen(scene);
        }
         */
    }
    else
#endif
    if (selection == "new")
    {
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance(); //FIXME : that was for tests
        s->setMultiplayer(false);
        s->setFromOverworld(false);
        StateManager::get()->pushScreen( s );
    }
    else if (selection == "multiplayer")
    {
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        s->setMultiplayer(true);
        s->setFromOverworld(false);
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
    else if (selection == "startTutorial")
    {
        race_manager->setNumLocalPlayers(1);
        race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
        race_manager->setMinorMode (RaceManager::MINOR_MODE_TUTORIAL);
        race_manager->setNumKarts( 1 );
        race_manager->setTrack( "tutorial" );
        race_manager->setDifficulty(RaceManager::DIFFICULTY_EASY);
        race_manager->setReverseTrack(false);

        // Use keyboard 0 by default (FIXME: let player choose?)
        InputDevice* device = input_manager->getDeviceList()->getKeyboard(0);

        // Create player and associate player with keyboard
        StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(),
                                                device, NULL);

        if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
        {
            fprintf(stderr, "[MainMenuScreen] WARNING: cannot find kart '%s', will revert to default\n",
                    UserConfigParams::m_default_kart.c_str());
            UserConfigParams::m_default_kart.revertToDefaults();
        }
        race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

        // ASSIGN should make sure that only input from assigned devices
        // is read.
        input_manager->getDeviceList()->setAssignMode(ASSIGN);
        input_manager->getDeviceList()
            ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

        StateManager::get()->enterGameState();
        race_manager->setupPlayerKartInfo();
        race_manager->startNew(false);
    }
    else if (selection == "story")
    {
        PlayerProfile *player = PlayerManager::getCurrentPlayer();
        if (player->isFirstTime())
        {
            StateManager::get()->enterGameState();
            race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
            race_manager->setNumKarts( 0 );
            race_manager->setNumPlayers(0);
            race_manager->setNumLocalPlayers(0);
            race_manager->startSingleRace("introcutscene", 999, false);

            std::vector<std::string> parts;
            parts.push_back("introcutscene");
            parts.push_back("introcutscene2");
            ((CutsceneWorld*)World::getWorld())->setParts(parts);
            //race_manager->startSingleRace("introcutscene2", 999, false);
            return;
        }
        else
        {
            const std::string default_kart = UserConfigParams::m_default_kart;
            if (player->isLocked(default_kart))
            {
                KartSelectionScreen *next = OfflineKartSelectionScreen::getInstance();
                next->setGoToOverworldNext();
                next->setMultiplayer(false);
                StateManager::get()->resetAndGoToScreen(next);
                return;
            }
            OverWorld::enterOverWorld();
        }
    }
    else if (selection == "online")
    {
        if(UserConfigParams::m_internet_status!=RequestManager::IPERM_ALLOWED)
        {
            new MessageDialog(_("You can not play online without internet access. "
                                "If you want to play online, go to options, select "
                                " tab 'User Interface', and edit "
                                "\"Allow STK to connect to the Internet\"."));
            return;
        }
        if (PlayerManager::getCurrentOnlineId())
            StateManager::get()->pushScreen(OnlineScreen::getInstance());
        else
            StateManager::get()->pushScreen(LoginScreen::getInstance());
    }
    else if (selection == "addons")
    {
        // Don't go to addons if there is no internet, unless some addons are
        // already installed (so that you can delete addons without being online).
        if(UserConfigParams::m_internet_status!=RequestManager::IPERM_ALLOWED &&
            !addons_manager->anyAddonsInstalled())
        {
            new MessageDialog(_("You can not download addons without internet access. "
                                "If you want to download addons, go to options, select "
                                " tab 'User Interface', and edit "
                                "\"Allow STK to connect to the Internet\"."));
            return;
        }
        StateManager::get()->pushScreen(AddonsScreen::getInstance());
    }
    else if (selection == "gpEditor")
    {
        StateManager::get()->pushScreen(GrandPrixEditorScreen::getInstance());
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
        if (UserConfigParams::m_internet_status != RequestManager::IPERM_ALLOWED)
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
