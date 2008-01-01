//  $Id: user_pointer.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_USER_POINTER_H
#define HEADER_USER_POINTER_H

/** Some bullet objects store 'user pointers'. This is a base class
 *  that allows to easily determine the type of the user pointer.
 */
class UserPointer
{
public:
    enum   UserPointerType {UP_UNDEF, UP_KART, UP_PROJECTILE, UP_TRACK} ;
protected:
    UserPointerType m_user_pointer_type;
public:
    UserPointerType getUserPointerType() const {return m_user_pointer_type;}
    void            setUserPointerType(UserPointerType t) 
                                                { m_user_pointer_type=t;   }
    UserPointer(): m_user_pointer_type(UP_UNDEF) {};
    UserPointer(UserPointerType t): m_user_pointer_type(t) {};
};
#endif
/* EOF */

