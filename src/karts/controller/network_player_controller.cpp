#include "karts/controller/network_player_controller.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/post_processing.hpp"
#include "input/input_manager.hpp"
#include "items/attachment.hpp"
#include "items/item.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_characteristic.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding.hpp"
#include "karts/rescue_animation.hpp"
#include "modes/world.hpp"
#include "race/history.hpp"
#include "states_screens/race_gui_base.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

NetworkPlayerController::NetworkPlayerController(AbstractKart *kart,
                                   StateManager::ActivePlayer *player)
                : Controller(kart)
{
    assert(player != NULL);
    m_player       = player;
    m_player->setKart(kart);
    m_penalty_time = 0.0f;

    reset();

    Log::info("NetworkPlayerController", "New network player controller.");
}   // NetworkPlayerController

//-----------------------------------------------------------------------------
/** Destructor for a player kart.
 */
NetworkPlayerController::~NetworkPlayerController()
{
}   // ~NetworkPlayerController

//-----------------------------------------------------------------------------
/** Resets the player kart for a new or restarted race.
 */
void NetworkPlayerController::reset()
{
    m_steer_val_l  = 0;
    m_steer_val_r  = 0;
    m_steer_val    = 0;
    m_prev_brake   = 0;
    m_prev_accel   = 0;
    m_prev_nitro   = false;
    m_penalty_time = 0;
}   // reset

// ----------------------------------------------------------------------------
/** Resets the state of control keys. This is used after the in-game menu to
 *  avoid that any keys pressed at the time the menu is opened are still
 *  considered to be pressed.
 */
