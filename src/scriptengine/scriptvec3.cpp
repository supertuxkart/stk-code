//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015  SuperTuxKart Team
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
#include "scriptvec3.hpp"
#include "scriptengine/aswrappedcall.hpp"

//debug
#include <iostream>


namespace Scripting
{
    void Constructor(void *memory)
    {
        // Initialize the pre-allocated memory by calling the
        // object constructor with the placement-new operator
        new(memory)SimpleVec3();
    }

    void Destructor(void *memory)
    {
        // Uninitialize the memory by calling the object destructor
        ((SimpleVec3*)memory)->~SimpleVec3();
    }

    void ConstructVector3FromFloats(float a, float b, float c, void *memory)
    {
        //Constructor using 3 floats
        new (memory)(SimpleVec3)(SimpleVec3(a, b, c));
    }

    //Print for debugging purposes
    void printVec3(asIScriptGeneric *gen)
    {
        SimpleVec3 *script_vec3 = (SimpleVec3*)gen->GetArgObject(0);
        std::cout << script_vec3->getX() << "," << script_vec3->getY() << "," << script_vec3->getZ() << std::endl;
    }

    float getX(SimpleVec3* v) { return v->getX(); }
    float getY(SimpleVec3* v) { return v->getY(); }
    float getZ(SimpleVec3* v) { return v->getZ(); }
    float getLength(SimpleVec3* v) { return v->getLength(); }

    void RegisterVec3(asIScriptEngine *engine)
    {
        bool mp = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
        asDWORD call_conv = mp ? asCALL_GENERIC : asCALL_CDECL_OBJLAST;
        int r; // of type asERetCodes
            
        r = engine->RegisterObjectType("Vec3", sizeof(SimpleVec3), asOBJ_VALUE | asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_CDA); assert(r >= 0);
        
        r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f()", 
                                            mp ? WRAP_OBJ_LAST(Constructor) : asFUNCTION(Constructor), 
                                            call_conv); assert(r >= 0);
                                            
        r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_DESTRUCT, "void f()", 
                                            mp ? WRAP_OBJ_LAST(Destructor) : asFUNCTION(Destructor), 
                                            call_conv); assert(r >= 0);
                                            
        r = engine->RegisterObjectMethod("Vec3", "Vec3 &opAssign(const Vec3 &in)", 
                                         mp ? WRAP_MFN_PR(SimpleVec3, operator =, (const SimpleVec3&), SimpleVec3&) 
                                            : asMETHODPR(SimpleVec3, operator =, (const SimpleVec3&), SimpleVec3&), 
                                         mp ? asCALL_GENERIC : asCALL_THISCALL); assert(r >= 0);
                                         
        r = engine->RegisterObjectBehaviour("Vec3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", 
                                            mp ? WRAP_OBJ_LAST(ConstructVector3FromFloats) 
                                               : asFUNCTION(ConstructVector3FromFloats), 
                                            call_conv); assert(r >= 0);
                                            
        r = engine->RegisterGlobalFunction("void printVec3(Vec3 a)", 
                                           asFUNCTION(printVec3), 
                                           asCALL_GENERIC); assert(r >= 0);
                                           
        r = engine->RegisterObjectMethod("Vec3", "float getX()", 
                                         mp ? WRAP_OBJ_LAST(getX) : asFUNCTION(getX), 
                                         call_conv); assert(r >= 0);
                                         
        r = engine->RegisterObjectMethod("Vec3", "float getY()", 
                                         mp ? WRAP_OBJ_LAST(getY) : asFUNCTION(getY), 
                                         call_conv); assert(r >= 0);
                                         
        r = engine->RegisterObjectMethod("Vec3", "float getZ()", 
                                         mp ? WRAP_OBJ_LAST(getZ) : asFUNCTION(getZ), 
                                         call_conv); assert(r >= 0);
                                         
        r = engine->RegisterObjectMethod("Vec3", "float getLength()", 
                                         mp ? WRAP_OBJ_LAST(getLength) : asFUNCTION(getLength), 
                                         call_conv); assert(r >= 0);
    }
}
