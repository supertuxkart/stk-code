//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "states_screens/dialogs/race_paused_dialog.hpp"

#include <string>

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/story_mode_timer.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/emoji_keyboard.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/layout_manager.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/CGUIEditBox.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "io/file_manager.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart.hpp"
#include "modes/overworld.hpp"
#include "modes/world.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/stk_host.hpp"
#include "race/race_manager.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/race_gui_base.hpp"
#include "states_screens/race_gui_multitouch.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IrrlichtDevice.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

// ----------------------------------------------------------------------------

RacePausedDialog::RacePausedDialog(const float percentWidth,
                                   const float percentHeight) :
    ModalDialog(percentWidth, percentHeight)
{
    m_target_team = KART_TEAM_NONE;
    m_self_destroy = false;
    m_from_overworld = false;

    if (dynamic_cast<OverWorld*>(World::getWorld()) != NULL)
    {
        loadFromFile("overworld_dialog.stkgui");
        m_from_overworld = true;
    }
    else if (!NetworkConfig::get()->isNetworking())
    {
        loadFromFile("race_paused_dialog.stkgui");
    }
    else
    {
        loadFromFile("online/network_ingame_dialog.stkgui");
    }

    GUIEngine::RibbonWidget* back_btn = getWidget<RibbonWidget>("backbtnribbon");
    back_btn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );

    if (NetworkConfig::get()->isNetworking())
    {
        music_manager->pauseMusic();
        SFXManager::get()->pauseAll();
        m_text_box->clearListeners();
        m_text_box->setTextBoxType(TBT_CAP_SENTENCES);
        // Unicode enter arrow
        getWidget("send")->setText(L"\u21B2");
        // Unicode smile emoji
        getWidget("emoji")->setText(L"\u263A");
        if (UserConfigParams::m_lobby_chat && UserConfigParams::m_race_chat)
        {
            m_text_box->setActive(true);
            getWidget("send")->setVisible(true);
            getWidget("emoji")->setVisible(true);
            m_text_box->addListener(this);
            auto cl = LobbyProtocol::get<ClientLobby>();
            if (cl && !cl->serverEnabledChat())
            {
                m_text_box->setActive(false);
                getWidget("send")->setActive(false);
                getWidget("emoji")->setActive(false);
                if (m_target_team != KART_TEAM_NONE)
                    getWidget("team")->setActive(false);
            }
        }
        else
        {
            m_text_box->setActive(false);
            m_text_box->setText(
                _("Chat is disabled, enable in options menu."));
            getWidget("send")->setVisible(false);
            getWidget("emoji")->setVisible(false);
            if (m_target_team != KART_TEAM_NONE)
                getWidget("team")->setVisible(false);
        }
    }
    else
    {
        World::getWorld()->schedulePause(WorldStatus::IN_GAME_MENU_PHASE);
    }
    if (dynamic_cast<OverWorld*>(World::getWorld()) == NULL)
    {
        if (RaceManager::get()->isBattleMode() || RaceManager::get()->isCTFMode())
        {
            getWidget<IconButtonWidget>("backbtn")->setLabel(_("Back to Battle"));
            if (!NetworkConfig::get()->isNetworking())
            {
                getWidget<IconButtonWidget>("newrace")->setLabel(_("Setup New Game"));
                if (getWidget<IconButtonWidget>("restart"))
                    getWidget<IconButtonWidget>("restart")->setLabel(_("Restart Battle"));
            }
            getWidget<IconButtonWidget>("exit")->setLabel(_("Exit Battle"));
        }
        else
        {
            getWidget<IconButtonWidget>("backbtn")->setLabel(_("Back to Race"));
            if (!NetworkConfig::get()->isNetworking())
            {
                getWidget<IconButtonWidget>("newrace")->setLabel(_("Setup New Race"));
                if (getWidget<IconButtonWidget>("restart"))
                    getWidget<IconButtonWidget>("restart")->setLabel(_("Restart Race"));
            }
            getWidget<IconButtonWidget>("exit")->setLabel(_("Exit Race"));
        }
    }
    
#ifndef MOBILE_STK
    if (m_text_box && UserConfigParams::m_lobby_chat)
        m_text_box->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
#endif
}   // RacePausedDialog

