//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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
#include "challenges/story_mode_timer.hpp"
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
#include "input/keyboard_device.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/overworld.hpp"
#include "modes/demo_world.hpp"
#include "network/network_config.hpp"
#include "online/request_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/cutscene_general.hpp"
#include "states_screens/grand_prix_editor_screen.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/online/online_profile_achievements.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#if DEBUG_MENU_ITEM
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#endif
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <string>


using namespace GUIEngine;
using namespace Online;

// ----------------------------------------------------------------------------

MainMenuScreen::MainMenuScreen() : Screen("main_menu.stkgui")
{
}   // MainMenuScreen

// ----------------------------------------------------------------------------

void MainMenuScreen::loadedFromFile()
{
    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    w->setScrollSpeed(GUIEngine::getFontHeight() / 2);
    
    RibbonWidget* rw_top = getWidget<RibbonWidget>("menu_toprow");
    assert(rw_top != NULL);
    
    if (track_manager->getTrack("overworld") == NULL ||
        track_manager->getTrack("introcutscene") == NULL ||
        track_manager->getTrack("introcutscene2") == NULL)
    {
        rw_top->removeChildNamed("story");
    }

#if DEBUG_MENU_ITEM != 1
    RibbonWidget* rw = getWidget<RibbonWidget>("menu_bottomrow");
    rw->removeChildNamed("test_gpwin");
    rw->removeChildNamed("test_gplose");
    rw->removeChildNamed("test_unlocked");
    rw->removeChildNamed("test_unlocked2");
    rw->removeChildNamed("test_intro");
    rw->removeChildNamed("test_outro");
#endif
}   // loadedFromFile

// ----------------------------------------------------------------------------

void MainMenuScreen::beforeAddingWidget()
{
#ifdef IOS_STK
    // iOS app doesn't like quit button in UI
    Widget* w = getWidget("quit");
    if (w)
        w->setVisible(false);
#endif
}