void NetworkPlayerController::resetInputState()
{
    m_steer_val_l           = 0;
    m_steer_val_r           = 0;
    m_steer_val             = 0;
    m_prev_brake            = 0;
    m_prev_accel            = 0;
    m_prev_nitro            = false;
    m_controls->reset();
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
void NetworkPlayerController::action(PlayerAction action, int value)
{
    switch (action)
    {
    case PA_STEER_LEFT:
        m_steer_val_l = value;
        if (value)
        {
          m_steer_val = value;
          if(m_controls->m_skid==KartControl::SC_NO_DIRECTION)
              m_controls->m_skid = KartControl::SC_LEFT;
        }
        else
          m_steer_val = m_steer_val_r;

        break;
    case PA_STEER_RIGHT:
        m_steer_val_r = -value;
        if (value)
        {
            m_steer_val = -value;
            if(m_controls->m_skid==KartControl::SC_NO_DIRECTION)
                m_controls->m_skid = KartControl::SC_RIGHT;
        }
        else
          m_steer_val = m_steer_val_l;

        break;
    case PA_ACCEL:
        m_prev_accel = value;
        if (value && !(m_penalty_time > 0.0f))
        {
            m_controls->m_accel = value/32768.0f;
            m_controls->m_brake = false;
            m_controls->m_nitro = m_prev_nitro;
        }
        else
        {
            m_controls->m_accel = 0.0f;
            m_controls->m_brake = m_prev_brake;
            m_controls->m_nitro = false;
        }
        break;
    case PA_BRAKE:
        m_prev_brake = value!=0;
        // let's consider below that to be a deadzone
        if(value > 32768/2)
        {
            m_controls->m_brake = true;
            m_controls->m_accel = 0.0f;
            m_controls->m_nitro = false;
        }
        else
        {
            m_controls->m_brake = false;
            m_controls->m_accel = m_prev_accel/32768.0f;
            // Nitro still depends on whether we're accelerating
            m_controls->m_nitro = (m_prev_nitro && m_prev_accel);
        }
        break;
    case PA_NITRO:
        // This basically keeps track whether the button still is being pressed
        m_prev_nitro = (value != 0);
        // Enable nitro only when also accelerating
        m_controls->m_nitro = ((value!=0) && m_controls->m_accel);
        break;
    case PA_RESCUE:
        m_controls->m_rescue = (value!=0);
        break;
    case PA_FIRE:
    {
        m_controls->m_fire = (value!=0);
        break;
    }
    case PA_LOOK_BACK:
        m_controls->m_look_back = (value!=0);
        break;
    case PA_DRIFT:
        if(value==0)
            m_controls->m_skid = KartControl::SC_NONE;
        else
            if(m_steer_val==0)
                m_controls->m_skid = KartControl::SC_NO_DIRECTION;
            else
                m_controls->m_skid = m_steer_val<0
                                   ? KartControl::SC_RIGHT
                                   : KartControl::SC_LEFT;
        break;
    case PA_PAUSE_RACE:
        if (value != 0) StateManager::get()->escapePressed();
        break;
    default:
       break;
    }

}   // action

//-----------------------------------------------------------------------------
/** Handles steering for a player kart.
 */
void NetworkPlayerController::steer(float dt, int steer_val)
{
    if(stk_config->m_disable_steer_while_unskid &&
        m_controls->m_skid==KartControl::SC_NONE &&
       m_kart->getSkidding()->getVisualSkidRotation()!=0)
    {
        m_controls->m_steer = 0;
    }


    // Amount the steering is changed for digital devices.
    // If the steering is 'back to straight', a different steering
    // change speed is used.
    const float STEER_CHANGE = ( (steer_val<=0 && m_controls->m_steer<0) ||
                                 (steer_val>=0 && m_controls->m_steer>0)   )
                     ? dt/m_kart->getCharacteristic()->getTurnTimeResetSteer()
                     : dt/m_kart->getTimeFullSteer(fabsf(m_controls->m_steer));
    if (steer_val < 0)
    {
        // If we got analog values do not cumulate.
        if (steer_val > -32767)
            m_controls->m_steer = -steer_val/32767.0f;
        else
            m_controls->m_steer += STEER_CHANGE;
    }
    else if(steer_val > 0)
    {
        // If we got analog values do not cumulate.
        if (steer_val < 32767)
            m_controls->m_steer = -steer_val/32767.0f;
        else
            m_controls->m_steer -= STEER_CHANGE;
    }
    else
    {   // no key is pressed
        if(m_controls->m_steer>0.0f)
        {
            m_controls->m_steer -= STEER_CHANGE;
            if(m_controls->m_steer<0.0f) m_controls->m_steer=0.0f;
        }
        else
        {   // m_controls->m_steer<=0.0f;
            m_controls->m_steer += STEER_CHANGE;
            if(m_controls->m_steer>0.0f) m_controls->m_steer=0.0f;
        }   // if m_controls->m_steer<=0.0f
    }   // no key is pressed
    if(UserConfigParams::m_gamepad_debug)
    {
        Log::debug("PlayerController", "  set to: %f\n", m_controls->m_steer);
    }

    m_controls->m_steer = std::min(1.0f, std::max(-1.0f, m_controls->m_steer));

}   // steer

//-----------------------------------------------------------------------------
/** Callback when the skidding bonus is triggered. The player controller
 *  resets the current steering to 0, which makes the kart easier to control.
 */
void NetworkPlayerController::skidBonusTriggered()
{
    m_controls->m_steer = 0;
}   // skidBonusTriggered

//-----------------------------------------------------------------------------
/** Updates the player kart, called once each timestep.
 */
void NetworkPlayerController::update(float dt)
{
    if (UserConfigParams::m_gamepad_debug)
    {
        // Print a dividing line so that it's easier to see which events
        // get received in which order in the one frame.
        Log::debug("PlayerController", "irr_driver", "-------------------------------------");
    }

    // Don't do steering if it's replay. In position only replay it doesn't
    // matter, but if it's physics replay the gradual steering causes
    // incorrect results, since the stored values are already adjusted.
    if (!history->replayHistory())
        steer(dt, m_steer_val);

    if (World::getWorld()->isStartPhase())
    {
        if (m_controls->m_accel || m_controls->m_brake ||
            m_controls->m_fire  || m_controls->m_nitro)
        {
            // Only give penalty time in SET_PHASE.
            // Penalty time check makes sure it doesn't get rendered on every
            // update.
            if (m_penalty_time == 0.0 &&
                World::getWorld()->getPhase() == WorldStatus::SET_PHASE)
            {
                RaceGUIBase* m=World::getWorld()->getRaceGUI();
                if (m)
                {
                    m->addMessage(_("Penalty time!!"), m_kart, 2.0f,
                                  video::SColor(255, 255, 128, 0));
                    m->addMessage(_("Don't accelerate before go"), m_kart, 2.0f,
                                  video::SColor(255, 210, 100, 50));
                }

                m_penalty_time = stk_config->m_penalty_time;
            }   // if penalty_time = 0

            m_controls->m_brake = false;
            m_controls->m_accel = 0.0f;
        }   // if key pressed

        return;
    }   // if isStartPhase

    if (m_penalty_time>0.0)
    {
        m_penalty_time-=dt;
        return;
    }

    // We can't restrict rescue to fulfil isOnGround() (which would be more like
    // MK), since e.g. in the City track it is possible for the kart to end
    // up sitting on a brick wall, with all wheels in the air :((
    // Only accept rescue if there is no kart animation is already playing
    // (e.g. if an explosion happens, wait till the explosion is over before
    // starting any other animation).
    if ( m_controls->m_rescue && !m_kart->getKartAnimation() )
    {
        new RescueAnimation(m_kart);
        m_controls->m_rescue=false;
    }
}   // update

//-----------------------------------------------------------------------------
/** Checks if the kart was overtaken, and if so plays a sound
*/
void NetworkPlayerController::setPosition(int p)
{
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
void NetworkPlayerController::finishedRace(float time)
{

}   // finishedRace

//-----------------------------------------------------------------------------
/** Called when a kart hits or uses a zipper.
 */
void NetworkPlayerController::handleZipper(bool play_sound)
{
    m_kart->showZipperFire();
}   // handleZipper

//-----------------------------------------------------------------------------
/** Called when a kart hits an item.
 *  \param item Item that was collected.
 *  \param add_info Additional info to be used then handling the item. If
 *                  this is -1 (default), the item type is selected
 *                  randomly. Otherwise it contains the powerup or
 *                  attachment for the kart. This is used in network mode to
 *                  let the server determine the powerup/attachment for
 *                  the clients.
 */
void NetworkPlayerController::collectedItem(const Item &item, int add_info, float old_energy)
{

}   // collectedItem
