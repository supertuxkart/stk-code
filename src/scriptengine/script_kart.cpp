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

//debug
#include <iostream>


namespace Scripting
{

    namespace Kart
    {
        //Squashes the specified kart, specified time
        void squashKart(asIScriptGeneric *gen)
        {
            int id = (int)gen->GetArgDWord(0);
            float time = gen->GetArgFloat(1);
            AbstractKart* kart = World::getWorld()->getKart(id);
            kart->setSquash(time, 0.5);  //0.5 * max speed is new max for squashed duration
        }
        //Teleports the kart to the specified Vec3 location
        void teleportKart(asIScriptGeneric *gen)
        {
            int id = (int)gen->GetArgDWord(0);
            Vec3 *position = (Vec3*)gen->GetArgAddress(1);

            float x = position->getX();
            float y = position->getY();
            float z = position->getZ();

            AbstractKart* kart = World::getWorld()->getKart(id);
            kart->setXYZ(btVector3(x, y, z));
            unsigned int index = World::getWorld()->getRescuePositionIndex(kart);
            btTransform s = World::getWorld()->getRescueTransform(index);
            const btVector3 &xyz = s.getOrigin();
            float angle = atan2(0, 0);
            s.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f), angle));
            World::getWorld()->moveKartTo(kart, s);
        }
        //Attempts to project kart to the given 2D location, to the position
        //with height 0, at a 45 degree angle.
        void jumpKartTo(asIScriptGeneric *gen)
        {
            //attempts to project kart to target destination
            //at present, assumes both initial and target location are
            //on the same horizontal plane (z=k plane) and projects
            //at a 45 degree angle.
            int id = (int)gen->GetArgDWord(0);

            float x = gen->GetArgFloat(1);
            float y = gen->GetArgFloat(2);
            //float velocity = gen->GetArgFloat(3);
            //angle = pi/4 so t = v/(root 2 * g)
            //d = t * v/root 2 so d = v^2/(2g) => v = root(2dg)
            //component in x = component in y = root (dg)
            AbstractKart* kart = World::getWorld()->getKart(id);
            Vec3 pos = kart->getXYZ();
            float dx = x - pos[0];
            float dy = y - pos[2]; //blender uses xyz, bullet xzy
            float d = (sqrtf(dx*dx + dy*dy));
            float normalized_dx = dx / d;
            float normalized_dy = dy / d;
            float g = 9.81;
            float velocity = sqrtf(d * g);
            
            kart->setVelocity(btVector3(velocity * normalized_dx, velocity, velocity * normalized_dy));
        }
        //Bind kart location
        void getKartLocation(asIScriptGeneric *gen)
        {
            int id = (int)gen->GetArgDWord(0);

            AbstractKart* kart = World::getWorld()->getKart(id);
            Vec3 kart_loc = kart->getXYZ();
            void *pointer = &kart_loc;

            gen->SetReturnObject(pointer);
        }
        //Bind setter for velocity
        void setVelocity(asIScriptGeneric *gen)
        {
            int id = (int)gen->GetArgDWord(0);
            Vec3 *position = (Vec3*)gen->GetArgAddress(1);

            float x = position->getX();
            float y = position->getY();
            float z = position->getZ();

            AbstractKart* kart = World::getWorld()->getKart(id);
            kart->setVelocity(btVector3(x, y, z));
        }
        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r;
            r = engine->RegisterGlobalFunction("void squashKart(int id, float time)", asFUNCTION(squashKart), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void teleportKart(int id, Vec3 &in)", asFUNCTION(teleportKart), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void setVelocity(int id, Vec3 &in)", asFUNCTION(setVelocity), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void jumpKartTo(int id, float x, float y)", asFUNCTION(jumpKartTo), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("Vec3 getKartLocation(int id)", asFUNCTION(getKartLocation), asCALL_GENERIC); assert(r >= 0);
        }
    }
}
