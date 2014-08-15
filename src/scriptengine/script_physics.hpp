//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014  SuperTuxKart Team
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

#ifndef HEADER_SCRIPT_PHYSICS_HPP
#define HEADER_SCRIPT_PHYSICS_HPP

#include <angelscript.h>
#include <string>

namespace Scripting
{
    
    namespace Physics
    {
        
        //IDs of kart collisions
        int m_collidingkartid1;
        int m_collidingkartid2;

        //Details of collision
        std::string m_collider1;
        std::string m_collider2;
        std::string m_collisionType;
        
        //script engine functions
        void registerScriptFunctions(asIScriptEngine *engine);
        asIScriptFunction* 
            registerScriptCallbacks(asIScriptEngine *engine);


        //game engine functions
        void setCollision(int collider1, int collider2);
        void setCollisionType(std::string);



        //script-bound functions
        void getCollidingKart1(asIScriptGeneric *gen);
        void getCollidingKart2(asIScriptGeneric *gen);
        void getCollsionType(asIScriptGeneric *gen);
        void getCollidingID(asIScriptGeneric *gen);

    }
    
    
    
    
    
    
}
#endif
