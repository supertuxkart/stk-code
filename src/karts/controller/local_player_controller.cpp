//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, Steve Baker
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

#include "karts/controller/local_player_controller.hpp"

#include "audio/sfx_base.hpp"
#include "config/player_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/post_processing.hpp"
#include "input/input_manager.hpp"
#include "items/attachment.hpp"
#include "items/item.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding.hpp"
#include "karts/rescue_animation.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/race_event_manager.hpp"
#include "race/history.hpp"
#include "states_screens/race_gui_base.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

/** The constructor for a loca player kart, i.e. a player that is playing
 *  on this machine (non-local player would be network clients).
 *  \param kart_name Name of the kart.
 *  \param position The starting position (1 to n).
 *  \param player The player to which this kart belongs.
 *  \param init_pos The start coordinates and heading of the kart.
 */
LocalPlayerController::LocalPlayerController(AbstractKart *kart,
                                   StateManager::ActivePlayer *player)
                     : PlayerController(kart)
{
    m_player = player;
    if(player)
        player->setKart(kart);

    // Keep a pointer to the camera to remove the need to search for
    // the right camera once per frame later.
    Camera *camera = Camera::createCamera(kart);
    m_camera_index = camera->getIndex();
    m_bzzt_sound   = SFXManager::get()->createSoundSource("bzzt");
    m_wee_sound    = SFXManager::get()->createSoundSource("wee");
    m_ugh_sound    = SFXManager::get()->createSoundSource("ugh");
    m_grab_sound   = SFXManager::get()->createSoundSource("grab_collectable");
    m_full_sound   = SFXManager::get()->createSoundSource("energy_bar_full");
}   // LocalPlayerController

//-----------------------------------------------------------------------------
/** Destructor for a player kart.
 */
LocalPlayerController::~LocalPlayerController()
{
    m_bzzt_sound->deleteSFX();
    m_wee_sound ->deleteSFX();
    m_ugh_sound ->deleteSFX();
    m_grab_sound->deleteSFX();
    m_full_sound->deleteSFX();
}   // ~LocalPlayerController

//-----------------------------------------------------------------------------
/** Resets the player kart for a new or restarted race.
 */
void LocalPlayerController::reset()
{
    PlayerController::reset();
    m_sound_schedule = false;
}   // reset

// ----------------------------------------------------------------------------
/** Resets the state of control keys. This is used after the in-game menu to
 *  avoid that any keys pressed at the time the menu is opened are still
 *  considered to be pressed.
 */
void LocalPlayerController::resetInputState()
{
    PlayerController::resetInputState();
    m_sound_schedule = false;
}   // resetInputState

// ----------------------------------------------------------------------------
/** This function interprets a kart action and value, and set the corresponding
 *  entries in the kart control data structure. This function handles esp.
 *  cases like 'press left, press right, release right' - in this case after
 *  releasing right, the steering must switch to left again. Similarly it
 *  handles 'press left, press right, release left' (in which case still
 *  right must be selected). Similarly for braking and acceleration.
 * \param action  The action to be executed.
 * \param value   If 32768, it indicates a digital value of 'fully set'
 *                if between 1 and 32767, it indicates an analog value,
 *                and if it's 0 it indicates that the corresponding button
 *                was released.
 */
void LocalPlayerController::action(PlayerAction action, int value)
{
    PlayerController::action(action, value);

    // If this is a client, send the action to the server
    if (World::getWorld()->isNetworkWorld()      && 
        NetworkConfig::get()->isClient()         &&
        RaceEventManager::getInstance()->isRunning()    )
    {
        RaceEventManager::getInstance()->controllerAction(this, action, value);
    }

}   // action

//-----------------------------------------------------------------------------
/** Handles steering for a player kart.
 */
void LocalPlayerController::steer(float dt, int steer_val)
{
    if(UserConfigParams::m_gamepad_debug)
    {
        Log::debug("LocalPlayerController", "steering: steer_val %d ", steer_val);
        RaceGUIBase* gui_base = World::getWorld()->getRaceGUI();
        gui_base->clearAllMessages();
        gui_base->addMessage(StringUtils::insertValues(L"steer_val %i", steer_val),
                             m_kart, 1.0f,
                             video::SColor(255, 255, 0, 255), false);
    }
    PlayerController::steer(dt, steer_val);
    
    if(UserConfigParams::m_gamepad_debug)
    {
        Log::debug("LocalPlayerController", "  set to: %f\n",
                   m_controls->getSteer());
    }
}   // steer

//-----------------------------------------------------------------------------
/** Updates the player kart, called once each timestep.
 */
