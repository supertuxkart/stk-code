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
#include "script_kart.hpp"

//debug
#include <iostream>


namespace Scripting
{


    void Constructor(void *memory)
    {
        // Initialize the pre-allocated memory by calling the
        // object constructor with the placement-new operator
        new(memory)Vec3();
    }
    void Destructor(void *memory)
    {
        // Uninitialize the memory by calling the object destructor
        ((Vec3*)memory)->~Vec3();
    }
    void ConstructVector3FromFloats(float a, float b, float c, void *memory){
        //Constructor using 3 floats
        new (memory)(Vec3)(Vec3(a, b, c));
    }
    //Print for debugging purposes
    void printVec3(asIScriptGeneric *gen)
    {
        Vec3 *script_vec3 = (Vec3*)gen->GetArgObject(0);
        std::cout << script_vec3->getX() << "," << script_vec3->getY() << "," << script_vec3->getZ() << std::endl;
    }
    void RegisterVec3(asIScriptEngine *engine)
    {
        int r;
        r = engine->RegisterObjectType("Vec3", sizeof(Vec3), asOBJ_VALUE); assert(r >= 0);
        // Register the behaviours
        r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Constructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
        r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
        r = engine->RegisterObjectMethod("Vec3", "Vec3 &opAssign(const Vec3 &in)", asMETHODPR(Vec3, operator =, (const Vec3&), Vec3&), asCALL_THISCALL); assert(r >= 0);
        r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(ConstructVector3FromFloats), asCALL_CDECL_OBJLAST); assert(r >= 0);
        r = engine->RegisterGlobalFunction("void printVec3(Vec3 a)", asFUNCTION(printVec3), asCALL_GENERIC); assert(r >= 0);

        
    }
}
