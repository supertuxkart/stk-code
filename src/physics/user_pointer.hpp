//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
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

#ifndef HEADER_USER_POINTER_HPP
#define HEADER_USER_POINTER_HPP

/** Some bullet objects store 'user pointers'. This is a base class
 *  that allows to easily determine the type of the user pointer.
 */
class AbstractKart;
class Flyable;
class Moveable;
class PhysicalObject;
class ThreeDAnimation;
class TriangleMesh;

/** A UserPointer is stored as a user pointer in all bullet bodies. This
 *  allows easily finding the appropriate STK object for a bullet body.
 */
class UserPointer
{
public:
    /** List of all possibles STK objects that are represented in the
     *  physics. */
    enum   UserPointerType {UP_UNDEF, UP_KART, UP_FLYABLE, UP_TRACK,
                            UP_PHYSICAL_OBJECT, UP_ANIMATION};
private:
    void*  m_pointer;
    UserPointerType m_user_pointer_type;
public:
    bool            is(UserPointerType t)      const {return m_user_pointer_type==t;     }
    TriangleMesh*   getPointerTriangleMesh()   const {return (TriangleMesh*)m_pointer;   }
    Moveable*       getPointerMoveable()       const {return (Moveable*)m_pointer;       }
    Flyable*        getPointerFlyable()        const {return (Flyable*)m_pointer;        }
    AbstractKart*   getPointerKart()           const {return (AbstractKart*)m_pointer;   }
    PhysicalObject *getPointerPhysicalObject() const {return (PhysicalObject*)m_pointer; }
    ThreeDAnimation*getPointerAnimation()      const {return (ThreeDAnimation*)m_pointer;}
    void            set(PhysicalObject* p) { m_user_pointer_type=UP_PHYSICAL_OBJECT;
                                             m_pointer          =p;           }
    void            set(AbstractKart* p)   { m_user_pointer_type=UP_KART;
                                             m_pointer          =p;           }
    void            set(Flyable* p)        { m_user_pointer_type=UP_FLYABLE;
                                             m_pointer          =p;           }
    void            set(TriangleMesh* p)   { m_user_pointer_type=UP_TRACK;
                                             m_pointer          =p;           }
    void            set(ThreeDAnimation* p){ m_user_pointer_type=UP_ANIMATION;
                                             m_pointer          =p;           }
                    UserPointer()          { zero();                          }
    void            zero()                 { m_user_pointer_type=UP_UNDEF;
                                             m_pointer          = NULL;       }
};
#endif
/* EOF */

