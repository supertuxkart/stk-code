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

#ifndef HEADER_ITEM_INFO_HPP
#define HEADER_ITEM_INFO_HPP
/** Class used to transfer information about collected items from
*  server to client.
*/
class ItemInfo
{
public:
    /** Kart id (in world) of the kart collecting the item. */
    unsigned char   m_kart_id;
    /** Index of the collected item. This is set to -1 if a kart
     *  triggers a rescue (i.e. attaches the butterfly).]
     */
    short           m_item_id;
    /** Additional info used, depending on item type. This is usually
     *  the type of the collected item.
     */
    char            m_add_info;

    /** Constructor to initialise all fields. */
    ItemInfo(int kart, int item, char add_info) :
        m_kart_id(kart), m_item_id(item),
        m_add_info(add_info)  
    {}
    // -------------------------------------------------------------
    /** Construct ItemInfo from a message (which is unpacked). */
    ItemInfo(Message *m)
    {
        m_kart_id    = m->getChar();
        m_item_id = m->getShort();
        m_add_info   = m->getChar();
    }
    // -------------------------------------------------------------
    /*** Returns size in bytes necessary to store ItemInfo. */
    static int getLength() {return 2*Message::getCharLength()
                                 +   Message::getShortLength();}
    // -------------------------------------------------------------
    /** Serialises this object into the message object */
    void serialise(Message *m)
    {
        m->addChar(m_kart_id);
        m->addShort(m_item_id);
        m->addChar(m_add_info);
    }   // serialise
};   // ItemInfo

#endif

