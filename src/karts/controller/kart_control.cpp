//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2016  Joerg Henrichs
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

#include "karts/controller/kart_control.hpp"

#include "network/rewind_manager.hpp"


// ------------------------------------------------------------------------
/** Called when going back in time during a rewind. Nothing to do here
 *  in this case.
 */
void KartControl::undo(BareNetworkString *buffer)
{
}   // undo

// ------------------------------------------------------------------------
/** Replay an event for a KartControl object from the buffer.
 *  \param buffer BareNetworkString with saved event into.
 */
void KartControl::rewind(BareNetworkString *buffer)
{
    if(buffer->getTotalSize()>1)
    {
        // Full state including accel and steering was saved
        setFromBuffer(buffer);
    }
    else // only a button event was stored
    {
        setButtonsCompressed(buffer->getUInt8());
    }
}   // rewind

// ------------------------------------------------------------------------
/** Sets this KartControl form the given value (basically a copy). This
 *  function uses the explicit setSteer() etc function, which means that
 *  rewind information will be collected.
 */
void KartControl::set(const KartControl &c)
{
    setAccel(c.getAccel());
    setBrake(c.getBrake());
    setFire(c.getFire());
    setLookBack(c.getLookBack());
    setNitro(c.getNitro());
    setRescue(c.getRescue());
    setSkidControl(c.getSkidControl());
    setSteer(c.getSteer());
}   // set

// ------------------------------------------------------------------------
/** Sets the current steering value. */
void KartControl::setSteer(float f)
{
    float old_steer = m_steer;
    m_steer         = f;
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_steer != m_steer                                                  )
    {
        // Save full status
        BareNetworkString *buffer = new BareNetworkString(getLength());
        copyToBuffer(buffer);
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setSteer

// ----------------------------------------------------------------------------
/** Sets the acceleration. */
void KartControl::setAccel(float f)
{
    float old_accel = m_accel;
    m_accel         = f; 
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_accel != m_accel                                                  )
    {
        BareNetworkString *buffer = new BareNetworkString(getLength());
        copyToBuffer(buffer);
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setAccel

// ----------------------------------------------------------------------------
/** Sets if the kart is braking. */
void KartControl::setBrake(bool b)
{
    bool old_brake    = m_brake;
    m_brake           = b;
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_brake != m_brake                                 )
    {
        // Only store the buttons in this case
        BareNetworkString *buffer = new BareNetworkString(1);
        buffer->addUInt8(getButtonsCompressed());
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setBrake
// ----------------------------------------------------------------------------
/** Sets if the kart activates nitro. */
void KartControl::setNitro(bool b)
{
    bool old_nitro = m_nitro;
    m_nitro        = b;
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_nitro != m_nitro                                 )
    {
        BareNetworkString *buffer = new BareNetworkString(1);
        buffer->addUInt8(getButtonsCompressed());
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setNitro

// ----------------------------------------------------------------------------
/** Sets the skid control for this kart. */
void KartControl::setSkidControl(SkidControl sc)
{
    SkidControl old_skid = m_skid;
    m_skid        = sc;
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_skid != m_skid                                  )
    {
        BareNetworkString *buffer = new BareNetworkString(1);
        buffer->addUInt8(getButtonsCompressed());
        RewindManager::get()->addEvent(this, buffer);
    }
}   // seSkidControl

// ----------------------------------------------------------------------------
/** Returns if this kart wants to get rescued. */
void KartControl::setRescue(bool b)
{ 
    bool old_rescue = m_rescue;
    m_rescue        = b;
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_rescue != m_rescue)
    {
        BareNetworkString *buffer = new BareNetworkString(1);
        buffer->addUInt8(getButtonsCompressed());
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setRescue

// ----------------------------------------------------------------------------
/** Sets if the kart wants to fire. */
void KartControl::setFire(bool b)
{ 
    bool old_fire = m_fire;
    m_fire        = b; 
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_fire != m_fire                                                    )
    {
        BareNetworkString *buffer = new BareNetworkString(1);
        buffer->addUInt8(getButtonsCompressed());
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setFire

// ----------------------------------------------------------------------------
/** Sets if the kart wants to look (and therefore also fires) backwards. */
void KartControl::setLookBack(bool b)
{
    bool old_look = m_look_back;
    m_look_back   = b;
    if (RewindManager::isEnabled() && !RewindManager::get()->isRewinding() &&
        old_look != m_look_back)
    {
        BareNetworkString *buffer = new BareNetworkString(1);
        buffer->addUInt8(getButtonsCompressed());
        RewindManager::get()->addEvent(this, buffer);
    }
}   // setLookBack
