//  $Id: kart_control.hpp 2173 2008-07-21 01:55:41Z auria $
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
    float lr;
    float accel;
    bool  brake;
    bool  wheelie;
    bool  jump;
    bool  rescue;
    bool  fire;

    KartControl() : lr(0.0f), accel(0.0f), brake(false),
                    wheelie(false), jump(false),  rescue(false), 
                    fire(false)
    {
    }
    // ------------------------------------------------------------------------
    /** Construct kart control from a Message (i.e. unserialise)             */
    KartControl(Message *m)
    {
        lr      = m->getFloat();
        accel   = m->getFloat();
        char c  = m->getChar();
        brake   = (c &  1) != 0;
        wheelie = (c &  2) != 0;
        jump    = (c &  4) != 0;
        rescue  = (c &  8) != 0;
        fire    = (c & 16) != 0;
    }   // KartControl(Message*)
    // ------------------------------------------------------------------------
    /** Return the serialised size in bytes.                                 */
    static int getLength() { return 9; }
    // ------------------------------------------------------------------------
    /** Serialises the kart control into a message.                          */
    void serialise(Message *m) const
    {
        m->addFloat(lr);
        m->addFloat(accel);
        m->addChar(getButtonsCompressed());
    }   // compress
    // ------------------------------------------------------------------------
    void uncompress(char *c)
    {
        lr      = ((float*)c)[0];  
        accel   = ((float*)c)[1];
        setButtonsCompressed(c[8]);
    }   // uncompress
    // ------------------------------------------------------------------------
    /** Compresses all buttons into a single integer value. */
    char getButtonsCompressed() const
    {
        return  brake   ?  1 : 0
              + wheelie ?  2 : 0
              + jump    ?  4 : 0
              + rescue  ?  8 : 0
              + fire    ? 16 : 0;
    }   // getButtonsCompressed
    // ------------------------------------------------------------------------
    /** Sets the buttons from a compressed representation.
     *  /param c Character containing the compressed representation.
     */
    void setButtonsCompressed(char c)
    {
        brake   = (c &  1) != 0;
        wheelie = (c &  2) != 0;
        jump    = (c &  4) != 0;
        rescue  = (c &  8) != 0;
        fire    = (c & 16) != 0;
    }   // setButtonsCompressed
};

#endif