// ----------------------------------------------------------------------------
RacePausedDialog::~RacePausedDialog()
{
    if (NetworkConfig::get()->isNetworking())
    {
        music_manager->resumeMusic();
        SFXManager::get()->resumeAll();
    }
    else
    {
        World::getWorld()->scheduleUnpause();
    }
    
    if (m_touch_controls != UserConfigParams::m_multitouch_controls)
    {
        UserConfigParams::m_multitouch_controls = m_touch_controls;
        
        if (World::getWorld() && World::getWorld()->getRaceGUI())
        {
            World::getWorld()->getRaceGUI()->recreateGUI();
        }

        user_config->saveConfig();
    }
}   // ~RacePausedDialog

// ----------------------------------------------------------------------------

void RacePausedDialog::loadedFromFile()
{
    // disable the "restart" button in GPs
    if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        GUIEngine::RibbonWidget* choice_ribbon =
            getWidget<GUIEngine::RibbonWidget>("choiceribbon");
#ifdef DEBUG
        const bool success = choice_ribbon->deleteChild("restart");
        assert(success);
#else
        choice_ribbon->deleteChild("restart");
#endif
    }
    // Remove "endrace" button for types not (yet?) implemented
    // Also don't show it unless the race has started. Prevents finishing in
    // a time of 0:00:00.
    if ((RaceManager::get()->getMinorMode() != RaceManager::MINOR_MODE_NORMAL_RACE  &&
         RaceManager::get()->getMinorMode() != RaceManager::MINOR_MODE_TIME_TRIAL ) ||
         World::getWorld()->isStartPhase() ||
         NetworkConfig::get()->isNetworking())
    {
        GUIEngine::RibbonWidget* choice_ribbon =
            getWidget<GUIEngine::RibbonWidget>("choiceribbon");
        choice_ribbon->deleteChild("endrace");
        // No restart in network game
        if (NetworkConfig::get()->isNetworking())
        {
            choice_ribbon->deleteChild("restart");
        }
    }
}

// ----------------------------------------------------------------------------

