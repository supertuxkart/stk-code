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

#ifndef HEADER_FLYABLE_INFO_HPP
#define HEADER_FLYABLE_INFO_HPP

#include "network/message.hpp"

/** Class used to transfer information about projectiles from server to client.
 *  It contains only the coordinates, rotation, and explosion state.    
 */
class FlyableInfo
{
public:
    Vec3         m_xyz;           /** Position of object. */
    btQuaternion m_rotation;      /** Orientation of object */
    bool         m_exploded;      /** If the object exploded in the current frame. */

    /** Constructor to initialise all fields. 
     */
    FlyableInfo(const Vec3& xyz, const btQuaternion &rotation, bool exploded) :
        m_xyz(xyz), m_rotation(rotation), m_exploded(exploded)
       {};
    // ------------------------------------------------------------------------
    /** Allow this object to be stored in std::vector fields. 
     */
    FlyableInfo() {};
    // ------------------------------------------------------------------------
    /** Construct a FlyableInfo from a message (which is unpacked).
     */
    FlyableInfo(Message *m)
    {
        m_xyz      = m->getVec3();
        m_rotation = m->getQuaternion();
        m_exploded = m->getBool();
    }   // FlyableInfo(Message)
    // ------------------------------------------------------------------------
    /** Returns the length of the serialised message. */
    static int getLength()
    {
        return Message::getVec3Length()
             + Message::getQuaternionLength()
             + Message::getBoolLength();
    }   // getLength
    // ------------------------------------------------------------------------
    void serialise(Message *m)
    {
        m->addVec3(m_xyz);
        m->addQuaternion(m_rotation);
        m->addBool(m_exploded);
    }   // serialise
};

#endif