// ----------------------------------------------------------------------------
//
void MainMenuScreen::init()
{
    Screen::init();

    m_user_id = getWidget<ButtonWidget>("user-id");
    assert(m_user_id);

    // reset in case we're coming back from a race
    NetworkConfig::get()->cleanNetworkPlayers();
    StateManager::get()->resetActivePlayers();
    input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
    input_manager->getDeviceManager()->setSinglePlayer( NULL );
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
    input_manager->getDeviceManager()->clearLatestUsedDevice();

#ifndef SERVER_ONLY
    if (addons_manager->isLoading())
    {
        IconButtonWidget* w = getWidget<IconButtonWidget>("addons");
        w->setActive(false);
        w->resetAllBadges();
        w->setBadge(LOADING_BADGE);
    }

    LabelWidget* w = getWidget<LabelWidget>("info_addons");
    const core::stringw &news_text = NewsManager::get()->getNextNewsMessage();
    w->setText(news_text, true);
    w->update(0.01f);
#endif

    RibbonWidget* r = getWidget<RibbonWidget>("menu_bottomrow");
    // FIXME: why do I need to do this manually
    ((IconButtonWidget*)r->getChildren().get(0))->unfocused(PLAYER_ID_GAME_MASTER, NULL);
    ((IconButtonWidget*)r->getChildren().get(1))->unfocused(PLAYER_ID_GAME_MASTER, NULL);
    ((IconButtonWidget*)r->getChildren().get(2))->unfocused(PLAYER_ID_GAME_MASTER, NULL);

    r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    DemoWorld::resetIdleTime();

#if _IRR_MATERIAL_MAX_TEXTURES_ < 8
    getWidget<IconButtonWidget>("logo")->setImage("gui/icons/logo_broken.png",
        IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
#endif

#ifdef IOS_STK
    // iOS app doesn't like quit button in UI
    Widget* quit = getWidget("quit");
    if (quit)
        quit->setVisible(false);
#endif
}   // init

// ----------------------------------------------------------------------------

void MainMenuScreen::onUpdate(float delta)
{
#ifndef SERVER_ONLY
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if(PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST  ||
       PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
    {
        m_user_id->setText(player->getLastOnlineName() + "@stk");
    }
    else if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_OUT)
    {
        m_user_id->setText(player->getName());
    }
    else
    {
        // now must be either logging in or logging out
        m_user_id->setText(player->getName());
    }

    IconButtonWidget* addons_icon = getWidget<IconButtonWidget>("addons");
    if (addons_icon != NULL)
    {
        if (addons_manager->wasError())
        {
            addons_icon->setActive(true);
            addons_icon->resetAllBadges();
            addons_icon->setBadge(BAD_BADGE);
        }
        else if (addons_manager->isLoading() && UserConfigParams::m_internet_status
            == Online::RequestManager::IPERM_ALLOWED)
        {
            // Addons manager is still initialising/downloading.
            addons_icon->setActive(false);
            addons_icon->resetAllBadges();
            addons_icon->setBadge(LOADING_BADGE);
        }
        else
        {
            addons_icon->setActive(true);
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
#endif
}   // onUpdate

// ----------------------------------------------------------------------------

void MainMenuScreen::eventCallback(Widget* widget, const std::string& name,
                                   const int playerID)
{
#ifndef SERVER_ONLY
    if(name=="user-id")
    {
        UserScreen::getInstance()->push();
        return;
    }

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
        race_manager->setNumPlayers(0);
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
    if (selection == "test_gpwin")
    {
        StoryModeStatus* sms = PlayerManager::getCurrentPlayer()->getStoryModeStatus();
        sms->unlockFeature(const_cast<ChallengeStatus*>(sms->getChallengeStatus("gp1")),
            RaceManager::DIFFICULTY_HARD);

        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumPlayers(0);
        race_manager->startSingleRace("gpwin", 999, false);
        GrandPrixWin* scene = GrandPrixWin::getInstance();
        scene->push();
        const std::pair<std::string, float> winners[] =
            {
                { "kiki", 0.6f },
                { "nolok", 1.0f },
                { "pidgin", 0.0f },
            };
        scene->setKarts(winners);
    }
    else if (selection == "test_gplose")
    {
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumPlayers(0);
        race_manager->startSingleRace("gplose", 999, false);
        GrandPrixLose* scene = GrandPrixLose::getInstance();
        scene->push();
        std::vector<std::pair<std::string, float> > losers;
        losers.emplace_back("nolok", 1.0f);
        losers.emplace_back("kiki", 0.6f);
        //losers.push_back("wilber");
        //losers.push_back("tux");
        scene->setKarts(losers);
    }
    else if (selection == "test_unlocked" || selection == "test_unlocked2")
    {
        StoryModeStatus* sms = PlayerManager::getCurrentPlayer()->getStoryModeStatus();
        sms->unlockFeature(const_cast<ChallengeStatus*>(sms->getChallengeStatus("gp1")),
            RaceManager::DIFFICULTY_HARD);

        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumPlayers(0);
        race_manager->startSingleRace("featunlocked", 999, false);

        FeatureUnlockedCutScene* scene =
            FeatureUnlockedCutScene::getInstance();

        std::vector<std::string> parts;
        parts.push_back("featunlocked");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);

        scene->addTrophy(RaceManager::DIFFICULTY_EASY, false);

        if (selection == "test_unlocked")
        {
            scene->addUnlockedKart(kart_properties_manager->getKart("tux"));
            scene->addUnlockedTrack(track_manager->getTrack("lighthouse"));
            scene->push();
        }
        else if (selection == "test_unlocked2")
        {
            std::vector<video::ITexture*> textures;
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("lighthouse")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("snowtuxpeak")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("sandtrack")
                             ->getScreenshotFile().c_str()));
            textures.push_back(irr_driver->getTexture(
                track_manager->getTrack("snowmountain")
                             ->getScreenshotFile().c_str()));

            scene->addUnlockedPictures(textures, 4.0, 3.0, L"You unlocked <actual text would go here...>");

            scene->push();
        }
    }
    else if (selection == "test_intro")
    {
        CutsceneWorld::setUseDuration(true);
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumPlayers(0);
        race_manager->startSingleRace("introcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("introcutscene");
        parts.push_back("introcutscene2");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        //race_manager->startSingleRace("introcutscene2", 999, false);
        
        CutSceneGeneral* scene = CutSceneGeneral::getInstance();
        scene->push();
        return;
    }
    else if (selection == "test_outro")
    {
        CutsceneWorld::setUseDuration(true);
        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumPlayers(0);
        race_manager->startSingleRace("endcutscene", 999, false);

        std::vector<std::string> parts;
        parts.push_back("endcutscene");
        ((CutsceneWorld*)World::getWorld())->setParts(parts);
        
        CutSceneGeneral* scene = CutSceneGeneral::getInstance();
        scene->push();
    }
    else
#endif
    if (selection == "new")
    {
        NetworkConfig::get()->unsetNetworking();
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(false);
        s->push();
    }
    else if (selection == "multiplayer")
    {
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        NetworkConfig::get()->unsetNetworking();
        s->setMultiplayer(true);
        s->setFromOverworld(false);
        s->push();
    }
    else if (selection == "options")
    {
        OptionsScreenGeneral::getInstance()->push();
    }
    else if (selection == "quit")
    {
#ifdef ANDROID
        GUIEngine::EventHandler::get()->setAcceptEvents(false);
        ANativeActivity_finish(global_android_app->activity);
#else
        StateManager::get()->popMenu();
#endif
        return;
    }
    else if (selection == "about")
    {
        CreditsScreen::getInstance()->push();
    }
    else if (selection == "help")
    {
        HelpScreen1::getInstance()->push();
    }
    else if (selection == "startTutorial")
    {
        race_manager->setNumPlayers(1);
        race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
        race_manager->setMinorMode (RaceManager::MINOR_MODE_TUTORIAL);
        race_manager->setNumKarts( 1 );
        race_manager->setTrack( "tutorial" );
        race_manager->setDifficulty(RaceManager::DIFFICULTY_EASY);
        race_manager->setReverseTrack(false);

        // Use keyboard 0 by default (FIXME: let player choose?)
        InputDevice* device = input_manager->getDeviceManager()->getKeyboard(0);

        // Create player and associate player with keyboard
        StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(),
                                                device);

        if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
        {
            Log::warn("MainMenuScreen", "Cannot find kart '%s', will revert to default",
                      UserConfigParams::m_default_kart.c_str());
            UserConfigParams::m_default_kart.revertToDefaults();
        }
        race_manager->setPlayerKart(0, UserConfigParams::m_default_kart);

        // ASSIGN should make sure that only input from assigned devices
        // is read.
        input_manager->getDeviceManager()->setAssignMode(ASSIGN);
        input_manager->getDeviceManager()
            ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

        StateManager::get()->enterGameState();
        race_manager->setupPlayerKartInfo();
        race_manager->startNew(false);
    }
    else if (selection == "story")
    {
        NetworkConfig::get()->unsetNetworking();
        PlayerProfile *player = PlayerManager::getCurrentPlayer();

        // Start the story mode (and speedrun) timer
        story_mode_timer->startTimer();

        if (player->isFirstTime())
        {
            CutsceneWorld::setUseDuration(true);
            StateManager::get()->enterGameState();
            race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
            race_manager->setNumKarts( 0 );
            race_manager->setNumPlayers(0);
            race_manager->startSingleRace("introcutscene", 999, false);

            std::vector<std::string> parts;
            parts.push_back("introcutscene");
            parts.push_back("introcutscene2");
            ((CutsceneWorld*)World::getWorld())->setParts(parts);
            //race_manager->startSingleRace("introcutscene2", 999, false);
            
            CutSceneGeneral* scene = CutSceneGeneral::getInstance();
            scene->push();
            return;
        }
        else
        {
            // Unpause the story mode timer when entering back the story mode
            story_mode_timer->unpauseTimer(/* exit loading pause */ false);

            const std::string default_kart = UserConfigParams::m_default_kart;
            if (player->isLocked(default_kart))
            {
                KartSelectionScreen *next = OfflineKartSelectionScreen::getInstance();
                next->setGoToOverworldNext();
                next->setMultiplayer(false);
                next->push();
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
                                "If you want to play online, go in the options menu, "
                                "and check \"Connect to the Internet\"."));
            return;
        }
        OnlineScreen::getInstance()->push();
    }
    else if (selection == "addons")
    {
        // Don't go to addons if there is no internet, unless some addons are
        // already installed (so that you can delete addons without being online).
        if(UserConfigParams::m_internet_status!=RequestManager::IPERM_ALLOWED)
        {
            if (!addons_manager->anyAddonsInstalled())
            {
                new MessageDialog(_("You can not download addons without internet access. "
                                    "If you want to download addons, go in the options menu, "
                                    "and check \"Connect to the Internet\"."));
                return;
            }
            else
            {
                AddonsScreen::getInstance()->push();
                new MessageDialog(_("You can not download addons without internet access. "
                                    "If you want to download addons, go in the options menu, "
                                    "and check \"Connect to the Internet\".\n\n"
                                    "You can however delete already downloaded addons."));
                return;
            }
        }
        AddonsScreen::getInstance()->push();
    }
    else if (selection == "gpEditor")
    {
        GrandPrixEditorScreen::getInstance()->push();
    }
    else if (selection == "achievements")
    {
        OnlineProfileAchievements::getInstance()->push();
    }
#endif
}   // eventCallback

// ----------------------------------------------------------------------------

void MainMenuScreen::tearDown()
{
}   // tearDown

// ----------------------------------------------------------------------------

void MainMenuScreen::onDisabledItemClicked(const std::string& item)
{
#ifndef SERVER_ONLY
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
#endif
}   // onDisabledItemClicked

// ----------------------------------------------------------------------------

bool MainMenuScreen::onEscapePressed()
{
#ifdef ANDROID
    GUIEngine::EventHandler::get()->setAcceptEvents(false);
    ANativeActivity_finish(global_android_app->activity);
    return false;
#endif

    return true;
}   // onEscapePressed
