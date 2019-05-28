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
#include "network/network_string.hpp"

#include "irrMath.h"
#include <algorithm>

// ------------------------------------------------------------------------
/** Sets the current steering value. */
void KartControl::setSteer(float f)
{
    int steer = irr::core::clamp((int)(f * 32767.0f), -32767, 32767);
    m_steer = (int16_t)steer;
}   // setSteer

// ----------------------------------------------------------------------------
/** Sets the acceleration. */
void KartControl::setAccel(float f)
{
    int accel = std::min((int)(f * 65535.0f), 65535);
    m_accel = (uint16_t)accel;
}   // setAccel

// ----------------------------------------------------------------------------
/** Sets if the kart is braking. */
void KartControl::setBrake(bool b)
{
    m_brake           = b;
}   // setBrake

// ----------------------------------------------------------------------------
/** Sets if the kart activates nitro. */
void KartControl::setNitro(bool b)
{
    m_nitro        = b;
}   // setNitro

// ----------------------------------------------------------------------------
/** Sets the skid control for this kart. */
void KartControl::setSkidControl(SkidControl sc)
{
    m_skid        = sc;
}   // seSkidControl

// ----------------------------------------------------------------------------
/** Returns if this kart wants to get rescued. */
void KartControl::setRescue(bool b)
{ 
    m_rescue        = b;
}   // setRescue

// ----------------------------------------------------------------------------
/** Sets if the kart wants to fire. */
void KartControl::setFire(bool b)
{ 
    m_fire        = b; 
}   // setFire

// ----------------------------------------------------------------------------
/** Sets if the kart wants to look (and therefore also fires) backwards. */
void KartControl::setLookBack(bool b)
{
    m_look_back   = b;
}   // setLookBack
// ----------------------------------------------------------------------------
/** Copies the important data from this objects into a memory buffer. */
void KartControl::saveState(BareNetworkString *buffer) const
{
    buffer->addUInt16(m_steer);
    buffer->addUInt16(m_accel);
    buffer->addChar(getButtonsCompressed());
}   // saveState

// ----------------------------------------------------------------------------
/** Restores this object from a previously saved memory  buffer. */
void KartControl::rewindTo(BareNetworkString *buffer)
{
    m_steer = buffer->getUInt16();
    m_accel = buffer->getUInt16();
    setButtonsCompressed(buffer->getUInt8());
}   // setFromMemory
