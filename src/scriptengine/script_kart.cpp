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
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "script_kart.hpp"



namespace Scripting{

    namespace Kart{

        void squashKart(asIScriptGeneric *gen)
        {
            int id = (int)gen->GetArgDWord(0);
            float time = gen->GetArgFloat(1);
            AbstractKart* kart = World::getWorld()->getKart(id);
            kart->setSquash(time, 0.5);  //0.5 * max speed is new max for squashed duration
        }

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r;
            r = engine->RegisterGlobalFunction("void squashKart(int id, float time)", asFUNCTION(squashKart), asCALL_GENERIC); assert(r >= 0);

        }
    }
}
