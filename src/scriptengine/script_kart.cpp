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

#include "script_kart.hpp"

#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "scriptvec3.hpp"
#include "scriptengine/aswrappedcall.hpp"

#include <assert.h>
#include <angelscript.h>
//debug
#include <iostream>

/** \cond DOXYGEN_IGNORE */
namespace Scripting
{
/** \endcond */

    namespace Kart
    {
        /** \addtogroup Scripting
          * @{
          */
        /** \addtogroup Kart
        * @{
        */

        /** Squashes the specified kart, for the specified time */
        void squash(int idKart, float time)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            kart->setSquash(time, 0.5);  //0.5 * max speed is new max for squashed duration
        }

        /** Teleports the kart to the specified Vec3 location */
        void teleport(int idKart, SimpleVec3* position)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            Vec3 v(position->getX(), position->getY(), position->getZ());
            kart->setXYZ(v);
            unsigned int index = World::getWorld()->getRescuePositionIndex(kart);
            btTransform s = World::getWorld()->getRescueTransform(index);
            s.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f), 0.0f));
            World::getWorld()->moveKartTo(kart, s);
        }
        
        /** Teleports the kart to the specified Vec3 location */
        void teleportExact(int idKart, SimpleVec3* position)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            Vec3 v(position->getX(), position->getY(), position->getZ());
            kart->setXYZ(v);
            btTransform s;
            s.setRotation(kart->getRotation());
            s.setOrigin(v);
            World::getWorld()->moveKartTo(kart, s);
        }

        /** Attempts to project kart to the given 2D location, to the position
          * with height 0, at a 45 degree angle.
          */
        // TODO: not sure what this function is for
        //void jumpTo(asIScriptGeneric *gen)
        //{
        //    //attempts to project kart to target destination
        //    //at present, assumes both initial and target location are
        //    //on the same horizontal plane (z=k plane) and projects
        //    //at a 45 degree angle.
        //    int id = (int)gen->GetArgDWord(0);
        //
        //    float x = gen->GetArgFloat(1);
        //    float y = gen->GetArgFloat(2);
        //    //float velocity = gen->GetArgFloat(3);
        //    //angle = pi/4 so t = v/(root 2 * g)
        //    //d = t * v/root 2 so d = v^2/(2g) => v = root(2dg)
        //    //component in x = component in y = root (dg)
        //    AbstractKart* kart = World::getWorld()->getKart(id);
        //    Vec3 pos = kart->getXYZ();
        //    float dx = x - pos[0];
        //    float dy = y - pos[2]; //blender uses xyz, bullet xzy
        //    float d = (sqrtf(dx*dx + dy*dy));
        //    float normalized_dx = dx / d;
        //    float normalized_dy = dy / d;
        //    float g = 9.81f;
        //    float velocity = sqrtf(d * g);
        //    
        //    kart->setVelocity(btVector3(velocity * normalized_dx, velocity, velocity * normalized_dy));
        //}
        
        /** Returns the location of the corresponding kart. */
        SimpleVec3 getLocation(int idKart)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            Vec3 v = kart->getXYZ();
            return SimpleVec3(v.getX(), v.getY(), v.getZ());
        }

        /** Sets the kart's velocity to the specified value. */
        void setVelocity(int idKart, SimpleVec3* position)
        {
            float x = position->getX();
            float y = position->getY();
            float z = position->getZ();

            AbstractKart* kart = World::getWorld()->getKart(idKart);
            kart->setVelocity(btVector3(x, y, z));
        }

        /** Gets the kart's velocity */
        SimpleVec3 getVelocity(int idKart)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            btVector3 velocity = kart->getVelocity();
            return SimpleVec3(velocity.getX(), velocity.getY(), velocity.getZ());
        }

        /** Gets the maximum speed (velocity) a kart can reach */
        float getMaxSpeed(int idKart)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            return kart->getKartProperties()->getEngineMaxSpeed();
        }

        /** Gets the maximum speed (velocity) a kart can reach */
        void changeKart(int idKart, std::string* new_id)
        {
            AbstractKart* kart = World::getWorld()->getKart(idKart);
            HandicapLevel hl = kart->getHandicap();
            auto ri = kart->getKartModel()->getRenderInfo();
            kart->changeKart(*new_id, hl, ri);
        }

        /** @}*/
        /** @}*/
        
        void registerScriptFunctions(asIScriptEngine *engine)
        {
            engine->SetDefaultNamespace("Kart");
            
            bool mp = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
            asDWORD call_conv = mp ? asCALL_GENERIC : asCALL_CDECL;
            int r; // of type asERetCodes
            
            r = engine->RegisterGlobalFunction("void squash(int id, float time)", 
                                               mp ? WRAP_FN(squash) : asFUNCTION(squash), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void teleport(int id, const Vec3 &in)", 
                                               mp ? WRAP_FN(teleport) : asFUNCTION(teleport), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void teleportExact(int id, const Vec3 &in)", 
                                               mp ? WRAP_FN(teleportExact) : asFUNCTION(teleportExact), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void setVelocity(int id, const Vec3 &in)", 
                                               mp ? WRAP_FN(setVelocity) : asFUNCTION(setVelocity), 
                                               call_conv); assert(r >= 0);
                                               
            //r = engine->RegisterGlobalFunction("void jumpTo(int id, float x, float y)", 
            //                                   mp ? WRAP_FN(jumpTo) : asFUNCTION(jumpTo), 
            //                                   call_conv); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("Vec3 getLocation(int id)", 
                                               mp ? WRAP_FN(getLocation) : asFUNCTION(getLocation), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("Vec3 getVelocity(int id)", 
                                               mp ? WRAP_FN(getVelocity) : asFUNCTION(getVelocity), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("float getMaxSpeed(int id)", 
                                               mp ? WRAP_FN(getMaxSpeed) : asFUNCTION(getMaxSpeed), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void changeKart(int id, string &in)",
                                               mp ? WRAP_FN(changeKart) : asFUNCTION(changeKart),
                                               call_conv); assert(r >= 0);
        }

        void registerScriptEnums(asIScriptEngine *engine)
        {
            // TODO: document enum in doxygen-generated scripting docs
            engine->SetDefaultNamespace("Kart");
            engine->RegisterEnum("PowerupType");
            engine->RegisterEnumValue("PowerupType", "ANVIL", PowerupManager::PowerupType::POWERUP_ANVIL);
            engine->RegisterEnumValue("PowerupType", "BOWLING", PowerupManager::PowerupType::POWERUP_BOWLING);
            engine->RegisterEnumValue("PowerupType", "BUBBLEGUM", PowerupManager::PowerupType::POWERUP_BUBBLEGUM);
            engine->RegisterEnumValue("PowerupType", "CAKE", PowerupManager::PowerupType::POWERUP_CAKE);
            engine->RegisterEnumValue("PowerupType", "PARACHUTE", PowerupManager::PowerupType::POWERUP_PARACHUTE);
            engine->RegisterEnumValue("PowerupType", "PLUNGER", PowerupManager::PowerupType::POWERUP_PLUNGER);
            engine->RegisterEnumValue("PowerupType", "RUBBERBALL", PowerupManager::PowerupType::POWERUP_RUBBERBALL);
            engine->RegisterEnumValue("PowerupType", "SWATTER", PowerupManager::PowerupType::POWERUP_SWATTER);
            engine->RegisterEnumValue("PowerupType", "SWITCH", PowerupManager::PowerupType::POWERUP_SWITCH);
            engine->RegisterEnumValue("PowerupType", "ZIPPER", PowerupManager::PowerupType::POWERUP_ZIPPER);
        }
    }

/** \cond DOXYGEN_IGNORE */
}
/** \endcond */
