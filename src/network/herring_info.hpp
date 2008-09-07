//  $Id: herring_info.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_HERRING_INFO_HPP
#define HEADER_HERRING_INFO_HPP
/** Class used to transfer information about collected herrings from
*  server to client.
*/
class HerringInfo
{
public:
    /** Kart id (in world) of the kart collecting the herring. */
    unsigned char   m_kart_id;
    /** Index of the collected herring. This is set to -1 if a kart
     *  triggers a rescue (i.e. attaches the butterfly).]
     */
    short           m_herring_id;
    /** Additional info used, depending on herring type. This is usually
     *  the type of the collected herring.
     */
    char            m_add_info;

    /** Constructor to initialise all fields. */
    HerringInfo(int kart, int herring, char add_info) :
        m_kart_id(kart), m_herring_id(herring),
        m_add_info(add_info)  
    {}
    // -------------------------------------------------------------
    /** Construct HerringInfo from a message (which is unpacked). */
    HerringInfo(Message *m)
    {
        m_kart_id    = m->getChar();
        m_herring_id = m->getShort();
        m_add_info   = m->getChar();
    }
    // -------------------------------------------------------------
    /*** Returns size in bytes necessary to store HerringInfo. */
    static int getLength() {return 2*Message::getCharLength()
                                 +   Message::getShortLength();}
    // -------------------------------------------------------------
    /** Serialises this object into the message object */
    void serialise(Message *m)
    {
        m->addChar(m_kart_id);
        m->addShort(m_herring_id);
        m->addChar(m_add_info);
    }   // serialise
};   // HerringInfo

#endif