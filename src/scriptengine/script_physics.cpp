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

#include <assert.h>
#include <angelscript.h>
#include "script_physics.hpp"


namespace Scripting{
    
    namespace Physics{
        
        void getCollidingKart1(asIScriptGeneric *gen)
        {
            gen->SetReturnDWord(m_collidingkartid1);
        }
        void getCollidingKart2(asIScriptGeneric *gen)
        {
            gen->SetReturnDWord(m_collidingkartid2);
        }
        void setCollision(int collider1,int collider2)
        {
            m_collidingkartid1 = collider1;
            m_collidingkartid2 = collider2;
        }
        
        asIScriptFunction* registerScriptCallbacks(asIScriptEngine *engine)
        {
            asIScriptFunction *func;
            func = engine->GetModule(0)->GetFunctionByDecl("void onCollision()");
            return func;
        }
        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r;
            r = engine->RegisterGlobalFunction("uint getCollidingKart1()", asFUNCTION(getCollidingKart1), asCALL_GENERIC); assert( r >= 0 );
            r = engine->RegisterGlobalFunction("uint getCollidingKart2()", asFUNCTION(getCollidingKart2), asCALL_GENERIC); assert( r >= 0 );

        }
    }
}
