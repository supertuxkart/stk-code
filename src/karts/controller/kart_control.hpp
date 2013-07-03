//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008  Joerg Henrichs
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

#include "network/message.hpp"

/**
  * \ingroup controller
  */
class KartControl
{
public:
    /** The current steering value in [-1, 1]. */
    float m_steer;
    /** Acceleration, in [0, 1]. */
    float m_accel;
    /** True if the kart brakes. */
    bool  m_brake;
    /** True if the kart activates nitro. */
    bool  m_nitro;
    /** The skidding control state: SC_NONE: not pressed;
        SC_NO_DIRECTION: pressed, but no steering;
        SC_LEFT/RIGHT: pressed in the specified direction. */
    enum  SkidControl {SC_NONE, SC_NO_DIRECTION, SC_LEFT, SC_RIGHT}  
          m_skid;
    /** True if rescue is selected. */
    bool  m_rescue;
    /** True if fire is selected. */
    bool  m_fire;
    /** True if the kart looks (and shoots) backwards. */ 
    bool  m_look_back;

    KartControl()
    {
        reset();
    }
    // ------------------------------------------------------------------------
    /** Construct kart control from a Message (i.e. unserialise)             */
    KartControl(Message *m)
    {
        m_steer     = m->getFloat();
        m_accel     = m->getFloat();
        char c      = m->getChar();
        setButtonsCompressed(c);
    }   // KartControl(Message*)
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
    /** Return the serialised size in bytes.                                 */
    static int getLength() { return 9; }
    // ------------------------------------------------------------------------
    /** Serialises the kart control into a message.                          */
    void serialise(Message *m) const
    {
        m->addFloat(m_steer);
        m->addFloat(m_accel);
        m->addChar(getButtonsCompressed());
    }   // compress
    // ------------------------------------------------------------------------
    void uncompress(char *c)
    {
        m_steer = ((float*)c)[0];  
        m_accel = ((float*)c)[1];
        setButtonsCompressed(c[8]);
    }   // uncompress
    // ------------------------------------------------------------------------
    /** Compresses all buttons into a single integer value. */
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
    /** Sets the buttons from a compressed representation.
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
};

#endif

