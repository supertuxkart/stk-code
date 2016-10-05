//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015  Joerg Henrichs
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

#ifndef HEADER_KART_CONTROL_HPP
#define HEADER_KART_CONTROL_HPP

#include "network/network_string.hpp"
#include "network/rewind_info.hpp"

/**
  * \ingroup controller
  */
class KartControl : public EventRewinder
{
public:
    /** The skidding control state: SC_NONE: not pressed;
     *  SC_NO_DIRECTION: pressed, but no steering;
     *  SC_LEFT/RIGHT: pressed in the specified direction. */
    enum  SkidControl {SC_NONE, SC_NO_DIRECTION, SC_LEFT, SC_RIGHT};

private:
    /** The current steering value in [-1, 1]. */
    float m_steer;
    /** Acceleration, in [0, 1]. */
    float m_accel;
    /** True if the kart brakes. */
    bool  m_brake;
    /** True if the kart activates nitro. */
    bool  m_nitro;
    /** Skidding control state. */
    SkidControl m_skid;
    /** True if rescue is selected. */
    bool  m_rescue;
    /** True if fire is selected. */
    bool  m_fire;
    /** True if the kart looks (and shoots) backwards. */
    bool  m_look_back;
public:
    virtual void undo(BareNetworkString *buffer);
    virtual void rewind(BareNetworkString *buffer);
    void setSteer(float f);
    void setAccel(float f);
    void setBrake(bool b);
    void setNitro(bool b);
    void setSkidControl(SkidControl sc);
    void setRescue(bool b);
    void setFire(bool b);
    void setLookBack(bool b);
    void set(const KartControl &c);

    // ------------------------------------------------------------------------
    KartControl()
    {
        reset();
    }
    // ------------------------------------------------------------------------
    /** Resets all controls. */
    void reset()
    {
        m_steer     = 0.0f;
        m_accel     = 0.0f;
        m_brake     = false;
        m_nitro     = false;
        m_skid      = SC_NONE;
        m_rescue    = false;
        m_fire      = false;
        m_look_back = false;
    }   // reset
    // ------------------------------------------------------------------------
    /** Tests if two KartControls are equal. 
      */
    bool operator==(const KartControl &other)
    {
        return m_steer     == other.m_steer   &&
               m_accel     == other.m_accel   &&
               m_brake     == other.m_brake   &&
               m_nitro     == other.m_nitro   &&
               m_skid      == other.m_skid    &&
               m_rescue    == other.m_rescue  &&
               m_fire      == other.m_fire    &&
               m_look_back == other.m_look_back;
    }    // operator==

    // ------------------------------------------------------------------------
    /** Return the serialised size in bytes.                                 */
    static int getLength() { return 9; }
    // ------------------------------------------------------------------------
    /** Copies the important data from this objects into a memory buffer. */
    void copyToBuffer(BareNetworkString *buffer) const
    {
        buffer->add(m_steer);
        buffer->add(m_accel);
        buffer->addChar(getButtonsCompressed());
    }   // copyToBuffer

    // ------------------------------------------------------------------------
    /** Restores this object from a previously saved memory  buffer. */
    void setFromBuffer(BareNetworkString *buffer)
    {
        m_steer = buffer->getFloat();
        m_accel = buffer->getFloat();
        setButtonsCompressed(buffer->getUInt8());
    }   // setFromMemory

    // ------------------------------------------------------------------------
    /** Compresses all buttons into a single byte. */
    char getButtonsCompressed() const
    {
        return  (m_brake     ?  1 : 0)
              + (m_nitro     ?  2 : 0)
              + (m_rescue    ?  4 : 0)
              + (m_fire      ?  8 : 0)
              + (m_look_back ? 16 : 0)
              + (m_skid<<5);             // m_skid is in {0,1,2,3}
    }   // getButtonsCompressed
    // ------------------------------------------------------------------------
    /** Sets the buttons from a compressed (1 byte) representation.
     *  /param c Character containing the compressed representation.
     */
    void setButtonsCompressed(char c)
    {
        m_brake     = (c &  1) != 0;
        m_nitro     = (c &  2) != 0;
        m_rescue    = (c &  4) != 0;
        m_fire      = (c &  8) != 0;
        m_look_back = (c & 16) != 0;
        m_skid      = (SkidControl)((c & 96) >> 5);
    }   // setButtonsCompressed
    // ------------------------------------------------------------------------
    /** Returns the current steering value. */
    float getSteer() const { return m_steer; }
    // ------------------------------------------------------------------------
    /** Returns current acceleration. */
    float getAccel() const { return m_accel; }
    // ------------------------------------------------------------------------
    /** Returns if the kart is braking. */
    bool getBrake() const { return m_brake; }
    // ------------------------------------------------------------------------
    /** Returns if the kart activates nitro. */
    bool  getNitro() const { return m_nitro; }
    // ------------------------------------------------------------------------
    /** Returns the skidding control state: SC_NONE: not pressed;
     *  SC_NO_DIRECTION: pressed, but no steering;
     *  SC_LEFT/RIGHT: pressed in the specified direction. */
    SkidControl getSkidControl() const { return m_skid; }
    // ------------------------------------------------------------------------
    /** Returns true if the kart triggered rescue. */
    bool getRescue() const { return m_rescue; }
    // ------------------------------------------------------------------------
    /** Returns if fire is selected. */
    bool getFire() const { return m_fire; }
    // ------------------------------------------------------------------------
    /** Returns if the kart wants to look back (which also implies that it
     *  will fire backwards. */
    bool getLookBack() const { return m_look_back; }
};

#endif