void RacePausedDialog::onEnterPressedInternal()
{
}   // onEnterPressedInternal

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation
           RacePausedDialog::processEvent(const std::string& eventSource)
{
    GUIEngine::RibbonWidget* choice_ribbon =
            getWidget<GUIEngine::RibbonWidget>("choiceribbon");
    GUIEngine::RibbonWidget* backbtn_ribbon =
            getWidget<GUIEngine::RibbonWidget>("backbtnribbon");

    if (eventSource == "send" && m_text_box)
    {
        m_target_team = KART_TEAM_NONE;
        handleChat(m_text_box->getText());
        return GUIEngine::EVENT_BLOCK;
    }
    if (eventSource == "team" && m_text_box)
    {
        handleChat(m_text_box->getText());
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "emoji" && m_text_box &&
        !ScreenKeyboard::isActive())
    {
        EmojiKeyboard* ek = new EmojiKeyboard(1.0f, 0.40f,
            m_text_box->getIrrlichtElement<CGUIEditBox>());
        ek->init();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "backbtnribbon")
    {
        const std::string& selection =
            backbtn_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
            
        if (selection == "backbtn")
        {
            // unpausing is done in the destructor so nothing more to do here
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "touch_device")
        {
            IrrlichtDevice* irrlicht_device = irr_driver->getDevice();
            assert(irrlicht_device != NULL);
            bool accelerometer_available = irrlicht_device->isAccelerometerAvailable();
            bool gyroscope_available = irrlicht_device->isGyroscopeAvailable() && accelerometer_available;
    
            if (m_touch_controls == MULTITOUCH_CONTROLS_STEERING_WHEEL)
            {
                m_touch_controls = MULTITOUCH_CONTROLS_ACCELEROMETER;
            }
            else if (m_touch_controls == MULTITOUCH_CONTROLS_ACCELEROMETER)
            {
                m_touch_controls = MULTITOUCH_CONTROLS_GYROSCOPE;
            }
            else if (m_touch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
            {
                m_touch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
            }
            
            if (m_touch_controls == MULTITOUCH_CONTROLS_ACCELEROMETER && 
                !accelerometer_available)
            {
                m_touch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
            }
            else if (m_touch_controls == MULTITOUCH_CONTROLS_GYROSCOPE && 
                !gyroscope_available)
            {
                m_touch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
            }
            
            updateTouchDeviceIcon();
            
            return GUIEngine::EVENT_BLOCK;
        }
    }
    else if (eventSource == "choiceribbon")
    {
        const std::string& selection =
            choice_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "exit")
        {
            bool from_overworld = m_from_overworld;
            ModalDialog::dismiss();
            if (STKHost::existHost())
            {
                STKHost::get()->shutdown();
            }
            RaceManager::get()->exitRace();
            RaceManager::get()->setAIKartOverride("");

            if (NetworkConfig::get()->isNetworking())
            {
                StateManager::get()->resetAndSetStack(
                    NetworkConfig::get()->getResetScreens().data());
                NetworkConfig::get()->unsetNetworking();
            }
            else
            {
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());

                // Pause story mode timer when quitting story mode
                if (from_overworld)
                    story_mode_timer->pauseTimer(/*loading screen*/ false);

                if (RaceManager::get()->raceWasStartedFromOverworld())
                {
                    OverWorld::enterOverWorld();
                }
            }
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "help")
        {
            dismiss();
            HelpScreen1::getInstance()->push();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "options")
        {
            dismiss();
            OptionsScreenGeneral::getInstance()->push();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "restart")
        {
            ModalDialog::dismiss();
            World::getWorld()->scheduleUnpause();
            RaceManager::get()->rerunRace();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "newrace")
        {
            ModalDialog::dismiss();
            if (NetworkConfig::get()->isNetworking())
            {
                // back lobby
                NetworkString back(PROTOCOL_LOBBY_ROOM);
                back.setSynchronous(true);
                back.addUInt8(LobbyProtocol::LE_CLIENT_BACK_LOBBY);
                STKHost::get()->sendToServer(&back, true);
            }
            else
            {
                World::getWorld()->scheduleUnpause();
                RaceManager::get()->exitRace();
                Screen* new_stack[] =
                    {
                        MainMenuScreen::getInstance(),
                        RaceSetupScreen::getInstance(),
                        NULL
                    };
                StateManager::get()->resetAndSetStack(new_stack);
            }
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "endrace")
        {
            ModalDialog::dismiss();
            if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
                RaceManager::get()->addSkippedTrackInGP();
            World::getWorld()->getRaceGUI()->removeReferee();
            World::getWorld()->endRaceEarly();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "selectkart")
        {
            dynamic_cast<OverWorld*>(World::getWorld())->scheduleSelectKart();
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------
void RacePausedDialog::beforeAddingWidgets()
{
    GUIEngine::RibbonWidget* choice_ribbon =
        getWidget<GUIEngine::RibbonWidget>("choiceribbon");

    bool showSetupNewRace = RaceManager::get()->raceWasStartedFromOverworld();
    int index = choice_ribbon->findItemNamed("newrace");
    if (index != -1)
        choice_ribbon->setItemVisible(index, !showSetupNewRace);

    // Disable in game menu to avoid timer desync if not racing in network
    // game
    if (NetworkConfig::get()->isNetworking() &&
        !(World::getWorld()->getPhase() == WorldStatus::MUSIC_PHASE ||
        World::getWorld()->getPhase() == WorldStatus::RACE_PHASE))
    {
        index = choice_ribbon->findItemNamed("help");
        if (index != -1)
            choice_ribbon->setItemVisible(index, false);
        index = choice_ribbon->findItemNamed("options");
        if (index != -1)
            choice_ribbon->setItemVisible(index, false);
        index = choice_ribbon->findItemNamed("newrace");
        if (index != -1)
            choice_ribbon->setItemVisible(index, false);
    }
    if (NetworkConfig::get()->isNetworking())
    {
        int id = 0;
        for (auto& kart : World::getWorld()->getKarts())
        {
            if (!World::getWorld()->hasTeam())
                break;
            // Handle only 1st local player even splitscreen
            if (kart->getController()->isLocalPlayerController())
            {
                m_target_team =
                    RaceManager::get()->getKartInfo(id).getKartTeam();
                break;
            }
            id++;
        }
        if (m_target_team == KART_TEAM_RED)
        {
            getWidget("team")->setVisible(true);
            getWidget("team")->m_properties[GUIEngine::PROP_WIDTH] = "7%";
            getWidget("team_space")->m_properties[GUIEngine::PROP_WIDTH] = "1%";
            getWidget("team")->setText(StringUtils::utf32ToWide({0x1f7e5}));
        }
        else if (m_target_team == KART_TEAM_BLUE)
        {
            getWidget("team")->setVisible(true);
            getWidget("team")->m_properties[GUIEngine::PROP_WIDTH] = "7%";
            getWidget("team_space")->m_properties[GUIEngine::PROP_WIDTH] = "1%";
            getWidget("team")->setText(StringUtils::utf32ToWide({0x1f7e6}));
        }
        else
        {
            getWidget("team")->m_properties[GUIEngine::PROP_WIDTH] = "0%";
            getWidget("team_space")->m_properties[GUIEngine::PROP_WIDTH] = "0%";
            getWidget("team")->setVisible(false);
        }
        LayoutManager::calculateLayout(m_widgets, this);
        m_text_box = getWidget<TextBoxWidget>("chat");
    }
    else
        m_text_box = NULL;
        
    bool has_multitouch_gui = false;
    
    if (World::getWorld() && World::getWorld()->getRaceGUI() && 
        World::getWorld()->getRaceGUI()->getMultitouchGUI() &&
        !World::getWorld()->getRaceGUI()->getMultitouchGUI()->isSpectatorMode())
    {
        has_multitouch_gui = true;
    }
    
    IrrlichtDevice* irrlicht_device = irr_driver->getDevice();
    assert(irrlicht_device != NULL);
    bool accelerometer_available = irrlicht_device->isAccelerometerAvailable();
    
    if (!has_multitouch_gui || !accelerometer_available)
    {
        GUIEngine::RibbonWidget* backbtn_ribbon =
                            getWidget<GUIEngine::RibbonWidget>("backbtnribbon");
        backbtn_ribbon->removeChildNamed("touch_device");
    }

}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
void RacePausedDialog::init()
{
    m_touch_controls = UserConfigParams::m_multitouch_controls;
    updateTouchDeviceIcon();
    
}   // init

// ----------------------------------------------------------------------------
void RacePausedDialog::handleChat(const irr::core::stringw& text)
{
    if (auto cl = LobbyProtocol::get<ClientLobby>())
    {
        if (!text.empty())
        {
            if (text[0] == L'/' && text.size() > 1)
            {
                std::string cmd = StringUtils::wideToUtf8(text);
                cl->handleClientCommand(cmd.erase(0, 1));
            }
            else
                cl->sendChat(text, m_target_team);
        }
    }
    m_self_destroy = true;
}   // onEnterPressed

// ----------------------------------------------------------------------------
bool RacePausedDialog::onEnterPressed(const irr::core::stringw& text)
{
    // Assume enter for non team chat
    m_target_team = KART_TEAM_NONE;
    handleChat(text);
    return true;
}   // onEnterPressed

// ----------------------------------------------------------------------------
void RacePausedDialog::updateTouchDeviceIcon()
{
    GUIEngine::RibbonWidget* backbtn_ribbon =
                            getWidget<GUIEngine::RibbonWidget>("backbtnribbon");
    GUIEngine::IconButtonWidget* widget = (IconButtonWidget*)backbtn_ribbon->
                                                findWidgetNamed("touch_device");
    if (!widget)
        return;
                                                  
    switch (m_touch_controls)
    {
    case MULTITOUCH_CONTROLS_UNDEFINED:
    case MULTITOUCH_CONTROLS_STEERING_WHEEL:
        widget->setLabel(_("Steering wheel"));
        widget->setImage(irr_driver->getTexture(FileManager::GUI_ICON, 
                                                "android/steering_wheel.png"));
        break;
    case MULTITOUCH_CONTROLS_ACCELEROMETER:
        widget->setLabel(_("Accelerometer"));
        widget->setImage(irr_driver->getTexture(FileManager::GUI_ICON, 
                                                "android/accelerator_icon.png"));
        break;
    case MULTITOUCH_CONTROLS_GYROSCOPE:
        widget->setLabel(_("Gyroscope"));
        widget->setImage(irr_driver->getTexture(FileManager::GUI_ICON, 
                                                "android/gyroscope_icon.png"));
        break;
    default:
        break;
    }
}   // updateTouchDeviceIcon
