//  $Id$
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

class KartControl
{
public:
    float m_steer;
    float m_accel;
    bool  m_brake;
    bool  m_nitro;
    bool  m_drift;
    bool  m_rescue;
    bool  m_fire;
    bool  m_look_back;

    KartControl() : m_steer(0.0f), m_accel(0.0f), m_brake(false),
                    m_nitro(false), m_drift(false),  m_rescue(false), 
                    m_fire(false), m_look_back(false)
    {
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
        return  m_brake     ?  1 : 0
              + m_nitro     ?  2 : 0
              + m_drift     ?  4 : 0
              + m_rescue    ?  8 : 0
              + m_fire      ? 16 : 0
              + m_look_back ? 32 : 0;
    }   // getButtonsCompressed
    // ------------------------------------------------------------------------
    /** Sets the buttons from a compressed representation.
     *  /param c Character containing the compressed representation.
     */
    void setButtonsCompressed(char c)
    {
        m_brake     = (c &  1) != 0;
        m_nitro     = (c &  2) != 0;
        m_drift     = (c &  4) != 0;
        m_rescue    = (c &  8) != 0;
        m_fire      = (c & 16) != 0;
        m_look_back = (c & 32) != 0;
    }   // setButtonsCompressed
};

#endif