void LocalPlayerController::update(float dt)
{
    if (UserConfigParams::m_gamepad_debug)
    {
        // Print a dividing line so that it's easier to see which events
        // get received in which order in the one frame.
        Log::debug("LocalPlayerController", "irr_driver", "-------------------------------------");
    }

    PlayerController::update(dt);

    // look backward when the player requests or
    // if automatic reverse camera is active
    Camera *camera = Camera::getCamera(m_camera_index);
    if (camera->getType() != Camera::CM_TYPE_END)
    {
        if (m_controls->getLookBack() || (UserConfigParams::m_reverse_look_threshold > 0 &&
            m_kart->getSpeed() < -UserConfigParams::m_reverse_look_threshold))
        {
            camera->setMode(Camera::CM_REVERSE);
        }
        else
        {
            if (camera->getMode() == Camera::CM_REVERSE)
                camera->setMode(Camera::CM_NORMAL);
        }
    }

    if (m_kart->getKartAnimation() && m_sound_schedule == false &&
        m_kart->getAttachment()->getType() != Attachment::ATTACH_TINYTUX)
    {
        m_sound_schedule = true;
    }
    else if (!m_kart->getKartAnimation() && m_sound_schedule == true)
    {
        m_sound_schedule = false;
        m_bzzt_sound->play();
    }
}   // update

//-----------------------------------------------------------------------------
/** Displays a penalty warning for player controlled karts. Called from
 *  LocalPlayerKart::update() if necessary.
 */
void LocalPlayerController::displayPenaltyWarning()
{
    RaceGUIBase* m=World::getWorld()->getRaceGUI();
    if (m)
    {
        m->addMessage(_("Penalty time!!"), m_kart, 2.0f,
                      GUIEngine::getSkin()->getColor("font::top"));
        m->addMessage(_("Don't accelerate before go"), m_kart, 2.0f,
                      GUIEngine::getSkin()->getColor("font::normal"));
    }
    m_bzzt_sound->play();
}   // displayPenaltyWarning

//-----------------------------------------------------------------------------
/** Called just before the kart position is changed. It checks if the kart was
 *  overtaken, and if so plays a sound from the overtaking kart.
 */
void LocalPlayerController::setPosition(int p)
{
    PlayerController::setPosition(p);


    if(m_kart->getPosition()<p)
    {
        World *world = World::getWorld();
        //have the kart that did the passing beep.
        //I'm not sure if this method of finding the passing kart is fail-safe.
        for(unsigned int i = 0 ; i < world->getNumKarts(); i++ )
        {
            AbstractKart *kart = world->getKart(i);
            if(kart->getPosition() == p + 1)
            {
                kart->beep();
                break;
            }
        }
    }
}   // setPosition

//-----------------------------------------------------------------------------
/** Called when a kart finishes race.
 *  /param time Finishing time for this kart.
 d*/
void LocalPlayerController::finishedRace(float time)
{
    // This will implicitely trigger setting the first end camera to be active
    Camera::changeCamera(m_camera_index, Camera::CM_TYPE_END);
}   // finishedRace

//-----------------------------------------------------------------------------
/** Called when a kart hits or uses a zipper.
 */
void LocalPlayerController::handleZipper(bool play_sound)
{
    PlayerController::handleZipper(play_sound);

    // Only play a zipper sound if it's not already playing, and
    // if the material has changed (to avoid machine gun effect
    // on conveyor belt zippers).
    if (play_sound || (m_wee_sound->getStatus() != SFXBase::SFX_PLAYING &&
                       m_kart->getMaterial()!=m_kart->getLastMaterial()      ) )
    {
        m_wee_sound->play();
    }

    // Apply the motion blur according to the speed of the kart
    irr_driver->getPostProcessing()->giveBoost(m_camera_index);

}   // handleZipper

//-----------------------------------------------------------------------------
/** Called when a kart hits an item. It plays certain sfx (e.g. nitro full,
 *  or item specific sounds).
 *  \param item Item that was collected.
 *  \param add_info Additional info to be used then handling the item. If
 *                  this is -1 (default), the item type is selected
 *                  randomly. Otherwise it contains the powerup or
 *                  attachment for the kart. This is used in network mode to
 *                  let the server determine the powerup/attachment for
 *                  the clients.
 *  \param old_energy The previous energy value
 */
void LocalPlayerController::collectedItem(const Item &item, int add_info,
                                          float old_energy)
{
    if (old_energy < m_kart->getKartProperties()->getNitroMax() &&
        m_kart->getEnergy() == m_kart->getKartProperties()->getNitroMax())
    {
        m_full_sound->play();
    }
    else if (race_manager->getCoinTarget() > 0 &&
             old_energy < race_manager->getCoinTarget() &&
             m_kart->getEnergy() == race_manager->getCoinTarget())
    {
        m_full_sound->play();
    }
    else
    {
        switch(item.getType())
        {
        case Item::ITEM_BANANA:
            m_ugh_sound->play();
            break;
        case Item::ITEM_BUBBLEGUM:
            //More sounds are played by the kart class
            //See Kart::collectedItem()
            m_ugh_sound->play();
            break;
        case Item::ITEM_TRIGGER:
            // no default sound for triggers
            break;
        default:
            m_grab_sound->play();
            break;
        }
    }
}   // collectedItem

// ----------------------------------------------------------------------------
/** Returns true if the player of this controller can collect achievements.
 *  At the moment only the current player can collect them.
 *  TODO: check this, possible all local players should be able to
 *        collect achievements - synching to online account will happen
 *        next time the account gets online.
 */
bool LocalPlayerController::canGetAchievements() const 
{
    return m_player->getConstProfile() == PlayerManager::getCurrentPlayer();
}   // canGetAchievements
